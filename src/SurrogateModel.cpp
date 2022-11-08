/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
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
  surrogateFnIndices(problem_db.get_szs("model.surrogate.function_indices")),
  corrType(problem_db.get_short("model.surrogate.correction_type")),
  corrOrder(problem_db.get_short("model.surrogate.correction_order")),
  surrModelEvalCntr(0), approxBuilds(0)
{
  // assign default responseMode based on correction specification;
  // NO_CORRECTION (0) is default
  responseMode = (corrType) ? AUTO_CORRECTED_SURROGATE : UNCORRECTED_SURROGATE;

  // process surrogateFnIndices. IntSets are sorted and unique.
  if (surrogateFnIndices.empty()) // default: all fns are approximated
    for (size_t i=0; i<numFns; ++i)
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
	       const SharedVariablesData& svd, bool share_svd,
	       const SharedResponseData&  srd, bool share_srd,
	       const ActiveSet& set, short corr_type, short output_level):
  // Allow DFSModel to employ sizing differences (e.g., consuming aggregations)
  Model(LightWtBaseConstructor(), svd, share_svd, srd, share_srd, set,
	output_level, problem_db, parallel_lib),
  corrType(corr_type), corrOrder(0), surrModelEvalCntr(0), approxBuilds(0)
{
  modelType = "surrogate";

  // assign default responseMode based on correction specification;
  // NO_CORRECTION (0) is default
  responseMode = (corrType) ? AUTO_CORRECTED_SURROGATE : UNCORRECTED_SURROGATE;

  // set up surrogateFnIndices to use default (all fns are approximated)
  for (size_t i=0; i<numFns; ++i)
    surrogateFnIndices.insert(i);
}


bool SurrogateModel::check_active_variables(const Model& sub_model)
{
  // Check for compatible array sizing between sub_model and currentVariables,
  // accounting for the use of different views in different variables sets:
  // > common case for local/multipoint/hierarchical: the variables view in
  //   sub_model and currentVariables are the same.
  // > common case for global: sub_model has an "All" vars view due to DACE
  //   usage and the currentVariables view may vary depending on the type
  //   of iterator interfaced with this SurrogateModel.  Enforcing an "all"
  //   view in the data returned from currentVariables ensures consistency.
  short active_view = currentVariables.view().first,
     sm_active_view = sub_model.current_variables().view().first;
  bool error_flag = false;
  if ( active_view == sm_active_view ) {
    // common cases: Distinct on Distinct (e.g., opt/UQ on local/multipt/hier)
    //               All on All           (e.g., DACE/PStudy on global)
    size_t sm_cv = sub_model.cv(),  sm_div = sub_model.div(),
      sm_dsv = sub_model.dsv(),     sm_drv = sub_model.drv(),
      cv  = currentVariables.cv(),  div = currentVariables.div(),
      dsv = currentVariables.dsv(), drv = currentVariables.drv();
    if (sm_cv != cv || sm_div != div || sm_dsv != dsv || sm_drv != drv) {
      Cerr << "Error: incompatibility between approximate and actual model "
	   << "variable sets within SurrogateModel:\n       Active approximate "
	   << "= " << cv << " continuous, " << div << " discrete int, " << dsv
	   << " discrete string, and " << drv << " discrete real.\n       "
	   << "Active      actual = " << sm_cv << " continuous, " << sm_div
	   << " discrete int, " << sm_dsv << " discrete string, and " << sm_drv
	   << " discrete real.\n       Check consistency of variables "
	   << "specifications." << std::endl;
      error_flag = true;
    }
  }
  else if ( ( sm_active_view == RELAXED_ALL || sm_active_view == MIXED_ALL ) &&
	    active_view >= RELAXED_DESIGN ) {
    // common case: Distinct on All (e.g., opt/UQ on global surrogate)
    size_t sm_cv  = sub_model.cv(),   sm_div = sub_model.div(),
      sm_dsv  = sub_model.dsv(),      sm_drv = sub_model.drv(),
      acv  = currentVariables.acv(),  adiv = currentVariables.adiv(),
      adsv = currentVariables.adsv(), adrv = currentVariables.adrv();
    if (sm_cv != acv || sm_div != adiv || sm_dsv != adsv || sm_drv != adrv) {
      Cerr << "Error: incompatibility between approximate and actual model "
	   << "variable sets within SurrogateModel:\n       Active "
	   << "approximate = " << acv << " continuous, " << adiv
	   << " discrete int, " << adsv << " discrete string, and " << adrv
	   << " discrete real (All view).\n       Active      actual = "
	   << sm_cv << " continuous, " << sm_div << " discrete int, " << sm_dsv
	   << " discrete string, and " << sm_drv << " discrete real.\n       "
	   << "Check consistency of variables specifications."<< std::endl;
      error_flag = true;
    }
  }
  else if ( ( active_view == RELAXED_ALL || active_view == MIXED_ALL ) &&
	    sm_active_view >= RELAXED_DESIGN ) {
    // common case: All on Distinct (e.g., DACE/PStudy on local/multipt/hier)
    // Note: force_rebuild() is critical for this case (prevents interrogation
    // of a surrogate for inconsistent values for vars not included in build)
    size_t sm_acv = sub_model.acv(), sm_adiv = sub_model.adiv(),
      sm_adsv = sub_model.adsv(),    sm_adrv = sub_model.adrv(),
      cv  = currentVariables.cv(),   div = currentVariables.div(),
      dsv = currentVariables.dsv(),  drv = currentVariables.drv();
    if (sm_acv != cv || sm_adiv != div || sm_adsv != dsv || sm_adrv != drv) {
      Cerr << "Error: incompatibility between approximate and actual model "
	   << "variable sets within SurrogateModel:\n       Active "
	   << "approximate = " << cv << " continuous, " << div
	   << " discrete int, " << dsv << " discrete string, and " << drv
	   << " discrete real.\n       Active      actual = " << sm_acv
	   << " continuous, " << sm_adiv << " discrete int, " << sm_adsv
	   << " discrete string, and " << sm_adrv << " discrete real (All "
	   << "view).\n       Check consistency of variables specifications."
	   << std::endl;
      error_flag = true;
    }
  }
  //else: other options are specific to DataFit and Hierarch surrogates

  return error_flag;
}


