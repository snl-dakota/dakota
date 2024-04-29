/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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
  const Pecos::SDVArray& sdv_array = gp_data.variables_data();
  const Pecos::SDRArray& sdr_array = gp_data.response_data();

  size_t i, index_star, num_data_pts = gp_data.points();
  truthFnStar = (maximize) ? -DBL_MAX : DBL_MAX;
  for (i=0; i<num_data_pts; ++i) {
    Real truth_fn = sdr_array[i].response_function();
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
    const Pecos::SurrogateDataVars& sdv = sdv_array[index_star];
    if (numContIntervalVars)
      fHatModel.continuous_variables(sdv.continuous_variables());
    if (numDiscIntervalVars || numDiscSetIntUncVars)
      fHatModel.discrete_int_variables(sdv.discrete_int_variables());
    if (numDiscSetRealUncVars)
      fHatModel.discrete_real_variables(sdv.discrete_real_variables());
    ActiveSet set = fHatModel.current_response().active_set();
    set.request_values(0); set.request_value(1, respFnCntr);
    fHatModel.evaluate(set);
    approxFnStar = fHatModel.current_response().function_value(respFnCntr);
  }
}


void NonDGlobalSingleInterval::post_process_cell_results(bool maximize)
{ finalStatistics.function_value(truthFnStar, statCntr++); }

} // namespace Dakota
