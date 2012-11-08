/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDLocalSingleInterval
//- Description: Class for interval bound estimation for epistemic UQ
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "NonDLocalSingleInterval.H"


namespace Dakota {

NonDLocalSingleInterval::NonDLocalSingleInterval(Model& model):
  NonDLocalInterval(model)
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
