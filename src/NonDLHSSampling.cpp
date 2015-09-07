/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDSampling
//- Description: Implementation code for NonDSampling class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NonDLHSSampling.hpp"
#include "ProblemDescDB.hpp"
#include "ReducedBasis.hpp"

#include "Teuchos_LAPACK.hpp"
#include "Teuchos_SerialDenseSolver.hpp"
#include "Teuchos_SerialDenseHelpers.hpp"
#include "DakotaApproximation.hpp"

static const char rcsId[]="@(#) $Id: NonDLHSSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDLHSSampling::NonDLHSSampling(ProblemDescDB& problem_db, Model& model):
  NonDSampling(problem_db, model), numResponseFunctions(0),
  pcaFlag(probDescDB.get_bool("method.principal_components")),
  varBasedDecompFlag(probDescDB.get_bool("method.variance_based_decomp")),
  percentVarianceExplained(
    probDescDB.get_real("method.percent_variance_explained"))
{ 
  if (model.primary_fn_type() == GENERIC_FNS)
    numResponseFunctions = model.num_primary_fns();
}


/** This alternate constructor is used for generation and evaluation
    of Model-based sample sets.  A set_db_list_nodes has not been
    performed so required data must be passed through the constructor.
    It's purpose is to avoid the need for a separate LHS specification
    within methods that use LHS sampling. */
NonDLHSSampling::
NonDLHSSampling(Model& model, unsigned short sample_type, int samples,
		int seed, const String& rng, bool vary_pattern,
		short sampling_vars_mode): 
  NonDSampling(RANDOM_SAMPLING, model, sample_type, samples, seed, rng,
	       vary_pattern, sampling_vars_mode),
  numResponseFunctions(numFunctions), pcaFlag(false), varBasedDecompFlag(false)
{ }


/** This alternate constructor is used by ConcurrentStrategy for
    generation of uniform, uncorrelated sample sets.  It is _not_ a
    letter-envelope instantiation and a set_db_list_nodes has not been
    performed.  It is called with all needed data passed through the
    constructor and is designed to allow more flexibility in variables
    set definition (i.e., relax connection to a variables
    specification and allow sampling over parameter sets such as
    multiobjective weights).  In this case, a Model is not used and 
    the object must only be used for sample generation (no evaluation). */
NonDLHSSampling::
NonDLHSSampling(unsigned short sample_type, int samples, int seed,
		const String& rng, const RealVector& lower_bnds,
		const RealVector& upper_bnds): 
  NonDSampling(sample_type, samples, seed, rng, lower_bnds, upper_bnds),
  numResponseFunctions(0), pcaFlag(false), varBasedDecompFlag(false)
{
  // since there will be no late data updates to capture in this case
  // (no sampling_reset()), go ahead and get the parameter sets.
  get_parameter_sets(lower_bnds, upper_bnds);
}

/** This alternate constructor is used to generate correlated normal 
    sample sets.  It is _not_ a letter-envelope instantiation and a 
    set_db_list_nodes has not been performed.  
    It is called with all needed data passed through the
    constructor.  In this case, a Model is not used and 
    the object must only be used for sample generation (no evaluation). */
NonDLHSSampling::
NonDLHSSampling(unsigned short sample_type, int samples, int seed,
		const String& rng, const RealVector& means, 
                const RealVector& std_devs, const RealVector& lower_bnds,
		const RealVector& upper_bnds, RealSymMatrix& correl): 
  NonDSampling(sample_type, samples, seed, rng, means, std_devs, lower_bnds, upper_bnds, correl),
  numResponseFunctions(0), pcaFlag(false), varBasedDecompFlag(false)
{
  // since there will be no late data updates to capture in this case
  // (no sampling_reset()), go ahead and get the parameter sets.
  get_parameter_sets(means, std_devs, lower_bnds, upper_bnds, correl);
}

NonDLHSSampling::~NonDLHSSampling()
{ }


