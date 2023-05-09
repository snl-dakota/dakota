/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
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
	       const ShortShortPair& surr_view,
	       const SharedVariablesData& svd, bool share_svd,
	       const SharedResponseData&  srd, bool share_srd,
	       const ActiveSet& surr_set, short corr_type, short output_level):
  // Allow DFSModel to employ sizing differences (e.g., consuming aggregations)
  Model(LightWtBaseConstructor(), surr_view, svd, share_svd, srd, share_srd,
	surr_set, output_level, problem_db, parallel_lib),
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
void SurrogateModel::init_model_constraints(Model& sub_model)
{
  if (sub_model.is_null()) // possible for DataFitSurrModel
    return;

  size_t num_cv  = currentVariables.cv(),  num_div = currentVariables.div(),
         num_drv = currentVariables.drv(), num_dsv = currentVariables.dsv(),
      num_sm_cv  = sub_model.cv(),         num_sm_div = sub_model.div(),
      num_sm_dsv = sub_model.dsv(),        num_sm_drv = sub_model.drv();

  // linear constraints (apply to active cv,div,drv)

  bool lin_ineq = (userDefinedConstraints.num_linear_ineq_constraints() > 0),
       lin_eq   = (userDefinedConstraints.num_linear_eq_constraints()   > 0);
  if ( (lin_ineq || lin_eq) && (sub_model.cv()  != currentVariables.cv()  ||
				sub_model.div() != currentVariables.div() ||
				sub_model.drv() != currentVariables.drv()) ) {
    // the views don't necessarily have to be the same, but number of active
    // continuous and active discrete int,real variables have to be consistent
    Cerr << "Error: cannot update linear constraints in SurrogateModel::"
	 << "init_model() due to inconsistent active variables." << std::endl;
    abort_handler(MODEL_ERROR);
  }
  if (lin_ineq) {
    sub_model.linear_ineq_constraint_coeffs(
      userDefinedConstraints.linear_ineq_constraint_coeffs());
    sub_model.linear_ineq_constraint_lower_bounds(
      userDefinedConstraints.linear_ineq_constraint_lower_bounds());
    sub_model.linear_ineq_constraint_upper_bounds(
      userDefinedConstraints.linear_ineq_constraint_upper_bounds());
  }
  if (lin_eq) {
    sub_model.linear_eq_constraint_coeffs(
      userDefinedConstraints.linear_eq_constraint_coeffs());
    sub_model.linear_eq_constraint_targets(
      userDefinedConstraints.linear_eq_constraint_targets());
  }

  // nonlinear constraints

  if (userDefinedConstraints.num_nonlinear_ineq_constraints()) {
    sub_model.nonlinear_ineq_constraint_lower_bounds(
      userDefinedConstraints.nonlinear_ineq_constraint_lower_bounds());
    sub_model.nonlinear_ineq_constraint_upper_bounds(
      userDefinedConstraints.nonlinear_ineq_constraint_upper_bounds());
  }
  if (userDefinedConstraints.num_nonlinear_eq_constraints())
    sub_model.nonlinear_eq_constraint_targets(
      userDefinedConstraints.nonlinear_eq_constraint_targets());
}


