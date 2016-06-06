/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SimulationModel
//- Description: Implementation code for the SimulationModel class
//- Owner:       Mike Eldred
//- Checked by:

#include "dakota_system_defs.hpp"
#include "SimulationModel.hpp"
#include "ProblemDescDB.hpp"

static const char rcsId[]="@(#) $Id: SimulationModel.cpp 6492 2009-12-19 00:04:28Z briadam $";


using namespace std;

namespace Dakota {

// define special values for componentParallelMode
#define INTERFACE 1


SimulationModel::SimulationModel(ProblemDescDB& problem_db):
  Model(BaseConstructor(), problem_db),
  userDefinedInterface(problem_db.get_interface()),
  solnCntlVarType(EMPTY_TYPE), solnCntlADVIndex(0), solnCntlSetIndex(0),
  simModelEvalCntr(0)
{
  componentParallelMode = INTERFACE;
  ignoreBounds = problem_db.get_bool("responses.ignore_bounds");
  centralHess  = problem_db.get_bool("responses.central_hess");

  initialize_solution_control(
    problem_db.get_string("model.simulation.solution_level_control"),
    problem_db.get_rv("model.simulation.solution_level_cost"));
}


void SimulationModel::
initialize_solution_control(const String& control, const RealVector& cost)
{
  if (control.empty()) return;

  // find the variable label used for solution control within the discrete
  // variables (all view).  It must be a discrete variable so that the number
  // of levels is finite; however, the discrete values may be int, string, or
  // real.  It should not be an active variable, but may not be an inactive
  // variable (inactive view assigned from a higher level context).
  solnCntlADVIndex = find_index(
    currentVariables.all_discrete_int_variable_labels(), control);
  if (solnCntlADVIndex != _NPOS) {
    solnCntlVarType = currentVariables.
      all_discrete_int_variable_types()[solnCntlADVIndex];
    if (find_index(currentVariables.discrete_int_variable_labels(), control)
	!= _NPOS) {
      Cerr << "Error: solution_level_control cannot be an active variable."
	   << std::endl;
      abort_handler(MODEL_ERROR);
    }
  }
  else {
    solnCntlADVIndex = find_index(
      currentVariables.all_discrete_string_variable_labels(), control);
    if (solnCntlADVIndex != _NPOS) {
      solnCntlVarType = currentVariables.
	all_discrete_string_variable_types()[solnCntlADVIndex];
      if (find_index(currentVariables.discrete_string_variable_labels(),
	  control) != _NPOS) {
	Cerr << "Error: solution_level_control cannot be an active variable."
	     << std::endl;
	abort_handler(MODEL_ERROR);
      }
    }
    else {
      solnCntlADVIndex = find_index(
	currentVariables.all_discrete_real_variable_labels(), control);
      if (solnCntlADVIndex != _NPOS) {
	solnCntlVarType = currentVariables.
	  all_discrete_real_variable_types()[solnCntlADVIndex];
	if (find_index(currentVariables.discrete_real_variable_labels(),
	    control) != _NPOS) {
	  Cerr << "Error: solution_level_control cannot be an active variable."
	       << std::endl;
	  abort_handler(MODEL_ERROR);
	}
      }
      else {
	Cerr << "Error: solution_level_control string identifier not found "
	     << "within discrete variable labels." << std::endl;
	abort_handler(MODEL_ERROR);
      }
    }
  }

  // get size of corresponding set values
  size_t i, num_lev;
  switch (solnCntlVarType) {
  case DISCRETE_DESIGN_RANGE: case DISCRETE_INTERVAL_UNCERTAIN:
  case DISCRETE_STATE_RANGE:
    //solnCntlSetIndex = solnCntlADVIndex;
    num_lev =
      userDefinedConstraints.all_discrete_int_upper_bounds()[solnCntlADVIndex] -
      userDefinedConstraints.all_discrete_int_lower_bounds()[solnCntlADVIndex];
    break;
  case DISCRETE_DESIGN_SET_INT:
    solnCntlSetIndex = solnCntlADVIndex -
      find_index(currentVariables.all_discrete_int_variable_types(),
		 DISCRETE_DESIGN_SET_INT);
    num_lev = discreteDesignSetIntValues[solnCntlSetIndex].size();
    break;
  case DISCRETE_DESIGN_SET_STRING:
    solnCntlSetIndex = solnCntlADVIndex -
      find_index(currentVariables.all_discrete_string_variable_types(),
		 DISCRETE_DESIGN_SET_STRING);
    num_lev = discreteDesignSetStringValues[solnCntlSetIndex].size();
    break;
  case DISCRETE_DESIGN_SET_REAL:
    solnCntlSetIndex = solnCntlADVIndex -
      find_index(currentVariables.all_discrete_real_variable_types(),
		 DISCRETE_DESIGN_SET_REAL);
    num_lev = discreteDesignSetRealValues[solnCntlSetIndex].size();
    break;
  case DISCRETE_UNCERTAIN_SET_INT:
    solnCntlSetIndex = solnCntlADVIndex -
      find_index(currentVariables.all_discrete_int_variable_types(),
		 DISCRETE_UNCERTAIN_SET_INT);
    num_lev = epistDistParams.discrete_set_int_values_probabilities()
      [solnCntlSetIndex].size();
    break;
  case DISCRETE_UNCERTAIN_SET_STRING:
    solnCntlSetIndex = solnCntlADVIndex -
      find_index(currentVariables.all_discrete_string_variable_types(),
		 DISCRETE_UNCERTAIN_SET_STRING);
    num_lev = epistDistParams.discrete_set_string_values_probabilities()
      [solnCntlSetIndex].size();
    break;
  case DISCRETE_UNCERTAIN_SET_REAL:
    solnCntlSetIndex = solnCntlADVIndex -
      find_index(currentVariables.all_discrete_real_variable_types(),
		 DISCRETE_UNCERTAIN_SET_REAL);
    num_lev = epistDistParams.discrete_set_real_values_probabilities()
      [solnCntlSetIndex].size();
    break;
  case DISCRETE_STATE_SET_INT:
    solnCntlSetIndex = solnCntlADVIndex -
      find_index(currentVariables.all_discrete_int_variable_types(),
		 DISCRETE_STATE_SET_INT);
    num_lev = discreteStateSetIntValues[solnCntlSetIndex].size();
    break;
  case DISCRETE_STATE_SET_STRING:
    solnCntlSetIndex = solnCntlADVIndex -
      find_index(currentVariables.all_discrete_string_variable_types(),
		 DISCRETE_STATE_SET_STRING);
    num_lev = discreteStateSetStringValues[solnCntlSetIndex].size();
    break;
  case DISCRETE_STATE_SET_REAL:
    solnCntlSetIndex = solnCntlADVIndex -
      find_index(currentVariables.all_discrete_real_variable_types(),
		 DISCRETE_STATE_SET_REAL);
    num_lev = discreteStateSetRealValues[solnCntlSetIndex].size();
    break;
  default:
    Cerr << "Error: unsupported variable type in SimulationModel::"
	 << "initialize_solution_control" << std::endl;
    abort_handler(MODEL_ERROR); break;
  }
  
  // process cost array to create ordered map.
  // Specified set values are sorted on input, but specified costs are not
  // --> solnCntlCostMap sorts on cost key and maps to the unordered index
  //     of these sorted values.
  // > Example 1:
  //     model simulation
  //       solution_level_control = 'dssiv1'
  //       solution_level_cost = 10. 2. 200.
  //   results in solnCntlCostMap = { {2., 1}, {10., 0}, {200., 2} }
  // > Example 2:
  //     model simulation
  //       solution_level_control = 'dssiv1'
  //       solution_level_cost = 10. # scalar multiplier
  // results in solnCntlCostMap = { {1., 0}, {10., 1}, {100., 2} }
  if (cost.length() == num_lev)
    for (i=0; i<num_lev; ++i)
      solnCntlCostMap.insert(std::pair<Real, size_t>(cost[i], i));
  else if (cost.length() == 1) {
    Real multiplier = cost[0], prod = 1.;
    for (i=0; i<num_lev; ++i) { // assume increasing cost
      solnCntlCostMap.insert(std::pair<Real, size_t>(prod, i));
      prod *= multiplier;
    }
  }
  else {
    Cerr << "Error: solution_level_cost specification of length "
	 << cost.length() << " provided;\n       expected scalar or vector "
	 << "of length " << num_lev << "." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


/* Real */void SimulationModel::solution_level_index(size_t lev_index)
{
  // incoming soln level index is an index into the ordered std::map<>,
  // not to be confused with the value in the key-value map that corresponds
  // to the discrete variable value index (val_index below).
  std::map<Real, size_t>::const_iterator cost_cit = solnCntlCostMap.begin();
  std::advance(cost_cit, lev_index);
  size_t val_index = cost_cit->second;

  switch (solnCntlVarType) {
  case DISCRETE_DESIGN_RANGE: case DISCRETE_INTERVAL_UNCERTAIN:
  case DISCRETE_STATE_RANGE: {
    int val = val_index + userDefinedConstraints.
      all_discrete_int_lower_bounds()[solnCntlADVIndex];
    currentVariables.all_discrete_int_variable(val, solnCntlADVIndex);
    break;
  }
  //////////////////////////////
  case DISCRETE_DESIGN_SET_INT: {
    ISCIter cit = discreteDesignSetIntValues[solnCntlSetIndex].begin();
    std::advance(cit, val_index);
    currentVariables.all_discrete_int_variable(*cit, solnCntlADVIndex);
    break;
  }
  case DISCRETE_DESIGN_SET_STRING: {
    SSCIter cit = discreteDesignSetStringValues[solnCntlSetIndex].begin();
    std::advance(cit, val_index);
    currentVariables.all_discrete_string_variable(*cit, solnCntlADVIndex);
    break;
  }
  case DISCRETE_DESIGN_SET_REAL: {
    RSCIter cit = discreteDesignSetRealValues[solnCntlSetIndex].begin();
    std::advance(cit, val_index);
    currentVariables.all_discrete_real_variable(*cit, solnCntlADVIndex);
    break;
  }
  //////////////////////////////
  case DISCRETE_UNCERTAIN_SET_INT: {
    IRMCIter cit = epistDistParams.discrete_set_int_values_probabilities()
      [solnCntlSetIndex].begin();
    std::advance(cit, val_index);
    currentVariables.all_discrete_int_variable(cit->first, solnCntlADVIndex);
    break;
  }
  case DISCRETE_UNCERTAIN_SET_STRING: {
    SRMCIter cit = epistDistParams.discrete_set_string_values_probabilities()
      [solnCntlSetIndex].begin();
    std::advance(cit, val_index);
    currentVariables.all_discrete_string_variable(cit->first, solnCntlADVIndex);
    break;
  }
  case DISCRETE_UNCERTAIN_SET_REAL: {
    RRMCIter cit = epistDistParams.discrete_set_real_values_probabilities()
      [solnCntlSetIndex].begin();
    std::advance(cit, val_index);
    currentVariables.all_discrete_real_variable(cit->first, solnCntlADVIndex);
    break;
  }
  //////////////////////////////
  case DISCRETE_STATE_SET_INT: {
    ISCIter cit = discreteStateSetIntValues[solnCntlSetIndex].begin();
    std::advance(cit, val_index);
    currentVariables.all_discrete_int_variable(*cit, solnCntlADVIndex);
    break;
  }
  case DISCRETE_STATE_SET_STRING: {
    SSCIter cit = discreteStateSetStringValues[solnCntlSetIndex].begin();
    std::advance(cit, val_index);
    currentVariables.all_discrete_string_variable(*cit, solnCntlADVIndex);
    break;
  }
  case DISCRETE_STATE_SET_REAL: {
    RSCIter cit = discreteStateSetRealValues[solnCntlSetIndex].begin();
    std::advance(cit, val_index);
    currentVariables.all_discrete_real_variable(*cit, solnCntlADVIndex);
    break;
  }
  }

  //return cost_cit->first; // cost estimate for this solution level index
}


RealVector SimulationModel::solution_level_cost() const
{
  RealVector cost_levels(solnCntlCostMap.size(), false);
  std::map<Real, size_t>::const_iterator cit; size_t i;
  for (cit=solnCntlCostMap.begin(), i=0; cit!=solnCntlCostMap.end(); ++cit, ++i)
    cost_levels[i] = cit->first;
  return cost_levels;
}


void SimulationModel::component_parallel_mode(short mode)
{
  if (mode != INTERFACE) {
    Cerr << "Error: SimulationModel only supports the INTERFACE component "
	 << "parallel mode." << std::endl;
    abort_handler(MODEL_ERROR);
  }
  parallelLib.parallel_configuration_iterator(modelPCIter);
  //componentParallelMode = mode;
}


/** SimulationModel doesn't need to change the tagging, so just forward to
    Interface */
void SimulationModel::eval_tag_prefix(const String& eval_id_str)
{
  // Simulation model uses the counter from the interface
  bool append_iface_id = true;
  userDefinedInterface.eval_tag_prefix(eval_id_str, append_iface_id);
}

} // namespace Dakota
