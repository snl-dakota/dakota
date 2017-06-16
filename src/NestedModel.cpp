/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NestedModel
//- Description: Implementation code for the NestedModel class
//- Owner:       Mike Eldred
//- Checked by:

#include "NestedModel.hpp"
#include "ProblemDescDB.hpp"
#include "dakota_system_defs.hpp"
#include "pecos_global_defs.hpp"
#include "DistributionParams.hpp"

static const char rcsId[]="@(#) $Id: NestedModel.cpp 7024 2010-10-16 01:24:42Z mseldre $";

//#define DEBUG

// define special values for componentParallelMode
#define OPTIONAL_INTERFACE 1
#define SUB_MODEL          2


namespace Dakota {

NestedModel::NestedModel(ProblemDescDB& problem_db):
  Model(BaseConstructor(), problem_db),
  nestedModelEvalCntr(0), firstUpdate(true), outerMIPLIndex(0),
  subIteratorSched(parallelLib,
		   true, // peer 1 must assign jobs to peers 2-n
		   problem_db.get_int("model.nested.iterator_servers"),
		   problem_db.get_int("model.nested.processors_per_iterator"),
		   problem_db.get_short("model.nested.iterator_scheduling")),
  subIteratorJobCntr(0),
  optInterfacePointer(problem_db.get_string("model.interface_pointer")),
  subMethodPointer(problem_db.get_string("model.nested.sub_method_pointer"))
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
  if ( ( num_var_map_1 && num_var_map_1 != num_curr_vars ) ||
       ( num_var_map_2 && num_var_map_2 != num_var_map_1 ) ) {
    Cerr << "\nError: length of variable mapping specification(s) does not "
	 << "match number of active variables." << std::endl;
    abort_handler(-1);
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

  // Map ACTIVE CONTINUOUS VARIABLES from currentVariables
  for (i=0; i<num_curr_cv; ++i) {
    curr_i = cv_index_map(i, currentVariables);
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
	abort_handler(-1);
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
	abort_handler(-1);
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
    curr_i = div_index_map(i, currentVariables);
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
	abort_handler(-1);
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
	abort_handler(-1);
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
    curr_i = dsv_index_map(i, currentVariables);
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
	abort_handler(-1);
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
	abort_handler(-1);
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
    curr_i = drv_index_map(i, currentVariables);
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
	abort_handler(-1);
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
	abort_handler(-1);
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
    curr_i = ccv_index_map(i, currentVariables);
    // Can't use label matching, since subModel labels may not be updated
    // until runtime.  index() returns the _first_ instance of the type.
    unsigned short curr_ac_type = curr_ac_types[curr_i];
    size_t sm_acv_offset = find_index(submodel_a_c_types, curr_ac_type);
    if (sm_acv_offset == _NPOS) {
      Cerr << "\nError: complement continuous variable type '" << curr_ac_type
	   << "' could not be matched within all sub-model continuous "
	   << "variable types." << std::endl;
      abort_handler(-1);
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
      abort_handler(-1);
    }
  }

  // Map COMPLEMENT DISCRETE INTEGER VARIABLES from currentVariables
  for (i=0; i<num_curr_cdiv; ++i) {
    curr_i = cdiv_index_map(i, currentVariables);
    // Can't use label matching, since subModel labels may not be updated
    // until runtime.  index() returns the _first_ instance of the type.
    unsigned short curr_adi_type = curr_adi_types[curr_i];
    size_t sm_adiv_offset = find_index(submodel_a_di_types, curr_adi_type);
    if (sm_adiv_offset == _NPOS) {
      Cerr << "\nError: complement discrete integer variable type '"
	   << curr_adi_type << "' could not be matched within all sub-model "
	   << "discrete integer variable types." << std::endl;
      abort_handler(-1);
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
      abort_handler(-1);
    }
  }

  // Map COMPLEMENT DISCRETE STRING VARIABLES from currentVariables
  for (i=0; i<num_curr_cdsv; ++i) {
    curr_i = cdsv_index_map(i, currentVariables);
    // Can't use label matching, since subModel labels may not be updated
    // until runtime.  index() returns the _first_ instance of the type.
    unsigned short curr_ads_type = curr_ads_types[curr_i];
    size_t sm_adsv_offset = find_index(submodel_a_ds_types, curr_ads_type);
    if (sm_adsv_offset == _NPOS) {
      Cerr << "\nError: complement discrete string variable type '"
	   << curr_ads_type << "' could not be matched within all sub-model "
	   << "discrete string variable types." << std::endl;
      abort_handler(-1);
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
      abort_handler(-1);
    }
  }

  // Map COMPLEMENT DISCRETE REAL VARIABLES from currentVariables
  for (i=0; i<num_curr_cdrv; ++i) {
    curr_i = cdrv_index_map(i, currentVariables);
    // Can't use label matching, since subModel labels may not be updated
    // until runtime.  index() returns the _first_ instance of the type.
    unsigned short curr_adr_type = curr_adr_types[curr_i];
    size_t sm_adrv_offset = find_index(submodel_a_dr_types, curr_adr_type);
    if (sm_adrv_offset == _NPOS) {
      Cerr << "\nError: complement discrete real variable type '"
	   << curr_adr_type << "' could not be matched within all sub-model "
	   << "discrete real variable types." << std::endl;
      abort_handler(-1);
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
      abort_handler(-1);
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
    update_sub_iterator(); // follow DB restore: extracts data from nested spec
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


void NestedModel::update_sub_iterator()
{
  subIterator.sub_iterator_flag(true);
  subIterator.active_variable_mappings(active1ACVarMapIndices,
    active1ADIVarMapIndices, active1ADSVarMapIndices, active1ADRVarMapIndices,
    active2ACVarMapTargets,  active2ADIVarMapTargets, active2ADSVarMapTargets,
    active2ADRVarMapTargets);

  const RealVector& primary_resp_coeffs
    = probDescDB.get_rv("model.nested.primary_response_mapping");
  const RealVector& secondary_resp_coeffs
    = probDescDB.get_rv("model.nested.secondary_response_mapping");

  if (primary_resp_coeffs.empty() && secondary_resp_coeffs.empty()) {
    Cerr << "\nError: no mappings provided for sub-iterator functions in "
	 << "NestedModel initialization." << std::endl;
    abort_handler(-1);
  }
  // Convert vectors to matrices using the number of subIterator response
  // results (e.g., the number of UQ statistics) as the number of columns
  // (rows are open ended since the number of subModel primary fns may
  // be different from the total number of primary fns and subModel's
  // constraints are a subset of the total constraints).
  numSubIterFns = subIterator.response_results().num_functions();
  if (!primary_resp_coeffs.empty()) {
    // this error would also be caught within copy_data(), but by checking here,
    // we can provide a more helpful error message.
    if (primary_resp_coeffs.length() % numSubIterFns) {
      Cerr << "\nError: number of entries in primary_response_mapping ("
	   << primary_resp_coeffs.length() << ") not evenly divisible"
	   << "\n       by number of sub-iterator response functions ("
	   << numSubIterFns << ") in NestedModel initialization." << std::endl;
      abort_handler(-1);
    }
    copy_data(primary_resp_coeffs, primaryRespCoeffs, 0, (int)numSubIterFns);
  }
  if (!secondary_resp_coeffs.empty()) {
    // this error would also be caught within copy_data(), but by checking here,
    // we can provide a more helpful error message.
    if (secondary_resp_coeffs.length() % numSubIterFns) {
      Cerr << "\nError: number of entries in secondary_response_mapping ("
	   << secondary_resp_coeffs.length() << ") not evenly divisible"
	   << "\n       by number of sub-iterator response functions ("
	   << numSubIterFns << ") in NestedModel initialization." << std::endl;
      abort_handler(-1);
    }
    copy_data(secondary_resp_coeffs, secondaryRespCoeffs, 0,(int)numSubIterFns);
  }

  // Back out the number of eq/ineq constraints within secondaryRespCoeffs
  // (subIterator constraints) from the total number of equality/inequality
  // constraints and the number of interface equality/inequality constraints.
  size_t num_mapped_ineq_con
    = probDescDB.get_sizet("responses.num_nonlinear_inequality_constraints"),
    num_mapped_eq_con
    = probDescDB.get_sizet("responses.num_nonlinear_equality_constraints");
  numSubIterMappedIneqCon = num_mapped_ineq_con - numOptInterfIneqCon;
  numSubIterMappedEqCon   = num_mapped_eq_con   - numOptInterfEqCon;
}


void NestedModel::
resolve_real_variable_mapping(const String& map1, const String& map2,
			      size_t curr_index, short& inactive_sm_view)
{
  UShortMultiArrayConstView submodel_a_c_types
    = subModel.all_continuous_variable_types();
  UShortMultiArrayConstView submodel_a_di_types
    = subModel.all_discrete_int_variable_types();
  UShortMultiArrayConstView submodel_a_ds_types
    = subModel.all_discrete_string_variable_types();
  UShortMultiArrayConstView submodel_a_dr_types
    = subModel.all_discrete_real_variable_types();
  Pecos::AleatoryDistParams& submodel_adp
    = subModel.aleatory_distribution_parameters();

  size_t ac_index1 = active1ACVarMapIndices[curr_index]
      = find_index(subModel.all_continuous_variable_labels(), map1),
    adi_index1 = active1ADIVarMapIndices[curr_index]
      = find_index(subModel.all_discrete_int_variable_labels(), map1),
    ads_index1 = active1ADSVarMapIndices[curr_index]
      = find_index(subModel.all_discrete_string_variable_labels(), map1),
    adr_index1 = active1ADRVarMapIndices[curr_index]
      = find_index(subModel.all_discrete_real_variable_labels(), map1);
  if (ac_index1  == _NPOS && adi_index1 == _NPOS &&
      ads_index1 == _NPOS && adr_index1 == _NPOS) {
    Cerr << "\nError: primary mapping " << map1 << " could not be matched "
	 << "within any sub-model variable labels." << std::endl;
    abort_handler(-1);
  }

  if (ac_index1 != _NPOS &&
      find_index(subModel.continuous_variable_labels(), map1) == _NPOS)
    // inactive CV target
    update_inactive_view(submodel_a_c_types[ac_index1], inactive_sm_view);
  else if (adi_index1 != _NPOS &&
	   find_index(subModel.discrete_int_variable_labels(), map1) == _NPOS)
    // inactive DIV target
    update_inactive_view(submodel_a_di_types[adi_index1], inactive_sm_view);
  else if (ads_index1 != _NPOS &&
	   find_index(subModel.discrete_string_variable_labels(),map1) == _NPOS)
    // inactive DSV target
    update_inactive_view(submodel_a_ds_types[ads_index1], inactive_sm_view);
  else if (adr_index1 != _NPOS &&
	   find_index(subModel.discrete_real_variable_labels(), map1) == _NPOS)
    // inactive DRV target
    update_inactive_view(submodel_a_dr_types[adr_index1], inactive_sm_view);

  if (!active2ACVarMapTargets.empty()) { // indicates num_var_map_2
    if (map2.empty())
      active2ACVarMapTargets[curr_index] = active2ADIVarMapTargets[curr_index]
	= active2ADSVarMapTargets[curr_index]
	= active2ADRVarMapTargets[curr_index] = Pecos::NO_TARGET;
    else if (ac_index1 != _NPOS) {
      unsigned short type = submodel_a_c_types[ac_index1];
      if (type == CONTINUOUS_DESIGN) {
	if (map2 == "lower_bound")
	  active2ACVarMapTargets[curr_index] = Pecos::CDV_LWR_BND;
	else if (map2 == "upper_bound")
	  active2ACVarMapTargets[curr_index] = Pecos::CDV_UPR_BND;
	else {
	  Cerr << "\nError: " << map2 << " mapping not supported for "
	       << "continuous design variables." << std::endl;
	  abort_handler(-1);
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
	  abort_handler(-1);
	}
      }
      else if (type == LOGNORMAL_UNCERTAIN) {
	// TO DO: 3 parameter lognormal adds a location parameter
	if (map2 == "mean") {
	  if (submodel_adp.lognormal_means().empty()) {
	    Cerr << "\nError: cannot insert mean without lognormal means "
		 << "specification." << std::endl;
	    abort_handler(-1);
	  }
	  else
	    active2ACVarMapTargets[curr_index] = Pecos::LN_MEAN;
	}
	else if (map2 == "std_deviation") {
	  if (submodel_adp.lognormal_std_deviations().empty()) {
	    Cerr << "\nError: cannot insert std_deviation without lognormal "
		 << "std_deviations specification." << std::endl;
	    abort_handler(-1);
	  }
	  else
	    active2ACVarMapTargets[curr_index] = Pecos::LN_STD_DEV;
	}
	else if (map2 == "lambda") {
	  if (submodel_adp.lognormal_lambdas().empty()) {
	    Cerr << "\nError: cannot insert lambda without lognormal lambdas "
		 << "specification." << std::endl;
	    abort_handler(-1);
	  }
	  else
	    active2ACVarMapTargets[curr_index] = Pecos::LN_LAMBDA;
	}
	else if (map2 == "zeta") {
	  if (submodel_adp.lognormal_zetas().empty()) {
	    Cerr << "\nError: cannot insert zeta without lognormal zetas "
		 << "specification." << std::endl;
	    abort_handler(-1);
	  }
	  else
	    active2ACVarMapTargets[curr_index] = Pecos::LN_ZETA;
	}
	else if (map2 == "error_factor") {
	  if (submodel_adp.lognormal_error_factors().empty()) {
	    Cerr << "\nError: cannot insert error_factor without lognormal "
		 << "error_factors specification." << std::endl;
	    abort_handler(-1);
	  }
	  else
	    active2ACVarMapTargets[curr_index] = Pecos::LN_ERR_FACT;
	}
	else if (map2 == "lower_bound")
	  active2ACVarMapTargets[curr_index] = Pecos::LN_LWR_BND;
	else if (map2 == "upper_bound")
	  active2ACVarMapTargets[curr_index] = Pecos::LN_UPR_BND;
	else {
	  Cerr << "\nError: " << map2 << " mapping not supported for "
	       << "lognormal distributions." << std::endl;
	  abort_handler(-1);
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
	  abort_handler(-1);
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
	  abort_handler(-1);
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
	  abort_handler(-1);
	}
      }
      else if (type == EXPONENTIAL_UNCERTAIN) {
	// TO DO: mean/std deviation or location/scale
	if (map2 == "beta")
	  active2ACVarMapTargets[curr_index] = Pecos::E_BETA;
	else {
	  Cerr << "\nError: " << map2 << " mapping not supported for "
	       << "exponential distributions." << std::endl;
	  abort_handler(-1);
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
	  abort_handler(-1);
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
	  abort_handler(-1);
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
	  abort_handler(-1);
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
	  abort_handler(-1);
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
	  abort_handler(-1);
	}
      }
      else if (type == CONTINUOUS_STATE) {
	if (map2 == "lower_bound")
	  active2ACVarMapTargets[curr_index] = Pecos::CSV_LWR_BND;
	else if (map2 == "upper_bound")
	  active2ACVarMapTargets[curr_index] = Pecos::CSV_UPR_BND;
	else {
	  Cerr << "\nError: " << map2 << " mapping not supported for "
	       << "continuous state variables." << std::endl;
	  abort_handler(-1);
	}
      }
      else {
	Cerr << "\nError: " << type << " variable type not supported in "
	     << "secondary real mappings\n       for primary continuous "
	     << "variable targets." << std::endl;
	abort_handler(-1);
      }
      active2ADIVarMapTargets[curr_index] = active2ADSVarMapTargets[curr_index]
	= active2ADRVarMapTargets[curr_index] = Pecos::NO_TARGET;
    }
    else if (adi_index1 != _NPOS) {
      unsigned short type = submodel_a_di_types[adi_index1];
      if (type == POISSON_UNCERTAIN) {
	if (map2 == "lambda")
	  active2ADIVarMapTargets[curr_index] = Pecos::P_LAMBDA;
	else {
	  Cerr << "\nError: " << map2 << " real mapping not supported for "
	       << "poisson uncertain variables." << std::endl;
	  abort_handler(-1);
	}
      }
      if (type == BINOMIAL_UNCERTAIN) {
	if (map2 == "prob_per_trial")
	  active2ADIVarMapTargets[curr_index] = Pecos::BI_P_PER_TRIAL;
	else {
	  Cerr << "\nError: " << map2 << " real mapping not supported for "
	       << "binomial uncertain variables." << std::endl;
	  abort_handler(-1);
	}
      }
      if (type == NEGATIVE_BINOMIAL_UNCERTAIN) {
	if (map2 == "prob_per_trial")
	  active2ADIVarMapTargets[curr_index] = Pecos::NBI_P_PER_TRIAL;
	else {
	  Cerr << "\nError: " << map2 << " real mapping not supported for "
	       << "negative binomial uncertain variables." << std::endl;
	  abort_handler(-1);
	}
      }
      if (type == GEOMETRIC_UNCERTAIN) {
	if (map2 == "prob_per_trial")
	  active2ADIVarMapTargets[curr_index] = Pecos::GE_P_PER_TRIAL;
	else {
	  Cerr << "\nError: " << map2 << " real mapping not supported for "
	       << "geometric uncertain variables." << std::endl;
	  abort_handler(-1);
	}
      }
      else {
	Cerr << "\nError: " << type << " variable type not supported in "
	     << "secondary real mappings\n       for primary discrete integer "
	     << "variable targets." << std::endl;
	abort_handler(-1);
      }
      active2ACVarMapTargets[curr_index] = active2ADSVarMapTargets[curr_index]
	= active2ADRVarMapTargets[curr_index] = Pecos::NO_TARGET;
    }
    else if (ads_index1 != _NPOS) {
      unsigned short type = submodel_a_ds_types[ads_index1];
      Cerr << "\nError: " << type << " variable type not supported in "
	   << "secondary real mappings\n       for primary discrete string "
	   << "variable targets." << std::endl;
      abort_handler(-1);
      active2ACVarMapTargets[curr_index] = active2ADIVarMapTargets[curr_index]
	= active2ADRVarMapTargets[curr_index] = Pecos::NO_TARGET;
    }
    else if (adr_index1 != _NPOS) {
      unsigned short type = submodel_a_dr_types[adr_index1];
      Cerr << "\nError: " << type << " variable type not supported in "
	   << "secondary real mappings\n       for primary discrete real "
	   << "variable targets." << std::endl;
      abort_handler(-1);
      active2ACVarMapTargets[curr_index] = active2ADIVarMapTargets[curr_index]
	= active2ADSVarMapTargets[curr_index] = Pecos::NO_TARGET;
    }
  }
}


void NestedModel::
resolve_integer_variable_mapping(const String& map1, const String& map2,
				 size_t curr_index, short& inactive_sm_view)
{
  UShortMultiArrayConstView submodel_a_c_types
    = subModel.all_continuous_variable_types();
  UShortMultiArrayConstView submodel_a_di_types
    = subModel.all_discrete_int_variable_types();
  UShortMultiArrayConstView submodel_a_ds_types
    = subModel.all_discrete_string_variable_types();
  UShortMultiArrayConstView submodel_a_dr_types
    = subModel.all_discrete_real_variable_types();

  size_t ac_index1 = active1ACVarMapIndices[curr_index]
      = find_index(subModel.all_continuous_variable_labels(), map1),
    adi_index1 = active1ADIVarMapIndices[curr_index]
      = find_index(subModel.all_discrete_int_variable_labels(), map1),
    ads_index1 = active1ADSVarMapIndices[curr_index]
      = find_index(subModel.all_discrete_string_variable_labels(), map1),
    adr_index1 = active1ADRVarMapIndices[curr_index]
      = find_index(subModel.all_discrete_real_variable_labels(), map1);
  if (ac_index1  == _NPOS && adi_index1 == _NPOS &&
      ads_index1 == _NPOS && adr_index1 == _NPOS) {
    Cerr << "\nError: primary mapping " << map1 << " could not be matched "
	 << "within any sub-model variable labels." << std::endl;
    abort_handler(-1);
  }

  if (ac_index1 != _NPOS &&
      find_index(subModel.continuous_variable_labels(), map1) == _NPOS)
    // inactive CV target
    update_inactive_view(submodel_a_c_types[ac_index1], inactive_sm_view);
  else if (adi_index1 != _NPOS &&
	   find_index(subModel.discrete_int_variable_labels(), map1) == _NPOS)
    // inactive DIV target
    update_inactive_view(submodel_a_di_types[adi_index1], inactive_sm_view);
  else if (ads_index1 != _NPOS &&
	   find_index(subModel.discrete_string_variable_labels(),map1) == _NPOS)
    // inactive DSV target
    update_inactive_view(submodel_a_ds_types[ads_index1], inactive_sm_view);
  else if (adr_index1 != _NPOS &&
	   find_index(subModel.discrete_real_variable_labels(), map1) == _NPOS)
    // inactive DRV target
    update_inactive_view(submodel_a_dr_types[adr_index1], inactive_sm_view);

  if (!active2ACVarMapTargets.empty()) { // indicates num_var_map_2
    if (map2.empty())
      active2ACVarMapTargets[curr_index] = active2ADIVarMapTargets[curr_index]
	= active2ADSVarMapTargets[curr_index]
	= active2ADRVarMapTargets[curr_index] = Pecos::NO_TARGET;
    else if (ac_index1 != _NPOS) {
      unsigned short type = submodel_a_c_types[ac_index1];
      Cerr << "\nError: " << type << " variable type not supported in "
	   << "secondary integer mappings\n       for primary continuous "
	   << "variable targets." << std::endl;
      abort_handler(-1);
      active2ADIVarMapTargets[curr_index] = active2ADSVarMapTargets[curr_index]
	= active2ADRVarMapTargets[curr_index] = Pecos::NO_TARGET;
    }
    else if (adi_index1 != _NPOS) {
      unsigned short type = submodel_a_di_types[adi_index1];
      if (type == DISCRETE_DESIGN_RANGE) {
	if (map2 == "lower_bound")
	  active2ADIVarMapTargets[curr_index] = Pecos::DDRIV_LWR_BND;
	else if (map2 == "upper_bound")
	  active2ADIVarMapTargets[curr_index] = Pecos::DDRIV_UPR_BND;
	else {
	  Cerr << "\nError: " << map2 << " mapping not supported for "
	       << "discrete design range variables." << std::endl;
	  abort_handler(-1);
	}
      }
      else if (type == BINOMIAL_UNCERTAIN) {
	if (map2 == "num_trials")
	  active2ADIVarMapTargets[curr_index] = Pecos::BI_TRIALS;
	else {
	  Cerr << "\nError: " << map2 << " mapping not supported for "
	       << "binomial uncertain variables." << std::endl;
	  abort_handler(-1);
	}
      }
      else if (type == NEGATIVE_BINOMIAL_UNCERTAIN) {
	if (map2 == "num_trials")
	  active2ADIVarMapTargets[curr_index] = Pecos::NBI_TRIALS;
	else {
	  Cerr << "\nError: " << map2 << " mapping not supported for "
	       << "negative binomial uncertain variables." << std::endl;
	  abort_handler(-1);
	}
      }
      else if (type == HYPERGEOMETRIC_UNCERTAIN) {
	if (map2 == "total_population")
	  active2ADIVarMapTargets[curr_index] = Pecos::HGE_TOT_POP;
	else if (map2 == "selected_population")
	  active2ADIVarMapTargets[curr_index] = Pecos::HGE_SEL_POP;
	else if (map2 == "num_drawn")
	  active2ADIVarMapTargets[curr_index] = Pecos::HGE_FAILED;
	else {
	  Cerr << "\nError: " << map2 << " mapping not supported for "
	       << "hypergeometric uncertain variables." << std::endl;
	  abort_handler(-1);
	}
      }
      else if (type == DISCRETE_STATE_RANGE) {
	if (map2 == "lower_bound")
	  active2ADIVarMapTargets[curr_index] = Pecos::DSRIV_LWR_BND;
	else if (map2 == "upper_bound")
	  active2ADIVarMapTargets[curr_index] = Pecos::DSRIV_UPR_BND;
	else {
	  Cerr << "\nError: " << map2 << " mapping not supported for "
	       << "discrete state range variables." << std::endl;
	  abort_handler(-1);
	}
      }
      else {
	Cerr << "\nError: " << type << " variable type not supported in "
	     << "secondary integer mappings\n       for primary discrete "
	     << "integer variable targets." << std::endl;
	abort_handler(-1);
      }
      active2ACVarMapTargets[curr_index] = active2ADSVarMapTargets[curr_index]
	= active2ADRVarMapTargets[curr_index] = Pecos::NO_TARGET;
    }
    else if (ads_index1 != _NPOS) {
      unsigned short type = submodel_a_ds_types[ads_index1];
      Cerr << "\nError: " << type << " variable type not supported in "
	   << "secondary integer mappings\n       for primary discrete string "
	   << "variable targets." << std::endl;
      abort_handler(-1);
      active2ACVarMapTargets[curr_index] = active2ADIVarMapTargets[curr_index]
	= active2ADRVarMapTargets[curr_index] = Pecos::NO_TARGET;
    }
    else if (adr_index1 != _NPOS) {
      unsigned short type = submodel_a_dr_types[adr_index1];
      Cerr << "\nError: " << type << " variable type not supported in "
	   << "secondary integer mappings\n       for primary discrete real "
	   << "variable targets." << std::endl;
      abort_handler(-1);
      active2ACVarMapTargets[curr_index] = active2ADIVarMapTargets[curr_index]
	= active2ADSVarMapTargets[curr_index] = Pecos::NO_TARGET;
    }
  }
}


void NestedModel::
resolve_string_variable_mapping(const String& map1, const String& map2,
				size_t curr_index, short& inactive_sm_view)
{
  UShortMultiArrayConstView submodel_a_c_types
    = subModel.all_continuous_variable_types();
  UShortMultiArrayConstView submodel_a_di_types
    = subModel.all_discrete_int_variable_types();
  UShortMultiArrayConstView submodel_a_ds_types
    = subModel.all_discrete_string_variable_types();
  UShortMultiArrayConstView submodel_a_dr_types
    = subModel.all_discrete_real_variable_types();

  size_t ac_index1 = active1ACVarMapIndices[curr_index]
      = find_index(subModel.all_continuous_variable_labels(), map1),
    adi_index1 = active1ADIVarMapIndices[curr_index]
      = find_index(subModel.all_discrete_int_variable_labels(), map1),
    ads_index1 = active1ADSVarMapIndices[curr_index]
      = find_index(subModel.all_discrete_string_variable_labels(), map1),
    adr_index1 = active1ADRVarMapIndices[curr_index]
      = find_index(subModel.all_discrete_real_variable_labels(), map1);
  if (ac_index1  == _NPOS && adi_index1 == _NPOS &&
      ads_index1 == _NPOS && adr_index1 == _NPOS) {
    Cerr << "\nError: primary mapping " << map1 << " could not be matched "
	 << "within any sub-model variable labels." << std::endl;
    abort_handler(-1);
  }

  if (ac_index1 != _NPOS &&
      find_index(subModel.continuous_variable_labels(), map1) == _NPOS)
    // inactive CV target
    update_inactive_view(submodel_a_c_types[ac_index1], inactive_sm_view);
  else if (adi_index1 != _NPOS &&
	   find_index(subModel.discrete_int_variable_labels(), map1) == _NPOS)
    // inactive DIV target
    update_inactive_view(submodel_a_di_types[adi_index1], inactive_sm_view);
  else if (ads_index1 != _NPOS &&
	   find_index(subModel.discrete_string_variable_labels(),map1) == _NPOS)
    // inactive DSV target
    update_inactive_view(submodel_a_ds_types[ads_index1], inactive_sm_view);
  else if (adr_index1 != _NPOS &&
	   find_index(subModel.discrete_real_variable_labels(), map1) == _NPOS)
    // inactive DRV target
    update_inactive_view(submodel_a_dr_types[adr_index1], inactive_sm_view);

  if (!active2ACVarMapTargets.empty()) { // indicates num_var_map_2
    if (map2.empty())
      active2ACVarMapTargets[curr_index] = active2ADIVarMapTargets[curr_index]
	= active2ADSVarMapTargets[curr_index]
	= active2ADRVarMapTargets[curr_index] = Pecos::NO_TARGET;
    else if (ac_index1 != _NPOS) {
      unsigned short type = submodel_a_c_types[ac_index1];
      Cerr << "\nError: " << type << " variable type not supported in "
	   << "secondary string mappings\n       for primary continuous "
	   << "variable targets." << std::endl;
      abort_handler(-1);
      active2ADIVarMapTargets[curr_index] = active2ADSVarMapTargets[curr_index]
	= active2ADRVarMapTargets[curr_index] = Pecos::NO_TARGET;
    }
    else if (adi_index1 != _NPOS) {
      unsigned short type = submodel_a_di_types[adi_index1];
      Cerr << "\nError: " << type << " variable type not supported in "
	   << "secondary string mappings\n       for primary discrete integer "
	   << "variable targets." << std::endl;
      abort_handler(-1);
      active2ACVarMapTargets[curr_index] = active2ADSVarMapTargets[curr_index]
	= active2ADRVarMapTargets[curr_index] = Pecos::NO_TARGET;
    }
    else if (ads_index1 != _NPOS) {
      unsigned short type = submodel_a_ds_types[ads_index1];
      Cerr << "\nError: " << type << " variable type not supported in "
	   << "secondary string mappings\n       for primary discrete string "
	   << "variable targets." << std::endl;
      abort_handler(-1);
      active2ACVarMapTargets[curr_index] = active2ADIVarMapTargets[curr_index]
	= active2ADRVarMapTargets[curr_index] = Pecos::NO_TARGET;
    }
    else if (adr_index1 != _NPOS) {
      unsigned short type = submodel_a_dr_types[adr_index1];
      Cerr << "\nError: " << type << " variable type not supported in "
	   << "secondary string mappings\n       for primary discrete real "
	   << "variable targets." << std::endl;
      abort_handler(-1);
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
    component_parallel_mode(OPTIONAL_INTERFACE);
    if (hierarchicalTagging) {
      String eval_tag = evalTagPrefix + '.' + 
	boost::lexical_cast<String>(nestedModelEvalCntr);
      // don't apply a redundant interface eval id
      bool append_iface_tag = false;
      optionalInterface.eval_tag_prefix(eval_tag, append_iface_tag);
    }

    ParConfigLIter pc_iter = parallelLib.parallel_configuration_iterator();
    parallelLib.parallel_configuration_iterator(modelPCIter);
    optionalInterface.map(currentVariables, opt_interface_set,
			  optInterfaceResponse);
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
    component_parallel_mode(SUB_MODEL);
    update_sub_model(currentVariables, userDefinedConstraints);
    subIterator.response_results_active_set(sub_iterator_set);
    if (hierarchicalTagging) {
      String eval_tag = evalTagPrefix + '.' + 
	boost::lexical_cast<String>(nestedModelEvalCntr);
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
    optionalInterface.map(currentVariables, opt_interface_set,
			  optInterfaceResponse, true);
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
  // overlapped as in HierarchSurrModel, given IteratorScheduler nowait support

  IntIntMIter id_it; IntRespMCIter r_cit;
  if (!optInterfacePointer.empty()) {
    component_parallel_mode(OPTIONAL_INTERFACE);

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
    component_parallel_mode(SUB_MODEL);
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
  size_t i, j, num_sub_iter_mapped_primary = primaryRespCoeffs.numRows(), 
    num_sub_iter_mapped_con = secondaryRespCoeffs.numRows(),
    num_opt_interf_con = numOptInterfIneqCon + numOptInterfEqCon,
    num_mapped_primary = std::max(numOptInterfPrimary, 
				  num_sub_iter_mapped_primary);

  const ShortArray& mapped_asv = mapped_set.request_vector();
  const SizetArray& mapped_dvv = mapped_set.derivative_vector();

  if (mapped_asv.size() != num_mapped_primary + num_opt_interf_con +
                           num_sub_iter_mapped_con) {
    Cerr << "\nError: mismatch is ASV lengths in NestedModel::set_mapping()."
	 << "\n       expected " << mapped_asv.size() << " total, received "
	 << num_mapped_primary << " primary plus " << num_opt_interf_con +
            num_sub_iter_mapped_con << " secondary." << std::endl;
    abort_handler(-1);
  }

  // sub_iterator_asv:

  sub_iterator_map = false;
  ShortArray sub_iterator_asv(numSubIterFns, 0);
  // augment sub_iterator_asv based on mapped primary asv and primaryRespCoeffs
  for (i=0; i<num_sub_iter_mapped_primary; ++i) {
    short mapped_asv_val = mapped_asv[i];
    if (mapped_asv_val) {
      for (j=0; j<numSubIterFns; ++j)
	if (std::fabs(primaryRespCoeffs(i,j)) > DBL_MIN)
	  sub_iterator_asv[j] |= mapped_asv_val;
      sub_iterator_map = true;
    }
  }
  // augment sub_iterator_asv based on mapped constr asv and secondaryRespCoeffs
  for (i=0; i<num_sub_iter_mapped_con; ++i) {
    short mapped_asv_val = mapped_asv[i+num_mapped_primary+num_opt_interf_con];
    if (mapped_asv_val) {
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
	abort_handler(-1);
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
    abort_handler(-1);
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
      = std::max(numOptInterfPrimary, (size_t)primaryRespCoeffs.numRows());
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
    abort_handler(-1);
  }
  check_response_map(mapped_asv);

  // build mapped response data:

  RealVector mapped_vals = mapped_response.function_values_view();
  const RealVector& sub_iterator_vals = sub_iterator_response.function_values();
  const RealMatrix& sub_iterator_grads
    = sub_iterator_response.function_gradients();
  const RealSymMatrixArray& sub_iterator_hessians
    = sub_iterator_response.function_hessians();

  // [W]{S}:
  size_t num_sub_iter_mapped_1 = primaryRespCoeffs.numRows();
  for (i=0; i<num_sub_iter_mapped_1; ++i) {
    if (mapped_asv[i] & 1) { // mapped_vals
      Real& inner_prod = mapped_vals[i];
      for (j=0; j<numSubIterFns; ++j)
	inner_prod += primaryRespCoeffs(i,j)*sub_iterator_vals[j]; // [W]{S}
    }
    if (mapped_asv[i] & 2) { // mapped_grads
      RealVector mapped_grad = mapped_response.function_gradient_view(i);
      for (j=0; j<num_mapped_deriv_vars; ++j) {
	Real& inner_prod = mapped_grad[j]; // [W]{S}
	for (k=0; k<numSubIterFns; ++k)
	  inner_prod += primaryRespCoeffs(i,k)*sub_iterator_grads(j,k);
      }
    }
    if (mapped_asv[i] & 4) { // mapped_hessians
      RealSymMatrix mapped_hess = mapped_response.function_hessian_view(i);
      for (j=0; j<num_mapped_deriv_vars; ++j) {
	for (k=0; k<=j; ++k) {
	  Real& inner_prod = mapped_hess(j,k); // [W]{S}
	  for (l=0; l<numSubIterFns; ++l)
	    inner_prod += primaryRespCoeffs(i,l)*sub_iterator_hessians[l](j,k);
	}
      }
    }
  }

  // [A]{S}:
  size_t num_sub_iter_mapped_2 = secondaryRespCoeffs.numRows(),
    num_mapped_1 = std::max(numOptInterfPrimary, num_sub_iter_mapped_1);
  for (i=0; i<num_sub_iter_mapped_2; ++i) {
    m_index = num_mapped_1 + numOptInterfIneqCon + i;// {a_l} <= [A]{S} <= {a_u}
    if (i>=numSubIterMappedIneqCon)
      m_index += numOptInterfEqCon;                           // [A]{S} == {a_t}
    if (mapped_asv[m_index] & 1) { // mapped_vals
      Real& inner_prod = mapped_vals[m_index]; inner_prod = 0.;
      for (j=0; j<numSubIterFns; ++j)
        inner_prod += secondaryRespCoeffs(i,j)*sub_iterator_vals[j];
    }
    if (mapped_asv[m_index] & 2) { // mapped_grads
      RealVector mapped_grad = mapped_response.function_gradient_view(m_index);
      for (j=0; j<num_mapped_deriv_vars; ++j) {
        Real& inner_prod = mapped_grad[j]; inner_prod = 0.;
        for (k=0; k<numSubIterFns; ++k)
          inner_prod += secondaryRespCoeffs(i,k)*sub_iterator_grads(j,k);
      }
    }
    if (mapped_asv[m_index] & 4) { // mapped_hessians
      RealSymMatrix mapped_hess
        = mapped_response.function_hessian_view(m_index);
      for (j=0; j<num_mapped_deriv_vars; ++j) {
        for (k=0; k<=j; ++k) {
          Real& inner_prod = mapped_hess(j,k); inner_prod = 0.;
          for (l=0; l<numSubIterFns; ++l)
            inner_prod += secondaryRespCoeffs(i,l)
	               *  sub_iterator_hessians[l](j,k);
	}
      }
    }
  }
}


void NestedModel::
iterator_error_estimation(const RealVector& sub_iterator_errors,
			  RealVector& mapped_errors)
{
  // In the future, could be overlaid with optional interface error estimates,
  // but for now, assume these are zero (e.g., deterministic mappings have no
  // estimator variance)

  size_t i, j, m_index, num_mapped_fns = currentResponse.num_functions();
  if (mapped_errors.length() != num_mapped_fns)
    mapped_errors.size(num_mapped_fns); // init to 0 
  else
    mapped_errors = 0.;

  // Assume independent Gaussian errors: then std error from linear combination
  // of Gaussian errors = Sqrt[ Sum [ coeff^2 sigma_i^2 ] ]
  // Note: final moments may be central or standard, but error estimates are
  //       always standard (sqrt of estimator variance of central/std moment)

  // [W]{S}:
  size_t num_sub_iter_mapped_1 = primaryRespCoeffs.numRows();
  Real sum, term;
  for (i=0; i<num_sub_iter_mapped_1; ++i) {
    sum = 0.;
    for (j=0; j<numSubIterFns; ++j) {
      term = primaryRespCoeffs(i,j) * sub_iterator_errors[j]; // [W]{S}
      sum += term * term;
    }
    mapped_errors[i] = std::sqrt(sum);
  }

  // [A]{S}:
  size_t num_sub_iter_mapped_2 = secondaryRespCoeffs.numRows(),
    num_mapped_1 = std::max(numOptInterfPrimary, num_sub_iter_mapped_1);
  for (i=0; i<num_sub_iter_mapped_2; ++i) {
    m_index = num_mapped_1 + numOptInterfIneqCon + i;// {a_l} <= [A]{S} <= {a_u}
    if (i>=numSubIterMappedIneqCon)
      m_index += numOptInterfEqCon;                           // [A]{S} == {a_t}
    sum = 0.;
    for (j=0; j<numSubIterFns; ++j) {
      term = secondaryRespCoeffs(i,j) * sub_iterator_errors[j]; // [W]{S}
      sum += term * term;
    }
    mapped_errors[m_index] = std::sqrt(sum);
  }
}


void NestedModel::check_response_map(const ShortArray& mapped_asv)
{
  // counter initialization & sanity checking
  // NOTE: numSubIterFns != num_sub_iter_mapped_primary+num_sub_iter_mapped_con
  // since subIterator response is converted to sub_iter_mapped_primary/con
  // through the action of [W] and [A].
  size_t num_opt_interf_con = numOptInterfIneqCon + numOptInterfEqCon,
    num_sub_iter_mapped_1 = primaryRespCoeffs.numRows(),
    num_sub_iter_mapped_2 = secondaryRespCoeffs.numRows(),
    num_mapped_1 = std::max(numOptInterfPrimary, num_sub_iter_mapped_1);
  if (mapped_asv.size() !=
      num_mapped_1 + num_opt_interf_con + num_sub_iter_mapped_2 ||
      num_sub_iter_mapped_2 !=
      numSubIterMappedIneqCon + numSubIterMappedEqCon) {
    Cerr << "\nError: bad function counts in NestedModel::check_response_map()."
         << std::endl;
    abort_handler(-1);
  }
}


void NestedModel::component_parallel_mode(short mode)
{
  // mode may be correct, but can't guarantee active parallel config is in sync
  //if (componentParallelMode == mode)
  //  return; // already in correct parallel mode

  // terminate previous serve mode (if active)
  if (componentParallelMode != mode) {
    if (componentParallelMode == OPTIONAL_INTERFACE) {
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
    else if (componentParallelMode == SUB_MODEL &&
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
  if (mode == OPTIONAL_INTERFACE)
    parallelLib.parallel_configuration_iterator(modelPCIter);
  else if (mode == SUB_MODEL) {
    // ParallelLibrary::currPCIter activation delegated to subModel
  }
  */

  // activate new serve mode (matches NestedModel::serve_run(pl_iter)).  This
  // bcast matches the outer parallel context prior to subIterator partitioning.
  // > OPTIONAL_INTERFACE & subModel eval scheduling only bcast if mode change
  // > concurrent subIterator scheduling rebroadcasts every time since this
  //   scheduling exits on its own (see IteratorScheduler::schedule_iterators())
  if ( ( componentParallelMode != mode ||
	 ( mode == SUB_MODEL && subIteratorSched.messagePass ) ) &&
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
    if (componentParallelMode == OPTIONAL_INTERFACE &&
	!optInterfacePointer.empty()) {
      // store/set/restore the ParallelLibrary::currPCIter
      ParConfigLIter pc_iter = parallelLib.parallel_configuration_iterator();
      parallelLib.parallel_configuration_iterator(modelPCIter);
      optionalInterface.serve_evaluations();
      parallelLib.parallel_configuration_iterator(pc_iter); // restore
    }
    else if (componentParallelMode == SUB_MODEL) {
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
      abort_handler(-1);
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
  // combination of variable insertions and augmentations.  Insertions
  // and augmentations are not mutually exclusive, so subModel updates must
  // account for both, potentially on a variable by variable basis.

  // Top-level active variables/bounds/labels are mapped to the subModel, but
  // top-level linear/nonlinear constraint coefficients/bounds/targets are not
  // propagated (as they are in the surrogate model classes) since they are not
  // relevant at the subModel level.

  // For defined variable mappings, insert active currentVariables into subModel
  // variables.  When secondary mappings are defined, this insertion involves
  // the updating of sub-parameters for the subModel variables.  When only
  // primary mappings are defined, the insertions update the subModel variable
  // values.  For null primary mappings (empty strings), augmentations are
  // performed by updating the subModel variables of the same variable type as
  // the active currentVariables.

  // Bounds and labels are only updated for the augmented variables.  Labels
  // should not be updated for inserted variables, and bounds should not be
  // updated for inserted variables with secondary mappings (since this would
  // be redundant with bound parameter insertion).  It may be desirable in the
  // future to update bounds for inserted variables with only primary mappings,
  // but this is omitted for now.

  // Distribution parameter insertion is used for second-order probability
  // (UQ sample inserted into distr. parameters for UQ reliability/sampling)
  // and for OUU with design/uncertain overlap (optimization design variables
  // inserted into distr. parameters for UQ reliability/sampling).  For now,
  // only the existing distribution parameters are supported for the secondary
  // variable mappings.  In the future, it would be desirable to allow
  // additional parameter mappings, such as setting the mean of a weibull.

  size_t i, num_var_map_2 = active2ACVarMapTargets.size();

  // Map ACTIVE CONTINUOUS VARIABLES from currentVariables
  size_t curr_i, num_curr_cv = vars.cv();
  const RealVector& curr_c_vars   = vars.continuous_variables();
  const RealVector& curr_c_l_bnds = cons.continuous_lower_bounds();
  const RealVector& curr_c_u_bnds = cons.continuous_upper_bounds();
  StringMultiArrayConstView curr_c_labels
    = vars.continuous_variable_labels();
  for (i=0; i<num_curr_cv; ++i) {
    curr_i = cv_index_map(i, vars);
    size_t pacvm_index = active1ACVarMapIndices[curr_i],
      padivm_index = active1ADIVarMapIndices[curr_i],
      padsvm_index = active1ADSVarMapIndices[curr_i],
      padrvm_index = active1ADRVarMapIndices[curr_i];
    if (pacvm_index != _NPOS) {
      short sacvm_target
	= (num_var_map_2) ? active2ACVarMapTargets[curr_i] : Pecos::NO_TARGET;
      if (sacvm_target == Pecos::NO_TARGET) {
	subModel.all_continuous_variable(curr_c_vars[i], pacvm_index);
	if (extraCVarsData[i]) {
	  subModel.all_continuous_lower_bound(curr_c_l_bnds[i], pacvm_index);
	  subModel.all_continuous_upper_bound(curr_c_u_bnds[i], pacvm_index);
	  if (firstUpdate)
	    subModel.all_continuous_variable_label(curr_c_labels[i],
						   pacvm_index);
	}
      }
      else {
	size_t mapped_index = sm_acv_index_map(pacvm_index, sacvm_target);
	real_variable_mapping(curr_c_vars[i], mapped_index, sacvm_target);
      }
    }
    else if (padivm_index != _NPOS) {
      short sadivm_target = (num_var_map_2) ? active2ADIVarMapTargets[curr_i]
	: Pecos::NO_TARGET;
      size_t mapped_index = sm_adiv_index_map(padivm_index, sadivm_target);
      real_variable_mapping(curr_c_vars[i], mapped_index, sadivm_target);
    }
    else if (padsvm_index != _NPOS) {
      short sadsvm_target = (num_var_map_2) ? active2ADSVarMapTargets[curr_i]
	: Pecos::NO_TARGET;
      size_t mapped_index = sm_adsv_index_map(padsvm_index, sadsvm_target);
      real_variable_mapping(curr_c_vars[i], mapped_index, sadsvm_target);
    }
    else if (padrvm_index != _NPOS) {
      short sadrvm_target = (num_var_map_2) ? active2ADRVarMapTargets[curr_i]
	: Pecos::NO_TARGET;
      size_t mapped_index = sm_adrv_index_map(padrvm_index, sadrvm_target);
      real_variable_mapping(curr_c_vars[i], mapped_index, sadrvm_target);
    }
  }

  // Map ACTIVE DISCRETE INTEGER VARIABLES from currentVariables
  size_t num_curr_div = vars.div();
  const IntVector& curr_di_vars   = vars.discrete_int_variables();
  const IntVector& curr_di_l_bnds = cons.discrete_int_lower_bounds();
  const IntVector& curr_di_u_bnds = cons.discrete_int_upper_bounds();
  StringMultiArrayConstView curr_di_labels
    = vars.discrete_int_variable_labels();
  for (i=0; i<num_curr_div; ++i) {
    curr_i = div_index_map(i, vars);
    size_t pacvm_index = active1ACVarMapIndices[curr_i],
      padivm_index = active1ADIVarMapIndices[curr_i],
      padsvm_index = active1ADSVarMapIndices[curr_i],
      padrvm_index = active1ADRVarMapIndices[curr_i];
    if (pacvm_index != _NPOS) {
      short sacvm_target
	= (num_var_map_2) ? active2ACVarMapTargets[curr_i] : Pecos::NO_TARGET;
      size_t mapped_index = sm_acv_index_map(pacvm_index, sacvm_target);
      integer_variable_mapping(curr_di_vars[i], mapped_index, sacvm_target);
    }
    else if (padivm_index != _NPOS) {
      short sadivm_target = (num_var_map_2) ?
	active2ADIVarMapTargets[curr_i] : Pecos::NO_TARGET;
      if (sadivm_target == Pecos::NO_TARGET) {
	subModel.all_discrete_int_variable(curr_di_vars[i], padivm_index);
	if (extraDIVarsData[i]) {
	  subModel.all_discrete_int_lower_bound(curr_di_l_bnds[i],padivm_index);
	  subModel.all_discrete_int_upper_bound(curr_di_u_bnds[i],padivm_index);
	  if (firstUpdate)
	    subModel.all_discrete_int_variable_label(curr_di_labels[i],
						     padivm_index);
	}
      }
      else {
	size_t mapped_index = sm_adiv_index_map(padivm_index, sadivm_target);
	integer_variable_mapping(curr_di_vars[i], mapped_index, sadivm_target);
      }
    }
    else if (padsvm_index != _NPOS) {
      short sadsvm_target = (num_var_map_2) ?
	active2ADSVarMapTargets[curr_i] : Pecos::NO_TARGET;
      size_t mapped_index = sm_adsv_index_map(padsvm_index, sadsvm_target);
      integer_variable_mapping(curr_di_vars[i], mapped_index, sadsvm_target);
    }
    else if (padrvm_index != _NPOS) {
      short sadrvm_target = (num_var_map_2) ?
	active2ADRVarMapTargets[curr_i] : Pecos::NO_TARGET;
      size_t mapped_index = sm_adrv_index_map(padrvm_index, sadrvm_target);
      integer_variable_mapping(curr_di_vars[i], mapped_index, sadrvm_target);
    }
  }

  // Map ACTIVE DISCRETE STRING VARIABLES from currentVariables
  size_t num_curr_dsv = vars.dsv();
  StringMultiArrayConstView curr_ds_vars = vars.discrete_string_variables();
  StringMultiArrayConstView curr_ds_labels
    = vars.discrete_string_variable_labels();
  for (i=0; i<num_curr_dsv; ++i) {
    curr_i = dsv_index_map(i, vars);
    size_t pacvm_index  = active1ACVarMapIndices[curr_i],
      padivm_index = active1ADIVarMapIndices[curr_i],
      padsvm_index = active1ADSVarMapIndices[curr_i],
      padrvm_index = active1ADRVarMapIndices[curr_i];
    if (pacvm_index != _NPOS) {
      short sacvm_target = (num_var_map_2) ?
	active2ACVarMapTargets[curr_i] : Pecos::NO_TARGET;
      size_t mapped_index = sm_acv_index_map(pacvm_index, sacvm_target);
      string_variable_mapping(curr_ds_vars[i], mapped_index, sacvm_target);
    }
    else if (padivm_index != _NPOS) {
      short sadivm_target = (num_var_map_2) ?
	active2ADIVarMapTargets[curr_i] : Pecos::NO_TARGET;
      size_t mapped_index = sm_adiv_index_map(padivm_index, sadivm_target);
      string_variable_mapping(curr_ds_vars[i], mapped_index, sadivm_target);
    }
    else if (padsvm_index != _NPOS) {
      short sadsvm_target = (num_var_map_2) ?
	active2ADSVarMapTargets[curr_i] : Pecos::NO_TARGET;
      if (sadsvm_target == Pecos::NO_TARGET) {
	subModel.all_discrete_string_variable(curr_ds_vars[i], padsvm_index);
	if (extraDSVarsData[i] && firstUpdate)
	  subModel.all_discrete_string_variable_label(curr_ds_labels[i],
						      padsvm_index);
      }
      else {
	size_t mapped_index = sm_adsv_index_map(padsvm_index, sadsvm_target);
	string_variable_mapping(curr_ds_vars[i], mapped_index, sadsvm_target);
      }
    }
    else if (padrvm_index != _NPOS) {
      short sadrvm_target = (num_var_map_2) ?
	active2ADRVarMapTargets[curr_i] : Pecos::NO_TARGET;
      size_t mapped_index = sm_adrv_index_map(padrvm_index, sadrvm_target);
      string_variable_mapping(curr_ds_vars[i], mapped_index, sadrvm_target);
    }
  }

  // Map ACTIVE DISCRETE REAL VARIABLES from currentVariables
  size_t num_curr_drv = vars.drv();
  const RealVector& curr_dr_vars   = vars.discrete_real_variables();
  const RealVector& curr_dr_l_bnds = cons.discrete_real_lower_bounds();
  const RealVector& curr_dr_u_bnds = cons.discrete_real_upper_bounds();
  StringMultiArrayConstView curr_dr_labels
    = vars.discrete_real_variable_labels();
  for (i=0; i<num_curr_drv; ++i) {
    curr_i = drv_index_map(i, vars);
    size_t pacvm_index  = active1ACVarMapIndices[curr_i],
      padivm_index = active1ADIVarMapIndices[curr_i],
      padsvm_index = active1ADSVarMapIndices[curr_i],
      padrvm_index = active1ADRVarMapIndices[curr_i];
    if (pacvm_index != _NPOS) {
      short sacvm_target = (num_var_map_2) ?
	active2ACVarMapTargets[curr_i] : Pecos::NO_TARGET;
      size_t mapped_index = sm_acv_index_map(pacvm_index, sacvm_target);
      real_variable_mapping(curr_dr_vars[i], mapped_index, sacvm_target);
    }
    else if (padivm_index != _NPOS) {
      short sadivm_target = (num_var_map_2) ?
	active2ADIVarMapTargets[curr_i] : Pecos::NO_TARGET;
      size_t mapped_index = sm_adiv_index_map(padivm_index, sadivm_target);
      real_variable_mapping(curr_dr_vars[i], mapped_index, sadivm_target);
    }
    else if (padsvm_index != _NPOS) {
      short sadsvm_target = (num_var_map_2) ?
	active2ADSVarMapTargets[curr_i] : Pecos::NO_TARGET;
      size_t mapped_index = sm_adsv_index_map(padsvm_index, sadsvm_target);
      real_variable_mapping(curr_dr_vars[i], mapped_index, sadsvm_target);
    }
    else if (padrvm_index != _NPOS) {
      short sadrvm_target = (num_var_map_2) ?
	active2ADRVarMapTargets[curr_i] : Pecos::NO_TARGET;
      if (sadrvm_target == Pecos::NO_TARGET) {
	subModel.all_discrete_real_variable(curr_dr_vars[i], padrvm_index);
	if (extraDRVarsData[i]) {
	  subModel.all_discrete_real_lower_bound(curr_dr_l_bnds[i],
						 padrvm_index);
	  subModel.all_discrete_real_upper_bound(curr_dr_u_bnds[i],
						 padrvm_index);
	  if (firstUpdate)
	    subModel.all_discrete_real_variable_label(curr_dr_labels[i],
						      padrvm_index);
	}
      }
      else {
	size_t mapped_index = sm_adrv_index_map(padrvm_index, sadrvm_target);
	real_variable_mapping(curr_dr_vars[i], mapped_index, sadrvm_target);
      }
    }
  }

  // Map COMPLEMENT CONTINUOUS VARIABLES from currentVariables into
  // corresponding subModel type (using same logic as default active mapping)
  size_t num_curr_ccv = vars.acv() - num_curr_cv;
  const RealVector& curr_ac_vars   = vars.all_continuous_variables();
  const RealVector& curr_ac_l_bnds = cons.all_continuous_lower_bounds();
  const RealVector& curr_ac_u_bnds = cons.all_continuous_upper_bounds();
  StringMultiArrayConstView curr_ac_labels
    = vars.all_continuous_variable_labels();
  for (i=0; i<num_curr_ccv; ++i) {
    curr_i = ccv_index_map(i, vars);
    size_t c1_index = complement1ACVarMapIndices[i];
    subModel.all_continuous_variable(curr_ac_vars[curr_i], c1_index);
    subModel.all_continuous_lower_bound(curr_ac_l_bnds[curr_i], c1_index);
    subModel.all_continuous_upper_bound(curr_ac_u_bnds[curr_i], c1_index);
    if (firstUpdate)
      subModel.all_continuous_variable_label(curr_ac_labels[curr_i], c1_index);
  }

  // Map COMPLEMENT DISCRETE INTEGER VARIABLES from currentVariables into
  // corresponding subModel type (using same logic as default active mapping)
  size_t num_curr_cdiv = vars.adiv() - num_curr_div;
  const IntVector& curr_adi_vars   = vars.all_discrete_int_variables();
  const IntVector& curr_adi_l_bnds = cons.all_discrete_int_lower_bounds();
  const IntVector& curr_adi_u_bnds = cons.all_discrete_int_upper_bounds();
  StringMultiArrayConstView curr_adi_labels
    = vars.all_discrete_int_variable_labels();
  for (i=0; i<num_curr_cdiv; ++i) {
    curr_i = cdiv_index_map(i, vars);
    size_t c1_index = complement1ADIVarMapIndices[i];
    subModel.all_discrete_int_variable(curr_adi_vars[curr_i], c1_index);
    subModel.all_discrete_int_lower_bound(curr_adi_l_bnds[curr_i], c1_index);
    subModel.all_discrete_int_upper_bound(curr_adi_u_bnds[curr_i], c1_index);
    if (firstUpdate)
      subModel.all_discrete_int_variable_label(curr_adi_labels[curr_i],
					       c1_index);
  }

  // Map COMPLEMENT DISCRETE STRING VARIABLES from currentVariables into
  // corresponding subModel type (using same logic as default active mapping)
  size_t num_curr_cdsv = vars.adsv() - num_curr_dsv;
  StringMultiArrayConstView curr_ads_vars
    = vars.all_discrete_string_variables();
  StringMultiArrayConstView curr_ads_labels
    = vars.all_discrete_string_variable_labels();
  for (i=0; i<num_curr_cdsv; ++i) {
    curr_i = cdsv_index_map(i, vars);
    size_t c1_index = complement1ADSVarMapIndices[i];
    subModel.all_discrete_string_variable(curr_ads_vars[curr_i], c1_index);
    if (firstUpdate)
      subModel.all_discrete_string_variable_label(curr_ads_labels[curr_i],
						  c1_index);
  }

  // Map COMPLEMENT DISCRETE REAL VARIABLES from currentVariables into
  // corresponding subModel type (using same logic as default active mapping)
  size_t num_curr_cdrv = vars.adrv() - num_curr_drv;
  const RealVector& curr_adr_vars   = vars.all_discrete_real_variables();
  const RealVector& curr_adr_l_bnds = cons.all_discrete_real_lower_bounds();
  const RealVector& curr_adr_u_bnds = cons.all_discrete_real_upper_bounds();
  StringMultiArrayConstView curr_adr_labels
    = vars.all_discrete_real_variable_labels();
  for (i=0; i<num_curr_cdrv; ++i) {
    curr_i = cdrv_index_map(i, vars);
    size_t c1_index = complement1ADRVarMapIndices[i];
    subModel.all_discrete_real_variable(curr_adr_vars[curr_i], c1_index);
    subModel.all_discrete_real_lower_bound(curr_adr_l_bnds[curr_i], c1_index);
    subModel.all_discrete_real_upper_bound(curr_adr_u_bnds[curr_i], c1_index);
    if (firstUpdate)
      subModel.all_discrete_real_variable_label(curr_adr_labels[curr_i],
						c1_index);
  }

  firstUpdate = false;
}


/** maps index within active continuous variables to index within aggregated
    active continuous/discrete-int/discrete-string/discrete-real variables. */
  size_t NestedModel::cv_index_map(size_t cv_index, const Variables& vars)
{
  size_t offset;
  const SharedVariablesData& svd = vars.shared_data();
  switch (svd.view().first) { // active view
  case MIXED_UNCERTAIN: case RELAXED_UNCERTAIN: {
    //  active cv order is cauv,ceuv;
    // aggregated order is cauv/dauiv/dausv/daurv,ceuv/deuiv/deusv/deurv:
    size_t num_cauv, num_dauiv, num_dausv, num_daurv;
    svd.aleatory_uncertain_counts(num_cauv, num_dauiv, num_dausv, num_daurv);
    offset = (cv_index < num_cauv) ? 0 : num_dauiv + num_dausv + num_daurv;
    break;
  }
  case MIXED_ALL: case RELAXED_ALL: {
    //  active cv order is cdv,cauv,ceuv,csv;
    // aggregated order is cdv/ddiv/ddsv/ddrv,cauv/dauiv/dausv/daurv,
    // ceuv/deuiv/deusv/deurv,csv/dsiv/dssv/dsrv:
    size_t num_cdv, num_ddiv, num_ddsv, num_ddrv;
    svd.design_counts(num_cdv, num_ddiv, num_ddsv, num_ddrv);
    if (cv_index < num_cdv)
      offset = 0;
    else {
      size_t num_cauv, num_dauiv, num_dausv, num_daurv;
      svd.aleatory_uncertain_counts(num_cauv, num_dauiv, num_dausv, num_daurv);
      if (cv_index < num_cdv + num_cauv)            // continuous aleatory
	offset = num_ddiv + num_ddsv + num_ddrv;
      else {
	size_t num_ceuv, num_deuiv, num_deusv, num_deurv;
	svd.epistemic_uncertain_counts(num_ceuv, num_deuiv, num_deusv,
				       num_deurv);
	if (cv_index < num_cdv + num_cauv + num_ceuv) // continuous epistemic
	  offset = num_ddiv  + num_ddsv  + num_ddrv
	         + num_dauiv + num_dausv + num_daurv;
	else                                              // continuous state
	  offset = num_ddiv  + num_ddsv  + num_ddrv
	         + num_dauiv + num_dausv + num_daurv
	         + num_deuiv + num_deusv + num_deurv;
      }
    }
    break;
  }
  default: // MIXED and RELAXED for single variable types
    offset = 0; break;
  }
  return cv_index + offset;
}


/** maps index within active discrete int variables to index within aggregated
    active continuous/discrete-int/discrete-string/discrete-real variables. */
size_t NestedModel::div_index_map(size_t div_index, const Variables& vars)
{
  size_t offset;
  const SharedVariablesData& svd = vars.shared_data();
  switch (svd.view().first) { // active view
  case MIXED_UNCERTAIN: case RELAXED_UNCERTAIN: {
    // active div order is dauiv/deuiv
    // aggregated order is cauv/dauiv/dausv/daurv,ceuv/deuiv/deusv/deurv:
    size_t num_cauv, num_dauiv, num_dausv, num_daurv;
    svd.aleatory_uncertain_counts(num_cauv, num_dauiv, num_dausv, num_daurv);
    if (div_index < num_dauiv) // discrete int aleatory
      offset = num_cauv;
    else { // discrete int epistemic
      size_t num_ceuv, num_deuiv, num_deusv, num_deurv;
      svd.epistemic_uncertain_counts(num_ceuv, num_deuiv, num_deusv, num_deurv);
      offset = num_cauv + num_dausv + num_daurv + num_ceuv;
    }
    break;
  }
  case MIXED_ALL: case RELAXED_ALL: {
    // active div order is ddiv,dauiv,deuiv,dsiv; aggregated order is
    // cdv/ddiv/ddsv/ddrv,cauv/dauiv/dausv/daurv,ceuv/deuiv/deusv/deurv,
    // csv/dsiv/dssv/dsrv:
    size_t num_cdv, num_ddiv, num_ddsv, num_ddrv;
    svd.design_counts(num_cdv, num_ddiv, num_ddsv, num_ddrv);
    if (div_index < num_ddiv)                              // disc int design
      offset = num_cdv;
    else {
      size_t num_cauv, num_dauiv, num_dausv, num_daurv;
      svd.aleatory_uncertain_counts(num_cauv, num_dauiv, num_dausv, num_daurv);
      if (div_index < num_ddiv + num_dauiv)             // disc int aleatory
	offset = num_cdv + num_ddsv + num_ddrv + num_cauv;
      else {
	size_t num_ceuv, num_deuiv, num_deusv, num_deurv;
	svd.epistemic_uncertain_counts(num_ceuv, num_deuiv, num_deusv,
				       num_deurv);
	if (div_index < num_ddiv + num_dauiv + num_deuiv) // disc int epistemic
	  offset = num_cdv  + num_ddsv  + num_ddrv
	         + num_cauv + num_dausv + num_daurv + num_ceuv;
	else { // disc int state
	  size_t num_csv, num_dsiv, num_dssv, num_dsrv;
	  svd.state_counts(num_csv, num_dsiv, num_dssv, num_dsrv);
	  offset = num_cdv  + num_ddsv  + num_ddrv
	         + num_cauv + num_dausv + num_daurv
	         + num_ceuv + num_deusv + num_deurv + num_csv;
	}
      }
    }
    break;
  }
  default: // MIXED and RELAXED for single variable types
    offset = vars.cv(); break;
  }
  return div_index + offset;
}


/** maps index within active discrete string variables to index within
    aggregated active continuous/discrete-int/discrete-string/discrete-string
    variables. */
size_t NestedModel::dsv_index_map(size_t dsv_index, const Variables& vars)
{
  size_t offset;
  const SharedVariablesData& svd = vars.shared_data();
  switch (svd.view().first) { // active view
  case MIXED_UNCERTAIN: case RELAXED_UNCERTAIN: {
    // active dsv order is dausv/deusv
    // aggregated order is cauv/dauiv/dausv/daurv,ceuv/deuiv/deusv/deurv:
    size_t num_cauv, num_dauiv, num_dausv, num_daurv;
    svd.aleatory_uncertain_counts(num_cauv, num_dauiv, num_dausv, num_daurv);
    if (dsv_index < num_dausv) // discrete string aleatory
      offset = num_cauv + num_dauiv;
    else { // discrete string epistemic
      size_t num_ceuv, num_deuiv, num_deusv, num_deurv;
      svd.epistemic_uncertain_counts(num_ceuv, num_deuiv, num_deusv, num_deurv);
      offset = num_cauv + num_dauiv + num_daurv + num_ceuv + num_deuiv;
    }
    break;
  }
  case MIXED_ALL: case RELAXED_ALL: {
    // active dsv order is ddsv,dausv,deusv,dssv; aggregated order is
    // cdv/ddiv/ddsv/ddrv,cauv/dauiv/dausv/daurv,ceuv/deuiv/deusv/deurv,
    // csv/dsiv/dssv/dsrv:
    size_t num_cdv, num_ddiv, num_ddsv, num_ddrv;
    svd.design_counts(num_cdv, num_ddiv, num_ddsv, num_ddrv);
    if (dsv_index < num_ddsv)                           // disc string design
      offset = num_cdv + num_ddiv;
    else {
      size_t num_cauv, num_dauiv, num_dausv, num_daurv;
      svd.aleatory_uncertain_counts(num_cauv, num_dauiv, num_dausv, num_daurv);
      if (dsv_index < num_ddsv + num_dausv)             // disc string aleatory
	offset = num_cdv + num_ddiv + num_ddrv + num_cauv + num_dauiv;
      else {
	size_t num_ceuv, num_deuiv, num_deusv, num_deurv;
	svd.epistemic_uncertain_counts(num_ceuv, num_deuiv, num_deusv,
				       num_deurv);
	if (dsv_index < num_ddsv + num_dausv + num_deusv)//disc string epistemic
	  offset = num_cdv  + num_ddiv  + num_ddrv
	         + num_cauv + num_dauiv + num_daurv
	         + num_ceuv + num_deuiv;
	else {                                          // disc string state
	  size_t num_csv, num_dsiv, num_dssv, num_dsrv;
	  svd.state_counts(num_csv, num_dsiv, num_dssv, num_dsrv);
	  offset = num_cdv  + num_ddiv  + num_ddrv
	         + num_cauv + num_dauiv + num_daurv
	         + num_ceuv + num_deuiv + num_deurv
	         + num_csv  + num_dsiv;
	}
      }
    }
    break;
  }
  default: // MIXED and RELAXED for single variable types
    offset = vars.cv() + vars.div(); break;
  }
  return dsv_index + offset;
}


/** maps index within active discrete real variables to index within aggregated
    active continuous/discrete-int/discrete-string/discrete-real variables. */
size_t NestedModel::drv_index_map(size_t drv_index, const Variables& vars)
{
  size_t offset;
  const SharedVariablesData& svd = vars.shared_data();
  switch (svd.view().first) { // active view
  case MIXED_UNCERTAIN: case RELAXED_UNCERTAIN: {
    // active drv order is daurv/deurv
    // aggregated order is cauv/dauiv/dausv/daurv,ceuv/deuiv/deusv/deurv:
    size_t num_cauv, num_dauiv, num_dausv, num_daurv;
    svd.aleatory_uncertain_counts(num_cauv, num_dauiv, num_dausv, num_daurv);
    if (drv_index < num_daurv) // discrete real aleatory
      offset = num_cauv + num_dauiv + num_dausv;
    else { // discrete real epistemic
      size_t num_ceuv, num_deuiv, num_deusv, num_deurv;
      svd.epistemic_uncertain_counts(num_ceuv, num_deuiv, num_deusv, num_deurv);
      offset = num_cauv + num_dauiv + num_dausv
	     + num_ceuv + num_deuiv + num_deusv;
    }
    break;
  }
  case MIXED_ALL: case RELAXED_ALL: {
    // active drv order is ddrv,daurv,deurv,dsrv; aggregated order is
    // cdv/ddiv/ddsv/ddrv,cauv/dauiv/dausv/daurv,ceuv/deuiv/deusv/deurv,
    // csv/dsiv/dssv/dsrv:
    size_t num_cdv, num_ddiv, num_ddsv, num_ddrv;
    svd.design_counts(num_cdv, num_ddiv, num_ddsv, num_ddrv);
    if (drv_index < num_ddrv)                             // disc real design
      offset = num_cdv + num_ddiv + num_ddsv;
    else {
      size_t num_cauv, num_dauiv, num_dausv, num_daurv;
      svd.aleatory_uncertain_counts(num_cauv, num_dauiv, num_dausv, num_daurv);
      if (drv_index < num_ddrv + num_daurv)               // disc real aleatory
	offset = num_cdv  + num_ddiv  + num_ddsv
	       + num_cauv + num_dauiv + num_dausv;
      else {
	size_t num_ceuv, num_deuiv, num_deusv, num_deurv;
	svd.epistemic_uncertain_counts(num_ceuv, num_deuiv, num_deusv,
				       num_deurv);
	if (drv_index < num_ddrv + num_daurv + num_deurv) // disc real epistemic
	  offset = num_cdv  + num_ddiv  + num_ddsv
	         + num_cauv + num_dauiv + num_dausv
	         + num_ceuv + num_deuiv + num_deusv;
	else {                                            // disc real state
	  size_t num_csv, num_dsiv, num_dssv, num_dsrv;
	  svd.state_counts(num_csv, num_dsiv, num_dssv, num_dsrv);
	  offset = num_cdv  + num_ddiv  + num_ddsv
	         + num_cauv + num_dauiv + num_dausv
	         + num_ceuv + num_deuiv + num_deusv
	         + num_csv  + num_dsiv  + num_dssv;
	}
      }
    }
    break;
  }
  default: // MIXED and RELAXED for single variable types
    offset = vars.cv() + vars.div() + vars.dsv(); break;
  }
  return drv_index + offset;
}


/** maps index within complement of active continuous variables to
    index within all continuous variables. */
size_t NestedModel::ccv_index_map(size_t ccv_index, const Variables& vars)
{
  const SharedVariablesData& svd = vars.shared_data();
  size_t offset, num_cdv, num_ddiv, num_ddsv, num_ddrv;
  svd.design_counts(num_cdv, num_ddiv, num_ddsv, num_ddrv);
  switch (svd.view().first) { // active view
  case MIXED_DESIGN: case RELAXED_DESIGN: // complement is cauv/ceuv/csv
    offset = num_cdv; break;
  case MIXED_ALEATORY_UNCERTAIN: case RELAXED_ALEATORY_UNCERTAIN:
    // complement is cdv/ceuv/csv
    if (ccv_index < num_cdv)
      offset = 0;
    else {
      size_t num_cauv, num_dauiv, num_dausv, num_daurv;
      svd.aleatory_uncertain_counts(num_cauv, num_dauiv, num_dausv, num_daurv);
      offset = num_cauv;
    }
    break;
  case MIXED_EPISTEMIC_UNCERTAIN: case RELAXED_EPISTEMIC_UNCERTAIN: {
    // complement is cdv/cauv/csv
    size_t num_cauv, num_dauiv, num_dausv, num_daurv;
    svd.aleatory_uncertain_counts(num_cauv, num_dauiv, num_dausv, num_daurv);
    if (ccv_index < num_cdv + num_cauv)
      offset = 0;
    else{
      size_t num_ceuv, num_deuiv, num_deusv, num_deurv;
      svd.epistemic_uncertain_counts(num_ceuv, num_deuiv, num_deusv, num_deurv);
      offset = num_ceuv;
    }
    break;
  }
  case MIXED_UNCERTAIN: case RELAXED_UNCERTAIN: // complement is cdv/csv
    if (ccv_index < num_cdv)
      offset = 0;
    else {
      size_t num_cauv, num_dauiv, num_dausv, num_daurv,
             num_ceuv, num_deuiv, num_deusv, num_deurv;
      svd.aleatory_uncertain_counts(num_cauv, num_dauiv, num_dausv, num_daurv);
      svd.epistemic_uncertain_counts(num_ceuv, num_deuiv, num_deusv, num_deurv);
      offset = num_cauv + num_ceuv;
    }
    break;
  case MIXED_STATE: case RELAXED_STATE:
    offset = 0; break;
  default: // MIXED_ALL, RELAXED_ALL
    Cerr << "Error: unsupported active view in NestedModel::ccv_index_map()."
	 << std::endl;
    abort_handler(-1); break;
  }
  return ccv_index + offset;
}


/** maps index within complement of active discrete int variables to
    index within all discrete int variables. */
size_t NestedModel::cdiv_index_map(size_t cdiv_index, const Variables& vars)
{
  const SharedVariablesData& svd = vars.shared_data();
  size_t offset, num_cdv, num_ddiv, num_ddsv, num_ddrv;
  svd.design_counts(num_cdv, num_ddiv, num_ddsv, num_ddrv);
  switch (svd.view().first) { // active view
  case MIXED_DESIGN: case RELAXED_DESIGN: // complement is dauiv/deuiv/dsiv
    offset = num_ddiv; break;
  case MIXED_ALEATORY_UNCERTAIN: case RELAXED_ALEATORY_UNCERTAIN:
    // complement is ddiv/deuiv/dsiv
    if (cdiv_index < num_ddiv)
      offset = 0;
    else {
      size_t num_cauv, num_dauiv, num_dausv, num_daurv;
      svd.aleatory_uncertain_counts(num_cauv, num_dauiv, num_dausv, num_daurv);
      offset = num_dauiv;
    }
    break;
  case MIXED_EPISTEMIC_UNCERTAIN: case RELAXED_EPISTEMIC_UNCERTAIN:
    // complement is ddiv/dauiv/dsiv
    size_t num_cauv, num_dauiv, num_dausv, num_daurv;
    svd.aleatory_uncertain_counts(num_cauv, num_dauiv, num_dausv, num_daurv);
    if (cdiv_index < num_ddiv + num_dauiv)
      offset = 0;
    else {
      size_t num_ceuv, num_deuiv, num_deusv, num_deurv;
      svd.epistemic_uncertain_counts(num_ceuv, num_deuiv, num_deusv, num_deurv);
      offset = num_deuiv;
    }
    break;
  case MIXED_UNCERTAIN: case RELAXED_UNCERTAIN: // complement is ddiv/dsiv
    if (cdiv_index < num_ddiv)
      offset = 0;
    else {
      size_t num_cauv, num_dauiv, num_dausv, num_daurv,
             num_ceuv, num_deuiv, num_deusv, num_deurv;
      svd.aleatory_uncertain_counts(num_cauv, num_dauiv, num_dausv, num_daurv);
      svd.epistemic_uncertain_counts(num_ceuv, num_deuiv, num_deusv, num_deurv);
      offset = num_dauiv + num_deuiv;
    }
    break;
  case MIXED_STATE: case RELAXED_STATE:
    offset = 0; break;
  default: // MIXED_ALL, RELAXED_ALL
    Cerr << "Error: unsupported active view in NestedModel::cdiv_index_map()."
	 << std::endl;
    abort_handler(-1); break;
  }
  return cdiv_index + offset;
}


/** maps index within complement of active discrete string variables to
    index within all discrete string variables. */
size_t NestedModel::cdsv_index_map(size_t cdsv_index, const Variables& vars)
{
  const SharedVariablesData& svd = vars.shared_data();
  size_t offset, num_cdv, num_ddiv, num_ddsv, num_ddrv;
  svd.design_counts(num_cdv, num_ddiv, num_ddsv, num_ddrv);
  switch (svd.view().first) { // active view
  case MIXED_DESIGN: case RELAXED_DESIGN:  // complement is dausv/deusv/dssv
    offset = num_ddsv; break;
  case MIXED_ALEATORY_UNCERTAIN: case RELAXED_ALEATORY_UNCERTAIN:
    // complement is ddsv/deusv/dssv
    if (cdsv_index < num_ddsv)
      offset = 0;
    else {
      size_t num_cauv, num_dauiv, num_dausv, num_daurv;
      svd.aleatory_uncertain_counts(num_cauv, num_dauiv, num_dausv, num_daurv);
      offset = num_dausv;
    }
    break;
  case MIXED_EPISTEMIC_UNCERTAIN: case RELAXED_EPISTEMIC_UNCERTAIN:
    // complement is ddsv/dausv/dssv
    size_t num_cauv, num_dauiv, num_dausv, num_daurv;
    svd.aleatory_uncertain_counts(num_cauv, num_dauiv, num_dausv, num_daurv);
    if (cdsv_index < num_ddsv + num_dausv)
      offset = 0;
    else {
      size_t num_ceuv, num_deuiv, num_deusv, num_deurv;
      svd.epistemic_uncertain_counts(num_ceuv, num_deuiv, num_deusv, num_deurv);
      offset = num_deusv;
    }
    break;
  case MIXED_UNCERTAIN: case RELAXED_UNCERTAIN: // complement is ddsv/dssv
    if (cdsv_index < num_ddsv)
      offset = 0;
    else {
      size_t num_cauv, num_dauiv, num_dausv, num_daurv,
             num_ceuv, num_deuiv, num_deusv, num_deurv;
      svd.aleatory_uncertain_counts(num_cauv, num_dauiv, num_dausv, num_daurv);
      svd.epistemic_uncertain_counts(num_ceuv, num_deuiv, num_deusv, num_deurv);
      offset = num_dausv + num_deusv;
    }
    break;
  case MIXED_STATE: case RELAXED_STATE:
    offset = 0; break;
  default: // MIXED_ALL, RELAXED_ALL
    Cerr << "Error: unsupported active view in NestedModel::cdsv_index_map()."
	 << std::endl;
    abort_handler(-1); break;
  }
  return cdsv_index + offset;
}


/** maps index within complement of active discrete real variables to
    index within all discrete real variables. */
size_t NestedModel::cdrv_index_map(size_t cdrv_index, const Variables& vars)
{
  const SharedVariablesData& svd = vars.shared_data();
  size_t offset, num_cdv, num_ddiv, num_ddsv, num_ddrv;
  svd.design_counts(num_cdv, num_ddiv, num_ddsv, num_ddrv);
  switch (svd.view().first) { // active view
  case MIXED_DESIGN: case RELAXED_DESIGN:  // complement is daurv/deurv/dsrv
    offset = num_ddrv; break;
  case MIXED_ALEATORY_UNCERTAIN: case RELAXED_ALEATORY_UNCERTAIN:
    // complement is ddrv/deurv/dsrv
    if (cdrv_index < num_ddrv)
      offset = 0;
    else {
      size_t num_cauv, num_dauiv, num_dausv, num_daurv;
      svd.aleatory_uncertain_counts(num_cauv, num_dauiv, num_dausv, num_daurv);
      offset = num_daurv;
    }
    break;
  case MIXED_EPISTEMIC_UNCERTAIN: case RELAXED_EPISTEMIC_UNCERTAIN:
    // complement is ddrv/daurv/dsrv
    size_t num_cauv, num_dauiv, num_dausv, num_daurv;
    svd.aleatory_uncertain_counts(num_cauv, num_dauiv, num_dausv, num_daurv);
    if (cdrv_index < num_ddrv + num_daurv)
      offset = 0;
    else {
      size_t num_ceuv, num_deuiv, num_deusv, num_deurv;
      svd.epistemic_uncertain_counts(num_ceuv, num_deuiv, num_deusv, num_deurv);
      offset = num_deurv;
    }
    break;
  case MIXED_UNCERTAIN: case RELAXED_UNCERTAIN: // complement is ddrv/dsrv
    if (cdrv_index < num_ddrv)
      offset = 0;
    else {
      size_t num_cauv, num_dauiv, num_dausv, num_daurv,
             num_ceuv, num_deuiv, num_deusv, num_deurv;
      svd.aleatory_uncertain_counts(num_cauv, num_dauiv, num_dausv, num_daurv);
      svd.epistemic_uncertain_counts(num_ceuv, num_deuiv, num_deusv, num_deurv);
      offset = num_daurv + num_deurv;
    }
    break;
  case MIXED_STATE: case RELAXED_STATE:
    offset = 0; break;
  default: // MIXED_ALL, RELAXED_ALL
    Cerr << "Error: unsupported active view in NestedModel::cdrv_index_map()."
	 << std::endl;
    abort_handler(-1); break;
  }
  return cdrv_index + offset;
}


size_t NestedModel::sm_acv_index_map(size_t pacvm_index, short sacvm_target)
{
  if (sacvm_target == Pecos::CDV_LWR_BND ||
      sacvm_target == Pecos::CDV_UPR_BND ||
      sacvm_target == Pecos::CSV_LWR_BND ||
      sacvm_target == Pecos::CSV_UPR_BND)
    return pacvm_index; // no offset since all_continuous_* used to update
  else {
    const SharedVariablesData& submodel_svd
      = subModel.current_variables().shared_data();
    size_t num_cdv, num_ddiv, num_ddsv, num_ddrv;
    submodel_svd.design_counts(num_cdv, num_ddiv, num_ddsv, num_ddrv);
    Pecos::AleatoryDistParams& submodel_adp
      = subModel.aleatory_distribution_parameters();
    size_t num_nuv = submodel_adp.normal_means().length(),
      num_lnuv  = std::max(submodel_adp.lognormal_means().length(),
			   submodel_adp.lognormal_lambdas().length()),
      num_uuv   = submodel_adp.uniform_lower_bounds().length(),
      num_luuv  = submodel_adp.loguniform_lower_bounds().length(),
      num_tuv   = submodel_adp.triangular_modes().length(),
      num_euv   = submodel_adp.exponential_betas().length(),
      num_beuv  = submodel_adp.beta_alphas().length(),
      num_gauv  = submodel_adp.gamma_alphas().length(),
      num_guuv  = submodel_adp.gumbel_alphas().length(),
      num_fuv   = submodel_adp.frechet_alphas().length();

    size_t dist_index = pacvm_index - num_cdv;
    switch (sacvm_target) {
    case Pecos::N_MEAN:     case Pecos::N_STD_DEV:
    case Pecos::N_LWR_BND:  case Pecos::N_UPR_BND:
    case Pecos::N_LOCATION: case Pecos::N_SCALE:
      break;
    case Pecos::LN_MEAN:     case Pecos::LN_STD_DEV:
    case Pecos::LN_LAMBDA:   case Pecos::LN_ZETA:
    case Pecos::LN_ERR_FACT: case Pecos::LN_LWR_BND: case Pecos::LN_UPR_BND:
      dist_index -= num_nuv; break;
    case Pecos::U_LWR_BND:  case Pecos::U_UPR_BND:
    case Pecos::U_LOCATION: case Pecos::U_SCALE:
      dist_index -= num_nuv + num_lnuv; break;
    case Pecos::LU_LWR_BND: case Pecos::LU_UPR_BND:
      dist_index -= num_nuv + num_lnuv + num_uuv; break;
    case Pecos::T_MODE:     case Pecos::T_LWR_BND:   case Pecos::T_UPR_BND:
    case Pecos::T_LOCATION: case Pecos::T_SCALE:
      dist_index -= num_nuv + num_lnuv + num_uuv + num_luuv; break;
    case Pecos::E_BETA:
      dist_index -= num_nuv + num_lnuv + num_uuv + num_luuv + num_tuv; break;
    case Pecos::BE_ALPHA:   case Pecos::BE_BETA:
    case Pecos::BE_LWR_BND: case Pecos::BE_UPR_BND:
      dist_index -= num_nuv + num_lnuv + num_uuv + num_luuv + num_tuv + num_euv;
      break;
    case Pecos::GA_ALPHA: case Pecos::GA_BETA:
      dist_index -= num_nuv + num_lnuv + num_uuv + num_luuv + num_tuv + num_euv
	+ num_beuv; break;
    case Pecos::GU_ALPHA: case Pecos::GU_BETA:
      dist_index -= num_nuv + num_lnuv + num_uuv + num_luuv + num_tuv + num_euv
	+ num_beuv + num_gauv; break;
    case Pecos::F_ALPHA: case Pecos::F_BETA:
      dist_index -= num_nuv + num_lnuv + num_uuv + num_luuv + num_tuv + num_euv
	+ num_beuv + num_gauv + num_guuv; break;
    case Pecos::W_ALPHA: case Pecos::W_BETA:
      dist_index -= num_nuv + num_lnuv + num_uuv + num_luuv + num_tuv + num_euv
	+ num_beuv + num_gauv + num_guuv + num_fuv; break;
    case Pecos::NO_TARGET: default:
      Cerr << "\nError: secondary mapping target unmatched for continuous "
	   << "variable in NestedModel::sm_acv_index_map()." << std::endl;
      abort_handler(-1); break;
    }
    return dist_index;
  }
}


size_t NestedModel::sm_adiv_index_map(size_t padivm_index, short sadivm_target)
{
  if (sadivm_target == Pecos::DDRIV_LWR_BND ||
      sadivm_target == Pecos::DDRIV_UPR_BND ||
      sadivm_target == Pecos::DSRIV_LWR_BND ||
      sadivm_target == Pecos::DSRIV_UPR_BND)
    return padivm_index; // no offset since all_discrete_int_* used to update
  else {
    const SharedVariablesData& submodel_svd
      = subModel.current_variables().shared_data();
    size_t num_cdv, num_ddiv, num_ddsv, num_ddrv;
    submodel_svd.design_counts(num_cdv, num_ddiv, num_ddsv, num_ddrv);
    Pecos::AleatoryDistParams& submodel_adp
      = subModel.aleatory_distribution_parameters();
    size_t num_puv = submodel_adp.poisson_lambdas().length(),
      num_biuv = submodel_adp.binomial_probability_per_trial().length(),
      num_nbiuv
        = submodel_adp.negative_binomial_probability_per_trial().length(),
      num_geuv = submodel_adp.geometric_probability_per_trial().length();

    size_t dist_index = padivm_index - num_ddiv;
    switch (sadivm_target) {
    case Pecos::P_LAMBDA:                                      break;
    case Pecos::BI_P_PER_TRIAL:  case Pecos::BI_TRIALS:
      dist_index -= num_puv;                                   break;
    case Pecos::NBI_P_PER_TRIAL: case Pecos::NBI_TRIALS:
      dist_index -= num_puv + num_biuv;                        break;
    case Pecos::GE_P_PER_TRIAL: 
      dist_index -= num_puv + num_biuv + num_nbiuv;            break;
    case Pecos::HGE_TOT_POP: case Pecos::HGE_SEL_POP: case Pecos::HGE_FAILED:
      dist_index -= num_puv + num_biuv + num_nbiuv + num_geuv; break;
    case Pecos::NO_TARGET: default:
      Cerr << "\nError: secondary mapping target unmatched for discrete "
	   << "integer variable in NestedModel::sm_adiv_index_map()."
	   << std::endl;
      abort_handler(-1);                                       break;
    }
    return dist_index;
  }
}


size_t NestedModel::sm_adsv_index_map(size_t padsvm_index, short sadsvm_target)
{
  //switch (sadsvm_target) {
  //case Pecos::DDSSV_LWR_BND: case Pecos::DDSSV_UPR_BND:
  //case Pecos::DSSSV_LWR_BND: case Pecos::DSSSV_UPR_BND:
  //  return padsvm_index; break;
  //case Pecos::DAUSV_DISTRIBUTION_PARAMETER:
  //  const SharedVariablesData& submodel_svd
  //    = subModel.current_variables().shared_data();
  //  size_t num_cdv, num_ddiv, num_ddsv, num_ddrv;
  //  submodel_svd.design_counts(num_cdv, num_ddiv, num_ddsv, num_ddrv);
  //  return padsvm_index - num_ddsv;
  //  break;
  //}
  //case Pecos::NO_TARGET: default:
    Cerr << "\nError: secondary mapping target unmatched for discrete string "
	 << "variable in NestedModel::sm_adsv_index_map()." << std::endl;
    abort_handler(-1);
  //}
  return _NPOS;
}


size_t NestedModel::sm_adrv_index_map(size_t padrvm_index, short sadrvm_target)
{
  //switch (sadrvm_target) {
  //case Pecos::DDSRV_LWR_BND: case Pecos::DDSRV_UPR_BND:
  //case Pecos::DSSRV_LWR_BND: case Pecos::DSSRV_UPR_BND:
  //  return padrvm_index; break;
  //case Pecos::DAURV_DISTRIBUTION_PARAMETER:
  //  const SharedVariablesData& submodel_svd
  //    = subModel.current_variables().shared_data();
  //  size_t num_cdv, num_ddiv, num_ddsv, num_ddrv;
  //  submodel_svd.design_counts(num_cdv, num_ddiv, num_ddsv, num_ddrv);
  //  return padrvm_index - num_ddrv;
  //  break;
  //}
  //case Pecos::NO_TARGET: default:
    Cerr << "\nError: secondary mapping target unmatched for discrete real "
	 << "variable in NestedModel::sm_adrv_index_map()." << std::endl;
    abort_handler(-1);
  //}
  return _NPOS;
}


void NestedModel::
real_variable_mapping(const Real& r_var, size_t mapped_index, short svm_target)
{
  Pecos::AleatoryDistParams& submodel_adp
    = subModel.aleatory_distribution_parameters();
  Real dbl_inf = std::numeric_limits<Real>::infinity();

  switch (svm_target) {
  case Pecos::CDV_LWR_BND: case Pecos::CSV_LWR_BND:
    subModel.all_continuous_lower_bound(r_var, mapped_index); break;
  case Pecos::CDV_UPR_BND: case Pecos::CSV_UPR_BND:
    subModel.all_continuous_upper_bound(r_var, mapped_index); break;
  // N_{MEAN,STD_DEV,LWR_BND,UPR_BND} change individual dist parameters only.
  // N_{LOCATION,SCALE} change multiple parameters to accomplish a translation
  // or scaling.  They are mapped to a convenient definition believed to be the
  // most natural for a user, where the definition changes from distribution to
  // distribution (a consistent meaning of mu/sigma would be more awkward for a
  // user to convert).  For Normal, location & scale are mean & std deviation.
  case Pecos::N_MEAN:
    submodel_adp.normal_mean(r_var, mapped_index); break;
  case Pecos::N_STD_DEV:
    submodel_adp.normal_std_deviation(r_var, mapped_index); break;
  case Pecos::N_LWR_BND:
    submodel_adp.normal_lower_bound(r_var, mapped_index); break;
  case Pecos::N_UPR_BND:
    submodel_adp.normal_upper_bound(r_var, mapped_index); break;
  case Pecos::N_LOCATION: { // a translation with no change in shape/scale
    Real mean = submodel_adp.normal_mean(mapped_index), delta = r_var - mean,
	l_bnd = submodel_adp.normal_lower_bound(mapped_index),
	u_bnd = submodel_adp.normal_upper_bound(mapped_index);
    // translate: change bounds by same amount as mean
    submodel_adp.normal_mean(r_var, mapped_index);
    if (l_bnd > -dbl_inf)
      submodel_adp.normal_lower_bound(l_bnd + delta, mapped_index);
    if (u_bnd <  dbl_inf)
      submodel_adp.normal_upper_bound(u_bnd + delta, mapped_index);
    break;
  }
  case Pecos::N_SCALE: { // change in shape/scale without translation
    Real mean = submodel_adp.normal_mean(mapped_index),
        stdev = submodel_adp.normal_std_deviation(mapped_index),
	l_bnd = submodel_adp.normal_lower_bound(mapped_index),
	u_bnd = submodel_adp.normal_upper_bound(mapped_index);
    // scale: preserve number of std deviations where l,u bound occurs
    submodel_adp.normal_std_deviation(r_var, mapped_index);
    if (l_bnd > -dbl_inf) {
      Real num_sig_l = (mean - l_bnd) / stdev;
      submodel_adp.normal_lower_bound(mean - num_sig_l * r_var, mapped_index);
    }
    if (u_bnd <  dbl_inf) {
      Real num_sig_u = (u_bnd - mean) / stdev;
      submodel_adp.normal_upper_bound(mean + num_sig_u * r_var, mapped_index);
    }
    break;
  }
  case Pecos::LN_MEAN:
    submodel_adp.lognormal_mean(r_var, mapped_index); break;
  case Pecos::LN_STD_DEV:
    submodel_adp.lognormal_std_deviation(r_var, mapped_index); break;
  case Pecos::LN_LAMBDA:
    submodel_adp.lognormal_lambda(r_var, mapped_index); break;
  case Pecos::LN_ZETA:
    submodel_adp.lognormal_zeta(r_var, mapped_index); break;
  case Pecos::LN_ERR_FACT:
    submodel_adp.lognormal_error_factor(r_var, mapped_index); break;
  case Pecos::LN_LWR_BND:
    submodel_adp.lognormal_lower_bound(r_var, mapped_index); break;
  case Pecos::LN_UPR_BND:
    submodel_adp.lognormal_upper_bound(r_var, mapped_index); break;
  // U_{LWR_BND,UPR_BND} change individual dist parameters only.
  // U_{LOCATION,SCALE} change multiple parameters to accomplish a translation
  // or scaling.  They are mapped to a convenient definition believed to be the
  // most natural for a user, where the definition changes from distribution to
  // distribution (a consistent meaning of mu/sigma would be more awkward for a
  // user to convert).  For Uniform, location & scale are center & range.
  case Pecos::U_LWR_BND:
    submodel_adp.uniform_lower_bound(r_var, mapped_index); break;
  case Pecos::U_UPR_BND:
    submodel_adp.uniform_upper_bound(r_var, mapped_index); break;
  case Pecos::U_LOCATION: {
    // translate: change both bounds by same amount
    Real l_bnd = submodel_adp.uniform_lower_bound(mapped_index),
	 u_bnd = submodel_adp.uniform_upper_bound(mapped_index),
        center = (u_bnd + l_bnd) / 2., delta = r_var - center;
    submodel_adp.uniform_lower_bound(l_bnd + delta, mapped_index);
    submodel_adp.uniform_upper_bound(u_bnd + delta, mapped_index);
    break;
  }
  case Pecos::U_SCALE: {
    // scale: move bounds in/out by same amount about consistent center
    Real l_bnd = submodel_adp.uniform_lower_bound(mapped_index),
	 u_bnd = submodel_adp.uniform_upper_bound(mapped_index),
	center = (u_bnd + l_bnd) / 2., half_range = r_var / 2.;
    submodel_adp.uniform_lower_bound(center - half_range, mapped_index);
    submodel_adp.uniform_upper_bound(center + half_range, mapped_index);
    break;
  }
  case Pecos::LU_LWR_BND:
    submodel_adp.loguniform_lower_bound(r_var, mapped_index); break;
  case Pecos::LU_UPR_BND:
    submodel_adp.loguniform_upper_bound(r_var, mapped_index); break;
  // T_{MODE,LWR_BND,UPR_BND} change individual dist parameters only.
  // T_{LOCATION,SCALE} change multiple parameters to accomplish a translation
  // or scaling.  They are mapped to a convenient definition believed to be the
  // most natural for a user, where the definition changes from distribution to
  // distribution (a consistent meaning of mu/sigma would be more awkward for a
  // user to convert).  For Triangular, location & scale are mode & range.
  case Pecos::T_MODE:
    submodel_adp.triangular_mode(r_var, mapped_index); break;
  case Pecos::T_LWR_BND:
    submodel_adp.triangular_lower_bound(r_var, mapped_index); break;
  case Pecos::T_UPR_BND:
    submodel_adp.triangular_upper_bound(r_var, mapped_index); break;
  case Pecos::T_LOCATION: {
    // translate: change mode and both bounds by same amount
    Real mode = submodel_adp.triangular_mode(mapped_index),
        l_bnd = submodel_adp.triangular_lower_bound(mapped_index),
        u_bnd = submodel_adp.triangular_upper_bound(mapped_index),
        delta = r_var - mode;
    submodel_adp.triangular_mode(r_var, mapped_index);
    submodel_adp.triangular_lower_bound(l_bnd + delta, mapped_index);
    submodel_adp.triangular_upper_bound(u_bnd + delta, mapped_index);
    break;
  }
  case Pecos::T_SCALE: {
    // scale: preserve L/M/U proportions while scaling range
    Real mode = submodel_adp.triangular_mode(mapped_index),
        l_bnd = submodel_adp.triangular_lower_bound(mapped_index),
	u_bnd = submodel_adp.triangular_upper_bound(mapped_index),
        range = u_bnd - l_bnd, perc_l = (mode - l_bnd) / range,
       perc_u = (u_bnd - mode) / range;
    submodel_adp.triangular_lower_bound(mode - perc_l * r_var, mapped_index);
    submodel_adp.triangular_upper_bound(mode + perc_u * r_var, mapped_index);
    break;
  }
  case Pecos::E_BETA:
    submodel_adp.exponential_beta(r_var, mapped_index); break;
  case Pecos::BE_ALPHA:
    submodel_adp.beta_alpha(r_var, mapped_index); break;
  case Pecos::BE_BETA:
    submodel_adp.beta_beta(r_var, mapped_index); break;
  case Pecos::BE_LWR_BND:
    submodel_adp.beta_lower_bound(r_var, mapped_index); break;
  case Pecos::BE_UPR_BND:
    submodel_adp.beta_upper_bound(r_var, mapped_index); break;
  case Pecos::GA_ALPHA:
    submodel_adp.gamma_alpha(r_var, mapped_index); break;
  case Pecos::GA_BETA:
    submodel_adp.gamma_beta(r_var, mapped_index); break;
  case Pecos::GU_ALPHA:
    submodel_adp.gumbel_alpha(r_var, mapped_index); break;
  case Pecos::GU_BETA:
    submodel_adp.gumbel_beta(r_var, mapped_index); break;
  case Pecos::F_ALPHA:
    submodel_adp.frechet_alpha(r_var, mapped_index); break;
  case Pecos::F_BETA:
    submodel_adp.frechet_beta(r_var, mapped_index); break;
  case Pecos::W_ALPHA:
    submodel_adp.weibull_alpha(r_var, mapped_index); break;
  case Pecos::W_BETA:
    submodel_adp.weibull_beta(r_var, mapped_index); break;
  case Pecos::P_LAMBDA:
    submodel_adp.poisson_lambda(r_var, mapped_index); break;
  case Pecos::BI_P_PER_TRIAL:
    submodel_adp.binomial_probability_per_trial(r_var, mapped_index); break;
  case Pecos::NBI_P_PER_TRIAL:
    submodel_adp.negative_binomial_probability_per_trial(r_var, mapped_index);
    break;
  case Pecos::GE_P_PER_TRIAL:
    submodel_adp.geometric_probability_per_trial(r_var, mapped_index); break;
  case Pecos::NO_TARGET: default:
    Cerr << "\nError: secondary mapping target unmatched for real value "
	 << "insertion in NestedModel::real_variable_mapping()." << std::endl;
    abort_handler(-1);
  }
}


void NestedModel::
integer_variable_mapping(const int& i_var, size_t mapped_index,
			 short svm_target)
{
  Pecos::AleatoryDistParams& submodel_adp
    = subModel.aleatory_distribution_parameters();

  switch (svm_target) {
  case Pecos::DDRIV_LWR_BND: case Pecos::DSRIV_LWR_BND:
    subModel.all_discrete_int_lower_bound(i_var, mapped_index); break;
  case Pecos::DDRIV_UPR_BND: case Pecos::DSRIV_UPR_BND:
    subModel.all_discrete_int_upper_bound(i_var, mapped_index); break;
  case Pecos::BI_TRIALS:
    submodel_adp.binomial_num_trials(i_var, mapped_index); break;
  case Pecos::NBI_TRIALS:
    submodel_adp.negative_binomial_num_trials(i_var, mapped_index); break;
  case Pecos::HGE_TOT_POP:
    submodel_adp.hypergeometric_total_population(i_var, mapped_index); break;
  case Pecos::HGE_SEL_POP:
    submodel_adp.hypergeometric_selected_population(i_var, mapped_index); break;
  case Pecos::HGE_FAILED:
    submodel_adp.hypergeometric_num_drawn(i_var, mapped_index); break;
  case Pecos::NO_TARGET: default:
    Cerr << "\nError: secondary mapping target unmatched for integer value "
	 << "insertion in NestedModel::integer_variable_mapping()" << std::endl;
    abort_handler(-1);
  }
}


void NestedModel::
string_variable_mapping(const String& s_var, size_t mapped_index,
			 short svm_target)
{
  Pecos::AleatoryDistParams& submodel_adp
    = subModel.aleatory_distribution_parameters();

  switch (svm_target) {
  case Pecos::NO_TARGET: default:
    Cerr << "\nError: secondary mapping target unmatched for string value "
	 << "insertion in NestedModel::string_variable_mapping()" << std::endl;
    abort_handler(-1);
  }
}


} // namespace Dakota