bool SurrogateModel::check_inactive_variables(const Model& sub_model)
{
  bool error_flag = false;
  // TO DO: Bayes exp design (hi2lo) introduces new requirements on a
  // hierarchical model, and MF active subspaces will as well.
  // > For (simulation-based) OED, one option is to enforce consistency in
  //   inactive state (config vars) and allow active parameterization to vary.
  // > For hi2lo, this implies that the active variable subset could be null
  //   for HF, as the active calibration variables only exist for LF.
  size_t sm_icv = sub_model.icv(),  sm_idiv = sub_model.idiv(),
    sm_idsv = sub_model.idsv(),     sm_idrv = sub_model.idrv(),
    icv  = currentVariables.icv(),  idiv = currentVariables.idiv(),
    idsv = currentVariables.idsv(), idrv = currentVariables.idrv();
  if (sm_icv != icv || sm_idiv != idiv || sm_idsv != idsv || sm_idrv != idrv) {
    Cerr << "Error: incompatibility between subordinate and aggregate model "
	 << "variable sets within\n       SurrogateModel: inactive "
	 << "subordinate = " << icv << " continuous, " << idiv
	 << " discrete int, " << idsv << " discrete string, and " << idrv
	 << " discrete real and\n       inactive aggregate = " << sm_icv
	 << " continuous, " << sm_idiv << " discrete int, " << sm_idsv
	 << " discrete string, and " << sm_idrv << " discrete real.  Check "
	 << "consistency of variables specifications." << std::endl;
    error_flag = true;
  }
  return error_flag;
}


bool SurrogateModel::check_response_qoi(const Model& sub_model)
{
  bool error_flag = false;
  // Check for compatible array sizing between sub_model and currentResponse.
  // NonHierarchSurrModel creates aggregations (and a DataFitSurrModel will
  // consume them). Aggregations may span truthModel, unorderedModels, or both.
  // For now, allow any aggregation factor.
  size_t sm_qoi = sub_model.qoi();//, aggregation = numFns / sm_qoi;
  if ( numFns % sm_qoi ) {
    Cerr << "Error: incompatibility between subordinate and aggregate model "
	 << "response function sets\n       within SurrogateModel: " << numFns
	 << " aggregate and " << sm_qoi << " subordinate functions.\n       "
	 << "Check consistency of responses specifications." << std::endl;
    error_flag = true;
  }
  return error_flag;
}


void SurrogateModel::init_model(Model& model)
{
  init_model_constraints(model);
  init_model_labels(model);

  // A push down of inactive vars data can disrupt subsequent calls to
  // update_from_subordinate_model() in surrogate-based methods with local
  // DataFit instantiations, such as local reliability, expansion UQ, SBO, etc.
  // It is therefore not included in the base-class default implementation.
  //init_model_inactive_variables(model);
  init_model_inactive_labels(model);
}


/** Update variables and constraints data within model using
    values and labels from currentVariables and bound/linear/nonlinear
    constraints from userDefinedConstraints. */
void SurrogateModel::init_model_constraints(Model& model)
{
  if (model.is_null()) // possible for DataFitSurrModel
    return;

  size_t num_cv  = currentVariables.cv(),  num_div = currentVariables.div(),
         num_drv = currentVariables.drv(), num_dsv = currentVariables.dsv(),
      num_sm_cv  = model.cv(),          num_sm_div = model.div(),
      num_sm_dsv = model.dsv(),         num_sm_drv = model.drv();

  // linear constraints (apply to active cv,div,drv)

  bool lin_ineq = (userDefinedConstraints.num_linear_ineq_constraints() > 0),
       lin_eq   = (userDefinedConstraints.num_linear_eq_constraints()   > 0);
  if ( (lin_ineq || lin_eq) && (model.cv()  != currentVariables.cv()  ||
				model.div() != currentVariables.div() ||
				model.drv() != currentVariables.drv()) ) {
    // the views don't necessarily have to be the same, but number of active
    // continuous and active discrete int,real variables have to be consistent
    Cerr << "Error: cannot update linear constraints in SurrogateModel::"
	 << "init_model() due to inconsistent active variables." << std::endl;
    abort_handler(MODEL_ERROR);
  }
  if (lin_ineq) {
    model.linear_ineq_constraint_coeffs(
      userDefinedConstraints.linear_ineq_constraint_coeffs());
    model.linear_ineq_constraint_lower_bounds(
      userDefinedConstraints.linear_ineq_constraint_lower_bounds());
    model.linear_ineq_constraint_upper_bounds(
      userDefinedConstraints.linear_ineq_constraint_upper_bounds());
  }
  if (lin_eq) {
    model.linear_eq_constraint_coeffs(
      userDefinedConstraints.linear_eq_constraint_coeffs());
    model.linear_eq_constraint_targets(
      userDefinedConstraints.linear_eq_constraint_targets());
  }

  // nonlinear constraints

  if (userDefinedConstraints.num_nonlinear_ineq_constraints()) {
    model.nonlinear_ineq_constraint_lower_bounds(
      userDefinedConstraints.nonlinear_ineq_constraint_lower_bounds());
    model.nonlinear_ineq_constraint_upper_bounds(
      userDefinedConstraints.nonlinear_ineq_constraint_upper_bounds());
  }
  if (userDefinedConstraints.num_nonlinear_eq_constraints())
    model.nonlinear_eq_constraint_targets(
      userDefinedConstraints.nonlinear_eq_constraint_targets());
}


