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
  // TO DO: av_cntr assumes all active variables are interval variables
  size_t j, v_cntr, av_cntr; 
  for (j=0, v_cntr=0, av_cntr=0; j<numContIntervalVars;
       ++j, ++v_cntr, ++av_cntr) {
    intervalOptModel.continuous_lower_bound(cellLowerBounds[cellCntr][v_cntr],
					    av_cntr);
    intervalOptModel.continuous_upper_bound(cellUpperBounds[cellCntr][v_cntr],
					    av_cntr);
  }
   
  for (j=0, av_cntr=0; j<numDiscIntervalVars; ++j, ++v_cntr, ++av_cntr) {
    intervalOptModel.discrete_int_lower_bound(cellLowerBounds[cellCntr][v_cntr],
					      av_cntr);
    intervalOptModel.discrete_int_upper_bound(cellUpperBounds[cellCntr][v_cntr],
					      av_cntr);
  } 

  for (j=0; j<numDiscSetIntUncVars; ++j, ++v_cntr, ++av_cntr)
    intervalOptModel.discrete_int_variable(cellLowerBounds[cellCntr][v_cntr],
					   av_cntr);

  for (j=0, av_cntr=0; j<numDiscSetRealUncVars; ++j, ++v_cntr, ++av_cntr)
    intervalOptModel.discrete_real_variable(cellUpperBounds[cellCntr][v_cntr],
					    av_cntr);
}


void NonDGlobalEvidence::post_process_cell_results(bool maximize)
{
  if (maximize)
    cellFnUpperBounds[respFnCntr][cellCntr] = truthFnStar;
  else
    cellFnLowerBounds[respFnCntr][cellCntr] = truthFnStar;
}


void NonDGlobalEvidence::post_process_response_fn_results()
{ calculate_cbf_cpf(); }


void NonDGlobalEvidence::post_process_final_results()
{
#ifdef DEBUG
  size_t i, j, v_cntr;
  for (i=0; i<numCells; ++i) {
    Cout << "Cell " << i << "\nBPA: " << cellBPA[i] << std::endl;;
    for (j=0, v_cntr=0; j<numContIntervalVars; ++j, ++v_cntr)
      Cout << "Cell bounds for continuous variable " << j << ": ("
	   << cellLowerBounds[i][v_cntr] << ", "
	   << cellUpperBounds[i][v_cntr] << ")\n";
    for (j=0; j<numDiscIntervalVars; ++j, ++v_cntr)
      Cout << "Cell bounds for discrete int range variable " << j << ": ("
	   << (int)cellLowerBounds[i][v_cntr] << ", "
	   << (int)cellUpperBounds[i][v_cntr] << ")\n"; // TO DO
    for (j=0; j<numDiscSetIntUncVars; ++j, ++v_cntr)
      Cout << "Cell value for discrete int set variable " << j << ": "
	   << (int)cellLowerBounds[i][v_cntr] << '\n'; // TO DO
    for (j=0; j<numDiscSetRealUncVars; ++j, ++v_cntr)
      Cout << "Cell value for discrete real set variable " << j << ": "
	   << cellLowerBounds[i][v_cntr] << '\n'; // TO DO
    for (j=0; j<numFunctions; ++j)
      Cout << "Response fn " << j << " (min,max) for cell " << i << ": ("
	   << cellFnLowerBounds[j][i] << ", " << cellFnUpperBounds[j][i]
	   << ")\n";
  }
#endif
  // compute statistics 
  compute_evidence_statistics();
}


void NonDGlobalEvidence::get_best_sample(bool maximize, bool eval_approx)
{
  // Pull the samples and responses from data used to build latest GP
  // to determine fnStar for use in the expected improvement function

  const Pecos::SurrogateData& gp_data
    = fHatModel.approximation_data(respFnCntr);

  // GT:
  // We want to make sure that we pick a data point that lies inside the cell
  size_t i, j, v_cntr, av_cntr, index_star, num_data_pts = gp_data.size();
  truthFnStar = (maximize) ? -DBL_MAX : DBL_MAX;
  for (i=0; i<num_data_pts; ++i) {
    const Real&      truth_fn = gp_data.response_function(i);
    const RealVector&  c_vars = gp_data.continuous_variables(i);
    const IntVector&  di_vars = gp_data.discrete_int_variables(i);
    const RealVector& dr_vars = gp_data.discrete_real_variables(i);
    bool in_bounds = true;
    for (j=0, v_cntr=0; j<numContIntervalVars; ++j, ++v_cntr)
      if (c_vars[j] < cellLowerBounds[cellCntr][v_cntr] ||
	  c_vars[j] > cellUpperBounds[cellCntr][v_cntr])
	{ in_bounds = false; break; }
    if (in_bounds)
      for (j=0; j<numDiscIntervalVars; ++j, ++v_cntr)
	if (di_vars[j] < (int)cellLowerBounds[cellCntr][v_cntr] ||
	    di_vars[j] > (int)cellUpperBounds[cellCntr][v_cntr]) // TO DO
	  { in_bounds = false; break; }
    if (in_bounds)
      for (j=0; j<numDiscSetIntUncVars; ++j, ++v_cntr)
	if (di_vars[j] != (int)cellLowerBounds[cellCntr][v_cntr]) // TO DO
	  { in_bounds = false; break; }
    if (in_bounds)
      for (j=0; j<numDiscSetRealUncVars; ++j, ++v_cntr)
	if (dr_vars[j] != cellLowerBounds[cellCntr][v_cntr]) // TO DO
	  { in_bounds = false; break; }

    if ( in_bounds && ( (  maximize && truth_fn > truthFnStar ) ||
			( !maximize && truth_fn < truthFnStar ) ) ) {
      index_star  = i;
      truthFnStar = truth_fn;
    }
  }

  if (eval_approx) {
    if (truthFnStar == DBL_MAX || truthFnStar == -DBL_MAX) {
      Cout << "No function evaluations were found in cell. Truth function is "
	   << "set to DBL_MAX and approxFnStar is evaluated at midpoint.\n";
      for (i=0, v_cntr=0, av_cntr=0; i<numContIntervalVars;
	   ++i, ++v_cntr, ++av_cntr)
	fHatModel.continuous_variable(
	  ( cellLowerBounds[cellCntr][v_cntr] + 
	    cellUpperBounds[cellCntr][v_cntr] ) / 2., av_cntr);
      for (i=0, av_cntr=0; i<numDiscIntervalVars; ++i, ++v_cntr, ++av_cntr)
	fHatModel.discrete_int_variable(
	  (int)( cellLowerBounds[cellCntr][v_cntr] + 
		 cellUpperBounds[cellCntr][v_cntr] ) / 2, av_cntr); // TO DO
      for (i=0; i<numDiscSetIntUncVars; ++i, ++v_cntr, ++av_cntr)
	fHatModel.discrete_int_variable(
	  (int)cellLowerBounds[cellCntr][v_cntr], av_cntr); // TO DO
      for (i=0, av_cntr=0; i<numDiscSetRealUncVars; ++i, ++v_cntr, ++av_cntr)
	fHatModel.discrete_real_variable(
	  cellLowerBounds[cellCntr][v_cntr], av_cntr); // TO DO
    }
    else {
      if (numContIntervalVars)
	fHatModel.continuous_variables(
	  gp_data.continuous_variables(index_star));
      if (numDiscIntervalVars || numDiscSetIntUncVars)
	fHatModel.discrete_int_variables(
          gp_data.discrete_int_variables(index_star));
      if (numDiscSetRealUncVars)
	fHatModel.discrete_real_variables(
          gp_data.discrete_real_variables(index_star));
    }
		
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
