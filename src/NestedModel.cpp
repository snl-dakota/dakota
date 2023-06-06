/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NestedModel
//- Description: Implementation code for the NestedModel class
//- Owner:       Mike Eldred
//- Checked by:

#include "NestedModel.hpp"
#include "ProblemDescDB.hpp"
#include "MarginalsCorrDistribution.hpp"
#include "dakota_system_defs.hpp"
#include "pecos_global_defs.hpp"
#include "EvaluationStore.hpp"

static const char rcsId[]="@(#) $Id: NestedModel.cpp 7024 2010-10-16 01:24:42Z mseldre $";

//#define DEBUG


namespace Dakota {

NestedModel::NestedModel(ProblemDescDB& problem_db):
  Model(BaseConstructor(), problem_db),
  nestedModelEvalCntr(0), firstUpdate(true), outerMIPLIndex(0),
  subIteratorSched(parallelLib,
		   true, // peer 1 must assign jobs to peers 2-n
		   problem_db.get_int("model.nested.iterator_servers"),
		   problem_db.get_int("model.nested.processors_per_iterator"),
		   problem_db.get_short("model.nested.iterator_scheduling")),
  subMethodPointer(problem_db.get_string("model.nested.sub_method_pointer")),
  subIteratorJobCntr(0),
  optInterfacePointer(problem_db.get_string("model.interface_pointer"))
{
  ignoreBounds = problem_db.get_bool("responses.ignore_bounds");
  centralHess  = problem_db.get_bool("responses.central_hess");

  // Retrieve the variable mapping inputs
  const StringArray& primary_var_mapping
    = problem_db.get_sa("model.nested.primary_variable_mapping");
  const StringArray& secondary_var_mapping
    = problem_db.get_sa("model.nested.secondary_variable_mapping");

  // NestedModel may set the DB list nodes, but is required to restore them
  // to their previous setting in order to remove the need to continuously
  // reset at the environment level (which would be wasteful since the type
  // of derived model may not be known at the environment level).
  size_t method_index = problem_db.get_db_method_node(); // for restoration
  size_t model_index  = problem_db.get_db_model_node();  // for restoration

  // interface for non-nested data is optional
  if (optInterfacePointer.empty())
    numOptInterfPrimary = numOptInterfIneqCon = numOptInterfEqCon = 0;
  else {
    const String& oi_resp_ptr
      = problem_db.get_string("model.optional_interface_responses_pointer");
    bool oi_resp_ptr_defined = !oi_resp_ptr.empty();
    if (oi_resp_ptr_defined)
      problem_db.set_db_responses_node(oi_resp_ptr);
    // JAS: We need to work a little harder here to make sure that the
    // optional interface responses have a gradient and hessian spec that is
    // compatible with the gradient and hessian spec for the nested model's
    // responses. Currently, if the nested model's responses specify analytic
    // or mixed gradients or hessians but the optional interface responses just 
    // have numerical or no, Dakota dies with a segfault and no error message.
    // An even better solution might be to wrap the optional interface in a
    // SimulationModel, which would allow gradient/hessian requests to be 
    // satisfied however the user wants, and would make it easier for us to
    // honor a request for scaling.

    optInterfGradientType = problem_db.get_string("responses.gradient_type");
    optInterfHessianType = problem_db.get_string("responses.hessian_type");
    optInterfGradIdAnalytic
      = problem_db.get_is("responses.gradients.mixed.id_analytic");
    optInterfHessIdAnalytic
      = problem_db.get_is("responses.hessians.mixed.id_analytic");

    numOptInterfIneqCon
      = problem_db.get_sizet("responses.num_nonlinear_inequality_constraints");
    numOptInterfEqCon
      = problem_db.get_sizet("responses.num_nonlinear_equality_constraints");
    optInterfaceResponse
      = problem_db.get_response(SIMULATION_RESPONSE, currentVariables);
    optionalInterface = problem_db.get_interface();
    size_t num_fns = optInterfaceResponse.num_functions();
    numOptInterfPrimary = num_fns - numOptInterfIneqCon - numOptInterfEqCon;

    if (oi_resp_ptr_defined) {
      // Echo a warning if there is a user specification of constraint bounds/
      // targets that will be superceded by top level constraint bounds/targets.
      const RealVector& interf_ineq_l_bnds
	= problem_db.get_rv("responses.nonlinear_inequality_lower_bounds");
      const RealVector& interf_ineq_u_bnds
	= problem_db.get_rv("responses.nonlinear_inequality_upper_bounds");
      const RealVector& interf_eq_targets
	= problem_db.get_rv("responses.nonlinear_equality_targets");
      bool warning_flag = false;
      size_t i;
      Real dbl_inf = std::numeric_limits<Real>::infinity();
      for (i=0; i<numOptInterfIneqCon; ++i)
	if ( interf_ineq_l_bnds[i] > -dbl_inf || interf_ineq_u_bnds[i] != 0. )
	  warning_flag = true;
      for (i=0; i<numOptInterfEqCon; ++i)
	if ( interf_eq_targets[i] != 0. )
	  warning_flag = true;
      if (warning_flag)
	Cerr << "Warning: nonlinear constraint bounds and targets in nested "
	     << "model optional interfaces\n         are superceded by "
	     << "composite response constraint bounds and targets."
	     << std::endl;
    }

    // db_responses restore not needed since set_db_list_nodes below will reset
  }

  problem_db.set_db_list_nodes(subMethodPointer); // even if empty

  subModel = problem_db.get_model();
  //check_submodel_compatibility(subModel); // sanity checks performed below
  // if outer level output is verbose/debug, request fine-grained evaluation 
  // reporting for purposes of the final output summary.  This allows verbose
  // final summaries without verbose output on every sub-iterator completion.
  if (outputLevel > NORMAL_OUTPUT)
    subModel.fine_grained_evaluation_counters();

  problem_db.set_db_method_node(method_index); // restore method only
  problem_db.set_db_model_nodes(model_index);  // restore all model nodes

  // Perform error checks on variable mapping inputs and convert from
  // strings to indices for efficiency at run time.
  size_t i, num_var_map_1 = primary_var_mapping.size(),
    num_var_map_2 = secondary_var_mapping.size(),
    num_curr_cv   = currentVariables.cv(),
    num_curr_div  = currentVariables.div(),
    num_curr_dsv  = currentVariables.dsv(),
    num_curr_drv  = currentVariables.drv(),
    num_curr_vars = num_curr_cv + num_curr_div + num_curr_dsv + num_curr_drv;
  // Error checks: maps can be empty strings, but must be present to assure
  // correct association.
  if (num_var_map_1 && num_var_map_1 != num_curr_vars) {
    Cerr << "\nError: length of primary variable mapping specification ("
	 << num_var_map_1 << ") does not match number of active variables ("
	 << num_curr_vars << ")." << std::endl;
    abort_handler(MODEL_ERROR);
  }
  if (num_var_map_2 && num_var_map_2 != num_var_map_1) {
    Cerr << "\nError: length of secondary variable mapping specification ("
	 << num_var_map_2 << ") does not match number of primary variables ("
	 << num_var_map_1 << ")." << std::endl;
    abort_handler(MODEL_ERROR);
  }
  // active are sized based on totals due to different mapping options
  active1ACVarMapIndices.resize(num_curr_vars);
  active1ADIVarMapIndices.resize(num_curr_vars);
  active1ADSVarMapIndices.resize(num_curr_vars);
  active1ADRVarMapIndices.resize(num_curr_vars);
  extraCVarsData.resize(num_curr_cv);
  extraDIVarsData.resize(num_curr_div);
  extraDSVarsData.resize(num_curr_dsv);
  extraDRVarsData.resize(num_curr_drv);
  if (num_var_map_2) {
    active2ACVarMapTargets.resize(num_curr_vars);
    active2ADIVarMapTargets.resize(num_curr_vars);
    active2ADSVarMapTargets.resize(num_curr_vars);
    active2ADRVarMapTargets.resize(num_curr_vars);
  }
  short inactive_sm_view = EMPTY_VIEW;
  UShortMultiArrayConstView curr_c_types
    = currentVariables.continuous_variable_types();
  UShortMultiArrayConstView curr_di_types
    = currentVariables.discrete_int_variable_types();
  UShortMultiArrayConstView curr_ds_types
    = currentVariables.discrete_string_variable_types();
  UShortMultiArrayConstView curr_dr_types
    = currentVariables.discrete_real_variable_types();
  UShortMultiArrayConstView submodel_a_c_types
    = subModel.all_continuous_variable_types();
  UShortMultiArrayConstView submodel_a_di_types
    = subModel.all_discrete_int_variable_types();
  UShortMultiArrayConstView submodel_a_ds_types
    = subModel.all_discrete_string_variable_types();
  UShortMultiArrayConstView submodel_a_dr_types
    = subModel.all_discrete_real_variable_types();
  size_t curr_i, sm_acv_cntr = 0, sm_adiv_cntr = 0, sm_adsv_cntr = 0,
    sm_adrv_cntr = 0, sm_acv_avail = 0, sm_adiv_avail = 0, sm_adsv_avail = 0,
    sm_adrv_avail = 0;
  unsigned short prev_c_type = USHRT_MAX, prev_di_type = USHRT_MAX,
    prev_ds_type = USHRT_MAX, prev_dr_type = USHRT_MAX;
  String empty_str;

  const SharedVariablesData& svd = currentVariables.shared_data();

  // Map ACTIVE CONTINUOUS VARIABLES from currentVariables
  for (i=0; i<num_curr_cv; ++i) {
    curr_i = svd.cv_index_to_active_index(i);
    const String& map1
      = (num_var_map_1) ? primary_var_mapping[curr_i] : empty_str;
    if (map1.empty()) {
      // for default mappings between consistent types, propagate bounds/labels
      extraCVarsData.set(i);
      // default mapping: inactive subModel vars = active currentVariables
      update_inactive_view(currentVariables.view().first, inactive_sm_view);
      // Can't use label matching, since subModel labels may not be updated
      // until runtime.  index() returns the _first_ instance of the type.
      unsigned short curr_c_type = curr_c_types[i];
      size_t sm_acv_offset = find_index(submodel_a_c_types, curr_c_type);
      if (sm_acv_offset == _NPOS) {
	Cerr << "\nError: active continuous variable type '" << curr_c_type
	     << "' could not be matched within all sub-model continuous "
	     << "variable types." << std::endl;
	abort_handler(MODEL_ERROR);
      }
      // For multiple types, sm_acv_cntr must be reset to 0 at the type change
      if (curr_c_type != prev_c_type) {
	sm_acv_cntr  = 0;
	sm_acv_avail = std::count(submodel_a_c_types.begin(),
				  submodel_a_c_types.end(), curr_c_type);
      }
      active1ACVarMapIndices[curr_i]  = sm_acv_offset + sm_acv_cntr++;
      active1ADIVarMapIndices[curr_i] = active1ADSVarMapIndices[curr_i]
	= active1ADRVarMapIndices[curr_i] = _NPOS;
      prev_c_type = curr_c_type;
      if (sm_acv_cntr > sm_acv_avail) {
	Cerr << "\nError: default insertions of type '" << curr_c_type
	     << "' exceed sub-model allocation." << std::endl;
	abort_handler(MODEL_ERROR);
      }
      if (num_var_map_2)
	active2ACVarMapTargets[curr_i] = active2ADIVarMapTargets[curr_i]
	  = active2ADSVarMapTargets[curr_i] = active2ADRVarMapTargets[curr_i]
	  = Pecos::NO_TARGET;
    }
    else {
      extraCVarsData.reset(i); // not a default mapping based on types
      const String& map2
	= (num_var_map_2) ? secondary_var_mapping[curr_i] : empty_str;
      resolve_real_variable_mapping(map1, map2, curr_i, inactive_sm_view);
    }
  }

  // Map ACTIVE DISCRETE INTEGER VARIABLES from currentVariables
  for (i=0; i<num_curr_div; ++i) {
    curr_i = svd.div_index_to_active_index(i);
    const String& map1
      = (num_var_map_1) ? primary_var_mapping[curr_i] : empty_str;
    if (map1.empty()) {
      // for default mappings between consistent types, propagate bounds/labels
      extraDIVarsData.set(i);
      // default mapping: inactive subModel vars = active currentVariables
      update_inactive_view(currentVariables.view().first, inactive_sm_view);
      // Can't use label matching, since subModel labels may not be updated
      // until runtime.  index() returns the _first_ instance of the type.
      unsigned short curr_di_type = curr_di_types[i];
      size_t sm_adiv_offset = find_index(submodel_a_di_types, curr_di_type);
      if (sm_adiv_offset == _NPOS) {
	Cerr << "\nError: active discrete integer variable type '"
	     << curr_di_type << "' could not be matched within all sub-model "
	     << "discrete integer variable types." << std::endl;
	abort_handler(MODEL_ERROR);
      }
      // For multiple types, sm_adiv_cntr must be reset to 0 at the type change
      if (curr_di_type != prev_di_type) {
	sm_adiv_cntr  = 0;
	sm_adiv_avail = std::count(submodel_a_di_types.begin(),
				   submodel_a_di_types.end(), curr_di_type);
      }
      active1ADIVarMapIndices[curr_i] = sm_adiv_offset + sm_adiv_cntr++;
      active1ACVarMapIndices[curr_i]  = active1ADSVarMapIndices[curr_i]
	= active1ADRVarMapIndices[curr_i] = _NPOS;
      prev_di_type = curr_di_type;
      if (sm_adiv_cntr > sm_adiv_avail) {
	Cerr << "\nError: default insertions of type '" << curr_di_type
	     << "' exceed sub-model allocation." << std::endl;
	abort_handler(MODEL_ERROR);
      }
      if (num_var_map_2)
	active2ACVarMapTargets[curr_i] = active2ADIVarMapTargets[curr_i]
	  = active2ADSVarMapTargets[curr_i] = active2ADRVarMapTargets[curr_i]
	  = Pecos::NO_TARGET;
    }
    else {
      extraDIVarsData.reset(i); // not a default mapping based on types
      const String& map2
	= (num_var_map_2) ? secondary_var_mapping[curr_i] : empty_str;
      resolve_integer_variable_mapping(map1, map2, curr_i, inactive_sm_view);
    }
  }

  // Map ACTIVE DISCRETE STRING VARIABLES from currentVariables
  for (i=0; i<num_curr_dsv; ++i) {
    curr_i = svd.dsv_index_to_active_index(i);
    const String& map1
      = (num_var_map_1) ? primary_var_mapping[curr_i] : empty_str;
    if (map1.empty()) {
      // for default mappings between consistent types, propagate bounds/labels
      extraDSVarsData.set(i);
      // default mapping: inactive subModel vars = active currentVariables
      update_inactive_view(currentVariables.view().first, inactive_sm_view);
      // Can't use label matching, since subModel labels may not be updated
      // until runtime.  index() returns the _first_ instance of the type.
      unsigned short curr_ds_type = curr_ds_types[i];
      size_t sm_adsv_offset = find_index(submodel_a_ds_types, curr_ds_type);
      if (sm_adsv_offset == _NPOS) {
	Cerr << "\nError: active discrete string variable type '"
	     << curr_ds_type << "' could not be matched within all sub-model "
	     << "discrete string variable types." << std::endl;
	abort_handler(MODEL_ERROR);
      }
      // For multiple types, sm_adsv_cntr must be reset to 0 at the type change
      if (curr_ds_type != prev_ds_type) {
	sm_adsv_cntr  = 0;
	sm_adsv_avail = std::count(submodel_a_ds_types.begin(),
				   submodel_a_ds_types.end(), curr_ds_type);
      }
      active1ADSVarMapIndices[curr_i] = sm_adsv_offset + sm_adsv_cntr++;
      active1ACVarMapIndices[curr_i]  = active1ADIVarMapIndices[curr_i]
	= active1ADRVarMapIndices[curr_i] = _NPOS;
      prev_ds_type = curr_ds_type;
      if (sm_adsv_cntr > sm_adsv_avail) {
	Cerr << "\nError: default insertions of type '" << curr_ds_type
	     << "' exceed sub-model allocation." << std::endl;
	abort_handler(MODEL_ERROR);
      }
      if (num_var_map_2)
	active2ACVarMapTargets[curr_i] = active2ADIVarMapTargets[curr_i]
	  = active2ADSVarMapTargets[curr_i] = active2ADRVarMapTargets[curr_i]
	  = Pecos::NO_TARGET;
    }
    else {
      extraDSVarsData.reset(i); // not a default mapping based on types
      const String& map2
	= (num_var_map_2) ? secondary_var_mapping[curr_i] : empty_str;
      resolve_string_variable_mapping(map1, map2, curr_i, inactive_sm_view);
    }
  }

  // Map ACTIVE DISCRETE REAL VARIABLES from currentVariables
  for (i=0; i<num_curr_drv; ++i) {
    curr_i = svd.drv_index_to_active_index(i);
    const String& map1
      = (num_var_map_1) ? primary_var_mapping[curr_i] : empty_str;
    if (map1.empty()) {
      // for default mappings between consistent types, propagate bounds/labels
      extraDRVarsData.set(i);
      // default mapping: inactive subModel vars = active currentVariables
      update_inactive_view(currentVariables.view().first, inactive_sm_view);
      // Can't use label matching, since subModel labels may not be updated
      // until runtime.  index() returns the _first_ instance of the type.
      unsigned short curr_dr_type = curr_dr_types[i];
      size_t sm_adrv_offset = find_index(submodel_a_dr_types, curr_dr_type);
      if (sm_adrv_offset == _NPOS) {
	Cerr << "\nError: active discrete real variable type '" << curr_dr_type
	     << "' could not be matched within all sub-model discrete real "
	     << "variable types." << std::endl;
	abort_handler(MODEL_ERROR);
      }
      // For multiple types, sm_adrv_cntr must be reset to 0 at the type change
      if (curr_dr_type != prev_dr_type) {
	sm_adrv_cntr  = 0;
	sm_adrv_avail = std::count(submodel_a_dr_types.begin(),
				   submodel_a_dr_types.end(), curr_dr_type);
      }
      active1ADRVarMapIndices[curr_i] = sm_adrv_offset + sm_adrv_cntr++;
      active1ACVarMapIndices[curr_i]  = active1ADIVarMapIndices[curr_i]
	= active1ADSVarMapIndices[curr_i] = _NPOS;
      prev_dr_type = curr_dr_type;
      if (sm_adrv_cntr > sm_adrv_avail) {
	Cerr << "\nError: default insertions of type '" << curr_dr_type
	     << "' exceed sub-model allocation." << std::endl;
	abort_handler(MODEL_ERROR);
      }
      if (num_var_map_2)
	active2ACVarMapTargets[curr_i] = active2ADIVarMapTargets[curr_i]
	  = active2ADSVarMapTargets[curr_i] = active2ADRVarMapTargets[curr_i]
	  = Pecos::NO_TARGET;
    }
    else {
      extraDRVarsData.reset(i); // not a default mapping based on types
      const String& map2
	= (num_var_map_2) ? secondary_var_mapping[curr_i] : empty_str;
      resolve_real_variable_mapping(map1, map2, curr_i, inactive_sm_view);
    }
  }

  size_t num_curr_ccv = currentVariables.acv() - num_curr_cv,//active complement
    num_curr_cdiv = currentVariables.adiv() - num_curr_div, // active complement
    num_curr_cdsv = currentVariables.adsv() - num_curr_dsv, // active complement
    num_curr_cdrv = currentVariables.adrv() - num_curr_drv; // active complement
  // complement can be sized based on corresponding currentVariables sizes
  // due to restriction to default mappings
  if (num_curr_ccv)
    complement1ACVarMapIndices.resize(num_curr_ccv);
  if (num_curr_cdiv)
    complement1ADIVarMapIndices.resize(num_curr_cdiv);
  if (num_curr_cdsv)
    complement1ADSVarMapIndices.resize(num_curr_cdsv);
  if (num_curr_cdrv)
    complement1ADRVarMapIndices.resize(num_curr_cdrv);
  UShortMultiArrayConstView curr_ac_types
    = currentVariables.all_continuous_variable_types();
  UShortMultiArrayConstView curr_adi_types
    = currentVariables.all_discrete_int_variable_types();
  UShortMultiArrayConstView curr_ads_types
    = currentVariables.all_discrete_string_variable_types();
  UShortMultiArrayConstView curr_adr_types
    = currentVariables.all_discrete_real_variable_types();
  unsigned short prev_ac_type = USHRT_MAX, prev_adi_type = USHRT_MAX,
    prev_ads_type = USHRT_MAX, prev_adr_type = USHRT_MAX;

  // Map COMPLEMENT CONTINUOUS VARIABLES from currentVariables
  for (i=0; i<num_curr_ccv; ++i) {
    curr_i = svd.ccv_index_to_acv_index(i);
    // Can't use label matching, since subModel labels may not be updated
    // until runtime.  index() returns the _first_ instance of the type.
    unsigned short curr_ac_type = curr_ac_types[curr_i];
    size_t sm_acv_offset = find_index(submodel_a_c_types, curr_ac_type);
    if (sm_acv_offset == _NPOS) {
      Cerr << "\nError: complement continuous variable type '" << curr_ac_type
	   << "' could not be matched within all sub-model continuous "
	   << "variable types." << std::endl;
      abort_handler(MODEL_ERROR);
    }
    // For multiple types, sm_acv_cntr must be reset to 0 at the type change
    if (curr_ac_type != prev_ac_type) {
      sm_acv_cntr  = 0;
      sm_acv_avail = std::count(submodel_a_c_types.begin(),
				submodel_a_c_types.end(), curr_ac_type);
    }
    complement1ACVarMapIndices[i] = sm_acv_offset + sm_acv_cntr++;
    prev_ac_type = curr_ac_type;
    if (sm_acv_cntr > sm_acv_avail) {
      Cerr << "\nError: default insertions of type '" << curr_ac_type
	   << "' exceed sub-model allocation." << std::endl;
      abort_handler(MODEL_ERROR);
    }
  }

  // Map COMPLEMENT DISCRETE INTEGER VARIABLES from currentVariables
  for (i=0; i<num_curr_cdiv; ++i) {
    curr_i = svd.cdiv_index_to_adiv_index(i);
    // Can't use label matching, since subModel labels may not be updated
    // until runtime.  index() returns the _first_ instance of the type.
    unsigned short curr_adi_type = curr_adi_types[curr_i];
    size_t sm_adiv_offset = find_index(submodel_a_di_types, curr_adi_type);
    if (sm_adiv_offset == _NPOS) {
      Cerr << "\nError: complement discrete integer variable type '"
	   << curr_adi_type << "' could not be matched within all sub-model "
	   << "discrete integer variable types." << std::endl;
      abort_handler(MODEL_ERROR);
    }
    // For multiple types, sm_adiv_cntr must be reset to 0 at the type change
    if (curr_adi_type != prev_adi_type) {
      sm_adiv_cntr  = 0;
      sm_adiv_avail = std::count(submodel_a_di_types.begin(),
				 submodel_a_di_types.end(), curr_adi_type);
    }
    complement1ADIVarMapIndices[i] = sm_adiv_offset + sm_adiv_cntr++;
    prev_adi_type = curr_adi_type;
    if (sm_adiv_cntr > sm_adiv_avail) {
      Cerr << "\nError: default insertions of type '" << curr_adi_type
	   << "' exceed sub-model allocation." << std::endl;
      abort_handler(MODEL_ERROR);
    }
  }

  // Map COMPLEMENT DISCRETE STRING VARIABLES from currentVariables
  for (i=0; i<num_curr_cdsv; ++i) {
    curr_i = svd.cdsv_index_to_adsv_index(i);
    // Can't use label matching, since subModel labels may not be updated
    // until runtime.  index() returns the _first_ instance of the type.
    unsigned short curr_ads_type = curr_ads_types[curr_i];
    size_t sm_adsv_offset = find_index(submodel_a_ds_types, curr_ads_type);
    if (sm_adsv_offset == _NPOS) {
      Cerr << "\nError: complement discrete string variable type '"
	   << curr_ads_type << "' could not be matched within all sub-model "
	   << "discrete string variable types." << std::endl;
      abort_handler(MODEL_ERROR);
    }
    // For multiple types, sm_adsv_cntr must be reset to 0 at the type change
    if (curr_ads_type != prev_ads_type) {
      sm_adsv_cntr  = 0;
      sm_adsv_avail = std::count(submodel_a_ds_types.begin(),
				 submodel_a_ds_types.end(), curr_ads_type);
    }
    complement1ADSVarMapIndices[i] = sm_adsv_offset + sm_adsv_cntr++;
    prev_ads_type = curr_ads_type;
    if (sm_adsv_cntr > sm_adsv_avail) {
      Cerr << "\nError: default insertions of type '" << curr_ads_type
	   << "' exceed sub-model allocation." << std::endl;
      abort_handler(MODEL_ERROR);
    }
  }

  // Map COMPLEMENT DISCRETE REAL VARIABLES from currentVariables
  for (i=0; i<num_curr_cdrv; ++i) {
    curr_i = svd.cdrv_index_to_adrv_index(i);
    // Can't use label matching, since subModel labels may not be updated
    // until runtime.  index() returns the _first_ instance of the type.
    unsigned short curr_adr_type = curr_adr_types[curr_i];
    size_t sm_adrv_offset = find_index(submodel_a_dr_types, curr_adr_type);
    if (sm_adrv_offset == _NPOS) {
      Cerr << "\nError: complement discrete real variable type '"
	   << curr_adr_type << "' could not be matched within all sub-model "
	   << "discrete real variable types." << std::endl;
      abort_handler(MODEL_ERROR);
    }
    // For multiple types, sm_adrv_cntr must be reset to 0 at the type change
    if (curr_adr_type != prev_adr_type) {
      sm_adrv_cntr  = 0;
      sm_adrv_avail = std::count(submodel_a_dr_types.begin(),
				 submodel_a_dr_types.end(), curr_adr_type);
    }
    complement1ADRVarMapIndices[i] = sm_adrv_offset + sm_adrv_cntr++;
    prev_adr_type = curr_adr_type;
    if (sm_adrv_cntr > sm_adrv_avail) {
      Cerr << "\nError: default insertions of type '" << curr_adr_type
	   << "' exceed sub-model allocation." << std::endl;
      abort_handler(MODEL_ERROR);
    }
  }

#ifdef DEBUG
  Cout << "\nactive primary variable mapping indices\nACV:\n"
       << active1ACVarMapIndices << "ADIV:\n" << active1ADIVarMapIndices
       << "ADSV:\n" << active1ADSVarMapIndices
       << "ADRV:\n" << active1ADRVarMapIndices
       << "\nactive secondary variable mapping targets:\nACV:\n"
       << active2ACVarMapTargets << "ADIV:\n" << active2ADIVarMapTargets 
       << "ADSV:\n" << active2ADSVarMapTargets 
       << "ADRV:\n" << active2ADRVarMapTargets
       << "\ncomplement primary variable mapping indices\nACV:\n"
       << complement1ACVarMapIndices << "ADIV:\n" << complement1ADIVarMapIndices
       << "ADSV:\n" << complement1ADSVarMapIndices
       << "ADRV:\n" << complement1ADRVarMapIndices << '\n';
#endif // DEBUG

  // subModel view updating must be performed before subIterator instantiation
  // since any model recastings will pick up the inactive view (and inactive
  // view differences cause problems with recursion updating).
  if (inactive_sm_view != EMPTY_VIEW)
    subModel.inactive_view(inactive_sm_view); // recurse

  currentResponse.reshape_metadata(0);
}


void NestedModel::declare_sources()
{
  evaluationsDB.declare_source(modelId, modelType, subIterator.method_id(),
    "iterator");
  if(!optionalInterface.is_null())
    evaluationsDB.declare_source(modelId, modelType,
      optionalInterface.interface_id(), "interface");
}


/** Asynchronous flags need to be initialized for the subModel.  In
    addition, max_eval_concurrency is the outer level iterator
    concurrency, not the subIterator concurrency that subModel will
    see, and recomputing the message_lengths on the subModel is
    probably not a bad idea either.  Therefore, recompute everything
    on subModel using init_communicators(). */
void NestedModel::
derived_init_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
			   bool recurse_flag)
{
  // initialize optionalInterface for parallel operations
  if (!optInterfacePointer.empty()) {
    // allow recursion to progress - don't store/set/restore
    parallelLib.parallel_configuration_iterator(modelPCIter);
    optionalInterface.init_communicators(messageLengths, max_eval_concurrency);
  }

  if (!recurse_flag)
    return;

  // Due to Model::init_communicators(), we know that pl_iter is the lowest
  // level within modelPCIter->miPLIters, from which we may further subdivide.

  // initializations for subIteratorSched:
  // > incoming max_eval_concurrency is for concurrent execs of NestedModel,
  //   which must be distinguished from eval concurrency within subIterator.
  // > circular dependency between concurrent iterator partitioning and
  //   subIterator instantiation is managed as described in
  //   ConcurrentMetaIterator::init_communicators().
  // > as for constructors, we recursively set and restore DB list nodes
  //   (initiated from the restored starting point following construction).
  size_t method_index = probDescDB.get_db_method_node(),
         model_index  = probDescDB.get_db_model_node();  // for restoration
  probDescDB.set_db_list_nodes(subMethodPointer);

  // > init_eval_concurrency instantiates subIterator on previous pl ranks
  subIteratorSched.update(modelPCIter);
  // > define min and max processors per iterator
  IntIntPair ppi_pr
    = subIteratorSched.configure(probDescDB, subIterator, subModel);
  // > passed in max_eval_concurrency is the outer nested model concurrency
  subIteratorSched.partition(max_eval_concurrency, ppi_pr);
  // > now augment prev subIterator instantiations for additional mi_pl ranks
  //   (new mi_pl is used via miPLIndex update in partition())
  // > idle server is managed here; a dedicated master processor is managed
  //   within IteratorScheduler::init_iterator().
  if (subIteratorSched.iteratorServerId <= subIteratorSched.numIteratorServers)
    subIteratorSched.init_iterator(probDescDB, subIterator, subModel);

  // > restore all DB nodes
  probDescDB.set_db_method_node(method_index);
  probDescDB.set_db_model_nodes(model_index);

  // > now that subIterator is constructed, perform downstream updates
  if (!subIterator.is_null()) {
    init_sub_iterator(); // follow DB restore: extracts data from nested spec
    if (subIteratorSched.messagePass) {
      // msg lengths: vars from this model, set & final results from subIterator
      MPIPackBuffer buff; int eval_id = 0;
      const Response& si_resp = subIterator.response_results();
      buff << currentVariables << si_resp.active_set() << eval_id;
      int params_buff_len = buff.size(); buff.reset();
      buff << si_resp;
      subIteratorSched.iterator_message_lengths(params_buff_len, buff.size());
    }
  }
}


