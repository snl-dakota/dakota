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
  ISIter it;
  computeAdditive = (correctionType == "additive" || combinedFlag);
  if (computeAdditive) {
    addCorrFns.resize(numFns);
    if (correctionOrder >= 1)
      addCorrGrads.shapeUninitialized(numDerivVars, numFns);
    if (correctionOrder == 2)
      addCorrHessians.resize(numFns);
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
      if (correctionOrder == 2)
	addCorrHessians[*it].reshape(numDerivVars);
    }
  }
  computeMultiplicative = (correctionType == "multiplicative" || combinedFlag);
  if (computeMultiplicative) {
    multCorrFns.resize(numFns);
    if (correctionOrder >= 1)
      multCorrGrads.shapeUninitialized(numDerivVars, numFns);
    if (correctionOrder == 2)
      multCorrHessians.resize(numFns);
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
      if (correctionOrder == 2)
	multCorrHessians[*it].reshape(numDerivVars);
    }
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
  computeAdditive = (correctionType == "additive" || combinedFlag);
  if (computeAdditive) {
    addCorrFns.resize(numFns);
    if (correctionOrder >= 1)
      addCorrGrads.shapeUninitialized(numDerivVars, numFns);
    if (correctionOrder == 2)
      addCorrHessians.resize(numFns);
    for (i=0; i<numFns; ++i) {
      if (correctionOrder == 2)
	addCorrHessians[i].reshape(numDerivVars);
    }
  }
  computeMultiplicative = (correctionType == "multiplicative" || combinedFlag);
  if (computeMultiplicative) {
    multCorrFns.resize(numFns);
    if (correctionOrder >= 1)
      multCorrGrads.shapeUninitialized(numDerivVars, numFns);
    if (correctionOrder == 2)
      multCorrHessians.resize(numFns);
    for (i=0; i<numFns; ++i) {
      if (correctionOrder == 2)
	multCorrHessians[i].reshape(numDerivVars);
    }
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
	   it!=surrogateFnIndices.end(); it++) {
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
compute_correction(const Response& truth_response, 
		   const Response& approx_response,
		   const RealVector& c_vars)
{
  // The incoming approx_response is assumed to be uncorrected (i.e.,
  // correction has not been applied to it previously).  In this case,
  // it is not necessary to back out a previous correction, and the
  // computation of the new correction is straightforward.

  const RealVector& truth_fns  = truth_response.function_values();
  const RealVector& approx_fns = approx_response.function_values();

  const RealMatrix& truth_grads  = truth_response.function_gradients();
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
  if (combinedFlag && correctionComputed) {
    // save previous correction data for multipoint correction
    correctionPrevCenterPt = correctionCenterPt;
    approxFnsPrevCenter    = approxFnsCenter; // OK for hierarchical approx
    truthFnsPrevCenter     = truthFnsCenter;
  }
  if (combinedFlag)
    truthFnsCenter = truth_fns;
  if (combinedFlag || correctionOrder >= 1)
    copy_data(c_vars, correctionCenterPt);
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
  size_t i, j, k;
  ISIter it;
  int index;
  badScalingFlag = false;
  if (computeMultiplicative) {
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); it++) {
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
      if (addCorrFns.empty()) {
	addCorrFns.resize(numFns);
	if (correctionOrder >= 1)
	  addCorrGrads.shapeUninitialized(numDerivVars, numFns);
	if (correctionOrder == 2)
	  addCorrHessians.resize(numFns);
	for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it){
	  index = *it;
	  if (correctionOrder == 2)
	    addCorrHessians[index].reshape(numDerivVars);
	}
      }
    }
  }

  if (computeAdditive || badScalingFlag) {

    // -----------------------------
    // Additive 0th order correction
    // -----------------------------
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
      index = *it;
      addCorrFns[index] = truth_fns[index] - approx_fns[index];
    }
    Cout << "\nCorrection computed: offset function values =\n" << addCorrFns;

    // -----------------------------
    // Additive 1st order correction
    // -----------------------------
    if (correctionOrder >= 1) {
      Cout << "                     offset function gradients =\n";
      for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
	index = *it;
	for (int j=0; j<numDerivVars; ++j)
	  addCorrGrads(j,index) = truth_grads(j,index) - approx_grads(j,index);

	write_col_vector_trans(Cout, index, true, true, true, addCorrGrads);
      }
    }

    // -----------------------------
    // Additive 2nd order correction
    // -----------------------------
    if (correctionOrder == 2) {
      Cout << "                     offset function Hessians =\n";
      for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
	index = *it;
	for (j=0; j<numDerivVars; j++)
	  for (k=0; k<numDerivVars; k++)
	    addCorrHessians[index](j,k) =  truth_hessians[index](j,k)
                                         - approx_hessians[index](j,k);
	Cout << addCorrHessians[index]; // with carriage return
      }
    }
    Cout << '\n';
  }

  if (computeMultiplicative && !badScalingFlag) {

    // -----------------------------------
    // Multiplicative 0th order correction
    // -----------------------------------
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
      index = *it;
      multCorrFns[index] = truth_fns[index] / approx_fns[index];
    }
    Cout << "\nCorrection computed: scaling function values =\n" << multCorrFns;

    // -----------------------------------
    // Multiplicative 1st order correction
    // -----------------------------------
    // The beta-correction method is based on the work of Chang and Haftka,
    // and Alexandrov.  It is a multiplicative correction like the "scaled"
    // correction method, but it uses gradient information to achieve 1st-order
    // consistency (matches the high-fidelity function values and the
    // high-fidelity gradients at the center of the approximation region).
    if (correctionOrder >= 1) {
      Cout << "                     scaling function gradients =\n";
      for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
	index = *it;
	Real f_lo_2 = std::pow(approx_fns[index], 2); // don't recompute for all
	for (j=0; j<numDerivVars; ++j)
	  multCorrGrads(j,index) = (approx_fns[index] * truth_grads(j,index)
	    - truth_fns[index] * approx_grads(j,index))/f_lo_2;

	write_col_vector_trans(Cout, index, true, true, true, multCorrGrads);
      }
    }

    // -----------------------------------
    // Multiplicative 2nd order correction
    // -----------------------------------
    if (correctionOrder == 2) {
      Cout << "                     scaling function Hessians =\n";
      for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
	index = *it;
	Real f_lo_f_hi = approx_fns[index] * truth_fns[index],
	     f_lo_2 = std::pow(approx_fns[index], 2),
	     f_lo_3 = std::pow(approx_fns[index], 3);
	for (j=0; j<numDerivVars; ++j)
	  for (k=0; k<numDerivVars; ++k)
	    multCorrHessians[index](j,k) = ( f_lo_2
	      * truth_hessians[index](j,k) - f_lo_f_hi
	      * approx_hessians[index](j,k) + 2. * truth_fns[index]
              * approx_grads(j,index) * approx_grads(k,index)
              - approx_fns[index] * ( truth_grads(j,index)
              * approx_grads(k,index) + approx_grads(j,index)
              * truth_grads(k,index) ) ) / f_lo_3;
	Cout << multCorrHessians[index]; // with carriage return
      }
    }
    Cout << '\n';
  }

  // Compute combination factors once for each new correction.  combineFactors =
  // [f_hi(x_pp) - f_hi_beta(x_pp)]/[f_hi_alpha(x_pp) - f_hi_beta(x_pp)].  This
  // ratio goes -> 1 (use additive alone) if f_hi_alpha(x_pp) -> f_hi(x_pp) and
  // it goes -> 0 (use multiplicative alone) if f_hi_beta(x_pp) -> f_hi(x_pp).
  if (combinedFlag && !badScalingFlag && correctionComputed) {
    ActiveSet fns_set = approx_response.active_set(); // copy
    fns_set.request_values(1); // correct fn values only
    RealVector alpha_corrected_fns = approxFnsPrevCenter,
                    beta_corrected_fns  = approxFnsPrevCenter;
    RealMatrix alpha_corrected_grads, beta_corrected_grads;
    RealSymMatrixArray alpha_corrected_hessians, beta_corrected_hessians;
    apply_additive_correction(alpha_corrected_fns, alpha_corrected_grads, 
			      alpha_corrected_hessians, correctionPrevCenterPt,
			      fns_set);
    apply_multiplicative_correction(beta_corrected_fns, beta_corrected_grads,
				    beta_corrected_hessians,
				    correctionPrevCenterPt, fns_set);
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); it++) {
      index = *it;
      Real numer =  truthFnsPrevCenter[index] - beta_corrected_fns[index];
      Real denom = alpha_corrected_fns[index] - beta_corrected_fns[index];
      combineFactors[index] = (std::fabs(denom) > 1.e-25) ? numer/denom : 1.;
#ifdef DEBUG
      Cout << "additive = " << alpha_corrected_fns[index] <<" multiplicative = "
	   << beta_corrected_fns[index] << "\nnumer = " << numer << " denom = "
	   << denom << '\n';
#endif
    }
    Cout << "\nCorrection computed: combination factors =\n" << combineFactors
	 << '\n';

