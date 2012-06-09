/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDGlobalEvidence
//- Description: Implementation code for NonDEvidence class
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "NonDGlobalEvidence.H"
#include "data_types.h"
#include "system_defs.h"
#include "DakotaApproximation.H"


namespace Dakota {

NonDGlobalEvidence::NonDGlobalEvidence(Model& model): NonDGlobalInterval(model)
{
  // if the user does not specify the number of samples, 
  // try to get at least one function evaluation in each cell 
  //if (!numSamples)
  //  numSamples = numCells;
}


NonDGlobalEvidence::~NonDGlobalEvidence()
{ }


void NonDGlobalEvidence::initialize()
{ calculate_cells_and_bpas(); }


void NonDGlobalEvidence::set_cell_bounds()
{
  eifModel.continuous_lower_bounds(cellLowerBounds[cellCntr]);
  eifModel.continuous_upper_bounds(cellUpperBounds[cellCntr]);
}


void NonDGlobalEvidence::post_process_cell_results(bool minimize)
{
  if (minimize)
    cellFnLowerBounds[respFnCntr][cellCntr] = truthFnStar;
  else
    cellFnUpperBounds[respFnCntr][cellCntr] = truthFnStar;
}


void NonDGlobalEvidence::post_process_response_fn_results()
{ calculate_cbf_cpf(); }


void NonDGlobalEvidence::post_process_final_results()
{
#ifdef DEBUG
  for (size_t i=0; i<numCells; i++) {
    Cout << "Cell " << i << "\nBPA: " << cellBPA[i] << std::endl;;
    for (size_t ii=0; ii<numContIntervalVars; ii++) {
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


void NonDGlobalEvidence::get_best_sample(bool find_max, bool eval_approx)
{
  // Pull the samples and responses from data used to build latest GP
  // to determine fnStar for use in the expected improvement function

  const Pecos::SurrogateData& gp_data
    = fHatModel.approximation_data(respFnCntr);

  // GT:
  // We want to make sure that we pick a data point that lies inside the cell
  size_t i, j, index_star, num_data_pts = gp_data.size();
  truthFnStar = (find_max) ? -DBL_MAX : DBL_MAX;
  for (i=0; i<num_data_pts; ++i) {
    const Real&       truth_fn   = gp_data.response_function(i);
    const RealVector& truth_data = gp_data.continuous_variables(i);
    bool in_bounds = true;
    for (j=0; j < numContIntervalVars; ++j) {
      if ((truth_data[j] < cellLowerBounds[cellCntr][j]) ||
	  (truth_data[j] > cellUpperBounds[cellCntr][j])) {	
	in_bounds = false;
	break;
      }
    }
    if ( ( (  find_max && truth_fn > truthFnStar ) ||
	   ( !find_max && truth_fn < truthFnStar ) ) && in_bounds ){
      index_star  = i;
      truthFnStar = truth_fn;
    }
  }

  if (eval_approx) {
    if ((truthFnStar == DBL_MAX) || (truthFnStar == -DBL_MAX)) {
      Cout << "No function evaluations were found in cell. "
	   << "Truth function is set to DBL_MAX and approxFnStar is evaluated "
	   << "at midpoint.\n";
      RealVector midpoint;
      Variables cell_midpoint(gpOptimizer.variables_results());
      midpoint.size(numContIntervalVars);
      for (size_t i=0; i<numContIntervalVars; i++) {
	midpoint[i] = 0.5*cellLowerBounds[cellCntr][i]
	            + 0.5*cellUpperBounds[cellCntr][i];
      }	
      fHatModel.continuous_variables(midpoint);
    }
    else
      fHatModel.continuous_variables(gp_data.continuous_variables(index_star));
		
    ActiveSet set = fHatModel.current_response().active_set();
    set.request_values(0); set.request_value(1, respFnCntr);
    fHatModel.compute_response(set);
    approxFnStar = fHatModel.current_response().function_value(respFnCntr);
  }
#ifdef DEBUG
  Cout << "truthFnStar: " << truthFnStar << "\napproxFnStar: " << approxFnStar
       << std::endl;
#endif
}

} // namespace Dakota
