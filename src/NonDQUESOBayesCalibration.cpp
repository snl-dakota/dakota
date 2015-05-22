/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDQUESOBayesCalibration
//- Description: Derived class for Bayesian inference using QUESO
//- Owner:       Laura Swiler, Brian Adams
//- Checked by:
//- Version:

// place Dakota headers first to minimize influence of QUESO defines
#include "NonDQUESOBayesCalibration.hpp"
#include "NonDExpansion.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "DakotaModel.hpp"
#include "DakotaApproximation.hpp"
#include "ProbabilityTransformation.hpp"
// then list QUESO headers
#include "queso/StatisticalInverseProblem.h"
#include "queso/StatisticalInverseProblemOptions.h"
#include "queso/MetropolisHastingsSGOptions.h"
#include "queso/SequenceStatisticalOptions.h"
#include "queso/GslVector.h"
#include "queso/GslMatrix.h"
#include "queso/Environment.h"
#include "queso/EnvironmentOptions.h"
#include "queso/Defines.h"
#include "queso/ValidationCycle.h"
#include "queso/GenericScalarFunction.h"
#include "queso/UniformVectorRV.h"

// for generic log prior eval
#include "queso/JointPdf.h"
#include "queso/VectorRV.h"

static const char rcsId[]="@(#) $Id$";


namespace Dakota {

// declaring inherited classes here for now during prototyping to
// avoid including QUESO header in our header

/// Dakota specialization of QUESO generic joint PDF
template <class V, class M>
class QuesoJointPdf: public QUESO::BaseJointPdf<V, M>
{
public:
  //! Default constructor.
  /*! Instantiates an object of the class, i.e. a scalar function,
      given a prefix and its domain.*/
  QuesoJointPdf(const char*                  prefix,
		const QUESO::VectorSet<V,M>& domainSet,
		NonDQUESOBayesCalibration*   nond_queso_ptr);
  //! Destructor
  virtual ~QuesoJointPdf();

  //! Actual value of the PDF (scalar function).
  double actualValue(const V& domainVector, const V* domainDirection, 
		     V* gradVector, M* hessianMatrix, V* hessianEffect) const;
  
  //! Logarithm of the value of the function.
  double lnValue(const V& domainVector, const V* domainDirection, 
		 V* gradVector, M* hessianMatrix, V* hessianEffect) const;

  // NOTE: likely don't need for MCMC:
  //! Computes the logarithm of the normalization factor.
  double computeLogOfNormalizationFactor(unsigned int numSamples,
					 bool m_logOfNormalizationFactor) const;

private:
  using QUESO::BaseScalarFunction<V,M>::m_env;
  using QUESO::BaseScalarFunction<V,M>::m_prefix;
  // using QUESO::BaseScalarFunction<V,M>::m_domainSet;
  // using QUESO::BaseJointPdf<V,M>::m_normalizationStyle;
  // using QUESO::BaseJointPdf<V,M>::m_logOfNormalizationFactor;
  NonDQUESOBayesCalibration* NonDQUESOInstance;
};

// Constructor -------------------------------------
template<class V,class M>
QuesoJointPdf<V,M>::QuesoJointPdf(const char* prefix,
				  const QUESO::VectorSet<V,M>& domainSet,
				  NonDQUESOBayesCalibration*   nond_queso_ptr)
  : QUESO::BaseJointPdf<V,M>(((std::string)(prefix)+"generic").c_str(),
			     domainSet), NonDQUESOInstance(nond_queso_ptr)
{
  if ((m_env.subDisplayFile()) && (m_env.displayVerbosity() >= 54)) {
    *m_env.subDisplayFile() << "Entering QuesoJointPdf<V,M>::constructor()"
                            << ": prefix = " << m_prefix
                            << std::endl;
  }

  if ((m_env.subDisplayFile()) && (m_env.displayVerbosity() >= 54)) {
    *m_env.subDisplayFile() << "Leaving QuesoJointPdf<V,M>::constructor()"
                            << ": prefix = " << m_prefix
                            << std::endl;
  }
}

// Destructor --------------------------------------
template<class V,class M>
QuesoJointPdf<V,M>::~QuesoJointPdf()
{ }

template<class V,class M>
double QuesoJointPdf<V,M>::
actualValue(const V& domainVector, const V* domainDirection,
	    V* gradVector, M* hessianMatrix, V* hessianEffect) const
{ return NonDQUESOInstance->prior_density(domainVector); }

template<class V,class M>
double QuesoJointPdf<V,M>::
lnValue(const V& domainVector, const V* domainDirection, 
	V* gradVector, M* hessianMatrix, V* hessianEffect) const
{ return std::log(NonDQUESOInstance->prior_density(domainVector)); }

template<class V,class M>
double QuesoJointPdf<V,M>::
computeLogOfNormalizationFactor(unsigned int numSamples, 
				bool m_logOfNormalizationFactor) const
{ }

/// Dakota specialization of QUESO vector-valued random variable
template <class V, class M>
class QuesoVectorRV: public QUESO::BaseVectorRV<V,M>
{

public:
  //! Default constructor
  /*! Constructs a generic queso vector RV, given a prefix and the
      image set of the vector RV.*/
  QuesoVectorRV(const char*                  prefix,
		const QUESO::VectorSet<V,M>& imageSet,
		NonDQUESOBayesCalibration*   nond_queso_ptr);
  //! Virtual destructor
  virtual ~QuesoVectorRV();
 
