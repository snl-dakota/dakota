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

#include "NonDBayesCalibration.H"
#include "ProblemDescDB.H"
#include "DataFitSurrModel.H"
#include "NonDPolynomialChaos.H"
#include "NonDStochCollocation.H"

static const char rcsId[]="@(#) $Id$";

namespace Dakota {


/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDBayesCalibration::NonDBayesCalibration(Model& model):
  NonDCalibration(model), standardizedSpace(false), // prior to adding to spec
  emulatorType(probDescDB.get_short("method.nond.emulator"))
{
  switch (emulatorType) {
  case POLYNOMIAL_CHAOS: case STOCHASTIC_COLLOCATION:
    standardizedSpace = true; break;
  case GAUSSIAN_PROCESS: case NO_EMULATOR:
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
  int mcmc_concurrency = 1; // prior to concurrent chains
  switch (emulatorType) {
  case POLYNOMIAL_CHAOS: case STOCHASTIC_COLLOCATION: {
    // instantiate a NonD{PolynomialChaos,StochCollocation} iterator
    bool use_derivs = probDescDB.get_bool("method.derivative_usage");
    unsigned short level
      = probDescDB.get_ushort("method.nond.sparse_grid_level");
    if (emulatorType == POLYNOMIAL_CHAOS)
      stochExpIterator.assign_rep(new NonDPolynomialChaos(iteratedModel,
	Pecos::SPARSE_GRID, level, EXTENDED_U, use_derivs));
    else
      stochExpIterator.assign_rep(new NonDStochCollocation(iteratedModel,
	Pecos::SPARSE_GRID, level, EXTENDED_U, use_derivs));
    // extract NonDExpansion's uSpaceModel for use in likelihood evals
    emulatorModel = stochExpIterator.algorithm_space_model(); // shared rep
    emulatorModel.init_communicators(mcmc_concurrency);
    break;
  }
  case GAUSSIAN_PROCESS: {
    String approx_type("global_gaussian")/*("global_kriging")*/,
      sample_reuse, corr_type;
    UShortArray approx_order; // not used by GP/kriging
    short corr_order = -1, data_order = 1;
    if (probDescDB.get_bool("method.derivative_usage")) {
      if (gradientType != "none") data_order |= 2;
      if (hessianType  != "none") data_order |= 4;
    }
    int samples = probDescDB.get_int("method.nond.emulator_samples"),
        seed    = probDescDB.get_int("method.random_seed");
    const String& rng = probDescDB.get_string("method.random_number_generator");
    // Consider elevating lhsSampler from NonDGPMSABayesCalibration:
    Iterator lhs_iterator;
    if (standardizedSpace) {
      Model g_u_model;
      construct_u_space_model(iteratedModel, g_u_model, true);//globally bounded
      construct_lhs(lhs_iterator, g_u_model, samples, seed, rng);
      emulatorModel.assign_rep(new DataFitSurrModel(lhs_iterator, g_u_model,
        approx_type, approx_order, corr_type, corr_order, data_order,
        sample_reuse), false);
    }
    else {
      construct_lhs(lhs_iterator, iteratedModel, samples, seed, rng);
      emulatorModel.assign_rep(new DataFitSurrModel(lhs_iterator, iteratedModel,
        approx_type, approx_order, corr_type, corr_order, data_order,
        sample_reuse), false);
    }
    emulatorModel.init_communicators(mcmc_concurrency);
    break;
  }
  case NO_EMULATOR:
    if (standardizedSpace) { // recast to standardized probability space
      construct_u_space_model(iteratedModel, emulatorModel); // no global bounds
      emulatorModel.init_communicators(mcmc_concurrency);
    }
    else
      emulatorModel = iteratedModel; // shared rep
    break;
  }
}


NonDBayesCalibration::~NonDBayesCalibration()
{
  if (emulatorType || standardizedSpace) {
    int mcmc_concurrency = 1; // prior to concurrent chains
    emulatorModel.free_communicators(mcmc_concurrency);
  }
}


void NonDBayesCalibration::quantify_uncertainty()
{
  switch (emulatorType) {
  case POLYNOMIAL_CHAOS: case STOCHASTIC_COLLOCATION:
    stochExpIterator.run_iterator(Cout); break;
  case GAUSSIAN_PROCESS:
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