void NonDLHSSampling::pre_run()
{
  // run LHS to generate parameter sets
  if (!varBasedDecompFlag)
    get_parameter_sets(iteratedModel);
}


void NonDLHSSampling::post_input()
{
  size_t cv_start, num_cv, div_start, num_div, dsv_start, num_dsv,
    drv_start, num_drv;
  mode_counts(iteratedModel, cv_start, num_cv, div_start, num_div,
	      dsv_start, num_dsv, drv_start, num_drv);
  size_t num_vars = num_cv + num_div + num_dsv + num_drv;
  // call convenience function from Analyzer
  read_variables_responses(numSamples, num_vars);
}


/** Loop over the set of samples and compute responses.  Compute
    statistics on the set of responses if statsFlag is set. */
void NonDLHSSampling::quantify_uncertainty()
{
  // If VBD has been selected, evaluate a series of parameter sets
  // (each of the size specified by the user) in order to compute VBD metrics.
  // If there are active discrete vars, they are included within allSamples.
  if (varBasedDecompFlag)
    variance_based_decomp(numContinuousVars, numDiscreteIntVars,
			  numDiscreteRealVars, numSamples);
  // if VBD has not been selected, generate allResponses from a single
  // set of allSamples or allVariables
  else {
    bool log_resp_flag = (allDataFlag || statsFlag);
    bool log_best_flag = !numResponseFunctions; // DACE mode w/ opt or NLS
    evaluate_parameter_sets(iteratedModel, log_resp_flag, log_best_flag);
  }
}


void NonDLHSSampling::post_run(std::ostream& s)
{
  //Statistics are generated here and output in NonDLHSSampling's
  // redefinition of print_results().
  if (statsFlag && !varBasedDecompFlag) // calculate statistics on allResponses
    compute_statistics(allSamples, allResponses);

  Analyzer::post_run(s);
 
  if (pcaFlag)
    compute_pca(s);
}


