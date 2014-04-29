/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
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
  standardizedSpace(false), // prior to adding to spec
  emulatorType(probDescDB.get_short("method.nond.emulator"))
{
  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR:
    standardizedSpace = true; break;
  case GP_EMULATOR: case KRIGING_EMULATOR: case NO_EMULATOR:
    // TO DO: add to spec
    //standardizedSpace = probDescDB.get_bool("method.nond.standardized_space");
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

  // Construct emulatorModel (no emulation, GP, PCE, or SC) for use in
  // likelihood evaluations
  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR: {
    // instantiate a NonD{PolynomialChaos,StochCollocation} iterator
    bool use_derivs = probDescDB.get_bool("method.derivative_usage");
    unsigned short level
      = probDescDB.get_usa("method.nond.sparse_grid_level")[0];
    if (emulatorType == PCE_EMULATOR)
      stochExpIterator.assign_rep(new NonDPolynomialChaos(iteratedModel,
	Pecos::COMBINED_SPARSE_GRID, level, EXTENDED_U, false, use_derivs));
    else
      stochExpIterator.assign_rep(new NonDStochCollocation(iteratedModel,
	Pecos::COMBINED_SPARSE_GRID, level, EXTENDED_U, false, use_derivs));
    // no level mappings
    NonD* se_rep = (NonD*)stochExpIterator.iterator_rep();
    RealVectorArray empty_rv_array; // empty
    se_rep->requested_levels(empty_rv_array, empty_rv_array, empty_rv_array,
			     empty_rv_array, respLevelTarget,
			     respLevelTargetReduce, cdfFlag);
    // extract NonDExpansion's uSpaceModel for use in likelihood evals
    emulatorModel = stochExpIterator.algorithm_space_model(); // shared rep
    break;
  }
  case GP_EMULATOR: case KRIGING_EMULATOR: {
    String sample_reuse;
    String approx_type =
      (probDescDB.get_short("method.nond.emulator") == GP_EMULATOR) ?
      "global_gaussian" : "global_kriging";
    UShortArray approx_order; // not used by GP/kriging
    short corr_order = -1, data_order = 1, corr_type = NO_CORRECTION;
    if (probDescDB.get_bool("method.derivative_usage")) {
      if (iteratedModel.gradient_type() != "none") data_order |= 2;
      if (iteratedModel.hessian_type()  != "none") data_order |= 4;
    }
    unsigned short sample_type = SUBMETHOD_DEFAULT;
    int samples = probDescDB.get_int("method.nond.emulator_samples"),
        seed    = probDescDB.get_int("method.random_seed");
    const String& rng = probDescDB.get_string("method.random_number_generator");
    // get point samples file
    const String& import_pts_file
      = probDescDB.get_string("method.import_points_file");
    if (!import_pts_file.empty())
      { samples = 0; sample_reuse = "all"; }
     
    // Consider elevating lhsSampler from NonDGPMSABayesCalibration:
    //Iterator lhs_iterator;
    if (standardizedSpace) {
      Model g_u_model;
      transform_model(iteratedModel, g_u_model, true); // globally bounded
      lhsIterator.assign_rep(new NonDLHSSampling(g_u_model, sample_type,
	samples, seed, rng, true, ACTIVE_UNIFORM), false);
      emulatorModel.assign_rep(new DataFitSurrModel(lhsIterator, g_u_model,
        approx_type, approx_order, corr_type, corr_order, data_order,
        outputLevel, sample_reuse,
	probDescDB.get_string("method.export_points_file"),
	probDescDB.get_bool("method.export_points_file_annotated"),
	import_pts_file,
	probDescDB.get_bool("method.import_points_file_annotated")), false);
    }
    else {
      lhsIterator.assign_rep(new NonDLHSSampling(iteratedModel, sample_type,
	samples, seed, rng, true, ACTIVE_UNIFORM), false);
      emulatorModel.assign_rep(new DataFitSurrModel(lhsIterator, iteratedModel,
        approx_type, approx_order, corr_type, corr_order, data_order,
        outputLevel, sample_reuse,
	probDescDB.get_string("method.export_points_file"),
	probDescDB.get_bool("method.export_points_file_annotated"),
	import_pts_file,
	probDescDB.get_bool("method.import_points_file_annotated")), false);
    }
    break;
  }
  case NO_EMULATOR:
    if (standardizedSpace) // recast to standardized probability space
      transform_model(iteratedModel, emulatorModel); // no global bounds
    else
      emulatorModel = iteratedModel; // shared rep
    break;
  }
}


NonDBayesCalibration::~NonDBayesCalibration()
{ }


void NonDBayesCalibration::init_communicators()
{
  //iteratedModel.init_communicators(maxEvalConcurrency);

  //if (emulatorType || standardizedSpace) {
    int mcmc_concurrency = 1; // prior to concurrent chains
    emulatorModel.init_communicators(mcmc_concurrency);
  //}
}


void NonDBayesCalibration::free_communicators()
{
  //if (emulatorType || standardizedSpace) {
    int mcmc_concurrency = 1; // prior to concurrent chains
    emulatorModel.free_communicators(mcmc_concurrency);
  //}

  //iteratedModel.free_communicators(maxEvalConcurrency);
}


void NonDBayesCalibration::quantify_uncertainty()
{
  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR:
    stochExpIterator.run(Cout); break;
  case GP_EMULATOR: case KRIGING_EMULATOR:
    if (standardizedSpace) {
      initialize_random_variable_parameters();
      //initialize_final_statistics_gradients(); // not required
      natafTransform.transform_correlations();
    }
    emulatorModel.build_approximation(); break;
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