#ifdef DEBUG
    Cout << "Testing final match at previous point\n";
    Response approx_copy = approx_response.copy();
    approx_copy.active_set(fns_set);
    approx_copy.function_values(approxFnsPrevCenter);
    apply_correction(approx_copy, correctionPrevCenterPt, false);
#endif
  }

  if (computeAdditive || computeMultiplicative)
    correctionComputed = true;
}


void SurrogateModel::
apply_correction(Response& approx_response, const RealVector& c_vars,
		 bool quiet_flag)
{
  if (!correctionComputed)
    return;

  const ActiveSet&  set = approx_response.active_set();
  const ShortArray& asv = set.request_vector();
  size_t i, j, k;
  ISIter it;
  int index;
  bool fn_flag = false, grad_flag = false, hess_flag = false;
  for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); it++) {
    index = *it;
    if (asv[index] & 1)
      fn_flag   = true;
    if (asv[index] & 2)
      grad_flag = true;
    if (asv[index] & 4)
      hess_flag = true;
  }

  // apply the alpha/beta corrections to the fn/grad/Hessian components
  // of approx_response
  RealVector alpha_corrected_fns, beta_corrected_fns;
  RealMatrix alpha_corrected_grads, beta_corrected_grads;
  RealSymMatrixArray alpha_corrected_hessians, beta_corrected_hessians;
  if (computeAdditive || badScalingFlag) {
    if (fn_flag)
      alpha_corrected_fns.resize(numFns);
    if (grad_flag && (correctionOrder >= 1 || combinedFlag) )
      alpha_corrected_grads.shapeUninitialized(numDerivVars, numFns);
    if (hess_flag && (correctionOrder == 2 || combinedFlag) )
      alpha_corrected_hessians.resize(numFns);
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
      index = *it;
      if (fn_flag)
	alpha_corrected_fns[index] = approx_response.function_value(index);
      if (grad_flag && (correctionOrder >= 1 || combinedFlag) )
	Teuchos::setCol(approx_response.function_gradient(index), index,
	                alpha_corrected_grads);
      if (hess_flag && (correctionOrder == 2 || combinedFlag) )
	alpha_corrected_hessians[index]=approx_response.function_hessian(index);
    }
    apply_additive_correction(alpha_corrected_fns, alpha_corrected_grads, 
			      alpha_corrected_hessians, c_vars, set);
  }
  if (computeMultiplicative && !badScalingFlag) {
    if (fn_flag)
      beta_corrected_fns.resize(numFns);
    if (grad_flag)
      beta_corrected_grads.shapeUninitialized(numDerivVars, numFns);
    if (hess_flag)
      beta_corrected_hessians.resize(numFns);
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
      index = *it;
      if (fn_flag)
	beta_corrected_fns[index] = approx_response.function_value(index);
      if (grad_flag)
	Teuchos::setCol(approx_response.function_gradient(index), index,
                        beta_corrected_grads);
      if (hess_flag)
	beta_corrected_hessians[index] =approx_response.function_hessian(index);
    }
    apply_multiplicative_correction(beta_corrected_fns, beta_corrected_grads,
				    beta_corrected_hessians, c_vars, set);
  }

  // update approx_response with the alpha/beta/combined corrected data
  if (correctionType == "additive" || badScalingFlag) { // use alpha_corrected_*
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); it++) {
      index = *it;
      if (fn_flag)
	approx_response.function_value(alpha_corrected_fns[index], index);
      if (grad_flag && correctionOrder >= 1)
        approx_response.function_gradient(Teuchos::getCol(Teuchos::View,
                                            alpha_corrected_grads, index),
                                          index);
      if (hess_flag && correctionOrder == 2)
	approx_response.function_hessian(alpha_corrected_hessians[index],index);
    }
  }
  else if (correctionType == "multiplicative") { // use beta_corrected_*
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
      index = *it;
      if (fn_flag)
	approx_response.function_value(beta_corrected_fns[index], index);
      if (grad_flag)
        approx_response.function_gradient(Teuchos::getCol(Teuchos::View,
                                            beta_corrected_grads, index),
                                          index);
      if (hess_flag)
	approx_response.function_hessian(beta_corrected_hessians[index], index);
    }
  }
  else if (combinedFlag) { // use both alpha_corrected_* and beta_corrected_*
    if (fn_flag) {
      Real corrected_fn;
      for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
	index = *it;
	if (asv[index] & 1){
	  Real cf = combineFactors[index], ccf = 1. - cf;
	  corrected_fn = cf*alpha_corrected_fns[index]
	    + ccf*beta_corrected_fns[index];
	  approx_response.function_value(corrected_fn, index);
	}
      }
    }
    if (grad_flag) {
      RealVector corrected_grad(numDerivVars);
      for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
	index = *it;
        if (asv[index] & 2) {
	  Real cf = combineFactors[index], ccf = 1. - cf;
          for (j=0; j<numDerivVars; j++)
	    corrected_grad[j] = cf*alpha_corrected_grads(j,index)
	      + ccf*beta_corrected_grads(j,index);
	  approx_response.function_gradient(corrected_grad, index);
	}
      }
    }
    if (hess_flag) {
      RealSymMatrix corrected_hessian(numDerivVars, numDerivVars);
      for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
	index = *it;
        if (asv[index] & 4) {
	  Real cf = combineFactors[index], ccf = 1. - cf;
          for (j=0; j<numDerivVars; j++)
            for (k=0; k<numDerivVars; k++)
	      corrected_hessian(j,k) = cf*alpha_corrected_hessians[index](j,k)
		+ ccf*beta_corrected_hessians[index](j,k);
	  approx_response.function_hessian(corrected_hessian, index);
	}
      }
    }
  }

  if (!quiet_flag)
    Cout << "Correction applied: corrected response =\n" << approx_response;
}


