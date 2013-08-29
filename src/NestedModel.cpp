/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
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


namespace Dakota {

NestedModel::NestedModel(ProblemDescDB& problem_db):
  Model(BaseConstructor(), problem_db), nestedModelEvalCntr(0),
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
  // reset at the strategy level (which would be wasteful since the type
  // of derived model may not be known at the strategy level).
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
    optInterfaceResponse = problem_db.get_response(currentVariables);
    optionalInterface    = problem_db.get_interface();
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
      for (i=0; i<numOptInterfIneqCon; ++i)
	if ( interf_ineq_l_bnds[i] > -DBL_MAX || interf_ineq_u_bnds[i] != 0.0 )
	  warning_flag = true;
      for (i=0; i<numOptInterfEqCon; ++i)
	if ( interf_eq_targets[i] != 0.0 )
	  warning_flag = true;
      if (warning_flag)
	Cerr << "Warning: nonlinear constraint bounds and targets in nested "
	     << "model optional interfaces\n         are superceded by "
	     << "composite response constraint bounds and targets."
	     << std::endl;
    }

    // db_responses restore not needed since set_db_list_nodes below will reset
  }

  const String& sub_method_ptr
    = problem_db.get_string("model.nested.sub_method_pointer");
  problem_db.set_db_list_nodes(sub_method_ptr); // even if empty

  subModel = problem_db.get_model();
  //check_submodel_compatibility(subModel); // sanity checks performed below
  // if outer level output is verbose/debug, request fine-grained evaluation 
  // reporting for purposes of the final output summary.  This allows verbose
  // final summaries without verbose output on every sub-iterator completion.
  if (outputLevel > NORMAL_OUTPUT)
    subModel.fine_grained_evaluation_counters();

  // Perform error checks on variable mapping inputs and convert from
  // strings to indices for efficiency at run time.
  size_t i, num_var_map_1 = primary_var_mapping.size(),
    num_var_map_2 = secondary_var_mapping.size(),
    num_curr_cv   = currentVariables.cv(),
    num_curr_div  = currentVariables.div(),
    num_curr_drv  = currentVariables.drv(),
    num_curr_vars = num_curr_cv + num_curr_div + num_curr_drv;
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
  active1ADRVarMapIndices.resize(num_curr_vars);
  extraCVarsData.resize(num_curr_cv);
  extraDIVarsData.resize(num_curr_div);
  extraDRVarsData.resize(num_curr_drv);
  if (num_var_map_2) {
    active2ACVarMapTargets.resize(num_curr_vars);
    active2ADIVarMapTargets.resize(num_curr_vars);
    active2ADRVarMapTargets.resize(num_curr_vars);
  }
  short inactive_sm_view = EMPTY;
  UShortMultiArrayConstView curr_c_types
    = currentVariables.continuous_variable_types();
  UShortMultiArrayConstView curr_di_types
    = currentVariables.discrete_int_variable_types();
  UShortMultiArrayConstView curr_dr_types
    = currentVariables.discrete_real_variable_types();
  UShortMultiArrayConstView submodel_a_c_types
    = subModel.all_continuous_variable_types();
  UShortMultiArrayConstView submodel_a_di_types
    = subModel.all_discrete_int_variable_types();
  UShortMultiArrayConstView submodel_a_dr_types
    = subModel.all_discrete_real_variable_types();
  size_t curr_i, sm_acv_cntr = 0, sm_adiv_cntr = 0, sm_adrv_cntr = 0,
    sm_acv_avail = 0, sm_adiv_avail = 0, sm_adrv_avail = 0;
  unsigned short prev_c_type = USHRT_MAX, prev_di_type = USHRT_MAX,
    prev_dr_type = USHRT_MAX;
  String empty_str;

  // Map ACTIVE CONTINUOUS VARIABLES from currentVariables
  for (i=0; i<num_curr_cv; ++i) {
    curr_i = cv_index_map(i);
    const String& map1
      = (num_var_map_1) ? primary_var_mapping[curr_i] : empty_str;
    if (map1.empty()) {
      // for default mappings between consistent types, propagate bounds/labels
      extraCVarsData[i] = true;
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
      active1ADIVarMapIndices[curr_i] = active1ADRVarMapIndices[curr_i] = _NPOS;
      prev_c_type = curr_c_type;
      if (sm_acv_cntr > sm_acv_avail) {
	Cerr << "\nError: default insertions of type '" << curr_c_type
	     << "' exceed sub-model allocation." << std::endl;
	abort_handler(-1);
      }
      if (num_var_map_2)
	active2ACVarMapTargets[curr_i] = active2ADIVarMapTargets[curr_i]
	  = active2ADRVarMapTargets[curr_i] = Pecos::NO_TARGET;
    }
    else {
      extraCVarsData[i] = false; // not a default mapping based on types
      const String& map2
	= (num_var_map_2) ? secondary_var_mapping[curr_i] : empty_str;
      resolve_real_variable_mapping(map1, map2, curr_i, inactive_sm_view);
    }
  }

  // Map ACTIVE DISCRETE INTEGER VARIABLES from currentVariables
  for (i=0; i<num_curr_div; ++i) {
    curr_i = div_index_map(i);
    const String& map1
      = (num_var_map_1) ? primary_var_mapping[curr_i] : empty_str;
    if (map1.empty()) {
      // for default mappings between consistent types, propagate bounds/labels
      extraDIVarsData[i] = true;
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
      active1ACVarMapIndices[curr_i]  = active1ADRVarMapIndices[curr_i] = _NPOS;
      prev_di_type = curr_di_type;
      if (sm_adiv_cntr > sm_adiv_avail) {
	Cerr << "\nError: default insertions of type '" << curr_di_type
	     << "' exceed sub-model allocation." << std::endl;
	abort_handler(-1);
      }
      if (num_var_map_2)
	active2ACVarMapTargets[curr_i] = active2ADIVarMapTargets[curr_i]
	  = active2ADRVarMapTargets[curr_i] = Pecos::NO_TARGET;
    }
    else {
      extraDIVarsData[i] = false; // not a default mapping based on types
      const String& map2
	= (num_var_map_2) ? secondary_var_mapping[curr_i] : empty_str;
      resolve_integer_variable_mapping(map1, map2, curr_i, inactive_sm_view);
    }
  }

  // Map ACTIVE DISCRETE REAL VARIABLES from currentVariables
  for (i=0; i<num_curr_drv; ++i) {
    curr_i = drv_index_map(i);
    const String& map1
      = (num_var_map_1) ? primary_var_mapping[curr_i] : empty_str;
    if (map1.empty()) {
      // for default mappings between consistent types, propagate bounds/labels
      extraDRVarsData[i] = true;
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
      active1ACVarMapIndices[curr_i]  = active1ADIVarMapIndices[curr_i] = _NPOS;
      prev_dr_type = curr_dr_type;
      if (sm_adrv_cntr > sm_adrv_avail) {
	Cerr << "\nError: default insertions of type '" << curr_dr_type
	     << "' exceed sub-model allocation." << std::endl;
	abort_handler(-1);
      }
      if (num_var_map_2)
	active2ACVarMapTargets[curr_i] = active2ADIVarMapTargets[curr_i]
	  = active2ADRVarMapTargets[curr_i] = Pecos::NO_TARGET;
    }
    else {
      extraDRVarsData[i] = false; // not a default mapping based on types
      const String& map2
	= (num_var_map_2) ? secondary_var_mapping[curr_i] : empty_str;
      resolve_real_variable_mapping(map1, map2, curr_i, inactive_sm_view);
    }
  }

  size_t num_curr_ccv = currentVariables.acv() - num_curr_cv,//active complement
    num_curr_cdiv = currentVariables.adiv() - num_curr_div, // active complement
    num_curr_cdrv = currentVariables.adrv() - num_curr_drv; // active complement
  // complement can be sized based on corresponding currentVariables sizes
  // due to restriction to default mappings
  if (num_curr_ccv)
    complement1ACVarMapIndices.resize(num_curr_ccv);
  if (num_curr_cdiv)
    complement1ADIVarMapIndices.resize(num_curr_cdiv);
  if (num_curr_cdrv)
    complement1ADRVarMapIndices.resize(num_curr_cdrv);
  UShortMultiArrayConstView curr_ac_types
    = currentVariables.all_continuous_variable_types();
  UShortMultiArrayConstView curr_adi_types
    = currentVariables.all_discrete_int_variable_types();
  UShortMultiArrayConstView curr_adr_types
    = currentVariables.all_discrete_real_variable_types();
  unsigned short prev_ac_type = USHRT_MAX, prev_adi_type = USHRT_MAX,
    prev_adr_type = USHRT_MAX;

  // Map COMPLEMENT CONTINUOUS VARIABLES from currentVariables
  for (i=0; i<num_curr_ccv; ++i) {
    curr_i = ccv_index_map(i);
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
    curr_i = cdiv_index_map(i);
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

  // Map COMPLEMENT DISCRETE REAL VARIABLES from currentVariables
  for (i=0; i<num_curr_cdrv; ++i) {
    curr_i = cdrv_index_map(i);
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
       << "ADRV:\n" << active1ADRVarMapIndices
       << "\nactive secondary variable mapping targets:\nACV:\n"
       << active2ACVarMapTargets << "ADIV:\n" << active2ADIVarMapTargets 
       << "ADRV:\n" << active2ADRVarMapTargets
       << "\ncomplement primary variable mapping indices\nACV:\n"
       << complement1ACVarMapIndices << "ADIV:\n" << complement1ADIVarMapIndices
       << "ADRV:\n" << complement1ADRVarMapIndices << '\n';
#endif // DEBUG

  // subModel view updating must be performed before subIterator instantiation
  // since any model recastings will pick up the inactive view (and inactive
  // view differences cause problems with recursion updating).
  if (inactive_sm_view != EMPTY)
    subModel.inactive_view(inactive_sm_view); // recurse

  //if (iteratorCommRank == 0) // only master needs an iterator object
    subIterator = problem_db.get_iterator(subModel);
  subIterator.sub_iterator_flag(true);
  subIterator.active_variable_mappings(active1ACVarMapIndices,
    active1ADIVarMapIndices, active1ADRVarMapIndices, active2ACVarMapTargets,
    active2ADIVarMapTargets, active2ADRVarMapTargets);

  problem_db.set_db_method_node(method_index); // restore method only
  problem_db.set_db_model_nodes(model_index);  // restore all model nodes
  const RealVector& primary_resp_coeffs
    = problem_db.get_rv("model.nested.primary_response_mapping");
  const RealVector& secondary_resp_coeffs
    = problem_db.get_rv("model.nested.secondary_response_mapping");

  if (primary_resp_coeffs.empty() && secondary_resp_coeffs.empty()) {
    Cerr << "\nError: no mappings provided for sub-iterator functions."
	 << std::endl;
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
	   << numSubIterFns << ")." << std::endl;
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
	   << numSubIterFns << ")." << std::endl;
      abort_handler(-1);
    }
    copy_data(secondary_resp_coeffs, secondaryRespCoeffs, 0,(int)numSubIterFns);
  }

  // Back out the number of eq/ineq constraints within secondaryRespCoeffs
  // (subIterator constraints) from the total number of equality/inequality
  // constraints and the number of interface equality/inequality constraints.
  size_t num_mapped_ineq_con
    = problem_db.get_sizet("responses.num_nonlinear_inequality_constraints"),
    num_mapped_eq_con
    = problem_db.get_sizet("responses.num_nonlinear_equality_constraints");
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
  UShortMultiArrayConstView submodel_a_dr_types
    = subModel.all_discrete_real_variable_types();
  Pecos::AleatoryDistParams& submodel_adp
    = subModel.aleatory_distribution_parameters();

  size_t ac_index1 = active1ACVarMapIndices[curr_index]
      = find_index(subModel.all_continuous_variable_labels(), map1),
    adi_index1 = active1ADIVarMapIndices[curr_index]
      = find_index(subModel.all_discrete_int_variable_labels(), map1),
    adr_index1 = active1ADRVarMapIndices[curr_index]
      = find_index(subModel.all_discrete_real_variable_labels(), map1);
  if (ac_index1 == _NPOS && adi_index1 == _NPOS && adr_index1 == _NPOS) {
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
  else if (adr_index1 != _NPOS &&
	   find_index(subModel.discrete_real_variable_labels(), map1) == _NPOS)
    // inactive DRV target
    update_inactive_view(submodel_a_dr_types[adr_index1], inactive_sm_view);

  if (!active2ACVarMapTargets.empty()) {
    if (map2.empty())
      active2ACVarMapTargets[curr_index] = active2ADIVarMapTargets[curr_index]
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
	// TO DO: mean/std deviation or location/scale
	if (map2 == "lower_bound")
	  active2ACVarMapTargets[curr_index] = Pecos::U_LWR_BND;
	else if (map2 == "upper_bound")
	  active2ACVarMapTargets[curr_index] = Pecos::U_UPR_BND;
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
	// TO DO: mean/std deviation or location/scale
	if (map2 == "mode")
	  active2ACVarMapTargets[curr_index] = Pecos::T_MODE;
	else if (map2 == "lower_bound")
	  active2ACVarMapTargets[curr_index] = Pecos::T_LWR_BND;
	else if (map2 == "upper_bound")
	  active2ACVarMapTargets[curr_index] = Pecos::T_UPR_BND;
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
      active2ADIVarMapTargets[curr_index] = active2ADRVarMapTargets[curr_index]
	= Pecos::NO_TARGET;
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
      active2ACVarMapTargets[curr_index] = active2ADRVarMapTargets[curr_index]
	= Pecos::NO_TARGET;
    }
    else if (adr_index1 != _NPOS) {
      unsigned short type = submodel_a_dr_types[adr_index1];
      Cerr << "\nError: " << type << " variable type not supported in "
	   << "secondary real mappings\n       for primary discrete real "
	   << "variable targets." << std::endl;
      abort_handler(-1);
      active2ACVarMapTargets[curr_index] = active2ADIVarMapTargets[curr_index]
	= Pecos::NO_TARGET;
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
  UShortMultiArrayConstView submodel_a_dr_types
    = subModel.all_discrete_real_variable_types();

  size_t ac_index1 = active1ACVarMapIndices[curr_index]
      = find_index(subModel.all_continuous_variable_labels(), map1),
    adi_index1 = active1ADIVarMapIndices[curr_index]
      = find_index(subModel.all_discrete_int_variable_labels(), map1),
    adr_index1 = active1ADRVarMapIndices[curr_index]
      = find_index(subModel.all_discrete_real_variable_labels(), map1);
  if (ac_index1 == _NPOS && adi_index1 == _NPOS && adr_index1 == _NPOS) {
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
  else if (adr_index1 != _NPOS &&
	   find_index(subModel.discrete_real_variable_labels(), map1) == _NPOS)
    // inactive DRV target
    update_inactive_view(submodel_a_dr_types[adr_index1], inactive_sm_view);

  if (!active2ACVarMapTargets.empty()) {
    if (map2.empty())
      active2ACVarMapTargets[curr_index] = active2ADIVarMapTargets[curr_index]
	= active2ADIVarMapTargets[curr_index] = Pecos::NO_TARGET;
    else if (ac_index1 != _NPOS) {
      unsigned short type = submodel_a_c_types[ac_index1];
      Cerr << "\nError: " << type << " variable type not supported in "
	   << "secondary integer mappings\n       for primary continuous "
	   << "variable targets." << std::endl;
      abort_handler(-1);
      active2ADIVarMapTargets[curr_index] = active2ADRVarMapTargets[curr_index]
	= Pecos::NO_TARGET;
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
      active2ACVarMapTargets[curr_index] = active2ADRVarMapTargets[curr_index]
	= Pecos::NO_TARGET;
    }
    else if (adr_index1 != _NPOS) {
      unsigned short type = submodel_a_dr_types[adr_index1];
      Cerr << "\nError: " << type << " variable type not supported in "
	   << "secondary integer mappings\n       for primary discrete real "
	   << "variable targets." << std::endl;
      abort_handler(-1);
      active2ACVarMapTargets[curr_index] = active2ADIVarMapTargets[curr_index]
	= Pecos::NO_TARGET;
    }
  }
}


/** Update subModel's inactive variables with active variables from
    currentVariables, compute the optional interface and sub-iterator
    responses, and map these to the total model response. */
void NestedModel::derived_compute_response(const ActiveSet& set)
{
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
	 << nestedModelEvalCntr+1 << ": performing optional interface mapping\n"
	 << "------------------------------------------------------------------"
	 << '\n';
    component_parallel_mode(OPTIONAL_INTERFACE);
    if (hierarchicalTagging) {
      String eval_tag = evalTagPrefix + '.' + 
	boost::lexical_cast<String>(nestedModelEvalCntr+1);
      optionalInterface.eval_tag_prefix(eval_tag);
    }
    optionalInterface.map(currentVariables, opt_interface_set,
			  optInterfaceResponse);
  }

  if (sub_iterator_map) {
    // need comm set up and master break off (see Strategy::run_iterator())
    Cout << "\n-------------------------------------------------\n"
	 << "NestedModel Evaluation " << std::setw(4) << nestedModelEvalCntr+1 
	 << ": running sub_iterator"
	 << "\n-------------------------------------------------\n";
    component_parallel_mode(SUB_MODEL);
    update_sub_model();
    subIterator.response_results_active_set(sub_iterator_set);
    if (hierarchicalTagging) {
      String eval_tag = evalTagPrefix + '.' + 
	boost::lexical_cast<String>(nestedModelEvalCntr+1);
      subIterator.prepend_evalid(eval_tag);
    }
    // output suppressed by default, unless sub-iterator is verbose:
    subIterator.run_iterator(Cout);
    Cout << "\nActive response data from sub_iterator:\n"
	 << subIterator.response_results() << '\n';
  }
  // Perform mapping of optInterface & subIterator results to currentResponse
  response_mapping(optInterfaceResponse, subIterator.response_results(),
		   currentResponse);

  ++nestedModelEvalCntr;
}


/** Not currently supported by NestedModels (need to add concurrent
    iterator support).  As a result, derived_synchronize() and
    derived_synchronize_nowait() are inactive as well). */
void NestedModel::derived_asynch_compute_response(const ActiveSet& set)
{
  Cerr << "\nError: derived_asynch_compute_response not yet available in "
       << "NestedModel." << std::endl;
  abort_handler(-1);

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
	 << nestedModelEvalCntr+1 << ": performing optional interface mapping\n"
	 << "------------------------------------------------------------------"
	 << '\n';
    // don't need to set component parallel mode since this only queues the job
    optionalInterface.map(currentVariables, opt_interface_set,
			  optInterfaceResponse, true);
  }

  if (sub_iterator_map) {
    // need comm set up and master break off (see Strategy::run_iterator())
    Cout << "\n-------------------------------------------------\n"
	 << "NestedModel Evaluation " << std::setw(4) << nestedModelEvalCntr+1 
	 << ": running sub_iterator"
	 << "\n-------------------------------------------------\n";

    // *** Need concurrent iterator support ***

    // Only makes sense for message passing (can't launch an asynch local 
    // iterator): load up queue of iterator jobs to be scheduled in derived 
    // synchronize fns.
    /*
    subIteratorEvalId++;
    ParamResponsePair current_pair(vars, nestedModelId, response,
                                   subIteratorEvalId);
    beforeSynchPRPQueue.insert(current_pair);
    // jobs are not queued until call to synch() to allow self-scheduling.
    Cout << "(Nested model asynchronous job " << subIteratorEvalId 
         << " added to queue)\n";
    */
  }

  ++nestedModelEvalCntr;
  //varsMap[nestedModelEvalCntr] = currentVariables.copy();
}


/* Asynchronous response computations are not currently supported by
   NestedModels.  Return a dummy to satisfy the compiler. */
//const IntResponseMap& NestedModel::derived_synchronize()
//{
//  Cerr << "\nError: derived_synchronize not yet available in NestedModel."
//       << std::endl;
//  abort_handler(-1);
//
//  varsMap.clear();
//  return responseMap;
//}


/* Asynchronous response computations are not currently supported by
   NestedModels.  Return a dummy to satisfy the compiler. */
//const IntResponseMap& NestedModel::derived_synchronize_nowait()
//{
//  Cerr << "\nError: derived_synchronize_nowait not yet available in "
//       << "NestedModel." << std::endl;
//  abort_handler(-1);
//
//  varsMap.erase(eval_id);
//  return responseMap;
//}


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


/** In the OUU case,
\verbatim
optionalInterface fns = {f}, {g} (deterministic primary functions, constraints)
subIterator fns       = {S}      (UQ response statistics)

Problem formulation for mapped functions:
                  minimize    {f} + [W]{S}
                  subject to  {g_l} <= {g}    <= {g_u}
                              {a_l} <= [A]{S} <= {a_u}
                              {g}    == {g_t}
                              [A]{S} == {a_t}
\endverbatim

where [W] is the primary_mapping_matrix user input (primaryRespCoeffs
class attribute), [A] is the secondary_mapping_matrix user input
(secondaryRespCoeffs class attribute), {{g_l},{a_l}} are the top level
inequality constraint lower bounds, {{g_u},{a_u}} are the top level
inequality constraint upper bounds, and {{g_t},{a_t}} are the top
level equality constraint targets.

NOTE: optionalInterface/subIterator primary fns (obj/lsq/generic fns)
overlap but optionalInterface/subIterator secondary fns (ineq/eq 
constraints) do not.  The [W] matrix can be specified so as to allow

\li some purely deterministic primary functions and some combined:
    [W] filled and [W].num_rows() < {f}.length() [combined first] 
    \e or [W].num_rows() == {f}.length() and [W] contains rows of 
    zeros [combined last]
\li some combined and some purely stochastic primary functions:
    [W] filled and [W].num_rows() > {f}.length()
\li separate deterministic and stochastic primary functions:
    [W].num_rows() > {f}.length() and [W] contains {f}.length()
    rows of zeros.

If the need arises, could change constraint definition to allow overlap
as well: {g_l} <= {g} + [A]{S} <= {g_u} with [A] usage the same as for
[W] above.

In the UOO case, things are simpler, just compute statistics of each 
optimization response function: [W] = [I], {f}/{g}/[A] are empty. */
void NestedModel::response_mapping(const Response& opt_interface_response,
				   const Response& sub_iterator_response,
				   Response& mapped_response)
{
  // mapped data initialization
  const ShortArray& mapped_asv = mapped_response.active_set_request_vector();
  const SizetArray& mapped_dvv = mapped_response.active_set_derivative_vector();
  size_t i, j, k, l, m_index, oi_index, num_mapped_fns = mapped_asv.size(),
    num_mapped_deriv_vars = mapped_dvv.size();
  bool grad_flag = false, hess_flag = false;
  for (i=0; i<num_mapped_fns; ++i) {
    if (mapped_asv[i] & 2)
      grad_flag = true;
    if (mapped_asv[i] & 4)
      hess_flag = true;
  }
  // Sanity check on derivative compatibility: (1) the optional interface
  // response must have the same DVV, (2) the derivatives in the sub-iterator
  // response must be with respect to the same variables; but since the
  // numbering may be different following insertion/augmentation, only the
  // DVV length is verified.
  if ( (grad_flag || hess_flag) &&
       (sub_iterator_response.active_set_derivative_vector().size() !=
	num_mapped_deriv_vars || (!optInterfacePointer.empty() &&
	opt_interface_response.active_set_derivative_vector() != mapped_dvv))) {
    Cerr << "\nError: derivative variables vector mismatch in NestedModel::"
         << "response_mapping()." << std::endl;
    abort_handler(-1);
  }

  // counter initialization & sanity checking
  size_t num_opt_interf_fns     = (optInterfacePointer.empty()) ? 0 :
                                  opt_interface_response.num_functions(),
    num_opt_interf_con          = numOptInterfIneqCon + numOptInterfEqCon,
    num_sub_iter_mapped_primary = primaryRespCoeffs.numRows(),
    num_sub_iter_mapped_con     = secondaryRespCoeffs.numRows(),
    num_mapped_primary          = std::max(numOptInterfPrimary, 
					   num_sub_iter_mapped_primary);
  // NOTE: numSubIterFns != num_sub_iter_mapped_primary+num_sub_iter_mapped_con
  // since subIterator response is converted to sub_iter_mapped_primary/con
  // through the action of [W] and [A].
  if (num_opt_interf_fns      != numOptInterfPrimary + num_opt_interf_con
   || num_sub_iter_mapped_con != numSubIterMappedIneqCon + numSubIterMappedEqCon
   || num_mapped_fns          != num_mapped_primary + num_opt_interf_con +
                                 num_sub_iter_mapped_con) {
    Cerr << "\nError: bad function counts in NestedModel::response_mapping()."
         << std::endl;
    abort_handler(-1);
  }

  // build mapped response data:

  mapped_response.reset_inactive();
  RealVector mapped_vals = mapped_response.function_values_view();
  RealMatrix empty_rm; RealSymMatrixArray empty_rma;
  const RealVector& sub_iterator_vals
    = sub_iterator_response.function_values();
  const RealMatrix& sub_iterator_grads
    = (grad_flag) ? sub_iterator_response.function_gradients() : empty_rm;
  const RealSymMatrixArray& sub_iterator_hessians
    = (hess_flag) ? sub_iterator_response.function_hessians()  : empty_rma;

  // {f} + [W]{S}:
  for (i=0; i<num_mapped_primary; ++i) {
    if (mapped_asv[i] & 1) { // mapped_vals
      mapped_vals[i] = (i<numOptInterfPrimary) ?
	opt_interface_response.function_value(i) : 0.; // {f}
      if (i<num_sub_iter_mapped_primary) {
        Real& inner_prod = mapped_vals[i];
        for (j=0; j<numSubIterFns; ++j)
          inner_prod += primaryRespCoeffs(i,j)*sub_iterator_vals[j]; // [W]{S}
      }
    }
    if (mapped_asv[i] & 2) { // mapped_grads
      RealVector mapped_grad = mapped_response.function_gradient_view(i);
      if (i<numOptInterfPrimary)
	copy_data(opt_interface_response.function_gradient(i),
		  (int)num_mapped_deriv_vars, mapped_grad); // {f}
      else
	mapped_grad = 0.;
      if (i<num_sub_iter_mapped_primary) {
        for (j=0; j<num_mapped_deriv_vars; ++j) {
          Real& inner_prod = mapped_grad[j]; // [W]{S}
          for (k=0; k<numSubIterFns; ++k)
            inner_prod += primaryRespCoeffs(i,k)*sub_iterator_grads(j,k);
	}
      }
    }
    if (mapped_asv[i] & 4) { // mapped_hessians
      RealSymMatrix mapped_hess = mapped_response.function_hessian_view(i);
      if (i<numOptInterfPrimary)
        mapped_hess.assign(opt_interface_response.function_hessian(i)); // {f}
      else
	mapped_hess = 0.;
      if (i<num_sub_iter_mapped_primary) {
        for (j=0; j<num_mapped_deriv_vars; ++j) {
          for (k=0; k<=j; ++k) {
            Real& inner_prod = mapped_hess(j,k); // [W]{S}
            for (l=0; l<numSubIterFns; ++l)
              inner_prod += primaryRespCoeffs(i,l)
                         *  sub_iterator_hessians[l](j,k);
	  }
	}
      }
    }
  }

  // {g}:
  for (i=0; i<num_opt_interf_con; ++i) {
    oi_index = i+numOptInterfPrimary;
    m_index  = i + num_mapped_primary; // {g_l} <= {g} <= {g_u}
    if (i>=numOptInterfIneqCon)     // {g} == {g_t}
      m_index += numOptInterfIneqCon + numSubIterMappedIneqCon;
    if (mapped_asv[m_index] & 1) // mapped_vals
      mapped_vals[m_index] = opt_interface_response.function_value(oi_index);
    if (mapped_asv[m_index] & 2) // mapped_grads
      mapped_response.function_gradient(
	opt_interface_response.function_gradient_view(oi_index), m_index);
    if (mapped_asv[m_index] & 4) // mapped_hessians
      mapped_response.function_hessian(
	opt_interface_response.function_hessian(oi_index), m_index);
  }

  // [A]{S}:
  for (i=0; i<num_sub_iter_mapped_con; ++i) {
    m_index = i + num_mapped_primary;
    if (i<numSubIterMappedIneqCon) // {a_l} <= [A]{S} <= {a_u}
      m_index += numOptInterfIneqCon;
    else                           // [A]{S} == {a_t}
      m_index += num_opt_interf_con + numSubIterMappedIneqCon;
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

  Cout << "\n---------------------------\nNestedModel total response:"
       << "\n---------------------------\n\nActive response data from nested "
       << "mapping:\n" << mapped_response << '\n';
}


void NestedModel::component_parallel_mode(short mode)
{
  // mode may be correct, but can't guarantee active parallel configuration is
  // in synch
  //if (componentParallelMode == mode)
  //  return; // already in correct parallel mode

  // terminate previous serve mode (if active)
  if (componentParallelMode != mode) {
    if (componentParallelMode == OPTIONAL_INTERFACE) {
      parallelLib.parallel_configuration_iterator(modelPCIter);
      const ParallelConfiguration& pc = parallelLib.parallel_configuration();
      if (parallelLib.si_parallel_level_defined() && 
	  pc.si_parallel_level().server_communicator_size() > 1)
	optionalInterface.stop_evaluation_servers();
    }
    else if (componentParallelMode == SUB_MODEL) {
      parallelLib.parallel_configuration_iterator(
        subModel.parallel_configuration_iterator());
      const ParallelConfiguration& pc = parallelLib.parallel_configuration();
      if (parallelLib.si_parallel_level_defined() && 
	  pc.si_parallel_level().server_communicator_size() > 1)
	subModel.stop_servers();
    }
  }

  // set ParallelConfiguration for new mode
  if (mode == SUB_MODEL)
    parallelLib.parallel_configuration_iterator(
      subModel.parallel_configuration_iterator());
  else if (mode == OPTIONAL_INTERFACE)
    parallelLib.parallel_configuration_iterator(modelPCIter);

  // retrieve new ParallelConfiguration data
  int new_iter_comm_size = 1;
  if (parallelLib.si_parallel_level_defined()) {
    const ParallelConfiguration& pc = parallelLib.parallel_configuration();
    new_iter_comm_size = pc.si_parallel_level().server_communicator_size();
  }

  // activate the new serve mode
  if (new_iter_comm_size > 1 && componentParallelMode != mode)
    parallelLib.bcast_i(mode);
  componentParallelMode = mode;
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
  else if (view == EMPTY)
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


void NestedModel::update_sub_model()
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
  size_t curr_i, num_curr_cv = currentVariables.cv();
  const RealVector& curr_c_vars = currentVariables.continuous_variables();
  const RealVector& curr_c_l_bnds
    = userDefinedConstraints.continuous_lower_bounds();
  const RealVector& curr_c_u_bnds
    = userDefinedConstraints.continuous_upper_bounds();
  StringMultiArrayConstView curr_c_labels
    = currentVariables.continuous_variable_labels();
  for (i=0; i<num_curr_cv; ++i) {
    curr_i = cv_index_map(i);
    size_t pacvm_index = active1ACVarMapIndices[curr_i],
      padivm_index = active1ADIVarMapIndices[curr_i],
      padrvm_index = active1ADRVarMapIndices[curr_i];
    if (pacvm_index != _NPOS) {
      short sacvm_target
	= (num_var_map_2) ? active2ACVarMapTargets[curr_i] : Pecos::NO_TARGET;
      if (sacvm_target == Pecos::NO_TARGET) {
	subModel.all_continuous_variable(curr_c_vars[i], pacvm_index);
	if (extraCVarsData[i]) {
	  subModel.all_continuous_lower_bound(curr_c_l_bnds[i], pacvm_index);
	  subModel.all_continuous_upper_bound(curr_c_u_bnds[i], pacvm_index);
	  if (!nestedModelEvalCntr)
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
    else if (padrvm_index != _NPOS) {
      short sadrvm_target = (num_var_map_2) ? active2ADRVarMapTargets[curr_i]
	: Pecos::NO_TARGET;
      size_t mapped_index = sm_adrv_index_map(padrvm_index, sadrvm_target);
      real_variable_mapping(curr_c_vars[i], mapped_index, sadrvm_target);
    }
  }

  // Map ACTIVE DISCRETE INTEGER VARIABLES from currentVariables
  size_t num_curr_div = currentVariables.div();
  const IntVector& curr_di_vars = currentVariables.discrete_int_variables();
  const IntVector& curr_di_l_bnds
    = userDefinedConstraints.discrete_int_lower_bounds();
  const IntVector& curr_di_u_bnds
    = userDefinedConstraints.discrete_int_upper_bounds();
  StringMultiArrayConstView curr_di_labels
    = currentVariables.discrete_int_variable_labels();
  for (i=0; i<num_curr_div; ++i) {
    curr_i = div_index_map(i);
    size_t pacvm_index = active1ACVarMapIndices[curr_i],
      padivm_index = active1ADIVarMapIndices[curr_i],
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
	  if (!nestedModelEvalCntr)
	    subModel.all_discrete_int_variable_label(curr_di_labels[i],
						     padivm_index);
	}
      }
      else {
	size_t mapped_index = sm_adiv_index_map(padivm_index, sadivm_target);
	integer_variable_mapping(curr_di_vars[i], mapped_index, sadivm_target);
      }
    }
    else if (padrvm_index != _NPOS) {
      short sadrvm_target = (num_var_map_2) ?
	active2ADRVarMapTargets[curr_i] : Pecos::NO_TARGET;
      size_t mapped_index = sm_adrv_index_map(padrvm_index, sadrvm_target);
      integer_variable_mapping(curr_di_vars[i], mapped_index, sadrvm_target);
    }
  }

  // Map ACTIVE DISCRETE REAL VARIABLES from currentVariables
  size_t num_curr_drv = currentVariables.drv();
  const RealVector& curr_dr_vars = currentVariables.discrete_real_variables();
  const RealVector& curr_dr_l_bnds
    = userDefinedConstraints.discrete_real_lower_bounds();
  const RealVector& curr_dr_u_bnds
    = userDefinedConstraints.discrete_real_upper_bounds();
  StringMultiArrayConstView curr_dr_labels
    = currentVariables.discrete_real_variable_labels();
  for (i=0; i<num_curr_drv; ++i) {
    curr_i = drv_index_map(i);
    size_t pacvm_index  = active1ACVarMapIndices[curr_i],
      padivm_index = active1ADIVarMapIndices[curr_i],
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
	  if (!nestedModelEvalCntr)
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
  size_t num_curr_ccv = currentVariables.acv() - num_curr_cv;
  const RealVector& curr_ac_vars
    = currentVariables.all_continuous_variables();
  const RealVector& curr_ac_l_bnds
    = userDefinedConstraints.all_continuous_lower_bounds();
  const RealVector& curr_ac_u_bnds
    = userDefinedConstraints.all_continuous_upper_bounds();
  StringMultiArrayConstView curr_ac_labels
    = currentVariables.all_continuous_variable_labels();
  for (i=0; i<num_curr_ccv; ++i) {
    curr_i = ccv_index_map(i);
    size_t c1_index = complement1ACVarMapIndices[i];
    subModel.all_continuous_variable(curr_ac_vars[curr_i], c1_index);
    subModel.all_continuous_lower_bound(curr_ac_l_bnds[curr_i], c1_index);
    subModel.all_continuous_upper_bound(curr_ac_u_bnds[curr_i], c1_index);
    if (!nestedModelEvalCntr)
      subModel.all_continuous_variable_label(curr_ac_labels[curr_i], c1_index);
  }

  // Map COMPLEMENT DISCRETE INTEGER VARIABLES from currentVariables into
  // corresponding subModel type (using same logic as default active mapping)
  size_t num_curr_cdiv = currentVariables.adiv() - num_curr_div;
  const IntVector& curr_adi_vars
    = currentVariables.all_discrete_int_variables();
  const IntVector& curr_adi_l_bnds
    = userDefinedConstraints.all_discrete_int_lower_bounds();
  const IntVector& curr_adi_u_bnds
    = userDefinedConstraints.all_discrete_int_upper_bounds();
  StringMultiArrayConstView curr_adi_labels
    = currentVariables.all_discrete_int_variable_labels();
  for (i=0; i<num_curr_cdiv; ++i) {
    curr_i = cdiv_index_map(i);
    size_t c1_index = complement1ADIVarMapIndices[i];
    subModel.all_discrete_int_variable(curr_adi_vars[curr_i], c1_index);
    subModel.all_discrete_int_lower_bound(curr_adi_l_bnds[curr_i], c1_index);
    subModel.all_discrete_int_upper_bound(curr_adi_u_bnds[curr_i], c1_index);
    if (!nestedModelEvalCntr)
      subModel.all_discrete_int_variable_label(curr_adi_labels[curr_i],
					       c1_index);
  }

  // Map COMPLEMENT DISCRETE REAL VARIABLES from currentVariables into
  // corresponding subModel type (using same logic as default active mapping)
  size_t num_curr_cdrv = currentVariables.adrv() - num_curr_drv;
  const RealVector& curr_adr_vars
    = currentVariables.all_discrete_real_variables();
  const RealVector& curr_adr_l_bnds
    = userDefinedConstraints.all_discrete_real_lower_bounds();
  const RealVector& curr_adr_u_bnds
    = userDefinedConstraints.all_discrete_real_upper_bounds();
  StringMultiArrayConstView curr_adr_labels
    = currentVariables.all_discrete_real_variable_labels();
  for (i=0; i<num_curr_cdrv; ++i) {
    curr_i = cdrv_index_map(i);
    size_t c1_index = complement1ADRVarMapIndices[i];
    subModel.all_discrete_real_variable(curr_adr_vars[curr_i], c1_index);
    subModel.all_discrete_real_lower_bound(curr_adr_l_bnds[curr_i], c1_index);
    subModel.all_discrete_real_upper_bound(curr_adr_u_bnds[curr_i], c1_index);
    if (!nestedModelEvalCntr)
      subModel.all_discrete_real_variable_label(curr_adr_labels[curr_i],
						c1_index);
  }
}


/** maps index within active continuous variables to index within
    aggregated active continuous/discrete-int/discrete-real variables. */
size_t NestedModel::cv_index_map(size_t cv_index)
{
  size_t offset;
  switch (currentVariables.view().first) { // active view
  case MIXED_UNCERTAIN: {
    const SizetArray& vc_totals
      = currentVariables.variables_components_totals();
    size_t num_cauv = vc_totals[3], num_dauv = vc_totals[4] + vc_totals[5];
    //  active cv order is cauv,ceuv;
    // aggregated order is cauv/dauiv/daurv,ceuv/deuiv/deurv:
    offset = (cv_index < num_cauv) ? 0 : num_dauv;
    break;
  }
  case MIXED_ALL: {
    const SizetArray& vc_totals
      = currentVariables.variables_components_totals();
    size_t num_cdv = vc_totals[0], num_cauv = vc_totals[3],
      num_ceuv = vc_totals[6], num_ddv  = vc_totals[1] + vc_totals[2],
      num_dauv = vc_totals[4] + vc_totals[5],
      num_deuv = vc_totals[7] + vc_totals[8];
    // active cv order is cdv,cauv,ceuv,csv; aggregated order is
    // cdv/ddiv/ddrv,cauv/dauiv/daurv,ceuv/deuiv/deurv,csv/dsiv/dsrv:
    if (cv_index < num_cdv)                            // continuous design
      offset = 0;
    else if (cv_index < num_cdv + num_cauv)            // continuous aleatory
      offset = num_ddv;
    else if (cv_index < num_cdv + num_cauv + num_ceuv) // continuous epistemic
      offset = num_ddv + num_dauv;
    else                                               // continuous state
      offset = num_ddv + num_dauv + num_deuv;
    break;
  }
  default: // MIXED for single variable types, RELAXED for all variable types
    offset = 0; break;
  }
  return cv_index + offset;
}


/** maps index within active discrete int variables to index within
    aggregated active continuous/discrete-int/discrete-real variables. */
size_t NestedModel::div_index_map(size_t div_index)
{
  size_t offset;
  switch (currentVariables.view().first) { // active view
  case MIXED_UNCERTAIN: {
    const SizetArray& vc_totals
      = currentVariables.variables_components_totals();
    size_t num_cauv  = vc_totals[3], num_dauiv = vc_totals[4],
           num_daurv = vc_totals[5], num_ceuv  = vc_totals[6];
    // active div order is dauiv/deuiv
    // aggregated order is cauv/dauiv/daurv,ceuv/deuiv/deurv:
    offset = (div_index < num_dauiv) ? num_cauv : // discrete int aleatory
      num_cauv + num_daurv + num_ceuv;            // discrete int epistemic
    break;
  }
  case MIXED_ALL: {
    const SizetArray& vc_totals
      = currentVariables.variables_components_totals();
    size_t num_cdv = vc_totals[0], num_ddiv  = vc_totals[1],
      num_ddrv  = vc_totals[2],    num_cauv  = vc_totals[3],
      num_dauiv = vc_totals[4],    num_daurv = vc_totals[5],
      num_ceuv  = vc_totals[6],    num_deuiv = vc_totals[7],
      num_deurv = vc_totals[8],    num_csv   = vc_totals[9];
    // active div order is ddiv,dauiv,deuiv,dsiv; aggregated order is
    // cdv/ddiv/ddrv,cauv/dauiv/daurv,ceuv/deuiv/deurv,csv/dsiv/dsrv:
    if (div_index < num_ddiv)                              // disc int design
      offset = num_cdv;
    else if (div_index < num_ddiv + num_dauiv)             // disc int aleatory
      offset = num_cdv + num_ddrv + num_cauv;
    else if (div_index < num_ddiv + num_dauiv + num_deuiv) // disc int epistemic
      offset = num_cdv + num_ddrv + num_cauv + num_daurv + num_ceuv;
    else                                                   // disc int state
      offset = num_cdv + num_ddrv + num_cauv + num_daurv + num_ceuv + num_deurv
	     + num_csv;
    break;
  }
  default: // MIXED for single variable types; RELAXED should not occur
    offset = currentVariables.cv(); break;
  }
  return div_index + offset;
}


/** maps index within active discrete real variables to index within
    aggregated active continuous/discrete-int/discrete-real variables. */
size_t NestedModel::drv_index_map(size_t drv_index)
{
  size_t offset;
  switch (currentVariables.view().first) { // active view
  case MIXED_UNCERTAIN: {
    const SizetArray& vc_totals
      = currentVariables.variables_components_totals();
    size_t num_cauv = vc_totals[3], num_dauiv = vc_totals[4],
      num_daurv = vc_totals[5], num_ceuv = vc_totals[6],
      num_deuiv = vc_totals[7];
    // active drv order is daurv/deurv
    // aggregated order is cauv/dauiv/daurv,ceuv/deuiv/deurv:
    offset = (drv_index < num_daurv) ? num_cauv + num_dauiv : // dr aleatory
      num_cauv + num_dauiv + num_ceuv + num_deuiv;            // dr epistemic
    break;
  }
  case MIXED_ALL: {
    const SizetArray& vc_totals
      = currentVariables.variables_components_totals();
    size_t num_cdv = vc_totals[0], num_ddiv  = vc_totals[1],
      num_ddrv  = vc_totals[2],    num_cauv  = vc_totals[3],
      num_dauiv = vc_totals[4],    num_daurv = vc_totals[5],
      num_ceuv  = vc_totals[6],    num_deuiv = vc_totals[7],
      num_deurv = vc_totals[8],    num_csv   = vc_totals[9],
      num_dsiv  = vc_totals[10];
    // active drv order is ddrv,daurv,deurv,dsrv; aggregated order is
    // cdv/ddiv/ddrv,cauv/dauiv/daurv,ceuv/deuiv/deurv,csv/dsiv/dsrv:
    if (drv_index < num_ddrv)                             // disc real design
      offset = num_cdv + num_ddiv;
    else if (drv_index < num_ddrv + num_daurv)            // disc real aleatory
      offset = num_cdv + num_ddiv + num_cauv + num_dauiv;
    else if (drv_index < num_ddrv + num_daurv + num_deurv)// disc real epistemic
      offset = num_cdv + num_ddiv + num_cauv + num_dauiv + num_ceuv + num_deuiv;
    else                                                  // disc real state
      offset = num_cdv + num_ddiv + num_cauv + num_dauiv + num_ceuv + num_deuiv
	     + num_csv + num_dsiv;
    break;
  }
  default: // MIXED for single variable types; RELAXED should not occur
    offset = currentVariables.cv() + currentVariables.div(); break;
  }
  return drv_index + offset;
}


/** maps index within complement of active continuous variables to
    index within all continuous variables. */
size_t NestedModel::ccv_index_map(size_t ccv_index)
{
  size_t offset;
  const SizetArray& vc_totals = currentVariables.variables_components_totals();
  size_t num_cdv = vc_totals[0];
  switch (currentVariables.view().first) { // active view
  case MIXED_DESIGN:  // complement is cauv/ceuv/csv
    offset = num_cdv; break;
  case MIXED_ALEATORY_UNCERTAIN: { // complement is cdv/ceuv/csv
    size_t num_cauv = vc_totals[3];
    offset = (ccv_index < num_cdv) ? 0 : num_cauv; break;
  }
  case MIXED_EPISTEMIC_UNCERTAIN: { // complement is cdv/cauv/csv
    size_t num_cauv = vc_totals[3], num_ceuv = vc_totals[6];
    offset = (ccv_index < num_cdv+num_cauv) ? 0 : num_ceuv; break;
  }
  case MIXED_UNCERTAIN: { // complement is cdv/csv
    size_t num_cauv = vc_totals[3], num_ceuv = vc_totals[6];
    offset = (ccv_index < num_cdv) ? 0 : num_cauv+num_ceuv; break;
  }
  case RELAXED_DESIGN: { // complement is auv/euv/sv
    size_t num_dv = num_cdv + vc_totals[1] + vc_totals[2];
    offset = num_dv; break;
  }
  case RELAXED_ALEATORY_UNCERTAIN: { // complement is dv/euv/sv
    size_t num_dv  = num_cdv + vc_totals[1] + vc_totals[2],
           num_auv = vc_totals[3] + vc_totals[4] + vc_totals[5];
    offset = (ccv_index < num_dv) ? 0 : num_auv; break;
  }
  case RELAXED_EPISTEMIC_UNCERTAIN: { // complement is dv/auv/sv
    size_t num_dv  = num_cdv + vc_totals[1] + vc_totals[2],
           num_auv = vc_totals[3] + vc_totals[4] + vc_totals[5],
           num_euv = vc_totals[6] + vc_totals[7] + vc_totals[8];
    offset = (ccv_index < num_dv+num_auv) ? 0 : num_euv; break;
  }
  case RELAXED_UNCERTAIN: { // complement is dv/sv
    size_t num_dv  = num_cdv + vc_totals[1] + vc_totals[2],
           num_auv = vc_totals[3] + vc_totals[4] + vc_totals[5],
           num_euv = vc_totals[6] + vc_totals[7] + vc_totals[8];
    offset = (ccv_index < num_dv) ? 0 : num_auv+num_euv; break;
  }
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
size_t NestedModel::cdiv_index_map(size_t cdiv_index)
{
  size_t offset;
  const SizetArray& vc_totals = currentVariables.variables_components_totals();
  size_t num_ddiv = vc_totals[1];
  switch (currentVariables.view().first) { // active view
  case MIXED_DESIGN:  // complement is dauiv/deuiv/dsiv
    offset = num_ddiv; break;
  case MIXED_ALEATORY_UNCERTAIN: { // complement is ddiv/deuiv/dsiv
    size_t num_dauiv = vc_totals[4];
    offset = (cdiv_index < num_ddiv) ? 0 : num_dauiv; break;
  }
  case MIXED_EPISTEMIC_UNCERTAIN: { // complement is ddiv/dauiv/dsiv
    size_t num_dauiv = vc_totals[4], num_deuiv = vc_totals[7];
    offset = (cdiv_index < num_ddiv+num_dauiv) ? 0 : num_deuiv; break;
  }
  case MIXED_UNCERTAIN: { // complement is ddiv/dsiv
    size_t num_dauiv = vc_totals[4], num_deuiv = vc_totals[7];
    offset = (cdiv_index < num_ddiv) ? 0 : num_dauiv+num_deuiv; break;
  }
  case MIXED_STATE:
    offset = 0; break;
  default: // MIXED_ALL, RELAXED_*
    Cerr << "Error: unsupported active view in NestedModel::cdiv_index_map()."
	 << std::endl;
    abort_handler(-1); break;
  }
  return cdiv_index + offset;
}


/** maps index within complement of active discrete real variables to
    index within all discrete real variables. */
size_t NestedModel::cdrv_index_map(size_t cdrv_index)
{
  size_t offset;
  const SizetArray& vc_totals = currentVariables.variables_components_totals();
  size_t num_ddrv = vc_totals[2];
  switch (currentVariables.view().first) { // active view
  case MIXED_DESIGN:  // complement is daurv/deurv/dsrv
    offset = num_ddrv; break;
  case MIXED_ALEATORY_UNCERTAIN: { // complement is ddrv/deurv/dsrv
    size_t num_daurv = vc_totals[5];
    offset = (cdrv_index < num_ddrv) ? 0 : num_daurv; break;
  }
  case MIXED_EPISTEMIC_UNCERTAIN: { // complement is ddrv/daurv/dsrv
    size_t num_daurv = vc_totals[5], num_deurv = vc_totals[8];
    offset = (cdrv_index < num_ddrv+num_daurv) ? 0 : num_deurv; break;
  }
  case MIXED_UNCERTAIN: { // complement is ddrv/dsrv
    size_t num_daurv = vc_totals[5], num_deurv = vc_totals[8];
    offset = (cdrv_index < num_ddrv) ? 0 : num_daurv+num_deurv; break;
  }
  case MIXED_STATE:
    offset = 0; break;
  default: // MIXED_ALL, RELAXED_*
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
    UShortMultiArrayConstView acv_types
      = subModel.all_continuous_variable_types();
    Pecos::AleatoryDistParams& submodel_adp
      = subModel.aleatory_distribution_parameters();
    size_t num_cdv = std::count(acv_types.begin(), acv_types.end(),
				(unsigned short)CONTINUOUS_DESIGN),
      num_nuv   = submodel_adp.normal_means().length(),
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
    case Pecos::N_MEAN:    case Pecos::N_STD_DEV:
    case Pecos::N_LWR_BND: case Pecos::N_UPR_BND:
      break;
    case Pecos::LN_MEAN:     case Pecos::LN_STD_DEV:
    case Pecos::LN_LAMBDA:   case Pecos::LN_ZETA:
    case Pecos::LN_ERR_FACT: case Pecos::LN_LWR_BND: case Pecos::LN_UPR_BND:
      dist_index -= num_nuv; break;
    case Pecos::U_LWR_BND: case Pecos::U_UPR_BND:
      dist_index -= num_nuv + num_lnuv; break;
    case Pecos::LU_LWR_BND: case Pecos::LU_UPR_BND:
      dist_index -= num_nuv + num_lnuv + num_uuv; break;
    case Pecos::T_MODE: case Pecos::T_LWR_BND: case Pecos::T_UPR_BND:
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
    UShortMultiArrayConstView adiv_types
      = subModel.all_discrete_int_variable_types();
    Pecos::AleatoryDistParams& submodel_adp
      = subModel.aleatory_distribution_parameters();
    size_t num_ddriv = std::count(adiv_types.begin(), adiv_types.end(),
				  (unsigned short)DISCRETE_DESIGN_RANGE),
      num_ddsiv = std::count(adiv_types.begin(), adiv_types.end(),
			     (unsigned short)DISCRETE_DESIGN_SET_INT),
      num_puv   = submodel_adp.poisson_lambdas().length(),
      num_biuv  = submodel_adp.binomial_probability_per_trial().length(),
      num_nbiuv
        = submodel_adp.negative_binomial_probability_per_trial().length(),
      num_geuv  = submodel_adp.geometric_probability_per_trial().length();

    size_t dist_index = padivm_index - num_ddriv - num_ddsiv;
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


size_t NestedModel::sm_adrv_index_map(size_t padrvm_index, short sadrvm_target)
{
  //switch (sadrvm_target) {
  //case Pecos::DDSRV_LWR_BND: case Pecos::DDSRV_UPR_BND:
  //case Pecos::DSSRV_LWR_BND: case Pecos::DSSRV_UPR_BND:
  //  return padrvm_index; break;
  //case Pecos::DAURV_DISTRIBUTION_PARAMETER:
  //  UShortMultiArrayConstView adrv_types
  //    = subModel.all_discrete_real_variable_types();
  //  size_t num_ddsrv = std::count(adrv_types.begin(), adrv_types.end(),
  //				    (unsigned short)DISCRETE_DESIGN_SET_REAL);
  //  return padrvm_index - num_ddsrv;
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

  switch (svm_target) {
  case Pecos::CDV_LWR_BND: case Pecos::CSV_LWR_BND:
    subModel.all_continuous_lower_bound(r_var, mapped_index); break;
  case Pecos::CDV_UPR_BND: case Pecos::CSV_UPR_BND:
    subModel.all_continuous_upper_bound(r_var, mapped_index); break;
  case Pecos::N_MEAN:
    submodel_adp.normal_mean(r_var, mapped_index); break;
  case Pecos::N_STD_DEV:
    submodel_adp.normal_std_deviation(r_var, mapped_index); break;
  case Pecos::N_LWR_BND:
    submodel_adp.normal_lower_bound(r_var, mapped_index); break;
  case Pecos::N_UPR_BND:
    submodel_adp.normal_upper_bound(r_var, mapped_index); break;
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
  case Pecos::U_LWR_BND:
    submodel_adp.uniform_lower_bound(r_var, mapped_index); break;
  case Pecos::U_UPR_BND:
    submodel_adp.uniform_upper_bound(r_var, mapped_index); break;
  case Pecos::LU_LWR_BND:
    submodel_adp.loguniform_lower_bound(r_var, mapped_index); break;
  case Pecos::LU_UPR_BND:
    submodel_adp.loguniform_upper_bound(r_var, mapped_index); break;
  case Pecos::T_MODE:
    submodel_adp.triangular_mode(r_var, mapped_index); break;
  case Pecos::T_LWR_BND:
    submodel_adp.triangular_lower_bound(r_var, mapped_index); break;
  case Pecos::T_UPR_BND:
    submodel_adp.triangular_upper_bound(r_var, mapped_index); break;
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

void NestedModel::prepend_evalid(const String& eval_id_str)
{
  evalTagPrefix = eval_id_str;
}


} // namespace Dakota