void SurrogateModel::init_model_labels(Model& sub_model)
{
  if (approxBuilds) return;

  // labels: update model with current{Variables,Response} descriptors
  // inactive vars / bounds: propagate inactive vars when necessary

  if (sub_model.response_labels().empty()) // should not happen
    switch (responseMode) {
    case AGGREGATED_MODELS: case AGGREGATED_MODEL_PAIR: {
      StringArray qoi_labels;
      copy_data_partial(currentResponse.function_labels(),
			0, sub_model.qoi(), qoi_labels);
      sub_model.response_labels(qoi_labels);
      break;
    }
    default:
      sub_model.response_labels(currentResponse.function_labels()); break;
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
  Variables& sm_vars = sub_model.current_variables();
  short active_view = currentVariables.view().first,
     sm_active_view = sm_vars.view().first;
  if (active_view == sm_active_view) {
    // update active model vars with active currentVariables data
    sm_vars.active_labels(currentVariables);
    // Now supported by init_model_inactive_labels()
    //if (!active_all) // models not in ALL view
    //  sm_vars.inactive_labels(currentVariables);
  }
  else {
    bool active_all = (active_view == RELAXED_ALL || active_view == MIXED_ALL),
      sm_active_all = (sm_active_view == RELAXED_ALL ||
		       sm_active_view == MIXED_ALL);
    if (!active_all && sm_active_all) // update active from "All" view
      sm_vars.all_to_active_labels(currentVariables);
    else if (!sm_active_all && active_all) // update "All" view from active
      // TO DO: only update the active labels in model (not all labels)
      sm_vars.active_to_all_labels(currentVariables);
  }
}


void SurrogateModel::init_model_inactive_variables(Model& sub_model)
{
  Variables&  sm_vars = sub_model.current_variables();
  short inactive_view = currentVariables.view().second;
  // matching non-empty:
  if (inactive_view && inactive_view == sm_vars.view().second) {
    // update model with inactive currentVariables/userDefinedConstraints
    // data. For efficiency, we avoid doing this on every evaluation, instead
    // calling it from a pre-execution initialization context.
    sm_vars.inactive_variables(currentVariables);
    sub_model.user_defined_constraints().
      inactive_bounds(userDefinedConstraints);
  }
}


void SurrogateModel::init_model_inactive_labels(Model& sub_model)
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

  Variables& sm_vars = sub_model.current_variables();
  const ShortShortPair&   sm_view =          sm_vars.view();
  const ShortShortPair& curr_view = currentVariables.view();
  short active_view = curr_view.first,    inactive_view = curr_view.second,
     sm_active_view =   sm_view.first, sm_inactive_view =   sm_view.second;
  bool active_all = (active_view == RELAXED_ALL || active_view == MIXED_ALL),
    sm_active_all = (sm_active_view == RELAXED_ALL ||
		     sm_active_view == MIXED_ALL);
  if (inactive_view && inactive_view == sm_inactive_view) // matching non-empty
    // Can't use inactive label matching since that is what we are updating,
    // so rely only on counts for now.
    sm_vars.inactive_labels(currentVariables);
  else if (!active_all && sm_active_all) {
    // nothing to do currenty for this case prior to more fine-grained
    // handling of active labels (all model labels are currently updated)
  }
  else if (!sm_active_all && active_all) {
    // nothing to do currenty for this case prior to more fine-grained
    // handling of active labels (all model labels are currently updated)
  }
}


void SurrogateModel::update_model(Model& sub_model)
{
  if (sub_model.is_null()) return; // possible for DataFitSurrModel
  update_model_active_variables(sub_model); // default operation
  update_model_active_constraints(sub_model); // default operation
  //update_model_distributions(sub_model);
}


void SurrogateModel::update_model_active_constraints(Model& sub_model)
{
  Constraints& sm_cons = sub_model.user_defined_constraints();
  short active_view = userDefinedConstraints.shared_data().view().first,
     sm_active_view = sm_cons.shared_data().view().first;
  if (active_view == sm_active_view)
    sm_cons.active_bounds(userDefinedConstraints);
  else {
    bool active_all = (active_view == RELAXED_ALL || active_view == MIXED_ALL),
      sm_active_all = (sm_active_view == RELAXED_ALL ||
		       sm_active_view == MIXED_ALL);
    if (!active_all && sm_active_all)
      sm_cons.all_to_active_bounds(userDefinedConstraints);
    else if (!sm_active_all && active_all)
      sm_cons.active_to_all_bounds(userDefinedConstraints);
    // TO DO: extend for aleatory/epistemic uncertain views
    else {
      Cerr << "Error: unsupported variable view differences in SurrogateModel::"
	   << "update_model_active_constraints()." << std::endl;
      abort_handler(MODEL_ERROR);
    }
  }
}


