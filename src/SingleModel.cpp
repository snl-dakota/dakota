/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SingleModel
//- Description: Implementation code for the SingleModel class
//- Owner:       Mike Eldred
//- Checked by:

#include "dakota_system_defs.hpp"
#include "SingleModel.hpp"
#include "ProblemDescDB.hpp"

static const char rcsId[]="@(#) $Id: SingleModel.cpp 6492 2009-12-19 00:04:28Z briadam $";


using namespace std;

namespace Dakota {

// define special values for componentParallelMode
#define INTERFACE 1


SingleModel::SingleModel(ProblemDescDB& problem_db):
  Model(BaseConstructor(), problem_db),
  userDefinedInterface(problem_db.get_interface())
{
  componentParallelMode = INTERFACE;
  ignoreBounds = problem_db.get_bool("responses.ignore_bounds");
  centralHess  = problem_db.get_bool("responses.central_hess");

  initialize_solution_control(
    problem_db.get_string("model.single.solution_level_control"),
    problem_db.get_rv("model.single.solution_level_cost"));
}


void SingleModel::
initialize_solution_control(const String& control, const RealVector& cost)
{
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
	abort_handler(-1);
      }
    }
  }

  // get size of corresponding set values
  size_t i, num_lev;
  switch (solnControlVarType) {
  case DISCRETE_DESIGN_RANGE: // TO DO
    break;
  case DISCRETE_DESIGN_SET_INT:
    num_lev = discreteDesignSetIntValues[solnControlVarIndex].size();    break;
  case DISCRETE_DESIGN_SET_STRING:
    num_lev = discreteDesignSetStringValues[solnControlVarIndex].size(); break;
  case DISCRETE_DESIGN_SET_REAL:
    num_lev = discreteDesignSetRealValues[solnControlVarIndex].size();   break;
  //case DISCRETE_INTERVAL_UNCERTAIN: case DISCRETE_UNCERTAIN_SET_INT:
  //case DISCRETE_UNCERTAIN_SET_STRING: case DISCRETE_UNCERTAIN_SET_REAL:
  //  break;
  case DISCRETE_STATE_RANGE: // TO DO
    break;
  case DISCRETE_STATE_SET_INT:
    num_lev = discreteStateSetIntValues[solnControlVarIndex].size();     break;
  case DISCRETE_STATE_SET_STRING:
    num_lev = discreteStateSetStringValues[solnControlVarIndex].size();  break;
  case DISCRETE_STATE_SET_REAL:
    num_lev = discreteStateSetRealValues[solnControlVarIndex].size();    break;
  }
  
  // process cost array to create ordered map
  for (i=0; i<num_lev; ++i)
    solnControlCostMap.insert(std::pair<Real, size_t>(cost[i], i));
}


void SingleModel::component_parallel_mode(short mode)
{
  if (mode != INTERFACE) {
    Cerr << "Error: SingleModel only supports the INTERFACE component parallel "
	 << "mode." << endl;
    abort_handler(-1);
  }
  parallelLib.parallel_configuration_iterator(modelPCIter);
  //componentParallelMode = mode;
}


/** SingleModel doesn't need to change the tagging, so just forward to
    Interface */
void SingleModel::eval_tag_prefix(const String& eval_id_str)
{
  // Single model uses the counter from the interface
  bool append_iface_id = true;
  userDefinedInterface.eval_tag_prefix(eval_id_str, append_iface_id);
}

} // namespace Dakota