void SurrogateModel::
apply_additive_correction(RealVector& alpha_corrected_fns,
			  RealMatrix& alpha_corrected_grads,
			  RealSymMatrixArray& alpha_corrected_hessians,
			  const RealVector& c_vars, const ActiveSet& set)
{
  if (!correctionComputed)
    return;

  size_t i, j, k;
  ISIter it;
  int index;
  bool fn_flag = false, grad_flag = false, hess_flag = false;
  const ShortArray& asv = set.request_vector();
  for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); it++) {
    index = *it;
    if (asv[index] & 1)
      fn_flag   = true;
    if (asv[index] & 2)
      grad_flag = true;
    if (asv[index] & 4)
      hess_flag = true;
  }

  // Sum up the effects of separation from correctionCenterPt:
  // alpha(x) = A(x_c) + grad_A(x_c) (x-x_c) + (x-x_c)^T hess_A(x_c) (x-x_c)/2.
  // grad_alpha(x) = grad_A(x_c) + hess_A(x_c) (x-x_c)
  // hess_alpha(x) = hess_A(x_c)  [no summation needed]
  RealVector sum_alpha_fns   = addCorrFns;
  RealMatrix sum_alpha_grads = addCorrGrads;
  if (correctionOrder >= 1) { // include gradient terms
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); it++) {
      index = *it;
      for (j=0; j<numDerivVars; j++) {
	Real dist_j = c_vars[j] - correctionCenterPt[j];
	if (fn_flag) // needed only for val correction
	  sum_alpha_fns[index] += addCorrGrads(j,index) * dist_j;
	if (correctionOrder == 2) { // include Hessian terms
	  for (k=0; k<numDerivVars; k++) {
	    Real dist_k = c_vars[k] - correctionCenterPt[k];
	    if (fn_flag) // needed only for val correction
	      sum_alpha_fns[index] += dist_j * addCorrHessians[index](j,k)
		* dist_k/2.;
	    if (grad_flag) // needed only for grad correction
	      sum_alpha_grads(j,index) += addCorrHessians[index](j,k) *dist_k;
          }
        }
      }
    }
  }

  // -----------------------------
  // Additive 0th order correction
  // -----------------------------
  // for additive 0th-order, an offset is applied to the function values only
  // and there is no grad/Hessian correction.
  if (fn_flag) { // offset values
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); it++) {
      index = *it;
      if (asv[index] & 1)
	alpha_corrected_fns[index] += sum_alpha_fns[index];
    }
  }

  // -----------------------------
  // Additive 1st order correction
  // -----------------------------
  // for additive 1st-order, previous sum_alpha_fns (where the 1st-order term
  // was already included in the summation) and new sum_alpha_grads offsets
  // are applied.  There is no Hessian correction.
  if (correctionOrder >= 1 && grad_flag) { // offset gradients
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); it++) {
      index = *it;
      if (asv[index] & 2)
	for (j=0; j<numDerivVars; j++)
	  alpha_corrected_grads(j,index) += sum_alpha_grads(j,index);
    }
  }

  // -----------------------------
  // Additive 2nd order correction
  // -----------------------------
  // for additive 2nd-order, previous sum_alpha_fns and sum_alpha_grads
  // (where the 2nd-order terms were already included in the summations)
  // and new correctionHessians offsets are applied.
  if (correctionOrder == 2 && hess_flag) { // offset Hessians
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); it++) {
      index = *it;
      if (asv[index] & 4)
	for (j=0; j<numDerivVars; j++)
	  for (k=0; k<numDerivVars; k++)
	    alpha_corrected_hessians[index](j,k)+=addCorrHessians[index](j,k);
    }
  }
}