void NestedModel::
derived_set_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
			  bool recurse_flag)
{
  // Outer context:
  outerMIPLIndex = modelPCIter->mi_parallel_level_index(pl_iter);

  if (!optInterfacePointer.empty()) {
    // allow recursion to progress - don't store/set/restore
    parallelLib.parallel_configuration_iterator(modelPCIter);
    optionalInterface.set_communicators(messageLengths, max_eval_concurrency);
    // initial setting for asynchEvalFlag & evaluationCapacity based on
    // optInterface (may be updated below)
    set_ie_asynchronous_mode(max_eval_concurrency);
  }
  if (recurse_flag) {
    // Inner context: set comms for subIterator
    // > pl_iter is incoming context prior to subIterator partitioning
    // > mi_pl_index reflects the miPL depth after subIterator partitioning
    size_t mi_pl_index = outerMIPLIndex + 1;
    subIteratorSched.update(modelPCIter, mi_pl_index);
    if (subIteratorSched.iteratorServerId <=
	subIteratorSched.numIteratorServers) {
      ParLevLIter si_pl_iter
	= modelPCIter->mi_parallel_level_iterator(mi_pl_index);
      subIteratorSched.set_iterator(subIterator, si_pl_iter);
    }

    // update asynchEvalFlag & evaluationCapacity based on subIteratorSched
    if (subIteratorSched.messagePass)
      asynchEvalFlag = true;
    if (subIteratorSched.numIteratorServers > evaluationCapacity)
      evaluationCapacity = subIteratorSched.numIteratorServers;
  }
}


void NestedModel::
derived_free_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
			   bool recurse_flag)
{
  /*
  if (!optInterfacePointer.empty()) {
    // allow recursion to progress - don't store/set/restore
    parallelLib.parallel_configuration_iterator(modelPCIter);
    optionalInterface.free_communicators();
  }
  */
  if (recurse_flag) {
    // finalize comms for subIterator
    // > pl_iter is incoming context prior to subIterator partitioning
    // > mi_pl_index reflects the miPL depth after subIterator partitioning
    size_t mi_pl_index = modelPCIter->mi_parallel_level_index(pl_iter) + 1;
    subIteratorSched.update(modelPCIter, mi_pl_index);
    if (subIteratorSched.iteratorServerId <=
	subIteratorSched.numIteratorServers) {
      ParLevLIter si_pl_iter
	= modelPCIter->mi_parallel_level_iterator(mi_pl_index);
      subIteratorSched.free_iterator(subIterator, si_pl_iter);
    }
    subIteratorSched.free_iterator_parallelism();
  }
}


