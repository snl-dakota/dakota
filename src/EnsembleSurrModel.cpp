/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       EnsembleSurrModel
//- Description: Implementation code for the EnsembleSurrModel class
//- Owner:       Mike Eldred
//- Checked by:

#include "EnsembleSurrModel.hpp"
#include "ProblemDescDB.hpp"

static const char rcsId[]=
  "@(#) $Id: EnsembleSurrModel.cpp 6656 2010-02-26 05:20:48Z mseldre $";

namespace Dakota {

extern Model dummy_model; // defined in DakotaModel.cpp


EnsembleSurrModel::EnsembleSurrModel(ProblemDescDB& problem_db):
  SurrogateModel(problem_db), sameModelInstance(false),
  sameInterfaceInstance(false), mfPrecedence(true), modeKeyBufferSize(0)
{
  // Ensemble surrogate models pass through numerical derivatives
  supportsEstimDerivs = false;
  // initialize ignoreBounds even though it's irrelevant for pass through
  ignoreBounds = problem_db.get_bool("responses.ignore_bounds");
  // initialize centralHess even though it's irrelevant for pass through
  centralHess = problem_db.get_bool("responses.central_hess");
}


void EnsembleSurrModel::init_model(Model& model)
{
  SurrogateModel::init_model(model);

  // Rather than map all inactive variables, propagate nested mappings from
  // currentVariables into the target model
  init_model_mapped_variables(model);
  //init_model_mapped_labels(model);
}


void EnsembleSurrModel::init_model_mapped_variables(Model& model)
{
  /*
  // can implement this once a use-case exists
  if (secondaryACVarMapTargets.size()  || secondaryADIVarMapTargets.size() ||
      secondaryADSVarMapTargets.size() || secondaryADRVarMapTargets.size()) {
    Cerr << "Error: secondary mappings not yet supported in SurrogateModel::"
	 << "init_model_mapped_variables()." << std::endl;
    abort_handler(MODEL_ERROR);
  }
  */

  size_t i, num_map = primaryACVarMapIndices.size(), // all sizes are the same
    ac_index1, adi_index1, ads_index1, adr_index1, m_index;
  for (i=0; i<num_map; ++i) {
    ac_index1  = primaryACVarMapIndices[i];
    adi_index1 = primaryADIVarMapIndices[i];
    ads_index1 = primaryADSVarMapIndices[i];
    adr_index1 = primaryADRVarMapIndices[i];
    if (ac_index1 != _NPOS) {
      // retrieve the label of the mapped variable from currentVariables
      const String& surr_label
	= currentVariables.all_continuous_variable_labels()[ac_index1];
      // map this to sub-ordinate label variables
      m_index = find_index(model.all_continuous_variable_labels(), surr_label);
      // push value from currentVariables to sub-ordinate variables
      if (m_index != _NPOS)
	model.all_continuous_variable(
	  currentVariables.all_continuous_variables()[ac_index1], m_index);
    }
    else if (adi_index1 != _NPOS) {
      const String& surr_label
	= currentVariables.all_discrete_int_variable_labels()[adi_index1];
      m_index = find_index(model.all_discrete_int_variable_labels(),surr_label);
      if (m_index != _NPOS)
	model.all_discrete_int_variable(
	  currentVariables.all_discrete_int_variables()[adi_index1], m_index);
    }
    else if (ads_index1 != _NPOS) {
      const String& surr_label
	= currentVariables.all_discrete_string_variable_labels()[ads_index1];
      m_index = find_index(model.all_discrete_string_variable_labels(),
			   surr_label);
      if (m_index != _NPOS)
	model.all_discrete_string_variable(
	  currentVariables.all_discrete_string_variables()[ads_index1],m_index);
    }
    else if (adr_index1 != _NPOS) {
      const String& surr_label
	= currentVariables.all_discrete_real_variable_labels()[adr_index1];
      m_index = find_index(model.all_discrete_real_variable_labels(),
			   surr_label);
      if (m_index != _NPOS)
	model.all_discrete_real_variable(
	  currentVariables.all_discrete_real_variables()[adr_index1], m_index);
    }
    else {
      Cerr << "Error: undefined mapping in SurrogateModel::"
	   << "init_model_mapped_variables()." << std::endl;
      abort_handler(MODEL_ERROR);
    }
  }
}


/** Blocking retrieval of asynchronous evaluations from LF model, HF
    model, or both (mixed case).  For the LF model portion, apply
    correction (if active) to each response in the array.
    derived_synchronize() is designed for the general case where
    derived_evaluate_nowait() may be inconsistent in its use of low
    fidelity evaluations, high fidelity evaluations, or both. */
const IntResponseMap& EnsembleSurrModel::derived_synchronize()
{
  surrResponseMap.clear();

  if (sameModelInstance || sameInterfaceInstance ||
      count_id_maps(modelIdMaps) <= 1) { // 1 queue: blocking synch
    IntResponseMapArray model_resp_maps_rekey(modelIdMaps.size()); // num_steps
    derived_synchronize_sequential(model_resp_maps_rekey, true);
    derived_synchronize_combine(model_resp_maps_rekey, surrResponseMap);
  }
  else                               // competing queues: nonblocking synch
    derived_synchronize_competing();

  return surrResponseMap;
}


/** Nonblocking retrieval of asynchronous evaluations from LF model,
    HF model, or both (mixed case).  For the LF model portion, apply
    correction (if active) to each response in the map.
    derived_synchronize_nowait() is designed for the general case
    where derived_evaluate_nowait() may be inconsistent in its use of
    actual evals, approx evals, or both. */
const IntResponseMap& EnsembleSurrModel::derived_synchronize_nowait()
{
  surrResponseMap.clear();

  IntResponseMapArray model_resp_maps_rekey(modelIdMaps.size());
  derived_synchronize_sequential(model_resp_maps_rekey, false);
  derived_synchronize_combine_nowait(model_resp_maps_rekey, surrResponseMap);

  return surrResponseMap;
}


void EnsembleSurrModel::derived_synchronize_competing()
{
  // in this case, we don't want to starve either LF or HF scheduling by
  // blocking on one or the other --> leverage derived_synchronize_nowait()
  IntResponseMap aggregated_map; // accumulate surrResponseMap returns
  while (test_id_maps(modelIdMaps)) {
    // partial_map is a reference to surrResponseMap, returned by _nowait()
    const IntResponseMap& partial_map = derived_synchronize_nowait();
    if (!partial_map.empty())
      aggregated_map.insert(partial_map.begin(), partial_map.end());
  }

  // Note: cached response maps and any LF/HF aggregations are managed
  // within derived_synchronize_nowait()

  std::swap(surrResponseMap, aggregated_map);
}


const String& EnsembleSurrModel::solution_control_label()
{
  Model&  hf_model = truth_model();
  size_t adv_index = hf_model.solution_control_discrete_variable_index();
  switch (hf_model.solution_control_variable_type()) {
  case DISCRETE_DESIGN_RANGE:       case DISCRETE_DESIGN_SET_INT:
  case DISCRETE_INTERVAL_UNCERTAIN: case DISCRETE_UNCERTAIN_SET_INT:
  case DISCRETE_STATE_RANGE:        case DISCRETE_STATE_SET_INT:
    return currentVariables.all_discrete_int_variable_labels()[adv_index];
    break;
  case DISCRETE_DESIGN_SET_STRING:  case DISCRETE_UNCERTAIN_SET_STRING:
  case DISCRETE_STATE_SET_STRING:
    return currentVariables.all_discrete_string_variable_labels()[adv_index];
    break;
  case DISCRETE_DESIGN_SET_REAL:  case DISCRETE_UNCERTAIN_SET_REAL:
  case DISCRETE_STATE_SET_REAL:
    return currentVariables.all_discrete_real_variable_labels()[adv_index];
    break;
  }
}


void EnsembleSurrModel::add_tabular_solution_level_value(Model& model)
{
  OutputManager& output_mgr = parallelLib.output_manager();
  switch (model.solution_control_variable_type()) {
  case DISCRETE_DESIGN_RANGE:       case DISCRETE_DESIGN_SET_INT:
  case DISCRETE_INTERVAL_UNCERTAIN: case DISCRETE_UNCERTAIN_SET_INT:
  case DISCRETE_STATE_RANGE:        case DISCRETE_STATE_SET_INT:
    output_mgr.add_tabular_scalar(model.solution_level_int_value());    break;
  case DISCRETE_DESIGN_SET_STRING:  case DISCRETE_UNCERTAIN_SET_STRING:
  case DISCRETE_STATE_SET_STRING:
    output_mgr.add_tabular_scalar(model.solution_level_string_value()); break;
  case DISCRETE_DESIGN_SET_REAL:  case DISCRETE_UNCERTAIN_SET_REAL:
  case DISCRETE_STATE_SET_REAL:
    output_mgr.add_tabular_scalar(model.solution_level_real_value());   break;
  }
}

} // namespace Dakota