void SurrogateModel::init_model_labels(Model& model)
{
  if (approxBuilds) return;

  // labels: update model with current{Variables,Response} descriptors
  // inactive vars / bounds: propagate inactive vars when necessary

  if (model.response_labels().empty()) // should not happen
    switch (responseMode) {
    case AGGREGATED_MODELS: case AGGREGATED_MODEL_PAIR: {
      StringArray qoi_labels;
      copy_data_partial(currentResponse.function_labels(),
			0, model.qoi(), qoi_labels);
      model.response_labels(qoi_labels);
      break;
    }
    default:
      model.response_labels(currentResponse.function_labels()); break;
    }

  // ***************************************************************************
  // TO DO: this needs better rigor now that we are allowing dissimilar
  // sub-model parameterizations:
  // > only assign label sets of matched size when the target labels were
  //   default-generated by the parser, as this was the intended purpose:
  //   to allow the user to avoid unnecessary replication or redundant info.
  // > Need to create an array of flags for default label generation, and allow
  //   convenient lookups that parallel the label accessors (may take some time)
  // > If they were user-specified for the target model, DO NOT OVERWRITE THEM.
  //
  // Note that this will occur upstream of EnsembleSurrModel::
  // init_model_mapped_variables(), which depends on these updated target labels
  // ***************************************************************************

  // Set the incoming model variable descriptors with the variable descriptors
  // from currentVariables (eliminates need to replicate descriptors in the
  // input file).  This only needs to be performed once.  However, performing
  // this in the ctor does not propagate properly for multiple surrogates /
  // nestings since the sub-model construction (and therefore any sub-sub-model
  // constructions) must finish before calling any set functions on it.  That
  // is, after-the-fact updating in constructors only propagates one level,
  // whereas before-the-fact updating in compute/build functions propagates
  short active_view = currentVariables.view().first,
     sm_active_view = model.current_variables().view().first;
  bool active_all = (active_view == RELAXED_ALL || active_view == MIXED_ALL),
    sm_active_all = (sm_active_view == RELAXED_ALL ||
		     sm_active_view == MIXED_ALL);
  if (active_view == sm_active_view) {
    // update active model vars with active currentVariables data
    model.continuous_variable_labels(
      currentVariables.continuous_variable_labels());
    model.discrete_int_variable_labels(
      currentVariables.discrete_int_variable_labels());
    model.discrete_string_variable_labels(
      currentVariables.discrete_string_variable_labels());
    model.discrete_real_variable_labels(
      currentVariables.discrete_real_variable_labels());
    /* Now supported by init_model_inactive_labels()
    if (!active_all) { // models not in ALL view
      model.inactive_continuous_variable_labels(
        currentVariables.inactive_continuous_variable_labels());
      model.inactive_discrete_int_variable_labels(
        currentVariables.inactive_discrete_int_variable_labels());
      model.inactive_discrete_string_variable_labels(
        currentVariables.inactive_discrete_string_variable_labels());
      model.inactive_discrete_real_variable_labels(
        currentVariables.inactive_discrete_real_variable_labels());
    }
    */
  }
  else if (!active_all && sm_active_all) { // update active from "All" view
    model.continuous_variable_labels(
      currentVariables.all_continuous_variable_labels());
    model.discrete_int_variable_labels(
      currentVariables.all_discrete_int_variable_labels());
    model.discrete_string_variable_labels(
      currentVariables.all_discrete_string_variable_labels());
    model.discrete_real_variable_labels(
      currentVariables.all_discrete_real_variable_labels());
  }
  else if (!sm_active_all && active_all) { // update "All" view from active
    // TO DO: only update the active labels in model (not all labels)
    model.all_continuous_variable_labels(
      currentVariables.continuous_variable_labels());
    model.all_discrete_int_variable_labels(
      currentVariables.discrete_int_variable_labels());
    model.all_discrete_string_variable_labels(
      currentVariables.discrete_string_variable_labels());
    model.all_discrete_real_variable_labels(
      currentVariables.discrete_real_variable_labels());
  }
}


void SurrogateModel::init_model_inactive_variables(Model& model)
{
  short active_view = currentVariables.view().first,
     sm_active_view = model.current_variables().view().first;
  bool active_all = (active_view == RELAXED_ALL || active_view == MIXED_ALL);
  if ( (active_view == sm_active_view) && !active_all) {
    // update model with inactive currentVariables/userDefinedConstraints
    // data. For efficiency, we avoid doing this on every evaluation, instead
    // calling it from a pre-execution initialization context.
    size_t num_icv = currentVariables.icv(),
      num_idiv = currentVariables.idiv(), num_idrv = currentVariables.idrv(),
      num_idsv = currentVariables.idsv();
    if (num_icv && num_icv == model.icv()) {// not enforced in check_sm_compat
      model.inactive_continuous_variables(
        currentVariables.inactive_continuous_variables());
      model.inactive_continuous_lower_bounds(
        userDefinedConstraints.inactive_continuous_lower_bounds());
      model.inactive_continuous_upper_bounds(
        userDefinedConstraints.inactive_continuous_upper_bounds());
    }
    if (num_idiv && num_idiv == model.idiv()) { // not enforced previously
      model.inactive_discrete_int_variables(
        currentVariables.inactive_discrete_int_variables());
      model.inactive_discrete_int_lower_bounds(
        userDefinedConstraints.inactive_discrete_int_lower_bounds());
      model.inactive_discrete_int_upper_bounds(
        userDefinedConstraints.inactive_discrete_int_upper_bounds());
    }
    if (num_idsv && num_idsv == model.idsv())  // not enforced previously
      model.inactive_discrete_string_variables(
        currentVariables.inactive_discrete_string_variables());
    if (num_idrv && num_idrv == model.idrv()) { // not enforced previously
      model.inactive_discrete_real_variables(
        currentVariables.inactive_discrete_real_variables());
      model.inactive_discrete_real_lower_bounds(
        userDefinedConstraints.inactive_discrete_real_lower_bounds());
      model.inactive_discrete_real_upper_bounds(
        userDefinedConstraints.inactive_discrete_real_upper_bounds());
    }
  }
}