void NestedModel::init_sub_iterator()
{
  // activates additional data processing options and manages output
  subIterator.sub_iterator_flag(true);
  // passes data through to subModel via recursions layered by subIterator;
  // VarMaps are used by ProbabilityTransformModel, which generally appears
  // within these Model recursions (so can't call subModel here without
  // requiring inclusion in the update_from_sub_model() chain)
  subIterator.nested_variable_mappings(active1ACVarMapIndices,
    active1ADIVarMapIndices, active1ADSVarMapIndices, active1ADRVarMapIndices,
    active2ACVarMapTargets,  active2ADIVarMapTargets, active2ADSVarMapTargets,
    active2ADRVarMapTargets);

  // Back out the number of eq/ineq constraints within secondaryRespCoeffs
  // (subIterator constraints) from the total number of equality/inequality
  // constraints and the number of interface equality/inequality constraints.
  size_t num_mapped_ineq_con
    = probDescDB.get_sizet("responses.num_nonlinear_inequality_constraints"),
    num_mapped_eq_con
    = probDescDB.get_sizet("responses.num_nonlinear_equality_constraints");
  numSubIterMappedIneqCon = num_mapped_ineq_con - numOptInterfIneqCon;
  numSubIterMappedEqCon   = num_mapped_eq_con   - numOptInterfEqCon;

  // For auto-generated identity map, we don't support an optional
  // interface and require that the total number of NestedModel
  // responses (pri+sec) equal the number of sub-iterator results
  // functions.
  //
  // Rationale: Too complex to get right and communicate to
  // user. Secondary responses from an optional interface augment any
  // mapped secondary responses. Nested model primary responses come
  // from an overlay of optional interface and mapped responses, with
  // no restrictions on which is larger or smaller.
  size_t num_mapped_total = currentResponse.num_functions(),
    num_mapped_sec = num_mapped_ineq_con + num_mapped_eq_con,
    num_mapped_pri = num_mapped_total - num_mapped_sec;
  numSubIterFns = subIterator.response_results().num_functions();

  identityRespMap = probDescDB.get_bool("model.nested.identity_resp_map");
  const RealVector& primary_resp_coeffs
    = probDescDB.get_rv("model.nested.primary_response_mapping");
  const RealVector& secondary_resp_coeffs
    = probDescDB.get_rv("model.nested.secondary_response_mapping");

  if (identityRespMap) {
    bool found_error = false;
    if (!optInterfacePointer.empty()) {
      Cerr << "\nError: identity_response_mapping not supported in conjunction"
       << " with optional_interface_pointer; use explicit primary/secondary_"
       << "response_mapping instead.\n";
      found_error = true;
    }
    if (!primary_resp_coeffs.empty() || !secondary_resp_coeffs.empty()) {
      Cerr << "\nError: Neither primary_response_mapping nor secondary_"
        << "response_mapping may be specified in conjunction with identity_"
        << "response_mapping.\n";
      found_error = true;
    }
    if (num_mapped_total != numSubIterFns) {
      Cerr << "\nError: For identity_response_mapping, number of nested model "
        << "responses (primary + secondary functions) must equal the number of "
        << "sub-method final results. Specified nested model has " 
        << num_mapped_total << " functions, while there are " << numSubIterFns 
        << " sub-method results.\n";
      if (outputLevel >= VERBOSE_OUTPUT)
	Cerr << "Info: Sub-method returns these results:\n"
	     << subIterator.response_results().function_labels() << "\n";
      else
	Cerr << "Info: Re-run with 'output verbose' to list the sub-method "
	     << "results.\n";
      found_error = true;
    }
    if (found_error)
      abort_handler(-1);

    if (outputLevel >= VERBOSE_OUTPUT)
      Cout << "Info: NestedModel using identity response mapping." << std::endl;
    subIterMappedPri = num_mapped_pri;
    subIterMappedSec = num_mapped_sec;

    // BMA TODO: When using identity map, propagate labels when not
    // present (there's no way to detect that the user didn't give
    // them as defaults get generated at parse time)
  }
  else if (primary_resp_coeffs.empty() && secondary_resp_coeffs.empty()) {
    Cerr << "\nError: no mappings provided for sub-iterator functions in "
	 << "NestedModel initialization." << std::endl;
    abort_handler(MODEL_ERROR);
  }
  // Convert vectors to matrices using the number of subIterator response
  // results (e.g., the number of UQ statistics) as the number of columns
  // (rows are open ended since the number of subModel primary fns may
  // be different from the total number of primary fns and subModel's
  // constraints are a subset of the total constraints).
  if (!primary_resp_coeffs.empty()) {
    // this error would also be caught within copy_data(), but by checking here,
    // we can provide a more helpful error message.
    if (primary_resp_coeffs.length() % numSubIterFns) {
      Cerr << "\nError: number of entries in primary_response_mapping ("
	   << primary_resp_coeffs.length() << ") not evenly divisible"
	   << "\n       by number of sub-iterator final results functions ("
	   << numSubIterFns << ") in NestedModel initialization." << std::endl;
      Cerr << "\nInfo: The primary_response_mapping must have between 1 and "
	   << num_mapped_pri
	   << " (number of nested model primary response functions) row(s).\n"
	   << "It must have " << numSubIterFns
           << " columns corresponding to the sub-method final results.\n";
      if (outputLevel >= VERBOSE_OUTPUT)
	Cerr << "Info: Sub-method returns these results:\n"
	     << subIterator.response_results().function_labels() << "\n";
      else
	Cerr << "Info: Re-run with 'output verbose' to list the sub-method "
	     << "results.\n";
      abort_handler(MODEL_ERROR);
    }
    copy_data(primary_resp_coeffs, primaryRespCoeffs, 0, (int)numSubIterFns);
    subIterMappedPri = primaryRespCoeffs.numRows();
  }
  if (!secondary_resp_coeffs.empty()) {
    // this error would also be caught within copy_data(), but by checking here,
    // we can provide a more helpful error message.
    if (secondary_resp_coeffs.length() % numSubIterFns) {
      Cerr << "\nError: number of entries in secondary_response_mapping ("
	   << secondary_resp_coeffs.length() << ") not evenly divisible"
	   << "\n       by number of sub-iterator final results functions ("
	   << numSubIterFns << ") in NestedModel initialization." << std::endl;
      Cerr << "\nInfo: The secondary_response_mapping must have "
	   << numSubIterMappedIneqCon + numSubIterMappedEqCon 
	   << " (number of nested model secondary response functions, less any "
           << "optional interface secondary response functions) row(s).\n"
	   << "It must have " << numSubIterFns
           << " columns corresponding to the sub-method final results.\n";
      if (outputLevel >= VERBOSE_OUTPUT)
	Cerr << "Info: Sub-method returns these results:\n"
	     << subIterator.response_results().function_labels() << "\n";
      else
	Cerr << "Info: Re-run with 'output verbose' to list the sub-method "
	     << "results.\n";
      abort_handler(MODEL_ERROR);
    }
    copy_data(secondary_resp_coeffs, secondaryRespCoeffs, 0,(int)numSubIterFns);
    subIterMappedSec = secondaryRespCoeffs.numRows();
  }
  subIterator.nested_response_mappings(primaryRespCoeffs, secondaryRespCoeffs);
}


void NestedModel::
resolve_map1(const String& map1, size_t& ac_index1, size_t& adi_index1,
	     size_t& ads_index1, size_t& adr_index1, size_t curr_index,
	     short& inactive_sm_view)
{
  adi_index1 = ads_index1 = adr_index1 = _NPOS;
  ac_index1 = find_index(subModel.all_continuous_variable_labels(), map1);
  if (ac_index1 == _NPOS) {
    adi_index1 = find_index(subModel.all_discrete_int_variable_labels(), map1);
    if (adi_index1 == _NPOS) {
      ads_index1
	= find_index(subModel.all_discrete_string_variable_labels(), map1);
      if (ads_index1 == _NPOS) {
	adr_index1
	  = find_index(subModel.all_discrete_real_variable_labels(), map1);
        if (adr_index1 == _NPOS) {
	  Cerr << "\nError: primary mapping " << map1 << " could not be "
	       << "matched within any sub-model variable labels." << std::endl;
	  abort_handler(MODEL_ERROR);
	}
	else if (find_index(subModel.discrete_real_variable_labels(), map1)
		 == _NPOS) // inactive DRV target
	  update_inactive_view(
	    subModel.all_discrete_real_variable_types()[adr_index1],
	    inactive_sm_view);
      }
      else if (find_index(subModel.discrete_string_variable_labels(), map1)
	       == _NPOS) // inactive DSV target
	update_inactive_view(
	  subModel.all_discrete_string_variable_types()[ads_index1],
	  inactive_sm_view);
    }
    else if (find_index(subModel.discrete_int_variable_labels(), map1)
	     == _NPOS) // inactive DIV target
      update_inactive_view(
	subModel.all_discrete_int_variable_types()[adi_index1],
	inactive_sm_view);
  }
  else if (find_index(subModel.continuous_variable_labels(), map1)
	   == _NPOS) // inactive CV target
    update_inactive_view(subModel.all_continuous_variable_types()[ac_index1],
			 inactive_sm_view);

  active1ACVarMapIndices[curr_index]  =  ac_index1;
  active1ADIVarMapIndices[curr_index] = adi_index1;
  active1ADSVarMapIndices[curr_index] = ads_index1;
  active1ADRVarMapIndices[curr_index] = adr_index1;
}


void NestedModel::
resolve_real_variable_mapping(const String& map1, const String& map2,
			      size_t curr_index, short& inactive_sm_view)
{
  size_t ac_index1, adi_index1, ads_index1, adr_index1;
  resolve_map1(map1, ac_index1, adi_index1, ads_index1, adr_index1,
	       curr_index, inactive_sm_view);

  if (!active2ACVarMapTargets.empty()) { // indicates num_var_map_2
    if (map2.empty())
      active2ACVarMapTargets[curr_index] = active2ADIVarMapTargets[curr_index]
	= active2ADSVarMapTargets[curr_index]
	= active2ADRVarMapTargets[curr_index] = Pecos::NO_TARGET;
    else if (ac_index1 != _NPOS) {
      unsigned short type = subModel.all_continuous_variable_types()[ac_index1];
      if (type == CONTINUOUS_DESIGN || type == CONTINUOUS_STATE) {
	if (map2 == "lower_bound")
	  active2ACVarMapTargets[curr_index] = Pecos::CR_LWR_BND;
	else if (map2 == "upper_bound")
	  active2ACVarMapTargets[curr_index] = Pecos::CR_UPR_BND;
	else {
	  Cerr << "\nError: " << map2 << " mapping not supported for "
	       << "continuous range variables." << std::endl;
	  abort_handler(MODEL_ERROR);
	}
      }
      else if (type == NORMAL_UNCERTAIN) {
	if (map2 == "mean")
	  active2ACVarMapTargets[curr_index] = Pecos::N_MEAN;
	else if (map2 == "std_deviation")
	  active2ACVarMapTargets[curr_index] = Pecos::N_STD_DEV;
	else if (map2 == "lower_bound")
	  active2ACVarMapTargets[curr_index] = Pecos::N_LWR_BND;
	else if (map2 == "upper_bound")
	  active2ACVarMapTargets[curr_index] = Pecos::N_UPR_BND;
	else if (map2 == "location")
	  active2ACVarMapTargets[curr_index] = Pecos::N_LOCATION;
	else if (map2 == "scale")
	  active2ACVarMapTargets[curr_index] = Pecos::N_SCALE;
	else {
	  Cerr << "\nError: " << map2 << " mapping not supported for "
	       << "normal distributions." << std::endl;
	  abort_handler(MODEL_ERROR);
	}
      }
      else if (type == LOGNORMAL_UNCERTAIN) {
	// TO DO: 3 parameter lognormal adds a location parameter
	if (map2 == "mean")
	  active2ACVarMapTargets[curr_index] = Pecos::LN_MEAN;
	else if (map2 == "std_deviation")
	  active2ACVarMapTargets[curr_index] = Pecos::LN_STD_DEV;
	else if (map2 == "lambda")
	    active2ACVarMapTargets[curr_index] = Pecos::LN_LAMBDA;
	else if (map2 == "zeta")
	  active2ACVarMapTargets[curr_index] = Pecos::LN_ZETA;
	else if (map2 == "error_factor")
	  active2ACVarMapTargets[curr_index] = Pecos::LN_ERR_FACT;
	else if (map2 == "lower_bound")
	  active2ACVarMapTargets[curr_index] = Pecos::LN_LWR_BND;
	else if (map2 == "upper_bound")
	  active2ACVarMapTargets[curr_index] = Pecos::LN_UPR_BND;
	else {
	  Cerr << "\nError: " << map2 << " mapping not supported for "
	       << "lognormal distributions." << std::endl;
	  abort_handler(MODEL_ERROR);
	}
      }
      else if (type == UNIFORM_UNCERTAIN) {
	if (map2 == "lower_bound")
	  active2ACVarMapTargets[curr_index] = Pecos::U_LWR_BND;
	else if (map2 == "upper_bound")
	  active2ACVarMapTargets[curr_index] = Pecos::U_UPR_BND;
	else if (map2 == "location")
	  active2ACVarMapTargets[curr_index] = Pecos::U_LOCATION;
	else if (map2 == "scale")
	  active2ACVarMapTargets[curr_index] = Pecos::U_SCALE;
	else {
	  Cerr << "\nError: " << map2 << " mapping not supported for "
	       << "uniform distributions." << std::endl;
	  abort_handler(MODEL_ERROR);
	}
      }
      else if (type == LOGUNIFORM_UNCERTAIN) {
	// TO DO: mean/std deviation or location/scale
	if (map2 == "lower_bound")
	  active2ACVarMapTargets[curr_index] = Pecos::LU_LWR_BND;
	else if (map2 == "upper_bound")
	  active2ACVarMapTargets[curr_index] = Pecos::LU_UPR_BND;
	else {
	  Cerr << "\nError: " << map2 << " mapping not supported for "
	       << "loguniform distributions." << std::endl;
	  abort_handler(MODEL_ERROR);
	}
      }
      else if (type == TRIANGULAR_UNCERTAIN) {
	if (map2 == "mode")
	  active2ACVarMapTargets[curr_index] = Pecos::T_MODE;
	else if (map2 == "lower_bound")
	  active2ACVarMapTargets[curr_index] = Pecos::T_LWR_BND;
	else if (map2 == "upper_bound")
	  active2ACVarMapTargets[curr_index] = Pecos::T_UPR_BND;
	else if (map2 == "location")
	  active2ACVarMapTargets[curr_index] = Pecos::T_LOCATION;
	else if (map2 == "scale")
	  active2ACVarMapTargets[curr_index] = Pecos::T_SCALE;
	else {
	  Cerr << "\nError: " << map2 << " mapping not supported for "
	       << "triangular distributions." << std::endl;
	  abort_handler(MODEL_ERROR);
	}
      }
      else if (type == EXPONENTIAL_UNCERTAIN) {
	// TO DO: mean/std deviation or location/scale
	if (map2 == "beta")
	  active2ACVarMapTargets[curr_index] = Pecos::E_BETA;
	else {
	  Cerr << "\nError: " << map2 << " mapping not supported for "
	       << "exponential distributions." << std::endl;
	  abort_handler(MODEL_ERROR);
	}
      }
      else if (type == BETA_UNCERTAIN) {
	// TO DO: mean/std deviation or location/scale
	if (map2 == "alpha")
	  active2ACVarMapTargets[curr_index] = Pecos::BE_ALPHA;
	else if (map2 == "beta")
	  active2ACVarMapTargets[curr_index] = Pecos::BE_BETA;
	else if (map2 == "lower_bound")
	  active2ACVarMapTargets[curr_index] = Pecos::BE_LWR_BND;
	else if (map2 == "upper_bound")
	  active2ACVarMapTargets[curr_index] = Pecos::BE_UPR_BND;
	else {
	  Cerr << "\nError: " << map2 << " mapping not supported for "
	       << "beta distributions." << std::endl;
	  abort_handler(MODEL_ERROR);
	}
      }
      else if (type == GAMMA_UNCERTAIN) {
	// TO DO: mean/std deviation or location/scale
	if (map2 == "alpha")
	  active2ACVarMapTargets[curr_index] = Pecos::GA_ALPHA;
	else if (map2 == "beta")
	  active2ACVarMapTargets[curr_index] = Pecos::GA_BETA;
	else {
	  Cerr << "\nError: " << map2 << " mapping not supported for "
	       << "gamma distributions." << std::endl;
	  abort_handler(MODEL_ERROR);
	}
      }
      else if (type == GUMBEL_UNCERTAIN) {
	// TO DO: mean/std deviation or location/scale
	if (map2 == "alpha")
	  active2ACVarMapTargets[curr_index] = Pecos::GU_ALPHA;
	else if (map2 == "beta")
	  active2ACVarMapTargets[curr_index] = Pecos::GU_BETA;
	else {
	  Cerr << "\nError: " << map2 << " mapping not supported for "
	       << "gumbel distributions." << std::endl;
	  abort_handler(MODEL_ERROR);
	}
      }
      else if (type == FRECHET_UNCERTAIN) {
	// TO DO: mean/std deviation or location/scale
	if (map2 == "alpha")
	  active2ACVarMapTargets[curr_index] = Pecos::F_ALPHA;
	else if (map2 == "beta")
	  active2ACVarMapTargets[curr_index] = Pecos::F_BETA;
	else {
	  Cerr << "\nError: " << map2 << " mapping not supported for "
	       << "frechet distributions." << std::endl;
	  abort_handler(MODEL_ERROR);
	}
      }
      else if (type == WEIBULL_UNCERTAIN) {
	// TO DO: mean/std deviation or location/scale
	if (map2 == "alpha")
	  active2ACVarMapTargets[curr_index] = Pecos::W_ALPHA;
	else if (map2 == "beta")
	  active2ACVarMapTargets[curr_index] = Pecos::W_BETA;
	else {
	  Cerr << "\nError: " << map2 << " mapping not supported for "
	       << "weibull distributions." << std::endl;
	  abort_handler(MODEL_ERROR);
	}
      }
      else {
	Cerr << "\nError: " << type << " variable type not supported in "
	     << "secondary real mappings\n       for primary continuous "
	     << "variable targets." << std::endl;
	abort_handler(MODEL_ERROR);
      }
      active2ADIVarMapTargets[curr_index] = active2ADSVarMapTargets[curr_index]
	= active2ADRVarMapTargets[curr_index] = Pecos::NO_TARGET;
    }
    else if (adi_index1 != _NPOS) {
      unsigned short type
	= subModel.all_discrete_int_variable_types()[adi_index1];
      if (type == POISSON_UNCERTAIN) {
	if (map2 == "lambda")
	  active2ADIVarMapTargets[curr_index] = Pecos::P_LAMBDA;
	else {
	  Cerr << "\nError: " << map2 << " real mapping not supported for "
	       << "poisson uncertain variables." << std::endl;
	  abort_handler(MODEL_ERROR);
	}
      }
      if (type == BINOMIAL_UNCERTAIN) {
	if (map2 == "prob_per_trial")
	  active2ADIVarMapTargets[curr_index] = Pecos::BI_P_PER_TRIAL;
	else {
	  Cerr << "\nError: " << map2 << " real mapping not supported for "
	       << "binomial uncertain variables." << std::endl;
	  abort_handler(MODEL_ERROR);
	}
      }
      if (type == NEGATIVE_BINOMIAL_UNCERTAIN) {
	if (map2 == "prob_per_trial")
	  active2ADIVarMapTargets[curr_index] = Pecos::NBI_P_PER_TRIAL;
	else {
	  Cerr << "\nError: " << map2 << " real mapping not supported for "
	       << "negative binomial uncertain variables." << std::endl;
	  abort_handler(MODEL_ERROR);
	}
      }
      if (type == GEOMETRIC_UNCERTAIN) {
	if (map2 == "prob_per_trial")
	  active2ADIVarMapTargets[curr_index] = Pecos::GE_P_PER_TRIAL;
	else {
	  Cerr << "\nError: " << map2 << " real mapping not supported for "
	       << "geometric uncertain variables." << std::endl;
	  abort_handler(MODEL_ERROR);
	}
      }
      else {
	Cerr << "\nError: " << type << " variable type not supported in "
	     << "secondary real mappings\n       for primary discrete integer "
	     << "variable targets." << std::endl;
	abort_handler(MODEL_ERROR);
      }
      active2ACVarMapTargets[curr_index] = active2ADSVarMapTargets[curr_index]
	= active2ADRVarMapTargets[curr_index] = Pecos::NO_TARGET;
    }
    else if (ads_index1 != _NPOS) {
      unsigned short type
	= subModel.all_discrete_string_variable_types()[ads_index1];
      Cerr << "\nError: " << type << " variable type not supported in "
	   << "secondary real mappings\n       for primary discrete string "
	   << "variable targets." << std::endl;
      abort_handler(MODEL_ERROR);
      active2ACVarMapTargets[curr_index] = active2ADIVarMapTargets[curr_index]
	= active2ADRVarMapTargets[curr_index] = Pecos::NO_TARGET;
    }
    else if (adr_index1 != _NPOS) {
      unsigned short type
	= subModel.all_discrete_real_variable_types()[adr_index1];
      Cerr << "\nError: " << type << " variable type not supported in "
	   << "secondary real mappings\n       for primary discrete real "
	   << "variable targets." << std::endl;
      abort_handler(MODEL_ERROR);
      active2ACVarMapTargets[curr_index] = active2ADIVarMapTargets[curr_index]
	= active2ADSVarMapTargets[curr_index] = Pecos::NO_TARGET;
    }
  }
}


