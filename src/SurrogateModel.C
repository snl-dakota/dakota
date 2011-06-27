/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SurrogateModel
//- Description: Implementation code for the SurrogateModel class
//- Owner:       Mike Eldred
//- Checked by:

#include "system_defs.h"
#include "SurrogateModel.H"
#include "ProblemDescDB.H"
#include "ParallelLibrary.H"
#include "ParamResponsePair.H"
#include "PRPMultiIndex.H"
#include "data_io.h"
#include "SurrogateData.hpp"

static const char rcsId[]="@(#) $Id: SurrogateModel.C 7024 2010-10-16 01:24:42Z mseldre $";


namespace Dakota {

SurrogateModel::SurrogateModel(ProblemDescDB& problem_db):
  Model(BaseConstructor(), problem_db), mixedResponseSet(false),
  surrogateFnIndices(problem_db.get_dis("model.surrogate.function_indices")),
  correctionType(problem_db.get_string("model.surrogate.correction_type")),
  correctionOrder(problem_db.get_short("model.surrogate.correction_order")),
  autoCorrection(false), correctionComputed(false), approxBuilds(0),
  surrogateBypass(false)
{
  // process surrogateFnIndices. IntSets are sorted and unique.
  if (surrogateFnIndices.empty()) // default: all fns are approximated
    for (int i=0; i<numFns; i++)
      surrogateFnIndices.insert(i);
  else {
    // check for out of range values
    if (*surrogateFnIndices.begin() < 0 ||
	*(--surrogateFnIndices.end()) >= numFns) {
      Cerr << "Error: id_surrogates out of range." << std::endl;
      abort_handler(-1);
    }
    if (surrogateFnIndices.size() != numFns)
      mixedResponseSet = true;
  }

  // initialize correction data
  combinedFlag = (correctionType == "combined");
  if (combinedFlag) {
    combineFactors.resize(numFns);
    combineFactors = 1.; // used on 1st cycle prior to existence of prev pt.
  }
  UShortArray approx_order(numDerivVars, correctionOrder);
  short data_order;
  switch (correctionOrder) {
  case 2: data_order = 7; break;
  case 1: data_order = 3; break;
  case 0: default: data_order = 1; break;
  }
  ISIter it;
  computeAdditive = (correctionType == "additive" || combinedFlag);
  if (computeAdditive) {
    addCorrections.resize(numFns);
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it)
      addCorrections[*it] = Approximation("local_taylor", approx_order, 
					   numDerivVars, data_order);
  }
  computeMultiplicative = (correctionType == "multiplicative" || combinedFlag);
  if (computeMultiplicative) {
    multCorrections.resize(numFns);
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it)
      multCorrections[*it] = Approximation("local_taylor", approx_order, 
					   numDerivVars, data_order);
  }
}