void NonDLHSSampling::compute_pca(std::ostream& s)
{
  IntRespMCIter r_it; size_t fn, samp;
  RealMatrix responseMatrix;
  responseMatrix.reshape(numSamples,numFunctions);
  for (samp=0, r_it=allResponses.begin(); samp<numSamples; ++samp, ++r_it)
    for (fn=0; fn<numFunctions; ++fn) 
      responseMatrix(samp,fn) = r_it->second.function_value(fn);
  s << "numFunctions " << numFunctions << "\nnumSamples " << numSamples << '\n';
  if (outputLevel == DEBUG_OUTPUT) {
    s << "Original Data\n";
    for (samp=0; samp<numSamples; ++samp) {
      for (fn=0; fn<numFunctions; ++fn)
	s << responseMatrix(samp,fn) << "  " ;
      s << '\n';
    }
    s << std::endl;
  }
  // Compute the SVD (includes centering the matrix)
  ReducedBasis pcaReducedBasis; 
  pcaReducedBasis.set_matrix(responseMatrix);
  pcaReducedBasis.update_svd();

  /*ReducedBasis::VarianceExplained trunc(0.99);
    RealVector sing_values = pcaReducedBasis.get_singular_values(trunc);
    int num_values = sing_values.length();
    s << "num_values " << num_values << '\n'; 
  */

  // Get the centered version of the original response matrix
  RealMatrix centered_matrix = pcaReducedBasis.get_matrix();
  if (outputLevel == DEBUG_OUTPUT) {
    s << "Centered matrix\n";
    centered_matrix.print(std::cout);
    s << std::endl;
  }

  // for now get the first factor score
  RealMatrix principal_comp
    = pcaReducedBasis.get_right_singular_vector_transpose();
    
  if (outputLevel == DEBUG_OUTPUT) {
    s << "principal components\n";
    for (samp=0; samp<numSamples; ++samp) {
      for (fn=0; fn<numFunctions; ++fn)
	s << principal_comp(samp,fn) << "  " ;
      s << '\n';
    }
    s << std::endl;
  }
    
  RealMatrix factor_scores(numSamples, numFunctions);
  int myerr = factor_scores.multiply(Teuchos::NO_TRANS, Teuchos::TRANS, 1., 
				     centered_matrix, principal_comp, 0.);
 
  RealMatrix
    f_scores(Teuchos::Copy, factor_scores, numSamples, numSamples, 0, 0);
  if (outputLevel == DEBUG_OUTPUT) {
    s << "myerr" << myerr <<'\n';
    s << "FactorScores\n";
    for (samp=0; samp<numSamples; ++samp) {
      for (fn=0; fn<numSamples; ++fn)
	s << f_scores(samp,fn) << " " ;
      s << '\n';
    }
    s << std::endl;
  }
  //Dakota::compute_svd( responseMatrix, numSamples, numFunctions,
  //                     singular_values );
  RealVector singular_values = pcaReducedBasis.get_singular_values();
  if (outputLevel == DEBUG_OUTPUT)
    s << "singular values " << singular_values <<'\n';
  s << std::endl;

  //Build GPs
  // Get number of principal components to retain
  s << "percent_variance_explained " << percentVarianceExplained << '\n';
  ReducedBasis::VarianceExplained truncation(percentVarianceExplained);
  int num_signif_Pcomps = truncation.get_num_components(pcaReducedBasis);
  s << "number of significant principal components " <<  num_signif_Pcomps
    << '\n' << std::endl;

  String approx_type; 
  approx_type = "global_kriging";  // Surfpack GP
  UShortArray approx_order;
  short data_order = 1;  // assume only function values
  short output_level = NORMAL_OUTPUT;
  SharedApproxData sharedData;

  sharedData = SharedApproxData(approx_type, approx_order, numContinuousVars,
				data_order, output_level);
	           
  std::vector<Approximation> gpApproximations;

  // build one GP for each Principal Component
  for (int i = 0; i < num_signif_Pcomps; ++i) {
    gpApproximations.push_back(Approximation(sharedData));
  }           
  for (int i = 0; i < num_signif_Pcomps; ++i) {
    RealVector factor_i = Teuchos::getCol(Teuchos::View,f_scores,i);
    gpApproximations[i].add(allSamples, factor_i );
    gpApproximations[i].build();
  }

  // Now form predictions based on new input points
   
  Real pca_coeff;
  RealVector this_pred(numFunctions);
  get_parameter_sets(iteratedModel); // NOTE:  allSamples is now different
   
  RealMatrix predMatrix;
  predMatrix.reshape(numSamples, numFunctions);

  RealVector column_means = pcaReducedBasis.get_column_means();

  for (int j = 0; j < numSamples; j++) {
    //this_pred = mean_vec;
    this_pred = 0.0;
    for (int i = 0; i < num_signif_Pcomps; ++i) {
      RealVector new_sample = Teuchos::getCol(Teuchos::View,allSamples,j);
      pca_coeff=gpApproximations[i].value(new_sample);
      if (outputLevel == DEBUG_OUTPUT)
	s << "pca_coeff " << pca_coeff << '\n';
      // need Row, not column
      RealVector local_pred(numFunctions);
      for( int k=0; k<numFunctions; ++k )
	local_pred(k) = pca_coeff*principal_comp(i,k);
      this_pred += local_pred;
    }
    for( int k=0; k<numFunctions; ++k )
      predMatrix(j,k) = this_pred(k)+column_means(k);
  }
  s << "\nPrediction Matrix\n";
  for (samp=0; samp<numSamples; ++samp) {
    for (fn=0; fn<numFunctions; ++fn)
      s << predMatrix(samp,fn) << "  " ;
    s << '\n';
  }
}


void NonDLHSSampling::print_results(std::ostream& s)
{
  if (!numResponseFunctions) // DACE mode w/ opt or NLS
    Analyzer::print_results(s);

  if (varBasedDecompFlag)
    print_sobol_indices(s);
  else if (statsFlag) {
    s << "\nStatistics based on " << numSamples << " samples:\n";
    print_statistics(s);
  }
}

} // namespace Dakota