void NestedModel::
resolve_integer_variable_mapping(const String& map1, const String& map2,
				 size_t curr_index, short& inactive_sm_view)
{
  size_t ac_index1, adi_index1, ads_index1, adr_index1;
  resolve_map1(map1, ac_index1, adi_index1, ads_index1, adr_index1,
	       curr_index, inactive_sm_view);

  if (!active2ACVarMapTargets.empty()) { // indicates num_var_map_2
    if (map2.empty())
      active2ACVarMapTargets[curr_index] = active2ADIVarMapTargets[curr_index]
	= active2ADSVarMapTargets[curr_index]
	= active2ADRVarMapTargets[curr_index] = Pecos::NO_TARGET;
    else if (ac_index1 != _NPOS) {
      unsigned short type = subModel.all_continuous_variable_types()[ac_index1];
      Cerr << "\nError: " << type << " variable type not supported in "
	   << "secondary integer mappings\n       for primary continuous "
	   << "variable targets." << std::endl;
      abort_handler(MODEL_ERROR);
      active2ADIVarMapTargets[curr_index] = active2ADSVarMapTargets[curr_index]
	= active2ADRVarMapTargets[curr_index] = Pecos::NO_TARGET;
    }
    else if (adi_index1 != _NPOS) {
      unsigned short type
	= subModel.all_discrete_int_variable_types()[adi_index1];
      if (type == DISCRETE_DESIGN_RANGE || type == DISCRETE_STATE_RANGE) {
	if (map2 == "lower_bound")
	  active2ADIVarMapTargets[curr_index] = Pecos::DR_LWR_BND;
	else if (map2 == "upper_bound")
	  active2ADIVarMapTargets[curr_index] = Pecos::DR_UPR_BND;
	else {
	  Cerr << "\nError: " << map2 << " mapping not supported for "
	       << "discrete range variables." << std::endl;
	  abort_handler(MODEL_ERROR);
	}
      }
      else if (type == BINOMIAL_UNCERTAIN) {
	if (map2 == "num_trials")
	  active2ADIVarMapTargets[curr_index] = Pecos::BI_TRIALS;
	else {
	  Cerr << "\nError: " << map2 << " mapping not supported for "
	       << "binomial uncertain variables." << std::endl;
	  abort_handler(MODEL_ERROR);
	}
      }
      else if (type == NEGATIVE_BINOMIAL_UNCERTAIN) {
	if (map2 == "num_trials")
	  active2ADIVarMapTargets[curr_index] = Pecos::NBI_TRIALS;
	else {
	  Cerr << "\nError: " << map2 << " mapping not supported for "
	       << "negative binomial uncertain variables." << std::endl;
	  abort_handler(MODEL_ERROR);
	}
      }
      else if (type == HYPERGEOMETRIC_UNCERTAIN) {
	if (map2 == "total_population")
	  active2ADIVarMapTargets[curr_index] = Pecos::HGE_TOT_POP;
	else if (map2 == "selected_population")
	  active2ADIVarMapTargets[curr_index] = Pecos::HGE_SEL_POP;
	else if (map2 == "num_drawn")
	  active2ADIVarMapTargets[curr_index] = Pecos::HGE_DRAWN;
	else {
	  Cerr << "\nError: " << map2 << " mapping not supported for "
	       << "hypergeometric uncertain variables." << std::endl;
	  abort_handler(MODEL_ERROR);
	}
      }
      else {
	Cerr << "\nError: " << type << " variable type not supported in "
	     << "secondary integer mappings\n       for primary discrete "
	     << "integer variable targets." << std::endl;
	abort_handler(MODEL_ERROR);
      }
      active2ACVarMapTargets[curr_index] = active2ADSVarMapTargets[curr_index]
	= active2ADRVarMapTargets[curr_index] = Pecos::NO_TARGET;
    }
    else if (ads_index1 != _NPOS) {
      unsigned short type
	= subModel.all_discrete_string_variable_types()[ads_index1];
      Cerr << "\nError: " << type << " variable type not supported in "
	   << "secondary integer mappings\n       for primary discrete string "
	   << "variable targets." << std::endl;
      abort_handler(MODEL_ERROR);
      active2ACVarMapTargets[curr_index] = active2ADIVarMapTargets[curr_index]
	= active2ADRVarMapTargets[curr_index] = Pecos::NO_TARGET;
    }
    else if (adr_index1 != _NPOS) {
      unsigned short type
	= subModel.all_discrete_real_variable_types()[adr_index1];
      Cerr << "\nError: " << type << " variable type not supported in "
	   << "secondary integer mappings\n       for primary discrete real "
	   << "variable targets." << std::endl;
      abort_handler(MODEL_ERROR);
      active2ACVarMapTargets[curr_index] = active2ADIVarMapTargets[curr_index]
	= active2ADSVarMapTargets[curr_index] = Pecos::NO_TARGET;
    }
  }
}


void NestedModel::
resolve_string_variable_mapping(const String& map1, const String& map2,
				size_t curr_index, short& inactive_sm_view)
{
  size_t ac_index1, adi_index1, ads_index1, adr_index1;
  resolve_map1(map1, ac_index1, adi_index1, ads_index1, adr_index1,
	       curr_index, inactive_sm_view);

  if (!active2ACVarMapTargets.empty()) { // indicates num_var_map_2
    if (map2.empty())
      active2ACVarMapTargets[curr_index] = active2ADIVarMapTargets[curr_index]
	= active2ADSVarMapTargets[curr_index]
	= active2ADRVarMapTargets[curr_index] = Pecos::NO_TARGET;
    else if (ac_index1 != _NPOS) {
      unsigned short type = subModel.all_continuous_variable_types()[ac_index1];
      Cerr << "\nError: " << type << " variable type not supported in "
	   << "secondary string mappings\n       for primary continuous "
	   << "variable targets." << std::endl;
      abort_handler(MODEL_ERROR);
      active2ADIVarMapTargets[curr_index] = active2ADSVarMapTargets[curr_index]
	= active2ADRVarMapTargets[curr_index] = Pecos::NO_TARGET;
    }
    else if (adi_index1 != _NPOS) {
      unsigned short type
	= subModel.all_discrete_int_variable_types()[adi_index1];
      Cerr << "\nError: " << type << " variable type not supported in "
	   << "secondary string mappings\n       for primary discrete integer "
	   << "variable targets." << std::endl;
      abort_handler(MODEL_ERROR);
      active2ACVarMapTargets[curr_index] = active2ADSVarMapTargets[curr_index]
	= active2ADRVarMapTargets[curr_index] = Pecos::NO_TARGET;
    }
    else if (ads_index1 != _NPOS) {
      unsigned short type
	= subModel.all_discrete_string_variable_types()[ads_index1];
      Cerr << "\nError: " << type << " variable type not supported in "
	   << "secondary string mappings\n       for primary discrete string "
	   << "variable targets." << std::endl;
      abort_handler(MODEL_ERROR);
      active2ACVarMapTargets[curr_index] = active2ADIVarMapTargets[curr_index]
	= active2ADRVarMapTargets[curr_index] = Pecos::NO_TARGET;
    }
    else if (adr_index1 != _NPOS) {
      unsigned short type
	= subModel.all_discrete_real_variable_types()[adr_index1];
      Cerr << "\nError: " << type << " variable type not supported in "
	   << "secondary string mappings\n       for primary discrete real "
	   << "variable targets." << std::endl;
      abort_handler(MODEL_ERROR);
      active2ACVarMapTargets[curr_index] = active2ADIVarMapTargets[curr_index]
	= active2ADSVarMapTargets[curr_index] = Pecos::NO_TARGET;
    }
  }
}


/** Update subModel's inactive variables with active variables from
    currentVariables, compute the optional interface and sub-iterator
    responses, and map these to the total model response. */
void NestedModel::derived_evaluate(const ActiveSet& set)
{
  ++nestedModelEvalCntr;

  // Set currentResponse asv and extract opt_interface_set/sub_iterator_set
  currentResponse.active_set(set); currentResponse.reset();
  ActiveSet opt_interface_set, sub_iterator_set;
  bool      opt_interface_map, sub_iterator_map;
  set_mapping(set, opt_interface_set, opt_interface_map,
	           sub_iterator_set,  sub_iterator_map);

  // Perform optionalInterface map (opt_interface_set is updated within
  // optInterfaceResponse by map):
  if (opt_interface_map) {
    Cout << "\n----------------------------------------------------------------"
	 << "--\nNestedModel Evaluation " << std::setw(4) << nestedModelEvalCntr
	 << ": performing optional interface mapping\n-------------------------"
	 << "-----------------------------------------\n";
    component_parallel_mode(INTERFACE_MODE);
    if (hierarchicalTagging) {
      String eval_tag = evalTagPrefix + '.' +
	std::to_string(nestedModelEvalCntr);
      // don't apply a redundant interface eval id
      bool append_iface_tag = false;
      optionalInterface.eval_tag_prefix(eval_tag, append_iface_tag);
    }

    ParConfigLIter pc_iter = parallelLib.parallel_configuration_iterator();
    parallelLib.parallel_configuration_iterator(modelPCIter);
    if(interfEvaluationsDBState == EvaluationsDBState::UNINITIALIZED)
      interfEvaluationsDBState = evaluationsDB.interface_allocate(modelId, 
          interface_id(), "simulation", currentVariables, optInterfaceResponse, 
          default_interface_active_set(), optionalInterface.analysis_components());

    optionalInterface.map(currentVariables, opt_interface_set,
			  optInterfaceResponse);
    if(interfEvaluationsDBState == EvaluationsDBState::ACTIVE) {
      evaluationsDB.store_interface_variables(modelId, interface_id(),
          optionalInterface.evaluation_id(), opt_interface_set, currentVariables);
      evaluationsDB.store_interface_response(modelId, interface_id(),
          optionalInterface.evaluation_id(), optInterfaceResponse);
    }
    parallelLib.parallel_configuration_iterator(pc_iter); // restore

    // map optInterface results into their contribution to currentResponse
    interface_response_overlay(optInterfaceResponse, currentResponse);
  }

  if (sub_iterator_map) {
    //++subIteratorJobCntr; // does not encompass blocking evals

    // need comm set up and master break off
    // (see IteratorScheduler::run_iterator())
    Cout << "\n-------------------------------------------------\nNestedModel "
	 << "Evaluation " << std::setw(4) << nestedModelEvalCntr << ": running "
	 << "sub_iterator\n-------------------------------------------------\n";
    component_parallel_mode(SUB_MODEL_MODE);
    update_sub_model(currentVariables, userDefinedConstraints);
    subIterator.response_results_active_set(sub_iterator_set);
    if (hierarchicalTagging) {
      String eval_tag = evalTagPrefix + '.' +
	std::to_string(nestedModelEvalCntr);
      subIterator.eval_tag_prefix(eval_tag);
    }

    ParLevLIter pl_iter
      = modelPCIter->mi_parallel_level_iterator(subIteratorSched.miPLIndex);
    if (subIteratorSched.messagePass) {
      // For derived_evaluate(), subIterator scheduling would not
      // normally be expected, but singleton jobs could use this fn assuming
      // no dedicated master overload (enforced in Model::evaluate()).
      // Given this protection, don't schedule the job -- execute it locally.
      if (subIteratorSched.iteratorScheduling == PEER_SCHEDULING &&
	  subIteratorSched.peerAssignJobs) {
	// match 2 bcasts in IteratorScheduler::peer_static_schedule_iterators()
	// needed by procs in NestedModel::serve_run()
	int num_jobs = 1;
	parallelLib.bcast_hs(num_jobs, *pl_iter); // over pl.hubServerIntraComm
	if (subIteratorSched.iteratorCommSize > 1)
	  parallelLib.bcast(num_jobs, *pl_iter);  // over pl.serverIntraComm
      }
      // run_iterator() is used since we stop subModel servers for consistency
      // with fall through behavior of schedule_iterators()
      subIteratorSched.run_iterator(subIterator, pl_iter);
      if (subIteratorSched.iteratorScheduling == MASTER_SCHEDULING)
	subIteratorSched.stop_iterator_servers();

      /* This approach has 2 issues: (1) a single-processor subIterator job is
	 always assigned by master to server 1 (ded master overload bypassed),
	 (2) peer static init/update bookkeeping is redundant of above/below.
      subIteratorSched.numIteratorJobs = 1;
      // can use shallow copy for queue of 1 job (avoids need to copy updated
      // entry in subIteratorPRPQueue back to subIterator.response_results())
      ParamResponsePair current_pair(currentVariables, subIterator.method_id(),
				     subIterator.response_results(), 1, false);
      subIteratorPRPQueue.insert(current_pair);
      subIteratorSched.schedule_iterators(*this, subIterator);
      */
    }
    else // run_iterator() is not used since we don't stop subModel servers
         // until change in component_parallel_mode
      subIterator.run(pl_iter);

    const Response& sub_iter_resp = subIterator.response_results();
    Cout << "\nActive response data from sub_iterator:\n"<< sub_iter_resp<<'\n';
    // map subIterator results into their contribution to currentResponse
    iterator_response_overlay(sub_iter_resp, currentResponse);
  }

  Cout << "\n---------------------------\nNestedModel Evaluation "
       << std::setw(4) << nestedModelEvalCntr << " results:"
       << "\n---------------------------\n";
  // if secondary variable mappings (distribution parameter insertions), the
  // Nested parameters are not directly reflected in the subIterator output:
  if (outputLevel >= VERBOSE_OUTPUT && !active2ACVarMapTargets.empty())
    Cout << "Nested parameters:\n" << currentVariables;
  Cout << "\nActive response data from nested mapping:\n" << currentResponse
       << '\n';
}