SurrogateModel::
SurrogateModel(ParallelLibrary& parallel_lib, const SharedVariablesData& svd,
	       const ActiveSet& set, const String& corr_type, short corr_order):
  Model(NoDBBaseConstructor(), parallel_lib, svd, set),
  mixedResponseSet(false), correctionType(corr_type),
  correctionOrder(corr_order), autoCorrection(false), correctionComputed(false),
  approxBuilds(0), surrogateBypass(false)
{
  modelType = "surrogate";

  // set up surrogateFnIndices to use default (all fns are approximated)
  int i;
  for (i=0; i<numFns; i++)
    surrogateFnIndices.insert(i);

  // initialize correction data
  combinedFlag = (correctionType == "combined");
  if (combinedFlag) {
    combineFactors.resize(numFns);
    combineFactors = 1.; // used on 1st cycle prior to existence of prev pt.
  }
  UShortArray approx_order(numDerivVars, correctionOrder);
  short data_order;
  switch (correctionOrder) {
  case 2: data_order = 7; break;
  case 1: data_order = 3; break;
  case 0: default: data_order = 1; break;
  }
  computeAdditive = (correctionType == "additive" || combinedFlag);
  if (computeAdditive) {
    addCorrections.resize(numFns);
    for (i=0; i<numFns; ++i)
      addCorrections[i] = Approximation("local_taylor", approx_order, 
					numDerivVars, data_order);
  }
  computeMultiplicative = (correctionType == "multiplicative" || combinedFlag);
  if (computeMultiplicative) {
    multCorrections.resize(numFns);
    for (i=0; i<numFns; ++i)
      multCorrections[i] = Approximation("local_taylor", approx_order, 
					 numDerivVars, data_order);
  }
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
    size_t sm_cv = sub_model.cv(), sm_div = sub_model.div(),
      sm_drv = sub_model.drv(), cv_cv = currentVariables.cv(),
      cv_div = currentVariables.div(), cv_drv = currentVariables.drv();
    if ( sm_cv != cv_cv || sm_div != cv_div || sm_drv != cv_drv ) {
      Cerr << "Error: incompatibility between approximate and actual model "
	   << "variable sets within\n       SurrogateModel: active approximate "
	   << "= " << cv_cv << " continuous, " << cv_div << " discrete int, "
	   << "and " << cv_drv << " discrete real and\n       active actual = "
	   << sm_cv << " continuous, " << sm_div << " discrete int, and "
	   << sm_drv << " discrete real.  Check consistency of variables "
	   << "specifications." << std::endl;
      error_flag = true;
    }
  }
  else {
    if ( ( sm_active_view == MERGED_ALL || sm_active_view == MIXED_ALL ) &&
	 cv_active_view >= MERGED_DISTINCT_DESIGN ) {
      // common case: Distinct on All (e.g., opt/UQ on global surrogate)
      size_t sm_cv  = sub_model.cv(), sm_div  = sub_model.div(),
	sm_drv  = sub_model.drv(), cv_acv = currentVariables.acv(),
	cv_adiv = currentVariables.adiv(), cv_adrv = currentVariables.adrv();
      if ( sm_cv != cv_acv || sm_div != cv_adiv || sm_drv != cv_adrv ) {
	Cerr << "Error: incompatibility between approximate and actual model "
	     << "variable sets within\n       SurrogateModel: active "
	     << "approximate = " << cv_acv << " continuous, " << cv_adiv
	     << " discrete int, and " << cv_adrv << " discrete real (All view) "
	     << "and\n       active actual = " << sm_cv << " continuous, "
	     << sm_div << " discrete int, and " << sm_drv << " discrete real.  "
	     << "Check consistency of variables specifications." << std::endl;
	error_flag = true;
      }
    }
    else if ( ( cv_active_view == MERGED_ALL || cv_active_view == MIXED_ALL ) &&
	      sm_active_view >= MERGED_DISTINCT_DESIGN ) {
      // common case: All on Distinct (e.g., DACE/PStudy on local/multipt/hier)
      size_t sm_acv = sub_model.acv(), sm_adiv = sub_model.adiv(),
	sm_adrv = sub_model.adrv(), cv_cv = currentVariables.cv(),
	cv_div = currentVariables.div(), cv_drv = currentVariables.drv();
      if ( sm_acv != cv_cv || sm_adiv != cv_div || sm_adrv != cv_drv ) {
	Cerr << "Error: incompatibility between approximate and actual model "
	     << "variable sets within\n       SurrogateModel: active "
	     << "approximate = " << cv_cv << " continuous, " << cv_div
	     << " discrete int, and " << cv_drv << " discrete real and\n       "
	     << "active actual = " << sm_acv << " continuous, " << sm_adiv
	     << " discrete int, and " << sm_adrv << " discrete real (All view)."
	     << "  Check consistency of variables specifications." << std::endl;
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
  if ( ( ( surrogateType.begins("local_") || surrogateType.begins("multipoint_")
	|| surrogateType == "hierarchical" ) &&
	 ( sub_model.cv() != currentVariables.cv() ||
           sub_model.div() != currentVariables.div() ||
           sub_model.drv() != currentVariables.drv() ) ) ||
       ( surrogateType.begins("global_") &&
	 ( sub_model.cv() != currentVariables.acv() ||
	   sub_model.div() != currentVariables.adiv() ||
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
  Model&           actual_model = truth_model();
  const Variables& actual_vars  = actual_model.current_variables();
  short sub_model_active_view   = actual_vars.view().first,
        approx_active_view      = currentVariables.view().first;

  // define flag for change in inactive variable values based on sub-model view.
  // It is assumed that any recastings within Model recursions do not affect the
  // inactive variables (while RecastModel::variablesMapping has access to all
  // of the vars, the convention is to modify only the active vars).
  if (actual_model.is_null()) {
    // compare reference against current inactive top-level data
    if ( referenceICVars  != currentVariables.inactive_continuous_variables() ||
	 referenceIDIVars !=
	 currentVariables.inactive_discrete_int_variables()                   ||
	 referenceIDRVars !=
	 currentVariables.inactive_discrete_real_variables() )
      return true;
  }
  else if ( approx_active_view == sub_model_active_view  &&
	    approx_active_view >= MERGED_DISTINCT_DESIGN &&
        // compare inactive top-level data against inactive sub-model data
        ( referenceICVars != currentVariables.inactive_continuous_variables() ||
	  referenceIDIVars !=
	  currentVariables.inactive_discrete_int_variables()                  ||
	  referenceIDRVars !=
	  currentVariables.inactive_discrete_real_variables() ) )
    return true;
  else if ( ( approx_active_view == MERGED_ALL ||
	      approx_active_view == MIXED_ALL ) &&
	    sub_model_active_view >= MERGED_DISTINCT_DESIGN ) {
    // coerce top level data to sub-model view, but don't update sub-model
    if (subModelVars.is_null())
      subModelVars = actual_vars.copy();
    subModelVars.all_continuous_variables(
      currentVariables.continuous_variables());
    subModelVars.all_discrete_int_variables(
      currentVariables.discrete_int_variables());
    subModelVars.all_discrete_real_variables(
      currentVariables.discrete_real_variables());
    // perform check on inactive data at sub-model level
    if ( referenceICVars  != subModelVars.inactive_continuous_variables()    ||
	 referenceIDIVars != subModelVars.inactive_discrete_int_variables()  ||
	 referenceIDRVars != subModelVars.inactive_discrete_real_variables() )
      return true;
  }
  // TO DO: extend for aleatory/epistemic uncertain views
  /*
  Model sub_model = actual_model.subordinate_model();
  while (sub_model.model_type() == "recast")
    sub_model = sub_model.subordinate_model();
  if ( referenceICVars != sub_model.inactive_continuous_variables()     ||
       referenceIDIVars != sub_model.inactive_discrete_int_variables()  ||
       referenceIDRVars != sub_model.inactive_discrete_real_variables() )
    return true;
  */

  if ( surrogateType.begins("global_") ) {

    if (actual_model.is_null()) {
      // compare reference against current active top-level data
      if ( referenceCLBnds != userDefinedConstraints.continuous_lower_bounds()||
	   referenceCUBnds != userDefinedConstraints.continuous_upper_bounds()||
	   referenceDILBnds !=
	   userDefinedConstraints.discrete_int_lower_bounds()                 ||
	   referenceDIUBnds !=
	   userDefinedConstraints.discrete_int_upper_bounds()                 ||
	   referenceDRLBnds !=
	   userDefinedConstraints.discrete_real_lower_bounds()                ||
	   referenceDRUBnds !=
	   userDefinedConstraints.discrete_real_upper_bounds() )
	return true;
    }
    else if (actual_model.model_type() == "recast") {
      // check for internal changes within subModel definition since the
      // SurrogateModel may be in a standard variable space (such that the
      // outer level values/bounds do not reflect inner level updates).

      // force_rebuild() is called within the context of an approximate
      // derived_compute_response(), whereas update_actual_model() and
      // update_global() are called within the context of build_approximation().
      // Therefore, one must be cautious with assuming that top-level updates
      // have propagated to lower levels.  (The only current use case involves
      // uSpaceModel.force_rebuild() within NonDExpansion::compute_expansion(),
      // although it may prove useful for other u-space approximations within
      // PCE/SC and local/global reliability).

      // Dive through Model recursion to bypass recasting.  This is not readily
      // handled within new Model virtual fns since the type of approximation
      // (known here, but not w/i virtual fns) could dictate different checks.
      Model sub_model = actual_model.subordinate_model();
      while (sub_model.model_type() == "recast")
	sub_model = sub_model.subordinate_model();

      if (referenceCLBnds != sub_model.continuous_lower_bounds()     ||
	  referenceCUBnds != sub_model.continuous_upper_bounds()     ||
	  referenceDILBnds != sub_model.discrete_int_lower_bounds()  ||
	  referenceDIUBnds != sub_model.discrete_int_upper_bounds()  ||
	  referenceDRLBnds != sub_model.discrete_real_lower_bounds() ||
	  referenceDRUBnds != sub_model.discrete_real_upper_bounds())
	return true;
    }
    else if ( approx_active_view == sub_model_active_view && 
	// compare active top-level data against active sub-model data
	( referenceCLBnds != userDefinedConstraints.continuous_lower_bounds() ||
	  referenceCUBnds != userDefinedConstraints.continuous_upper_bounds() ||
	  referenceDILBnds !=
	  userDefinedConstraints.discrete_int_lower_bounds()                  ||
	  referenceDIUBnds !=
	  userDefinedConstraints.discrete_int_upper_bounds()                  ||
	  referenceDRLBnds !=
	  userDefinedConstraints.discrete_real_lower_bounds()                 ||
	  referenceDRUBnds !=
	  userDefinedConstraints.discrete_real_upper_bounds() ) )
      return true;
    else if ( approx_active_view >= MERGED_DISTINCT_DESIGN &&
	      ( sub_model_active_view == MERGED_ALL ||
		sub_model_active_view == MIXED_ALL ) && 
	// compare top-level data in All view against active sub-model data
        ( referenceCLBnds !=
	  userDefinedConstraints.all_continuous_lower_bounds()     ||
	  referenceCUBnds != 
	  userDefinedConstraints.all_continuous_upper_bounds()     ||
	  referenceDILBnds !=
	  userDefinedConstraints.all_discrete_int_lower_bounds()   ||
	  referenceDIUBnds !=
	  userDefinedConstraints.all_discrete_int_upper_bounds()   ||
	  referenceDRLBnds !=
	  userDefinedConstraints.all_discrete_real_lower_bounds()  ||
	  referenceDRUBnds !=
	  userDefinedConstraints.all_discrete_real_upper_bounds() ) )
      return true;
    else if ( ( approx_active_view  == MERGED_ALL ||
		approx_active_view  == MIXED_ALL ) &&
	      sub_model_active_view >= MERGED_DISTINCT_DESIGN ) {
      // coerce top level data to sub-model view, but don't update sub-model
      if (subModelCons.is_null())
	subModelCons = actual_model.user_defined_constraints().copy();
      subModelCons.all_continuous_lower_bounds(
	userDefinedConstraints.continuous_lower_bounds());
      subModelCons.all_continuous_upper_bounds(
	userDefinedConstraints.continuous_upper_bounds());
      subModelCons.all_discrete_int_lower_bounds(
	userDefinedConstraints.discrete_int_lower_bounds());
      subModelCons.all_discrete_int_upper_bounds(
	userDefinedConstraints.discrete_int_upper_bounds());
      subModelCons.all_discrete_real_lower_bounds(
	userDefinedConstraints.discrete_real_lower_bounds());
      subModelCons.all_discrete_real_upper_bounds(
	userDefinedConstraints.discrete_real_upper_bounds());
      // perform check on active data at sub-model level
      if ( referenceCLBnds  != subModelCons.continuous_lower_bounds()    ||
	   referenceCUBnds  != subModelCons.continuous_upper_bounds()    ||
	   referenceDILBnds != subModelCons.discrete_int_lower_bounds()  ||
	   referenceDIUBnds != subModelCons.discrete_int_upper_bounds()  ||
	   referenceDRLBnds != subModelCons.discrete_real_lower_bounds() ||
	   referenceDRUBnds != subModelCons.discrete_real_upper_bounds() )
	return true;
    }

    /*
    // -----------------------COLLAPSED----------------------------------
    if ( // SBO: rebuild over {d} for each new TR of {d}
	 // OUU All view: rebuild over {u}+{d} for each new TR of {d}
	 active_bounds_differ ||
	 // OUU Distinct view: rebuild over {u} for each new instance of {d}
	 ( sub_model_active_view >= MERGED_DISTINCT_DESIGN &&
	   inactive_values_differ ) )
      return true;

    // -----------------------EXPANDED-----------------------------------
    if (approx_active_view == sub_model_active_view &&
	approx_active_view >= MERGED_DISTINCT_DESIGN) { // Distinct to Distinct
      // SBO: rebuild over {d} for each new TR of {d} 
      // OUU: force rebuild over {u} for each new instance of {d}
      // inactive bounds are irrelevant
      if (active_bounds_differ || inactive_values_differ)
	return true;
    }
    else if ( approx_active_view   == sub_model_active_view &&
              ( approx_active_view == MERGED_ALL || 
	        approx_active_view == MIXED_ALL ) ) { // All to All
      // unusual case: Surrogate-based DACE,PStudy
      // there are no inactive vars/bounds
      if (active_bounds_differ)
	return true;
    }
    else if ( approx_active_view >= MERGED_DISTINCT_DESIGN &&
              ( sub_model_active_view == MERGED_ALL ||
	        sub_model_active_view == MIXED_ALL ) ) { // Distinct to All
      // OUU: force rebuild over {u}+{d} for each new TR of {d}
      if (active_bounds_differ)
	return true;
    }
    else if ( ( approx_active_view  == MERGED_ALL ||
                approx_active_view  == MIXED_ALL ) &&
	      sub_model_active_view >= MERGED_DISTINCT_DESIGN ) {//All->Distinct
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
	 sub_model_active_view >= MERGED_DISTINCT_DESIGN &&
	 inactive_values_differ )
      return true;

    // -------------------------EXPANDED-------------------------------
    if (approx_active_view == sub_model_active_view &&
	approx_active_view >= MERGED_DISTINCT_DESIGN) { // Distinct to Distinct
      // SBO: rebuild over {d} for each new TR of {d} 
      // OUU: force rebuild over {u} for each new instance of {d}
      // inactive bounds are irrelevant
      if (inactive_values_differ)
	return true;
    }
    else if ( approx_active_view   == sub_model_active_view &&
              ( approx_active_view == MERGED_ALL || 
	        approx_active_view == MIXED_ALL ) ) { // All to All
      // unusual case: Surrogate-based DACE,PStudy
      // there are no inactive vars
    }
    else if ( approx_active_view >= MERGED_DISTINCT_DESIGN &&
              ( sub_model_active_view == MERGED_ALL ||
	        sub_model_active_view == MIXED_ALL ) ) { // Distinct to All
      // OUU: force rebuild over {u}+{d} for each new TR of {d}
    }
    else if ( ( approx_active_view  == MERGED_ALL ||
                approx_active_view  == MIXED_ALL ) &&
	      sub_model_active_view >= MERGED_DISTINCT_DESIGN ) {//All->Distinct
      // unusual case: approx over subset of active top-level vars
      if (inactive_values_differ)
	return true;
    }
  }
  */

  return false; // no rebuild required
}


void SurrogateModel::
asv_mapping(const ShortArray& orig_asv, ShortArray& actual_asv,
	    ShortArray& approx_asv, bool build_flag)
{
  if (build_flag) { // construct mode: define actual_asv
    if (mixedResponseSet) {
      actual_asv.resize(numFns); actual_asv.assign(actual_asv.size(), 0);
      for (ISIter it=surrogateFnIndices.begin();
	   it!=surrogateFnIndices.end(); ++it) {
	int index = *it;
	actual_asv[index] = orig_asv[index];
      }
    }
    else
      actual_asv = orig_asv;
  }
  else { // eval mode: define actual_asv & approx_asv contributions
    if (surrogateBypass)
      actual_asv = orig_asv;
    else if (mixedResponseSet) {
      for (size_t i=0; i<numFns; i++) {
	bool  surr_id      = (surrogateFnIndices.count(i)) ? true : false;
	short orig_asv_val = orig_asv[i];
	// keep asv's at null size if no active requests
	if (surr_id && orig_asv_val) {
	  if (approx_asv.empty()) {
	    approx_asv.resize(numFns);
	    approx_asv.assign(approx_asv.size(), 0);
	  }
	  approx_asv[i] = orig_asv_val;
	}
	else if (!surr_id && orig_asv_val) {
	  if (actual_asv.empty()) {
	    actual_asv.resize(numFns);
	    actual_asv.assign(actual_asv.size(), 0);
	  }
	  actual_asv[i] = orig_asv_val;
	}
      }
    }
    else
      approx_asv = orig_asv;
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
      combined_asv[i]
	= (surrogateFnIndices.count(i)) ? approx_asv[i] : actual_asv[i];
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
	    approx_response.function_gradient(i), i );
	if (combined_asv[i] & 4)
	  combined_response.function_hessian(approx_hessians[i], i);
      }
      else {
	if (combined_asv[i] & 1)
	  combined_response.function_value(actual_fns[i], i);
	if (combined_asv[i] & 2)
	  combined_response.function_gradient(
	   actual_response.function_gradient(i), i ); 
	if (combined_asv[i] & 4)
	  combined_response.function_hessian(actual_hessians[i], i);
      }
    }
  }
}


/** Compute an additive or multiplicative correction that corrects the
    approx_response to have 0th-order consistency (matches values),
    1st-order consistency (matches values and gradients), or 2nd-order
    consistency (matches values, gradients, and Hessians) with the
    truth_response at a single point (e.g., the center of a trust
    region).  The 0th-order, 1st-order, and 2nd-order corrections use
    scalar values, linear scaling functions, and quadratic scaling
    functions, respectively, for each response function. */
void SurrogateModel::
compute_correction(const RealVector& c_vars, const Response& truth_response, 
		   const Response& approx_response)
{
  // The incoming approx_response is assumed to be uncorrected (i.e.,
  // correction has not been applied to it previously).  In this case,
  // it is not necessary to back out a previous correction, and the
  // computation of the new correction is straightforward.

  const RealVector&    truth_fns = truth_response.function_values();
  const RealVector&   approx_fns = approx_response.function_values();
  const RealMatrix&  truth_grads = truth_response.function_gradients();
  const RealMatrix& approx_grads = approx_response.function_gradients();
  const RealSymMatrixArray& truth_hessians
    = truth_response.function_hessians();
  const RealSymMatrixArray& approx_hessians
    = approx_response.function_hessians();

  // Catalog data needed later
  // *** TO DO: augment approxFnsPrevCenter logic for data fit surrogates.  May
  // require additional fn evaluation.  This may also be a better approach for
  // approxFnsCenter/approxGradsCenter within apply_multiplicative_correction
  // (if not found in search, evaluate it instead of falling back on center
  // approx values).
  int index; size_t j, k; ISIter it; short data_order;
  switch (correctionOrder) {
  case 2: data_order = 7; break;
  case 1: data_order = 3; break;
  case 0: default: data_order = 1; break;
  }
  if (combinedFlag && correctionComputed) {
    // save previous correction data for multipoint correction
    //correctionPrevCenterPt = correctionCenterPt;
    index = *surrogateFnIndices.begin();
    correctionPrevCenterPt = (computeAdditive || badScalingFlag) ?
      addCorrections[index].approximation_data().anchor_continuous_variables() :
      multCorrections[index].approximation_data().anchor_continuous_variables();
    approxFnsPrevCenter    = approxFnsCenter; // OK for hierarchical approx
    truthFnsPrevCenter     = truthFnsCenter;
  }
  if (combinedFlag)
    truthFnsCenter = truth_fns;
  //if (combinedFlag || correctionOrder >= 1)
  //  copy_data(c_vars, correctionCenterPt);
  if (combinedFlag || (computeMultiplicative && correctionOrder >= 1))
    approxFnsCenter = approx_fns;
  if (computeMultiplicative && correctionOrder >= 1)
    approxGradsCenter = approx_grads;

  // Multiplicative will fail if response functions are near zero.
  //   0th order:     a truth_val == 0 causes a zero scaling which will cause
  //                  optimization failure; an approx_val == 0 will cause a
  //                  division by zero FPE.
  //   1st/2nd order: a truth_val == 0 is OK (so long as the total scaling
  //                  function != 0); an approx_val == 0 will cause a division
  //                  by zero FPE.
  // In either case, automatically transition to additive correction.  Current
  // logic transitions back to multiplicative as soon as the response fns are
  // no longer near zero.
  badScalingFlag = false;
  if (computeMultiplicative) {
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
      index = *it;
      if ( std::fabs(approx_fns[index]) < 1.e-25 ||
	   ( correctionOrder == 0 && std::fabs(truth_fns[index]) < 1.e-25 ) ) {
	badScalingFlag = true;
        break;
      }
    }
    if (badScalingFlag) {
      Cout << "\nWarning: Multiplicative correction temporarily deactivated "
	   << "due to functions near zero.\n         Additive correction will "
	   << "be used.\n";
      if (addCorrections.empty()) {
	addCorrections.resize(numFns);
	UShortArray approx_order(numDerivVars, correctionOrder);
	for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it)
	  addCorrections[*it] = Approximation("local_taylor", approx_order, 
					      numDerivVars, data_order);
      }
    }
  }

  Pecos::SurrogateDataVars sdv(c_vars, Pecos::DEEP_COPY);
  if (computeAdditive || badScalingFlag) {
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
      index = *it;
      Pecos::SurrogateDataResp sdr(data_order, numDerivVars);
      // -----------------------------
      // Additive 0th order correction
      // -----------------------------
      if (data_order & 1)
	sdr.response_function(truth_fns[index] - approx_fns[index]);
      // -----------------------------
      // Additive 1st order correction
      // -----------------------------
      if (data_order & 2) {
	const Real*  truth_grad =  truth_grads[index];
	const Real* approx_grad = approx_grads[index];
	for (j=0; j<numDerivVars; ++j)
	  sdr.response_gradient(truth_grad[j] - approx_grad[j], j);
      }
      // -----------------------------
      // Additive 2nd order correction
      // -----------------------------
      if (data_order & 4) {
	const RealSymMatrix&  truth_hess =  truth_hessians[index];
	const RealSymMatrix& approx_hess = approx_hessians[index];
	for (j=0; j<numDerivVars; ++j)
	  for (k=0; k<=j; ++k) // lower half
	    sdr.response_hessian(truth_hess(j,k) - approx_hess(j,k), j, k);
      }
      // update anchor data
      addCorrections[index].add(sdv, true); // shallow copy into SurrogateData
      addCorrections[index].add(sdr, true); // shallow copy into SurrogateData

      if (outputLevel >= NORMAL_OUTPUT)
	Cout << "\nAdditive correction computed:\n" << sdr;
    }
  }

  if (computeMultiplicative && !badScalingFlag) {
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
      index = *it;
      Pecos::SurrogateDataResp sdr(data_order, numDerivVars);
      // -----------------------------------
      // Multiplicative 0th order correction
      // -----------------------------------
      if (data_order & 1)
	sdr.response_function(truth_fns[index] / approx_fns[index]);
      // -----------------------------------
      // Multiplicative 1st order correction
      // -----------------------------------
      // The beta-correction method is based on the work of Chang and Haftka,
      // and Alexandrov.  It is a multiplicative correction like the "scaled"
      // correction method, but it uses gradient information to achieve
      // 1st-order consistency (matches the high-fidelity function values and
      // the high-fidelity gradients at the center of the approximation region).
      if (data_order & 2) {
	const Real&    truth_fn =    truth_fns[index];
	const Real&   approx_fn =   approx_fns[index];
	const Real*  truth_grad =  truth_grads[index];
	const Real* approx_grad = approx_grads[index];
	Real f_lo_2 = approx_fn*approx_fn;
	for (j=0; j<numDerivVars; ++j)
	  sdr.response_gradient(truth_grad[j] / approx_fn -
				truth_fn * approx_grad[j] / f_lo_2, j);
      }
      // -----------------------------------
      // Multiplicative 2nd order correction
      // -----------------------------------
      if (data_order & 4) {
	const Real&             truth_fn =       truth_fns[index];
	const Real&            approx_fn =      approx_fns[index];
	const Real*           truth_grad =     truth_grads[index];
	const Real*          approx_grad =    approx_grads[index];
	const RealSymMatrix&  truth_hess =  truth_hessians[index];
	const RealSymMatrix& approx_hess = approx_hessians[index];
	// consider use of Teuchos assign and operator-=
	Real f_lo_f_hi = approx_fn * truth_fn, f_lo_2 = approx_fn * approx_fn,
	     f_lo_3 = approx_fn * f_lo_2;
	for (j=0; j<numDerivVars; ++j)
	  for (k=0; k<=j; ++k) // lower half
	    sdr.response_hessian(( f_lo_2 * truth_hess(j,k) - f_lo_f_hi
	      * approx_hess(j,k) + 2. * truth_fn * approx_grad[j]
	      * approx_grad[k] - approx_fn * ( truth_grad[j] * approx_grad[k]
	      + approx_grad[j] * truth_grad[k] ) ) / f_lo_3, j, k);
      }
      // update anchor data
      multCorrections[index].add(sdv, true); // shallow copy into SurrogateData
      multCorrections[index].add(sdr, true); // shallow copy into SurrogateData

      if (outputLevel >= NORMAL_OUTPUT)
	Cout << "\nMultiplicative correction computed:\n" << sdr;
    }
  }

  // Compute combination factors once for each new correction.  combineFactors =
  // [f_hi(x_pp) - f_hi_beta(x_pp)]/[f_hi_alpha(x_pp) - f_hi_beta(x_pp)].  This
  // ratio goes -> 1 (use additive alone) if f_hi_alpha(x_pp) -> f_hi(x_pp) and
  // it goes -> 0 (use multiplicative alone) if f_hi_beta(x_pp) -> f_hi(x_pp).
  if (combinedFlag && !badScalingFlag && correctionComputed) {
    RealVector alpha_corr_fns = approxFnsPrevCenter,
               beta_corr_fns  = approxFnsPrevCenter;
    apply_additive_correction(correctionPrevCenterPt, alpha_corr_fns);
    apply_multiplicative_correction(correctionPrevCenterPt, beta_corr_fns);
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
      index = *it;
      Real numer = truthFnsPrevCenter[index] - beta_corr_fns[index];
      Real denom =     alpha_corr_fns[index] - beta_corr_fns[index];
      combineFactors[index] = (std::fabs(denom) > 1.e-25) ? numer/denom : 1.;
#ifdef DEBUG
      Cout << "additive = " << alpha_corr_fns[index] <<" multiplicative = "
	   << beta_corr_fns[index] << "\nnumer = " << numer << " denom = "
	   << denom << '\n';
#endif
    }
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "\nCombined correction computed: combination factors =\n"
	   << combineFactors << '\n';

