/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       MinimizerAdapterModel
//- Description: Implementation code for the MinimizerAdapterModel class
//- Owner:       Mike Eldred
//- Checked by:

//#include "dakota_system_defs.hpp"
#include "MinimizerAdapterModel.hpp"
//#include "EvaluationStore.hpp"

static const char rcsId[]="@(#) $Id: MinimizerAdapterModel.cpp 7029 2010-10-22 00:17:02Z mseldre $";


namespace Dakota {

//#define DEBUG


/** Default constructor.  Includes full definition of a minimization
    sub-problem. */
MinimizerAdapterModel::
MinimizerAdapterModel(const RealVector& cv_initial_pt,//, di_vars, dr_vars, ...
		      const RealVector& cv_lower_bnds,
		      const RealVector& cv_upper_bnds,
		      const RealMatrix& lin_ineq_coeffs,
		      const RealVector& lin_ineq_lower_bnds,
		      const RealVector& lin_ineq_upper_bnds,
		      const RealMatrix& lin_eq_coeffs,
		      const RealVector& lin_eq_targets,
		      const RealVector& nonlin_ineq_lower_bnds,
		      const RealVector& nonlin_ineq_upper_bnds,
		      const RealVector& nonlin_eq_targets,
		      void (*resp_map) (const Variables& vars,
					const ActiveSet& set,
					Response& response)):
  AdapterModel(resp_map) // this ctor path provides minimal initialization
{
  modelId = "MINIMIZER_ADAPTER";
  outputLevel = SILENT_OUTPUT;

  assign_variables(cv_initial_pt);
  assign_constraints(cv_lower_bnds, cv_upper_bnds, lin_ineq_coeffs,
		     lin_ineq_lower_bnds, lin_ineq_upper_bnds,
		     lin_eq_coeffs, lin_eq_targets, nonlin_ineq_lower_bnds,
		     nonlin_ineq_upper_bnds, nonlin_eq_targets);
  initialize_response();
}


/** Rely on AdapterModel for this generic case (not Minimizer specific)
MinimizerAdapterModel::
MinimizerAdapterModel(const Variables& initial_vars, const Constraints& cons,
		      const Response& resp,
		      void (*resp_map) (const Variables& vars,
					const ActiveSet& set,
					Response& response)):
  AdapterModel(initial_vars, cons, resp, resp_map)
{
  modelId = "MINIMIZER_ADAPTER";
  outputLevel = SILENT_OUTPUT;
}
*/


/** This alternate constructor defers initialization of the variable and
    constraint data until separate calls to initialize_*(). */
MinimizerAdapterModel::
MinimizerAdapterModel(void (*resp_map) (const Variables& vars,
					const ActiveSet& set,
					Response& response)):
  AdapterModel(resp_map)
{
  modelId = "MINIMIZER_ADAPTER";
  outputLevel = SILENT_OUTPUT;
}

} // namespace Dakota