/** Asynchronous execution of subIterator on subModel and, optionally,
    optionalInterface. */
void NestedModel::derived_evaluate_nowait(const ActiveSet& set)
{
  ++nestedModelEvalCntr;

  // Set currentResponse asv and extract opt_interface_set/sub_iterator_set
  currentResponse.active_set(set);
  ActiveSet opt_interface_set, sub_iterator_set;
  bool      opt_interface_map, sub_iterator_map;
  set_mapping(set, opt_interface_set, opt_interface_map,
	           sub_iterator_set,  sub_iterator_map);

  // Perform optionalInterface map (opt_interface_set is updated within
  // optInterfaceResponse by map):
  if (opt_interface_map) {
    Cout << "\n----------------------------------------------------------------"
	 << "--\nNestedModel Evaluation " << std::setw(4)
	 << nestedModelEvalCntr << ": queueing optional interface mapping\n"
	 << "------------------------------------------------------------------"
	 << '\n';
    // don't need to set component parallel mode since this only queues the job
    if(interfEvaluationsDBState == EvaluationsDBState::UNINITIALIZED)
      interfEvaluationsDBState = evaluationsDB.interface_allocate(modelId, interface_id(), 
          "simulation", currentVariables, optInterfaceResponse, default_interface_active_set(),
          optionalInterface.analysis_components());
    optionalInterface.map(currentVariables, opt_interface_set,
			  optInterfaceResponse, true);
    if(interfEvaluationsDBState == EvaluationsDBState::ACTIVE)
      evaluationsDB.store_interface_variables(modelId, interface_id(),
          optionalInterface.evaluation_id(), opt_interface_set, currentVariables);
    optInterfaceIdMap[optionalInterface.evaluation_id()] = nestedModelEvalCntr;
  }

  if (sub_iterator_map) {
    ++subIteratorJobCntr;

    // need comm set up and master break off
    // (see IteratorScheduler::run_iterator())
    Cout << "\n-------------------------------------------------\n"
	 << "NestedModel Evaluation " << std::setw(4) << nestedModelEvalCntr 
	 << ": queueing sub_iterator"
	 << "\n-------------------------------------------------\n";

    // load up queue of iterator jobs to be scheduled in derived synchronize:
    // > use the subIterator's method id as the PRP interface id
    // > subIterator's execNum could potentially be used as execution counter
    //   for id mapping but it isn't defined/incremented until run time (see
    //   Iterator::run())
    // > simplest approach of tagging with nestedModelEvalCntr is sufficient,
    //   since we do not need to map from a set of eval ids returned from
    //   lower level bookkeeping
    subIterator.response_results_active_set(sub_iterator_set);
    ParamResponsePair current_pair(currentVariables, subIterator.method_id(),
				   subIterator.response_results(),
				   nestedModelEvalCntr);
    subIteratorPRPQueue.insert(current_pair);

    // update bookkeeping for job_index mappings in IteratorScheduler callbacks
    subIteratorIdMap[subIteratorJobCntr] = nestedModelEvalCntr;
  }
}


/** Recovery of asynchronous subIterator executions and, optionally,
    asynchronous optionalInterface mappings. */
const IntResponseMap& NestedModel::derived_synchronize()
{
  nestedResponseMap.clear();

  // TO DO: optInt/subIter scheduling is currently sequential, but could be
  // overlapped as in EnsembleSurrModel, given IteratorScheduler nowait support

  IntIntMIter id_it; IntRespMCIter r_cit;
  if (!optInterfacePointer.empty()) {
    component_parallel_mode(INTERFACE_MODE);

    ParConfigLIter pc_iter = parallelLib.parallel_configuration_iterator();
    parallelLib.parallel_configuration_iterator(modelPCIter);
    const IntResponseMap& opt_int_resp_map = optionalInterface.synchronize();
    parallelLib.parallel_configuration_iterator(pc_iter); // restore

    // overlay response sets
    r_cit = opt_int_resp_map.begin();
    while (r_cit != opt_int_resp_map.end()) {
      int oi_eval_id = r_cit->first;
      id_it = optInterfaceIdMap.find(oi_eval_id);
      if (id_it != optInterfaceIdMap.end()) {
	interface_response_overlay(r_cit->second,
				   nested_response(id_it->second));
	optInterfaceIdMap.erase(id_it);
	++r_cit;
      }
      else { // see also Model::rekey_synch()
	++r_cit; // prior to invalidation from erase within cache_unmatched
	optionalInterface.cache_unmatched_response(oi_eval_id);
      }
    }
  }

  if (!subIteratorPRPQueue.empty()) {
    // schedule subIteratorPRPQueue jobs
    component_parallel_mode(SUB_MODEL_MODE);
    subIteratorSched.numIteratorJobs = subIteratorPRPQueue.size();
    subIteratorSched.schedule_iterators(*this, subIterator);
    // overlay response sets (no rekey or cache necessary)
    for (PRPQueueIter q_it=subIteratorPRPQueue.begin();
	 q_it!=subIteratorPRPQueue.end(); ++q_it)
      iterator_response_overlay(q_it->response(),
				nested_response(q_it->eval_id()));
    // clear sub-iterator jobs
    subIteratorPRPQueue.clear();
    // Reset bookkeeping used in IteratorScheduler callbacks (e.g.,
    // {pack,unpack}_* in NestedModel.hpp); sub-iterator job counter
    // mirrors the passed job_index and maps to nestedModelEvalCntr
    // for subIteratorPRPQueue lookups.
    subIteratorIdMap.clear(); subIteratorJobCntr = 0;
  }

  //nestedVarsMap.clear();
  for (r_cit=nestedResponseMap.begin(); r_cit!=nestedResponseMap.end(); ++r_cit)
    Cout << "\n---------------------------\nNestedModel Evaluation "
	 << std::setw(4) << r_cit->first << " total response:"
	 << "\n---------------------------\n\nActive response data "
	 << "from nested mapping:\n" << r_cit->second << '\n';
  return nestedResponseMap;
}


/* Asynchronous response computations are not currently supported by
   NestedModels.  Return a dummy to satisfy the compiler.
const IntResponseMap& NestedModel::derived_synchronize_nowait()
{
  // TO DO: will require nowait support in IteratorScheduler

  //nestedVarsMap.erase(eval_id);
  return nestedResponseMap;
}
*/


void NestedModel::
set_mapping(const ActiveSet& mapped_set, ActiveSet& opt_interface_set,
	    bool& opt_interface_map,     ActiveSet& sub_iterator_set,
	    bool& sub_iterator_map)
{
  size_t i, j, num_opt_interf_con = numOptInterfIneqCon + numOptInterfEqCon,
    num_mapped_primary = std::max(numOptInterfPrimary, subIterMappedPri);

  const ShortArray& mapped_asv = mapped_set.request_vector();
  const SizetArray& mapped_dvv = mapped_set.derivative_vector();

  if (mapped_asv.size() != num_mapped_primary + num_opt_interf_con +
                           subIterMappedSec) {
    Cerr << "\nError: mismatch is ASV lengths in NestedModel::set_mapping()."
	 << "\n       expected " << mapped_asv.size() << " total, received "
	 << num_mapped_primary << " primary plus " << num_opt_interf_con +
            subIterMappedSec << " secondary." << std::endl;
    abort_handler(MODEL_ERROR);
  }

  // sub_iterator_asv:

  sub_iterator_map = false;
  ShortArray sub_iterator_asv(numSubIterFns, 0);
  // augment sub_iterator_asv based on mapped primary asv and primaryRespCoeffs
  for (i=0; i<subIterMappedPri; ++i) {
    short mapped_asv_val = mapped_asv[i];
    if (mapped_asv_val) {
      if (identityRespMap)
	sub_iterator_asv[i] |= mapped_asv_val;
      else
	for (j=0; j<numSubIterFns; ++j)
	  if (std::fabs(primaryRespCoeffs(i,j)) > DBL_MIN)
	    sub_iterator_asv[j] |= mapped_asv_val;
      sub_iterator_map = true;
    }
  }
  // augment sub_iterator_asv based on mapped constr asv and secondaryRespCoeffs
  for (i=0; i<subIterMappedSec; ++i) {
    short mapped_asv_val = mapped_asv[i+num_mapped_primary+num_opt_interf_con];
    if (mapped_asv_val) {
      if (identityRespMap)
	sub_iterator_asv[subIterMappedPri + i] |= mapped_asv_val;
      else
	for (j=0; j<numSubIterFns; ++j)
	  if (std::fabs(secondaryRespCoeffs(i,j)) > DBL_MIN)
	    sub_iterator_asv[j] |= mapped_asv_val;
      sub_iterator_map = true;
    }
  }
  if (sub_iterator_map) {
    sub_iterator_set.request_vector(sub_iterator_asv);

    // sub_iterator_dvv:

    SizetMultiArrayConstView cv_ids
      = currentVariables.continuous_variable_ids();
    SizetMultiArrayConstView sm_acv_ids
      = subModel.all_continuous_variable_ids();

    /* Old Approach: subIterator must decipher/replace top-level DVV as reqd.
    // Note: the ordering of top-level active variables may differ from the
    // ordering of the inactive sub-model variables.  However, the subModel's
    // inactive_continuous_variable_ids() omit inserted variables, which would
    // result in erroneous subIterator final response array sizing.  Therefore,
    // use the top-level active variable ids and rely on the subIterator to
    // perform the mappings to the subModel variables.
    sub_iterator_set.derivative_vector(cv_ids);
    */

    size_t num_mapped_dvv = mapped_dvv.size();
    SizetArray sub_iterator_dvv;
    bool var_map_2 = !active2ACVarMapTargets.empty();
    if (!var_map_2)
      sub_iterator_dvv.resize(num_mapped_dvv);
    for (i=0; i<num_mapped_dvv; ++i) {
      size_t cv_index = find_index(cv_ids, mapped_dvv[i]);
      if (cv_index == _NPOS) {
	Cerr << "\nError: NestedModel DVV component not contained within "
	     << "active continuous variable ids." << std::endl;
	abort_handler(MODEL_ERROR);
      }
      size_t sm_acv_index = active1ACVarMapIndices[cv_index],
	     sm_acv_id    = sm_acv_ids[sm_acv_index];
      if (var_map_2) { // enforce uniqueness in insertion targets
	if (!contains(sub_iterator_dvv, sm_acv_id))
	  sub_iterator_dvv.push_back(sm_acv_id);
      }
      else
	sub_iterator_dvv[i] = sm_acv_id;
    }
    sub_iterator_set.derivative_vector(sub_iterator_dvv);
    //Cout << "\nmapped_dvv:\n" << mapped_dvv << "\nsub_iterator_dvv:\n"
    //     << sub_iterator_dvv << '\n';
  }

  // opt_interface_asv:

  // num_mapped_primary >= numOptInterfPrimary with 1-to-1 correspondence
  // up to the cut off
  opt_interface_map = false;
  size_t num_opt_interf_fns = numOptInterfPrimary+num_opt_interf_con;
  ShortArray opt_interface_asv(num_opt_interf_fns);
  for (i=0; i<numOptInterfPrimary; ++i)
    opt_interface_asv[i] = mapped_asv[i];
  // num_opt_interf_con has 1-to-1 correspondence with different offsets
  for (i=0; i<num_opt_interf_con; ++i)
    opt_interface_asv[i+numOptInterfPrimary] = mapped_asv[i+num_mapped_primary];
  // Special case of forcing an optional interface execution that lacks fns:
  // this allows usage of the optional interface to generate data used only
  // by the sub-iterator.  Put another way, the optional interface is active
  // unless functions are present and all functions are inactive.
  if (!optInterfacePointer.empty() && num_opt_interf_fns == 0)
    opt_interface_map = sub_iterator_map;
  else // normal case of mapping optional interface fns
    for (i=0; i<num_opt_interf_fns; ++i)
      if (opt_interface_asv[i])
	{ opt_interface_map = true; break; }
  if (opt_interface_map) {
    opt_interface_set.request_vector(opt_interface_asv);

    // opt_interface_dvv:

    opt_interface_set.derivative_vector(mapped_dvv);
  }
}


void NestedModel::
interface_response_overlay(const Response& opt_interface_response,
			   Response& mapped_response)
{
  // mapped data initialization
  const ShortArray& mapped_asv = mapped_response.active_set_request_vector();
  const SizetArray& mapped_dvv = mapped_response.active_set_derivative_vector();
  size_t i, m_index, oi_index, num_mapped_fns = mapped_asv.size();
  bool deriv_flag = false;
  for (i=0; i<num_mapped_fns; ++i)
    { if (mapped_asv[i] & 6) deriv_flag = true; break; }
  // Sanity checks: the optional interface response must have the same DVV
  if ( deriv_flag &&
       opt_interface_response.active_set_derivative_vector() != mapped_dvv ) {
    Cerr << "\nError: derivative variables vector mismatch in NestedModel::"
         << "interface_response_overlay()." << std::endl;
    abort_handler(MODEL_ERROR);
  }
  check_response_map(mapped_asv);

  // build mapped response data:

  // {f}:
  for (i=0; i<numOptInterfPrimary; ++i) {
    if (mapped_asv[i] & 1) 
      mapped_response.function_value(
	opt_interface_response.function_value(i), i);
    if (mapped_asv[i] & 2)
      mapped_response.function_gradient(
	opt_interface_response.function_gradient_view(i), i);
    if (mapped_asv[i] & 4)
      mapped_response.function_hessian(
	opt_interface_response.function_hessian(i), i);
  }

  // {g}:
  size_t num_opt_interf_con = numOptInterfIneqCon + numOptInterfEqCon,
    num_mapped_1
      = std::max(numOptInterfPrimary, subIterMappedPri);
  for (i=0; i<num_opt_interf_con; ++i) {
    oi_index = numOptInterfPrimary + i;
    m_index  = num_mapped_1 + i; // {g_l} <= {g} <= {g_u}
    if (i>=numOptInterfIneqCon)  //          {g} == {g_t}
      m_index += numSubIterMappedIneqCon;
    if (mapped_asv[m_index] & 1) // mapped_vals
      mapped_response.function_value(
	opt_interface_response.function_value(oi_index), m_index);
    if (mapped_asv[m_index] & 2) // mapped_grads
      mapped_response.function_gradient(
	opt_interface_response.function_gradient_view(oi_index), m_index);
    if (mapped_asv[m_index] & 4) // mapped_hessians
      mapped_response.function_hessian(
	opt_interface_response.function_hessian(oi_index), m_index);
  }
}