#ifdef DEBUG
    Cout << "Testing final match at previous point\n";
    Response approx_copy = approx_response.copy();
    approx_copy.active_set(fns_set);
    approx_copy.function_values(approxFnsPrevCenter);
    apply_correction(correctionPrevCenterPt, approx_copy);
#endif
  }

  if (computeAdditive || computeMultiplicative)
    correctionComputed = true;
}


void SurrogateModel::
apply_correction(const RealVector& c_vars, Response& approx_response,
		 bool quiet_flag)
{
  if (!correctionComputed)
    return;

  // update approx_response with the alpha/beta/combined corrected data
  if (correctionType == "additive" || badScalingFlag) // use alpha_corrected_*
    apply_additive_correction(c_vars, approx_response);
  else if (correctionType == "multiplicative") // use beta_corrected_*
    apply_multiplicative_correction(c_vars, approx_response);
  else if (combinedFlag) { // use both alpha_corrected_* and beta_corrected_*

    // compute {add,mult}_response contributions to combined correction
    Response add_response = approx_response.copy(),
            mult_response = approx_response.copy();
    apply_additive_correction(c_vars, add_response);
    apply_multiplicative_correction(c_vars, mult_response);

    // compute convex combination of add_response and mult_response
    ISIter it; int index; size_t j, k;
    const ShortArray& asv = approx_response.active_set_request_vector();
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
      index = *it;
      Real cf = combineFactors[index], ccf = 1. - cf;
      if (asv[index] & 1) {
	Real corrected_fn =  cf *  add_response.function_value(index)
	                  + ccf * mult_response.function_value(index);
	approx_response.function_value(corrected_fn, index);
      }
      if (asv[index] & 2) {
	RealVector corrected_grad(numDerivVars, false),
	  add_grad  =  add_response.function_gradient(index), // view
	  mult_grad = mult_response.function_gradient(index); // view
	for (j=0; j<numDerivVars; j++)
	  corrected_grad[j] = cf * add_grad[j] + ccf * mult_grad[j];
	approx_response.function_gradient(corrected_grad, index);
      }
      if (asv[index] & 4) {
	RealSymMatrix corrected_hess(numDerivVars, false);
	const RealSymMatrix&  add_hess =  add_response.function_hessian(index);
	const RealSymMatrix& mult_hess = mult_response.function_hessian(index);
	for (j=0; j<numDerivVars; ++j)
	  for (k=0; k<=j; ++k)
	    corrected_hess(j,k) = cf * add_hess(j,k) + ccf * mult_hess(j,k);
	approx_response.function_hessian(corrected_hess, index);
      }
    }
  }

  if (!quiet_flag)
    Cout << "\nCorrection applied: corrected response =\n" << approx_response;
}


