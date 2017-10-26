/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDWASABIBayesCalibration
//- Description: Derived class for Bayesian inference using WASABI
//- Owner:       Tim Wildey
//- Checked by:
//- Version:

#include "NonDWASABIBayesCalibration.hpp"
#include "ProblemDescDB.hpp"
#include "DakotaModel.hpp"
#include "ProbabilityTransformation.hpp"
#include "NonDSampling.hpp"

// BMA TODO: remove this header
// for uniform PDF and samples
//#include "pdflib.hpp"

// included to set seed in RNGLIB:
#include "rnglib.hpp"
//#include <random>

#define DEBUG

using std::string;

namespace Dakota {


/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDWASABIBayesCalibration::
NonDWASABIBayesCalibration(ProblemDescDB& problem_db, Model& model):
  NonDBayesCalibration(problem_db, model),
  numPushforwardSamples(probDescDB.get_int("method.nond.pushforward_samples")),
  dataDistMeans(probDescDB.get_rv("method.nond.data_dist_means")),
  dataDistCovariance(probDescDB.get_rv("method.nond.data_dist_covariance")),
  dataDistFilename(probDescDB.get_string("method.nond.data_dist_filename")),
  dataDistCovType(probDescDB.get_string("method.nond.data_dist_cov_type")),
  posteriorSamplesImportFile(probDescDB.get_string("method.nond.posterior_samples_import_file")),
  posteriorSamplesImportFormat(0), // not used
  exportPosteriorDensityFile(probDescDB.get_string("method.nond.posterior_density_export_file")),
  exportPosteriorSamplesFile(probDescDB.get_string("method.nond.posterior_samples_export_file")),
  exportFileFormat(0), // not used
  generateRandomPosteriorSamples(probDescDB.get_bool("method.nond.generate_posterior_samples")),
  evaluatePosteriorDensity(probDescDB.get_bool("method.nond.evaluate_posterior_density"))
{ 
  // don't use max_function_evaluations, since we have num_samples
  // consider max_iterations = generations, and adjust as needed?

}


NonDWASABIBayesCalibration::~NonDWASABIBayesCalibration()
{ }


/** Perform the uncertainty quantification */
void NonDWASABIBayesCalibration::calibrate()
{

  ////////////////////////////////////////////////////////
  // Step 1 of 10: Build the response surface approximation (RSA)
  ////////////////////////////////////////////////////////
  
  initialize_model();
  if (emulatorType == NO_EMULATOR) {
    Cerr << "\nError: WASABI requires an emulator!"<<std::endl;
    abort_handler(METHOD_ERROR);
  }

  // set the seed for the rng 
  if (randomSeed) {
    rnumGenerator.seed(randomSeed);
    Cout << " WASABI Seed (user-specified) = " << randomSeed << std::endl;
  }
  else {
    // Use NonD convenience function for system seed
    int clock_seed = generate_system_seed();
    rnumGenerator.seed(clock_seed);
    Cout << " WASABI Seed (system-generated) = " << clock_seed << std::endl;
  }
 
  // The experimental data is loaded in a base class.  We assume that
  // the distribution on the data will either be given as a Gaussian
  // or estimated using a KDE.

  // initialize the prior PDF and sampler
  // the prior is currently assumed uniform, but this will be generalized

  // set the bounds on the parameters (TMW: can this be moved?  
  // Does it require initialize_model() to be called first? 
  // resize, initializing to zero
  paramMins.size(numContinuousVars);
  paramMaxs.size(numContinuousVars);
  RealRealPairArray bnds = (standardizedSpace) ?
    natafTransform.u_bounds() : natafTransform.x_bounds();
  for (size_t i=0; i<numContinuousVars; ++i)
    { paramMins[i] = bnds[i].first; paramMaxs[i] = bnds[i].second; }

  // TMW: evaluation of prior should be elevated to NonDBayes 
  // (even for MCMC-based methods)
  // Concerned about application of nataf for dependent priors
  // May want to use KDE to generate prior samples if given samples from prior
  // Nataf should be encapsulated in new pecos model class

  Cout << "INFO (WASABI): paramMins  " << paramMins << '\n';
  Cout << "INFO (WASABI): paramMaxs  " << paramMaxs << '\n';

  ////////////////////////////////////////////////////////
  // Step 2 of 10: Generate a large set of samples (s_prior) from the prior
  ////////////////////////////////////////////////////////

  // diagnostic information
  // BMA: changed this from chainSamples to avoid confusion
  Cout << "INFO (WASABI): Num Samples " << numPushforwardSamples << '\n';
 
  RealMatrix samples_from_prior((int)numContinuousVars, numPushforwardSamples, false);
 
  for (int j=0; j<numPushforwardSamples; j++) {
    RealVector samp_j(Teuchos::View, samples_from_prior[j], numContinuousVars);
    prior_sample(rnumGenerator, samp_j);
  }
# ifdef DEBUG
  Cout << "samples_from_prior " << samples_from_prior;
# endif 

  ////////////////////////////////////////////////////////
  // Step 3 of 10: Evaluate the response surface at these samples
  ////////////////////////////////////////////////////////

  RealMatrix pushforward_responses_from_prior;
  compute_responses(samples_from_prior, pushforward_responses_from_prior);
# ifdef DEBUG
  Cout << "pushforward_responses_from_prior " << pushforward_responses_from_prior;  
#endif 

  ////////////////////////////////////////////////////////
  // Step 4 of 10: Build a density estimate using the samples of the RSA
  ////////////////////////////////////////////////////////

  // compute_responses returns a matrix (num_qoi x num_samples)
  // but kde.inititalize expects the transpose of this matrix
  Pecos::DensityEstimator response_kde("gaussian_kde");
  response_kde.initialize(pushforward_responses_from_prior, Teuchos::TRANS );

  ////////////////////////////////////////////////////////
  // Step 5 of 10: Pick a set of points (s_eval) to evaluate 
  // the posterior (default: s_eval = s_prior)
  ////////////////////////////////////////////////////////

  // right now we are using the samples from the prior

  RealMatrix samples_for_posterior_eval(samples_from_prior);
  if ( posteriorSamplesImportFile.empty() ){
    samples_for_posterior_eval.shapeUninitialized( samples_from_prior.numRows(),
						   samples_from_prior.numCols());
    samples_for_posterior_eval.assign(samples_from_prior);
  }else{
    bool verbose=(outputLevel>NORMAL_OUTPUT);
    std::ifstream file_stream;
    TabularIO::open_file(file_stream, posteriorSamplesImportFile, 
			 "samples_for_posterior_eval");
    RealVectorArray va;
    read_unsized_data(file_stream, va);
    copy_data(va, samples_for_posterior_eval );
    if (outputLevel) 
      std::cout << "evaluating the posterior at " 
		<< samples_for_posterior_eval.numCols() << " points\n";
  }

  ////////////////////////////////////////////////////////
  // Step 6 of 10: Evaluate the prior density at samples_for_posterior_eval
  ////////////////////////////////////////////////////////

  RealVector prior_density_vals(samples_for_posterior_eval.numCols(), false);

  for (int j=0; j<samples_for_posterior_eval.numCols(); j++) {
    RealVector samp_j(Teuchos::View, samples_for_posterior_eval[j],
		      numContinuousVars);
    prior_density_vals[j] = prior_density(samp_j);
  }
  
  ////////////////////////////////////////////////////////
  // Step 7 of 10: Evaluate the RSA at s_eval -> q_eval = RSA(s_eval)
  ////////////////////////////////////////////////////////

  // if prior samples are used, no need to re-evaluate on the surrogate 
  RealMatrix responses_for_posterior_eval;
  if ( !posteriorSamplesImportFile.empty() )
    compute_responses(samples_for_posterior_eval, responses_for_posterior_eval);
  else 
    responses_for_posterior_eval = pushforward_responses_from_prior;

#ifdef DEBUG
  Cout << "responses_for_posterior_eval " << responses_for_posterior_eval;  
#endif

  ////////////////////////////////////////////////////////
  // Step 8 of 10: Evaluate the density at q_eval
  ////////////////////////////////////////////////////////

  RealVector response_density_vals_for_posterior_eval;
  response_kde.pdf(responses_for_posterior_eval, 
		   response_density_vals_for_posterior_eval,
		   Teuchos::TRANS);

  ////////////////////////////////////////////////////////
  // Step 9 of 10: Evaluate the given data distribution at q_eval
  ////////////////////////////////////////////////////////
  int num_obs_data;
  if (dataDistMeans.length() > 0){
    num_obs_data = dataDistMeans.length();
    Cout << "INFO(WASABI): num_obs_data " << num_obs_data << '\n';
  }  
  else if (!dataDistFilename.empty()) {
    std::ifstream file_stream;
    TabularIO::open_file(file_stream, dataDistFilename,
                         "obs_data");
    RealVectorArray va;
    read_unsized_data(file_stream, va);
    dataDistMeans = va[0];
    dataDistCovariance = va[1];
    num_obs_data = dataDistMeans.length();
    Cout << "INFO(WASABI): num_obs_data " << num_obs_data << '\n';
  }
  else { 
    Cerr << "INFO(WASABI):  You need to provide observational data" 
         << " for the Wasabi method. " << '\n';
    abort_handler(IO_ERROR);
  }
#ifdef DEBUG 
  Cout << "Data Distribution Means " << dataDistMeans << '\n';
  Cout << "Data Distribution Covariance " << dataDistCovariance << '\n';
#endif
  //boost::math::normal datadist(mean_data, stdev_data);
  std::vector<boost::math::normal> datadists;
  for (int k=0; k <num_obs_data; k++) 
    datadists.push_back(boost::math::normal(dataDistMeans[k],dataDistCovariance[k]));
 
  RealVector data_density_vals(samples_for_posterior_eval.numCols(), false);

  for (int j=0; j<samples_for_posterior_eval.numCols(); j++) {
    double currval = responses_for_posterior_eval(0,j);
    data_density_vals[j] = 1.0;
    for (int k=0; k<num_obs_data; k++) {
      double dataval = boost::math::pdf(datadists[k], currval);
      data_density_vals[j] *= dataval;
    }
  }
#ifdef DEBUG 
  Cout << "Data density values " << data_density_vals << '\n';
#endif

  ////////////////////////////////////////////////////////
  // Step 10 of 10: Compute the posterior distribution at s_eval
  ////////////////////////////////////////////////////////

  RealVector posterior_density(samples_for_posterior_eval.numCols(), false);
  for (int j=0; j<samples_for_posterior_eval.numCols(); j++) {
    //note:  originally this was: 
    //posterior_density[j] = prior_density_vals[j] * (data_density_vals[j] / 
    //			  response_density_vals_for_posterior_eval[j]);
    //Tim suggested eliminating the multiplication and then division by the 
    //prior to get the accept/reject ratio below.  So, at this stage, it 
    //is not truly a posterior density.  That happens in step 11.	
    posterior_density[j] = (data_density_vals[j] / 
			  response_density_vals_for_posterior_eval[j]);
  }

  // calculate information and K-L 
  Real sum1=0; Real sum2=0;
  for (int j=0; j<samples_for_posterior_eval.numCols(); j++) {
    sum1 += posterior_density[j];
    sum2 += posterior_density[j]*log(posterior_density[j]);
  }
  Cout << "Information ratio of observed/push forward prior " 
       <<  sum1/samples_for_posterior_eval.numCols() << '\n';
  Cout << "K-L divergence of observed/push forward prior " 
       <<  sum2/samples_for_posterior_eval.numCols() << '\n';
    
#ifdef DEBUG 
  Cout << "Posterior density values " << posterior_density << '\n';
#endif 
  ////////////////////////////////////////////////////////
  // Step 11 (optional): Use an acceptance-rejection algorithm 
  // to select a subset of the samples consistent with the posteror 
  ////////////////////////////////////////////////////////

  std::vector<int> points_to_keep;
  boost::random::mt19937 rng;
  boost::random::uniform_real_distribution<double> distribution(0.0, 1.0);

  if ( !generateRandomPosteriorSamples || !evaluatePosteriorDensity ){
    Cout << "Must specify at least one of generate_posterior_samples "
         << " and evaluate_posterior_density";
    abort_handler(IO_ERROR);
  }

  if ( generateRandomPosteriorSamples )
    for (int j=0; j<samples_for_posterior_eval.numCols(); j++) {
      //double ratio = posterior_density[j] / prior_density_vals[j];
      double ratio = posterior_density[j];
      double rnum = distribution(rng);
      if (ratio > rnum)
	points_to_keep.push_back(j);
    }
  // Thus far, we have not calculated the true posterior to eliminate 
  // unnecessary division.  But we need to correct that now and 
  // multiply by prior.  Also calculate the mean of the posterior now. 

  momentStatistics.shapeUninitialized(1,numFunctions);
  for (int k=0; k<numFunctions; k++) 
    momentStatistics(0,k)=0.0; 
  for (int j=0; j<samples_for_posterior_eval.numCols(); j++) {
    posterior_density[j] = posterior_density[j]*prior_density_vals[j];
    for (int k=0; k<numFunctions; k++) 
      momentStatistics(0,k) += responses_for_posterior_eval(k,j)*posterior_density[j];
  }
  for (int k=0; k<numFunctions; k++){ 
    momentStatistics(0,k) = momentStatistics(0,k)/samples_for_posterior_eval.numCols();
  }
#ifdef DEBUG 
  Cout << "Posterior density values " << posterior_density << '\n';
#endif 

  if ( !exportPosteriorSamplesFile.empty() ){
    RealMatrix posterior_data;
    extract_selected_posterior_samples(points_to_keep,
				       samples_for_posterior_eval,
				       posterior_density,
				       posterior_data );
    export_posterior_samples_to_file( exportPosteriorSamplesFile, 
				      posterior_data );
  }

  if ( evaluatePosteriorDensity ){
    points_to_keep.resize( samples_for_posterior_eval.numCols() ) ;
    for (int j=0; j<samples_for_posterior_eval.numCols(); j++)
      points_to_keep[j] = j;
    if ( !exportPosteriorDensityFile.empty() ){
      RealMatrix posterior_data;
      extract_selected_posterior_samples(points_to_keep,
					 samples_for_posterior_eval,
					 posterior_density,
					 posterior_data );
      export_posterior_samples_to_file( exportPosteriorDensityFile, 
					posterior_data );
    } 
  }
  return;
}

void NonDWASABIBayesCalibration::
extract_selected_posterior_samples(const std::vector<int> &points_to_keep,
				   const RealMatrix &samples_for_posterior_eval,
				   const RealVector &posterior_density,
				   RealMatrix &posterior_data ) const {
  int num_points_to_keep = points_to_keep.size();
  posterior_data.shapeUninitialized( num_points_to_keep, 
				     numContinuousVars + 1 );
  RealMatrix posterior_samples( Teuchos::View, posterior_data, 
				num_points_to_keep, numContinuousVars );
  for (size_t i=0;i<num_points_to_keep;i++)
    for (size_t j=0;j<numContinuousVars;j++)
      posterior_samples(i,j) = samples_for_posterior_eval(j,points_to_keep[i]);
  
  RealVector posterior_values( Teuchos::View, 
			       posterior_data[numContinuousVars],
			       num_points_to_keep );
  for (size_t i=0;i<num_points_to_keep;i++)
    posterior_values[i] = posterior_density[points_to_keep[i]];
}

void NonDWASABIBayesCalibration::
export_posterior_samples_to_file( const std::string filename, 
				  const RealMatrix &posterior_data ) const{
  std::ofstream export_file_stream;
  TabularIO::open_file(export_file_stream, filename,
		       "posterior samples and values file export");
  bool brackets = false, row_rtn = true, final_rtn = true;
  Dakota::write_data(export_file_stream, posterior_data,
		     brackets, row_rtn, final_rtn);
  export_file_stream.close();
}


void NonDWASABIBayesCalibration::
print_results(std::ostream& s, short results_state)
{
  //NonDBayesCalibration::print_results(s, results_state);

  // WASABI-specific output
  StringArray resp_labels = mcmcModel.current_response().function_labels();
  NonDSampling::print_moments(s, momentStatistics, RealMatrix(),
      "response function", STANDARD_MOMENTS, resp_labels, false);
}


void NonDWASABIBayesCalibration::
compute_responses(RealMatrix & samples, RealMatrix & responses)
{

  int num_samples = samples.numCols();
  responses.shapeUninitialized(numFunctions, num_samples);

  for (int j=0; j<num_samples; j++) {
    RealVector sample(Teuchos::View, samples[j], numContinuousVars);
    
    mcmcModel.continuous_variables(sample); 

    mcmcModel.evaluate();
    const Response& curr_resp = mcmcModel.current_response();
    const RealVector& fn_vals = curr_resp.function_values();

    RealVector response_col(Teuchos::View, responses[j], numFunctions); 
    response_col.assign(fn_vals);
  }
}

} // namespace Dakota
