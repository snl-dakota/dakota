/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDLocalEvidence
//- Description: Implementation code for NonDEvidence class
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "system_defs.h"
#include "data_io.h"           
#include "NonDLocalEvidence.H"
#include "data_types.h"
#include "ProblemDescDB.H"

//#define DEBUG


namespace Dakota {

NonDLocalEvidence::NonDLocalEvidence(Model& model): NonDLocalInterval(model)
{ }


NonDLocalEvidence::~NonDLocalEvidence()
{ }


void NonDLocalEvidence::initialize()
{ calculate_cells_and_bpas(); }


void NonDLocalEvidence::set_cell_bounds()
{
  minMaxModel.continuous_lower_bounds(cellLowerBounds[cellCntr]);
  minMaxModel.continuous_upper_bounds(cellUpperBounds[cellCntr]);
}


void NonDLocalEvidence::truncate_to_cell_bounds(RealVector& initial_pt)
{
  size_t i, num_vars = initial_pt.length();
  const RealVector& cell_l_bnds = cellLowerBounds[cellCntr];
  const RealVector& cell_u_bnds = cellUpperBounds[cellCntr];
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


void NonDLocalEvidence::post_process_cell_results(bool minimize)
{
  NonDLocalInterval::post_process_cell_results(minimize);

  // Write the optimization results to cell data structures
  const RealVector& fns_star_approx
    = minMaxOptimizer.response_results().function_values();
  Real fn_star = (minimize) ? fns_star_approx[0] : -fns_star_approx[0];
  if (minimize)
    cellFnLowerBounds[respFnCntr][cellCntr] = fn_star;
  else
    cellFnUpperBounds[respFnCntr][cellCntr] = fn_star;
}


void NonDLocalEvidence::post_process_response_fn_results()
{ calculate_cbf_cpf(); }


void NonDLocalEvidence::post_process_final_results()
{
#ifdef DEBUG
  for (size_t i=0; i < numCells; i++) {
    Cout << "Cell " << i << "\nBPA: " << cellBPA[i] << std::endl;;
    for (size_t ii=0; ii<numIntervalVars; ii++) {
      Cout << "Cell Bounds for variable " << ii << ": ("
	   << cellLowerBounds[i][ii] << ", " << cellUpperBounds[i][ii] << ")"
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
