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
  proposalUpdates(probDescDB.get_int("method.nond.proposal_updates")),
  randomSeed(probDescDB.get_int("method.random_seed")),
  emulatorType(probDescDB.get_short("method.nond.emulator")),
  adaptPosteriorRefine(
    probDescDB.get_bool("method.nond.adaptive_posterior_refinement"))
{
  numSamples = (proposalUpdates > 1) ?
    (int)floor((Real)probDescDB.get_int("method.samples") /
	       (Real)proposalUpdates + .5) :
    probDescDB.get_int("method.samples");

  // assign default maxIterations (DataMethod default is -1)
  if (adaptPosteriorRefine && maxIterations < 0)
    maxIterations = 25;

  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR:
    standardizedSpace = true; break;
  default:
    standardizedSpace = probDescDB.get_bool("method.nond.standardized_space");
    if (standardizedSpace) {
      initialize_random_variable_transformation();
      initialize_random_variable_types(STD_NORMAL_U); // need ranVarTypesX below
      // Note: initialize_random_variable_parameters() is performed at run time
      initialize_random_variable_correlations();
      verify_correlation_support();
      //initialize_final_statistics(); // statistics set is not default
    }
    break;
  }

  // Construct mcmcModel (no emulation, GP, PCE, or SC) for use in
  // likelihood evaluations
  switch (emulatorType) {

  case PCE_EMULATOR: { // instantiate a NonDPolynomialChaos iterator
    const UShortArray& level_seq
      = probDescDB.get_usa("method.nond.sparse_grid_level");
    if (!level_seq.empty())
      stochExpIterator.assign_rep(new NonDPolynomialChaos(iteratedModel,
	Pecos::COMBINED_SPARSE_GRID, level_seq, // ssg level sequence
	probDescDB.get_rv("method.nond.dimension_preference"), // not exposed
	EXTENDED_U, false, false));
    else // regression: least squares, compressed sensing, orthog least interp
      stochExpIterator.assign_rep(new NonDPolynomialChaos(iteratedModel,
	Pecos::DEFAULT_REGRESSION,
	probDescDB.get_usa("method.nond.expansion_order"), // exp_order sequence
	probDescDB.get_rv("method.nond.dimension_preference"), // not exposed
	probDescDB.get_real("method.nond.collocation_ratio"), randomSeed,
	EXTENDED_U, false,
	probDescDB.get_bool("method.derivative_usage"),        // not exposed
	probDescDB.get_bool("method.nond.cross_validation"))); // not exposed

    // no level mappings
    NonD* se_rep = (NonD*)stochExpIterator.iterator_rep();
    RealVectorArray empty_rv_array; // empty
    se_rep->requested_levels(empty_rv_array, empty_rv_array, empty_rv_array,
			     empty_rv_array, respLevelTarget,
			     respLevelTargetReduce, cdfFlag);
    // extract NonDExpansion's uSpaceModel for use in likelihood evals
    mcmcModel = stochExpIterator.algorithm_space_model(); // shared rep
    break;
  }

  case SC_EMULATOR: { // instantiate a NonDStochCollocation iterator
    stochExpIterator.assign_rep(new NonDStochCollocation(iteratedModel,
      Pecos::COMBINED_SPARSE_GRID,
      probDescDB.get_usa("method.nond.sparse_grid_level"), // ssg level sequence
      probDescDB.get_rv("method.nond.dimension_preference"), // not exposed
      EXTENDED_U, false,
      probDescDB.get_bool("method.derivative_usage")));      // not exposed

    // no level mappings
    NonD* se_rep = (NonD*)stochExpIterator.iterator_rep();
    RealVectorArray empty_rv_array; // empty
    se_rep->requested_levels(empty_rv_array, empty_rv_array, empty_rv_array,
			     empty_rv_array, respLevelTarget,
			     respLevelTargetReduce, cdfFlag);
    // extract NonDExpansion's uSpaceModel for use in likelihood evals
    mcmcModel = stochExpIterator.algorithm_space_model(); // shared rep
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
      transform_model(iteratedModel, g_u_model, true); // globally bounded
      lhs_iterator.assign_rep(new NonDLHSSampling(g_u_model, sample_type,
	samples, randomSeed,
	probDescDB.get_string("method.random_number_generator"), true,
	ACTIVE_UNIFORM), false);
      ActiveSet gp_set = g_u_model.current_response().active_set(); // copy
      gp_set.request_values(deriv_order); // for misfit Hessian
      mcmcModel.assign_rep(new DataFitSurrModel(lhs_iterator, g_u_model,
	gp_set, approx_type, approx_order, corr_type, corr_order, data_order,
        outputLevel, sample_reuse,
	probDescDB.get_string("method.export_points_file"),
	probDescDB.get_bool("method.export_points_file_annotated"),
	import_pts_file,
        probDescDB.get_bool("method.import_points_file_annotated"),
        probDescDB.get_bool("method.import_points_file_active")), false);
    }
    else {
      lhs_iterator.assign_rep(new NonDLHSSampling(iteratedModel, sample_type,
	samples, randomSeed,
	probDescDB.get_string("method.random_number_generator"), true,
	ACTIVE_UNIFORM), false);
      ActiveSet gp_set = iteratedModel.current_response().active_set(); // copy
      gp_set.request_values(deriv_order); // for misfit Hessian
      mcmcModel.assign_rep(new DataFitSurrModel(lhs_iterator, iteratedModel,
	gp_set, approx_type, approx_order, corr_type, corr_order, data_order,
        outputLevel, sample_reuse,
	probDescDB.get_string("method.export_points_file"),
	probDescDB.get_bool("method.export_points_file_annotated"),
	import_pts_file,
        probDescDB.get_bool("method.import_points_file_annotated"),
	probDescDB.get_bool("method.import_points_file_active")), false);
    }
    break;
  }

  case NO_EMULATOR:
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
    stochExpIterator.init_communicators(pl_iter);                  break;
  case GP_EMULATOR: case KRIGING_EMULATOR: case NO_EMULATOR:
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
    stochExpIterator.set_communicators(pl_iter);                  break;
  case GP_EMULATOR: case KRIGING_EMULATOR: case NO_EMULATOR:
    mcmcModel.set_communicators(pl_iter, maxEvalConcurrency); break;
  }
}


void NonDBayesCalibration::derived_free_communicators(ParLevLIter pl_iter)
{
  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR:
    stochExpIterator.free_communicators(pl_iter);                  break;
  case GP_EMULATOR: case KRIGING_EMULATOR: case NO_EMULATOR:
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
  case GP_EMULATOR: case KRIGING_EMULATOR:
    if (standardizedSpace) {
      initialize_random_variable_parameters();
      //initialize_final_statistics_gradients(); // not required
      natafTransform.transform_correlations();
    }
    mcmcModel.build_approximation(); break;
  case NO_EMULATOR:
    if (standardizedSpace) {
      initialize_random_variable_parameters();
      //initialize_final_statistics_gradients(); // not required
      natafTransform.transform_correlations();
    }
    break;
  }
}


//void NonDBayesCalibration::print_results(std::ostream& s)
//{ Cout << "Posterior sample results " << '\n'; }

} // namespace Dakota