void NestedModel::
iterator_response_overlay(const Response& sub_iterator_response,
			  Response& mapped_response)
{
  // mapped data initialization
  const ShortArray& mapped_asv = mapped_response.active_set_request_vector();
  const SizetArray& mapped_dvv = mapped_response.active_set_derivative_vector();
  size_t i, j, k, l, m_index, num_mapped_fns = mapped_asv.size(),
    num_mapped_deriv_vars = mapped_dvv.size();
  bool deriv_flag = false;
  for (i=0; i<num_mapped_fns; ++i)
    if (mapped_asv[i] & 6) { deriv_flag = true; break; }
  // Sanity checks: the derivatives in the sub-iterator response must be with
  // respect to the same variables; but since the numbering may be different
  // following insertion/augmentation, only the DVV length is verified.
  if ( deriv_flag && num_mapped_deriv_vars !=
       sub_iterator_response.active_set_derivative_vector().size() ) {
    Cerr << "\nError: derivative variables vector mismatch in NestedModel::"
         << "iterator_response_overlay()." << std::endl;
    abort_handler(MODEL_ERROR);
  }
  check_response_map(mapped_asv);

  // build mapped response data:

  RealVector mapped_vals = mapped_response.function_values_view();
  const RealVector& sub_iterator_vals = sub_iterator_response.function_values();
  const RealMatrix& sub_iterator_grads
    = sub_iterator_response.function_gradients();
  const RealSymMatrixArray& sub_iterator_hessians
    = sub_iterator_response.function_hessians();
  Real coeff;

  // [W]{S}:
  for (i=0; i<subIterMappedPri; ++i) {
    if (mapped_asv[i] & 1) { // mapped_vals
      if (identityRespMap)
	mapped_vals[i] = sub_iterator_vals[i];
      else {
	Real& inner_prod = mapped_vals[i];
	for (j=0; j<numSubIterFns; ++j) {
	  coeff = primaryRespCoeffs(i,j);
	  if (coeff != 0.) // avoid propagation of nan/inf for no mapping
	    inner_prod += coeff * sub_iterator_vals[j]; // [W]{S}
	}
      }
    }
    if (mapped_asv[i] & 2) { // mapped_grads
      RealVector mapped_grad = mapped_response.function_gradient_view(i);
      for (j=0; j<num_mapped_deriv_vars; ++j) {
	if (identityRespMap)
	  mapped_grad[j] = sub_iterator_grads(j,i);
	else {
	  Real& inner_prod = mapped_grad[j]; // [W]{S}
	  for (k=0; k<numSubIterFns; ++k) {
	    coeff = primaryRespCoeffs(i,k);
	    if (coeff != 0.) // avoid propagation of nan/inf for no mapping
	      inner_prod += coeff * sub_iterator_grads(j,k);
	  }
	}
      }
    }
    if (mapped_asv[i] & 4) { // mapped_hessians
      RealSymMatrix mapped_hess = mapped_response.function_hessian_view(i);
      for (j=0; j<num_mapped_deriv_vars; ++j) {
	for (k=0; k<=j; ++k) {
	  if (identityRespMap)
	    mapped_hess(j,k) = sub_iterator_hessians[i](j,k);
	  else {
	    Real& inner_prod = mapped_hess(j,k); // [W]{S}
	    for (l=0; l<numSubIterFns; ++l) {
	      coeff = primaryRespCoeffs(i,l);
	      if (coeff != 0.) // avoid propagation of nan/inf for no mapping
		inner_prod += coeff * sub_iterator_hessians[l](j,k);
	    }
	  }
	}
      }
    }
  }

  // [A]{S}:
  size_t num_mapped_1 = std::max(numOptInterfPrimary, subIterMappedPri);
  for (i=0; i<subIterMappedSec; ++i) {
    m_index = num_mapped_1 + numOptInterfIneqCon + i;// {a_l} <= [A]{S} <= {a_u}
    if (i>=numSubIterMappedIneqCon)
      m_index += numOptInterfEqCon;                           // [A]{S} == {a_t}
    // BMA: Using m_index is only safe for the identity map case
    // because optional interfaces are disallowed and all the opt
    // interf sizes will be 0
    if (mapped_asv[m_index] & 1) { // mapped_vals
      if (identityRespMap)
	mapped_vals[m_index] = sub_iterator_vals[m_index];
      else {
	Real& inner_prod = mapped_vals[m_index]; inner_prod = 0.;
	for (j=0; j<numSubIterFns; ++j) {
	  coeff = secondaryRespCoeffs(i,j);
	  if (coeff != 0.) // avoid propagation of nan/inf for no mapping
	    inner_prod += coeff * sub_iterator_vals[j];
	}
      }
    }
    if (mapped_asv[m_index] & 2) { // mapped_grads
      RealVector mapped_grad = mapped_response.function_gradient_view(m_index);
      for (j=0; j<num_mapped_deriv_vars; ++j) {
	if (identityRespMap)
	  mapped_grad[m_index] = sub_iterator_grads(j,m_index);
	else {
	  Real& inner_prod = mapped_grad[j]; inner_prod = 0.;
	  for (k=0; k<numSubIterFns; ++k) {
	    coeff = secondaryRespCoeffs(i,k);
	    if (coeff != 0.) // avoid propagation of nan/inf for no mapping
	      inner_prod += coeff * sub_iterator_grads(j,k);
	  }
	}
      }
    }
    if (mapped_asv[m_index] & 4) { // mapped_hessians
      RealSymMatrix mapped_hess
        = mapped_response.function_hessian_view(m_index);
      for (j=0; j<num_mapped_deriv_vars; ++j) {
        for (k=0; k<=j; ++k) {
	  if (identityRespMap)
	    mapped_hess(j,k) = sub_iterator_hessians[m_index](j,k);
	  else {
	    Real& inner_prod = mapped_hess(j,k); inner_prod = 0.;
	    for (l=0; l<numSubIterFns; ++l) {
	      coeff = secondaryRespCoeffs(i,l);
	      if (coeff != 0.) // avoid propagation of nan/inf for no mapping
		inner_prod += coeff * sub_iterator_hessians[l](j,k);
	    }
	  }
	}
      }
    }
  }
}


void NestedModel::
iterator_error_estimation(const RealSymMatrix& sub_iterator_errors,
        RealVector& mapped_errors)
{
  // In the future, could be overlaid with optional interface error estimates,
  // but for now, assume these are zero (e.g., deterministic mappings have no
  // estimator variance)

  if (sub_iterator_errors.empty()) {
    Cerr << "Error: sub_iterator_errors are undefined in NestedModel::"
   << "`().\n       Check error estimation "
   << "support in sub-method." << std::endl;
    abort_handler(MODEL_ERROR);
  }

  /*
  Cout << "## sub_iterator_erros size: " << sub_iterator_errors.length() << std::endl;
  Cout << "## subIterMappedPri: " << subIterMappedPri << " numSubIterFns: " << numSubIterFns << std::endl;
  Cout << "## subIterMappedSec: " << subIterMappedSec << " numSubIterMappedIneqCon: " << numSubIterMappedIneqCon << std::endl;
  Cout << "## numOptInterfPrimary: " << numOptInterfPrimary << " numOptInterfIneqCon: " << numOptInterfIneqCon << " numOptInterfEqCon: " << numOptInterfEqCon << std::endl;
  */
  size_t i, j, k, m_index, num_mapped_fns = currentResponse.num_functions();
  if (static_cast<unsigned>(mapped_errors.length()) != num_mapped_fns)
    mapped_errors.size(num_mapped_fns); // init to 0 
  else
    mapped_errors = 0.;

  // Assume independent Gaussian errors: then std error from linear combination
  // of Gaussian errors = Sqrt[ Sum [ coeff^2 sigma_i^2 ] ]
  // Note: final moments may be central or standard, but error estimates are
  //       always standard (sqrt of estimator variance of central/std moment)
  // [W]{S}:
  Real sum, term, coeff, coeff2;
  for (i=0; i<subIterMappedPri; ++i) {
    if (identityRespMap)
      mapped_errors[i] = sub_iterator_errors(2*i, 2*i);
    else {
      sum = 0.;
      for (j=0; j < numSubIterFns/2; ++j) { //Nb of sub responses
        Real coeff1 = primaryRespCoeffs(i,2*j);
        Real coeff2 = primaryRespCoeffs(i,(2*j)+1);
        if (coeff1 != 0.) { 
          term = coeff1 * sub_iterator_errors(2*j, 2*j); // [W]{S} Mean term (Diagonal)
          sum += term * term;
        }
        if (coeff2 != 0.) { 
          term = coeff2 * sub_iterator_errors(2*j+1, 2*j+1); // [W]{S} Var/Sigma (Diagonal)
          sum += term * term;
        }
        if (coeff1 != 0 && coeff2 != 0.) { 
          term = 2. * coeff1 * coeff2 * sub_iterator_errors(2*j+1, 2*j); // [W]{S} Cov (Subdiagonal)
          sum += term;
        }
      }
      mapped_errors[i] = std::sqrt(sum);
    }
  }

  // [A]{S}:
  size_t num_mapped_1 = std::max(numOptInterfPrimary, subIterMappedPri);
  for (i=0; i<subIterMappedSec; ++i) {
    m_index = num_mapped_1 + numOptInterfIneqCon + i;// {a_l} <= [A]{S} <= {a_u}
    if (i>=numSubIterMappedIneqCon)
      m_index += numOptInterfEqCon;                           // [A]{S} == {a_t}
    if (identityRespMap)
      mapped_errors[m_index] = sub_iterator_errors(2*i, 2*i);
    else {
      sum = 0.;
      for (j=0; j < numSubIterFns/2; ++j) { //Nb of sub responses
        Real coeff1 = secondaryRespCoeffs(i,2*j);
        Real coeff2 = secondaryRespCoeffs(i,(2*j)+1);
        if (coeff1 != 0.) { 
          term = coeff1 * sub_iterator_errors(2*j, 2*j); // [W]{S} Mean term
          sum += term * term;
        }
        if (coeff2 != 0.) { 
          term = coeff2 * sub_iterator_errors(2*j+1, 2*j+1); // [W]{S} Var/Sigma
          sum += term * term;
        }
        if (coeff1 != 0 && coeff2 != 0.) { 
          term = 2. * coeff1 * coeff2 * sub_iterator_errors(2*j+1, 2*j); // [W]{S} Cov
          sum += term;
        }
      }
      mapped_errors[m_index] = std::sqrt(sum);
    }
  }
}


