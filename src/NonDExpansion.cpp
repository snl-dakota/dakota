/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDExpansion
//- Description: Implementation code for NonDExpansion class
//- Owner:       Mike Eldred

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "NonDExpansion.hpp"
#include "NonDCubature.hpp"
#include "NonDQuadrature.hpp"
#include "NonDSparseGrid.hpp"
#include "NonDLHSSampling.hpp"
#include "NonDAdaptImpSampling.hpp" 
#include "RecastModel.hpp"
#include "DakotaResponse.hpp"
#include "ProblemDescDB.hpp"
#include "SharedPecosApproxData.hpp"
#include "PecosApproximation.hpp"
#include "pecos_stat_util.hpp"
#include "SensAnalysisGlobal.hpp"
#include "DiscrepancyCorrection.hpp"

//#define DEBUG
//#define CONVERGENCE_DATA


namespace Dakota {

NonDExpansion::NonDExpansion(Model& model): NonD(model),
  expansionCoeffsApproach(-1), numUncertainQuant(0), numSamplesOnModel(0),
  numSamplesOnExpansion(probDescDB.get_int("method.samples")),
  nestedRules(false),
  piecewiseBasis(probDescDB.get_bool("method.nond.piecewise_basis")),
  useDerivs(probDescDB.get_bool("method.derivative_usage")),
  refineType(probDescDB.get_short("method.nond.expansion_refinement_type")),
  refineControl(
    probDescDB.get_short("method.nond.expansion_refinement_control")),
  ruleNestingOverride(probDescDB.get_short("method.nond.nesting_override")),
  ruleGrowthOverride(probDescDB.get_short("method.nond.growth_override")),
  expSampling(false), impSampling(false),
  covarianceControl(probDescDB.get_short("method.nond.covariance_control")),
  vbdFlag(probDescDB.get_bool("method.variance_based_decomp")),
  // Note: minimum VBD order for variance-controlled refinement is enforced
  //       in NonDExpansion::construct_{quadrature,sparse_grid}
  vbdOrderLimit(probDescDB.get_ushort("method.nond.vbd_interaction_order")),
  vbdDropTol(probDescDB.get_real("method.vbd_drop_tolerance"))
{
  // override default definition in NonD ctor.  If there are any aleatory
  // variables, then we will sample on that subset for probabilistic stats.
  epistemicStats = (numEpistemicUncVars && !numAleatoryUncVars);

  initialize_response_covariance();
  initialize_final_statistics(); // level mappings are available
}


NonDExpansion::
NonDExpansion(Model& model, short exp_coeffs_approach, short u_space_type,
	      bool piecewise_basis, bool use_derivs):
  NonD(NoDBBaseConstructor(), model),
  expansionCoeffsApproach(exp_coeffs_approach),
  numUncertainQuant(0), numSamplesOnModel(0), numSamplesOnExpansion(0),
  nestedRules(false), piecewiseBasis(piecewise_basis), useDerivs(use_derivs),
  refineType(Pecos::NO_REFINEMENT), refineControl(Pecos::NO_CONTROL),
  ruleNestingOverride(Pecos::NO_NESTING_OVERRIDE),
  ruleGrowthOverride(Pecos::NO_GROWTH_OVERRIDE), expSampling(false),
  impSampling(false), vbdFlag(false), vbdOrderLimit(0), vbdDropTol(-1.),
  covarianceControl(DEFAULT_COVARIANCE)
{
  // override default definition in NonD ctor.  If there are any aleatory
  // variables, then we will sample on that subset for probabilistic stats.
  epistemicStats = (numEpistemicUncVars && !numAleatoryUncVars);

  // level mappings not yet available
  // (defer initialize_response_covariance() and initialize_final_statistics())
}


NonDExpansion::~NonDExpansion()
{ 
  if (impSampling)
    uSpaceModel.free_communicators(importanceSampler.maximum_concurrency());
}


void NonDExpansion::initialize_response_covariance()
{
  // if diagonal only, utilize a vector (or sparse matrix) to optimize
  // both computational performance and memory footprint)
  bool refine_by_covar = (refineControl == Pecos::UNIFORM_CONTROL ||
    refineControl   == Pecos::DIMENSION_ADAPTIVE_CONTROL_SOBOL ||
    refineControl   == Pecos::DIMENSION_ADAPTIVE_CONTROL_DECAY ||
    ( refineControl == Pecos::DIMENSION_ADAPTIVE_CONTROL_GENERALIZED &&
      !totalLevelRequests ) );
  switch (covarianceControl) {
  case DEFAULT_COVARIANCE: // assign context-specific default
    if (refine_by_covar)      covarianceControl = FULL_COVARIANCE;
    else if (subIteratorFlag) covarianceControl = NO_COVARIANCE;
    else                      covarianceControl = (numFunctions > 10) ?
				DIAGONAL_COVARIANCE : FULL_COVARIANCE;
    break;
  case NO_COVARIANCE: // won't happen since NO_COVARIANCE not exposed in spec
    if (refine_by_covar) {
      Cerr << "Warning: covariance required by refinement.  Adding diagonal "
	   << "covariance terms." << std::endl;
      covarianceControl = DIAGONAL_COVARIANCE;
    }
    break;
  }
  // now that setting is defined, size the variance/covariance storage
  if (covarianceControl == FULL_COVARIANCE)
    respCovariance.shapeUninitialized(numFunctions);
  else if (covarianceControl == DIAGONAL_COVARIANCE)
    respVariance.sizeUninitialized(numFunctions);
}


void NonDExpansion::resolve_inputs(short& u_space_type, short& data_order)
{
  bool err_flag = false;

  // Check for suitable distribution types.
  if (numDiscreteIntVars || numDiscreteRealVars) {
    Cerr << "\nError: discrete random variables are not currently supported in "
	 << "NonDExpansion." << std::endl;
    err_flag = true;
  }

  // check compatibility of refinement type and u-space type
  if (refineType == Pecos::H_REFINEMENT) { // override
    if (u_space_type == ASKEY_U) // non-default
      Cerr << "\nWarning: overriding ASKEY to STD_UNIFORM for h-refinement.\n"
	   << std::endl;
    else if (u_space_type == STD_NORMAL_U) // non-default
      Cerr << "\nWarning: overriding WIENER to STD_UNIFORM for h-refinement.\n"
	   << std::endl;
    u_space_type = STD_UNIFORM_U; piecewiseBasis = true;
  }
  else if (refineType == Pecos::P_REFINEMENT && piecewiseBasis) {
    Cerr << "\nError: fixed order piecewise bases are incompatible with "
	 << "p-refinement." << std::endl;
    err_flag = true;
  }

  if (err_flag)
    abort_handler(-1);
}


void NonDExpansion::initialize(short u_space_type)
{
  // if multifidelity UQ, initial phase captures the model discrepancy
  if (iteratedModel.surrogate_type() == "hierarchical")
    iteratedModel.surrogate_response_mode(MODEL_DISCREPANCY);

  // use Wiener/Askey/extended/piecewise u-space defn in Nataf transformation
  initialize_random_variable_transformation();
  initialize_random_variable_types(u_space_type); // need x/u_types below
  initialize_random_variable_correlations();
  // for lightweight ctor, defer until call to requested_levels()
  //initialize_final_statistics();

  // check for correlations that are not supported for (1) use in extended
  // u-space and (2) use by Der Kiureghian & Liu for basic u-space (bounded
  // normal, bounded lognormal, loguniform, triangular, and beta vars).
  size_t i, j;
  if (natafTransform.x_correlation()) {

    const Pecos::ShortArray&    x_types = natafTransform.x_types();
    Pecos::ShortArray           u_types = natafTransform.u_types(); // copy
    const Pecos::RealSymMatrix& x_corr  = natafTransform.x_correlation_matrix();

    // We can only decorrelate in std normal space; therefore, if a variable
    // with a u_type other than STD_NORMAL is correlated with anything, revert
    // its u_type to STD_NORMAL.   Note: loop below must check all columns,
    // despite symmetry, since only rows are checked for u_type.
    bool u_space_modified = false;
    for (i=numContDesVars; i<numContDesVars+numContAleatUncVars; ++i)
      if (u_types[i] != Pecos::STD_NORMAL)
	for (j=numContDesVars; j<numContDesVars+numContAleatUncVars; ++j)
	  if (i != j && std::fabs(x_corr(i, j)) > Pecos::SMALL_NUMBER) {
	    Cerr << "\nWarning: in NonDExpansion, u-space type for random "
		 << "variable " << i-numContDesVars+1 << " changed to\n"
		 << "         STD_NORMAL due to decorrelation requirements.\n";
	    u_types[i] = Pecos::STD_NORMAL; u_space_modified = true; break;
	  }

    if (u_space_modified) // update ranVarTypesU in natafTransform
      natafTransform.initialize_random_variable_types(x_types, u_types);

    verify_correlation_support(); // Der Kiureghian & Liu correlation warping
  }
}


void NonDExpansion::
construct_cubature(Iterator& u_space_sampler, Model& g_u_model,
		   unsigned short cub_int_order)
{
  // sanity checks: CUBATURE precluded since no grid anisotropy for adaptive
  // and very limited refinement opportunities for uniform/adaptive
  if (refineType) {
    Cerr << "Error: uniform/adaptive refinement of cubature grids not "
	 << "supported." << std::endl;
    abort_handler(-1);
  }

  u_space_sampler.assign_rep(new 
    NonDCubature(g_u_model, natafTransform.u_types(), cub_int_order), false);
}


void NonDExpansion::
construct_quadrature(Iterator& u_space_sampler, Model& g_u_model,
		     const UShortArray& quad_order_seq,
		     const RealVector& dim_pref)
{
  // sanity checks: no GSG for TPQ
  if (refineControl == Pecos::DIMENSION_ADAPTIVE_CONTROL_GENERALIZED) {
    Cerr << "Error: generalized option does not support adaptive refinement of "
	 << "tensor grids." << std::endl;
    abort_handler(-1);
  }

  // enforce minimum required VBD control
  if (!vbdFlag && refineControl == Pecos::DIMENSION_ADAPTIVE_CONTROL_SOBOL)
    { vbdFlag = true; vbdOrderLimit = 1; }

  // manage rule nesting override
  nestedRules = ( ruleNestingOverride == Pecos::NESTED ||
		  ( refineType && ruleNestingOverride != Pecos::NON_NESTED ) );

  u_space_sampler.assign_rep(new
    NonDQuadrature(g_u_model, quad_order_seq, dim_pref), false);
}


void NonDExpansion::
construct_quadrature(Iterator& u_space_sampler, Model& g_u_model,
		     int filtered_samples, const RealVector& dim_pref)
{
  // sanity checks: only uniform refinement supported for probabilistic
  // collocation (regression using filtered tensor grids)
  if (refineType && refineControl > Pecos::UNIFORM_CONTROL) {
    Cerr << "Error: only uniform refinement is supported for regression with "
	 << "the tensor_grid option." << std::endl;
    abort_handler(-1);
  }

  /*
  // enforce minimum required VBD control
  if (!vbdFlag && refineControl == Pecos::DIMENSION_ADAPTIVE_CONTROL_SOBOL)
    { vbdFlag = true; vbdOrderLimit = 1; }
  // nested overrides not currently part of tensor regression spec
  nestedRules = (ruleNestingOverride == Pecos::NESTED ||
    (refineType && ruleNestingOverride != Pecos::NON_NESTED));
  */

  u_space_sampler.assign_rep(new
    NonDQuadrature(g_u_model, filtered_samples, dim_pref), false);
}


void NonDExpansion::
construct_quadrature(Iterator& u_space_sampler, Model& g_u_model,
		     int random_samples, int seed,
		     const UShortArray& quad_order_seq,
		     const RealVector& dim_pref)
{
  // sanity checks: only uniform refinement supported for probabilistic
  // collocation (regression using filtered tensor grids)
  if (refineType && refineControl > Pecos::UNIFORM_CONTROL) {
    Cerr << "Error: only uniform refinement is supported for regression with "
	 << "the tensor_grid option." << std::endl;
    abort_handler(-1);
  }

  /*
  // enforce minimum required VBD control
  if (!vbdFlag && refineControl == Pecos::DIMENSION_ADAPTIVE_CONTROL_SOBOL)
    { vbdFlag = true; vbdOrderLimit = 1; }
  // nested overrides not currently part of tensor regression spec
  nestedRules = (ruleNestingOverride == Pecos::NESTED ||
    (refineType && ruleNestingOverride != Pecos::NON_NESTED));
  */

  u_space_sampler.assign_rep(new NonDQuadrature(
    g_u_model, random_samples, seed, quad_order_seq, dim_pref), false);
}


void NonDExpansion::
construct_sparse_grid(Iterator& u_space_sampler, Model& g_u_model,
		      const UShortArray& ssg_level_seq,
		      const RealVector& dim_pref)
{
  // enforce minimum required VBD control
  if (!vbdFlag && refineControl == Pecos::DIMENSION_ADAPTIVE_CONTROL_SOBOL)
    { vbdFlag = true; vbdOrderLimit = 1; }

  nestedRules = (ruleNestingOverride != Pecos::NON_NESTED);

  // tracking of unique product weights needed for PCE and SC standard modes
  // since both employ PolynomialApproximation::compute_numerical_moments(4).
  // Neither PCE nor SC require product wts for all_vars mode, since moment
  // calculations must employ gauss_wts_1d.
  // Exception 1: all_vars Nodal SC requires weights for total covariance()
  //              evaluation in VBD.
  // Exception 2: NonDIntegration::print_points_weights() needs weights for
  //              outputLevel > NORMAL_OUTPUT.
  bool all_vars  = (numContDesVars || numContEpistUncVars || numContStateVars);
  bool nodal_vbd = (vbdFlag && methodName == "nond_stoch_collocation" &&
    expansionCoeffsApproach != Pecos::HIERARCHICAL_SPARSE_GRID);
  bool track_wts = (!all_vars || nodal_vbd || outputLevel > NORMAL_OUTPUT);

  // tracking of collocation indices within the SparseGridDriver
  bool track_colloc = (expansionCoeffsApproach == Pecos::COMBINED_SPARSE_GRID
    || refineControl == Pecos::DIMENSION_ADAPTIVE_CONTROL_GENERALIZED);

  short growth_rate;
  if (ruleGrowthOverride == Pecos::UNRESTRICTED ||
      refineControl      == Pecos::DIMENSION_ADAPTIVE_CONTROL_GENERALIZED)
    // unstructured index set evolution: no motivation to restrict
    growth_rate = Pecos::UNRESTRICTED_GROWTH;
  else if (piecewiseBasis)
    // no reason to match Gaussian precision, but restriction still useful:
    // use SLOW i=2l+1 since it is more natural for NEWTON_COTES,CLENSHAW_CURTIS
    // and is more consistent with UNRESTRICTED generalized sparse grids.
    growth_rate = Pecos::SLOW_RESTRICTED_GROWTH;
  else // standardize rules on linear Gaussian prec: i = 2m-1 = 2(2l+1)-1 = 4l+1
    growth_rate = Pecos::MODERATE_RESTRICTED_GROWTH;

  u_space_sampler.assign_rep(new
    NonDSparseGrid(g_u_model, expansionCoeffsApproach, ssg_level_seq, dim_pref,
		   growth_rate, refineControl, track_wts, track_colloc), false);
}


/*
void NonDExpansion::
construct_incremental_lhs(Iterator& u_space_sampler, Model& u_model,
			  int num_samples, int seed, const String& rng)
{
  // sanity checks
  if (num_samples <= 0) {
    Cerr << "Error: bad samples specification (" << num_samples << ") in "
	 << "NonDExpansion::construct_incremental_lhs()." << std::endl;
    abort_handler(-1);
  }

  // use default LHS sample_type for consistency with collocation support for
  // incremental_lhs but not incremental_random
  String sample_type; // default
  u_space_sampler.assign_rep(new NonDIncremLHSSampling(u_model, sample_type,
    num_samples, orig_seed, rng, ACTIVE), false);
}
*/


void NonDExpansion::initialize_u_space_model()
{
  // Commonly used approx settings are passed through the DataFitSurrModel ctor
  // chain.  Additional data are passed using Pecos::ExpansionConfigOptions.
  // Note: passing outputLevel again is redundant with DataFitSurrModel ctor.
  SharedPecosApproxData* shared_data_rep = (SharedPecosApproxData*)
    uSpaceModel.shared_approximation().data_rep();
  Pecos::ExpansionConfigOptions ec_options(expansionCoeffsApproach, outputLevel,
    vbdFlag, vbdOrderLimit, refineControl, maxIterations, convergenceTol);
  shared_data_rep->configuration_options(ec_options);

  // if all variables mode, initialize key to random variable subset
  if (numContDesVars || numContEpistUncVars || numContStateVars) {
    Pecos::BitArray random_vars_key(numContinuousVars);
    size_t i, num_cdv_cauv = numContDesVars + numContAleatUncVars;
    for (i=0; i<numContinuousVars; ++i)
      random_vars_key[i] = (i >= numContDesVars && i < num_cdv_cauv);
    shared_data_rep->random_variables_key(random_vars_key);
  }

  // if numerical integration, manage u_space_sampler updates
  if (expansionCoeffsApproach == Pecos::QUADRATURE ||
      expansionCoeffsApproach == Pecos::CUBATURE ||
      expansionCoeffsApproach == Pecos::COMBINED_SPARSE_GRID ||
      expansionCoeffsApproach == Pecos::HIERARCHICAL_SPARSE_GRID) {
    Iterator& u_space_sampler = uSpaceModel.subordinate_iterator();
    Model&    g_u_model       = uSpaceModel.subordinate_model();
    shared_data_rep->integration_iterator(u_space_sampler);
    u_space_sampler.output_level(outputLevel); // for tabular output of pts/wts
    numSamplesOnModel = u_space_sampler.maximum_concurrency()
                      / g_u_model.derivative_concurrency();
    // maxConcurrency already updated for expansion samples and regression
    if (numSamplesOnModel) // optional with default = 0
      maxConcurrency *= numSamplesOnModel;
  }
}


void NonDExpansion::construct_expansion_sampler()
{
  //expSampling = impSampling = false; // initialized in ctor
  if (totalLevelRequests)
    for (size_t i=0; i<numFunctions; ++i)
      if ( requestedProbLevels[i].length() || requestedGenRelLevels[i].length()
	   || ( requestedRespLevels[i].length() &&
		respLevelTarget != RELIABILITIES ) )
	{ expSampling = true; break; }

  if (expSampling) {
    // sanity check for samples spec
    if (!numSamplesOnExpansion) {
      Cerr << "\nError: number of samples must be specified for numerically "
	   << "evaluating statistics on a stochastic expansion." << std::endl;
      abort_handler(-1);
    }

    // could use construct_lhs() except for non-default ALEATORY_UNCERTAIN
    // sampling mode.  Don't vary sampling pattern since we want to reuse
    // same sampling stencil for different design/epistemic vars or for
    // (goal-oriented) adaptivity.
    const String& sample_type = probDescDB.get_string("method.sample_type");
    int orig_seed = probDescDB.get_int("method.random_seed");
    const String& rng = probDescDB.get_string("method.random_number_generator");
    expansionSampler.assign_rep(new NonDLHSSampling(uSpaceModel, sample_type,
      numSamplesOnExpansion, orig_seed, rng, false, ALEATORY_UNCERTAIN), false);

    //expansionSampler.sampling_reset(numSamplesOnExpansion, true, false);
    NonD* exp_sampler_rep = (NonD*)expansionSampler.iterator_rep();
    // publish output verbosity
    exp_sampler_rep->output_level(outputLevel);
    // publish level mappings to expansion sampler, but suppress reliability
    // moment mappings, which are performed locally within compute_statistics()
    RealVectorArray empty_rv_array; // empty
    RealVectorArray& req_resp_levs = (respLevelTarget == RELIABILITIES) ?
      empty_rv_array : requestedRespLevels;
    exp_sampler_rep->requested_levels(req_resp_levs, requestedProbLevels,
      empty_rv_array, requestedGenRelLevels, respLevelTarget,
      respLevelTargetReduce, cdfFlag);

    const String& integration_refine
      = probDescDB.get_string("method.nond.integration_refinement");
    if (!integration_refine.empty())
      for (size_t i=0; i<numFunctions; ++i)
	if (requestedRespLevels[i].length() && respLevelTarget != RELIABILITIES)
	  { impSampling = true; break; }

    if (impSampling) {
      short is_type;
      if (integration_refine == "is")
	is_type = IS;
      else if (integration_refine == "ais")
	is_type = AIS;
      else if (integration_refine == "mmais")
	is_type = MMAIS;
      Cout << "IS refinement type " << is_type;

      int refine_samples = probDescDB.get_int("method.nond.refinement_samples");
      if (!refine_samples) refine_samples = 1000; // context-specific default
      bool vary_pattern = true;
      importanceSampler.assign_rep(new NonDAdaptImpSampling(uSpaceModel,
	sample_type, refine_samples, orig_seed, rng, vary_pattern, is_type,
	cdfFlag, false, false, false), false);
      uSpaceModel.init_communicators(importanceSampler.maximum_concurrency());
 
      NonDAdaptImpSampling* imp_sampler_rep = 
        (NonDAdaptImpSampling*)importanceSampler.iterator_rep();
      imp_sampler_rep->output_level(outputLevel);
      imp_sampler_rep->requested_levels(req_resp_levs, empty_rv_array,
	empty_rv_array, empty_rv_array, respLevelTarget, respLevelTargetReduce,
	cdfFlag);
    }
  }
}


void NonDExpansion::quantify_uncertainty()
{
  initialize_expansion();

  // single fidelity or multifidelity discrepancy expansion
  compute_expansion();  // nominal iso/aniso expansion from input spec
  if (refineType)
    refine_expansion(); // uniform/adaptive p-/h-refinement

  // multifidelity LF expansion
  if (iteratedModel.surrogate_type() == "hierarchical") {
    // output and capture discrepancy results
    Cout << "\n-------------------------------------------"
	 << "\nMultifidelity UQ: model discrepancy results"
	 << "\n-------------------------------------------\n\n";
    compute_print_converged_results(true);
    // store current state.  Note: a subsequent finalize_approximation()
    // within refine_expansion() must distinguish between saved trial sets
    // and stored expansions.
    uSpaceModel.store_approximation();

    // change HierarchSurrModel::responseMode to uncorrected LF model
    iteratedModel.surrogate_response_mode(UNCORRECTED_SURROGATE);
    increment_specification_sequence(); // advance from discrepancy to LF spec
    update_expansion();   // nominal iso/aniso expansion from input spec
    if (refineType)
      refine_expansion(); // uniform/adaptive p-/h-refinement
    Cout << "\n--------------------------------------"
	 << "\nMultifidelity UQ: low fidelity results"
	 << "\n--------------------------------------\n\n";
    compute_print_converged_results(true);

    // compute aggregate expansion and generate its statistics
    uSpaceModel.combine_approximation(
      iteratedModel.discrepancy_correction().correction_type());
    Cout << "\n----------------------------------------------------"
	 << "\nMultifidelity UQ: approximated high fidelity results"
	 << "\n----------------------------------------------------\n\n";
  }
  
  // generate final results
  compute_print_converged_results();
  update_final_statistics(); // virtual fn redefined below
  ++numUncertainQuant;
}


void NonDExpansion::update_final_statistics()
{
  // aleatory final stats & their grads are updated directly within
  // compute_statistics() (NonD::update_aleatory_final_statistics() is awkward
  // for NonDExpansion since Pecos manages the moment arrays), such that all
  // that remains are system final stats and additional gradient scaling.
  update_system_final_statistics();
  update_system_final_statistics_gradients();
  update_final_statistics_gradients();
}


void NonDExpansion::initialize_expansion()
{
  // update ranVar info to capture any distribution param insertions
  initialize_random_variable_parameters();
  initialize_final_statistics_gradients();
  natafTransform.transform_correlations();

  // now that data has flowed down at run-time from any higher level recursions
  // to iteratedModel, it must be propagated up through the local g_u_model and
  // uSpaceModel recursions (so they are correct when propagated back down).
  //
  // RecastModel::update_from_sub_model() has insufficient context to update
  // distribution params for variables that are not transformed (i.e., 
  // numerically-generated bases/points); it assumes that the presence of any
  // variable transformation precludes flow of distribution parameters.  So
  // we handle that special case here prior to the general recursion.  The
  // alternative would be to supply another function pointer to RecastModel
  // to support partial distribution parameter mappings.
  Pecos::AleatoryDistParams& adp_u
    = uSpaceModel.subordinate_model().aleatory_distribution_parameters();
  const Pecos::AleatoryDistParams& adp_x
    = iteratedModel.aleatory_distribution_parameters();
  adp_u.update_partial(adp_x, natafTransform.x_types(),
		       natafTransform.u_types());
  // now perform the general recursion
  uSpaceModel.update_from_subordinate_model(); // recurse_flag = true

  // propagate latest natafTransform settings to u-space sampler
  Iterator& u_space_sampler = uSpaceModel.subordinate_iterator();
  if (!u_space_sampler.is_null())
    ((NonD*)u_space_sampler.iterator_rep())->
      initialize_random_variables(natafTransform);

  // if a sub-iterator, reset any refinements that may have occurred
  if (subIteratorFlag && numUncertainQuant && refineType &&
      !u_space_sampler.is_null())
    u_space_sampler.reset();

  // store the current design/state vars in u-space
  size_t i, j, cntr = 0;
  RealVector initial_pt_x;
  if (numContDesVars || numContEpistUncVars || numContStateVars ||
      !subIteratorFlag){
    copy_data(iteratedModel.continuous_variables(), initial_pt_x); // view->copy
    if (numUncertainQuant) { // reset uncertain values to means
      const Pecos::RealVector& x_means = natafTransform.x_means();
      for (i=numContDesVars; i<numContDesVars + numContAleatUncVars; ++i)
	initial_pt_x[i] = x_means[i];
    }
    natafTransform.trans_X_to_U(initial_pt_x, initialPtU);
  }
}


void NonDExpansion::compute_expansion()
{
#ifdef DERIV_DEBUG
  // numerical verification of analytic Jacobian/Hessian routines
  RealVector rdv_u;
  natafTransform.trans_X_to_U(iteratedModel.continuous_variables(), rdv_u);
  natafTransform.verify_trans_jacobian_hessian(rdv_u);//(rdv_x);
  natafTransform.verify_design_jacobian(rdv_u);
#endif // DERIV_DEBUG

  Iterator& u_space_sampler = uSpaceModel.subordinate_iterator();
  NonD* u_space_sampler_rep = (NonD*)u_space_sampler.iterator_rep();

  const ShortArray& final_asv = finalStatistics.active_set_request_vector();
  const SizetArray& final_dvv = finalStatistics.active_set_derivative_vector();
  size_t i, j, rl_len, pl_len, bl_len, gl_len, cntr = 0,
    num_final_stats = final_asv.size(), num_final_grad_vars = final_dvv.size();
  bool final_stat_value_flag = false, final_stat_grad_flag = false;
  for (i=0; i<num_final_stats; ++i)
    if (final_asv[i] & 1)
      { final_stat_value_flag = true; break; }
  for (i=0; i<num_final_stats; ++i)
    if (final_asv[i] & 2)
      { final_stat_grad_flag  = true; break; }

  // define ASV for u_space_sampler and expansion coefficient/gradient
  // data flags for PecosApproximation
  bool all_vars = (numContDesVars || numContEpistUncVars || numContStateVars);
  ShortArray sampler_asv(numFunctions, 0);
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  for (i=0; i<numFunctions; ++i) {
    PecosApproximation* poly_approx_rep
      = (PecosApproximation*)poly_approxs[i].approx_rep();
    bool expansion_coeff_flag = false, expansion_grad_flag = false,
         mean_grad_flag       = false, std_dev_grad_flag   = false;
    if (totalLevelRequests) {
      rl_len = requestedRespLevels[i].length();
      pl_len = requestedProbLevels[i].length();
      bl_len = requestedRelLevels[i].length();
      gl_len = requestedGenRelLevels[i].length();
    }
    else
      rl_len = pl_len = bl_len = gl_len = 0;

    if (final_stat_value_flag)
      // map final_asv value bits into expansion_coeff_flag requirements
      for (j=0; j<2+rl_len+pl_len+bl_len+gl_len; ++j)
	if (final_asv[cntr+j] & 1)
	  { expansion_coeff_flag = true; break; }

    if (final_stat_grad_flag) {
      // map final_asv gradient bits into moment grad requirements
      if (final_asv[cntr++] & 2)
	mean_grad_flag    = true;
      if (final_asv[cntr++] & 2)
	std_dev_grad_flag = true;
      if (respLevelTarget == RELIABILITIES)
	for (j=0; j<rl_len; ++j)
	  if (final_asv[cntr+j] & 2)
	    { mean_grad_flag = std_dev_grad_flag = true; break; }
      cntr += rl_len + pl_len;
      for (j=0; j<bl_len; ++j)
	if (final_asv[cntr+j] & 2)
	  { mean_grad_flag = std_dev_grad_flag = true; break; }
      cntr += bl_len + gl_len;
      // map moment grad requirements into expansion_{coeff,grad}_flag reqmts
      // (refer to *PolyApproximation::get_*_gradient() implementations)
      if (all_vars) { // aleatory + design/epistemic
	if (std_dev_grad_flag)
	  expansion_coeff_flag = true;
	size_t deriv_index, num_deriv_vars = final_dvv.size();
	for (j=0; j<num_deriv_vars; ++j) {
	  deriv_index = final_dvv[j] - 1; // OK since we are in an "All" view
	  if (deriv_index >= numContDesVars &&
	      deriv_index <  numContDesVars + numContAleatUncVars) { // ran var
	    if (mean_grad_flag || std_dev_grad_flag)
	      expansion_grad_flag = true;
	  }
	  else if (mean_grad_flag)
	    expansion_coeff_flag = true;
	}
      }
      else { // aleatory expansion variables
	if (mean_grad_flag)
	  expansion_grad_flag = true;
	if (std_dev_grad_flag)
	  expansion_coeff_flag = expansion_grad_flag = true;
      }
    }
    else
      cntr += 2 + rl_len + pl_len + bl_len + gl_len;

    // map expansion_{coeff,grad}_flag requirements into ASV and
    // PecosApproximation settings
    if (expansion_coeff_flag)
      sampler_asv[i] |= 1;
    if (expansion_grad_flag || useDerivs)
      sampler_asv[i] |= 2;
    poly_approx_rep->expansion_coefficient_flag(expansion_coeff_flag);
    poly_approx_rep->expansion_gradient_flag(expansion_grad_flag);
  }

  // If OUU/SOP (multiple calls to quantify_uncertainty()), an expansion
  // constructed over the full range of all variables does not need to be
  // reconstructed on subsequent calls.  However, an all_vars construction
  // over a trust region needs rebuilding when the trust region is updated.
  // In the checks below, all_approx detects any variable insertions or ASV
  // omissions and force_rebuild() manages variable augmentations.
  bool all_approx = false;
  if (all_vars && numUncertainQuant && secondaryACVarMapTargets.empty()) {
    all_approx = true;
    // does sampler_asv contain content not evaluated previously
    const ShortArray& prev_asv = u_space_sampler.active_set().request_vector();
    for (i=0; i<numFunctions; ++i)
      // bit-wise AND checks if each sampler_asv bit is present in prev_asv
      if ( (prev_asv[i] & sampler_asv[i]) != sampler_asv[i] )
	{ all_approx = false; break; }
  }
  if (!all_approx || uSpaceModel.force_rebuild()) {

    if (u_space_sampler_rep) {

      // Set the sampler ASV (defined from previous loop over numFunctions)
      ActiveSet sampler_set;
      sampler_set.request_vector(sampler_asv);

      // if required statistical sensitivities are not covered by All variables
      // mode for augmented design variables, then the simulations must evaluate
      // response sensitivities.
      bool sampler_grad = false, dist_param_deriv = false;
      if (final_stat_grad_flag) {
	size_t i, num_outer_cv = secondaryACVarMapTargets.size();
	for (i=0; i<num_outer_cv; ++i)
	  if (secondaryACVarMapTargets[i] != Pecos::NO_TARGET) // insertion
	    { dist_param_deriv = true; break; }
	sampler_grad = (all_vars) ? dist_param_deriv : true;
      }
      u_space_sampler_rep->distribution_parameter_derivatives(dist_param_deriv);
      if (dist_param_deriv)
	u_space_sampler.active_variable_mappings(primaryACVarMapIndices,
	  primaryADIVarMapIndices, primaryADRVarMapIndices,
	  secondaryACVarMapTargets, secondaryADIVarMapTargets,
	  secondaryADRVarMapTargets);

      // Set the u_space_sampler DVV, managing different gradient modes & their
      // combinations.  The u_space_sampler's DVV may then be augmented for
      // correlations in NonD::set_u_to_x_mapping().  Sources for DVV content
      // include the model's continuous var ids and the final_dvv set by a
      // NestedModel.  In the latter case, NestedModel::derived_compute_response
      // maps top-level optimizer derivative vars to sub-iterator derivative
      // vars in NestedModel::set_mapping() and then sets this DVV within
      // finalStats using subIterator.response_results_active_set().
      if (useDerivs) {
	SizetMultiArrayConstView cv_ids
	  = iteratedModel.continuous_variable_ids();
	if (sampler_grad) { // merge cv_ids with final_dvv
	  SizetSet merged_set; SizetArray merged_dvv;
	  merged_set.insert(cv_ids.begin(), cv_ids.end());
	  merged_set.insert(final_dvv.begin(), final_dvv.end());
	  std::copy(merged_set.begin(), merged_set.end(), merged_dvv.begin());
	  sampler_set.derivative_vector(merged_dvv);
	}
	else // assign cv_ids
	  sampler_set.derivative_vector(cv_ids);
      }
      else if (all_vars && sampler_grad) { // filter: retain only insertion tgts
	SizetArray filtered_final_dvv;
	size_t num_cdv_cauv = numContDesVars+numContAleatUncVars;
	for (i=0; i<num_final_grad_vars; ++i) {
	  size_t dvv_i = final_dvv[i];
	  if (dvv_i > numContDesVars && dvv_i <= num_cdv_cauv)
	    filtered_final_dvv.push_back(dvv_i);
	}
	sampler_set.derivative_vector(filtered_final_dvv);
      }
      else if (sampler_grad)
	sampler_set.derivative_vector(final_dvv);
      else // derivs not needed, but correct DVV len needed for MPI buffers
	sampler_set.derivative_vector(iteratedModel.continuous_variable_ids());

      // Build the orthogonal/interpolation polynomial approximations:
      u_space_sampler.active_set(sampler_set);
    }

    uSpaceModel.build_approximation();
  }
}


void NonDExpansion::refine_expansion()
{
  // --------------------------------------
  // Uniform/adaptive refinement approaches
  // --------------------------------------
  // DataMethod default for maxIterations is -1, indicating no user spec.
  // Assign a context-specific default in this case.
  size_t i, iter = 1, max_iter = (maxIterations < 0) ? 100 : maxIterations;
  bool converged = (iter > max_iter);
  Real metric;

  // post-process nominal expansion
  if (!converged)
    compute_print_iteration_results(true);

  // initialize refinement algorithms (if necessary)
  switch (refineControl) {
  case Pecos::DIMENSION_ADAPTIVE_CONTROL_GENERALIZED:
    initialize_sets(); break;
  }

  while (!converged) {

    switch (refineControl) {
    case Pecos::UNIFORM_CONTROL:
      switch (expansionCoeffsApproach) {
      case Pecos::QUADRATURE: case Pecos::COMBINED_SPARSE_GRID:
      case Pecos::HIERARCHICAL_SPARSE_GRID: {
	// ramp SSG level or TPQ order, keeping initial isotropy/anisotropy
	NonDIntegration* nond_integration = (NonDIntegration*)
	  uSpaceModel.subordinate_iterator().iterator_rep();
	nond_integration->increment_grid(); // TPQ or SSG
	update_expansion();
	break;
      }
      case Pecos::DEFAULT_REGRESSION: case Pecos::DEFAULT_LEAST_SQ_REGRESSION:
      case Pecos::SVD_LEAST_SQ_REGRESSION:
      case Pecos::EQ_CON_LEAST_SQ_REGRESSION:
      case Pecos::BASIS_PURSUIT:        case Pecos::BASIS_PURSUIT_DENOISING:
      case Pecos::ORTHOG_MATCH_PURSUIT: case Pecos::LASSO_REGRESSION:
      case Pecos::LEAST_ANGLE_REGRESSION:
	// ramp expansion order and update regression samples, keeping
	// initial collocation ratio (either user specified or inferred)
	increment_order(); // virtual fn defined for NonDPCE
	update_expansion(); // invokes uSpaceModel.build_approximation()
	break;
      }
      metric = compute_covariance_metric();
      break;
    case Pecos::DIMENSION_ADAPTIVE_CONTROL_SOBOL: {
      // Dimension adaptive refinement: define anisotropic preference
      // vector from total Sobol' indices, averaged over response fn set.
      RealVector dim_pref;
      reduce_total_sobol_sets(dim_pref);
      // incrementing grid & updating aniso wts best performed together
      NonDIntegration* nond_integration = (NonDIntegration*)
	uSpaceModel.subordinate_iterator().iterator_rep();
      nond_integration->increment_grid_preference(dim_pref); // TPQ or SSG
      update_expansion();
      metric = compute_covariance_metric();
      break;
    }
    case Pecos::DIMENSION_ADAPTIVE_CONTROL_DECAY: {
      // Dimension adaptive refinement: define anisotropic weight vector
      // from min of spectral decay rates (PCE only) over response fn set.
      RealVector aniso_wts;
      reduce_decay_rate_sets(aniso_wts);
      // incrementing grid & updating aniso wts best performed together
      NonDIntegration* nond_integration = (NonDIntegration*)
	uSpaceModel.subordinate_iterator().iterator_rep();
      nond_integration->increment_grid_weights(aniso_wts); // TPQ or SSG
      update_expansion();
      metric = compute_covariance_metric();
      break;
    }
    case Pecos::DIMENSION_ADAPTIVE_CONTROL_GENERALIZED:
      // Dimension adaptive refinement using generalized sparse grids.
      // > Start GSG from iso/aniso SSG: starting from scratch (w=0) is
      //   most efficient if fully nested; otherwise, unique points from
      //   lowest levels may not contribute (smolyak coeff = 0).
      // > Starting GSG from TPQ is conceptually straightforward but
      //   awkward in implementation (would need something like
      //   nond_sparse->ssg_driver->compute_tensor_grid()).
      metric = increment_sets(); // SSG only
      break;
    }

    converged = (metric <= convergenceTol || ++iter > max_iter);
    if (!converged)
      compute_print_iteration_results(false);
    Cout << "\nRefinement iteration convergence metric = " << metric << '\n';
  }

  // finalize refinement algorithms (if necessary)
  switch (refineControl) {
  case Pecos::DIMENSION_ADAPTIVE_CONTROL_GENERALIZED:
    bool converged_within_tol = (metric <= convergenceTol);
    finalize_sets(converged_within_tol); break;
  }
}


void NonDExpansion::increment_order()
{
  Cerr << "Error: virtual increment_order() not redefined by NonDExpansion "
       << "derived class.  This class foes not support uniform expansion order "
       << "increments." << std::endl;
  abort_handler(-1);
}


void NonDExpansion::update_expansion()
{
  // leave sampler_set, expansion flags, and distribution parameter settings
  // as set previously by compute_expansion(); there should be no need to
  // update these for an expansion refinement.

  // Ultimately want to be more incremental than this, but need to support
  // sgmgg usage for incremental grid updates following initial sgmg/sgmga
  // before this can be implemented.  For now, employ incremental rebuilds
  // only for hierarchical SC and rely on evaluation duplicate detection
  // within non-incremental builds from scratch.
  uSpaceModel.build_approximation();//.rebuild_approximation();
}


/** default implementation is overridden by PCE */
void NonDExpansion::increment_specification_sequence()
{
  NonDIntegration* nond_integration
    = (NonDIntegration*)uSpaceModel.subordinate_iterator().iterator_rep();
  nond_integration->increment_specification_sequence(); // TPQ or SSG
  /*
  bool multiple_num_int = (nond_integration->{order,level}_spec().size() > 1);
  switch (refineControl) {
  case NO_CONTROL:             // no refinement
    if (multiple_num_int)
      nond_integration->increment_specification_sequence(); // TPQ or SSG
    //else the initial spec has not been modified: no u_space_sampler.reset()
    break;
  case Pecos::UNIFORM_CONTROL: // uniform refinement
    if (multiple_num_int)
      nond_integration->increment_specification_sequence(); // TPQ or SSG
    // else carry final discrepancy refinement level as initial LF refinement
    break;
  default:                     // adaptive refinement
    break;
  }
  */
}


void NonDExpansion::initialize_sets()
{
  Cout << "\n>>>>> Initialization of generalized sparse grid sets.\n";
  NonDSparseGrid* nond_sparse
    = (NonDSparseGrid*)uSpaceModel.subordinate_iterator().iterator_rep();
  nond_sparse->initialize_sets();
}


Real NonDExpansion::increment_sets()
{
  Cout << "\n>>>>> Begin evaluation of active index sets.\n";

  NonDSparseGrid* nond_sparse
    = (NonDSparseGrid*)uSpaceModel.subordinate_iterator().iterator_rep();
  std::set<UShortArray>::const_iterator cit, cit_star;
  Real delta, delta_star = -1.;
  RealSymMatrix covar_ref, covar_star; RealVector stats_ref, stats_star;

  // store reference points for computing refinement metrics
  if (totalLevelRequests) stats_ref = finalStatistics.function_values();
  else if (covarianceControl == FULL_COVARIANCE) covar_ref = respCovariance;
  else stats_ref = respVariance;

  // Reevaluate the effect of every active set every time, since the reference
  // point for the surplus calculation changes (and the overlay should
  // eventually be inexpensive since each point set is only evaluated once).
  const std::set<UShortArray>& active_mi = nond_sparse->active_multi_index();
  for (cit=active_mi.begin(); cit!=active_mi.end(); ++cit) {

    // increment grid with current candidate
    Cout << "\n>>>>> Evaluating trial index set:\n" << *cit;
    nond_sparse->increment_set(*cit);
    if (uSpaceModel.restore_available()) {    // has been active previously
      nond_sparse->restore_set();
      uSpaceModel.restore_approximation();
    }
    else {                                    // a new active set
      nond_sparse->evaluate_set();
      uSpaceModel.append_approximation(true); // rebuild
    }

    // assess effect of increment (non-negative norm)
    delta = (totalLevelRequests) ? compute_final_statistics_metric()
                                 : compute_covariance_metric();
    // normalize effect of increment based on cost (# of collocation pts).
    // Note: increment size must be nonzero since growth restriction is
    // precluded for generalized sparse grids.
    delta /= nond_sparse->increment_size();
    // track best increment evaluated thus far
    if (delta > delta_star) {
      cit_star = cit; delta_star = delta;
      // partial results tracking avoids need to recompute statistics
      // on the selected index set
      if (outputLevel < DEBUG_OUTPUT) {
	if (totalLevelRequests) stats_star = finalStatistics.function_values();
	else if (covarianceControl == FULL_COVARIANCE)
	                        covar_star = respCovariance;
	else                    stats_star = respVariance;
      }
    }
    compute_print_increment_results();
    Cout << "\n<<<<< Trial set refinement metric = " << delta << '\n';

    // restore previous state (destruct order is reversed from construct order)
    uSpaceModel.pop_approximation(true); // store SDP set for use in restore
    nond_sparse->decrement_set();
    // restore reference point for next metric calculation
    if (totalLevelRequests) finalStatistics.function_values(stats_ref);
    else if (covarianceControl == FULL_COVARIANCE) respCovariance = covar_ref;
    else respVariance = stats_ref;
  }
  Cout << "\n<<<<< Evaluation of active index sets completed.\n"
       << "\n<<<<< Index set selection:\n" << *cit_star;

  // permanently apply best increment and update ref points for next increment
  nond_sparse->update_sets(*cit_star);
  uSpaceModel.restore_approximation();
  nond_sparse->update_reference();
  if (outputLevel < DEBUG_OUTPUT) { // partial results tracking
    if (totalLevelRequests) finalStatistics.function_values(stats_star);
    else if (covarianceControl == FULL_COVARIANCE) respCovariance = covar_star;
    else respVariance = stats_star;
  }

  return delta_star;
}


void NonDExpansion::finalize_sets(bool converged_within_tol)
{
  Cout << "\n<<<<< Finalization of generalized sparse grid sets.\n";
  NonDSparseGrid* nond_sparse
    = (NonDSparseGrid*)uSpaceModel.subordinate_iterator().iterator_rep();
  // apply all remaining increments not previously selected
  if (outputLevel >= VERBOSE_OUTPUT)
    nond_sparse->print_final_sets(converged_within_tol);
  nond_sparse->finalize_sets();
  uSpaceModel.finalize_approximation();
}


void NonDExpansion::compute_print_increment_results()
{
  switch (refineControl) {
  case Pecos::DIMENSION_ADAPTIVE_CONTROL_GENERALIZED:
    if (totalLevelRequests) { // both covariance and full results available
      if (outputLevel == DEBUG_OUTPUT) print_results(Cout);
      //else                           print_covariance(Cout);
    }
    else { // only covariance available, compute full results if needed
      if (outputLevel == DEBUG_OUTPUT)
	{ compute_statistics(); print_results(Cout); }
      else
	print_covariance(Cout);
    }
    break;
  }
}


void NonDExpansion::compute_print_iteration_results(bool initialize)
{
#ifdef CONVERGENCE_DATA
  // full results compute/print mirroring Iterator::post_run(),
  // allowing output level to be set low in performance testing
  compute_statistics();
  iteratedModel.print_evaluation_summary(Cout);
  NonDExpansion::print_results(Cout);
  if ( refineControl == Pecos::DIMENSION_ADAPTIVE_CONTROL_GENERALIZED &&
       ( expansionCoeffsApproach == Pecos::COMBINED_SPARSE_GRID ||
	 expansionCoeffsApproach == Pecos::HIERARCHICAL_SPARSE_GRID ) ) {
    NonDSparseGrid* nond_sparse
      = (NonDSparseGrid*)uSpaceModel.subordinate_iterator().iterator_rep();
    nond_sparse->print_smolyak_multi_index();
  }
#else
  switch (refineControl) {
  case Pecos::UNIFORM_CONTROL:
    // In these cases, metric calculations already performed are still valid
    if (outputLevel == DEBUG_OUTPUT) // compute/print all stats
      { compute_statistics(); print_results(Cout); }
    else // compute/print subset of stats required for convergence assessment
      { if (initialize) compute_covariance(); print_covariance(Cout); }
    break;
  case Pecos::DIMENSION_ADAPTIVE_CONTROL_SOBOL:
  case Pecos::DIMENSION_ADAPTIVE_CONTROL_DECAY:
    // In these cases, metric calculations already performed are still valid
    if (outputLevel == DEBUG_OUTPUT) // compute/print all stats
      { compute_statistics(); print_results(Cout); }
    else // compute/print subset of stats required for convergence assessment
      { if (initialize) compute_covariance(); print_covariance(Cout); }
    break;
  case Pecos::DIMENSION_ADAPTIVE_CONTROL_GENERALIZED:
    // In this case, the last index set calculation may not be the selected
    // index set. However, in the case where of non-debug output, we are using
    // partial results updating to eliminate the need to recompute stats for
    // the selected index set. This case also differs from other refinement
    // cases above in that compute_print_increment_results() has already
    // provided per increment output, so we do not push output if non-debug.
    if (totalLevelRequests) {
      //if (initialize || outputLevel == DEBUG_OUTPUT)
      //  { compute_statistics(); print_results(Cout); }
      if (initialize || outputLevel == DEBUG_OUTPUT) compute_statistics();
      if (outputLevel == DEBUG_OUTPUT)               print_results(Cout);
      // else no output!
    }
    else {
      if (outputLevel == DEBUG_OUTPUT)
	{ compute_statistics(); print_results(Cout); }
      else if (initialize)
	{ compute_covariance(); print_covariance(Cout); }
      // else no output!
    }
    break;
  }
#endif // CONVERGENCE_DATA
}


void NonDExpansion::compute_print_converged_results(bool print_override)
{
#ifdef CONVERGENCE_DATA
  // output fine-grained data on generalized index sets
  if ( refineControl == Pecos::DIMENSION_ADAPTIVE_CONTROL_GENERALIZED &&
       ( expansionCoeffsApproach == Pecos::COMBINED_SPARSE_GRID ||
	 expansionCoeffsApproach == Pecos::HIERARCHICAL_SPARSE_GRID ) ) {
    NonDSparseGrid* nond_sparse
      = (NonDSparseGrid*)uSpaceModel.subordinate_iterator().iterator_rep();
    nond_sparse->print_smolyak_multi_index();
  }
  // output spectral data for multifidelity UQ.  To get finer grain data,
  // activate DECAY_DEBUG in packages/pecos/src/OrthogPolyApproximation.cpp
  if (iteratedModel.surrogate_type() == "hierarchical" &&
      methodName == "nond_polynomial_chaos") {
    std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
    PecosApproximation* poly_approx_rep;
    for (size_t i=0; i<numFunctions; ++i) {
      poly_approx_rep = (PecosApproximation*)poly_approxs[i].approx_rep();
      Cout << "Variable decay rates for response function " << i+1 << ":\n"
	   << poly_approx_rep->dimension_decay_rates();
    }
  }
#endif // CONVERGENCE_DATA

  // if not already performed above, compute all stats
  switch (refineControl) {
  case Pecos::NO_CONTROL:
    compute_statistics();
    break;
  case Pecos::UNIFORM_CONTROL:
    if (outputLevel != DEBUG_OUTPUT)
      compute_statistics();
    break;
  case Pecos::DIMENSION_ADAPTIVE_CONTROL_SOBOL:
  case Pecos::DIMENSION_ADAPTIVE_CONTROL_DECAY:
    if (outputLevel != DEBUG_OUTPUT)
      compute_statistics();
    break;
  case Pecos::DIMENSION_ADAPTIVE_CONTROL_GENERALIZED:
    compute_statistics();
    break;
  }

  // For stand-alone executions, print_results occurs in Iterator::post_run().
  // For sub-iterator executions, stats are normally suppressed.
  if (print_override || (subIteratorFlag && outputLevel == DEBUG_OUTPUT))
    print_results(Cout);
}


/** computes the default refinement metric based on change in respCovariance */
Real NonDExpansion::compute_covariance_metric()
{
  // default implementation for use when direct (hierarchical) calculation
  // of increments is not available

  switch (covarianceControl) {
  case DIAGONAL_COVARIANCE: {
    RealVector delta_resp_var = respVariance; // deep copy
    compute_covariance();                     // update
    delta_resp_var -= respVariance;           // compute change
#ifdef DEBUG
    Cout << "resp_var_ref:\n"; write_data(Cout, resp_var_ref);
    Cout << "respVariance:\n"; write_data(Cout, respVariance);
    Cout << "norm of delta_resp_var = " << delta_resp_var.normFrobenius()
	 << std::endl;
#endif // DEBUG
    return delta_resp_var.normFrobenius();
    break;
  }
  case FULL_COVARIANCE: {
    RealSymMatrix delta_resp_covar = respCovariance; // deep copy
    compute_covariance();                            // update
    delta_resp_covar -= respCovariance;              // compute change
#ifdef DEBUG
    Cout << "resp_covar_ref:\n";
    write_data(Cout, resp_covar_ref, false, true, true);
    Cout << "respCovariance:\n";
    write_data(Cout, respCovariance, false, true, true);
    Cout << "norm of delta_resp_covar = " << delta_resp_covar.normFrobenius()
	 << std::endl;
#endif // DEBUG
    return delta_resp_covar.normFrobenius();
    break;
  }
  default: // NO_COVARIANCE or failure to redefine DEFAULT_COVARIANCE
    return 0.;
    break;
  }
}


/** computes a "goal-oriented" refinement metric employing finalStatistics */
Real NonDExpansion::compute_final_statistics_metric()
{
  // default implementation for use when direct (hierarchical) calculation
  // of increments is not available

  RealVector delta_final_stats = finalStatistics.function_values(); // deep copy
  compute_statistics();                                             //    update
  delta_final_stats -= finalStatistics.function_values();      // compute change

#ifdef DEBUG
  Cout << "final_stats_ref:\n" << final_stats_ref
       << "final_stats:\n" << finalStatistics.function_values()
       << "delta_final_stats:\n" << delta_final_stats << std::endl;
#endif // DEBUG

  // sum up only the level mapping stats (don't mix with mean and variance due
  // to scaling issues)
  // TO DO: if the level mappings are of mixed type, then would need to scale
  //        with a target value or measure norm of relative change.
  Real sum_sq = 0.; size_t i, j, cntr = 0, num_levels_i;
  for (i=0; i<numFunctions; ++i) {

    /* this modification can be used to mirror the metrics in Gerstner & Griebel
       2003.  However, their approach uses a hierarchical integral contribution
       evaluation which is less prone to roundoff (and is refined to very tight
       tolerances in the paper).
    sum_sq += std::pow(delta_final_stats[cntr], 2.); // mean
    cntr += 1 + requestedRespLevels[i].length() +
      requestedProbLevels[i].length() + requestedRelLevels[i].length() +
      requestedGenRelLevels[i].length();
    */

    // simple approach takes 2-norm of level mappings (no relative scaling),
    // which should be fine for mappings that are not of mixed type
    cntr += 2; // skip moments
    num_levels_i = requestedRespLevels[i].length() +
      requestedProbLevels[i].length() + requestedRelLevels[i].length() +
      requestedGenRelLevels[i].length();
    for (j=0; j<num_levels_i; ++j, ++cntr)
      sum_sq += std::pow(delta_final_stats[cntr], 2.);
  }

  return std::sqrt(sum_sq);
}


void NonDExpansion::reduce_total_sobol_sets(RealVector& avg_sobol)
{
  // anisotropy based on total Sobol indices (univariate effects only) averaged
  // over the response fn set.  [Addition of interaction effects based on
  // individual Sobol indices would require a nonlinear index set constraint
  // within anisotropic sparse grids.]

  if (numFunctions > 1) {
    if (avg_sobol.empty()) avg_sobol.size(numContinuousVars); // init to 0
    else                   avg_sobol = 0.;
  }

  size_t i;
  bool all_vars = (numContDesVars || numContEpistUncVars || numContStateVars);
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  PecosApproximation* poly_approx_rep;
  for (i=0; i<numFunctions; ++i) {
    poly_approx_rep = (PecosApproximation*)poly_approxs[i].approx_rep();

    // InterpPolyApproximation::compute_*_effects() assumes moments are
    // available and compute_statistics() has not been called if !DEBUG_OUTPUT
    if (outputLevel < DEBUG_OUTPUT) {
      if (all_vars) poly_approx_rep->compute_moments(initialPtU);
      else          poly_approx_rep->compute_moments();
    }

    if (!vbdOrderLimit) // no order limit --> component used within total
      poly_approx_rep->compute_component_effects();
    poly_approx_rep->compute_total_effects(); // from scratch or using component
    // Note: response functions for which negligible variance is detected have
    // their totalSobolIndices assigned to zero.  This avoid corrupting the
    // aggregation, although the scaling that follows could be improved to
    // divide by the number of nonzero contributions (avg_sobol is currently
    // used only in a relative sense, so this is low priority).
    if (numFunctions > 1) avg_sobol += poly_approx_rep->total_sobol_indices();
    else                  avg_sobol  = poly_approx_rep->total_sobol_indices();
  }
  if (numFunctions > 1)
    avg_sobol.scale(1./(Real)numFunctions);

  // manage small values that are not 0 (SGMGA has special handling for 0)
  Real pref_tol = 1.e-2; // TO DO
  for (i=0; i<numContinuousVars; ++i)
    if (std::abs(avg_sobol[i]) < pref_tol)
      avg_sobol[i] = 0.;
#ifdef DEBUG
  Cout << "avg_sobol truncated at " << pref_tol << ":\n";
  write_data(Cout, avg_sobol);
#endif // DEBUG
}


void NonDExpansion::reduce_decay_rate_sets(RealVector& min_decay)
{
  // anisotropy based on linear approximation to coefficient decay rates for
  // each dimension as measured from univariate PCE terms.  In this case,
  // averaging tends to wash out the interesting anisotropy, especially if
  // some functions converge quickly with high rates.  Thus, it is more
  // appropriate to extract the minimum decay rates over the response fn set.

  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  PecosApproximation* poly_approx_rep
    = (PecosApproximation*)poly_approxs[0].approx_rep();
  min_decay = poly_approx_rep->dimension_decay_rates();
  size_t i, j;
  for (i=1; i<numFunctions; ++i) {
    poly_approx_rep = (PecosApproximation*)poly_approxs[i].approx_rep();
    const RealVector& decay_i = poly_approx_rep->dimension_decay_rates();
    for (j=0; j<numContinuousVars; ++j)
      if (decay_i[j] < min_decay[j])
	min_decay[j] = decay_i[j];
  }
  // enforce a lower bound on minimum decay (disallow negative/zero decay rates)
  Real decay_tol = 1.e-2; // TO DO
  for (j=0; j<numContinuousVars; ++j)
    if (min_decay[j] < decay_tol)
      min_decay[j] = decay_tol;

#ifdef DEBUG
  Cout << "min_decay:\n"; write_data(Cout, min_decay);
#endif // DEBUG
}


void NonDExpansion::compute_covariance()
{
  if (covarianceControl >= DIAGONAL_COVARIANCE)
    compute_diagonal_variance();
  if (numFunctions > 1 && covarianceControl == FULL_COVARIANCE)
    compute_off_diagonal_covariance();
}


void NonDExpansion::compute_diagonal_variance()
{
  bool warn_flag = false,
    all_vars = (numContDesVars || numContEpistUncVars || numContStateVars);
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  for (size_t i=0; i<numFunctions; ++i) {
    Real& var_i = (covarianceControl == DIAGONAL_COVARIANCE) ?
      respVariance[i] : respCovariance(i,i);
    PecosApproximation* poly_approx_rep_i
      = (PecosApproximation*)poly_approxs[i].approx_rep();
    if (poly_approx_rep_i->expansion_coefficient_flag())
      var_i = (all_vars) ? poly_approx_rep_i->variance(initialPtU) :
	                   poly_approx_rep_i->variance();
    else
      { warn_flag = true; var_i = 0.; }
  }
  if (warn_flag)
    Cerr << "Warning: expansion coefficients unavailable in NonDExpansion::"
	 << "compute_diagonal_variance().\n         Zeroing affected variance "
	 << "terms." << std::endl;
}


void NonDExpansion::compute_off_diagonal_covariance()
{
  size_t i, j;
  bool warn_flag = false,
    all_vars = (numContDesVars || numContEpistUncVars || numContStateVars);
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  for (i=0; i<numFunctions; ++i) {
    PecosApproximation* poly_approx_rep_i
      = (PecosApproximation*)poly_approxs[i].approx_rep();
    if (poly_approx_rep_i->expansion_coefficient_flag())
      for (j=0; j<i; ++j) {
	PecosApproximation* poly_approx_rep_j
	  = (PecosApproximation*)poly_approxs[j].approx_rep();
	if (poly_approx_rep_j->expansion_coefficient_flag())
	  respCovariance(i,j) = (all_vars) ?
	    poly_approx_rep_i->covariance(initialPtU, poly_approx_rep_j) :
	    poly_approx_rep_i->covariance(poly_approx_rep_j);
	else
	  { warn_flag = true; respCovariance(i,j) = 0.; }
      }
    else {
      warn_flag = true;
      for (j=0; j<i; ++j)
	respCovariance(i,j) = 0.;
    }
  }
  if (warn_flag)
    Cerr << "Warning: expansion coefficients unavailable in NonDExpansion::"
	 << "compute_off_diagonal_covariance().\n         Zeroing affected "
	 << "covariance terms." << std::endl;
}


/** Calculate analytic and numerical statistics from the expansion and
    log results within final_stats for use in OUU. */
void NonDExpansion::compute_statistics()
{
  const ShortArray& final_asv = finalStatistics.active_set_request_vector();
  const SizetArray& final_dvv = finalStatistics.active_set_derivative_vector();
  bool all_vars = (numContDesVars || numContEpistUncVars || numContStateVars);
  size_t i, j, k, rl_len, pl_len, bl_len, gl_len, cntr = 0, sampler_cntr = 0,
    num_final_grad_vars = final_dvv.size();

  // initialize computed*Levels and expGradsMeanX
  if (totalLevelRequests)
    initialize_distribution_mappings();
  if (!subIteratorFlag && outputLevel >= NORMAL_OUTPUT && expGradsMeanX.empty())
    expGradsMeanX.shapeUninitialized(numContinuousVars, numFunctions);

  // -----------------------------
  // Calculate analytic statistics
  // -----------------------------

  // loop over response fns and compute/store analytic stats/stat grads
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  Real mu, sigma, beta, gen_beta, z;
  RealVector mu_grad, sigma_grad, final_stat_grad;
  PecosApproximation* poly_approx_rep;
  for (i=0; i<numFunctions; ++i) {
    if (totalLevelRequests) {
      rl_len = requestedRespLevels[i].length();
      pl_len = requestedProbLevels[i].length();
      bl_len = requestedRelLevels[i].length();
      gl_len = requestedGenRelLevels[i].length();
    }
    else
      rl_len = pl_len = bl_len = gl_len = 0;

    poly_approx_rep = (PecosApproximation*)poly_approxs[i].approx_rep();

    if (poly_approx_rep->expansion_coefficient_flag()) {
      if (all_vars) poly_approx_rep->compute_moments(initialPtU);
      else          poly_approx_rep->compute_moments();

      const RealVector& moments = poly_approx_rep->moments(); // virtual
      // Pecos provides central moments
      mu = moments[0]; const Real& var = moments[1];
      if (covarianceControl ==  DIAGONAL_COVARIANCE) respVariance[i]     = var;
      else if (covarianceControl == FULL_COVARIANCE) respCovariance(i,i) = var;
      if (var >= 0.)
	sigma = std::sqrt(var);
      else { // negative variance can happen with SC on sparse grids
	Cerr << "Warning: stochastic expansion variance is negative in "
	     << "computation of std deviation.\n         Setting std "
	     << "deviation to zero." << std::endl;
	sigma = 0.;
      }
    }

    // compute moment gradients if needed for beta mappings
    bool moment_grad_mapping_flag = false;
    if (respLevelTarget == RELIABILITIES)
      for (j=0; j<rl_len; ++j) // dbeta/ds requires mu,sigma,dmu/ds,dsigma/ds
	if (final_asv[cntr+2+j] & 2)
	  { moment_grad_mapping_flag = true; break; }
    if (!moment_grad_mapping_flag)
      for (j=0; j<bl_len; ++j)   // dz/ds requires dmu/ds,dsigma/ds
	if (final_asv[cntr+2+pl_len+j] & 2)
	  { moment_grad_mapping_flag = true; break; }

    // *** mean
    if (final_asv[cntr] & 1)
      finalStatistics.function_value(mu, cntr);
    // *** mean gradient
    if (final_asv[cntr] & 2 || moment_grad_mapping_flag) {
      const RealVector& grad = (all_vars) ?
	poly_approx_rep->mean_gradient(initialPtU, final_dvv) :
	poly_approx_rep->mean_gradient();
      if (final_asv[cntr] & 2)
	finalStatistics.function_gradient(grad, cntr);
      if (moment_grad_mapping_flag)
	mu_grad = grad; // transfer to code below
    }
    ++cntr;

    // *** std deviation
    if (final_asv[cntr] & 1)
      finalStatistics.function_value(sigma, cntr);
    // *** std deviation gradient
    if (final_asv[cntr] & 2 || moment_grad_mapping_flag) {
      sigma_grad = (all_vars) ?
	poly_approx_rep->variance_gradient(initialPtU, final_dvv) :
	poly_approx_rep->variance_gradient();
      if (sigma > 0.)
	for (j=0; j<num_final_grad_vars; ++j)
	  sigma_grad[j] /= 2.*sigma;
      else {
	Cerr << "Warning: stochastic expansion std deviation is zero in "
	     << "computation of std deviation gradient.\n         Setting "
	     << "gradient to zero." << std::endl;
	sigma_grad = 0.;
      }
      if (final_asv[cntr] & 2)
	finalStatistics.function_gradient(sigma_grad, cntr);
    }
    ++cntr;

    if (respLevelTarget == RELIABILITIES) {
      for (j=0; j<rl_len; ++j, ++cntr) {
	// *** beta
	if (final_asv[cntr] & 1) {
	  const Real& z_bar = requestedRespLevels[i][j];
	  if (sigma > Pecos::SMALL_NUMBER) {
	    Real ratio = (mu - z_bar)/sigma;
	    computedRelLevels[i][j] = beta = (cdfFlag) ? ratio : -ratio;
	  }
	  else
	    computedRelLevels[i][j] = beta =
	      ( (cdfFlag && mu <= z_bar) || (!cdfFlag && mu > z_bar) ) ?
	      -Pecos::LARGE_NUMBER : Pecos::LARGE_NUMBER;
	  finalStatistics.function_value(beta, cntr);
	}
	// *** beta gradient
	if (final_asv[cntr] & 2) {
	  if (final_stat_grad.empty())
	    final_stat_grad.sizeUninitialized(num_final_grad_vars);
	  if (sigma > Pecos::SMALL_NUMBER) {
	    const Real& z_bar = requestedRespLevels[i][j];
	    for (k=0; k<num_final_grad_vars; ++k) {
	      Real dratio_dx = (sigma*mu_grad[k] - (mu - z_bar)*
				sigma_grad[k]) / std::pow(sigma, 2);
	      final_stat_grad[k] = (cdfFlag) ? dratio_dx : -dratio_dx;
	    }
	  }
	  else
	    final_stat_grad = 0.;
	  finalStatistics.function_gradient(final_stat_grad, cntr);
	}
      }
    }
    else
      cntr += rl_len;

    // no analytic mappings for probability levels
    cntr += pl_len;

    for (j=0; j<bl_len; ++j, ++cntr) {
      const Real& beta_bar = requestedRelLevels[i][j];
      if (final_asv[cntr] & 1) {
	// *** z
	computedRespLevels[i][j+pl_len] = z = (cdfFlag) ?
	  mu - beta_bar * sigma : mu + beta_bar * sigma;
	finalStatistics.function_value(z, cntr);
      }
      if (final_asv[cntr] & 2) {
	// *** z gradient
	if (final_stat_grad.empty())
	  final_stat_grad.sizeUninitialized(num_final_grad_vars);
	for (k=0; k<num_final_grad_vars; ++k)
	  final_stat_grad[k] = (cdfFlag) ?
	    mu_grad[k] - beta_bar * sigma_grad[k] :
	    mu_grad[k] + beta_bar * sigma_grad[k];
	finalStatistics.function_gradient(final_stat_grad, cntr);
      }
    }

    // no analytic mappings for generalized reliability levels
    cntr += gl_len;
 
    // *** local sensitivities
    if (!subIteratorFlag && outputLevel >= NORMAL_OUTPUT &&
	poly_approx_rep->expansion_coefficient_flag()) {
      // expansion sensitivities are defined from the coefficients and basis
      // polynomial derivatives.  They are computed for the means of the
      // uncertain varables and are intended to serve as importance factors.
      uSpaceModel.continuous_variables(initialPtU);
      const RealVector& exp_grad_u_rv
	= poly_approxs[i].gradient(uSpaceModel.current_variables());
      RealVector exp_grad_u_pv, exp_grad_x_pv;
      copy_data(exp_grad_u_rv, exp_grad_u_pv);
      SizetMultiArrayConstView cv_ids = iteratedModel.continuous_variable_ids();
      SizetArray x_dvv; copy_data(cv_ids, x_dvv);
      natafTransform.trans_grad_U_to_X(exp_grad_u_pv, exp_grad_x_pv,
				       natafTransform.x_means(), x_dvv, cv_ids);
      Teuchos::setCol(exp_grad_x_pv, (int)i, expGradsMeanX);
    }

    // *** global sensitivities:
    if (vbdFlag && poly_approx_rep->expansion_coefficient_flag()) {
      poly_approx_rep->compute_component_effects(); // main or main+interaction
      poly_approx_rep->compute_total_effects();     // total
    }
  }
  if (numFunctions > 1 && covarianceControl == FULL_COVARIANCE)
    compute_off_diagonal_covariance(); // diagonal entries were filled in above

  // ------------------------------
  // Calculate numerical statistics
  // ------------------------------

  // Estimate CDF/CCDF statistics by sampling on the expansion
  if (expSampling) {
    NonDSampling* exp_sampler_rep
      = (NonDSampling*)expansionSampler.iterator_rep();

    // pass x-space data so that u-space Models can perform inverse transforms
    exp_sampler_rep->initialize_random_variables(natafTransform);
    // since expansionSampler uses an ALEATORY_UNCERTAIN sampling mode,
    // we must set the unsampled variables to their u-space values.
    if (numContDesVars || numContEpistUncVars || numContStateVars)
      uSpaceModel.continuous_variables(initialPtU);

    // response fn is active for z->p, z->beta*, p->z, or beta*->z
    ShortArray sampler_asv(numFunctions, 0);
    cntr = 0;
    for (i=0; i<numFunctions; ++i) {
      cntr += 2;
      size_t rl_len = requestedRespLevels[i].length();
      if (respLevelTarget != RELIABILITIES)
	for (j=0; j<rl_len; ++j)
	  if (final_asv[cntr+j] & 1)
	    { sampler_asv[i] |= 1; break; }
      cntr += rl_len;
      size_t pl_len = requestedProbLevels[i].length();
      for (j=0; j<pl_len; ++j)
	if (final_asv[cntr+j] & 1)
	  { sampler_asv[i] |= 1; break; }
      cntr += pl_len + requestedRelLevels[i].length();
      size_t gl_len = requestedGenRelLevels[i].length();
      for (j=0; j<gl_len; ++j)
	if (final_asv[cntr+j] & 1)
	  { sampler_asv[i] |= 1; break; }
      cntr += gl_len;
    }
    ActiveSet sampler_set = expansionSampler.active_set(); // copy
    sampler_set.request_vector(sampler_asv);
    expansionSampler.active_set(sampler_set);

    // no summary output since on-the-fly constructed:
    expansionSampler.run_iterator(Cout);
    exp_sampler_rep->
      compute_distribution_mappings(expansionSampler.all_responses());
    exp_sampler_rep->update_final_statistics();
    const RealVector& exp_sampler_stats
      = expansionSampler.response_results().function_values();
 
    // Update probability estimates with importance sampling, if requested.
    RealVectorArray imp_sampler_stats;
    if (impSampling) {
      NonDAdaptImpSampling* imp_sampler_rep
	= (NonDAdaptImpSampling*)importanceSampler.iterator_rep();

      //imp_sampler_rep->initialize_random_variables(natafTransform);
      // since importanceSampler uses an ALEATORY_UNCERTAIN sampling mode,
      // we must set the unsampled variables to their u-space values.
      //if (numContDesVars || numContEpistUncVars || numContStateVars)
      //  uSpaceModel.continuous_variables(initialPtU);
      // response fn is active for z->p, z->beta*, p->z, or beta*->z
      //ActiveSet sampler_set = importanceSampler.active_set(); // copy
      //ShortArray sampler_asv(numFunctions, 0);

      const RealMatrix& exp_vars = expansionSampler.all_samples();
      //const IntResponseMap& exp_responses = expansionSampler.all_responses();
      int exp_cv = exp_vars.numRows();

      sampler_cntr = 0;
      imp_sampler_stats.resize(numFunctions);
      for (i=0; i<numFunctions; ++i) {
	sampler_cntr += 2;
	size_t rl_len = requestedRespLevels[i].length();
	if (rl_len && respLevelTarget != RELIABILITIES) {
	  imp_sampler_stats[i].resize(rl_len);
	  // Currently initializing importance sampling with both 
	  // original build points and LHS expansion sampler points
	  const Pecos::SurrogateData& exp_data
	    = uSpaceModel.approximation_data(i);
	  size_t m, num_data_pts = exp_data.size(),
	    num_to_is = numSamplesOnExpansion + num_data_pts;
	  RealVectorArray initial_points(num_to_is);
	  for (m=0; m<num_data_pts; ++m)
	    initial_points[m] = exp_data.continuous_variables(m); // view OK
	  for (m=0; m<numSamplesOnExpansion; ++m)
	    copy_data(exp_vars[m], exp_cv, initial_points[m+num_data_pts]);
	  for (j=0; j<rl_len; ++j, ++sampler_cntr) {
            //Cout << "Initial estimate of p to seed "
	    //     << exp_sampler_stats[sampler_cntr] << '\n';
	    imp_sampler_rep->initialize(initial_points, i, 
	      exp_sampler_stats[sampler_cntr], requestedRespLevels[i][j]);
          
	    // no summary output since on-the-fly constructed:
            importanceSampler.run_iterator(Cout);

            //Real p = imp_sampler_rep->final_probability();
            //Cout << "importance sampling estimate for function " << i 
            //     << " level " << j << " = " << p << "\n";
	    imp_sampler_stats[i][j] = imp_sampler_rep->final_probability();
	  }
	}
	// sampler_cntr offset by moments, rl_len for p, pl_len, and gl_len
	sampler_cntr += requestedProbLevels[i].length()
	             +  requestedGenRelLevels[i].length();
      }
    }
 
    // Update finalStatistics from {exp,imp}_sampler_stats.  Moment mappings
    // are not recomputed since empty level arrays are passed in construct_
    // expansion_sampler() and these levels are omitting from sampler_cntr.
    archive_allocate_mappings();
    cntr = sampler_cntr = 0;
    for (i=0; i<numFunctions; ++i) {
      cntr += 2; sampler_cntr += 2;

      if (respLevelTarget == RELIABILITIES)
	cntr += requestedRespLevels[i].length();
      else {
	size_t rl_len = requestedRespLevels[i].length();
	for (j=0; j<rl_len; ++j, ++cntr, ++sampler_cntr) {
	  if (final_asv[cntr] & 1) {
	    const Real& p = (impSampling) ? imp_sampler_stats[i][j]
	                                  : exp_sampler_stats[sampler_cntr];
	    if (respLevelTarget == PROBABILITIES) {
	      computedProbLevels[i][j] = p;
	      finalStatistics.function_value(p, cntr);
	    }
	    else if (respLevelTarget == GEN_RELIABILITIES) {
	      computedGenRelLevels[i][j] = gen_beta = -Pecos::Phi_inverse(p);
	      finalStatistics.function_value(gen_beta, cntr);
	    }
	  }
	  if (final_asv[cntr] & 2) { // TO DO: sampling sensitivity analysis
	    Cerr << "\nError: analytic sensitivity of response ";
	    if (respLevelTarget == PROBABILITIES)
	      Cerr << "probability";
	    else if (respLevelTarget == GEN_RELIABILITIES)
	      Cerr << "generalized reliability";
	    Cerr << " not yet supported." << std::endl;
	    abort_handler(-1);
	  }
	}
      }

      size_t pl_len = requestedProbLevels[i].length();
      for (j=0; j<pl_len; ++j, ++cntr, ++sampler_cntr) {
	if (final_asv[cntr] & 1) {
	  computedRespLevels[i][j] = z = exp_sampler_stats[sampler_cntr];
	  finalStatistics.function_value(z, cntr);
	}
	if (final_asv[cntr] & 2) { // TO DO: p->z sampling sensitivity analysis
	  Cerr << "\nError: analytic sensitivity of response level not yet "
	       << "supported for mapping from probability." << std::endl;
	  abort_handler(-1);
	}
      }

      size_t bl_len = requestedRelLevels[i].length();
      cntr += bl_len;

      size_t gl_len = requestedGenRelLevels[i].length();
      for (j=0; j<gl_len; ++j, ++cntr, ++sampler_cntr) {
	if (final_asv[cntr] & 1) {
	  computedRespLevels[i][j+pl_len+bl_len] = z
	    = exp_sampler_stats[sampler_cntr];
	  finalStatistics.function_value(z, cntr);
	}
	if (final_asv[cntr] & 2) { // TO DO: beta*->p->z sampling SA
	  Cerr << "\nError: analytic sensitivity of response level not yet "
	       << "supported for mapping from generalized reliability."
	       << std::endl;
	  abort_handler(-1);
	}
      }
      
      // archive the mappings from response levels
      archive_from_resp(i); 
      // archive the mappings to response levels
      archive_to_resp(i); 
 
    }
  }

  // archive the active variables with the results
  if (resultsDB.active()) {
    resultsDB.insert(run_identifier(), resultsNames.cv_labels, 
		     iteratedModel.continuous_variable_labels());
    resultsDB.insert(run_identifier(), resultsNames.fn_labels, 
		     iteratedModel.response_labels());
  }
  archive_moments();
  archive_coefficients();
}


void NonDExpansion::archive_moments()
{
  if (!resultsDB.active())
    return;

  // for now, archive only central moments to avoid duplicating all above logic
  // insert NaN for missing data
  bool exp_active = false, num_active = false;
  RealMatrix exp_matrix(4, numFunctions), num_matrix(4, numFunctions);
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  PecosApproximation* poly_approx_rep;
  for (size_t i=0; i<numFunctions; ++i) {
    poly_approx_rep = (PecosApproximation*)poly_approxs[i].approx_rep();
    if (poly_approx_rep && poly_approx_rep->expansion_coefficient_flag()) {
      // Pecos provides central moments
      const RealVector& exp_moments = poly_approx_rep->expansion_moments();
      const RealVector& num_moments = poly_approx_rep->numerical_moments();
      size_t exp_mom = exp_moments.length(), num_mom = num_moments.length();
      if (exp_mom)  exp_active = true;
      if (num_mom)  num_active = true;
      for (size_t j=0; j<exp_mom; ++j)
	exp_matrix(j,i) = exp_moments[j];
      for (size_t j=exp_mom; j<4; ++j)
	exp_matrix(j,i) = std::numeric_limits<Real>::quiet_NaN();
      for (size_t j=0; j<num_mom; ++j)
	num_matrix(j,i) = num_moments[j];
      for (size_t j=num_mom; j<4; ++j)
	num_matrix(j,i) = std::numeric_limits<Real>::quiet_NaN();
    }
  }

  if (exp_active) {
    MetaDataType md_moments; 
    md_moments["Row Labels"] = 
      make_metadatavalue("Mean", "Variance", "3rdCentral", "4thCentral"); 
    md_moments["Column Labels"] = make_metadatavalue(iteratedModel.response_labels()); 
    resultsDB.insert(run_identifier(), resultsNames.moments_central_exp, 
		     exp_matrix, md_moments); 
  }
  if (num_active) {
    MetaDataType md_moments; 
    md_moments["Row Labels"] = 
      make_metadatavalue("Mean", "Variance", "3rdCentral", "4thCentral"); 
    md_moments["Column Labels"] = make_metadatavalue(iteratedModel.response_labels()); 
    resultsDB.insert(run_identifier(), resultsNames.moments_central_num, 
		     num_matrix, md_moments); 
  }
}

void NonDExpansion::update_final_statistics_gradients()
{
  if (finalStatistics.function_gradients().empty())
    return;

  // Augmented design vars:
  // > All vars: transform dg/du to dg/dx -> provides desired dg/ds for x = s
  // > Distinct vars: PCE/SC approximations for dg/ds are formed
  // Inserted design vars:
  // > All and Distinct views for the subIterator are equivalent
  // > PCE/SC approximations for dg/ds are formed
  // > Alternative: All view could force an artificial cdv augmentation
  // Mixed augmented/inserted design vars:
  // > All vars: bookkeep the two dg/ds approaches
  // > Distinct vars: PCE/SC approximations for dg/ds are formed

  // for all_variables, finalStatistics design grads are in extended u-space
  // -> transform to the original design space
  if (numContDesVars || numContEpistUncVars || numContStateVars) {
    // this approach is more efficient but less general.  If we can assume
    // that the DVV only contains design/state vars, then we know they are
    // uncorrelated and the jacobian matrix is diagonal with terms 2./range.
    SizetMultiArrayConstView cv_ids = iteratedModel.continuous_variable_ids();
    const SizetArray& final_dvv
      = finalStatistics.active_set_derivative_vector();
    size_t num_final_grad_vars = final_dvv.size();
    Real factor;
    const Pecos::RealVector& x_l_bnds = natafTransform.x_lower_bounds();
    const Pecos::RealVector& x_u_bnds = natafTransform.x_upper_bounds();

    RealMatrix final_stat_grads = finalStatistics.function_gradients_view();
    int num_final_stats = final_stat_grads.numCols();
    for (size_t j=0; j<num_final_grad_vars; ++j) {
      size_t deriv_j = find_index(cv_ids, final_dvv[j]); //final_dvv[j]-1;
      if ( deriv_j <  numContDesVars ||
	   deriv_j >= numContDesVars+numContAleatUncVars ) {
	// augmented design variable sensitivity
	factor = 2. / (x_u_bnds(deriv_j) - x_l_bnds(deriv_j));
	for (size_t i=0; i<num_final_stats; ++i)
	  final_stat_grads(j,i) *= factor; // see jacobian_dZ_dX()
      }
      // else inserted design variable sensitivity: no scaling required
    }
      
    // This approach is more general, but is overkill for this purpose
    // and incurs additional copying overhead.
    /*
    RealVector initial_pt_x_pv, fn_grad_u, fn_grad_x;
    copy_data(initial_pt_x, initial_pt_x_pv);
    RealMatrix jacobian_ux;
    natafTransform.jacobian_dU_dX(initial_pt_x_pv, jacobian_ux);
    RealBaseVector final_stat_grad;
    for (i=0; i<num_final_stats; ++i) {
      copy_data(finalStatistics.function_gradient_view(i), fn_grad_u);
      natafTransform.trans_grad_U_to_X(fn_grad_u, fn_grad_x, jacobian_ux,
	                               final_dvv);
      copy_data(fn_grad_x, final_stat_grad);
      finalStatistics.function_gradient(final_stat_grad, i)
    }
    */
  }

  // For distinct vars, nothing additional is needed since u_space_sampler
  // has been configured to compute dg/ds at each of the sample points (these
  // are already in user-space and are not part of the variable transform).
  // uSpaceModel.build_approximation() -> PecosApproximation::build()
  // then constructs PCE/SC approximations of these gradients, and
  // PecosApproximation::<mean,variance>_gradient()
  // are used above to generate dmu/ds, dsigma/ds, and dbeta/ds.
}


void NonDExpansion::print_moments(std::ostream& s)
{
  s << std::scientific << std::setprecision(write_precision);

  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  const StringArray& fn_labels = iteratedModel.response_labels();
  size_t i, j, width = write_precision+7;

  s << "\nMoment-based statistics for each response function:\n";

  // Handle cases of both expansion/numerical moments or only one or the other:
  //   both exp/num: SC and PCE with numerical integration
  //   exp only:     PCE with unstructured grids (regression, exp sampling)
  // Also handle numerical exception of negative variance in either exp or num
  PecosApproximation* poly_approx_rep;
  size_t exp_mom, num_mom; bool exception = false, prev_exception = false;
  RealVector std_exp_moments, std_num_moments;
  for (i=0; i<numFunctions; ++i) {
    poly_approx_rep = (PecosApproximation*)poly_approxs[i].approx_rep();
    if (poly_approx_rep && poly_approx_rep->expansion_coefficient_flag()) {
      // Pecos provides central moments
      const RealVector& exp_moments = poly_approx_rep->expansion_moments();
      const RealVector& num_moments = poly_approx_rep->numerical_moments();
      exp_mom = exp_moments.length(); num_mom = num_moments.length();
      if ( (exp_mom == 2 && exp_moments[1] <  0.) ||
	   (num_mom == 2 && num_moments[1] <  0.) ||
	   (exp_mom >  2 && exp_moments[1] <= 0.) ||
	   (num_mom >  2 && num_moments[1] <= 0.) ) {
	if (i==0 || !prev_exception)
	  s << std::setw(width+15) << "Mean" << std::setw(width+1) << "Variance"
	    << std::setw(width+1)  << "3rdCentral" << std::setw(width+2)
	    << "4thCentral\n";
	if (exp_mom && num_mom) s << fn_labels[i];
	else                    s << std::setw(14) << fn_labels[i];
	if (exp_mom) {
	  if (num_mom)          s << '\n' << std::setw(14) << "expansion:  ";
	  for (j=0; j<exp_mom; ++j)
	    s << ' ' << std::setw(width) << exp_moments[j];
	}
	if (num_mom) {
	  if (exp_mom)          s << '\n' << std::setw(14) << "numerical:  ";
	  for (j=0; j<num_mom; ++j)
	    s << ' ' << std::setw(width) << num_moments[j];
	}
	exception = prev_exception = true;
      }
      else {
	if (i==0 || prev_exception)
	  s << std::setw(width+15) << "Mean" << std::setw(width+1) << "Std Dev"
	    << std::setw(width+1)  << "Skewness"
	    << std::setw(width+2)  << "Kurtosis\n";
	if (exp_mom && num_mom) s << fn_labels[i];
	else                    s << std::setw(14) << fn_labels[i];
	if (exp_mom) {
	  poly_approx_rep->standardize_moments(exp_moments, std_exp_moments);
	  if (num_mom)          s << '\n' << std::setw(14) << "expansion:  ";
	  for (j=0; j<exp_mom; ++j)
	    s << ' ' << std::setw(width) << std_exp_moments[j];
	}
	if (num_mom) {
	  poly_approx_rep->standardize_moments(num_moments, std_num_moments);
	  if (exp_mom)          s << '\n' << std::setw(14) << "numerical:  ";
	  for (j=0; j<num_mom; ++j)
	    s << ' ' << std::setw(width) << std_num_moments[j];
	}
	prev_exception = false;
      }
      s << '\n';

      /* COV has been removed:
      if (std::abs(mean) > Pecos::SMALL_NUMBER)
        s << "  " << std::setw(width)   << std_dev/mean << '\n';
      else
        s << "  " << std::setw(width+1) << "Undefined\n";
      */
    }
  }
  if (exception)
    s << "\nNote: due to non-positive variance (resulting from under-resolved "
      << "numerical integration),\n      standardized moments have been "
      << "replaced with central moments for at least one response.\n";
}


void NonDExpansion::print_covariance(std::ostream& s)
{
  switch (covarianceControl) {
  case DIAGONAL_COVARIANCE:
    if (!respVariance.empty()) {
      s << "\nVariance vector for response functions:\n";
      write_col_vector_trans(s, 0, true, true, true, respVariance);
    }
    break;
  case FULL_COVARIANCE:
    if (!respCovariance.empty()) {
      s << "\nCovariance matrix for response functions:\n";
      write_data(s, respCovariance, true, true, true);
    }
    break;
  }
}


void NonDExpansion::print_sobol_indices(std::ostream& s)
{
  // effects are computed per resp fn within compute_statistics()
  // if vbdFlag and expansion_coefficient_flag

  // this fn called if vbdFlag and prints per resp fn if
  // expansion_coefficient_flag and non-negligible variance
 
  s << "\nGlobal sensitivity indices for each response function:\n";

  const StringArray& fn_labels = iteratedModel.response_labels();
  StringMultiArrayConstView cv_labels
    = iteratedModel.continuous_variable_labels();

  // construct labels corresponding to (aggregated) sobol index map
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  PecosApproximation* poly_approx_rep; StringArray sobol_labels;
  size_t i, j, num_indices;
  if (vbdOrderLimit != 1) { // unlimited (0) or includes interactions (>1)
    // create aggregate interaction labels (once for all response fns)
    SharedPecosApproxData* shared_data_rep = (SharedPecosApproxData*)
      uSpaceModel.shared_approximation().data_rep();
    const Pecos::BitArrayULongMap& s_index_map
      = shared_data_rep->sobol_index_map();
    sobol_labels.resize(s_index_map.size());
    for (Pecos::BAULMCIter map_cit=s_index_map.begin();
	 map_cit!=s_index_map.end(); ++map_cit) { // loop in key sorted order
      const BitArray& set = map_cit->first;
      unsigned long index = map_cit->second; // 0-way -> n 1-way -> interaction
      if (index > numContinuousVars) {       // an interaction
	String& label = sobol_labels[index]; // store in index order
	for (j=0; j<numContinuousVars; ++j)
	  if (set[j])
	    label += cv_labels[j] + " ";
      }
    }
  }

  // print sobol indices per response function
  // TO DO: manage sparsity --> proper usage of aggregated labels
  for (i=0; i<numFunctions; ++i) {
    poly_approx_rep = (PecosApproximation*)poly_approxs[i].approx_rep();
    if (poly_approx_rep->expansion_coefficient_flag()) {
      // Note: vbdFlag can be defined for covarianceControl == NO_COVARIANCE.
      // In this case, we cannot screen effectively at this level.
      bool well_posed = ( ( covarianceControl   == DIAGONAL_COVARIANCE &&
			    respVariance[i]     <= Pecos::SMALL_NUMBER ) ||
			  ( covarianceControl   == FULL_COVARIANCE &&
			    respCovariance(i,i) <= Pecos::SMALL_NUMBER ) )
	              ? false : true;
      if (well_posed) {
	const RealVector& sobol_indices = poly_approx_rep->sobol_indices();
	const RealVector& total_indices
	  = poly_approx_rep->total_sobol_indices();
	s << fn_labels[i] << " Sobol' indices:\n" << std::setw(38) << "Main"
	  << std::setw(19) << "Total\n";
	for (j=0; j<numContinuousVars; ++j)
	  if (std::abs(sobol_indices[j+1]) > vbdDropTol ||
	      std::abs(total_indices[j])   > vbdDropTol)   // print main / total
	    s << "                     "   << std::setw(write_precision+7) 
	      << sobol_indices[j+1] << ' ' << std::setw(write_precision+7)
	      << total_indices[j]   << ' ' << cv_labels[j] << '\n';
	if (vbdOrderLimit != 1) { // unlimited (0) or includes interactions (>1)
	  num_indices = sobol_indices.length();
	  s << std::setw(39) << "Interaction\n";
	  for (j=numContinuousVars+1; j<num_indices; ++j)
	    if (std::abs(sobol_indices[j]) > vbdDropTol) // print interaction
	      s << "                     " << std::setw(write_precision+7) 
		<< sobol_indices[j] << ' ' << sobol_labels[j] << '\n'; // TO DO
	}
      }
      else
	s << fn_labels[i] << " Sobol' indices not available due to negligible "
	  << "variance\n";
    }
    else
      s << fn_labels[i] << "Sobol' indices not available due to expansion "
	<< "coefficient mode\n";
  }
}


void NonDExpansion::print_local_sensitivity(std::ostream& s)
{
  const StringArray& fn_labels = iteratedModel.response_labels();
  s << "\nLocal sensitivities for each response function evaluated at "
    << "uncertain variable means:\n";
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  for (size_t i=0; i<numFunctions; ++i) {
    PecosApproximation* poly_approx_rep
      = (PecosApproximation*)poly_approxs[i].approx_rep();
    if (poly_approx_rep->expansion_coefficient_flag()) {
      s << fn_labels[i] << ":\n";
      write_col_vector_trans(s, (int)i, true, true, true, expGradsMeanX);
    }
  }
}


void NonDExpansion::print_results(std::ostream& s)
{
  s << std::scientific << std::setprecision(write_precision);

  print_coefficients(s);
  s << "-----------------------------------------------------------------------"
    << "------\nStatistics derived analytically from polynomial expansion:\n";
  print_moments(s);
  print_covariance(s);

  if (!subIteratorFlag && outputLevel >= NORMAL_OUTPUT)
    print_local_sensitivity(s);
  if (vbdFlag)
    print_sobol_indices(s);

  if (totalLevelRequests) {
    s << "\nStatistics based on ";
    if (expSampling)
      s << numSamplesOnExpansion << " samples performed on polynomial "
	<< "expansion:\n";
    else
      s << "projection of analytic moments:\n";

    // Note: PDF output ignores any importance sampling refinements
    if (expSampling) {
      NonDSampling* exp_sampler_rep
	= (NonDSampling*)expansionSampler.iterator_rep();
      if (exp_sampler_rep->pdf_output())
	exp_sampler_rep->print_pdf_mappings(s);
    }

    print_distribution_mappings(s);
    print_system_mappings(s);
  }
  s << "-----------------------------------------------------------------------"
    << "------" << std::endl;
}

} // namespace Dakota