void SurrogateModel::update_model_distributions(Model& sub_model)
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
  // > currentVariables may have different active view from incoming sub_model
  //   vars, but MultivariateDistribution updates can be performed for all
  //   vars (independent of view)
  // > when sub_model is a ProbabilityTransformModel, its mvDist is in u-space.
  //   DataFit operates in and pushes updates to this transformed space for
  //   parameterized std distribs (e.g. {JACOBI,GEN_LAGUERE,NUM_GEN}_ORTHOG).
  // > it is sufficient to pull parameters at initialize_mapping() time, as
  //   this data varies per iterator execution rather than per-evaluation
  const SharedVariablesData&    svd = currentVariables.shared_data();
  const SharedVariablesData& sm_svd
    = sub_model.current_variables().shared_data();
  if (svd.id() == sm_svd.id()) // same set of variables
    sub_model.multivariate_distribution().pull_distribution_parameters(mvDist);
  else { // map between related sets of variables based on labels
    StringArray pull_labels;     svd.assemble_all_labels(pull_labels);
    StringArray push_labels;  sm_svd.assemble_all_labels(push_labels);
    sub_model.multivariate_distribution().
      pull_distribution_parameters(mvDist, pull_labels, push_labels);
  }
}


/** Update values and labels in currentVariables and
    bound/linear/nonlinear constraints in userDefinedConstraints from
    variables and constraints data within sub_model. */
void SurrogateModel::update_from_model(const Model& sub_model)
{
  if (sub_model.is_null()) return; // possible for DataFitSurrModel
  update_variables_from_model(sub_model);
  //update_distributions_from_model(sub_model);
  update_response_from_model(sub_model);
}


void SurrogateModel::update_variables_from_model(const Model& sub_model)
{
  // vars/bounds/labels

  if (currentVariables.variables_id() ==
      sub_model.current_variables().variables_id())
    update_all_variables_from_model(sub_model);
  else // fine-grained update based on label lookups
    update_complement_variables_from_model(sub_model);
}


void SurrogateModel::update_distributions_from_model(const Model& sub_model)
{
  // uncertain variable distribution data (dependent on label updates above)

  // See notes in init_model() above, with the difference that these
  // updates are performed once at lightweight construct time.
  const SharedVariablesData&    svd = currentVariables.shared_data();
  const SharedVariablesData& sm_svd
    = sub_model.current_variables().shared_data();
  if (svd.id() == sm_svd.id()) // same variables specification
    mvDist.pull_distribution_parameters(sub_model.multivariate_distribution());
  else { // map between related sets of variables based on labels
    StringArray pull_labels;  sm_svd.assemble_all_labels(pull_labels);
    StringArray push_labels;     svd.assemble_all_labels(push_labels);
    mvDist.pull_distribution_parameters(sub_model.multivariate_distribution(),
					pull_labels, push_labels);
  }
}