void SurrogateModel::init_model_inactive_labels(Model& model)
{
  if (approxBuilds) return;

  // ***************************************************************************
  // TO DO: this needs better rigor now that we allow dissimilar sub-model
  // parameterizations:
  // > only assign label sets of matched size when the target labels were
  //   default-generated by the parser, as this was the intended purpose:
  //   to allow the user to avoid unnecessary replication or redundant info.
  // > Need to create an array of flags for default label generation, and allow
  //   convenient lookups that parallel the label accessors (may take some time)
  // > If they were user-specified for the target model, DO NOT OVERWRITE THEM.
  //
  // Note that this will occur upstream of EnsembleSurrModel::
  // init_model_mapped_variables(), which depends on these updated target labels
  // ***************************************************************************

  short active_view = currentVariables.view().first,
     sm_active_view = model.current_variables().view().first;
  bool active_all = (active_view == RELAXED_ALL || active_view == MIXED_ALL),
    sm_active_all = (sm_active_view == RELAXED_ALL ||
		     sm_active_view == MIXED_ALL);
  if (active_view == sm_active_view && !active_all) { // models not in ALL view
    // Can't use inactive label matching since that is what we are updating,
    // so rely only on counts for now.
    size_t num_icv = currentVariables.icv(),
      num_idiv = currentVariables.idiv(), num_idrv = currentVariables.idrv(),
      num_idsv = currentVariables.idsv();
    if (num_icv && num_icv == model.icv())
      model.inactive_continuous_variable_labels(
        currentVariables.inactive_continuous_variable_labels());
    if (num_idiv && num_idiv == model.idiv())
      model.inactive_discrete_int_variable_labels(
        currentVariables.inactive_discrete_int_variable_labels());
    if (num_idsv && num_idsv == model.idsv())
      model.inactive_discrete_string_variable_labels(
        currentVariables.inactive_discrete_string_variable_labels());
    if (num_idrv && num_idrv == model.idrv())
      model.inactive_discrete_real_variable_labels(
        currentVariables.inactive_discrete_real_variable_labels());
  }
  else if (!active_all && sm_active_all) {
    // nothing to do currenty for this case prior to more fine-grained handling
    // of active labels (all model labels are currently updated)
  }
  else if (!sm_active_all && active_all) {
    // nothing to do currenty for this case prior to more fine-grained handling
    // of active labels (all model labels are currently updated)
  }
}


void SurrogateModel::update_model(Model& model)
{
  if (model.is_null()) return; // possible for DataFitSurrModel
  update_model_active_variables(model); // default operation
  //update_model_distributions(model);
}


