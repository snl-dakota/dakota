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
#include "MarginalsCorrDistribution.hpp"

static const char rcsId[]="@(#) $Id: SimulationModel.cpp 6492 2009-12-19 00:04:28Z briadam $";


using namespace std;

namespace Dakota {

// define special values for componentParallelMode
#define INTERFACE 1


SimulationModel::SimulationModel(ProblemDescDB& problem_db):
  Model(BaseConstructor(), problem_db),
  userDefinedInterface(problem_db.get_interface()),
  solnCntlVarType(EMPTY_TYPE), solnCntlADVIndex(0), solnCntlRVIndex(0),
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
  solnCntlCostMap.clear();

  size_t cost_len = cost.length();
  if (control.empty()) {
    // cost_len of 0: empty map for no solution control
    // cost_len of 1: nominal cost for model w/o any soln levels
    // cost_len  > 1: error
    if (cost_len == 1)   // nominal cost with no solution control
      solnCntlCostMap.insert(std::pair<Real, size_t>(cost[0], _NPOS));
    else if (cost_len) { // more than 1 cost requires associated control
      Cerr << "Error: vector-valued solution cost requires an associated "
	   << "solution control." << std::endl;
      abort_handler(MODEL_ERROR);
    }
    return;
  }

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
  Pecos::MarginalsCorrDistribution* mvd_rep
    = (Pecos::MarginalsCorrDistribution*)mvDist.multivar_dist_rep();
  const SharedVariablesData& svd = currentVariables.shared_data();
  switch (solnCntlVarType) {
  case DISCRETE_DESIGN_RANGE: case DISCRETE_INTERVAL_UNCERTAIN:
  case DISCRETE_STATE_RANGE:
    //solnCntlRVIndex = svd.adiv_index_to_all_index(solnCntlADVIndex);
    num_lev =
      userDefinedConstraints.all_discrete_int_upper_bounds()[solnCntlADVIndex] -
      userDefinedConstraints.all_discrete_int_lower_bounds()[solnCntlADVIndex];
    break;
  case DISCRETE_DESIGN_SET_INT: case DISCRETE_STATE_SET_INT:
    solnCntlRVIndex = svd.adiv_index_to_all_index(solnCntlADVIndex);
    num_lev = mvd_rep->
      pull_parameter_size<IntSet>(solnCntlRVIndex, Pecos::DSI_VALUES);
    break;
  case DISCRETE_DESIGN_SET_STRING: case DISCRETE_STATE_SET_STRING:
    solnCntlRVIndex = svd.adsv_index_to_all_index(solnCntlADVIndex);
    num_lev = mvd_rep->
      pull_parameter_size<StringSet>(solnCntlRVIndex, Pecos::DSS_VALUES);
    break;
  case DISCRETE_DESIGN_SET_REAL: case DISCRETE_STATE_SET_REAL:
    solnCntlRVIndex = svd.adrv_index_to_all_index(solnCntlADVIndex);
    num_lev = mvd_rep->
      pull_parameter_size<RealSet>(solnCntlRVIndex, Pecos::DSR_VALUES);
    break;
  case DISCRETE_UNCERTAIN_SET_INT:
    solnCntlRVIndex = svd.adiv_index_to_all_index(solnCntlADVIndex);
    num_lev = mvd_rep->
      pull_parameter_size<IntRealMap>(solnCntlRVIndex,Pecos::DUSI_VALUES_PROBS);
    break;
  case DISCRETE_UNCERTAIN_SET_STRING:
    solnCntlRVIndex = svd.adsv_index_to_all_index(solnCntlADVIndex);
    num_lev = mvd_rep->
      pull_parameter_size<StringRealMap>(solnCntlRVIndex,
					 Pecos::DUSS_VALUES_PROBS);
    break;
  case DISCRETE_UNCERTAIN_SET_REAL:
    solnCntlRVIndex = svd.adrv_index_to_all_index(solnCntlADVIndex);
    num_lev = mvd_rep->
      pull_parameter_size<RealRealMap>(solnCntlRVIndex,
				       Pecos::DUSR_VALUES_PROBS);
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


/* Real */void SimulationModel::solution_level_index(unsigned short lev_index)
{
  // incoming soln level index is an index into the ordered solnCntlCostMap,
  // not to be confused with the value in the key-value pair that corresponds
  // to the discrete variable value index (val_index below).
  if (lev_index == USHRT_MAX) { // just return quietly to simplify calling code
    return;                     // (rather than always checking index validity)

    //Cerr << "Error: USHRT_MAX passed to SimulationModel::"
    //     << "solution_level_index()." << std::endl;
    //abort_handler(MODEL_ERROR);
  }

  std::map<Real, size_t>::const_iterator c_cit = solnCntlCostMap.begin();
  std::advance(c_cit, lev_index);
  size_t val_index = c_cit->second;

  Pecos::MarginalsCorrDistribution* mvd_rep
    = (Pecos::MarginalsCorrDistribution*)mvDist.multivar_dist_rep();
  switch (solnCntlVarType) {
  case DISCRETE_DESIGN_RANGE: case DISCRETE_INTERVAL_UNCERTAIN:
  case DISCRETE_STATE_RANGE: {
    int val = val_index + userDefinedConstraints.
      all_discrete_int_lower_bounds()[solnCntlADVIndex];
    currentVariables.all_discrete_int_variable(val, solnCntlADVIndex);
    break;
  }
  //////////////////////////////
  case DISCRETE_DESIGN_SET_INT: case DISCRETE_STATE_SET_INT: {
    IntSet is;
    mvd_rep->pull_parameter<IntSet>(solnCntlRVIndex, Pecos::DSI_VALUES, is);
    ISIter is_it = is.begin();  std::advance(is_it, val_index);
    currentVariables.all_discrete_int_variable(*is_it, solnCntlADVIndex);
    break;
  }
  case DISCRETE_DESIGN_SET_STRING: case DISCRETE_STATE_SET_STRING: {
    StringSet ss;
    mvd_rep->pull_parameter<StringSet>(solnCntlRVIndex, Pecos::DSS_VALUES, ss);
    SSCIter ss_it = ss.begin();  std::advance(ss_it, val_index);
    currentVariables.all_discrete_string_variable(*ss_it, solnCntlADVIndex);
    break;
  }
  case DISCRETE_DESIGN_SET_REAL: case DISCRETE_STATE_SET_REAL: {
    RealSet rs;
    mvd_rep->pull_parameter<RealSet>(solnCntlRVIndex, Pecos::DSR_VALUES, rs);
    RSIter rs_it = rs.begin();  std::advance(rs_it, val_index);
    currentVariables.all_discrete_real_variable(*rs_it, solnCntlADVIndex);
    break;
  }
  //////////////////////////////
  case DISCRETE_UNCERTAIN_SET_INT: {
    IntRealMap irm;
    mvd_rep->pull_parameter<IntRealMap>(solnCntlRVIndex,
					Pecos::DUSI_VALUES_PROBS, irm);
    IRMIter ir_it = irm.begin();  std::advance(ir_it, val_index);
    currentVariables.all_discrete_int_variable(ir_it->first, solnCntlADVIndex);
    break;
  }
  case DISCRETE_UNCERTAIN_SET_STRING: {
    StringRealMap srm;
    mvd_rep->pull_parameter<StringRealMap>(solnCntlRVIndex,
					   Pecos::DUSS_VALUES_PROBS, srm);
    SRMIter sr_it = srm.begin();  std::advance(sr_it, val_index);
    currentVariables.all_discrete_string_variable(sr_it->first,
						  solnCntlADVIndex);
    break;
  }
  case DISCRETE_UNCERTAIN_SET_REAL: {
    RealRealMap rrm;
    mvd_rep->pull_parameter<RealRealMap>(solnCntlRVIndex,
					 Pecos::DUSR_VALUES_PROBS, rrm);
    RRMIter rr_it = rrm.begin();  std::advance(rr_it, val_index);
    currentVariables.all_discrete_real_variable(rr_it->first, solnCntlADVIndex);
    break;
  }
  }

  //return c_cit->first; // cost estimate for this solution level index
}


unsigned short SimulationModel::solution_level_index() const
{
  size_t val_index;
  Pecos::MarginalsCorrDistribution* mvd_rep
    = (Pecos::MarginalsCorrDistribution*)mvDist.multivar_dist_rep();
  switch (solnCntlVarType) {
  case DISCRETE_DESIGN_RANGE: case DISCRETE_INTERVAL_UNCERTAIN:
  case DISCRETE_STATE_RANGE: {
    int val = currentVariables.all_discrete_int_variables()[solnCntlADVIndex],
      l_bnd = userDefinedConstraints.all_discrete_int_lower_bounds()
        [solnCntlADVIndex];
    val_index = (size_t)(val - l_bnd);
    break;
  }
  //////////////////////////////
  case DISCRETE_DESIGN_SET_INT: case DISCRETE_STATE_SET_INT: {
    IntSet is;
    mvd_rep->pull_parameter<IntSet>(solnCntlRVIndex, Pecos::DSI_VALUES, is);
    val_index = set_value_to_index(
      currentVariables.all_discrete_int_variables()[solnCntlADVIndex], is);
    break;
  }
  case DISCRETE_DESIGN_SET_STRING: case DISCRETE_STATE_SET_STRING: {
    StringSet ss;
    mvd_rep->pull_parameter<StringSet>(solnCntlRVIndex, Pecos::DSS_VALUES, ss);
    val_index = set_value_to_index(
      currentVariables.all_discrete_string_variables()[solnCntlADVIndex], ss);
    break;
  }
  case DISCRETE_DESIGN_SET_REAL: case DISCRETE_STATE_SET_REAL: {
    RealSet rs;
    mvd_rep->pull_parameter<RealSet>(solnCntlRVIndex, Pecos::DSR_VALUES, rs);
    val_index = set_value_to_index(
      currentVariables.all_discrete_real_variables()[solnCntlADVIndex], rs);
    break;
  }
  //////////////////////////////
  case DISCRETE_UNCERTAIN_SET_INT: {
    IntRealMap irm;
    mvd_rep->pull_parameter<IntRealMap>(solnCntlRVIndex,
					Pecos::DUSI_VALUES_PROBS, irm);
    val_index = map_key_to_index(
      currentVariables.all_discrete_int_variables()[solnCntlADVIndex], irm);
    break;
  }
  case DISCRETE_UNCERTAIN_SET_STRING: {
    StringRealMap srm;
    mvd_rep->pull_parameter<StringRealMap>(solnCntlRVIndex,
					   Pecos::DUSS_VALUES_PROBS, srm);
    val_index = map_key_to_index(
      currentVariables.all_discrete_string_variables()[solnCntlADVIndex], srm);
    break;
  }
  case DISCRETE_UNCERTAIN_SET_REAL: {
    RealRealMap rrm;
    mvd_rep->pull_parameter<RealRealMap>(solnCntlRVIndex,
					 Pecos::DUSR_VALUES_PROBS, rrm);
    val_index = map_key_to_index(
      currentVariables.all_discrete_real_variables()[solnCntlADVIndex], rrm);
    break;
  }
  //////////////////////////////
  default: // EMPTY_TYPE (no solution_level_control provided)
    return USHRT_MAX; break;
  }

  // convert val_index to lev_index and return
  size_t lev_index = map_value_to_index(val_index, solnCntlCostMap);
  return (lev_index == _NPOS) ? USHRT_MAX : (unsigned short)lev_index;
}


RealVector SimulationModel::solution_level_costs() const
{
  RealVector cost_levels(solnCntlCostMap.size(), false);
  std::map<Real, size_t>::const_iterator cit; size_t i;
  for (cit=solnCntlCostMap.begin(), i=0; cit!=solnCntlCostMap.end(); ++cit, ++i)
    cost_levels[i] = cit->first;
  return cost_levels;
}


Real SimulationModel::solution_level_cost() const
{
  std::map<Real, size_t>::const_iterator cit = solnCntlCostMap.begin();
  if (cit == solnCntlCostMap.end()) return 0.;
  else {
    unsigned short lev_index = solution_level_index();
    if (lev_index != USHRT_MAX)
      std::advance(cit, lev_index);
    return cit->first;
  }
}


/*
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
*/


/** SimulationModel doesn't need to change the tagging, so just forward to
    Interface */
void SimulationModel::eval_tag_prefix(const String& eval_id_str)
{
  // Simulation model uses the counter from the interface
  bool append_iface_id = true;
  userDefinedInterface.eval_tag_prefix(eval_id_str, append_iface_id);
}

/*ActiveSet SimulationModel::default_active_set() {
  ActiveSet set(numFns, numDerivVars);
  ShortArray asv(numFns, 1);
 
  if(gradientType != "none" && (gradientType == "analytic" || supportsEstimDerivs))
      for(auto &a : asv)
        a |=  2;

  if(hessianType != "none" && (hessianType == "analytic" || supportsEstimDerivs))
      for(auto &a : asv)
        a |=  4;

  set.request_vector(asv);
  return set;
}
*/
ActiveSet SimulationModel::default_interface_active_set() {
  // compute the default active set for the user-defined interface
  ActiveSet set;
  set.derivative_vector(currentVariables.all_continuous_variable_ids());
  bool has_deriv_vars = set.derivative_vector().size() != 0;
  ShortArray asv(numFns, 1);
  if(has_deriv_vars) {
    if(gradientType == "analytic") {
      for(auto &a : asv)
        a |=  2;
    } else if(gradientType == "mixed") {
      for(const auto &gi : gradIdAnalytic)
        asv[gi-1] |= 2;
    }

    if(hessianType == "analytic") {
      for(auto &a : asv)
        a |=  4;
    } else if(hessianType == "mixed") {
      for(const auto &hi : hessIdAnalytic)
        asv[hi-1] |= 4;
    }
  }
  set.request_vector(asv);
  return set;
}

void SimulationModel::declare_sources() {
  evaluationsDB.declare_source(modelId, modelType, interface_id(), "interface");
}
} // namespace Dakota
