/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDGlobalSingleInterval
//- Description: Class for interval bound estimation for epistemic UQ
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "NonDGlobalSingleInterval.H"
#include "system_defs.h"
#include "DakotaModel.H"
#include "DakotaApproximation.H"


namespace Dakota {

NonDGlobalSingleInterval::NonDGlobalSingleInterval(Model& model):
  NonDGlobalInterval(model)
{ }


NonDGlobalSingleInterval::~NonDGlobalSingleInterval()
{ }


void NonDGlobalSingleInterval::initialize()
{ numCells = 1; statCntr = 0; }


void NonDGlobalSingleInterval::post_process_cell_results(bool minimize)
{ finalStatistics.function_value(truthFnStar, statCntr++); }


void NonDGlobalSingleInterval::get_best_sample(bool find_max, bool eval_approx)
{
  // Pull the samples and responses from data used to build latest GP
  // to determine fnStar for use in the expected improvement function

  const SDPList& gp_data = fHatModel.approximation_data(respFnCntr);
  SDPLCIter cit, cit_star;

  truthFnStar = (find_max) ? -DBL_MAX : DBL_MAX;
  for (cit = gp_data.begin(); cit != gp_data.end(); ++cit) {
    const Real& truth_fn = cit->response_function();
    if ( (  find_max && truth_fn > truthFnStar ) ||
	 ( !find_max && truth_fn < truthFnStar ) ) {
      cit_star    = cit;
      truthFnStar = truth_fn;
    }
  }

  if (eval_approx) {
    fHatModel.continuous_variables(cit_star->continuous_variables());
    ActiveSet set = fHatModel.current_response().active_set();
    set.request_values(0); set.request_value(respFnCntr, 1);
    fHatModel.compute_response(set);
    approxFnStar = fHatModel.current_response().function_values()[respFnCntr];
  }
}

} // namespace Dakota