  //! TODO: Prints the vector RV (required pure virtual).
  void print(std::ostream& os) const;

private:
  using QUESO::BaseVectorRV<V,M>::m_env;
  using QUESO::BaseVectorRV<V,M>::m_prefix;
  using QUESO::BaseVectorRV<V,M>::m_imageSet;
  using QUESO::BaseVectorRV<V,M>::m_pdf;
  // using QUESO::BaseVectorRV<V,M>::m_realizer;  // only needed to make draws
  // using QUESO::BaseVectorRV<V,M>::m_subCdf;
  // using QUESO::BaseVectorRV<V,M>::m_unifiedCdf;
  // using QUESO::BaseVectorRV<V,M>::m_mdf;
};


// Default constructor-------------------------------
template<class V, class M>
QuesoVectorRV<V,M>::QuesoVectorRV(const char* prefix,
				  const QUESO::VectorSet<V,M>& imageSet,
				  NonDQUESOBayesCalibration*   nond_queso_ptr)
  : QUESO::BaseVectorRV<V,M>(((std::string)(prefix)+"generic").c_str(),imageSet)
{
  if ((m_env.subDisplayFile()) && (m_env.displayVerbosity() >= 54)) {
    *m_env.subDisplayFile() << "Entering QuesoVectorRV<V,M>::constructor()"
                            << ": prefix = " << m_prefix
                            << std::endl;
  }

  m_pdf = new QuesoJointPdf<V,M>(m_prefix.c_str(),
				 m_imageSet, nond_queso_ptr);
  // m_realizer   = NULL; // FIX ME: complete code
  // m_subCdf     = NULL; // FIX ME: complete code
  // m_unifiedCdf = NULL; // FIX ME: complete code
  // m_mdf        = NULL; // FIX ME: complete code

  if ((m_env.subDisplayFile()) && (m_env.displayVerbosity() >= 54)) {
    *m_env.subDisplayFile() << "Leaving QuesoVectorRV<V,M>::constructor()"
                            << ": prefix = " << m_prefix
                            << std::endl;
  }
}

// Destructor ---------------------------------------
template<class V, class M>
QuesoVectorRV<V,M>::~QuesoVectorRV()
{
  // delete m_mdf;
  // delete m_unifiedCdf;
  // delete m_subCdf;
  // delete m_realizer;
  delete m_pdf;
}

// I/O methods --------------------------------------
template <class V, class M>
void
QuesoVectorRV<V,M>::print(std::ostream& os) const
{
  os << "QuesoVectorRV<V,M>::print() says, 'Please implement me.'" << std::endl;
  return;
}


// initialization of statics
NonDQUESOBayesCalibration* NonDQUESOBayesCalibration::NonDQUESOInstance(NULL);

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDQUESOBayesCalibration::
NonDQUESOBayesCalibration(ProblemDescDB& problem_db, Model& model):
  NonDBayesCalibration(problem_db, model),
  mcmcType(probDescDB.get_string("method.nond.mcmc_type")),
  // these two deprecated:
  likelihoodScale(probDescDB.get_real("method.likelihood_scale")),
  calibrateSigma(probDescDB.get_bool("method.nond.calibrate_sigma")),
  precondRequestValue(0),
  logitTransform(probDescDB.get_bool("method.nond.logit_transform"))
{
  ////////////////////////////////////////////////////////
  // Step 1 of 5: Instantiate the QUESO environment 
  ////////////////////////////////////////////////////////
  init_queso_environment();
 
  // Read in all of the experimental data:  any x configuration 
  // variables, y observations, and y_std if available 
  //if (outputLevel > NORMAL_OUTPUT)
  //  Cout << "Read data from file " << calibrationData << '\n';
  if (calibrationData)
    expData.load_data("QUESO Bayes Calibration", calibrateSigma);
  else
    Cout << "No experiment data from files\n"
	 << "QUESO is assuming the simulation is returning the residuals\n";
  if (calibrateSigma && !calibrationData) {
    Cerr << "\nError: you are attempting to calibrate the measurement error " 
         << "but have not provided experimental data information."<<std::endl;
     //calibration of sigma temporarily unsupported."<<std::endl;
    abort_handler(METHOD_ERROR);
  }
}


NonDQUESOBayesCalibration::~NonDQUESOBayesCalibration()
{ }


/** Perform the uncertainty quantification */
void NonDQUESOBayesCalibration::quantify_uncertainty()
{
  // instantiate QUESO objects and execute
  NonDQUESOInstance = this;

  // construct mcmcModel and initialize transformations, as needed
  initialize_model();

  // initialize the ASV request value for preconditioning and
  // construct a Response for use in residual computation
  if (proposalCovarType == "derivatives")
    init_precond_request_value();
  init_residual_response();

  ////////////////////////////////////////////////////////
  // Step 2 of 5: Instantiate the parameter domain
  ////////////////////////////////////////////////////////
  init_parameter_domain();

  // Size our Queso covariance matrix and initialize trailing diagonal if
  // calibrating sigma terms
  proposalCovMatrix.reset(new QUESO::GslMatrix(paramSpace->zeroVector()));
  if (calibrateSigma) {
    Real uniform_variance_factor = 1.99 * 1.99 / 12.; // uniform on [.01,2.]
    for (int i=0; i<numFunctions; i++) {
      // assuming the sigma terms are uniformly distributed
      Real var_i = uniform_variance_factor * expData.scalar_sigma_est(i);
      if (outputLevel > NORMAL_OUTPUT )  
        Cout << "Diagonal estimate for sigma_i " << var_i << '\n';
      (*proposalCovMatrix)(i+numContinuousVars,i+numContinuousVars) = var_i;
    }
  }
  // initialize proposal covariance (must follow parameter domain init)
  // This is the leading sub-matrix in the case of calibrating sigma terms
  if (proposalCovarType == "user") // either filename OR data values defined
    user_proposal_covariance(proposalCovarInputType, proposalCovarData,
			     proposalCovarFilename);
  else if (proposalCovarType == "prior")
    prior_proposal_covariance(); // prior selection or default for no emulator

  if (proposalCovarType == "user" || proposalCovarType == "prior") {
    QUESO::GslMatrix test_mat = proposalCovMatrix->transpose();
    test_mat -= *proposalCovMatrix;
    if( test_mat.normMax() > 1.e-14 )
      throw std::runtime_error("Queso covariance matrix is not symmetric.");

    // validate that provided data is a valid covariance matrix -
    // test PD part of SPD
    test_mat = *proposalCovMatrix;
    int ierr = test_mat.chol();
    if( ierr == QUESO::UQ_MATRIX_IS_NOT_POS_DEFINITE_RC)
      throw std::runtime_error("Queso covariance data is not SPD.");
  }
  if (outputLevel > NORMAL_OUTPUT ) { 
    Cout << "Proposal Covariance " << '\n';
    proposalCovMatrix->print(Cout);
    Cout << std::endl;
  }
  // init likelihoodFunctionObj, prior/posterior random vectors, inverse problem
  init_queso_solver();

  switch (adaptPosteriorRefine) {
  case false:
    run_chain_with_restarting();
    break;
  case true:
    if (!emulatorType) { // current spec prevents this
      Cerr << "Error: adaptive posterior refinement requires emulator model."
	   << std::endl;
      abort_handler(METHOD_ERROR);
    }
    compactMode = true; // update_model() uses all{Samples,Responses}
    Real adapt_metric = DBL_MAX; unsigned short int num_iter = 0;
    while (adapt_metric > convergenceTol && num_iter < maxIterations) {

      // TO DO: treat this like cross-validation as there is likely a sweet
      // spot prior to degradation of conditioning (too much refinement data)

      // place update block here so that chain is always run for initial or
      // updated emulator; placing block at loop end could result in emulator
      // convergence w/o final chain.
      if (num_iter) {
	// update the emulator surrogate data with new truth evals and
	// reconstruct surrogate (e.g., via PCE sparse recovery)
	update_model();
	// assess posterior convergence via convergence of the emulator coeffs
	adapt_metric = assess_emulator_convergence();
      }

      // execute MCMC chain, optionally in batches
      run_chain_with_restarting();
      ++num_iter;

      // assess convergence of the posterior via sample-based K-L divergence:
      //adapt_metric = assess_posterior_convergence();
    }
    break;
  }
}


void NonDQUESOBayesCalibration::run_chain_with_restarting()
{
  if (outputLevel >= NORMAL_OUTPUT) {
    if (chainCycles > 1)
      Cout << "Running chain in batches of " << numSamples << " with "
	   << chainCycles << " restarts." << std::endl;
    else
      Cout << "Running chain with " << numSamples << " samples." << std::endl;
  }

  // clear for each (composite) chain (best samples for current emulator)
  bestSamples.clear();

  //Real restart_metric = DBL_MAX;
  size_t update_cntr = 0;
  unsigned short batch_size = (adaptPosteriorRefine) ? 5 : 1;
  //copy_data(mcmcModel.continuous_variables(), prevCenter);
  update_chain_size(numSamples);

  // update proposal covariance and recenter after short chain: a
  // workaround for inability to update proposal covariance on the fly
  while (//restart_metric > convergenceTol &&
	 update_cntr < chainCycles) {

    // define proposal covariance from misfit Hessian
    if (proposalCovarType == "derivatives")
      precondition_proposal();
    ++update_cntr;

    run_queso_solver(); // solve inverse problem with MCMC 

    filter_chain(update_cntr, batch_size);

    // account for redundancy between final and initial
    if (update_cntr == 1)
      update_chain_size(numSamples+1);

    // This approach is too greedy and can get stuck (i.e., no point in new
    // chain has smaller mismatch than current optimal value):
    //restart_metric = update_center(allSamples[last_index]);
    //
    // Rather, use final point in acceptance chain as if we were periodically
    // refreshing the proposal covariance within a single integrated chain.
    if (update_cntr < chainCycles || adaptPosteriorRefine)
      update_center();

    if (outputLevel >= NORMAL_OUTPUT)
      Cout << std::endl;
  }
}


void NonDQUESOBayesCalibration::init_queso_environment()
{
  // NOTE:  for now we are assuming that DAKOTA will be run with 
  // mpiexec to call MPI_Init.  Eventually we need to generalize this 
  // and send QUESO the proper MPI subenvironments.

  // TODO: see if this can be a local, or if the env retains a pointer
  envOptionsValues.reset(new QUESO::EnvOptionsValues());
  envOptionsValues->m_subDisplayFileName = "outputData/display";
  envOptionsValues->m_subDisplayAllowedSet.insert(0);
  envOptionsValues->m_subDisplayAllowedSet.insert(1);
  envOptionsValues->m_displayVerbosity = 2;
  envOptionsValues->m_seed = (randomSeed) ? randomSeed : 1 + (int)clock(); 
 
  if (mcmcType == "multilevel")
    quesoEnv.reset(new QUESO::FullEnvironment(MPI_COMM_SELF,"ml.inp","",NULL));
  else // dram, dr, am, or mh
    quesoEnv.reset(new QUESO::FullEnvironment(MPI_COMM_SELF,"","",
					      envOptionsValues.get()));
}


void NonDQUESOBayesCalibration::init_precond_request_value()
{
  // Gauss-Newton approximate Hessian requires gradients of residuals (rv=2)
  // full Hessian requires values/gradients/Hessians of residuals (rv=7)
  precondRequestValue = 0;
  switch (emulatorType) {
  case PCE_EMULATOR: case KRIGING_EMULATOR:
    precondRequestValue = 7; break;
  case  SC_EMULATOR: case      GP_EMULATOR: // no Hessian support yet
    precondRequestValue = 2; break;
  case  NO_EMULATOR:
    // Note: mixed gradients/Hessians are still globally "on", regardless of
    // mixed computation approach
    if (mcmcModel.gradient_type() != "none")
      precondRequestValue |= 2; // gradients
    if (mcmcModel.hessian_type()  != "none")
      precondRequestValue |= 5; // values & Hessians
    break;
  }
}


void NonDQUESOBayesCalibration::init_residual_response()
{
  // The residual response is sized based on total experiment data
  // size.  It has to allocate space for derivatives in the case of
  // preconditioned proposal covariance.
  const Response& resp = mcmcModel.current_response();
  residualResponse = resp.copy(false);  // false: SRD can be shared until resize
  size_t total_residuals = 
    calibrationData ? expData.num_total_exppoints() : resp.num_functions();
  // likelihood needs fn_vals; preconditioning may need derivs
  short request_value_needed = 1 | precondRequestValue;

  // reshape needed if change in size or derivative request; Response
  // only resizes contained data if needed
  residualResponse.reshape(total_residuals,
			   resp.active_set_derivative_vector().size(),
			   request_value_needed & 2, request_value_needed & 4);
  // TODO: fully map the active set vector as in Minimizer, or replace
  // with a RecastModel for data.
}


void NonDQUESOBayesCalibration::init_queso_solver()
{
  ////////////////////////////////////////////////////////
  // Step 3 of 5: Instantiate the likelihood function object
  ////////////////////////////////////////////////////////
  // routine computes [ln(function)]
  likelihoodFunctionObj.reset(new
    QUESO::GenericScalarFunction<QUESO::GslVector,QUESO::GslMatrix>("like_",
    *paramDomain, &dakotaLikelihoodRoutine, (void *) NULL, true));

  ////////////////////////////////////////////////////////
  // Step 4 of 5: Instantiate the inverse problem
  ////////////////////////////////////////////////////////
  // initial approach was restricted to uniform priors
  //priorRv.reset(new QUESO::UniformVectorRV<QUESO::GslVector,QUESO::GslMatrix> 
  //		  ("prior_", *paramDomain));
  // new approach supports arbitrary priors:
  priorRv.reset(new QuesoVectorRV<QUESO::GslVector,QUESO::GslMatrix> 
   		("prior_", *paramDomain, NonDQUESOInstance));

  postRv.reset(new QUESO::GenericVectorRV<QUESO::GslVector,QUESO::GslMatrix>
	       ("post_", *paramSpace));
  // Q: are Prior/posterior RVs copied by StatInvProblem, such that these
  //    instances can be local and allowed to go out of scope?

  // define calIpOptionsValues
  set_ip_options();
  // set options specific to MH algorithm
  set_mh_options();
  // Inverse problem: instantiate it (posterior rv is instantiated internally)
  inverseProb.reset(new
    QUESO::StatisticalInverseProblem<QUESO::GslVector,QUESO::GslMatrix>("",
    calIpOptionsValues.get(), *priorRv, *likelihoodFunctionObj, *postRv));
}


void NonDQUESOBayesCalibration::precondition_proposal()
{
  if (!precondRequestValue) {
    Cerr << "Error: response derivative specification required for proposal "
	 << "preconditioning." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // update mcmcModel's continuous variables from paramInitials
  for (size_t i=0; i<numContinuousVars; ++i)
    mcmcModel.continuous_variable((*paramInitials)[i], i);
  // update request vector values
  ActiveSet set = mcmcModel.current_response().active_set(); // copy
  set.request_values(precondRequestValue);
  // compute response (echoed to Cout if outputLevel > NORMAL)
  mcmcModel.compute_response(set);

  // compute Hessian of log-likelihood misfit r^T r (where is Gamma inverse?)
  RealSymMatrix log_like_hess;
  const Response& emulator_resp = mcmcModel.current_response();
  RealMatrix prop_covar;
  if (precondRequestValue & 4) {
    // try to use full misfit Hessian; fall back if indefinite
    expData.build_hessian_of_sum_square_residuals(emulator_resp, log_like_hess);
    bool ev_truncation =
      get_positive_definite_covariance_from_hessian(log_like_hess, prop_covar);
    if (ev_truncation) { // fallback to Gauss-Newton
      ShortArray asrv_override(numFunctions, 2); // override asrv in response
      expData.build_hessian_of_sum_square_residuals(emulator_resp,
						    asrv_override,
						    log_like_hess);
      get_positive_definite_covariance_from_hessian(log_like_hess, prop_covar);
    }
  }
  else {
    expData.build_hessian_of_sum_square_residuals(emulator_resp, log_like_hess);
    get_positive_definite_covariance_from_hessian(log_like_hess, prop_covar);
  }

  if (outputLevel >= NORMAL_OUTPUT) {
    Cout << "Hessian of misfit (negative log-likelihood):\n";
    write_data(Cout, log_like_hess, true, true, true);
    //Cout << "2x2 determinant = " << log_like_hess(0,0)*log_like_hess(1,1) -
    //  log_like_hess(0,1)*log_like_hess(1,0) << '\n';

    Cout << "Positive definite covariance from inverse of misfit Hessian:\n";
    write_data(Cout, prop_covar, true, true, true);
    //Cout << "2x2 determinant = " << prop_covar(0,0)*prop_covar(1,1) -
    //  prop_covar(0,1)*prop_covar(1,0) << '\n';
  }

  // pack GSL proposalCovMatrix
  int i, j, nv = log_like_hess.numRows();
  if (!proposalCovMatrix) {
    proposalCovMatrix.reset(new QUESO::GslMatrix(paramSpace->zeroVector()));
    if ((paramSpace->dimGlobal() != nv) || 
        (paramSpace->dimGlobal() !=(nv+numFunctions))) {
      Cerr << "Error: Queso vector space is not consistent with proposal "
	   << "covariance dimension." << std::endl;
      abort_handler(METHOD_ERROR);
    }
  }
  for (i=0; i<nv; ++i )
    for (j=0; j<nv; ++j )
      (*proposalCovMatrix)(i,j) = prop_covar(i,j);
}


void NonDQUESOBayesCalibration::run_queso_solver()
{
  Cout << "Running Bayesian Calibration with QUESO " << mcmcType << " using "
       << calIpMhOptionsValues->m_rawChainSize << " MCMC samples." << std::endl;
  //if (outputLevel > NORMAL_OUTPUT)
  //  Cout << "\n  Calibrate Sigma " << calibrateSigma << std::endl;

  ////////////////////////////////////////////////////////
  // Step 5 of 5: Solve the inverse problem
  ////////////////////////////////////////////////////////
  if (mcmcType == "multilevel")
    inverseProb->solveWithBayesMLSampling();
  else
    inverseProb->solveWithBayesMetropolisHastings(calIpMhOptionsValues.get(),
						  *paramInitials, 
						  proposalCovMatrix.get());

  Cout << "QUESO MCMC chain completed.  QUESO results are in the outputData "
       << "directory:\n  display_sub0.txt contains MCMC diagnostics.\n"
       << "  Matlab files contain the chain values.\n"; //  The files to " 
  //   << "load in Matlab are\nfile_cal_ip_raw.m (the actual chain) " 
  //   << "or file_cal_ip_filt.m (the filtered chain,\nwhich contains " 
  //   << "every 20th step in the chain.\n"
  //   << "NOTE:  the chain values in these Matlab files are currently " 
  //   << "in scaled space. \n  You will have to transform them back to "
  //   << "original space by:\n"
  //   << "lower_bounds + chain_values * (upper_bounds - lower_bounds)\n"
  //   << "The rejection rate is in the tgaCalOutput file.\n"
  //   << "We hope to improve the postprocessing of the chains by the " 
  //   << "next Dakota release.\n";
}


void NonDQUESOBayesCalibration::
filter_chain(size_t update_cntr, unsigned short batch_size)
{
  // filter chain -or- extract full chain and sort on likelihood values
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "Filtering chain: extracting best " << batch_size
	 << " from MCMC chain " << update_cntr << " containing "
	 << inverseProb->chain().subSequenceSize() << " samples.\n";

  std::/*multi*/map<Real, size_t> local_best;
  chain_to_local(batch_size, local_best);
  if (adaptPosteriorRefine) { // extract best MCMC samples from current batch
    if (chainCycles > 1) {
      local_to_aggregated(batch_size, local_best);
      if (update_cntr == chainCycles)
	aggregated_to_all();
    }
    else
      local_to_all(local_best);
  }
  else // track MAP for final results
    local_to_aggregated(batch_size, local_best);
}


void NonDQUESOBayesCalibration::
chain_to_local(unsigned short batch_size, std::map<Real, size_t>& local_best)
{
  // TODO: Need to transform chain back to unscaled space for reporting
  // to user; possibly also in auxilliary data files for user consumption.

  // to get the full acceptance chain, m_filteredChainGenerate is set to
  // false in set_invpb_mh_options()

  // Note: the QUESO VectorSequence class has a number of helpful filtering
  // and statistics functions.

  const QUESO::BaseVectorSequence<QUESO::GslVector,QUESO::GslMatrix>& mcmc_chain
    = inverseProb->chain();
  const QUESO::ScalarSequence<double>& loglike_vals
    = inverseProb->logLikelihoodValues();
  unsigned int num_mcmc = mcmc_chain.subSequenceSize();
  if (num_mcmc != loglike_vals.subSequenceSize()) {
    Cerr << "Error (NonDQUESO): final mcmc chain has length " << num_mcmc 
	 << "\n                 but likelihood set has length"
	 << loglike_vals.subSequenceSize() << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // TO DO: want to keep different samples with same likelihood, but not 
  // replicate samples with same likelihood (from rejection); for now, use
  // a std::map since the latter is unlikely.
  QUESO::GslVector mcmc_sample(paramSpace->zeroVector());
  for (size_t chain_pos = 0; chain_pos < num_mcmc; ++chain_pos) {
    // extract GSL sample vector from QUESO vector sequence:
    mcmc_chain.getPositionValues(chain_pos, mcmc_sample);
    // evaluate log of posterior from log likelihood and log prior:
    Real log_prior     = std::log(prior_density(mcmc_sample)),
         log_posterior = loglike_vals[chain_pos] + log_prior;
    //std::log(mcmcModel.continuous_probability_density(mcmc_sample));
    if (outputLevel > NORMAL_OUTPUT)
      Cout << "MCMC sample: " << mcmc_sample << " log prior = " << log_prior
	   << " log posterior = " << log_posterior << std::endl;
    // sort ascending by log posterior (highest prob are last) and retain
    // batch_size samples
    local_best.insert(std::pair<Real, size_t>(log_posterior, chain_pos));
    if (local_best.size() > batch_size)
      local_best.erase(local_best.begin()); // pop front (lowest prob)
  }
  if (outputLevel > NORMAL_OUTPUT)
    Cout << "local_best map:\n" << local_best << std::endl;
}


void NonDQUESOBayesCalibration::
local_to_aggregated(unsigned short batch_size,
		    const std::map<Real, size_t>& local_best)
{
  // Merge local std::map<Real, size_t> into aggregate/class-scope
  // std::map<Real, QUESO::GslVector> 
  const QUESO::BaseVectorSequence<QUESO::GslVector,QUESO::GslMatrix>&
    mcmc_chain = inverseProb->chain();
  std::/*multi*/map<Real, size_t>::const_iterator cit;
  QUESO::GslVector mcmc_sample(paramSpace->zeroVector());
  for (cit=local_best.begin(); cit!=local_best.end(); ++cit) {
    mcmc_chain.getPositionValues(cit->second, mcmc_sample);
    bestSamples.insert(
      std::pair<Real, QUESO::GslVector>(cit->first, mcmc_sample));
    // continuously maintain length to reduce sorting cost
    if (bestSamples.size() > batch_size)
      bestSamples.erase(bestSamples.begin()); // pop front
  }
  if (outputLevel > NORMAL_OUTPUT)
    Cout << "bestSamples map:\n" << bestSamples << std::endl;
}


void NonDQUESOBayesCalibration::aggregated_to_all()
{
  // copy bestSamples into allSamples
  size_t num_best = bestSamples.size();
  if (allSamples.numCols() != num_best)
    allSamples.shapeUninitialized(numContinuousVars, num_best);
  std::/*multi*/map<Real, QUESO::GslVector>::iterator it; size_t i;
  if (outputLevel >= NORMAL_OUTPUT) Cout << "Chain filtering results:\n";
  for (it=bestSamples.begin(), i=0; it!=bestSamples.end(); ++it, ++i) {
    copy_gsl(it->second, allSamples, i);
    if (outputLevel >= NORMAL_OUTPUT) {
      Cout << "Best point " << i+1 << ": Log posterior = " << it->first
	   << " Sample:";
      write_col_vector_trans(Cout, (int)i, false, false, true, allSamples);
    }
  }
}


void NonDQUESOBayesCalibration::
local_to_all(const std::map<Real, size_t>& local_best)
{
  // copy local_best into allSamples
  size_t num_best = local_best.size();
  if (allSamples.numCols() != num_best)
    allSamples.shapeUninitialized(numContinuousVars, num_best);
  const QUESO::BaseVectorSequence<QUESO::GslVector,QUESO::GslMatrix>&
    mcmc_chain = inverseProb->chain();
  QUESO::GslVector mcmc_sample(paramSpace->zeroVector());
  std::/*multi*/map<Real, size_t>::const_iterator cit; size_t i;
  if (outputLevel >= NORMAL_OUTPUT) Cout << "Chain filtering results:\n";
  for (cit=local_best.begin(), i=0; cit!=local_best.end(); ++cit, ++i) {
    mcmc_chain.getPositionValues(cit->second, mcmc_sample);
    copy_gsl(mcmc_sample, allSamples, i);
    if (outputLevel >= NORMAL_OUTPUT) {
      Cout << "Best point " << i+1 << ": Log posterior = " << cit->first
	   << " Sample:";
      write_col_vector_trans(Cout, (int)i, false, false, true, allSamples);
    }
  }
}


void NonDQUESOBayesCalibration::update_center()
{
  const QUESO::BaseVectorSequence<QUESO::GslVector,QUESO::GslMatrix>& 
    mcmc_chain = inverseProb->chain();
  unsigned int num_mcmc = mcmc_chain.subSequenceSize();

  // extract GSL sample vector from QUESO vector sequence:
  // Note can't use the most recent mcmcModel evaluation since this could
  // correspond to a rejected point.
  size_t last_index = num_mcmc - 1;
  mcmc_chain.getPositionValues(last_index, *paramInitials);
  if (outputLevel > NORMAL_OUTPUT)
    Cout << "New center:\n" << *paramInitials << "Log likelihood = "
	 << inverseProb->logLikelihoodValues()[last_index] << std::endl;
}


/*
Real NonDQUESOBayesCalibration::update_center(const RealVector& new_center)
{
  // update QUESO initial vars for starting point of chain
  for (int i=0; i<numContinuousVars; i++)
    (*paramInitials)[i] = new_center[i];

  // evaluate and return L2 norm of change in chain starting point
  RealVector delta_center = new_center;
  delta_center -= prevCenter;
  prevCenter = new_center;
  return delta_center.normFrobenius();
}
*/


void NonDQUESOBayesCalibration::update_model()
{
  if (!emulatorType) {
    Cerr << "Error: NonDQUESOBayesCalibration::update_model() requires an "
	 << "emulator model." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // perform truth evals (in parallel) for selected points
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "Updating emulator: evaluating " << allSamples.numCols()
	 << " best points." << std::endl;
  // bypass surrogate but preserve transformations to standardized space
  short orig_resp_mode = mcmcModel.surrogate_response_mode(); // store mode
  mcmcModel.surrogate_response_mode(BYPASS_SURROGATE); // actual model evals
  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR:
    nondInstance = (NonD*)stochExpIterator.iterator_rep();
    evaluate_parameter_sets(mcmcModel, true, false); // log allResp, no best
    nondInstance = this; // restore
    break;
  case GP_EMULATOR: case KRIGING_EMULATOR:
    if (standardizedSpace)
      nondInstance = (NonD*)mcmcModel.subordinate_iterator().iterator_rep();
    evaluate_parameter_sets(mcmcModel, true, false); // log allResp, no best
    if (standardizedSpace)
      nondInstance = this; // restore
    break;
  }
  mcmcModel.surrogate_response_mode(orig_resp_mode); // restore mode

  // update mcmcModel with new data from iteratedModel
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "Updating emulator: appending " << allResponses.size()
	 << " new data sets." << std::endl;
  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR: {
    // Adapt the expansion in sync with the dataset using a top-down design
    // (more explicit than embedded logic w/i mcmcModel.append_approximation).
    NonDExpansion* se_iterator
      = (NonDExpansion*)stochExpIterator.iterator_rep();
    se_iterator->append(allSamples, allResponses);
    // TO DO: order increment places addtnl reqmts on emulator conv assessment
    break;
  }
  case GP_EMULATOR: case KRIGING_EMULATOR:
    mcmcModel.append_approximation(allSamples, allResponses, true); // rebuild
    break;
  }
}


Real NonDQUESOBayesCalibration::assess_emulator_convergence()
{
  // coeff reference point not yet available; force another iteration rather
  // than use norm of current coeffs (stopping on small norm is not meaningful)
  if (prevCoeffs.empty()) {
    switch (emulatorType) {
    case PCE_EMULATOR:
      prevCoeffs = mcmcModel.approximation_coefficients(true);  break;
    case SC_EMULATOR:
      prevCoeffs = mcmcModel.approximation_coefficients(false); break;
    case GP_EMULATOR: case KRIGING_EMULATOR:
      Cerr << "Warning: convergence norm not yet defined for GP emulators in "
	   << "NonDQUESOBayesCalibration::assess_emulator_convergence()."
	   << std::endl;
      break;
    }
    return DBL_MAX;
  }

  Real l2_norm_delta_coeffs = 0., delta_coeff_ij;
  switch (emulatorType) {
  case PCE_EMULATOR: {
    // normalized coeffs:
    const RealVectorArray& coeffs = mcmcModel.approximation_coefficients(true);
    size_t i, j, num_qoi = coeffs.size(), num_coeffs_i;

    // This approach requires an unchanged multiIndex, which is acceptable 
    // for regression PCE using a fixed (candidate) expansion definition.
    // Sparsity is not a concern as returned coeffs are inflated to be
    // dense with respect to SharedOrthogPolyApproxData::multiIndex.
    for (i=0; i<num_qoi; ++i) {
      const RealVector&      coeffs_i =     coeffs[i];
      const RealVector& prev_coeffs_i = prevCoeffs[i];
      num_coeffs_i = coeffs_i.length();
      for (j=0; j<num_coeffs_i; ++j) {
	delta_coeff_ij = coeffs_i[j] - prev_coeffs_i[j];
	l2_norm_delta_coeffs += delta_coeff_ij * delta_coeff_ij;
      }
    }

    prevCoeffs = coeffs;
    break;
  }
  case SC_EMULATOR: {
    // Interpolation could use a similar concept with the expansion coeffs,
    // although adaptation would imply differences in the grid.
    const RealVectorArray& coeffs = mcmcModel.approximation_coefficients(false);

    Cerr << "Warning: convergence norm not yet defined for SC emulator in "
	 << "NonDQUESOBayesCalibration::assess_emulator_convergence()."
	 << std::endl;
    //abort_handler(METHOD_ERROR);
    return DBL_MAX;
    break;
  }
  case GP_EMULATOR: case KRIGING_EMULATOR:
    // Consider use of correlation lengths.
    // TO DO: define SurfpackApproximation::approximation_coefficients()...
    Cerr << "Warning: convergence norm not yet defined for GP emulators in "
	 << "NonDQUESOBayesCalibration::assess_emulator_convergence()."
	 << std::endl;
    //abort_handler(METHOD_ERROR);
    return DBL_MAX;
    break;
  }

  if (outputLevel >= NORMAL_OUTPUT) {
    Real norm = std::sqrt(l2_norm_delta_coeffs);
    Cout << "Assessing emulator convergence: l2 norm = " << norm << std::endl;
    return norm;
  }
  else
    return std::sqrt(l2_norm_delta_coeffs);
}


void NonDQUESOBayesCalibration::init_parameter_domain()
{
  int total_num_params = (calibrateSigma) ?
    numContinuousVars + numFunctions : numContinuousVars; 
  
  paramSpace.reset(new QUESO::VectorSpace<QUESO::GslVector,QUESO::GslMatrix>
		   (*quesoEnv, "param_", total_num_params, NULL));

  QUESO::GslVector paramMins(paramSpace->zeroVector()),
                   paramMaxs(paramSpace->zeroVector());
  for (size_t i=0; i<numContinuousVars; i++) {
    RealRealPair bnds = mcmcModel.continuous_distribution_bounds(i);
    paramMins[i] = bnds.first; paramMaxs[i] = bnds.second;
  }
  // if calibrateSigma, the parameter domain is expanded by sigma terms 
  if (calibrateSigma) {
    //RealVector covariance_diagonal;
    // Todo: pass in corrrect experiment number as second argument
    //expData.get_main_diagonal( covariance_diagonal, 0 );
    for (int j=0; j<numFunctions; j++){
      // TODO: Need to be able to get sigma from new covariance class.
      // Also need to sync up this with the logic in the ctor.  Also need
      // a default if there's no experimental data (may not be sensible)
      //paramMins[numContinuousVars+j] = 1.;
      //paramMaxs[numContinuousVars+j] = 1.;
      //Real std_j = std::sqrt( covariance_diagonal[j] );
      Real std_j = std::sqrt( expData.scalar_sigma_est(j));
      Cout << " scalar_sigma_est " << std_j;
      paramMins[numContinuousVars+j] = .01 * std_j;
      paramMaxs[numContinuousVars+j] =  2. * std_j;
    }
  }
  paramDomain.reset(new QUESO::BoxSubset<QUESO::GslVector,QUESO::GslMatrix>
		    ("param_", *paramSpace, paramMins, paramMaxs));

  paramInitials.reset(new QUESO::GslVector(paramSpace->zeroVector()));
  const RealVector& init_pt = mcmcModel.continuous_variables();
  if (standardizedSpace) {
    switch (emulatorType) {
    case PCE_EMULATOR: case SC_EMULATOR:
      // init_pt already propagated through transform
      for (size_t i=0; i<numContinuousVars; i++)
	(*paramInitials)[i] = init_pt[i];
      break;
    default: {
      // init_pt not already propagated through transform
      RealVector u_pt;
      if (emulatorType == NO_EMULATOR) // use local nataf
	natafTransform.trans_X_to_U(init_pt, u_pt);
      else { // use nataf in DataFitSurrModel::daceIterator
	NonD* nond_iterator
	  = (NonD*)mcmcModel.subordinate_iterator().iterator_rep();
	nond_iterator->variable_transformation().trans_X_to_U(init_pt, u_pt);
      }
      for (size_t i=0; i<numContinuousVars; i++)
	(*paramInitials)[i] = u_pt[i];
      break;
    }
    }
  }
  else
    for (size_t i=0; i<numContinuousVars; i++)
      (*paramInitials)[i] = init_pt[i];
  if (calibrateSigma)
    for (size_t i=numContinuousVars; i<total_num_params; i++)
      (*paramInitials)[i] = (paramMaxs[i] + paramMins[i]) / 2.;

  if (outputLevel > NORMAL_OUTPUT)
    Cout << "Initial Parameter values sent to QUESO (may be in scaled)\n"
	 << *paramInitials << "\nParameter bounds sent to QUESO (may be scaled)"
	 << ":\nparamMins " << paramMins << "\nparamMaxs " << paramMaxs << '\n';
}


/** Must be called after paramMins/paramMaxs set above */
void NonDQUESOBayesCalibration::prior_proposal_covariance()
{
  int total_num_params = numContinuousVars;
  // int total_num_params = paramSpace->dimGlobal();
  // QUESO::GslVector covDiag(paramSpace->zeroVector());

  // diagonal covariance from variance of prior marginals
  Real stdev;
  for (int i=0; i<total_num_params; i++) {
    stdev = mcmcModel.continuous_distribution_moment(i, 2);
    (*proposalCovMatrix)(i,i) = stdev * stdev;
  }
  //proposalCovMatrix.reset(new QUESO::GslMatrix(covDiag));

  if (outputLevel > NORMAL_OUTPUT) {
    //Cout << "Diagonal elements of the proposal covariance sent to QUESO";
    //if (standardizedSpace) Cout << " (scaled space)";
    //Cout << '\n' << covDiag << '\n';
    Cout << "QUESO ProposalCovMatrix"; 
    if (standardizedSpace) Cout << " (scaled space)";
    Cout << '\n'; 
    for (size_t i=0; i<total_num_params; i++) {
      for (size_t j=0; j<total_num_params; j++) 
	Cout <<  (*proposalCovMatrix)(i,j) << "  "; 
      Cout << '\n'; 
    }
  }
}


/** This function will convert user-specified cov_type = "diagonal" |
    "matrix" data from either cov_data or cov_filename and populate a
    full QUESO::GslMatrix* in proposalCovMatrix with the covariance. */
void NonDQUESOBayesCalibration::
user_proposal_covariance(const String& input_fmt, const RealVector& cov_data, 
			 const String& cov_filename)
{
  // TODO: transform user covariance for use in standardized probability space
  if (standardizedSpace)
    throw std::runtime_error("user-defined proposal covariance is invalid for use in transformed probability spaces.");
  // Note: if instead a warning, then fallback to prior_proposal_covariance()

  bool use_file = !cov_filename.empty();

  // Sanity check
  if( ("diagonal" != input_fmt) &&
      ("matrix"   != input_fmt) )
    throw std::runtime_error("User-specified covariance must have type of either \"diagonal\" of \"matrix\".  You have \""+input_fmt+"\".");

  // Sanity check
  if( cov_data.length() && use_file )
    throw std::runtime_error("You cannot provide both covariance values and a covariance data filename.");

  // Size our Queso covariance matrix
  //proposalCovMatrix.reset(new QUESO::GslMatrix(paramSpace->zeroVector()));

  // Sanity check
  int total_num_params = numContinuousVars; //paramSpace->dimGlobal();
  /*if( (proposalCovMatrix->numRowsLocal()  != total_num_params) || 
      (proposalCovMatrix->numRowsGlobal() != total_num_params) || 
      (proposalCovMatrix->numCols()       != total_num_params)   )
    throw std::runtime_error("Queso vector space is not consistent with parameter dimension.");
  */      
  // Read in a general way and then check that the data is consistent
  RealVectorArray values_from_file;
  if( use_file )
  {
    std::ifstream s;
    TabularIO::open_file(s, cov_filename, "read_queso_covariance_data");
    bool row_major = false;
    read_unsized_data(s, values_from_file, row_major);
  }

  if( "diagonal" == input_fmt )
  {
    if( use_file ) {
      // Allow either row or column data layout
      bool row_data = false;
      // Sanity checks
      if( values_from_file.size() != 1 ) {
        if( values_from_file.size() == total_num_params ) 
          row_data = true;
        else
          throw std::runtime_error("\"diagonal\" Queso covariance file data should have either 1 column (or row) and "
              +convert_to_string(total_num_params)+" rows (or columns).");
      }
      if( row_data ) {
        for( int i=0; i<total_num_params; ++i )
          (*proposalCovMatrix)(i,i) = values_from_file[i](0);
      }
      else {
        if( values_from_file[0].length() != total_num_params )
          throw std::runtime_error("\"diagonal\" Queso covariance file data should have "
              +convert_to_string(total_num_params)+" rows.  Found "
              +convert_to_string(values_from_file[0].length())+" rows.");
        for( int i=0; i<total_num_params; ++i )
          (*proposalCovMatrix)(i,i) = values_from_file[0](i);
      }
    }
    else {
      // Sanity check
      if( total_num_params != cov_data.length() )
        throw std::runtime_error("Expected num covariance values is "+convert_to_string(total_num_params)
                                 +" but incoming vector provides "+convert_to_string(cov_data.length())+".");
      for( int i=0; i<total_num_params; ++i )
        (*proposalCovMatrix)(i,i) = cov_data(i);
    }
  }
  else // "matrix" == input_fmt
  {
    if( use_file ) {
      // Sanity checks
      if( values_from_file.size() != total_num_params ) 
        throw std::runtime_error("\"matrix\" Queso covariance file data should have "
                                 +convert_to_string(total_num_params)+" columns.  Found "
                                 +convert_to_string(values_from_file.size())+" columns.");
      if( values_from_file[0].length() != total_num_params )
        throw std::runtime_error("\"matrix\" Queso covariance file data should have "
                                 +convert_to_string(total_num_params)+" rows.  Found "
                                 +convert_to_string(values_from_file[0].length())+" rows.");
      for( int i=0; i<total_num_params; ++i )
        for( int j=0; j<total_num_params; ++j )
          (*proposalCovMatrix)(i,j) = values_from_file[i](j);
    }
    else {
      // Sanity check
      if( total_num_params*total_num_params != cov_data.length() )
        throw std::runtime_error("Expected num covariance values is "+convert_to_string(total_num_params*total_num_params)
            +" but incoming vector provides "+convert_to_string(cov_data.length())+".");
      int count = 0;
      for( int i=0; i<total_num_params; ++i )
        for( int j=0; j<total_num_params; ++j )
          (*proposalCovMatrix)(i,j) = cov_data[count++];
    }
  }

  // validate that provided data is a valid covariance - test symmetry
  //    Note: I had hoped to have this check occur in the call to chol() below, 
  //    but chol doesn't seem to mind that matrices are not symmetric... RWH
  //proposalCovMatrix->print(std::cout);
  //std::cout << std::endl;
  /*QUESO::GslMatrix test_mat = proposalCovMatrix->transpose();
  test_mat -= *proposalCovMatrix;
  if( test_mat.normMax() > 1.e-14 )
    throw std::runtime_error("Queso covariance matrix is not symmetric.");

  // validate that provided data is a valid covariance matrix - test PD part of SPD
  test_mat = *proposalCovMatrix;
  int ierr = test_mat.chol();
  if( ierr == QUESO::UQ_MATRIX_IS_NOT_POS_DEFINITE_RC)
    throw std::runtime_error("Queso covariance data is not SPD.");
 */
  //proposalCovMatrix->print(std::cout);
  //std::cout << std::endl;
  //cov_data.print(std::cout);
}


/// set inverse problem options common to all solvers
void NonDQUESOBayesCalibration::set_ip_options() 
{
  calIpOptionsValues.reset(new QUESO::SipOptionsValues());
  //definitely want to retain computeSolution
  calIpOptionsValues->m_computeSolution    = true;
  calIpOptionsValues->m_dataOutputFileName = "outputData/invpb_output";
  calIpOptionsValues->m_dataOutputAllowedSet.insert(0);
  calIpOptionsValues->m_dataOutputAllowedSet.insert(1);
}


void NonDQUESOBayesCalibration::set_mh_options() 
{
  calIpMhOptionsValues.reset(new QUESO::MhOptionsValues());

  calIpMhOptionsValues->m_dataOutputFileName = "outputData/mh_output";
  calIpMhOptionsValues->m_dataOutputAllowedSet.insert(0);
  calIpMhOptionsValues->m_dataOutputAllowedSet.insert(1);

  calIpMhOptionsValues->m_rawChainDataInputFileName   = ".";
  calIpMhOptionsValues->m_rawChainSize = (numSamples) ? numSamples : 48576;
  //calIpMhOptionsValues->m_rawChainGenerateExtra     = false;
  //calIpMhOptionsValues->m_rawChainDisplayPeriod     = 20000;
  //calIpMhOptionsValues->m_rawChainMeasureRunTimes   = true;
  calIpMhOptionsValues->m_rawChainDataOutputFileName  = "outputData/raw_chain";
  calIpMhOptionsValues->m_rawChainDataOutputAllowedSet.insert(0);
  calIpMhOptionsValues->m_rawChainDataOutputAllowedSet.insert(1);
  // NO LONGER SUPPORTED.  calIpMhOptionsValues->m_rawChainComputeStats = true;

  //calIpMhOptionsValues->m_displayCandidates         = false;
  calIpMhOptionsValues->m_putOutOfBoundsInChain       = false;
  //calIpMhOptionsValues->m_tkUseLocalHessian         = false;
  //calIpMhOptionsValues->m_tkUseNewtonComponent      = true;

  // delayed rejection option:
  calIpMhOptionsValues->m_drMaxNumExtraStages
    = (mcmcType == "delayed_rejection" || mcmcType == "dram") ? 1 : 0;
  calIpMhOptionsValues->m_drScalesForExtraStages.resize(1);
  calIpMhOptionsValues->m_drScalesForExtraStages[0] = 5;
  //calIpMhOptionsValues->m_drScalesForExtraStages[1] = 10;
  //calIpMhOptionsValues->m_drScalesForExtraStages[2] = 20;

  // adaptive metropolis option:
  calIpMhOptionsValues->m_amInitialNonAdaptInterval
    = (mcmcType == "adaptive_metropolis" || mcmcType == "dram") ? 100 : 0;
  calIpMhOptionsValues->m_amAdaptInterval           = 100;
  calIpMhOptionsValues->m_amEta                     = 2.88;
  calIpMhOptionsValues->m_amEpsilon                 = 1.e-8;

  // chain management options:
  if (adaptPosteriorRefine || chainCycles > 1)
    // In this case, we process the full chain for maximum posterior values
    calIpMhOptionsValues->m_filteredChainGenerate         = false;
  else {
    // In this case, we filter the chain for final output
    calIpMhOptionsValues->m_filteredChainGenerate         = true;
    calIpMhOptionsValues->m_filteredChainDiscardedPortion = 0.;
    calIpMhOptionsValues->m_filteredChainLag              = 20;
    calIpMhOptionsValues->
      m_filteredChainDataOutputFileName = "outputData/filtered_chain";
    calIpMhOptionsValues->m_filteredChainDataOutputAllowedSet.insert(0);
    calIpMhOptionsValues->m_filteredChainDataOutputAllowedSet.insert(1);
    //calIpMhOptionsValues->m_filteredChainComputeStats   = true;
  }

  // logit transform addresses high rejection rates in corners of bounded
  // domains.  It is hardwired on at this time, although potentially redundant
  // in some cases (e.g., WIENER u-space type).
  calIpMhOptionsValues->m_doLogitTransform = logitTransform;
}


void NonDQUESOBayesCalibration::update_chain_size(unsigned int size)
{ if (size) calIpMhOptionsValues->m_rawChainSize = size; }


void NonDQUESOBayesCalibration::print_results(std::ostream& s)
{
  //NonDBayesCalibration::print_results(s);

  if (bestSamples.empty()) return;
  // ----------------------------------------
  // Output best sample which appoximates MAP
  // ----------------------------------------
  std::/*multi*/map<Real, QUESO::GslVector>::iterator it = --bestSamples.end();
  //std::pair<Real, QUESO::GslVector>& best = bestSamples.back();
  QUESO::GslVector& qv = it->second; size_t j, wpp7 = write_precision+7;
  s << "<<<<< Best parameters          =\n";
  if (standardizedSpace) {
    RealVector u_rv, x_rv; copy_gsl(qv, u_rv);
    switch (emulatorType) {
    case PCE_EMULATOR:     case SC_EMULATOR: {
      NonD* nond_iterator = (NonD*)stochExpIterator.iterator_rep();
      nond_iterator->variable_transformation().trans_U_to_X(u_rv, x_rv);
      break;
    }
    // GPs use nataf in lhs_iterator
    case KRIGING_EMULATOR: case GP_EMULATOR: {
      NonD* nond_iterator
    	= (NonD*)mcmcModel.subordinate_iterator().iterator_rep();
      nond_iterator->variable_transformation().trans_U_to_X(u_rv, x_rv);
      break;
    }
    default: //case NO_EMULATOR:
      natafTransform.trans_U_to_X(u_rv, x_rv);
      break;
    }
    write_data(Cout, x_rv);
  }
  else
    for (j=0; j<numContinuousVars; ++j)
      s << "                     " << std::setw(wpp7) << qv[j] << '\n';
  s << "<<<<< Best log posterior       =\n                     "
    << std::setw(wpp7) << it->first << '\n';

  /*
  // --------------------------
  // Multipoint results summary
  // --------------------------
  std::map<Real, QUESO::GslVector>::iterator it;
  size_t i, j, num_best = bestSamples.size(), wpp7 = write_precision+7;
  for (it=bestSamples.begin(), i=1; it!=bestSamples.end(); ++it, ++i) {
    s << "<<<<< Best parameters          ";
    if (num_best > 1) s << "(set " << i << ") ";
    s << "=\n";
    QUESO::GslVector& qv = it->second;
    for (j=0; j<numContinuousVars; ++j)
      s << "                     " << std::setw(wpp7) << qv[j] << '\n';
    s << "<<<<< Best log posterior       ";
    if (num_best > 1) s << "(set " << i << ") ";
    s << "=\n                     " << std::setw(wpp7) << it->first << '\n';
  }
  */
}


double NonDQUESOBayesCalibration::dakotaLikelihoodRoutine(
  const QUESO::GslVector& paramValues, const QUESO::GslVector* paramDirection,
  const void*         functionDataPtr, QUESO::GslVector*       gradVector,
  QUESO::GslMatrix*     hessianMatrix, QUESO::GslVector*       hessianEffect)
{
  double result = 0.;
  size_t i, j;
  int num_exp = NonDQUESOInstance->numExperiments,
      num_fn  = NonDQUESOInstance->numFunctions,
      num_cv  = NonDQUESOInstance->numContinuousVars; 

  // The GP/KRIGING/NO EMULATOR may use an unstandardized space (original)
  // and the PCE or SC cases always use standardized space.
  //
  // We had discussed having QUESO search in the original space:  this may 
  // difficult for high dimensional spaces depending on the scaling, 
  // because QUESO calculates the volume of the hypercube in which it is 
  // searching and will stop if it is too small (e.g. if one input is 
  // of small magnitude, searching in the original space will not be viable).

  //RealVector x; NonDQUESOInstance->copy_gsl(paramValues, x);
  //NonDQUESOInstance->mcmcModel.continuous_variables(x);
  for (i=0; i<num_cv; ++i)
    NonDQUESOInstance->mcmcModel.continuous_variable(paramValues[i], i);

  NonDQUESOInstance->mcmcModel.compute_response();

  const Response& resp = NonDQUESOInstance->mcmcModel.current_response();
  const RealVector& fn_vals = resp.function_values();
  if (NonDQUESOInstance->outputLevel >= DEBUG_OUTPUT)
    Cout << "Likelihood input is " << paramValues
	 << "\noutput is " << fn_vals << '\n';

  // TODO: Update treatment of standard deviations as inference
  // vs. fixed parameters; also advanced use cases of calibrated
  // scalar sigma against user-provided covariance structure.

  // Calculate the likelihood depending on what information is
  // available for the standard deviations NOTE: If the calibration of
  // the sigma terms is included, we assume ONE sigma term per
  // function is calibrated.  Otherwise, we assume that yStdData has
  // already had the correct values placed depending if there is zero,
  // one, num_fn, or a full num_exp*num_fn matrix of standard deviations.
  // Thus, we just have to iterate over this to calculate the likelihood.

  if (!NonDQUESOInstance->calibrationData) {
    for (j=0; j<num_fn; j++)
      result += std::pow(fn_vals[j],2.);
  }
  else if (NonDQUESOInstance->calibrateSigma) {
    for (i=0; i<num_exp; i++) {
      const RealVector& exp_data = NonDQUESOInstance->expData.all_data(i);
      for (j=0; j<num_fn; j++)
        result += std::pow((fn_vals[j]-exp_data[j])/paramValues[num_cv+j],2.);
    }
  }
  else {
    RealVector total_residuals( NonDQUESOInstance->expData.num_total_exppoints() );
    int cntr = 0;
    for (i=0; i<num_exp; i++) {
      RealVector residuals;
      NonDQUESOInstance->expData.form_residuals_deprecated(resp, i, residuals);
      copy_data_partial( residuals, 0, residuals.length(), total_residuals,
			 cntr );
      cntr += residuals.length();
    }
    for (i=0; i<num_exp; i++) 
      result += NonDQUESOInstance->expData.apply_covariance(total_residuals, i);

    /*ShortArray total_asv;
    bool interrogate_field_data = 
      ( ( matrixCovarianceActive ) || ( expData.interpolate_flag() ) );
    total_asv=NonDQUESOInstance->expData.determine_active_request(curr_resp,
					  interrogate_field_data);
    NonDQUESOInstance->expData.form_residuals(curr_resp, total_asv, 
					      residual_response);
    if (applyCovariance) 
      NonDQUESOInstance->expData.scale_residuals(residual_response, total_asv);
    RealVector residuals = residual_resp.function_values_view();
    result = residuals.dot( residuals );*/
  }

  result /= -2. * NonDQUESOInstance->likelihoodScale;
  if (NonDQUESOInstance->outputLevel >= DEBUG_OUTPUT)
    Cout << "Log likelihood is " << result << " Likelihood is " << exp(result)
	 << '\n';
  
  // TODO: open file once and append here, or rely on QUESO to output
  if (NonDQUESOInstance->outputLevel > NORMAL_OUTPUT) {
    std::ofstream QuesoOutput;
    QuesoOutput.open("QuesoOutput.txt", std::ios::out | std::ios::app);
    for (i=0; i<num_cv; i++) QuesoOutput << paramValues[i] << ' ' ;
    for (j=0; j<num_fn; j++) QuesoOutput << fn_vals[j] << ' ' ;
    QuesoOutput << result << '\n';
    QuesoOutput.close();
  }
  return result;
}


void NonDQUESOBayesCalibration::
copy_gsl(const QUESO::GslVector& qv, RealVector& rv)
{
  size_t i, size_qv = qv.sizeLocal();
  if (size_qv != rv.length())
    rv.sizeUninitialized(size_qv);
  for (i=0; i<size_qv; ++i)
    rv[i] = qv[i];
}


void NonDQUESOBayesCalibration::
copy_gsl(const QUESO::GslVector& qv, RealMatrix& rm, int col)
{
  size_t i, size_qv = qv.sizeLocal();
  if (col < 0 || col >= rm.numCols() || size_qv != rm.numRows()) {
    Cerr << "Error: inconsistent matrix access in copy_gsl()." << std::endl;
    abort_handler(METHOD_ERROR);
  }
  Real* rm_c = rm[col];
  for (i=0; i<size_qv; ++i)
    rm_c[i] = qv[i];
}


Real NonDQUESOBayesCalibration::prior_density(const QUESO::GslVector& qv)
{
  // TO DO: consider QUESO-based approach for this using priorRv.pdf(),
  // which may in turn call back to our GenericVectorRV prior plug-in

  // Mirrors Model::continuous_probability_density() for efficiency:
  // > avoid incurring overhead of GslVector -> RealVector copy
  // > avoid repeated dist_index lookups when looping over single var version

  // error trap on correlated random variables
  // TO DO: add support for evaluation of correlated prior density
  const Pecos::AleatoryDistParams& a_dist
    = mcmcModel.aleatory_distribution_parameters();
  if (!a_dist.uncertain_correlations().empty()) {
    Cerr << "Error: prior_density() currently uses a product of marginal "
	 << "densities\n       and can only be used for independent random "
	 << "variables at this time.\n       Consider use of variable "
	 << "transformation option." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  UShortMultiArrayConstView cv_types
    = mcmcModel.continuous_variable_types();
  Real pdf = 1.; size_t i, dist_index = 0, num_cv = cv_types.size();
  for (i=0; i<num_cv; ++i) {
    // design/epistemic/state return 1.
    pdf *= mcmcModel.continuous_probability_density(qv[i], cv_types[i],
						    dist_index);
    // shortcut: increment distribution index if same type, else reset
    if (i+1 < num_cv)
      dist_index = (cv_types[i] == cv_types[i+1]) ? dist_index + 1 : 0;
  }

  // TO DO
  //if (calibrateSigma) {
  //  for (i=num_cv; i<total; ++i)
  //    pdf *= ; // sigma priors
  //}
  return pdf;
}

} // namespace Dakota