void SurrogateModel::
apply_additive_correction(const RealVector& c_vars, Response& approx_response)
{
  if (!correctionComputed)
    return;

  size_t index; ISIter it;
  const ShortArray& asv = approx_response.active_set_request_vector();
  for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
    index = *it;
    Approximation& add_corr = addCorrections[index];
    if (asv[index] & 1)
      approx_response.function_value(approx_response.function_value(index) +
				     add_corr.get_value(c_vars), index);
    if (correctionOrder >= 1 && asv[index] & 2) {
      // update view (no reassignment):
      RealVector approx_grad = approx_response.function_gradient(index);
      approx_grad += add_corr.get_gradient(c_vars);
      // update copy and reassign:
      //RealVector approx_grad = approx_response.function_gradient_copy(index);
      //approx_grad += add_corr.get_gradient(c_vars);
      //approx_response.function_gradient(approx_grad, index)
    }
    if (correctionOrder == 2 && asv[index] & 4) {
      // update copy and reassign:
      RealSymMatrix approx_hess = approx_response.function_hessian(index);
      approx_hess += add_corr.get_hessian(c_vars);
      approx_response.function_hessian(approx_hess, index);
    }
  }
}


void SurrogateModel::
apply_additive_correction(const RealVector& c_vars, RealVector& approx_fns)
{
  if (!correctionComputed)
    return;

  for (ISIter it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it)
    approx_fns[*it] += addCorrections[*it].get_value(c_vars);
}


