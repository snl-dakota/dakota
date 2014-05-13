/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDLocalSingleInterval
//- Description: Class for interval bound estimation for epistemic UQ
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "NonDLocalSingleInterval.hpp"


namespace Dakota {

NonDLocalSingleInterval::
NonDLocalSingleInterval(ProblemDescDB& problem_db, Model& model):
  NonDLocalInterval(problem_db, model)
{ }


NonDLocalSingleInterval::~NonDLocalSingleInterval()
{ }


void NonDLocalSingleInterval::initialize()
{ numCells = 1; statCntr = 0; }


void NonDLocalSingleInterval::post_process_cell_results(bool maximize)
{
  NonDLocalInterval::post_process_cell_results(maximize);

  const RealVector& fns_star_approx
    = minMaxOptimizer.response_results().function_values();
  finalStatistics.function_value(fns_star_approx[0], statCntr++);
}

} // namespace Dakota