void SurrogateModel::update_response_from_model(const Model& sub_model)
{
  if (!approxBuilds &&
      currentResponse.function_labels().empty()) // should not happen
    switch (responseMode) {
    case AGGREGATED_MODELS: case AGGREGATED_MODEL_PAIR: {
      const StringArray& model_labels = sub_model.response_labels();
      size_t i, start = 0, num_fns = currentResponse.num_functions(),
	qoi = sub_model.qoi(), num_repl = num_fns / qoi;
      StringArray repl_labels(num_fns);
      for (i=0; i<num_repl; ++i, start+=qoi)
	copy_data_partial(model_labels, repl_labels, start);
      currentResponse.function_labels(repl_labels);
      break;
    }
    default:
      currentResponse.function_labels(sub_model.response_labels()); break;
    }

  // weights and sense for primary response functions

  primaryRespFnWts   = sub_model.primary_response_fn_weights();
  primaryRespFnSense = sub_model.primary_response_fn_sense();

  // linear constraints

  bool lin_ineq = (sub_model.num_linear_ineq_constraints() > 0),
       lin_eq   = (sub_model.num_linear_eq_constraints()   > 0);
  if ( (lin_ineq || lin_eq) && (sub_model.cv()  != currentVariables.cv()  ||
				sub_model.div() != currentVariables.div() ||
				sub_model.drv() != currentVariables.drv()) ) {
    // the views don't necessarily have to be the same, but the number of
    // active continuous and active discrete variables have to be consistent
    Cerr << "Error: cannot update linear constraints in SurrogateModel::update"
	 << "_from_model() due to inconsistent active variables." << std::endl;
    abort_handler(MODEL_ERROR);
  }
  if (lin_ineq) {
    userDefinedConstraints.linear_ineq_constraint_coeffs(
      sub_model.linear_ineq_constraint_coeffs());
    userDefinedConstraints.linear_ineq_constraint_lower_bounds(
      sub_model.linear_ineq_constraint_lower_bounds());
    userDefinedConstraints.linear_ineq_constraint_upper_bounds(
      sub_model.linear_ineq_constraint_upper_bounds());
  }
  if (lin_eq) {
    userDefinedConstraints.linear_eq_constraint_coeffs(
      sub_model.linear_eq_constraint_coeffs());
    userDefinedConstraints.linear_eq_constraint_targets(
      sub_model.linear_eq_constraint_targets());
  }

  // nonlinear constraints

  if (sub_model.num_nonlinear_ineq_constraints()) {
    userDefinedConstraints.nonlinear_ineq_constraint_lower_bounds(
      sub_model.nonlinear_ineq_constraint_lower_bounds());
    userDefinedConstraints.nonlinear_ineq_constraint_upper_bounds(
      sub_model.nonlinear_ineq_constraint_upper_bounds());
  }
  if (sub_model.num_nonlinear_eq_constraints())
    userDefinedConstraints.nonlinear_eq_constraint_targets(
      sub_model.nonlinear_eq_constraint_targets());
}


void SurrogateModel::update_all_variables_from_model(const Model& sub_model)
{
  // update vars/bounds/labels with model data using All view for both
  // (since approx arrays are sized but otherwise uninitialized)
  currentVariables.all_variables(sub_model.current_variables());
  userDefinedConstraints.all_bounds(sub_model.user_defined_constraints());

  if (!approxBuilds)
    currentVariables.all_labels(sub_model.current_variables());
}


/** Update values and labels in currentVariables and
    bound/linear/nonlinear constraints in userDefinedConstraints from
    variables and constraints data within model. */
