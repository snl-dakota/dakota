/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDBayesCalibration
//- Description: Base class for generic Bayesian inference
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "NonDBayesCalibration.hpp"
#include "ProblemDescDB.hpp"
#include "DataFitSurrModel.hpp"
#include "NonDPolynomialChaos.hpp"
#include "NonDStochCollocation.hpp"
#include "NonDLHSSampling.hpp"

static const char rcsId[]="@(#) $Id$";

namespace Dakota {


/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDBayesCalibration::
NonDBayesCalibration(ProblemDescDB& problem_db, Model& model):
  NonDCalibration(problem_db, model),
  emulatorType(probDescDB.get_short("method.nond.emulator")),
  randomSeed(probDescDB.get_int("method.random_seed")),
  adaptPosteriorRefine(
    probDescDB.get_bool("method.nond.adaptive_posterior_refinement")),
  proposalCovarType(
    probDescDB.get_string("method.nond.proposal_covariance_type")),
  proposalCovarData(probDescDB.get_rv("method.nond.proposal_covariance_data")),
  proposalCovarFilename(
    probDescDB.get_string("method.nond.proposal_covariance_filename")),
  proposalCovarInputType(
    probDescDB.get_string("method.nond.proposal_covariance_input_type"))
{
  // assign default proposalCovarType
  if (proposalCovarType.empty()) {
    if (emulatorType) proposalCovarType = "derivatives"; // misfit Hessian
    else              proposalCovarType = "prior";       // prior covariance
  }

  // manage sample partitions and defaults
  int samples_spec = probDescDB.get_int("method.samples");
  if (proposalCovarType == "derivatives") {
    int pc_update_spec
      = probDescDB.get_int("method.nond.proposal_covariance_updates");
    if (pc_update_spec < 1) { // default partition: update every 100 samples
      numSamples  = 100;
      chainCycles = (int)floor((Real)samples_spec / (Real)numSamples + .5);
    }
    else { // partition as specified
      numSamples  = (int)floor((Real)samples_spec / (Real)pc_update_spec + .5);
      chainCycles = pc_update_spec;
    }
  }
  else
    { numSamples = samples_spec; chainCycles = 1; }

  // assign default maxIterations (DataMethod default is -1)
  if (adaptPosteriorRefine && maxIterations < 0)
    maxIterations = 25;

  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR:
    standardizedSpace = true; break; // natafTransform defined w/i NonDExpansion
  default:
    standardizedSpace = probDescDB.get_bool("method.nond.standardized_space");

    // define local natafTransform, whether standardized space or not,
    // since we utilize x-space bounds, moments, density routines
    initialize_random_variable_transformation();
    initialize_random_variable_types(ASKEY_U); // need ranVarTypesX below
    // initialize_random_variable_parameters() is performed at run time
    initialize_random_variable_correlations();
    //initialize_final_statistics(); // statistics set is not default

    // only needed if Nataf transform will actually be performed
    if (standardizedSpace)
      verify_correlation_support(ASKEY_U);
    break;
  }

