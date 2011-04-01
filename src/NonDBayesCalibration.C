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

static const char rcsId[]="@(#) $Id$";

namespace Dakota {


// define special values for emulator_type
enum { NO_EMULATOR, STOCHASTIC_EXPANSION, GAUSSIAN_PROCESS };


/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDBayesCalibration::NonDBayesCalibration(Model& model):
  NonDCalibration(model), seedSpec(probDescDB.get_int("method.random_seed")),
  numSamples(probDescDB.get_int("method.samples")),
  rngName(probDescDB.get_string("method.random_number_generator"))
{
  // Create a generic DataFitSurrModel recursion: GP, stoch expansion, none
  short emulator_type = NO_EMULATOR; // hard wire for now

  switch (emulator_type) {
  case STOCHASTIC_EXPANSION: {

    // Should we instantiate a NonDPCE/NonDSC iterator instead and then
    // extract its uSpaceModel for use in the likelihood evaluations?

    String approx_type = (true) ? // TO DO
      "global_orthogonal_polynomial" : "global_interpolation_polynomial";
    String sample_reuse, corr_type;
    UShortArray exp_order; // TO DO
    short corr_order = -1, data_order = 1;
    if (probDescDB.get_bool("method.derivative_usage")) {
      if (gradientType != "none") data_order |= 2;
      if (hessianType  != "none") data_order |= 4;
    }
    int samples = 0, seed = 0;
    Iterator colloc_iterator; // TO DO
    //const Variables& curr_vars = iteratedModel.current_variables();
    emulatorModel.assign_rep(new DataFitSurrModel(colloc_iterator,
      iteratedModel, //curr_vars.view(), curr_vars.variables_components(),
      //iteratedModel.current_response().active_set(),
      approx_type, exp_order, corr_type, corr_order, data_order, sample_reuse),
      false);

    int mcmc_concurrency = 1;
    emulatorModel.init_communicators(mcmc_concurrency);
    break;
  }
  case GAUSSIAN_PROCESS: {
    String approx_type("global_gaussian"), sample_reuse, corr_type;
    UShortArray approx_order; // TO DO
    short corr_order = -1, data_order = 1;
    if (probDescDB.get_bool("method.derivative_usage")) {
      if (gradientType != "none") data_order |= 2;
      if (hessianType  != "none") data_order |= 4;
    }
    int samples = 0, seed = 0;
    // Consider elevating lhsSampler from NonDGPMSABayesCalibration:
    Iterator lhs_iterator; // TO DO
    //const Variables& curr_vars = iteratedModel.current_variables();
    emulatorModel.assign_rep(new DataFitSurrModel(lhs_iterator, iteratedModel,
      //curr_vars.view(), curr_vars.variables_components(),
      //iteratedModel.current_response().active_set(),
      approx_type, approx_order, corr_type, corr_order, data_order,
      sample_reuse), false);

    int mcmc_concurrency = 1;
    emulatorModel.init_communicators(mcmc_concurrency);
    break;
  }
  case NO_EMULATOR:
    emulatorModel = iteratedModel; // shared rep
    break;
  }
}


NonDBayesCalibration::~NonDBayesCalibration()
{
  if (emulatorModel.model_type() == "surrogate") {
    int mcmc_concurrency = 1;
    emulatorModel.free_communicators(mcmc_concurrency);
  }
}


//void NonDBayesCalibration::print_results(std::ostream& s)
//{ Cout << "Posterior sample results " << '\n'; }

} // namespace Dakota
