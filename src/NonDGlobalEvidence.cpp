/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "NonDGlobalEvidence.hpp"
#include "dakota_data_types.hpp"
#include "dakota_system_defs.hpp"
#include "DakotaApproximation.hpp"


namespace Dakota {

NonDGlobalEvidence::NonDGlobalEvidence(ProblemDescDB& problem_db, Model& model):
  NonDGlobalInterval(problem_db, model)
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
  size_t j; 
  for (j=0; j<numContIntervalVars; ++j) {
    ModelUtils::continuous_lower_bound(intervalOptModel, cellContLowerBounds[cellCntr][j],j);
    ModelUtils::continuous_upper_bound(intervalOptModel, cellContUpperBounds[cellCntr][j],j);
  }
   
  for (j=0; j<numDiscIntervalVars; ++j) {
    intervalOptModel.discrete_int_lower_bound(
      cellIntRangeLowerBounds[cellCntr][j], j);
    intervalOptModel.discrete_int_upper_bound(
      cellIntRangeUpperBounds[cellCntr][j], j);
  } 

  for (j=0; j<numDiscSetIntUncVars; ++j)
    intervalOptModel.current_variables().discrete_int_variable(cellIntSetBounds[cellCntr][j],
					   j+numDiscIntervalVars);

  for (j=0; j<numDiscSetRealUncVars; ++j)
    intervalOptModel.current_variables().discrete_real_variable(cellRealSetBounds[cellCntr][j],j);
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
  size_t i, j;
  for (i=0; i<numCells; ++i) {
    Cout << "Cell " << i << "\nBPA: " << cellBPA[i] << std::endl;;
    for (j=0, j<numContIntervalVars; ++j)
      Cout << "Cell bounds for continuous variable " << j << ": ("
	   << cellContLowerBounds[i][j] << ", "
	   << cellContUpperBounds[i][j] << ")\n";
    for (j=0; j<numDiscIntervalVars; ++j)
      Cout << "Cell bounds for discrete int range variable " << j << ": ("
	   << cellIntRangeLowerBounds[i][j] << ", "
	   << cellIntRangeUpperBounds[i][j] << ")\n";
    for (j=0; j<numDiscSetIntUncVars; ++j)
      Cout << "Cell value for discrete int set variable " << j << ": "
	   << cellIntSetBounds[i][j] << '\n';
    for (j=0; j<numDiscSetRealUncVars; ++j)
      Cout << "Cell value for discrete real set variable " << j << ": "
	   << cellRealSetBounds[i][j] << '\n';
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
  const Pecos::SDVArray& sdv_array = gp_data.variables_data();
  const Pecos::SDRArray& sdr_array = gp_data.response_data();

  // GT:
  // We want to make sure that we pick a data point that lies inside the cell
  size_t i, j, index_star, num_data_pts = gp_data.points();
  truthFnStar = (maximize) ? -DBL_MAX : DBL_MAX;
  for (i=0; i<num_data_pts; ++i) {
    const Real&      truth_fn = sdr_array[i].response_function();
    const Pecos::SurrogateDataVars& sdv = sdv_array[i];
    const RealVector&  c_vars = sdv.continuous_variables();
    const IntVector&  di_vars = sdv.discrete_int_variables();
    const RealVector& dr_vars = sdv.discrete_real_variables();
    bool in_bounds = true;
    for (j=0; j<numContIntervalVars; ++j)
      if (c_vars[j] < cellContLowerBounds[cellCntr][j] ||
	  c_vars[j] > cellContUpperBounds[cellCntr][j])
	{ in_bounds = false; break; }
    if (in_bounds)
      for (j=0; j<numDiscIntervalVars; ++j)
	if (di_vars[j] < cellIntRangeLowerBounds[cellCntr][j] ||
	    di_vars[j] > cellIntRangeUpperBounds[cellCntr][j])
	  { in_bounds = false; break; }
    if (in_bounds)
      for (j=0; j<numDiscSetIntUncVars; ++j)
	if (di_vars[j+numDiscIntervalVars] != cellIntSetBounds[cellCntr][j])
	  { in_bounds = false; break; }
    if (in_bounds)
      for (j=0; j<numDiscSetRealUncVars; ++j)
	if (dr_vars[j] != cellRealSetBounds[cellCntr][j])
	  { in_bounds = false; break; }

    if ( in_bounds && ( (  maximize && truth_fn > truthFnStar ) ||
			( !maximize && truth_fn < truthFnStar ) ) ) {
      index_star  = i;
      truthFnStar = truth_fn;
    }
  }

  if (eval_approx) {
    if ( ( !maximize && truthFnStar ==  DBL_MAX ) ||
	 (  maximize && truthFnStar == -DBL_MAX ) ) {
      Cout << "No function evaluations were found in cell. Truth function is "
	   << "set to DBL_MAX and approxFnStar is evaluated at midpoint.\n";
      for (i=0; i<numContIntervalVars; ++i)
	fHatModel.current_variables().continuous_variable(
	  ( cellContLowerBounds[cellCntr][i] + 
	    cellContUpperBounds[cellCntr][i] ) / 2., i);
      for (i=0; i<numDiscIntervalVars; ++i)
	fHatModel.current_variables().discrete_int_variable(
	  ( cellIntRangeLowerBounds[cellCntr][i] + 
	    cellIntRangeUpperBounds[cellCntr][i] ) / 2, i);
      for (i=0; i<numDiscSetIntUncVars; ++i)
	fHatModel.current_variables().discrete_int_variable(
	  cellIntSetBounds[cellCntr][i], i+numDiscIntervalVars);
      for (i=0; i<numDiscSetRealUncVars; ++i)
	fHatModel.current_variables().discrete_real_variable(
	  cellRealSetBounds[cellCntr][i], i);
    }
    else {
      const Pecos::SurrogateDataVars& sdv = sdv_array[index_star];
      if (numContIntervalVars)
	fHatModel.current_variables().continuous_variables(sdv.continuous_variables());
      if (numDiscIntervalVars || numDiscSetIntUncVars)
	fHatModel.current_variables().discrete_int_variables(sdv.discrete_int_variables());
      if (numDiscSetRealUncVars)
	fHatModel.current_variables().discrete_real_variables(sdv.discrete_real_variables());
    }
		
    ActiveSet set = fHatModel.current_response().active_set();
    set.request_values(0); set.request_value(1, respFnCntr);
    fHatModel.evaluate(set);
    approxFnStar = fHatModel.current_response().function_value(respFnCntr);
  }
#ifdef DEBUG
  Cout << "truthFnStar: " << truthFnStar << "\napproxFnStar: " << approxFnStar
       << std::endl;
#endif
}

} // namespace Dakota