void SurrogateModel::
update_complement_variables_from_model(const Model& sub_model)
{
  // updates the complement of the active variables from sub_model

  // This approach is rendered robust to differing parameterizations through
  // use of variable tag lookups.  Omits mappings for failed lookups.

  // Note: label assignments do not make sense in this case since we are
  //       relying on them for lookups

  const Variables&   vars = sub_model.current_variables();
  const Constraints& cons = sub_model.user_defined_constraints();

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
bool SurrogateModel::
check_rebuild(const RealVector& ref_icv,        const IntVector&  ref_idiv,
	      const StringMultiArray& ref_idsv, const RealVector& ref_idrv,
	      const RealVector& ref_c_l_bnds,   const RealVector& ref_c_u_bnds,
	      const IntVector&  ref_di_l_bnds,  const IntVector&  ref_di_u_bnds,
	      const RealVector& ref_dr_l_bnds,  const RealVector& ref_dr_u_bnds)
{
  // force rebuild for change in inactive vars based on sub-model view.  It
  // is assumed that any recastings within Model recursions do not affect the
  // inactive variables (while RecastModel::variablesMapping has access to
  // all of the vars, the convention is to modify only the active vars).

  // for global surrogates, force rebuild for change in active bounds

  Model& actual_model = active_truth_model();

  // Don't force rebuild for active subspace model:
  // JAM TODO: there's probably a more elegant way to accomodate subspace models
  if (actual_model.model_type() == "active_subspace")
    return false;

  short approx_active_view = currentVariables.view().first;
  if (actual_model.is_null()) {
    // compare reference vars against current inactive top-level data
    if ( ref_icv  != currentVariables.inactive_continuous_variables() ||
	 ref_idiv !=
	 currentVariables.inactive_discrete_int_variables()                   ||
	 ref_idsv !=
	 currentVariables.inactive_discrete_string_variables()                ||
	 ref_idrv !=
	 currentVariables.inactive_discrete_real_variables() )
      return true;

    if ( strbegins(surrogateType, "global_") &&
	 // compare reference bounds against current active top-level data
	 ( ref_c_l_bnds != userDefinedConstraints.continuous_lower_bounds()||
	   ref_c_u_bnds != userDefinedConstraints.continuous_upper_bounds()||
	   ref_di_l_bnds !=
	   userDefinedConstraints.discrete_int_lower_bounds()                 ||
	   ref_di_u_bnds !=
	   userDefinedConstraints.discrete_int_upper_bounds()                 ||
	   // no discrete string bounds
	   ref_dr_l_bnds !=
	   userDefinedConstraints.discrete_real_lower_bounds()                ||
	   ref_dr_u_bnds !=
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
	 ( ref_icv  != currentVariables.inactive_continuous_variables()      ||
	   ref_idiv != currentVariables.inactive_discrete_int_variables()    ||
	   ref_idsv != currentVariables.inactive_discrete_string_variables() ||
	   ref_idrv != currentVariables.inactive_discrete_real_variables() ) )
      return true;
    else if ( ( approx_active_view == RELAXED_ALL ||
		approx_active_view == MIXED_ALL ) &&
	      sub_model_active_view >= RELAXED_DESIGN ) {
      const SharedVariablesData& sm_svd = actual_vars.shared_data();
      if (!is_equal_partial(ref_icv, currentVariables.continuous_variables(),
			    sm_svd.icv_start())  ||
	  !is_equal_partial(ref_idiv, currentVariables.discrete_int_variables(),
			    sm_svd.idiv_start()) ||
	  !is_equal_partial(ref_idsv,
			    currentVariables.discrete_string_variables(),
			    sm_svd.idsv_start()) ||
	  !is_equal_partial(ref_idrv,currentVariables.discrete_real_variables(),
			    sm_svd.idrv_start()))
	return true;
    }
    // TO DO: extend for aleatory/epistemic uncertain views
    /*
    Model sub_model = actual_model.subordinate_model();
    while (sub_model.model_type() == "recast")
      sub_model = sub_model.subordinate_model();
    if ( ref_icv  != sub_model.inactive_continuous_variables()      ||
         ref_idiv != sub_model.inactive_discrete_int_variables()    ||
         ref_idsv != sub_model.inactive_discrete_string_variables() ||
         ref_idrv != sub_model.inactive_discrete_real_variables() )
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

	if (ref_c_l_bnds  != sub_model.continuous_lower_bounds()    ||
	    ref_c_u_bnds  != sub_model.continuous_upper_bounds()    ||
	    ref_di_l_bnds != sub_model.discrete_int_lower_bounds()  ||
	    ref_di_u_bnds != sub_model.discrete_int_upper_bounds()  ||
	    // no discrete string bounds
	    ref_dr_l_bnds != sub_model.discrete_real_lower_bounds() ||
	    ref_dr_u_bnds != sub_model.discrete_real_upper_bounds())
	  return true;
      }
      else if ( approx_active_view == sub_model_active_view && 
		// compare active top-level data against active sub-model data
		( ref_c_l_bnds !=
		  userDefinedConstraints.continuous_lower_bounds()    ||
		  ref_c_u_bnds !=
		  userDefinedConstraints.continuous_upper_bounds()    ||
		  ref_di_l_bnds !=
		  userDefinedConstraints.discrete_int_lower_bounds()  ||
		  ref_di_u_bnds !=
		  userDefinedConstraints.discrete_int_upper_bounds()  ||
		  // no discrete string bounds
		  ref_dr_l_bnds !=
		  userDefinedConstraints.discrete_real_lower_bounds() ||
		  ref_dr_u_bnds !=
		  userDefinedConstraints.discrete_real_upper_bounds() ) )
	return true;
      else if ( approx_active_view >= RELAXED_DESIGN &&
		( sub_model_active_view == RELAXED_ALL ||
		  sub_model_active_view == MIXED_ALL ) && 
		// compare top-level data in All view w/ active sub-model data
		( ref_c_l_bnds !=
		  userDefinedConstraints.all_continuous_lower_bounds()     ||
		  ref_c_u_bnds != 
		  userDefinedConstraints.all_continuous_upper_bounds()     ||
		  ref_di_l_bnds !=
		  userDefinedConstraints.all_discrete_int_lower_bounds()   ||
		  ref_di_u_bnds !=
		  userDefinedConstraints.all_discrete_int_upper_bounds()   ||
		  // no discrete string bounds
		  ref_dr_l_bnds !=
		  userDefinedConstraints.all_discrete_real_lower_bounds()  ||
		  ref_dr_u_bnds !=
		  userDefinedConstraints.all_discrete_real_upper_bounds() ) )
	return true;
      else if ( ( approx_active_view  == RELAXED_ALL ||
		  approx_active_view  == MIXED_ALL ) &&
		sub_model_active_view >= RELAXED_DESIGN ) {
	const SharedVariablesData& sm_svd = actual_vars.shared_data();
	size_t cv_s = sm_svd.cv_start(), div_s = sm_svd.div_start(),
	      drv_s = sm_svd.drv_start();
	if (!is_equal_partial(ref_c_l_bnds,
	     userDefinedConstraints.continuous_lower_bounds(),     cv_s) ||
	    !is_equal_partial(ref_c_u_bnds,
	     userDefinedConstraints.continuous_upper_bounds(),     cv_s) ||
	    !is_equal_partial(ref_di_l_bnds,
	     userDefinedConstraints.discrete_int_lower_bounds(),  div_s) ||
	    !is_equal_partial(ref_di_u_bnds,
	     userDefinedConstraints.discrete_int_upper_bounds(),  div_s) ||
	     // no discrete string bounds
	    !is_equal_partial(ref_dr_l_bnds,
	     userDefinedConstraints.discrete_real_lower_bounds(), drv_s) ||
	    !is_equal_partial(ref_dr_u_bnds,
	     userDefinedConstraints.discrete_real_upper_bounds(), drv_s) )
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
aggregate_response(const Response& resp1, const Response& resp2,
		   Response& agg_resp)
{
  if (agg_resp.is_null())
    agg_resp = currentResponse.copy(); // resized to 2x in resize_response()

  const ShortArray& asv1 =  resp1.active_set_request_vector();
  const ShortArray& asv2 =  resp2.active_set_request_vector();
  ShortArray&      agg_asv = agg_resp.active_set_request_vector();
  size_t i, offset_i, num_fns2 = asv2.size(), num_fns1 = asv1.size();
  short asv_i;

  // Order with HF first since it corresponds to the active model key
  for (i=0; i<num_fns1; ++i) {
    agg_asv[i] = asv_i = asv1[i];
    if (asv_i & 1) agg_resp.function_value(resp1.function_value(i), i);
    if (asv_i & 2)
      agg_resp.function_gradient(resp1.function_gradient_view(i), i);
    if (asv_i & 4)
      agg_resp.function_hessian(resp1.function_hessian(i), i);
  }

  // Order with LF second since it corresponds to a previous/decremented key
  for (i=0; i<num_fns2; ++i) {
    offset_i = i + num_fns1;
    agg_asv[offset_i] = asv_i = asv2[i];
    if (asv_i & 1) agg_resp.function_value(resp2.function_value(i), offset_i);
    if (asv_i & 2)
      agg_resp.function_gradient(resp2.function_gradient_view(i), offset_i);
    if (asv_i & 4)
      agg_resp.function_hessian(resp2.function_hessian(i), offset_i);
  }

  const RealArray& md1 = resp1.metadata();
  agg_resp.metadata(md1, 0);
  agg_resp.metadata(resp2.metadata(), md1.size());
  //if (outputLevel >= DEBUG_OUTPUT)
  //  Cout << "Metadata 1:\n" << md1 << "Metadata 2:\n" << resp2.metadata();
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
