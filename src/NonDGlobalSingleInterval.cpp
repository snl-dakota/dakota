/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDGlobalSingleInterval
//- Description: Class for interval bound estimation for epistemic UQ
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "NonDGlobalSingleInterval.hpp"
#include "dakota_system_defs.hpp"
#include "DakotaModel.hpp"
#include "DakotaApproximation.hpp"

//#define DEBUG


namespace Dakota {

NonDGlobalSingleInterval::
NonDGlobalSingleInterval(ProblemDescDB& problem_db, Model& model):
  NonDGlobalInterval(problem_db, model)
{ }


NonDGlobalSingleInterval::~NonDGlobalSingleInterval()
{ }


void NonDGlobalSingleInterval::initialize()
{ numCells = 1; statCntr = 0; }


void NonDGlobalSingleInterval::get_best_sample(bool maximize, bool eval_approx)
{
  // Pull the samples and responses from data used to build latest GP
  // to determine truthFnStar for use in the expected improvement function
  const Pecos::SurrogateData& gp_data
    = fHatModel.approximation_data(respFnCntr);
  size_t i, index_star, num_data_pts = gp_data.points();
  truthFnStar = (maximize) ? -DBL_MAX : DBL_MAX;
  for (i=0; i<num_data_pts; ++i) {
    const Real& truth_fn = gp_data.response_function(i);
#ifdef DEBUG
    Cout << "GP response function[" << i+1 << "] = " << truth_fn << std::endl;
#endif // DEBUG
    if ( (  maximize && truth_fn > truthFnStar ) ||
	 ( !maximize && truth_fn < truthFnStar ) ) {
      index_star  = i;
      truthFnStar = truth_fn;
    }
  }

  // If needed, evaluate GP to update approxFnStar
  if (eval_approx) {
    if (numContIntervalVars)
      fHatModel.continuous_variables(gp_data.continuous_variables(index_star));
    if (numDiscIntervalVars || numDiscSetIntUncVars)
      fHatModel.discrete_int_variables(
        gp_data.discrete_int_variables(index_star));
    if (numDiscSetRealUncVars)
      fHatModel.discrete_real_variables(
        gp_data.discrete_real_variables(index_star));
    ActiveSet set = fHatModel.current_response().active_set();
    set.request_values(0); set.request_value(1, respFnCntr);
    fHatModel.evaluate(set);
    approxFnStar = fHatModel.current_response().function_value(respFnCntr);
  }
}


void NonDGlobalSingleInterval::post_process_cell_results(bool maximize)
{ finalStatistics.function_value(truthFnStar, statCntr++); }

} // namespace Dakota