void NestedModel::check_response_map(const ShortArray& mapped_asv)
{
  // counter initialization & sanity checking
  // NOTE: numSubIterFns != subIterMappedPri + subIterMappedSec
  // since subIterator response is converted to sub_iter_mapped_primary/sec
  // through the action of [W] and [A].
  size_t num_opt_interf_con = numOptInterfIneqCon + numOptInterfEqCon,
    num_mapped_1 = std::max(numOptInterfPrimary, subIterMappedPri);
  if (mapped_asv.size() !=
      num_mapped_1 + num_opt_interf_con + subIterMappedSec ||
      subIterMappedSec != numSubIterMappedIneqCon + numSubIterMappedEqCon) {
    Cerr << "\nError: bad function counts in NestedModel::check_response_map()."
         << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void NestedModel::component_parallel_mode(short mode)
{
  // mode may be correct, but can't guarantee active parallel config is in sync
  //if (componentParallelMode == mode)
  //  return; // already in correct parallel mode

  // terminate previous serve mode (if active)
  if (componentParallelMode != mode) {
    if (componentParallelMode == INTERFACE_MODE) {
      size_t index = subIteratorSched.miPLIndex;
      if (modelPCIter->mi_parallel_level_defined(index) && 
	  modelPCIter->mi_parallel_level(index).server_communicator_size() > 1){
	ParConfigLIter pc_iter = parallelLib.parallel_configuration_iterator();
	parallelLib.parallel_configuration_iterator(modelPCIter);
	optionalInterface.stop_evaluation_servers();
	parallelLib.parallel_configuration_iterator(pc_iter); // restore
      }
    }
    // concurrent subIterator scheduling exits on its own (see IteratorScheduler
    // ::schedule_iterators(), but subModel eval scheduling is terminated here.
    else if (componentParallelMode == SUB_MODEL_MODE &&
	     !subIteratorSched.messagePass) {
      ParConfigLIter pc_it = subModel.parallel_configuration_iterator();
      size_t index = subModel.mi_parallel_level_index();
      if (pc_it->mi_parallel_level_defined(index) && 
	  pc_it->mi_parallel_level(index).server_communicator_size() > 1)
	subModel.stop_servers();
    }
  }

  /* Moved up a level so that config can be restored after optInterface usage
  // set ParallelConfiguration for new mode
  if (mode == INTERFACE_MODE)
    parallelLib.parallel_configuration_iterator(modelPCIter);
  else if (mode == SUB_MODEL_MODE) {
    // ParallelLibrary::currPCIter activation delegated to subModel
  }
  */

  // activate new serve mode (matches NestedModel::serve_run(pl_iter)).  This
  // bcast matches the outer parallel context prior to subIterator partitioning.
  // > INTERFACE_MODE & subModel eval scheduling only broadcasts
  //   for mode change
  // > concurrent subIterator scheduling rebroadcasts every time since this
  //   scheduling exits on its own (see IteratorScheduler::schedule_iterators())
  if ( ( componentParallelMode != mode ||
	 ( mode == SUB_MODEL_MODE && subIteratorSched.messagePass ) ) &&
       modelPCIter->mi_parallel_level_defined(outerMIPLIndex) ) {
    const ParallelLevel& mi_pl = modelPCIter->mi_parallel_level(outerMIPLIndex);
    if (mi_pl.server_communicator_size() > 1)
      parallelLib.bcast(mode, mi_pl);
  }

  componentParallelMode = mode;
}


void NestedModel::serve_run(ParLevLIter pl_iter, int max_eval_concurrency)
{
  // don't recurse, as subModel.serve() will set subModel comms
  set_communicators(pl_iter, max_eval_concurrency, false);

  // manage optionalInterface and subModel servers
  componentParallelMode = 1;
  while (componentParallelMode) {
    // outer context: matches bcast at bottom of component_parallel_mode()
    parallelLib.bcast(componentParallelMode, *pl_iter);
    if (componentParallelMode == INTERFACE_MODE &&
	!optInterfacePointer.empty()) {
      // store/set/restore the ParallelLibrary::currPCIter
      ParConfigLIter pc_iter = parallelLib.parallel_configuration_iterator();
      parallelLib.parallel_configuration_iterator(modelPCIter);
      optionalInterface.serve_evaluations();
      parallelLib.parallel_configuration_iterator(pc_iter); // restore
    }
    else if (componentParallelMode == SUB_MODEL_MODE) {
      if (subIteratorSched.messagePass) // serve concurrent subIterator execs
	subIteratorSched.schedule_iterators(*this, subIterator);
      else { // service the subModel for a single subIterator execution
	ParLevLIter si_pl_iter // inner context
	  = modelPCIter->mi_parallel_level_iterator(subIteratorSched.miPLIndex);
	subModel.serve_run(si_pl_iter,
			   subIterator.maximum_evaluation_concurrency());
      }
    }
  }
}


void NestedModel::update_inactive_view(short new_view, short& view)
{
  // Perform view aggregation/separation.  Additional logic and sanity
  // checking occurs within Variables::inactive_view() and Variables::
  // check_view_compatibility().

  if (new_view == RELAXED_ALL || new_view == MIXED_ALL) {
    // outer level has an ALL view --> infer what subset of this view should
    // be inner inactive by computing the complement of the inner active.
    // Can't use inactive types/ids, since inactive is either not defined
    // or not up to date.
    const Variables& sm_vars = subModel.current_variables();
    size_t i, num_sm_acv = sm_vars.acv(), num_sm_cv = sm_vars.cv(),
      sm_cv_start = sm_vars.cv_start();
    UShortMultiArrayConstView sm_acv_types
      = sm_vars.all_continuous_variable_types();
    bool relaxed = (new_view == RELAXED_ALL);
    // TO DO: THIS IS NOT GOOD ENOUGH SINCE THIS DOES NOT DISCRIMINATE INACTIVE
    // VARIABLES THAT ARE ACTUALLY BEING MAPPED FROM INACTIVE VARIABLES THAT
    // ARE MERELY PRESENT.
    for (i=0; i<num_sm_acv; ++i)
      if (i < sm_cv_start || i >= sm_cv_start+num_sm_cv) { // inactive
	unsigned short type_i = sm_acv_types[i];
	if (type_i == CONTINUOUS_DESIGN ||
	    ( type_i >= DISCRETE_DESIGN_RANGE &&
	      type_i <= DISCRETE_DESIGN_SET_REAL ) )
	  view = (relaxed) ? RELAXED_DESIGN : MIXED_DESIGN;
	else if ( type_i == CONTINUOUS_STATE ||
		  ( type_i >= DISCRETE_STATE_RANGE &&
		    type_i <= DISCRETE_STATE_SET_REAL ) )
	  view = (relaxed) ? RELAXED_STATE : MIXED_STATE;
	else if (type_i >= NORMAL_UNCERTAIN &&
		 type_i <= DISCRETE_UNCERTAIN_SET_REAL)
	  view = (relaxed) ? RELAXED_UNCERTAIN : MIXED_UNCERTAIN;
      }
  }
  else if (view == EMPTY_VIEW)
    view = new_view;   
  else if (view != new_view) {
    // there are a few acceptable view promotions
    if ( ( view     == MIXED_ALEATORY_UNCERTAIN &&
	   new_view == MIXED_EPISTEMIC_UNCERTAIN ) ||
	 ( view     == MIXED_EPISTEMIC_UNCERTAIN &&
	   new_view == MIXED_ALEATORY_UNCERTAIN ) )
      view = MIXED_UNCERTAIN; // aggregate
    else if ( ( view     == RELAXED_ALEATORY_UNCERTAIN &&
		new_view == RELAXED_EPISTEMIC_UNCERTAIN ) ||
	      ( view     == RELAXED_EPISTEMIC_UNCERTAIN &&
		new_view == RELAXED_ALEATORY_UNCERTAIN ) )
      view = RELAXED_UNCERTAIN; // aggregate
    else if ( ( view == MIXED_UNCERTAIN &&
		( new_view == MIXED_ALEATORY_UNCERTAIN ||
		  new_view == MIXED_EPISTEMIC_UNCERTAIN ) ) ||
	      ( view == RELAXED_UNCERTAIN &&
		( new_view == RELAXED_ALEATORY_UNCERTAIN ||
		  new_view == RELAXED_EPISTEMIC_UNCERTAIN ) ) )
      { } // already aggregated
    else {
      Cerr << "\nError: inactive sub-model view discrepancy in NestedModel::"
	   << "update_inactive_view()." << std::endl;
      abort_handler(MODEL_ERROR);
    }
  }
}


void NestedModel::update_inactive_view(unsigned short type, short& view)
{
  // determine RELAXED or MIXED primary view at sub-model level
  short new_view, active_sm_view = subModel.current_variables().view().first;
  bool relaxed = ( active_sm_view == RELAXED_ALL ||
		   ( active_sm_view >= RELAXED_DESIGN &&
		     active_sm_view <= RELAXED_STATE ) );

  if (type >= CONTINUOUS_DESIGN && type <= DISCRETE_DESIGN_SET_REAL) {
    new_view = (relaxed) ? RELAXED_DESIGN : MIXED_DESIGN;
    update_inactive_view(new_view, view);
  }
  else if (type >= CONTINUOUS_STATE && type <= DISCRETE_STATE_SET_REAL) {
    new_view = (relaxed) ? RELAXED_STATE : MIXED_STATE;
    update_inactive_view(new_view, view);
  }
  else if (type >= NORMAL_UNCERTAIN && type <= DISCRETE_UNCERTAIN_SET_REAL) {
    if (type >= CONTINUOUS_INTERVAL_UNCERTAIN)
      new_view = (relaxed) ? RELAXED_EPISTEMIC_UNCERTAIN :
	                     MIXED_EPISTEMIC_UNCERTAIN;
    else
      new_view = (relaxed) ? RELAXED_ALEATORY_UNCERTAIN :
			     MIXED_ALEATORY_UNCERTAIN;
    update_inactive_view(new_view, view);
  }
}


void NestedModel::
update_sub_model(const Variables& vars, const Constraints& cons)
{
  // Update subModel variables using active currentVariables through a
  // combination of variable insertions and augmentations.  Insertions and
  // augmentations are not mutually exclusive, so subModel updates must
  // account for both, potentially on a variable by variable basis.

  // Top-level active variables/bounds/labels are mapped to the subModel, but
  // top-level linear/nonlinear constraint coefficients/bounds/targets are not
  // propagated (as they are in the surrogate model classes) since they are not
  // relevant at the subModel level.

  // For defined variable mappings, insert active currentVariables into subModel
  // variables (1). When secondary mappings are defined, this insertion involves
  // the updating of sub-parameters for the subModel variables (2). When only
  // primary mappings are defined, the insertions update the subModel variable
  // values. For null primary mappings (empty strings), augmentations are
  // performed by updating the subModel variables of the same variable type as
  // the active currentVariables (3). Bounds, labels, and sub-parameters are
  // additionally propagated for augmentations and active complement mappings
  // (both based on matching consistent types), with some differences in
  // required updating frequency.

  // Distribution parameter insertion is used for second-order probability
  // (UQ sample inserted into distrib parameters for UQ reliability/sampling)
  // and for OUU with design/uncertain overlap (optimization design variables
  // inserted into distr. parameters for UQ reliability/sampling).  For now,
  // existing distribution parameters are supported for the secondary variable
  // mappings, and a few variables additionally support LOCATION and SCALE
  // (this can be expanded in the future).

  size_t i, curr_i, num_var_map_2 = active2ACVarMapTargets.size(),
    num_curr_cv  = vars.cv(),  num_curr_div = vars.div(),
    num_curr_dsv = vars.dsv(), num_curr_drv = vars.drv(),
    pacvm_index, padivm_index, padsvm_index, padrvm_index;
  const SharedVariablesData& svd = vars.shared_data();
  const SharedVariablesData& sm_svd
    = subModel.current_variables().shared_data();
  std::shared_ptr<Pecos::MarginalsCorrDistribution> sm_mvd_rep =
    std::static_pointer_cast<Pecos::MarginalsCorrDistribution>
    (subModel.multivariate_distribution().multivar_dist_rep());

  // Map ACTIVE CONTINUOUS VARIABLES from currentVariables
  if (num_curr_cv) {
    const RealVector& curr_c_vars   = vars.continuous_variables();
    const RealVector& curr_c_l_bnds = cons.continuous_lower_bounds();
    const RealVector& curr_c_u_bnds = cons.continuous_upper_bounds();
    StringMultiArrayConstView curr_c_labels
      = vars.continuous_variable_labels();
    for (i=0; i<num_curr_cv; ++i) {
      curr_i = svd.cv_index_to_active_index(i);
      pacvm_index  =  active1ACVarMapIndices[curr_i];
      padivm_index = active1ADIVarMapIndices[curr_i];
      padsvm_index = active1ADSVarMapIndices[curr_i];
      padrvm_index = active1ADRVarMapIndices[curr_i];
      if (pacvm_index != _NPOS) {
	short sacvm_target
	  = (num_var_map_2) ? active2ACVarMapTargets[curr_i] : Pecos::NO_TARGET;
	if (sacvm_target == Pecos::NO_TARGET) {
	  subModel.all_continuous_variable(curr_c_vars[i], pacvm_index);
	  if (extraCVarsData[i]) { // default mapping between consistent types
	    subModel.all_continuous_lower_bound(curr_c_l_bnds[i], pacvm_index);
	    subModel.all_continuous_upper_bound(curr_c_u_bnds[i], pacvm_index);
	    // Note: this is more general than just bounds (all dist params):
	    sm_mvd_rep->pull_distribution_parameters(mvDist,
	      svd.cv_index_to_all_index(i),
	      sm_svd.acv_index_to_all_index(pacvm_index));
	    if (firstUpdate)
	      subModel.all_continuous_variable_label(curr_c_labels[i],
						     pacvm_index);
	  }
	}
	else
	  real_variable_mapping(curr_c_vars[i], pacvm_index, sacvm_target);
      }
      else if (padivm_index != _NPOS) {
	short sadivm_target = (num_var_map_2) ? active2ADIVarMapTargets[curr_i]
	  : Pecos::NO_TARGET;
	real_variable_mapping(curr_c_vars[i], padivm_index, sadivm_target);
      }
      else if (padsvm_index != _NPOS) {
	short sadsvm_target = (num_var_map_2) ? active2ADSVarMapTargets[curr_i]
	  : Pecos::NO_TARGET;
	real_variable_mapping(curr_c_vars[i], padsvm_index, sadsvm_target);
      }
      else if (padrvm_index != _NPOS) {
	short sadrvm_target = (num_var_map_2) ? active2ADRVarMapTargets[curr_i]
	  : Pecos::NO_TARGET;
	real_variable_mapping(curr_c_vars[i], padrvm_index, sadrvm_target);
      }
    }
  }

  // Map ACTIVE DISCRETE INTEGER VARIABLES from currentVariables
  if (num_curr_div) {
    const IntVector& curr_di_vars   = vars.discrete_int_variables();
    const IntVector& curr_di_l_bnds = cons.discrete_int_lower_bounds();
    const IntVector& curr_di_u_bnds = cons.discrete_int_upper_bounds();
    StringMultiArrayConstView curr_di_labels
      = vars.discrete_int_variable_labels();
    for (i=0; i<num_curr_div; ++i) {
      curr_i = svd.div_index_to_active_index(i);
      pacvm_index  =  active1ACVarMapIndices[curr_i];
      padivm_index = active1ADIVarMapIndices[curr_i];
      padsvm_index = active1ADSVarMapIndices[curr_i];
      padrvm_index = active1ADRVarMapIndices[curr_i];
      if (pacvm_index != _NPOS) {
	short sacvm_target
	  = (num_var_map_2) ? active2ACVarMapTargets[curr_i] : Pecos::NO_TARGET;
	integer_variable_mapping(curr_di_vars[i], pacvm_index, sacvm_target);
      }
      else if (padivm_index != _NPOS) {
	short sadivm_target = (num_var_map_2) ?
	  active2ADIVarMapTargets[curr_i] : Pecos::NO_TARGET;
	if (sadivm_target == Pecos::NO_TARGET) {
	  subModel.all_discrete_int_variable(curr_di_vars[i], padivm_index);
	  if (extraDIVarsData[i]) { // default mapping between consistent types
	    subModel.all_discrete_int_lower_bound(curr_di_l_bnds[i],
						  padivm_index);
	    subModel.all_discrete_int_upper_bound(curr_di_u_bnds[i],
						  padivm_index);
	    // Note: this is more general than just bounds (all dist params):
	    sm_mvd_rep->pull_distribution_parameters(mvDist,
	      svd.div_index_to_all_index(i),
	      sm_svd.adiv_index_to_all_index(padivm_index));
	    if (firstUpdate)
	      subModel.all_discrete_int_variable_label(curr_di_labels[i],
						       padivm_index);
	  }
	}
	else
	  integer_variable_mapping(curr_di_vars[i], padivm_index,sadivm_target);
      }
      else if (padsvm_index != _NPOS) {
	short sadsvm_target = (num_var_map_2) ?
	  active2ADSVarMapTargets[curr_i] : Pecos::NO_TARGET;
	integer_variable_mapping(curr_di_vars[i], padsvm_index, sadsvm_target);
      }
      else if (padrvm_index != _NPOS) {
	short sadrvm_target = (num_var_map_2) ?
	  active2ADRVarMapTargets[curr_i] : Pecos::NO_TARGET;
	integer_variable_mapping(curr_di_vars[i], padrvm_index, sadrvm_target);
      }
    }
  }

  // Map ACTIVE DISCRETE STRING VARIABLES from currentVariables
  if (num_curr_dsv) {
    StringMultiArrayConstView curr_ds_vars = vars.discrete_string_variables();
    StringMultiArrayConstView curr_ds_labels
      = vars.discrete_string_variable_labels();
    for (i=0; i<num_curr_dsv; ++i) {
      curr_i = svd.dsv_index_to_active_index(i);
      pacvm_index  =  active1ACVarMapIndices[curr_i];
      padivm_index = active1ADIVarMapIndices[curr_i];
      padsvm_index = active1ADSVarMapIndices[curr_i];
      padrvm_index = active1ADRVarMapIndices[curr_i];
      if (pacvm_index != _NPOS) {
	short sacvm_target = (num_var_map_2) ?
	  active2ACVarMapTargets[curr_i] : Pecos::NO_TARGET;
	string_variable_mapping(curr_ds_vars[i], pacvm_index, sacvm_target);
      }
      else if (padivm_index != _NPOS) {
	short sadivm_target = (num_var_map_2) ?
	  active2ADIVarMapTargets[curr_i] : Pecos::NO_TARGET;
	string_variable_mapping(curr_ds_vars[i], padivm_index, sadivm_target);
      }
      else if (padsvm_index != _NPOS) {
	short sadsvm_target = (num_var_map_2) ?
	  active2ADSVarMapTargets[curr_i] : Pecos::NO_TARGET;
	if (sadsvm_target == Pecos::NO_TARGET) {
	  subModel.all_discrete_string_variable(curr_ds_vars[i], padsvm_index);
	  if (extraDSVarsData[i]) { // default mapping between consistent types
	    // Note: this is more general than just bounds (all dist params):
	    sm_mvd_rep->pull_distribution_parameters(mvDist,
	      svd.dsv_index_to_all_index(i),
	      sm_svd.adsv_index_to_all_index(padsvm_index));
	    if (firstUpdate)
	      subModel.all_discrete_string_variable_label(curr_ds_labels[i],
							  padsvm_index);
	  }
	}
	else
	  string_variable_mapping(curr_ds_vars[i], padsvm_index, sadsvm_target);
      }
      else if (padrvm_index != _NPOS) {
	short sadrvm_target = (num_var_map_2) ?
	  active2ADRVarMapTargets[curr_i] : Pecos::NO_TARGET;
	string_variable_mapping(curr_ds_vars[i], padrvm_index, sadrvm_target);
      }
    }
  }

  // Map ACTIVE DISCRETE REAL VARIABLES from currentVariables
  if (num_curr_drv) {
    const RealVector& curr_dr_vars   = vars.discrete_real_variables();
    const RealVector& curr_dr_l_bnds = cons.discrete_real_lower_bounds();
    const RealVector& curr_dr_u_bnds = cons.discrete_real_upper_bounds();
    StringMultiArrayConstView curr_dr_labels
      = vars.discrete_real_variable_labels();
    for (i=0; i<num_curr_drv; ++i) {
      curr_i = svd.drv_index_to_active_index(i);
      pacvm_index  =  active1ACVarMapIndices[curr_i];
      padivm_index = active1ADIVarMapIndices[curr_i];
      padsvm_index = active1ADSVarMapIndices[curr_i];
      padrvm_index = active1ADRVarMapIndices[curr_i];
      if (pacvm_index != _NPOS) {
	short sacvm_target = (num_var_map_2) ?
	  active2ACVarMapTargets[curr_i] : Pecos::NO_TARGET;
	real_variable_mapping(curr_dr_vars[i], pacvm_index, sacvm_target);
      }
      else if (padivm_index != _NPOS) {
	short sadivm_target = (num_var_map_2) ?
	  active2ADIVarMapTargets[curr_i] : Pecos::NO_TARGET;
	real_variable_mapping(curr_dr_vars[i], padivm_index, sadivm_target);
      }
      else if (padsvm_index != _NPOS) {
	short sadsvm_target = (num_var_map_2) ?
	  active2ADSVarMapTargets[curr_i] : Pecos::NO_TARGET;
	real_variable_mapping(curr_dr_vars[i], padsvm_index, sadsvm_target);
      }
      else if (padrvm_index != _NPOS) {
	short sadrvm_target = (num_var_map_2) ?
	  active2ADRVarMapTargets[curr_i] : Pecos::NO_TARGET;
	if (sadrvm_target == Pecos::NO_TARGET) {
	  subModel.all_discrete_real_variable(curr_dr_vars[i], padrvm_index);
	  if (extraDRVarsData[i]) { // default mapping between consistent types
	    subModel.all_discrete_real_lower_bound(curr_dr_l_bnds[i],
						   padrvm_index);
	    subModel.all_discrete_real_upper_bound(curr_dr_u_bnds[i],
						   padrvm_index);
	    // Note: this is more general than just bounds (all dist params):
	    sm_mvd_rep->pull_distribution_parameters(mvDist,
	      svd.drv_index_to_all_index(i),
	      sm_svd.adrv_index_to_all_index(padrvm_index));
	    if (firstUpdate)
	      subModel.all_discrete_real_variable_label(curr_dr_labels[i],
							padrvm_index);
	  }
	}
	else
	  real_variable_mapping(curr_dr_vars[i], padrvm_index, sadrvm_target);
      }
    }
  }

  // Map COMPLEMENT CONTINUOUS VARIABLES from currentVariables into
  // corresponding subModel type (using same logic as default active mapping)
  size_t num_curr_ccv = vars.acv() - num_curr_cv, c1_index;
  if (num_curr_ccv) {
    const RealVector& curr_ac_vars   = vars.all_continuous_variables();
    const RealVector& curr_ac_l_bnds = cons.all_continuous_lower_bounds();
    const RealVector& curr_ac_u_bnds = cons.all_continuous_upper_bounds();
    StringMultiArrayConstView curr_ac_labels
      = vars.all_continuous_variable_labels();
    for (i=0; i<num_curr_ccv; ++i) {
      curr_i = svd.ccv_index_to_acv_index(i);
      c1_index = complement1ACVarMapIndices[i];
      subModel.all_continuous_variable(curr_ac_vars[curr_i], c1_index);
      subModel.all_continuous_lower_bound(curr_ac_l_bnds[curr_i], c1_index);
      subModel.all_continuous_upper_bound(curr_ac_u_bnds[curr_i], c1_index);
      if (firstUpdate) {
	subModel.all_continuous_variable_label(curr_ac_labels[curr_i],c1_index);
	// Note: this is more general than just bounds (all dist params):
	sm_mvd_rep->pull_distribution_parameters(mvDist,
	  svd.ccv_index_to_all_index(i),
	  sm_svd.acv_index_to_all_index(c1_index));
      }
    }
  }

  // Map COMPLEMENT DISCRETE INTEGER VARIABLES from currentVariables into
  // corresponding subModel type (using same logic as default active mapping)
  size_t num_curr_cdiv = vars.adiv() - num_curr_div;
  if (num_curr_cdiv) {
    const IntVector& curr_adi_vars   = vars.all_discrete_int_variables();
    const IntVector& curr_adi_l_bnds = cons.all_discrete_int_lower_bounds();
    const IntVector& curr_adi_u_bnds = cons.all_discrete_int_upper_bounds();
    StringMultiArrayConstView curr_adi_labels
      = vars.all_discrete_int_variable_labels();
    for (i=0; i<num_curr_cdiv; ++i) {
      curr_i = svd.cdiv_index_to_adiv_index(i);
      c1_index = complement1ADIVarMapIndices[i];
      subModel.all_discrete_int_variable(curr_adi_vars[curr_i], c1_index);
      subModel.all_discrete_int_lower_bound(curr_adi_l_bnds[curr_i], c1_index);
      subModel.all_discrete_int_upper_bound(curr_adi_u_bnds[curr_i], c1_index);
      if (firstUpdate) {
	subModel.all_discrete_int_variable_label(curr_adi_labels[curr_i],
						 c1_index);
	// Note: this is more general than just bounds (all dist params):
	sm_mvd_rep->pull_distribution_parameters(mvDist,
	  svd.cdiv_index_to_all_index(i),
	  sm_svd.adiv_index_to_all_index(c1_index));
      }
    }
  }

  // Map COMPLEMENT DISCRETE STRING VARIABLES from currentVariables into
  // corresponding subModel type (using same logic as default active mapping)
  size_t num_curr_cdsv = vars.adsv() - num_curr_dsv;
  if (num_curr_cdsv) {
    StringMultiArrayConstView curr_ads_vars
      = vars.all_discrete_string_variables();
    StringMultiArrayConstView curr_ads_labels
      = vars.all_discrete_string_variable_labels();
    for (i=0; i<num_curr_cdsv; ++i) {
      curr_i = svd.cdsv_index_to_adsv_index(i);
      c1_index = complement1ADSVarMapIndices[i];
      subModel.all_discrete_string_variable(curr_ads_vars[curr_i], c1_index);
      if (firstUpdate) {
	subModel.all_discrete_string_variable_label(curr_ads_labels[curr_i],
						    c1_index);
	// Note: this is more general than just bounds (all dist params):
	sm_mvd_rep->pull_distribution_parameters(mvDist,
	  svd.cdsv_index_to_all_index(i),
	  sm_svd.adsv_index_to_all_index(c1_index));
      }
    }
  }

  // Map COMPLEMENT DISCRETE REAL VARIABLES from currentVariables into
  // corresponding subModel type (using same logic as default active mapping)
  size_t num_curr_cdrv = vars.adrv() - num_curr_drv;
  if (num_curr_cdrv) {
    const RealVector& curr_adr_vars   = vars.all_discrete_real_variables();
    const RealVector& curr_adr_l_bnds = cons.all_discrete_real_lower_bounds();
    const RealVector& curr_adr_u_bnds = cons.all_discrete_real_upper_bounds();
    StringMultiArrayConstView curr_adr_labels
      = vars.all_discrete_real_variable_labels();
    for (i=0; i<num_curr_cdrv; ++i) {
      curr_i = svd.cdrv_index_to_adrv_index(i);
      c1_index = complement1ADRVarMapIndices[i];
      subModel.all_discrete_real_variable(curr_adr_vars[curr_i], c1_index);
      subModel.all_discrete_real_lower_bound(curr_adr_l_bnds[curr_i], c1_index);
      subModel.all_discrete_real_upper_bound(curr_adr_u_bnds[curr_i], c1_index);
      if (firstUpdate) {
	subModel.all_discrete_real_variable_label(curr_adr_labels[curr_i],
						  c1_index);
	// Note: this is more general than just bounds (all dist params):
	sm_mvd_rep->pull_distribution_parameters(mvDist,
	  svd.cdrv_index_to_all_index(i),
	  sm_svd.adrv_index_to_all_index(c1_index));
      }
    }
  }

  firstUpdate = false;
}


void NestedModel::
real_variable_mapping(Real r_var, size_t av_index, short svm_target)
{
  Pecos::MultivariateDistribution& sm_mvd
    = subModel.multivariate_distribution();
  std::shared_ptr<Pecos::MarginalsCorrDistribution> sm_mvd_rep =
    std::static_pointer_cast<Pecos::MarginalsCorrDistribution>
    (sm_mvd.multivar_dist_rep());

  const SharedVariablesData& sm_svd
    = subModel.current_variables().shared_data();

  switch (svm_target) {
  case Pecos::CR_LWR_BND:  case Pecos::N_LWR_BND:  case Pecos::LN_LWR_BND:
  case Pecos::U_LWR_BND:   case Pecos::LU_LWR_BND: case Pecos::T_LWR_BND:
  case Pecos::BE_LWR_BND:
    sm_mvd_rep->push_parameter(sm_svd.acv_index_to_all_index(av_index),
			       svm_target, r_var);
    subModel.all_continuous_lower_bound(r_var, av_index);
    break;
  case Pecos::CR_UPR_BND:  case Pecos::N_UPR_BND:  case Pecos::LN_UPR_BND:
  case Pecos::U_UPR_BND:   case Pecos::LU_UPR_BND: case Pecos::T_UPR_BND:
  case Pecos::BE_UPR_BND:
    sm_mvd_rep->push_parameter(sm_svd.acv_index_to_all_index(av_index),
			       svm_target, r_var);
    subModel.all_continuous_upper_bound(r_var, av_index);
    break;
  case Pecos::N_MEAN:      case Pecos::N_STD_DEV:  case Pecos::LN_MEAN:
  case Pecos::LN_STD_DEV:  case Pecos::LN_LAMBDA:  case Pecos::LN_ZETA:
  case Pecos::LN_ERR_FACT: case Pecos::T_MODE:     case Pecos::E_BETA:
  case Pecos::BE_ALPHA:    case Pecos::BE_BETA:    case Pecos::GA_ALPHA:
  case Pecos::GA_BETA:     case Pecos::GU_ALPHA:   case Pecos::GU_BETA:
  case Pecos::F_ALPHA:     case Pecos::F_BETA:     case Pecos::W_ALPHA:
  case Pecos::W_BETA:
    sm_mvd_rep->push_parameter(sm_svd.acv_index_to_all_index(av_index),
			       svm_target, r_var);
    break;
  case Pecos::P_LAMBDA:        case Pecos::BI_P_PER_TRIAL:
  case Pecos::NBI_P_PER_TRIAL: case Pecos::GE_P_PER_TRIAL:
    sm_mvd_rep->push_parameter(sm_svd.adiv_index_to_all_index(av_index),
			       svm_target, r_var);
    break;
  // N_{MEAN,STD_DEV,LWR_BND,UPR_BND} change individual dist parameters only.
  // N_{LOCATION,SCALE} change multiple parameters to accomplish a translation
  // or scaling.  They are mapped to a convenient definition believed to be the
  // most natural for a user, where the definition changes from distribution to
  // distribution (a consistent meaning of mu/sigma would be more awkward for a
  // user to convert).  For Normal, location & scale are mean & std deviation.
  case Pecos::N_LOCATION: { // a translation with no change in shape/scale
    size_t rv_index = sm_svd.acv_index_to_all_index(av_index);
    Real mean, l_bnd, u_bnd;
    sm_mvd_rep->pull_parameter<Real>(rv_index, Pecos::N_MEAN, mean);
    sm_mvd_rep->pull_parameter<Real>(rv_index, Pecos::N_LWR_BND, l_bnd);
    sm_mvd_rep->pull_parameter<Real>(rv_index, Pecos::N_UPR_BND, u_bnd);
    Real delta = r_var - mean;
    // translate: change bounds by same amount as mean
    sm_mvd_rep->push_parameter(rv_index, Pecos::N_MEAN, r_var);
    Real dbl_inf = std::numeric_limits<Real>::infinity();
    if (l_bnd > -dbl_inf) {
      Real new_l_bnd = l_bnd + delta;
      sm_mvd_rep->push_parameter(rv_index, Pecos::N_LWR_BND, new_l_bnd);
      subModel.all_continuous_lower_bound(new_l_bnd, av_index);
    }
    if (u_bnd <  dbl_inf) {
      Real new_u_bnd = u_bnd + delta;
      sm_mvd_rep->push_parameter(rv_index, Pecos::N_UPR_BND, new_u_bnd);
      subModel.all_continuous_upper_bound(new_u_bnd, av_index);   
    }
    break;
  }
  case Pecos::N_SCALE: { // change in shape/scale without translation
    size_t rv_index = sm_svd.acv_index_to_all_index(av_index);
    Real mean, stdev, l_bnd, u_bnd;
    sm_mvd_rep->pull_parameter<Real>(rv_index, Pecos::N_MEAN, mean);
    sm_mvd_rep->pull_parameter<Real>(rv_index, Pecos::N_STD_DEV, stdev);
    sm_mvd_rep->pull_parameter<Real>(rv_index, Pecos::N_LWR_BND, l_bnd);
    sm_mvd_rep->pull_parameter<Real>(rv_index, Pecos::N_UPR_BND, u_bnd);
    // scale: preserve number of std deviations where l,u bound occurs
    sm_mvd_rep->push_parameter(rv_index, Pecos::N_STD_DEV, r_var);
    Real dbl_inf = std::numeric_limits<Real>::infinity();
    if (l_bnd > -dbl_inf) {
      Real num_sig_l = (mean - l_bnd) / stdev,
	   new_l_bnd = mean - num_sig_l * r_var;
      sm_mvd_rep->push_parameter(rv_index, Pecos::N_LWR_BND, new_l_bnd);
      subModel.all_continuous_lower_bound(new_l_bnd, av_index);
    }
    if (u_bnd <  dbl_inf) {
      Real num_sig_u = (u_bnd - mean) / stdev,
	   new_u_bnd = mean + num_sig_u * r_var;
      sm_mvd_rep->push_parameter(rv_index, Pecos::N_UPR_BND, new_u_bnd);
      subModel.all_continuous_upper_bound(new_u_bnd, av_index);   
    }
    break;
  }
  // U_{LWR_BND,UPR_BND} change individual dist parameters only.
  // U_{LOCATION,SCALE} change multiple parameters to accomplish a translation
  // or scaling.  They are mapped to a convenient definition believed to be the
  // most natural for a user, where the definition changes from distribution to
  // distribution (a consistent meaning of mu/sigma would be more awkward for a
  // user to convert).  For Uniform, location & scale are center & range.
  case Pecos::U_LOCATION: {
    size_t rv_index = sm_svd.acv_index_to_all_index(av_index);
    // translate: change both bounds by same amount
    Real l_bnd, u_bnd;
    sm_mvd_rep->pull_parameter<Real>(rv_index, Pecos::U_LWR_BND, l_bnd);
    sm_mvd_rep->pull_parameter<Real>(rv_index, Pecos::U_UPR_BND, u_bnd);
    Real center = (u_bnd + l_bnd) / 2., delta = r_var - center,
      new_l_bnd = l_bnd + delta, new_u_bnd = u_bnd + delta;
    sm_mvd_rep->push_parameter(rv_index, Pecos::U_LWR_BND, new_l_bnd);
    sm_mvd_rep->push_parameter(rv_index, Pecos::U_UPR_BND, new_u_bnd);
    subModel.all_continuous_lower_bound(new_l_bnd, av_index);
    subModel.all_continuous_upper_bound(new_u_bnd, av_index);   
    break;
  }
  case Pecos::U_SCALE: {
    size_t rv_index = sm_svd.acv_index_to_all_index(av_index);
    // scale: move bounds in/out by same amount about consistent center
    Real l_bnd, u_bnd;
    sm_mvd_rep->pull_parameter<Real>(rv_index, Pecos::U_LWR_BND, l_bnd);
    sm_mvd_rep->pull_parameter<Real>(rv_index, Pecos::U_UPR_BND, u_bnd);
    Real center = (u_bnd + l_bnd) / 2., half_range = r_var / 2.,
      new_l_bnd = center-half_range, new_u_bnd = center+half_range;
    sm_mvd_rep->push_parameter(rv_index, Pecos::U_LWR_BND, new_l_bnd);
    sm_mvd_rep->push_parameter(rv_index, Pecos::U_UPR_BND, new_u_bnd);
    subModel.all_continuous_lower_bound(new_l_bnd, av_index);
    subModel.all_continuous_upper_bound(new_u_bnd, av_index);   
    break;
  }
  // T_{MODE,LWR_BND,UPR_BND} change individual dist parameters only.
  // T_{LOCATION,SCALE} change multiple parameters to accomplish a translation
  // or scaling.  They are mapped to a convenient definition believed to be the
  // most natural for a user, where the definition changes from distribution to
  // distribution (a consistent meaning of mu/sigma would be more awkward for a
  // user to convert).  For Triangular, location & scale are mode & range.
  case Pecos::T_LOCATION: {
    size_t rv_index = sm_svd.acv_index_to_all_index(av_index);
    // translate: change mode and both bounds by same amount
    Real mode, l_bnd, u_bnd;
    sm_mvd_rep->pull_parameter<Real>(rv_index, Pecos::T_MODE, mode);
    sm_mvd_rep->pull_parameter<Real>(rv_index, Pecos::T_LWR_BND, l_bnd);
    sm_mvd_rep->pull_parameter<Real>(rv_index, Pecos::T_UPR_BND, u_bnd);
    Real  delta = r_var - mode, new_l_bnd = l_bnd + delta,
      new_u_bnd = u_bnd + delta;
    sm_mvd_rep->push_parameter(rv_index, Pecos::T_MODE,    r_var);
    sm_mvd_rep->push_parameter(rv_index, Pecos::T_LWR_BND, new_l_bnd);
    sm_mvd_rep->push_parameter(rv_index, Pecos::T_UPR_BND, new_u_bnd);
    subModel.all_continuous_lower_bound(new_l_bnd, av_index);
    subModel.all_continuous_upper_bound(new_u_bnd, av_index);   
    break;
  }
  case Pecos::T_SCALE: {
    size_t rv_index = sm_svd.acv_index_to_all_index(av_index);
    // scale: preserve L/M/U proportions while scaling range
    Real mode, l_bnd, u_bnd;
    sm_mvd_rep->pull_parameter<Real>(rv_index, Pecos::T_MODE, mode);
    sm_mvd_rep->pull_parameter<Real>(rv_index, Pecos::T_LWR_BND, l_bnd);
    sm_mvd_rep->pull_parameter<Real>(rv_index, Pecos::T_UPR_BND, u_bnd);
    Real  range = u_bnd - l_bnd, perc_l = (mode - l_bnd) / range,
         perc_u = (u_bnd - mode) / range, new_l_bnd = mode - perc_l * r_var,
      new_u_bnd = mode + perc_u * r_var;
    sm_mvd_rep->push_parameter(rv_index, Pecos::T_LWR_BND, new_l_bnd);
    sm_mvd_rep->push_parameter(rv_index, Pecos::T_UPR_BND, new_u_bnd);
    subModel.all_continuous_lower_bound(new_l_bnd, av_index);
    subModel.all_continuous_upper_bound(new_u_bnd, av_index);   
    break;
  }
  case Pecos::NO_TARGET: default:
    Cerr << "\nError: secondary mapping target unmatched for real value "
	 << "insertion in NestedModel::real_variable_mapping()." << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void NestedModel::
integer_variable_mapping(int i_var, size_t av_index, short svm_target)
{
  Pecos::MultivariateDistribution& sm_mvd
    = subModel.multivariate_distribution();
  std::shared_ptr<Pecos::MarginalsCorrDistribution> sm_mvd_rep =
    std::static_pointer_cast<Pecos::MarginalsCorrDistribution>
    (sm_mvd.multivar_dist_rep());

  const SharedVariablesData& sm_svd
    = subModel.current_variables().shared_data();

  switch (svm_target) {
  case Pecos::DR_LWR_BND:
    sm_mvd_rep->push_parameter(sm_svd.adiv_index_to_all_index(av_index),
			       svm_target, i_var);
    subModel.all_discrete_int_lower_bound(i_var, av_index);
    break;
  case Pecos::DR_UPR_BND:
    sm_mvd_rep->push_parameter(sm_svd.adiv_index_to_all_index(av_index),
			       svm_target, i_var);
    subModel.all_discrete_int_upper_bound(i_var, av_index);
    break;
  case Pecos::BI_TRIALS:    case Pecos::NBI_TRIALS:
  case Pecos::HGE_TOT_POP:  case Pecos::HGE_SEL_POP:  case Pecos::HGE_DRAWN: {
    unsigned int ui_var = (unsigned int)i_var;
    sm_mvd_rep->push_parameter(sm_svd.adiv_index_to_all_index(av_index),
			       svm_target, ui_var);
    break;
  }
  case Pecos::NO_TARGET: default:
    Cerr << "\nError: secondary mapping target unmatched for integer value "
	 << "insertion in NestedModel::integer_variable_mapping()" << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void NestedModel::
string_variable_mapping(const String& s_var, size_t av_index,
			short svm_target)
{
  Pecos::MultivariateDistribution& sm_mvd
    = subModel.multivariate_distribution();
  std::shared_ptr<Pecos::MarginalsCorrDistribution> sm_mvd_rep =
    std::static_pointer_cast<Pecos::MarginalsCorrDistribution>
    (sm_mvd.multivar_dist_rep());

  const SharedVariablesData& sm_svd
    = subModel.current_variables().shared_data();

  switch (svm_target) {
  case Pecos::NO_TARGET: default:
    Cerr << "\nError: secondary mapping target unmatched for string value "
	 << "insertion in NestedModel::string_variable_mapping()" << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


ActiveSet NestedModel::default_interface_active_set()
{
  size_t num_fun = numOptInterfPrimary + numOptInterfIneqCon + numOptInterfEqCon;
  ActiveSet set;
  set.derivative_vector(currentVariables.all_continuous_variable_ids());
  bool has_deriv_vars = set.derivative_vector().size() != 0;
  ShortArray asv(num_fun, 1);
  if(has_deriv_vars) {
    if(optInterfGradientType == "analytic") {
      for(auto &a : asv)
        a |=  2;
    } else if(optInterfGradientType == "mixed") {
      for(const auto &gi : optInterfGradIdAnalytic)
        asv[gi-1] |= 2;
    }

    if(optInterfHessianType == "analytic") {
      for(auto &a : asv)
        a |=  4;
    } else if(optInterfHessianType == "mixed") {
      for(const auto &hi : optInterfHessIdAnalytic)
        asv[hi-1] |= 4;
    }
  }
  set.request_vector(asv);
  return set;
}

} // namespace Dakota