void SurrogateModel::
apply_multiplicative_correction(RealVector& beta_corrected_fns,
  RealMatrix& beta_corrected_grads,
  RealSymMatrixArray& beta_corrected_hessians,
  const RealVector& c_vars, const ActiveSet& set)
{
  if (!correctionComputed)
    return;

  size_t i, j, k;
  ISIter it;
  int index;
  bool fn_flag = false, grad_flag = false, hess_flag = false;
  const ShortArray& asv = set.request_vector();
  for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); it++) {
    index = *it;
    if (asv[index] & 1)
      fn_flag   = true;
    if (asv[index] & 2)
      grad_flag = true;
    if (asv[index] & 4)
      hess_flag = true;
  }

  // Sum up the effects of separation from correctionCenterPt:
  // beta(x)  = B(x_c) + grad_B(x_c) (x-x_c) + (x-x_c)^T hess_B(x_c) (x-x_c)/2.
  // grad_beta(x)  = grad_B(x_c) + hess_B(x_c) (x-x_c)
  // hess_beta(x)  = hess_B(x_c)  [no summation needed]
  RealVector sum_beta_fns   = multCorrFns;
  RealMatrix sum_beta_grads = multCorrGrads;
  if (correctionOrder >= 1) { // include gradient terms
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
      index = *it;
      for (j=0; j<numDerivVars; ++j) {
	Real dist_j = c_vars[j] - correctionCenterPt[j];
	// sum_beta_fns needed for val/grad/Hess correction
	sum_beta_fns[index] += multCorrGrads(j,index) * dist_j;
	if (correctionOrder == 2) { // include Hessian terms
	  for (k=0; k<numDerivVars; ++k) {
	    Real dist_k = c_vars[k] - correctionCenterPt[k];
	    // sum_beta_fns needed for val/grad/Hess correction
	    sum_beta_fns[index] += dist_j * multCorrHessians[index](j,k)
	      * dist_k/2.;
	    if (grad_flag || hess_flag) // needed for grad/Hess correction
	      sum_beta_grads(j,index) += multCorrHessians[index](j,k)*dist_k;
          }
        }
      }
    }
  }

  // -----------------------------------------------------------------------
  // Retrieve uncorrected data for special cases where the data required to
  // apply the correction is different from the active data being corrected.
  // -----------------------------------------------------------------------
  // Retrieve the uncorrected fn values for use in gradient and Hessian
  // corrections.  The mathematically correct approach is to use the fn
  // values for the current point in parameter space, and this approach is
  // used if these fn values are available (either directly as indicated by
  // the asv or through a data_pairs search).  A fall-back position if all
  // else fails is to use the fn values from the center of the current trust
  // region.  This still satisifies consistency at the center of the trust
  // region, but is a less accurate approximation over the rest of the trust
  // region.  The data_pairs search will be needed in cases where the
  // correction is applied only to the fn gradient (i.e., when the current
  // asv contains 2's due to DOT/CONMIN/OPT++ requesting gradients
  // separately), but the fall-back position should not occur in practice.
  RealVector uncorrected_fns;
  if ( ( correctionOrder >= 1 && grad_flag) ||
       ( correctionOrder == 2 && hess_flag ) ) {
    if (beta_corrected_fns.empty())
      uncorrected_fns.resize(numFns);
    else
      uncorrected_fns = beta_corrected_fns;
    bool db_search = false;
    ShortArray search_asv(numFns, 0);
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
      index = *it;
      if ( asv[index] && !(asv[index] & 1) ) {
	search_asv[index] = 1;
	db_search = true;
      }
    }
    if (db_search) {
      // query data_pairs to extract the fn values at the current pt
      Response desired_resp;
      Variables search_vars = currentVariables.copy();
      search_vars.continuous_variables(c_vars);
      ActiveSet search_set = set;
      search_set.request_vector(search_asv);
      extern PRPCache data_pairs; // global container
      if ( lookup_by_val(data_pairs, surrogate_model().interface_id(),
			 search_vars, search_set, desired_resp) ) {
	const RealVector& dp_fns = desired_resp.function_values();
	for (i=0; i<numFns; ++i)
	  if (search_asv[i])
	    uncorrected_fns[i] = dp_fns[i];
      }
      else { // last resort *** TO DO: perform approx fn eval instead
	Cerr << "Warning: current function values not available.\n         "
	     << "beta correction using function values from the correction "
	     << "point." << std::endl;
	for (i=0; i<numFns; ++i)
	  if (search_asv[i])
	    uncorrected_fns[i] = approxFnsCenter[i];
      }
    }
  }

  // Retrieve the uncorrected fn gradients for use in Hessian corrections.
  // The mathematically correct approach is to use the fn grads for the 
  // current point in parameter space, and this approach is used if these fn
  // grads are available (either directly as indicated by the asv or through a
  // data_pairs search). A fall-back position if all else fails is to use the
  // fn grads from the center of the current trust region.  This still
  // satisifies consistency at the center of the trust region, but is a less
  // accurate approximation over the rest of the trust region.  The data_pairs
  // search will be needed in cases where the correction is applied only to
  // the fn Hessian (i.e., when the current asv contains 4's due to OPT++
  // requesting Hessians separately), but the fall-back position should not
  // occur in practice.
  RealMatrix uncorrected_grads;
  if (correctionOrder >= 1 && hess_flag) {
    if (beta_corrected_grads.empty())
      uncorrected_grads.shapeUninitialized(numDerivVars, numFns);
    else
      uncorrected_grads = beta_corrected_grads;
    bool db_search = false;
    ShortArray search_asv(numFns, 0);
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
      index = *it;
      if ( (asv[index] & 4) && !(asv[index] & 2) ) {
	search_asv[index] = 2;
	db_search = true;
      }
    }
    if (db_search) {
      // query data_pairs to extract the fn gradients at the current pt
      Response desired_resp;
      Variables search_vars = currentVariables.copy();
      search_vars.continuous_variables(c_vars);
      ActiveSet search_set = set;
      search_set.request_vector(search_asv);
      extern PRPCache data_pairs; // global container
      if( lookup_by_val(data_pairs, surrogate_model().interface_id(),
			search_vars, search_set, desired_resp) ) {
	//const RealMatrix& dp_grads = desired_resp.function_gradients();
	for (i=0; i<numFns; ++i)
	  if (search_asv[i])
	    Teuchos::setCol(desired_resp.function_gradient(i), (int)i,
                            uncorrected_grads);
      }
      else { // last resort *** TO DO: perform approx fn eval instead
	Cerr << "Warning: current function gradients not available.\n"
	     << "         beta correction using function gradients from the"
	     << " correction point." << std::endl;
	for (i=0; i<numFns; ++i)
	  if (search_asv[i])
	    Teuchos::setCol(Teuchos::getCol(Teuchos::View, approxGradsCenter,
	                                    (int)i),
	                    (int)i, uncorrected_grads);
      }
    }
  }

  // ------------------------------
  // All multiplicative corrections
  // ------------------------------
  if (fn_flag) { // scale values by sum_beta_fns for all correction orders
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
      index = *it;
      if (asv[index] & 1)
	beta_corrected_fns[index] *= sum_beta_fns[index];
    }
  }

  if (grad_flag) { // capture common term: sum_beta_fns * uncorrected_grads
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
      index = *it;
      if (asv[index] & 2)
	for (j=0; j<numDerivVars; j++)
	  beta_corrected_grads(j,index) *= sum_beta_fns[index];
    }
  }

  if (hess_flag) { // capture common term: sum_beta_fns * uncorrected_hessians
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
      index = *it;
      if (asv[index] & 4)
	for (j=0; j<numDerivVars; j++)
	  for (k=0; k<numDerivVars; k++)
	    beta_corrected_hessians[index](j,k) *= sum_beta_fns[index];
    }
  }

  // -----------------------------------
  // Multiplicative 0th order correction
  // -----------------------------------
  //if (correctionOrder == 0) {
    // in the 0th-order case, the common term in the baseline grad/Hessian
    // corrections is the complete correction.
  //}

  // -----------------------------------
  // Multiplicative 1st order correction
  // -----------------------------------
  if (correctionOrder >= 1) {
    if (grad_flag) { // add one term to the base beta_corrected_grads
      for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
	index = *it;
	if (asv[index] & 2)
	  for (j=0; j<numDerivVars; j++)
	    beta_corrected_grads(j,index) += sum_beta_grads(j,index)
                                          *  uncorrected_fns[index];
      }
    }
    if (hess_flag) { // add two terms to the base beta_corrected_hessians
      for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
	index = *it;
	if (asv[index] & 4)
	  for (j=0; j<numDerivVars; j++)
	    for (k=0; k<numDerivVars; k++)
	      beta_corrected_hessians[index](j,k)
		+= sum_beta_grads(j,index) * uncorrected_grads(k,index)
		+  uncorrected_grads(j,index) * sum_beta_grads(k,index);
      }
    }
  }

  // -----------------------------------
  // Multiplicative 2nd order correction
  // -----------------------------------
  if (correctionOrder == 2 && hess_flag) { // add one final term
    for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
      index = *it;
      if (asv[index] & 4)
	for (j=0; j<numDerivVars; j++)
	  for (k=0; k<numDerivVars; k++)
	    beta_corrected_hessians[index](j,k) += uncorrected_fns[index]
	                                        *  multCorrHessians[index](j,k);
    }
  }
}

} // namespace Dakota