void SurrogateModel::
apply_multiplicative_correction(const RealVector& c_vars,
				Response& approx_response)
{
  if (!correctionComputed)
    return;

  // Retrieve uncorrected data for special cases where the data required to
  // apply the correction is different from the active data being corrected.
  bool fn_db_search = false, grad_db_search = false;
  const ShortArray& asv = approx_response.active_set_request_vector();
  ShortArray fn_db_asv, grad_db_asv; ISIter it; size_t j, k, index;
  for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
    index = *it;
    if ( !(asv[index] & 1) && ( ((asv[index] & 2) && correctionOrder >= 1) ||
				((asv[index] & 4) && correctionOrder == 2) ) ) {
      if (fn_db_asv.empty()) fn_db_asv.assign(numFns, 0);
      fn_db_asv[index] = 1; fn_db_search = true;
    }
    if ( !(asv[index] & 2) && (asv[index] & 4) && correctionOrder >= 1) {
      if (grad_db_asv.empty()) grad_db_asv.assign(numFns, 0);
      grad_db_asv[index] = 2; grad_db_search = true;
    }
  }
  // Retrieve the uncorrected fn values for use in gradient and Hessian
  // corrections.  They are not immediately available in cases where the
  // correction is applied only to the fn gradient (i.e., when the current
  // asv contains 2's due to DOT/CONMIN/OPT++ requesting gradients separately).
  RealVector uncorrected_fns;
  if (fn_db_search) {
    const Response& db_resp = search_db(c_vars, fn_db_asv);
    uncorrected_fns.sizeUninitialized(numFns);
    for (size_t i=0; i<numFns; ++i)
      if (fn_db_asv[i])
	uncorrected_fns[i] = db_resp.function_value(i);
  }
  // Retrieve the uncorrected fn gradients for use in Hessian corrections.
  // They are not immediately available in cases where the correction is
  // applied only to the fn Hessian (i.e., when the current asv contains
  // 4's due to OPT++ requesting Hessians separately).
  RealMatrix uncorrected_grads;
  if (grad_db_search) {
    const Response& db_resp = search_db(c_vars, grad_db_asv);
    uncorrected_grads.shapeUninitialized(numDerivVars, numFns);
    for (int i=0; i<numFns; ++i)
      if (grad_db_asv[i])
	Teuchos::setCol(db_resp.function_gradient(i), i, uncorrected_grads);
  }

  RealVector empty_rv;
  for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
    index = *it;
    Approximation&    mult_corr = multCorrections[index];
    const Real&         fn_corr = mult_corr.get_value(c_vars);
    const RealVector& grad_corr = (correctionOrder >= 1 && (asv[index] & 6)) ?
      mult_corr.get_gradient(c_vars) : empty_rv;
    // apply corrections in descending derivative order to avoid
    // disturbing original approx fn/grad values
    if (asv[index] & 4) {
      // update copy and reassign:
      RealSymMatrix approx_hess = approx_response.function_hessian(index);
      const Real*   approx_grad = (grad_db_search) ? uncorrected_grads[index] :
	approx_response.function_gradients()[index];
      approx_hess *= fn_corr; // all correction orders
      switch (correctionOrder) {
      case 2: {
	const RealSymMatrix& hess_corr = mult_corr.get_hessian(c_vars);
	const Real& approx_fn = (fn_db_search) ? uncorrected_fns[index] :
	  approx_response.function_value(index);
	for (j=0; j<numDerivVars; ++j)
	  for (k=0; k<=j; ++k)
	    approx_hess(j,k) += hess_corr(j,k) * approx_fn
	      +  grad_corr[j] * approx_grad[k] + grad_corr[k] * approx_grad[j];
	break;
      }
      case 1:
	for (j=0; j<numDerivVars; ++j)
	  for (k=0; k<=j; ++k)
	    approx_hess(j,k)
	      += grad_corr[j] * approx_grad[k] + grad_corr[k] * approx_grad[j];
	break;
      }
      approx_response.function_hessian(approx_hess, index);
    }
    if (asv[index] & 2) {
      // update view (no reassignment):
      RealVector approx_grad = approx_response.function_gradient(index);
      //RealVector approx_grad = approx_response.function_gradient_copy(index);
      const Real& approx_fn  = (fn_db_search) ? uncorrected_fns[index] :
	approx_response.function_value(index);
      approx_grad *= fn_corr; // all correction orders
      if (correctionOrder >= 1)
	for (j=0; j<numDerivVars; ++j)
	  approx_grad[j] += grad_corr[j] * approx_fn;
      //approx_response.function_gradient(approx_grad, index)
    }
    if (asv[index] & 1)
      approx_response.function_value(approx_response.function_value(index) *
				     fn_corr, index);
  }
}


