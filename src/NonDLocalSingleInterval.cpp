/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "NonDLocalSingleInterval.hpp"


namespace Dakota {

NonDLocalSingleInterval::
NonDLocalSingleInterval(ProblemDescDB& problem_db, std::shared_ptr<Model> model):
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
