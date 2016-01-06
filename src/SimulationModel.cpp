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
  solnControlVarIndex(0), solnControlVarType(EMPTY_TYPE)
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
  
  // find the variable label used for solution control within the
  // inactive discrete variables
  solnControlVarIndex = find_index(
    currentVariables.inactive_discrete_int_variable_labels(), control);
  if (solnControlVarIndex != _NPOS)
    solnControlVarType = currentVariables.
      inactive_discrete_int_variable_types()[solnControlVarIndex];
  else {
    solnControlVarIndex = find_index(
      currentVariables.inactive_discrete_string_variable_labels(), control);
    if (solnControlVarIndex != _NPOS)
      solnControlVarType = currentVariables.
	inactive_discrete_string_variable_types()[solnControlVarIndex];
    else {
      solnControlVarIndex = find_index(
	currentVariables.inactive_discrete_real_variable_labels(), control);
      if (solnControlVarIndex != _NPOS)
	solnControlVarType = currentVariables.
	  inactive_discrete_real_variable_types()[solnControlVarIndex];
     else {
	Cerr << "Error: solution_level_control string identifier not found "
	     << "within inactive discrete variable labels." << std::endl;
	abort_handler(MODEL_ERROR);
      }
    }
  }

  // get size of corresponding set values
  size_t i, num_lev, offset;
  switch (solnControlVarType) {
  case DISCRETE_DESIGN_RANGE: case DISCRETE_INTERVAL_UNCERTAIN:
  case DISCRETE_STATE_RANGE:
    num_lev = userDefinedConstraints.inactive_discrete_int_upper_bounds()
              [solnControlVarIndex]
            - userDefinedConstraints.inactive_discrete_int_lower_bounds()
              [solnControlVarIndex];
    break;
  case DISCRETE_DESIGN_SET_INT:
    offset = find_index(currentVariables.inactive_discrete_int_variable_types(),
			DISCRETE_DESIGN_SET_INT);
    num_lev = discreteDesignSetIntValues[solnControlVarIndex-offset].size();
    break;
  case DISCRETE_DESIGN_SET_STRING:
    offset = find_index(
      currentVariables.inactive_discrete_string_variable_types(),
      DISCRETE_DESIGN_SET_STRING);
    num_lev = discreteDesignSetStringValues[solnControlVarIndex-offset].size();
    break;
  case DISCRETE_DESIGN_SET_REAL:
    offset = find_index(
      currentVariables.inactive_discrete_real_variable_types(),
      DISCRETE_DESIGN_SET_REAL);
    num_lev = discreteDesignSetRealValues[solnControlVarIndex-offset].size();
    break;
  case DISCRETE_UNCERTAIN_SET_INT:
    offset = find_index(currentVariables.inactive_discrete_int_variable_types(),
			DISCRETE_UNCERTAIN_SET_INT);
    num_lev = epistDistParams.discrete_set_int_values_probabilities()
      [solnControlVarIndex-offset].size();
    break;
  case DISCRETE_UNCERTAIN_SET_STRING:
    offset = find_index(
      currentVariables.inactive_discrete_string_variable_types(),
      DISCRETE_UNCERTAIN_SET_STRING);
    num_lev = epistDistParams.discrete_set_string_values_probabilities()
      [solnControlVarIndex-offset].size();
    break;
  case DISCRETE_UNCERTAIN_SET_REAL:
    offset = find_index(
      currentVariables.inactive_discrete_real_variable_types(),
      DISCRETE_UNCERTAIN_SET_REAL);
    num_lev = epistDistParams.discrete_set_real_values_probabilities()
      [solnControlVarIndex-offset].size();
    break;
  case DISCRETE_STATE_SET_INT:
    offset = find_index(currentVariables.inactive_discrete_int_variable_types(),
			DISCRETE_STATE_SET_INT);
    num_lev = discreteStateSetIntValues[solnControlVarIndex-offset].size();
    break;
  case DISCRETE_STATE_SET_STRING:
    offset = find_index(
      currentVariables.inactive_discrete_string_variable_types(),
      DISCRETE_STATE_SET_STRING);
    num_lev = discreteStateSetStringValues[solnControlVarIndex-offset].size();
    break;
  case DISCRETE_STATE_SET_REAL:
    offset = find_index(
      currentVariables.inactive_discrete_real_variable_types(),
      DISCRETE_STATE_SET_REAL);
    num_lev = discreteStateSetRealValues[solnControlVarIndex-offset].size();
    break;
  default:
    Cerr << "Error: unsupported variable type in SimulationModel::"
	 << "initialize_solution_control" << std::endl;
    abort_handler(MODEL_ERROR); break;
  }
  
  // process cost array to create ordered map.
  // Specified set values are sorted on input, but specified costs are not
  // --> solnControlCostMap sorts on cost key and maps to the unordered index
  //     of these sorted values.
  // > Example 1:
  //     model simulation
  //       solution_level_control = 'dssiv1'
  //       solution_level_cost = 10. 2. 200.
  //   results in solnControlCostMap = { {2., 1}, {10., 0}, {200., 2} }
  // > Example 2:
  //     model simulation
  //       solution_level_control = 'dssiv1'
  //       solution_level_cost = 10. # scalar multiplier
  // results in solnControlCostMap = { {1., 0}, {10., 1}, {100., 2} }
  if (cost.length() == 1) {
    Real multiplier = cost[0], prod = 1.;
    for (i=0; i<num_lev; ++i) { // assume increasing cost
      solnControlCostMap.insert(std::pair<Real, size_t>(prod, i));//TO DO:ranges
      prod *= multiplier;
    }
  }
  else if (cost.length() == num_lev)
    for (i=0; i<num_lev; ++i)
      solnControlCostMap.insert(std::pair<Real, size_t>(cost[i], i));//TO DO
  else {
    Cerr << "Error: solution_level_cost specification of length "
	 << cost.length() << " provided;\n       expected scalar or vector "
	 << "of length " << num_lev << "." << std::endl;
    abort_handler(MODEL_ERROR);
  }
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