  // Construct mcmcModel (no emulation, GP, PCE, or SC) for use in
  // likelihood evaluations
  switch (emulatorType) {

  case PCE_EMULATOR: case SC_EMULATOR: {
    const UShortArray& level_seq
      = probDescDB.get_usa("method.nond.sparse_grid_level");
    const RealVector& dim_pref
      = probDescDB.get_rv("method.nond.dimension_preference");    // not exposed
    bool derivs = probDescDB.get_bool("method.derivative_usage"); // not exposed
    NonD* se_rep;
    if (emulatorType == SC_EMULATOR) // SC sparse grid interpolation
      se_rep = new NonDStochCollocation(iteratedModel,
	Pecos::COMBINED_SPARSE_GRID, level_seq, dim_pref, EXTENDED_U,
	false, derivs);
    else if (level_seq.empty()) // PCE with regression: LeastSq, CS, OLI
      se_rep = new NonDPolynomialChaos(iteratedModel, Pecos::DEFAULT_REGRESSION,
	probDescDB.get_usa("method.nond.expansion_order"), // exp_order sequence
	dim_pref, probDescDB.get_real("method.nond.collocation_ratio"),
	randomSeed, EXTENDED_U, false, derivs,	
	probDescDB.get_bool("method.nond.cross_validation")); // not exposed
    else // PCE with spectral projection via sparse grid
      se_rep = new NonDPolynomialChaos(iteratedModel,
	Pecos::COMBINED_SPARSE_GRID, level_seq, dim_pref, EXTENDED_U,
	false, false);
    stochExpIterator.assign_rep(se_rep);
    // no level mappings
    RealVectorArray empty_rv_array; // empty
    se_rep->requested_levels(empty_rv_array, empty_rv_array, empty_rv_array,
			     empty_rv_array, respLevelTarget,
			     respLevelTargetReduce, cdfFlag);
    // extract NonDExpansion's uSpaceModel for use in likelihood evals
    mcmcModel = stochExpIterator.algorithm_space_model(); // shared rep
    natafTransform = se_rep->variable_transformation();   // shared rep
    break;
  }

  case GP_EMULATOR: case KRIGING_EMULATOR: {
    String sample_reuse; String approx_type; short deriv_order;
    if (emulatorType == GP_EMULATOR)
      { approx_type = "global_gaussian"; deriv_order = 3; } // grad support
    else
      { approx_type = "global_kriging";  deriv_order = 7; } // grad,Hess support
    UShortArray approx_order; // not used by GP/kriging
    short corr_order = -1, data_order = 1, corr_type = NO_CORRECTION;
    if (probDescDB.get_bool("method.derivative_usage")) {
      // derivatives for emulator construction (not emulator evaluation)
      if (iteratedModel.gradient_type() != "none") data_order |= 2;
      if (iteratedModel.hessian_type()  != "none") data_order |= 4;
    }
    unsigned short sample_type = SUBMETHOD_DEFAULT;
    int samples = probDescDB.get_int("method.nond.emulator_samples");
    // get point samples file
    const String& import_pts_file
      = probDescDB.get_string("method.import_points_file");
    if (!import_pts_file.empty())
      { samples = 0; sample_reuse = "all"; }
     
    // Consider elevating lhsSampler from NonDGPMSABayesCalibration:
    Iterator lhs_iterator;
    if (standardizedSpace) {
      Model g_u_model;
      transform_model(iteratedModel, g_u_model, true); // global bnds

      NonDLHSSampling* lhs_rep = new NonDLHSSampling(g_u_model,
	sample_type, samples, randomSeed,
	probDescDB.get_string("method.random_number_generator"),
	true, ACTIVE_UNIFORM);
      lhs_iterator.assign_rep(lhs_rep, false);

      // natafTransform is not fully updated at this point, but using
      // a shallow copy allows run time updates to propagate
      lhs_rep->initialize_random_variables(natafTransform); // shallow copy

      ActiveSet gp_set = g_u_model.current_response().active_set(); // copy
      gp_set.request_values(deriv_order); // for misfit Hessian
      mcmcModel.assign_rep(new DataFitSurrModel(lhs_iterator, g_u_model,
	gp_set, approx_type, approx_order, corr_type, corr_order, data_order,
        outputLevel, sample_reuse,
	probDescDB.get_string("method.export_points_file"),
	probDescDB.get_ushort("method.export_points_file_format"),
	import_pts_file,
        probDescDB.get_ushort("method.import_points_file_format"),
        probDescDB.get_bool("method.import_points_file_active")), false);
    }
    else {
      lhs_iterator.assign_rep(new NonDLHSSampling(iteratedModel,
	sample_type, samples, randomSeed,
	probDescDB.get_string("method.random_number_generator"),
	true, ACTIVE_UNIFORM), false);

      ActiveSet gp_set = iteratedModel.current_response().active_set(); // copy
      gp_set.request_values(deriv_order); // for misfit Hessian
      mcmcModel.assign_rep(new DataFitSurrModel(lhs_iterator, iteratedModel,
	gp_set, approx_type, approx_order, corr_type, corr_order, data_order,
        outputLevel, sample_reuse,
	probDescDB.get_string("method.export_points_file"),
	probDescDB.get_ushort("method.export_points_file_format"),
	import_pts_file,
        probDescDB.get_ushort("method.import_points_file_format"),
	probDescDB.get_bool("method.import_points_file_active")), false);
    }
    break;
  }

  case NO_EMULATOR:
    standardizedSpace = probDescDB.get_bool("method.nond.standardized_space");
    if (standardizedSpace) // recast to standardized probability space
      transform_model(iteratedModel, mcmcModel); // no global bounds
    else
      mcmcModel = iteratedModel; // shared rep
    break;
  }

  int mcmc_concurrency = 1; // prior to concurrent chains
  maxEvalConcurrency *= mcmc_concurrency;
}


NonDBayesCalibration::~NonDBayesCalibration()
{ }


void NonDBayesCalibration::derived_init_communicators(ParLevLIter pl_iter)
{
  //iteratedModel.init_communicators(maxEvalConcurrency);

  // stochExpIterator and mcmcModel use NoDBBaseConstructor,
  // so no need to manage DB list nodes at this level
  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR:
    stochExpIterator.init_communicators(pl_iter);              break;
  default:
    mcmcModel.init_communicators(pl_iter, maxEvalConcurrency); break;
  }
}


void NonDBayesCalibration::derived_set_communicators(ParLevLIter pl_iter)
{
  miPLIndex = methodPCIter->mi_parallel_level_index(pl_iter);
  //iteratedModel.set_communicators(maxEvalConcurrency);

  // stochExpIterator and mcmcModel use NoDBBaseConstructor,
  // so no need to manage DB list nodes at this level
  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR:
    stochExpIterator.set_communicators(pl_iter);              break;
  default:
    mcmcModel.set_communicators(pl_iter, maxEvalConcurrency); break;
  }
}


void NonDBayesCalibration::derived_free_communicators(ParLevLIter pl_iter)
{
  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR:
    stochExpIterator.free_communicators(pl_iter);              break;
  default:
    mcmcModel.free_communicators(pl_iter, maxEvalConcurrency); break;
  }

  //iteratedModel.free_communicators(maxEvalConcurrency);
}


void NonDBayesCalibration::initialize_model()
{
  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR: {
    ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
    stochExpIterator.run(pl_iter); break;
  }
  default: // GPs and NO_EMULATOR
    initialize_random_variable_parameters(); // standardizedSpace or not
    //initialize_final_statistics_gradients(); // not required
    if (standardizedSpace) transform_correlations();
    if (emulatorType)      mcmcModel.build_approximation();
    break;
  }
}


//void NonDBayesCalibration::print_results(std::ostream& s)
//{ Cout << "Posterior sample results " << '\n'; }

} // namespace Dakota