void SurrogateModel::
apply_multiplicative_correction(const RealVector& c_vars,
				RealVector& approx_fns)
{
  if (!correctionComputed)
    return;

  for (ISIter it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it)
    approx_fns[*it] *= multCorrections[*it].get_value(c_vars);
}


const Response& SurrogateModel::
search_db(const RealVector& c_vars, const ShortArray& search_asv)
{
  // Retrieve missing uncorrected approximate data for use in derivative
  // multiplicative corrections.  The correct approach is to retrieve the
  // missing data for the current point in parameter space, and this
  // approach is now enforced in all cases (data is either available
  // directly as indicated by the asv, is retrieved via a data_pairs
  // search, or is recomputed).  A previous fallback was to employ approx
  // center data (see comments below).  Recomputation can occur either for
  // ApproximationInterface data in DataFitSurrModels or low fidelity data
  // in HierarchSurrModels that involves additional model recursions, since
  // neither of these data sets are catalogued in data_pairs.

  // query data_pairs to extract the response at the current pt
  Model& surr_model = surrogate_model();
  Variables search_vars = surr_model.current_variables().copy();     // copy
  search_vars.continuous_variables(c_vars);
  ActiveSet search_set = surr_model.current_response().active_set(); // copy
  search_set.request_vector(search_asv);
  extern PRPCache data_pairs; // global container
  PRPCacheHIter cache_it = lookup_by_val(data_pairs, surr_model.interface_id(),
					 search_vars, search_set);

  if (cache_it == data_pairs.get<hashed>().end()) {
    // perform approx fn eval to retrieve missing data
    surr_model.continuous_variables(c_vars);
    surr_model.compute_response(search_set);
    return surr_model.current_response();

    /* Old fall-back position uses the approximate data from the
    // center of the current trust region.  This still satisifies
    // consistency at the center of the trust region, but is a less
    // accurate approximation over the rest of the trust region.

    Cerr << "Warning: current function values not available.\n         "
	 << "beta correction using function values from the correction "
	 << "point." << std::endl;
    for (size_t i=0; i<numFns; ++i)
      if (search_asv[i] & 1)
	uncorrected_fns[i] = approxFnsCenter[i];

    Cerr << "Warning: current function gradients not available.\n"
	 << "         beta correction using function gradients from the"
	 << " correction point." << std::endl;
    for (int i=0; i<numFns; ++i)
      if (search_asv[i] & 2)
	Teuchos::setCol(Teuchos::getCol(Teuchos::View, approxGradsCenter, i),
	                i, uncorrected_grads);
    */
  }
  else
    return cache_it->prp_response();
}

} // namespace Dakota
