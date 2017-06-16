/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SurrogateModel
//- Description: Implementation code for the SurrogateModel class
//- Owner:       Mike Eldred
//- Checked by:

#include "dakota_system_defs.hpp"
#include "SurrogateModel.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "ParamResponsePair.hpp"
#include "PRPMultiIndex.hpp"
#include "dakota_data_io.hpp"
#include "SurrogateData.hpp"

static const char rcsId[]="@(#) $Id: SurrogateModel.cpp 7024 2010-10-16 01:24:42Z mseldre $";

//#define DEBUG


namespace Dakota {

SurrogateModel::SurrogateModel(ProblemDescDB& problem_db):
  Model(BaseConstructor(), problem_db),
  surrogateFnIndices(problem_db.get_is("model.surrogate.function_indices")),
  corrType(problem_db.get_short("model.surrogate.correction_type")),
  surrModelEvalCntr(0), approxBuilds(0), mappingInitialized(false)
{
  // assign default responseMode based on correction specification;
  // NO_CORRECTION (0) is default
  responseMode = (corrType) ? AUTO_CORRECTED_SURROGATE : UNCORRECTED_SURROGATE;

  // process surrogateFnIndices. IntSets are sorted and unique.
  if (surrogateFnIndices.empty()) // default: all fns are approximated
    for (int i=0; i<numFns; ++i)
      surrogateFnIndices.insert(i);
  else {
    // check for out of range values
    if (*surrogateFnIndices.begin() < 0 ||
	*(--surrogateFnIndices.end()) >= numFns) {
      Cerr << "Error: id_surrogates out of range." << std::endl;
      abort_handler(-1);
    }
  }
}


SurrogateModel::
SurrogateModel(ProblemDescDB& problem_db, ParallelLibrary& parallel_lib,
	       const SharedVariablesData& svd, const SharedResponseData& srd,
	       const ActiveSet& set, short corr_type, short output_level):
  Model(LightWtBaseConstructor(), problem_db, parallel_lib, svd, srd,
	set, output_level),
  corrType(corr_type), surrModelEvalCntr(0), approxBuilds(0),
  mappingInitialized(false)
{
  modelType = "surrogate";

  // assign default responseMode based on correction specification;
  // NO_CORRECTION (0) is default
  responseMode = (corrType) ? AUTO_CORRECTED_SURROGATE : UNCORRECTED_SURROGATE;

  // set up surrogateFnIndices to use default (all fns are approximated)
  for (int i=0; i<numFns; ++i)
    surrogateFnIndices.insert(i);
}


void SurrogateModel::check_submodel_compatibility(const Model& sub_model)
{
  bool error_flag = false;
  // Check for compatible array sizing between sub_model and currentResponse
  size_t sm_num_fns = sub_model.num_functions();
  if ( sm_num_fns != numFns ) {
    Cerr << "Error: incompatibility between approximate and actual model "
	 << "response function sets\n       within SurrogateModel: "
	 << numFns << " approximate and " << sm_num_fns << " actual functions."
	 << "\n       Check consistency of responses specifications."
	 << std::endl;
    error_flag = true;
  }

  // Check for compatible array sizing between sub_model and currentVariables,
  // accounting for the use of different views in different variables sets:
  //   > common case for local/multipoint/hierarchical: the variables view in
  //     sub_model and currentVariables are the same.
  //   > common case for global: sub_model has an "All" vars view due to DACE
  //     usage and the currentVariables view may vary depending on the type
  //     of iterator interfaced with this SurrogateModel.  Enforcing an "all"
  //     view in the data returned from currentVariables ensures consistency.
  short cv_active_view = currentVariables.view().first;
  short sm_active_view = sub_model.current_variables().view().first;
  if ( cv_active_view == sm_active_view ) {
    // common cases: Distinct on Distinct (e.g., opt/UQ on local/multipt/hier)
    //               All on All           (e.g., DACE/PStudy on global)
    size_t sm_cv = sub_model.cv(),     sm_div = sub_model.div(),
      sm_dsv = sub_model.dsv(),        sm_drv = sub_model.drv(),
      cv_cv  = currentVariables.cv(),  cv_div = currentVariables.div(),
      cv_dsv = currentVariables.dsv(), cv_drv = currentVariables.drv();
    if ( sm_cv  != cv_cv  || sm_div != cv_div ||
	 sm_dsv != cv_dsv || sm_drv != cv_drv ) {
      Cerr << "Error: incompatibility between approximate and actual model "
	   << "variable sets within\n       SurrogateModel: active approximate "
	   << "= " << cv_cv << " continuous, " << cv_div << " discrete int, "
	   << cv_dsv << " discrete string, and " << cv_drv << " discrete real "
	   << "and\n       active actual = " << sm_cv << " continuous, "
	   << sm_div << " discrete int, " << sm_dsv << " discrete string, and "
	   << sm_drv << " discrete real.  Check consistency of variables "
	   << "specifications." << std::endl;
      error_flag = true;
    }
  }
  else {
    if ( ( sm_active_view == RELAXED_ALL || sm_active_view == MIXED_ALL ) &&
	 cv_active_view >= RELAXED_DESIGN ) {
      // common case: Distinct on All (e.g., opt/UQ on global surrogate)
      size_t sm_cv  = sub_model.cv(),      sm_div  = sub_model.div(),
	sm_dsv  = sub_model.dsv(),         sm_drv  = sub_model.drv(),
	cv_acv  = currentVariables.acv(),  cv_adiv = currentVariables.adiv(),
	cv_adsv = currentVariables.adsv(), cv_adrv = currentVariables.adrv();
      if ( sm_cv  != cv_acv  || sm_div != cv_adiv ||
	   sm_dsv != cv_adsv || sm_drv != cv_adrv ) {
	Cerr << "Error: incompatibility between approximate and actual model "
	     << "variable sets within\n       SurrogateModel: active "
	     << "approximate = " << cv_acv << " continuous, " << cv_adiv
	     << " discrete int, " << cv_adsv << " discrete string, and "
	     << cv_adrv << " discrete real (All view) and\n       "
	     << "active actual = " << sm_cv << " continuous, " << sm_div
	     << " discrete int, " << sm_dsv << " discrete string, and "
	     << sm_drv << " discrete real.  Check consistency of variables "
	     << "specifications." << std::endl;
	error_flag = true;
      }
    }
    else if ( ( cv_active_view == RELAXED_ALL || cv_active_view == MIXED_ALL )
	      && sm_active_view >= RELAXED_DESIGN ) {
      // common case: All on Distinct (e.g., DACE/PStudy on local/multipt/hier)
      size_t sm_acv = sub_model.acv(),    sm_adiv = sub_model.adiv(),
	sm_adsv = sub_model.adsv(),       sm_adrv = sub_model.adrv(),
	cv_cv   = currentVariables.cv(),  cv_div  = currentVariables.div(),
	cv_dsv  = currentVariables.dsv(), cv_drv  = currentVariables.drv();
      if ( sm_acv  != cv_cv  || sm_adiv != cv_div ||
	   sm_adsv != cv_dsv || sm_adrv != cv_drv ) {
	Cerr << "Error: incompatibility between approximate and actual model "
	     << "variable sets within\n       SurrogateModel: active "
	     << "approximate = " << cv_cv << " continuous, " << cv_div
	     << " discrete int, " << cv_dsv << " discrete string, and "
	     << cv_drv << " discrete real and\n       active actual = "
	     << sm_acv << " continuous, " << sm_adiv << " discrete int, "
	     << sm_adsv << " discrete string, and " << sm_adrv
	     << " discrete real (All view).  Check consistency of variables "
	     << "specifications." << std::endl;
	error_flag = true;
      }
    }
    else {
      Cerr << "Error: unsupported variable view differences between approximate"
	   << " and actual models within SurrogateModel." << std::endl;
      error_flag = true;
    }
  }

  /*
  if ( ( ( strbegins(surrogateType, "local_") || 
           strbegins(surrogateType, "multipoint_")
	|| surrogateType == "hierarchical" ) &&
	 ( sub_model.cv() != currentVariables.cv() ||
           sub_model.div() != currentVariables.div() ||
           sub_model.dsv() != currentVariables.dsv() ||
           sub_model.drv() != currentVariables.drv() ) ) ||
       ( strbegins(surrogateType, "global_") &&
	 ( sub_model.cv() != currentVariables.acv() ||
	   sub_model.div() != currentVariables.adiv() ||
	   sub_model.dsv() != currentVariables.adsv() ||
	   sub_model.drv() != currentVariables.adrv() ) ) ) {
    Cerr << "Error: subordinate model not compatible within SurrogateModel.\n"
	 << "       Check consistency of variables specifications for\n"
	 << "       approximation and actual models." << std::endl;
    error_flag = true;
  }
  */

  if (error_flag)
    abort_handler(-1);
}


/** This function forces a rebuild of the approximation according to
    the sub-model variables view, the approximation type, and whether
    the active approximation bounds or inactive variable values have
    changed since the last approximation build. */
bool SurrogateModel::force_rebuild()
{
  // force rebuild for change in inactive vars based on sub-model view.  It
  // is assumed that any recastings within Model recursions do not affect the
  // inactive variables (while RecastModel::variablesMapping has access to
  // all of the vars, the convention is to modify only the active vars).

  // for global surrogates, force rebuild for change in active bounds

  Model& actual_model = truth_model();

  // Don't force rebuild for active subspace model:
  // JAM TODO: There is probably a more elegant way to accomodate subspace models
  if (actual_model.model_type() == "active_subspace")
    return false;

  short approx_active_view = currentVariables.view().first;
  if (actual_model.is_null()) {
    // compare reference vars against current inactive top-level data
    if ( referenceICVars  != currentVariables.inactive_continuous_variables() ||
	 referenceIDIVars !=
	 currentVariables.inactive_discrete_int_variables()                   ||
	 referenceIDSVars !=
	 currentVariables.inactive_discrete_string_variables()                ||
	 referenceIDRVars !=
	 currentVariables.inactive_discrete_real_variables() )
      return true;

    if ( strbegins(surrogateType, "global_") &&
	 // compare reference bounds against current active top-level data
	 ( referenceCLBnds != userDefinedConstraints.continuous_lower_bounds()||
	   referenceCUBnds != userDefinedConstraints.continuous_upper_bounds()||
	   referenceDILBnds !=
	   userDefinedConstraints.discrete_int_lower_bounds()                 ||
	   referenceDIUBnds !=
	   userDefinedConstraints.discrete_int_upper_bounds()                 ||
	   // no discrete string bounds
	   referenceDRLBnds !=
	   userDefinedConstraints.discrete_real_lower_bounds()                ||
	   referenceDRUBnds !=
	   userDefinedConstraints.discrete_real_upper_bounds() ) )
	return true;
  }
  else { // actual_model is defined
    const Variables& actual_vars = actual_model.current_variables();
    short sub_model_active_view  = actual_vars.view().first;

    // compare reference vars against current inactive top-level data
    if ( approx_active_view == sub_model_active_view  &&
	 approx_active_view >= RELAXED_DESIGN &&
	 // compare inactive top-level data against inactive sub-model data
	 ( referenceICVars != currentVariables.inactive_continuous_variables()||
	   referenceIDIVars !=
	   currentVariables.inactive_discrete_int_variables()                 ||
	   referenceIDSVars !=
	   currentVariables.inactive_discrete_string_variables()              ||
	   referenceIDRVars !=
	   currentVariables.inactive_discrete_real_variables() ) )
      return true;
    else if ( ( approx_active_view == RELAXED_ALL ||
		approx_active_view == MIXED_ALL ) &&
	      sub_model_active_view >= RELAXED_DESIGN ) {
      // coerce top level data to sub-model view, but don't update sub-model
      if (truthModelVars.is_null())
	truthModelVars = actual_vars.copy();
      truthModelVars.all_continuous_variables(
        currentVariables.continuous_variables());
      truthModelVars.all_discrete_int_variables(
        currentVariables.discrete_int_variables());
      truthModelVars.all_discrete_string_variables(
        currentVariables.discrete_string_variables());
      truthModelVars.all_discrete_real_variables(
        currentVariables.discrete_real_variables());
      // perform check on inactive data at sub-model level
      if ( referenceICVars  != truthModelVars.inactive_continuous_variables() ||
	   referenceIDIVars !=
	   truthModelVars.inactive_discrete_int_variables() ||
	   referenceIDSVars !=
	   truthModelVars.inactive_discrete_string_variables() ||
	   referenceIDRVars !=
	   truthModelVars.inactive_discrete_real_variables() )
	return true;
    }
    // TO DO: extend for aleatory/epistemic uncertain views
    /*
    Model sub_model = actual_model.subordinate_model();
    while (sub_model.model_type() == "recast")
      sub_model = sub_model.subordinate_model();
    if ( referenceICVars  != sub_model.inactive_continuous_variables()      ||
         referenceIDIVars != sub_model.inactive_discrete_int_variables()    ||
         referenceIDSVars != sub_model.inactive_discrete_string_variables() ||
         referenceIDRVars != sub_model.inactive_discrete_real_variables() )
      return true;
    */

    // compare reference bounds against current active top-level data
    if ( strbegins(surrogateType, "global_") ) {

      if (actual_model.model_type() == "recast") {
	// check for internal changes within subModel definition since the
	// SurrogateModel may be in a standard variable space (such that the
	// outer level values/bounds do not reflect inner level updates).

	// force_rebuild() is called within the context of an approximate
	// derived_evaluate(), whereas update_actual_model() and update_global()
	// are called w/i the context of build_approximation().  Therefore, one
	// must be cautious with assuming that top-level updates have propagated
	// to lower levels.  (The only current use case involves
	// uSpaceModel.force_rebuild() w/i NonDExpansion::compute_expansion(),
	// although it may prove useful for other u-space approximations within
	// PCE/SC and local/global reliability).

	// Dive through Model recursion to bypass recasting. This is not readily
	// handled within new Model virtual fns since the type of approximation
	// (known here, but not w/i virtual fns) could dictate different checks.
	Model sub_model = actual_model.subordinate_model();
	while (sub_model.model_type() == "recast")
	  sub_model = sub_model.subordinate_model();

	if (referenceCLBnds  != sub_model.continuous_lower_bounds()    ||
	    referenceCUBnds  != sub_model.continuous_upper_bounds()    ||
	    referenceDILBnds != sub_model.discrete_int_lower_bounds()  ||
	    referenceDIUBnds != sub_model.discrete_int_upper_bounds()  ||
	    // no discrete string bounds
	    referenceDRLBnds != sub_model.discrete_real_lower_bounds() ||
	    referenceDRUBnds != sub_model.discrete_real_upper_bounds())
	  return true;
      }
      else if ( approx_active_view == sub_model_active_view && 
		// compare active top-level data against active sub-model data
		( referenceCLBnds !=
		  userDefinedConstraints.continuous_lower_bounds()    ||
		  referenceCUBnds !=
		  userDefinedConstraints.continuous_upper_bounds()    ||
		  referenceDILBnds !=
		  userDefinedConstraints.discrete_int_lower_bounds()  ||
		  referenceDIUBnds !=
		  userDefinedConstraints.discrete_int_upper_bounds()  ||
		  // no discrete string bounds
		  referenceDRLBnds !=
		  userDefinedConstraints.discrete_real_lower_bounds() ||
		  referenceDRUBnds !=
		  userDefinedConstraints.discrete_real_upper_bounds() ) )
	return true;
      else if ( approx_active_view >= RELAXED_DESIGN &&
		( sub_model_active_view == RELAXED_ALL ||
		  sub_model_active_view == MIXED_ALL ) && 
		// compare top-level data in All view w/ active sub-model data
		( referenceCLBnds !=
		  userDefinedConstraints.all_continuous_lower_bounds()     ||
		  referenceCUBnds != 
		  userDefinedConstraints.all_continuous_upper_bounds()     ||
		  referenceDILBnds !=
		  userDefinedConstraints.all_discrete_int_lower_bounds()   ||
		  referenceDIUBnds !=
		  userDefinedConstraints.all_discrete_int_upper_bounds()   ||
		  // no discrete string bounds
		  referenceDRLBnds !=
		  userDefinedConstraints.all_discrete_real_lower_bounds()  ||
		  referenceDRUBnds !=
		  userDefinedConstraints.all_discrete_real_upper_bounds() ) )
	return true;
      else if ( ( approx_active_view  == RELAXED_ALL ||
		  approx_active_view  == MIXED_ALL ) &&
		sub_model_active_view >= RELAXED_DESIGN ) {
	// coerce top level data to sub-model view, but don't update sub-model
	if (truthModelCons.is_null())
	  truthModelCons = actual_model.user_defined_constraints().copy();
	truthModelCons.all_continuous_lower_bounds(
	  userDefinedConstraints.continuous_lower_bounds());
	truthModelCons.all_continuous_upper_bounds(
	  userDefinedConstraints.continuous_upper_bounds());
	truthModelCons.all_discrete_int_lower_bounds(
	  userDefinedConstraints.discrete_int_lower_bounds());
	truthModelCons.all_discrete_int_upper_bounds(
	  userDefinedConstraints.discrete_int_upper_bounds());
	// no discrete string bounds
	truthModelCons.all_discrete_real_lower_bounds(
	  userDefinedConstraints.discrete_real_lower_bounds());
	truthModelCons.all_discrete_real_upper_bounds(
	  userDefinedConstraints.discrete_real_upper_bounds());
	// perform check on active data at sub-model level
	if ( referenceCLBnds  != truthModelCons.continuous_lower_bounds()    ||
	     referenceCUBnds  != truthModelCons.continuous_upper_bounds()    ||
	     referenceDILBnds != truthModelCons.discrete_int_lower_bounds()  ||
	     referenceDIUBnds != truthModelCons.discrete_int_upper_bounds()  ||
	     // no discrete string bounds
	     referenceDRLBnds != truthModelCons.discrete_real_lower_bounds() ||
	     referenceDRUBnds != truthModelCons.discrete_real_upper_bounds() )
	  return true;
      }

      /*
      // -----------------------COLLAPSED----------------------------------
      if ( // SBO: rebuild over {d} for each new TR of {d}
	   // OUU All view: rebuild over {u}+{d} for each new TR of {d}
	   active_bounds_differ ||
	   // OUU Distinct view: rebuild over {u} for each new instance of {d}
	   ( sub_model_active_view >= RELAXED_DESIGN &&
	     inactive_values_differ ) )
        return true;

      // -----------------------EXPANDED-----------------------------------
      if (approx_active_view == sub_model_active_view &&
	  approx_active_view >= RELAXED_DESIGN) { // Distinct to Distinct
        // SBO: rebuild over {d} for each new TR of {d} 
        // OUU: force rebuild over {u} for each new instance of {d}
        // inactive bounds are irrelevant
        if (active_bounds_differ || inactive_values_differ)
	  return true;
      }
      else if ( approx_active_view   == sub_model_active_view &&
                ( approx_active_view == RELAXED_ALL || 
	          approx_active_view == MIXED_ALL ) ) { // All to All
        // unusual case: Surrogate-based DACE,PStudy
        // there are no inactive vars/bounds
        if (active_bounds_differ)
	  return true;
      }
      else if ( approx_active_view >= RELAXED_DESIGN &&
                ( sub_model_active_view == RELAXED_ALL ||
	          sub_model_active_view == MIXED_ALL ) ) { // Distinct to All
        // OUU: force rebuild over {u}+{d} for each new TR of {d}
        if (active_bounds_differ)
	  return true;
      }
      else if ( ( approx_active_view  == RELAXED_ALL ||
                  approx_active_view  == MIXED_ALL ) &&
	        sub_model_active_view >= RELAXED_DESIGN ) { // All->Distinct
        // unusual case: approx over subset of active top-level vars
        if (active_bounds_differ || inactive_values_differ)
	  return true;
      }
      */
    }
    /*
    else { // local, multipoint, hierarchical

      // For local/multipoint/hierarchical, the approximation is not dependent
      // on the bounds.  For an "All" sub-model view, the surrogate accounts for
      // _all_ continuous variables and a rebuild never needs to be forced
      // (although many surrogate-based algorithms will rebuild for each new
      // approx region).  For a "Distinct" view, a rebuild is required for any
      // change in inactive variable values.

      // -------------------------COLLAPSED------------------------------
      if ( // OUU Distinct view: rebuild over {u} for each new instance of {d}
	   sub_model_active_view >= RELAXED_DESIGN &&
	   inactive_values_differ )
        return true;

      // -------------------------EXPANDED-------------------------------
      if (approx_active_view == sub_model_active_view &&
	  approx_active_view >= RELAXED_DESIGN) { // Distinct to Distinct
        // SBO: rebuild over {d} for each new TR of {d} 
        // OUU: force rebuild over {u} for each new instance of {d}
        // inactive bounds are irrelevant
        if (inactive_values_differ)
	  return true;
      }
      else if ( approx_active_view   == sub_model_active_view &&
                ( approx_active_view == RELAXED_ALL || 
	          approx_active_view == MIXED_ALL ) ) { // All to All
        // unusual case: Surrogate-based DACE,PStudy
        // there are no inactive vars
      }
      else if ( approx_active_view >= RELAXED_DESIGN &&
                ( sub_model_active_view == RELAXED_ALL ||
	          sub_model_active_view == MIXED_ALL ) ) { // Distinct to All
        // OUU: force rebuild over {u}+{d} for each new TR of {d}
      }
      else if ( ( approx_active_view  == RELAXED_ALL ||
                  approx_active_view  == MIXED_ALL ) &&
	        sub_model_active_view >= RELAXED_DESIGN ) { // All->Distinct
        // unusual case: approx over subset of active top-level vars
        if (inactive_values_differ)
	  return true;
      }
    }
    */
  }

  return false; // no rebuild required
}


void SurrogateModel::
asv_mapping(const ShortArray& orig_asv, ShortArray& actual_asv,
	    ShortArray& approx_asv, bool build_flag)
{
  if (surrogateFnIndices.size() == numFns) {
    if (build_flag) actual_asv = orig_asv;
    else            approx_asv = orig_asv;
  }
  else { // mixed response set
    if (build_flag) { // construct mode: define actual_asv
      actual_asv.assign(numFns, 0);
      for (ISIter it=surrogateFnIndices.begin();
	   it!=surrogateFnIndices.end(); ++it)
	actual_asv[*it] = orig_asv[*it];
    }
    else { // eval mode: define actual_asv & approx_asv contributions
      for (size_t i=0; i<numFns; ++i) {
	short orig_asv_val = orig_asv[i];
	if (orig_asv_val) {
	  if (surrogateFnIndices.count(i)) {
	    if (approx_asv.empty()) // keep empty if no active requests
	      approx_asv.assign(numFns, 0);
	    approx_asv[i] = orig_asv_val;
	  }
	  else {
	    if (actual_asv.empty()) // keep empty if no active requests
	      actual_asv.assign(numFns, 0);
	    actual_asv[i] = orig_asv_val;
	  }
	}
      }
    }
  }
}


void SurrogateModel::
asv_mapping(const ShortArray& actual_asv, const ShortArray& approx_asv,
	    ShortArray& combined_asv)
{
  if (combined_asv.empty())
    combined_asv.resize(numFns);

  if (actual_asv.empty())
    combined_asv = approx_asv;
  else if (approx_asv.empty())
    combined_asv = actual_asv;
  else 
    for (size_t i=0; i<numFns; ++i)
      combined_asv[i] = (surrogateFnIndices.count(i)) ?
	approx_asv[i] : actual_asv[i];
}


void SurrogateModel::
response_mapping(const Response& actual_response,
		 const Response& approx_response, Response& combined_response)
{
  const ShortArray& actual_asv = actual_response.active_set_request_vector();
  const ShortArray& approx_asv = approx_response.active_set_request_vector();
  ShortArray combined_asv;
  if (combined_response.is_null()) {
    combined_response = currentResponse.copy();
    asv_mapping(actual_asv, approx_asv, combined_asv);
    combined_response.active_set_request_vector(combined_asv);
  }
  else
    combined_asv = combined_response.active_set_request_vector();

  if (approx_asv.empty())
    combined_response.update(actual_response);
  else if (actual_asv.empty())
    combined_response.update(approx_response);
  else { // combined
    const RealVector& actual_fns   = actual_response.function_values();
    const RealVector& approx_fns   = approx_response.function_values();
    //const RealMatrix& actual_grads = actual_response.function_gradients();
    //const RealMatrix& approx_grads = approx_response.function_gradients();
    const RealSymMatrixArray& actual_hessians
      = actual_response.function_hessians();
    const RealSymMatrixArray& approx_hessians
      = approx_response.function_hessians();
    for (size_t i=0; i<numFns; i++) {
      if (surrogateFnIndices.count(i)) {
	if (combined_asv[i] & 1)
	  combined_response.function_value(approx_fns[i], i);
	if (combined_asv[i] & 2)
	  combined_response.function_gradient(
	    approx_response.function_gradient_view(i), i);
	if (combined_asv[i] & 4)
	  combined_response.function_hessian(approx_hessians[i], i);
      }
      else {
	if (combined_asv[i] & 1)
	  combined_response.function_value(actual_fns[i], i);
	if (combined_asv[i] & 2)
	  combined_response.function_gradient(
	   actual_response.function_gradient_view(i), i); 
	if (combined_asv[i] & 4)
	  combined_response.function_hessian(actual_hessians[i], i);
      }
    }
  }
}


void SurrogateModel::
aggregate_response(const Response& hf_resp, const Response& lf_resp,
		   Response& agg_resp)
{
  if (agg_resp.is_null())
    agg_resp = currentResponse.copy(); // resized to 2x in resize_response()

  const ShortArray& lf_asv =  lf_resp.active_set_request_vector();
  const ShortArray& hf_asv =  hf_resp.active_set_request_vector();
  ShortArray       agg_asv = agg_resp.active_set_request_vector();
  size_t i, offset_i, num_lf_fns = lf_asv.size(), num_hf_fns = hf_asv.size();
  short asv_i;

  for (i=0; i<num_lf_fns; ++i) {
    agg_asv[i] = asv_i = lf_asv[i];
    if (asv_i & 1) agg_resp.function_value(lf_resp.function_value(i), i);
    if (asv_i & 2)
      agg_resp.function_gradient(lf_resp.function_gradient_view(i), i);
    if (asv_i & 4) agg_resp.function_hessian(lf_resp.function_hessian(i), i);
  }
  
  for (i=0; i<num_hf_fns; ++i) {
    offset_i = i + num_lf_fns;
    agg_asv[offset_i] = asv_i = hf_asv[i];
    if (asv_i & 1) agg_resp.function_value(hf_resp.function_value(i), offset_i);
    if (asv_i & 2)
      agg_resp.function_gradient(hf_resp.function_gradient_view(i), offset_i);
    if (asv_i & 4)
      agg_resp.function_hessian(hf_resp.function_hessian(i), offset_i);
  }

  agg_resp.active_set_request_vector(agg_asv);
}

} // namespace Dakota
