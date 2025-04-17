/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"           
#include "NonDLocalEvidence.hpp"
#include "dakota_data_types.hpp"
#include "ProblemDescDB.hpp"

//#define DEBUG


namespace Dakota {

NonDLocalEvidence::NonDLocalEvidence(ProblemDescDB& problem_db, std::shared_ptr<Model> model):
  NonDLocalInterval(problem_db, model)
{ }


NonDLocalEvidence::~NonDLocalEvidence()
{ }


void NonDLocalEvidence::initialize()
{ calculate_cells_and_bpas(); }


void NonDLocalEvidence::set_cell_bounds()
{
  size_t j ;
  for (j=0; j<numContIntervalVars; j++) {
    ModelUtils::continuous_lower_bound(*minMaxModel, cellContLowerBounds[cellCntr][j],j);
    ModelUtils::continuous_upper_bound(*minMaxModel, cellContUpperBounds[cellCntr][j],j);
  }

  for (j=0; j<numDiscIntervalVars; j++) {
    ModelUtils::discrete_int_lower_bound(*minMaxModel, cellIntRangeLowerBounds[cellCntr][j],j);
    ModelUtils::discrete_int_upper_bound(*minMaxModel, cellIntRangeUpperBounds[cellCntr][j],j);
  }

  for (j=0; j<numDiscSetIntUncVars; j++) {
    ModelUtils::discrete_int_variable(*minMaxModel, cellIntSetBounds[cellCntr][j],j+numDiscIntervalVars);
  }

  for (j=0; j<numDiscSetRealUncVars; j++) {
    ModelUtils::discrete_real_variable(*minMaxModel, cellRealSetBounds[cellCntr][j],j);
  }

}


void NonDLocalEvidence::truncate_to_cell_bounds(RealVector& initial_pt)
{
  size_t i, num_vars = initial_pt.length();
  const RealVector& cell_l_bnds = cellContLowerBounds[cellCntr];
  const RealVector& cell_u_bnds = cellContUpperBounds[cellCntr];
  for (i=0; i<num_vars; ++i) {
    const Real& lwr = cell_l_bnds[i];
    const Real& upr = cell_u_bnds[i];
    Real& initial_pt_i = initial_pt[i];
    if (initial_pt_i < lwr)
      initial_pt_i = lwr;
    else if (initial_pt_i > upr)
      initial_pt_i = upr;
  }
}


void NonDLocalEvidence::post_process_cell_results(bool maximize)
{
  NonDLocalInterval::post_process_cell_results(maximize);

  // Write the optimization results to cell data structures
  const RealVector& fns_star_approx
    = minMaxOptimizer->response_results().function_values();
  if (maximize)
    cellFnUpperBounds[respFnCntr][cellCntr] = fns_star_approx[0];
  else
    cellFnLowerBounds[respFnCntr][cellCntr] = fns_star_approx[0];
}


void NonDLocalEvidence::post_process_response_fn_results()
{ calculate_cbf_cpf(); }


void NonDLocalEvidence::post_process_final_results()
{
#ifdef DEBUG
  for (size_t i=0; i < numCells; i++) {
    Cout << "Cell " << i << "\nBPA: " << cellBPA[i] << std::endl;;
    for (size_t ii=0; ii<numContIntervalVars; ii++) {
      Cout << "Cell Bounds for variable " << ii << ": ("
	   << cellContLowerBounds[i][ii] << ", " << cellContUpperBounds[i][ii] << ")"
	   << std::endl;
    }
    Cout << "(min,max) for cell " << i << ": (" << cellFnLowerBounds[0][i]
	 << ", " << cellFnUpperBounds[0][i] << ")\n";
  }
#endif
  // compute statistics 
  compute_evidence_statistics();
}

} // namespace Dakota