void SurrogateModel::update_model_active_variables(Model& model)
{
  // vars/bounds/labels

  short active_view = currentVariables.view().first,
     sm_active_view = model.current_variables().view().first;
  bool active_all = (active_view == RELAXED_ALL || active_view == MIXED_ALL),
    sm_active_all = (sm_active_view == RELAXED_ALL ||
		     sm_active_view == MIXED_ALL);
  // Update model variables, bounds, and labels in all view cases.
  // Note 1: bounds updating isn't strictly required for local/multipoint, but
  // is needed for global and could be relevant in cases where model
  // involves additional surrogates/nestings.
  // Note 2: label updating eliminates the need to replicate variable
  // descriptors, e.g., in SBOUU input files.  It only needs to be performed
  // once (as opposed to the update of vars and bounds).  However, performing
  // this updating in the constructor does not propagate properly for multiple
  // surrogates/nestings since the sub-model construction (and therefore any
  // sub-sub-model constructions) must finish before calling any set functions
  // on it.  That is, after-the-fact updating in constructors only propagates
  // one level, whereas before-the-fact updating in compute/build functions
  // propagates multiple levels.
  if (active_view == sm_active_view) {
    // update active model vars/cons with active currentVariables data
    // Note: inactive vals/bnds/labels and linear/nonlinear constr coeffs/bnds
    //       updated in init_model()
    if (currentVariables.cv()) {
      model.continuous_variables(currentVariables.continuous_variables());
      model.continuous_lower_bounds(
        userDefinedConstraints.continuous_lower_bounds());
      model.continuous_upper_bounds(
        userDefinedConstraints.continuous_upper_bounds());
    }
    if (currentVariables.div()) {
      model.discrete_int_variables(
        currentVariables.discrete_int_variables());
      model.discrete_int_lower_bounds(
        userDefinedConstraints.discrete_int_lower_bounds());
      model.discrete_int_upper_bounds(
        userDefinedConstraints.discrete_int_upper_bounds());
    }
    if (currentVariables.dsv())
      model.discrete_string_variables(
        currentVariables.discrete_string_variables());
    if (currentVariables.drv()) {
      model.discrete_real_variables(
        currentVariables.discrete_real_variables());
      model.discrete_real_lower_bounds(
        userDefinedConstraints.discrete_real_lower_bounds());
      model.discrete_real_upper_bounds(
        userDefinedConstraints.discrete_real_upper_bounds());
    }
  }
  else if (!active_all && sm_active_all) {
    // update active model vars/cons using "All" view of
    // currentVariables/userDefinedConstraints data
    if (currentVariables.acv()) {
      model.continuous_variables(
        currentVariables.all_continuous_variables());
      model.continuous_lower_bounds(
        userDefinedConstraints.all_continuous_lower_bounds());
      model.continuous_upper_bounds(
        userDefinedConstraints.all_continuous_upper_bounds());
    }
    if (currentVariables.adiv()) {
      model.discrete_int_variables(
        currentVariables.all_discrete_int_variables());
      model.discrete_int_lower_bounds(
        userDefinedConstraints.all_discrete_int_lower_bounds());
      model.discrete_int_upper_bounds(
        userDefinedConstraints.all_discrete_int_upper_bounds());
    }
    if (currentVariables.adsv())
      model.discrete_string_variables(
        currentVariables.all_discrete_string_variables());
    if (currentVariables.adrv()) {
      model.discrete_real_variables(
        currentVariables.all_discrete_real_variables());
      model.discrete_real_lower_bounds(
        userDefinedConstraints.all_discrete_real_lower_bounds());
      model.discrete_real_upper_bounds(
        userDefinedConstraints.all_discrete_real_upper_bounds());
    }
  }
  else if (!sm_active_all && active_all) {
    // update "All" view of model vars/cons using active
    // currentVariables/userDefinedConstraints data
    if (currentVariables.cv()) {
      model.all_continuous_variables(
        currentVariables.continuous_variables());
      model.all_continuous_lower_bounds(
        userDefinedConstraints.continuous_lower_bounds());
      model.all_continuous_upper_bounds(
        userDefinedConstraints.continuous_upper_bounds());
    }
    if (currentVariables.div()) {
      model.all_discrete_int_variables(
        currentVariables.discrete_int_variables());
      model.all_discrete_int_lower_bounds(
        userDefinedConstraints.discrete_int_lower_bounds());
      model.all_discrete_int_upper_bounds(
        userDefinedConstraints.discrete_int_upper_bounds());
    }
    if (currentVariables.dsv())
      model.all_discrete_string_variables(
        currentVariables.discrete_string_variables());
    if (currentVariables.drv()) {
      model.all_discrete_real_variables(
        currentVariables.discrete_real_variables());
      model.all_discrete_real_lower_bounds(
        userDefinedConstraints.discrete_real_lower_bounds());
      model.all_discrete_real_upper_bounds(
        userDefinedConstraints.discrete_real_upper_bounds());
    }
  }
  // TO DO: extend for aleatory/epistemic uncertain views
  else {
    Cerr << "Error: unsupported variable view differences in "
	 << "SurrogateModel::update_model()" << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


void SurrogateModel::update_model_distributions(Model& model)
{
  // uncertain variable distribution data (dependent on label updates above)

  // Note: Variables instances defined from the same variablesId are not shared
  // (see ProblemDescDB::get_variables()), so we propagate any distribution
  // updates (e.g., NestedModel insertions) up/down the Model recursion.  For
  // differing variablesId, we assume that the distribution information can be
  // mapped when a variable label is matched, but this precludes the case
  // where the distribution for the same variable differs between that used to
  // build and that used to evaluate.   More careful logic could involve
  // matching both variable label and distribution type (presumably the dist
  // params would be consistent when the dist type is the same), and this could
  // be implemented at the lower (MultivariateDistribution) level.
  // > currentVariables may have different active view from incoming model
  //   vars, but MultivariateDistribution updates can be performed for all
  //   vars (independent of view)
  // > when model is a ProbabilityTransformModel, its mvDist is in u-space.
  //   DataFit operates in and pushes updates to this transformed space for
  //   parameterized std distribs (e.g. {JACOBI,GEN_LAGUERE,NUM_GEN}_ORTHOG).
  // > it is sufficient to pull parameters at initialize_mapping() time, as
  //   this data varies per iterator execution rather than per-evaluation
  const SharedVariablesData&   svd =          currentVariables.shared_data();
  const SharedVariablesData& m_svd = model.current_variables().shared_data();
  if (svd.id() == m_svd.id()) // same set of variables
    model.multivariate_distribution().pull_distribution_parameters(mvDist);
  else { // map between related sets of variables based on labels
    StringArray pull_labels;    svd.assemble_all_labels(pull_labels);
    StringArray push_labels;  m_svd.assemble_all_labels(push_labels);
    model.multivariate_distribution().
      pull_distribution_parameters(mvDist, pull_labels, push_labels);
  }
}


/** Update values and labels in currentVariables and
    bound/linear/nonlinear constraints in userDefinedConstraints from
    variables and constraints data within model. */
void SurrogateModel::update_from_model(const Model& model)
{
  if (model.is_null()) return; // possible for DataFitSurrModel
  update_variables_from_model(model);
  //update_distributions_from_model(model);
  update_response_from_model(model);
}


void SurrogateModel::update_variables_from_model(const Model& model)
{
  // vars/bounds/labels

  if (currentVariables.variables_id()==model.current_variables().variables_id())
    update_all_variables_from_model(model);
  else // fine-grained update based on label lookups
    update_complement_variables_from_model(model);
}


void SurrogateModel::update_distributions_from_model(const Model& model)
{
  // uncertain variable distribution data (dependent on label updates above)

  // See notes in init_model() above, with the difference that these
  // updates are performed once at lightweight construct time.
  const SharedVariablesData&   svd =          currentVariables.shared_data();
  const SharedVariablesData& m_svd = model.current_variables().shared_data();
  if (svd.id() == m_svd.id()) // same variables specification
    mvDist.pull_distribution_parameters(model.multivariate_distribution());
  else { // map between related sets of variables based on labels
    StringArray pull_labels;  m_svd.assemble_all_labels(pull_labels);
    StringArray push_labels;    svd.assemble_all_labels(push_labels);
    mvDist.pull_distribution_parameters(model.multivariate_distribution(),
					pull_labels, push_labels);
  }
}


void SurrogateModel::update_response_from_model(const Model& model)
{
  if (!approxBuilds &&
      currentResponse.function_labels().empty()) // should not happen
    switch (responseMode) {
    case AGGREGATED_MODELS: case AGGREGATED_MODEL_PAIR: {
      const StringArray& model_labels = model.response_labels();
      size_t i, start = 0, num_fns = currentResponse.num_functions(),
	qoi = model.qoi(), num_repl = num_fns / qoi;
      StringArray repl_labels(num_fns);
      for (i=0; i<num_repl; ++i, start+=qoi)
	copy_data_partial(model_labels, repl_labels, start);
      currentResponse.function_labels(repl_labels);
      break;
    }
    default:
      currentResponse.function_labels(model.response_labels()); break;
    }

  // weights and sense for primary response functions

  primaryRespFnWts   = model.primary_response_fn_weights();
  primaryRespFnSense = model.primary_response_fn_sense();

  // linear constraints

  bool lin_ineq = (model.num_linear_ineq_constraints() > 0),
       lin_eq   = (model.num_linear_eq_constraints()   > 0);
  if ( (lin_ineq || lin_eq) && (model.cv()  != currentVariables.cv()  ||
				model.div() != currentVariables.div() ||
				model.drv() != currentVariables.drv()) ) {
    // the views don't necessarily have to be the same, but the number of
    // active continuous and active discrete variables have to be consistent
    Cerr << "Error: cannot update linear constraints in SurrogateModel::update"
	 << "_from_model() due to inconsistent active variables." << std::endl;
    abort_handler(MODEL_ERROR);
  }
  if (lin_ineq) {
    userDefinedConstraints.linear_ineq_constraint_coeffs(
      model.linear_ineq_constraint_coeffs());
    userDefinedConstraints.linear_ineq_constraint_lower_bounds(
      model.linear_ineq_constraint_lower_bounds());
    userDefinedConstraints.linear_ineq_constraint_upper_bounds(
      model.linear_ineq_constraint_upper_bounds());
  }
  if (lin_eq) {
    userDefinedConstraints.linear_eq_constraint_coeffs(
      model.linear_eq_constraint_coeffs());
    userDefinedConstraints.linear_eq_constraint_targets(
      model.linear_eq_constraint_targets());
  }

  // nonlinear constraints

  if (model.num_nonlinear_ineq_constraints()) {
    userDefinedConstraints.nonlinear_ineq_constraint_lower_bounds(
      model.nonlinear_ineq_constraint_lower_bounds());
    userDefinedConstraints.nonlinear_ineq_constraint_upper_bounds(
      model.nonlinear_ineq_constraint_upper_bounds());
  }
  if (model.num_nonlinear_eq_constraints())
    userDefinedConstraints.nonlinear_eq_constraint_targets(
      model.nonlinear_eq_constraint_targets());
}


void SurrogateModel::update_all_variables_from_model(const Model& model)
{
  // update vars/bounds/labels with model data using All view for both
  // (since approx arrays are sized but otherwise uninitialized)
  currentVariables.all_continuous_variables(
    model.all_continuous_variables());
  userDefinedConstraints.all_continuous_lower_bounds(
    model.all_continuous_lower_bounds());
  userDefinedConstraints.all_continuous_upper_bounds(
    model.all_continuous_upper_bounds());

  currentVariables.all_discrete_int_variables(
    model.all_discrete_int_variables());
  userDefinedConstraints.all_discrete_int_lower_bounds(
    model.all_discrete_int_lower_bounds());
  userDefinedConstraints.all_discrete_int_upper_bounds(
    model.all_discrete_int_upper_bounds());

  currentVariables.all_discrete_string_variables(
    model.all_discrete_string_variables());

  currentVariables.all_discrete_real_variables(
    model.all_discrete_real_variables());
  userDefinedConstraints.all_discrete_real_lower_bounds(
    model.all_discrete_real_lower_bounds());
  userDefinedConstraints.all_discrete_real_upper_bounds(
    model.all_discrete_real_upper_bounds());

  if (!approxBuilds) {
    currentVariables.all_continuous_variable_labels(
      model.all_continuous_variable_labels());
    currentVariables.all_discrete_int_variable_labels(
      model.all_discrete_int_variable_labels());
    currentVariables.all_discrete_string_variable_labels(
      model.all_discrete_string_variable_labels());
    currentVariables.all_discrete_real_variable_labels(
      model.all_discrete_real_variable_labels());
  }
}


/** Update values and labels in currentVariables and
    bound/linear/nonlinear constraints in userDefinedConstraints from
    variables and constraints data within model. */
void SurrogateModel::update_complement_variables_from_model(const Model& model)
{
  // updates the complement of the active variables from model

  // This approach is rendered robust to differing parameterizations through
  // use of variable tag lookups.  Omits mappings for failed lookups.

  // Note: label assignments do not make sense in this case since we are
  //       relying on them for lookups

  const Variables&   vars = model.current_variables();
  const Constraints& cons = model.user_defined_constraints();

  const RealVector& acv = vars.all_continuous_variables();
  StringMultiArrayConstView acv_labels = vars.all_continuous_variable_labels();
  const RealVector& acv_l_bnds = cons.all_continuous_lower_bounds();
  const RealVector& acv_u_bnds = cons.all_continuous_upper_bounds();
  StringMultiArrayConstView cv_acv_labels
    = currentVariables.all_continuous_variable_labels();
  size_t i, index, cv_begin = vars.cv_start(), num_cv = vars.cv(),
    cv_end = cv_begin + num_cv, num_acv = vars.acv();
  for (i=0; i<cv_begin; ++i) {
    index = find_index(cv_acv_labels, acv_labels[i]);
    if (index != _NPOS) {
      currentVariables.all_continuous_variable(acv[i], index);
      userDefinedConstraints.all_continuous_lower_bound(acv_l_bnds[i], index);
      userDefinedConstraints.all_continuous_upper_bound(acv_u_bnds[i], index);
    }
  }
  for (i=cv_end; i<num_acv; ++i) {
    index = find_index(cv_acv_labels, acv_labels[i]);
    if (index != _NPOS) {
      currentVariables.all_continuous_variable(acv[i], index);
      userDefinedConstraints.all_continuous_lower_bound(acv_l_bnds[i], index);
      userDefinedConstraints.all_continuous_upper_bound(acv_u_bnds[i], index);
    }
  }

  const IntVector& adiv = vars.all_discrete_int_variables();
  StringMultiArrayConstView adiv_labels
    = vars.all_discrete_int_variable_labels();
  const IntVector& adiv_l_bnds = cons.all_discrete_int_lower_bounds();
  const IntVector& adiv_u_bnds = cons.all_discrete_int_upper_bounds();
  StringMultiArrayConstView cv_adiv_labels
    = currentVariables.all_discrete_int_variable_labels();
  size_t div_begin = vars.div_start(), num_div = vars.div(),
    div_end = div_begin + num_div, num_adiv = vars.adiv();
  for (i=0; i<div_begin; ++i) {
    index = find_index(cv_adiv_labels, adiv_labels[i]);
    if (index != _NPOS) {
      currentVariables.all_discrete_int_variable(adiv[i], index);
      userDefinedConstraints.all_discrete_int_lower_bound(adiv_l_bnds[i],index);
      userDefinedConstraints.all_discrete_int_upper_bound(adiv_u_bnds[i],index);
    }
  }
  for (i=div_end; i<num_adiv; ++i) {
    index = find_index(cv_adiv_labels, adiv_labels[i]);
    if (index != _NPOS) {
      currentVariables.all_discrete_int_variable(adiv[i], index);
      userDefinedConstraints.all_discrete_int_lower_bound(adiv_l_bnds[i],index);
      userDefinedConstraints.all_discrete_int_upper_bound(adiv_u_bnds[i],index);
    }
  }

  size_t dsv_begin = vars.dsv_start(), num_dsv = vars.dsv(),
    dsv_end = dsv_begin + num_dsv, num_adsv = vars.adsv();
  StringMultiArrayConstView adsv = vars.all_discrete_string_variables();
  StringMultiArrayConstView adsv_labels
    = vars.all_discrete_string_variable_labels();
  StringMultiArrayConstView cv_adsv_labels
    = currentVariables.all_discrete_string_variable_labels();
  for (i=0; i<dsv_begin; ++i) {
    index = find_index(cv_adsv_labels, adsv_labels[i]);
    if (index != _NPOS)
      currentVariables.all_discrete_string_variable(adsv[i], index);
  }
  for (i=dsv_end; i<num_adsv; ++i) {
    index = find_index(cv_adsv_labels, adsv_labels[i]);
    if (index != _NPOS)
      currentVariables.all_discrete_string_variable(adsv[i], index);
  }

  const RealVector& adrv = vars.all_discrete_real_variables();
  StringMultiArrayConstView adrv_labels
    = vars.all_discrete_real_variable_labels();
  const RealVector& adrv_l_bnds = cons.all_discrete_real_lower_bounds();
  const RealVector& adrv_u_bnds = cons.all_discrete_real_upper_bounds();
  StringMultiArrayConstView cv_adrv_labels
    = currentVariables.all_discrete_real_variable_labels();
  size_t drv_begin = vars.drv_start(), num_drv = vars.drv(),
    drv_end = drv_begin + num_drv, num_adrv = vars.adrv();
  for (i=0; i<drv_begin; ++i) {
    index = find_index(cv_adrv_labels, adrv_labels[i]);
    if (index != _NPOS) {
      currentVariables.all_discrete_real_variable(adrv[i], index);
      userDefinedConstraints.all_discrete_real_lower_bound(adrv_l_bnds[i],
							   index);
      userDefinedConstraints.all_discrete_real_upper_bound(adrv_u_bnds[i],
							   index);
    }
  }
  for (i=drv_end; i<num_adrv; ++i) {
    index = find_index(cv_adrv_labels, adrv_labels[i]);
    if (index != _NPOS) {
      currentVariables.all_discrete_real_variable(adrv[i], index);
      userDefinedConstraints.all_discrete_real_lower_bound(adrv_l_bnds[i],
							   index);
      userDefinedConstraints.all_discrete_real_upper_bound(adrv_u_bnds[i],
							   index);
    }
  }
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
asv_split(const ShortArray& orig_asv, ShortArray& actual_asv,
	  ShortArray& approx_asv, bool build_flag)
{
  size_t i, num_qoi = qoi();
  switch (responseMode) {
  case AGGREGATED_MODEL_PAIR: {
    // split actual & approx asv (can ignore build_flag)
    if (orig_asv.size() != 2*num_qoi) {
      Cerr << "Error: ASV not aggregated for AGGREGATED_MODEL_PAIR mode in "
	   << "SurrogateModel::asv_split()." << std::endl;
      abort_handler(MODEL_ERROR);
    }
    approx_asv.resize(num_qoi); actual_asv.resize(num_qoi);
    // aggregated response uses {HF,LF} order:
    for (i=0; i<num_qoi; ++i)
      actual_asv[i] = orig_asv[i];
    for (i=0; i<num_qoi; ++i)
      approx_asv[i] = orig_asv[i+num_qoi];
    break;
  }
  default: // non-aggregated modes have consistent ASV request vector lengths
    if (surrogateFnIndices.size() == num_qoi) {
      if (build_flag) actual_asv = orig_asv;
      else            approx_asv = orig_asv;
    }
    // else response set is mixed:
    else if (build_flag) { // construct mode: define actual_asv
      actual_asv.assign(num_qoi, 0);
      for (StSIter it=surrogateFnIndices.begin();
	   it!=surrogateFnIndices.end(); ++it)
	actual_asv[*it] = orig_asv[*it];
    }
    else { // eval mode: define actual_asv & approx_asv contributions
      for (i=0; i<num_qoi; ++i) {
	short orig_asv_val = orig_asv[i];
	if (orig_asv_val) {
	  if (surrogateFnIndices.count(i)) {
	    if (approx_asv.empty()) // keep empty if no active requests
	      approx_asv.assign(num_qoi, 0);
	    approx_asv[i] = orig_asv_val;
	  }
	  else {
	    if (actual_asv.empty()) // keep empty if no active requests
	      actual_asv.assign(num_qoi, 0);
	    actual_asv[i] = orig_asv_val;
	  }
	}
      }
    }
    break;
  }
}


void SurrogateModel::
asv_split(const ShortArray& aggregate_asv, Short2DArray& indiv_asv)
{
  // This API only used for AGGREGATED_MODELS mode

  size_t i, j, num_qoi = qoi();
  if (aggregate_asv.size() % num_qoi) {
    Cerr << "Error: size remainder for aggregated ASV in SurrogateModel::"
	 << "asv_split()." << std::endl;
    abort_handler(MODEL_ERROR);
  }
  size_t num_indiv = aggregate_asv.size() / num_qoi, cntr = 0;
  indiv_asv.resize(num_indiv);
  for (i=0; i<num_indiv; ++i) {
    ShortArray& asv_i = indiv_asv[i];
    asv_i.resize(num_qoi);
    for (j=0; j<num_qoi; ++j, ++cntr)
      asv_i[j] = aggregate_asv[cntr];
  }
}


void SurrogateModel::
asv_combine(const ShortArray& actual_asv, const ShortArray& approx_asv,
	    ShortArray& combined_asv)
{
  if (actual_asv.empty())
    combined_asv = approx_asv;
  else if (approx_asv.empty())
    combined_asv = actual_asv;
  else {
    combined_asv.resize(numFns);
    for (size_t i=0; i<numFns; ++i)
      combined_asv[i] = (surrogateFnIndices.count(i)) ?
	approx_asv[i] : actual_asv[i];
  }
}


void SurrogateModel::
response_combine(const Response& actual_response,
		 const Response& approx_response, Response& combined_response)
{
  const ShortArray& actual_asv = actual_response.active_set_request_vector();
  const ShortArray& approx_asv = approx_response.active_set_request_vector();
  ShortArray combined_asv;
  if (combined_response.is_null()) {
    combined_response = currentResponse.copy();
    asv_combine(actual_asv, approx_asv, combined_asv);
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

  // preserve the simulation-based metadata
  if (!actual_asv.empty())
    combined_response.metadata(actual_response.metadata(), 0);
}


void SurrogateModel::
aggregate_response(const Response& hf_resp, const Response& lf_resp,
		   Response& agg_resp)
{
  if (agg_resp.is_null())
    agg_resp = currentResponse.copy(); // resized to 2x in resize_response()

  const ShortArray& lf_asv =  lf_resp.active_set_request_vector();
  const ShortArray& hf_asv =  hf_resp.active_set_request_vector();
  ShortArray&      agg_asv = agg_resp.active_set_request_vector();
  size_t i, offset_i, num_lf_fns = lf_asv.size(), num_hf_fns = hf_asv.size();
  short asv_i;

  // Order with HF first since it corresponds to the active model key
  for (i=0; i<num_hf_fns; ++i) {
    agg_asv[i] = asv_i = hf_asv[i];
    if (asv_i & 1) agg_resp.function_value(hf_resp.function_value(i), i);
    if (asv_i & 2)
      agg_resp.function_gradient(hf_resp.function_gradient_view(i), i);
    if (asv_i & 4)
      agg_resp.function_hessian(hf_resp.function_hessian(i), i);
  }

  // Order with LF second since it corresponds to a previous/decremented key
  for (i=0; i<num_lf_fns; ++i) {
    offset_i = i + num_hf_fns;
    agg_asv[offset_i] = asv_i = lf_asv[i];
    if (asv_i & 1) agg_resp.function_value(lf_resp.function_value(i), offset_i);
    if (asv_i & 2)
      agg_resp.function_gradient(lf_resp.function_gradient_view(i), offset_i);
    if (asv_i & 4)
      agg_resp.function_hessian(lf_resp.function_hessian(i), offset_i);
  }

  const RealArray& hf_md = hf_resp.metadata();
  agg_resp.metadata(hf_md, 0);
  agg_resp.metadata(lf_resp.metadata(), hf_md.size());
  //if (outputLevel >= DEBUG_OUTPUT)
  //  Cout << "HF Metadata:\n" << hf_md << "LF Metadata:\n"<<lf_resp.metadata();
}


/* Note: insert_response() eliminates need for ResponseArray accumulation
void SurrogateModel::
aggregate_response(const ResponseArray& resp_array, Response& agg_response)
{
  if (agg_response.is_null())
    agg_response = currentResponse.copy();// resize_response() -> aggregate size

  size_t i, j, num_resp = resp_array.size(), num_fns, cntr = 0;  short asv_j;
  ShortArray& agg_asv = agg_response.active_set_request_vector();
  // append in order provided (any order customizations need to occur upstream
  // in the definition of resp_array)
  for (i=0; i<num_resp; ++i) {
    const Response& resp_i = resp_array[i];
    const ShortArray&  asv = resp_i.active_set_request_vector();
    num_fns = asv.size();
    for (j=0; j<num_fns; ++j, ++cntr) {
      agg_asv[cntr] = asv_j = asv[j];
      if (asv_j & 1)
	agg_response.function_value(resp_i.function_value(j), cntr);
      if (asv_j & 2)
	agg_response.function_gradient(resp_i.function_gradient_view(j), cntr);
      if (asv_j & 4)
	agg_response.function_hessian(resp_i.function_hessian(j), cntr);
    }
  }
}
*/


void SurrogateModel::
insert_response(const Response& response, size_t position,
		Response& agg_response)//, bool include_metadata)
{
  if (agg_response.is_null())
    agg_response = currentResponse.copy();// resize_response() -> aggregate size

  ShortArray& agg_asv = agg_response.active_set_request_vector();
  // append in order provided (any order customizations need to occur upstream
  // in the definition of resp_array)
  const ShortArray& asv = response.active_set_request_vector();
  size_t fn, num_fns = asv.size(), cntr = insert_response_start(position);
  short asv_fn;
  for (fn=0; fn<num_fns; ++fn, ++cntr) {
    agg_asv[cntr] = asv_fn = asv[fn];
    if (asv_fn & 1)
      agg_response.function_value(response.function_value(fn), cntr);
    if (asv_fn & 2)
      agg_response.function_gradient(response.function_gradient_view(fn), cntr);
    if (asv_fn & 4)
      agg_response.function_hessian(response.function_hessian(fn), cntr);
  }

  //if (include_metadata)
  insert_metadata(response.metadata(), position, agg_response);
}


size_t SurrogateModel::insert_response_start(size_t position)
{
  // default to be overridden, given knowledge of ensemble response sizes
  size_t num_fns
    = truth_model().current_response().active_set_request_vector().size();
  return position * num_fns;
}


void SurrogateModel::
insert_metadata(const RealArray& md, size_t position, Response& agg_response)
{
  // default to be overridden, given knowledge of ensemble metadata sizes
  agg_response.metadata(md, position * md.size());
}

} // namespace Dakota
