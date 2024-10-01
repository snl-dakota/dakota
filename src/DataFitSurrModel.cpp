/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DataFitSurrModel.hpp"
#include "ProbabilityTransformModel.hpp"
#include "ApproximationInterface.hpp"
#include "ParamResponsePair.hpp"
#include "ProblemDescDB.hpp"
#include "PRPMultiIndex.hpp"
#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/rolling_mean.hpp>
#include "EvaluationStore.hpp"

static const char rcsId[]="@(#) $Id: DataFitSurrModel.cpp 7034 2010-10-22 20:16:32Z mseldre $";


namespace Dakota {

extern PRPCache data_pairs;


DataFitSurrModel::DataFitSurrModel(ProblemDescDB& problem_db):
  SurrogateModel(problem_db),
  pointsTotal(problem_db.get_int("model.surrogate.points_total")),
  pointsManagement(problem_db.get_short("model.surrogate.points_management")),
  pointReuse(problem_db.get_string("model.surrogate.point_reuse")),
  exportSurrogate(problem_db.get_bool("model.surrogate.export_surrogate")),
  importPointsFile(
    problem_db.get_string("model.surrogate.import_build_points_file")),
  exportPointsFile(
    problem_db.get_string("model.surrogate.export_approx_points_file")),
  exportFormat(problem_db.get_ushort("model.surrogate.export_approx_format")),
  exportVarianceFile(
    problem_db.get_string("model.surrogate.export_approx_variance_file")),
  exportVarianceFormat(
    problem_db.get_ushort("model.surrogate.export_approx_variance_format")),
  autoRefine(problem_db.get_bool("model.surrogate.auto_refine")),
  maxIterations(problem_db.get_sizet("model.max_iterations")),
  maxFuncEvals(problem_db.get_sizet("model.max_function_evals")),
  convergenceTolerance(problem_db.get_real("model.convergence_tolerance")),
  softConvergenceLimit(problem_db.get_int("model.soft_convergence_limit")),
  refineCVMetric(problem_db.get_string("model.surrogate.refine_cv_metric")),
  refineCVFolds(problem_db.get_int("model.surrogate.refine_cv_folds"))
{
  // ignore bounds when finite differencing on data fits, since the bounds are
  // artificial in this case (and reflecting the stencil degrades accuracy)
  ignoreBounds = true;

  // assign default responseMode based on correction specification;
  // NO_CORRECTION (0) is default
  responseMode = (corrType) ? AUTO_CORRECTED_SURROGATE : UNCORRECTED_SURROGATE;

  // if no user points management spec, assign RECOMMENDED_POINTS as default
  if (pointsManagement == DEFAULT_POINTS)
    pointsManagement = (pointsTotal > 0) ? TOTAL_POINTS : RECOMMENDED_POINTS;

  bool import_pts = !importPointsFile.empty(),
    export_pts = !exportPointsFile.empty() || !exportVarianceFile.empty();
  if (pointReuse.empty()) // assign default
    pointReuse = (import_pts) ? "all" : "none";

  // DataFitSurrModel is allowed to set the db list nodes, so long as it 
  // restores the list nodes to their previous setting
  const String& dace_method_pointer
    = problem_db.get_string("model.dace_method_pointer");
  const String& actual_model_pointer
    = problem_db.get_string("model.surrogate.truth_model_pointer");
  bool dace_construct = !dace_method_pointer.empty(),
      model_construct = (dace_construct || !actual_model_pointer.empty());
  size_t method_index = _NPOS, model_index = _NPOS;
  if (dace_construct) {
    method_index = problem_db.get_db_method_node(); // for restoration
    model_index  = problem_db.get_db_model_node();  // for restoration
    problem_db.set_db_list_nodes(dace_method_pointer);
  }
  else if (model_construct) {
    model_index = problem_db.get_db_model_node(); // for restoration
    problem_db.set_db_model_nodes(actual_model_pointer);
  }

  // Instantiate actual model from DB
  bool basis_expansion = false;  short u_space_type;
  if (model_construct) {
    // If approx type uses standardized random variables (hard-wired for now),
    // wrap with a ProbabilityTransformModel (retaining distribution bounds as
    // in PCE, SC, C3 ctors)
    if (strends(surrogateType, "_orthogonal_polynomial") ||
	strends(surrogateType, "_interpolation_polynomial")) {
      basis_expansion = true;
      u_space_type = problem_db.get_short("model.surrogate.expansion_type");
    }
    else if (strends(surrogateType, "_function_train" )) {
      basis_expansion = true;
      // Hardwire for C3 case prior to availability of XML spec:
      u_space_type = PARTIAL_ASKEY_U;//problem_db.get_short("model.surrogate.expansion_type");
    }
    else {
      actualModel = problem_db.get_model();
      // leave mvDist as initialized in Model ctor (from variables spec)
    }

    if (basis_expansion) {
      const Model& db_model = problem_db.get_model();
      actualModel.assign_rep(std::make_shared<ProbabilityTransformModel>(
	db_model, u_space_type));
      // overwrite mvDist from Model ctor by copying transformed u-space dist
      // (keep them distinct to allow for different active views).
      // construct time augmented with run time pull_distribution_parameters().
      mvDist = actualModel.multivariate_distribution().copy();
    }

    // ensure consistency of inputs/outputs between actual and approx
    check_submodel_compatibility(actualModel);
  }

  // Instantiate dace iterator from DB
  if (dace_construct) {
    daceIterator = problem_db.get_iterator(actualModel); // no meta-iterators
    daceIterator.sub_iterator_flag(true);
    // if outer level output is verbose/debug and actualModel verbosity is
    // defined by the DACE method spec, request fine-grained evaluation
    // reporting for purposes of the final output summary.  This allows verbose
    // final summaries without verbose output on every dace-iterator completion.
    if (outputLevel > NORMAL_OUTPUT)
      actualModel.fine_grained_evaluation_counters();
  }

  // reset all method/model pointers
  if (dace_construct) {
    problem_db.set_db_method_node(method_index); // restore method only
    problem_db.set_db_model_nodes(model_index);  // restore all model nodes
  }
  else if (model_construct)
    problem_db.set_db_model_nodes(model_index);  // restore all model nodes
  // else global approx. built solely from reuse_points: daceIterator/
  // actualModel remain empty envelopes.  Verify that there is a data source:
  // this basic check is augmented with a build_global() check which enforces
  // that the total points from both sources be >= minimum required.
  else if ( pointReuse == "none" ) {
    Cerr << "Error: to build a data fit surrogate model, either a global "
	 << "approximation\n       must be specified with reuse_points or "
	 << "dace_method_pointer, or a\n       local/multipoint approximation "
	 << "must be specified with a truth_model_pointer." << std::endl;
    abort_handler(MODEL_ERROR);
  }

  // assign the ApproximationInterface instance which manages the
  // local/multipoint/global approximation.  The number of
  // approximation variables is defined by the active variable set in
  // the sub-model, and any conversions based on differing variable
  // views must be performed in ApproximationInterface::map().
  const Variables& vars = (actualModel.is_null()) ? currentVariables :
    actualModel.current_variables();
  bool cache = false; String am_interface_id;
  if (!actualModel.is_null()) {
    am_interface_id = actualModel.interface_id();
    // for ApproximationInterface to be able to look up actualModel eval records
    // within data_pairs, the actualModel must have an active evaluation cache
    // and derivative estimation (which causes consolidation of Interface evals
    // within Model evals, breaking Model eval lookups) must be off.
    // Note: use of ProbabilityTransform recursion prevents data_pairs lookup
    if ( actualModel.evaluation_cache(false) &&
	!actualModel.derivative_estimation())
      cache = true;
  }
  // size approxInterface based on currentResponse, which is constructed from
  // DB response spec, since actualModel could contain response aggregations
  approxInterface.assign_rep(std::make_shared<ApproximationInterface>(
    problem_db, vars, cache, am_interface_id,
    currentResponse.function_labels()));

  // initialize the basis, if needed
  if (basis_expansion)
    shared_approximation().construct_basis(mvDist);

  // initialize the DiscrepancyCorrection instance
  switch (responseMode) {
  case MODEL_DISCREPANCY: case AUTO_CORRECTED_SURROGATE:
    if (corrType)
      deltaCorr.initialize(*this, surrogateFnIndices, corrType, corrOrder);
    break;
  }

  if (import_pts)
    import_points(problem_db.get_ushort("model.surrogate.import_build_format"),
      problem_db.get_bool("model.surrogate.import_use_variable_labels"),
      problem_db.get_bool("model.surrogate.import_build_active_only"));
  if (export_pts)
    initialize_export();
  if (import_pts || export_pts)
    manage_data_recastings();

  // actual import of the model happens in ctor of specific Approximations
  // this prevents an initial build
  if (problem_db.get_bool("model.surrogate.import_surrogate")) {
    for (auto& approx : approxInterface.approximations())
      approx.map_variable_labels(vars);
    ++approxBuilds;
    if (strbegins(surrogateType, "global_")) update_global_reference();
    else                                     update_local_reference();
  }

  currentResponse.reshape_metadata(0);
}


DataFitSurrModel::
DataFitSurrModel(Iterator& dace_iterator, Model& actual_model,
		 //const SharedVariablesData& svd,const SharedResponseData& srd,
		 const ActiveSet& dfs_set, const ShortShortPair& dfs_view,
		 const String& approx_type, const UShortArray& approx_order,
		 short corr_type, short corr_order, short data_order,
		 short output_level, const String& point_reuse,
		 const String& import_build_points_file,
		 unsigned short import_build_format,
		 bool import_build_active_only,
		 const String& export_approx_points_file,
		 unsigned short export_approx_format):
  // SVD can be shared, but don't share SRD as QoI aggregations are consumed:
  SurrogateModel(actual_model.problem_description_db(),
		 actual_model.parallel_library(), dfs_view,
		 actual_model.current_variables().shared_data(), true,
		 actual_model.current_response().shared_data(), false,
		 dfs_set, corr_type, output_level),
  daceIterator(dace_iterator), actualModel(actual_model), pointsTotal(0),
  pointsManagement(DEFAULT_POINTS), pointReuse(point_reuse),
  exportSurrogate(false), exportPointsFile(export_approx_points_file),
  exportFormat(export_approx_format),
  importPointsFile(import_build_points_file), autoRefine(false),
  maxIterations(100), maxFuncEvals(1000), convergenceTolerance(1e-4),
  softConvergenceLimit(0), refineCVMetric("root_mean_square"), refineCVFolds(10)
{
  // dace_iterator may be an empty envelope (local, multipoint approx),
  // but actual_model must be defined.
  if (actualModel.is_null()) {
    Cerr << "Error: actualModel is empty envelope in alternate "
	 << "DataFitSurrModel constructor." << std::endl;
    abort_handler(MODEL_ERROR);
  }

  surrogateType = approx_type;

  // assign default responseMode based on correction specification;
  // NO_CORRECTION (0) is default
  responseMode = (corrType) ? AUTO_CORRECTED_SURROGATE : UNCORRECTED_SURROGATE;

  bool import_pts = !importPointsFile.empty(),
       export_pts = !exportPointsFile.empty() || !exportVarianceFile.empty();
  if (pointReuse.empty()) // assign default
    pointReuse = (import_pts) ? "all" : "none";

  // copy actualModel dist (keep distinct to allow for different active views).
  // ref values for distribution params at construct time are updated at run
  // time via pull_distribution_parameters().
  mvDist = actualModel.multivariate_distribution().copy();
  if (dfs_view != actualModel.current_variables().view())
    initialize_active_types(mvDist);

  // update constraint counts in userDefinedConstraints
  userDefinedConstraints.reshape(actualModel.num_nonlinear_ineq_constraints(),
				 actualModel.num_nonlinear_eq_constraints(),
				 currentVariables.shared_data());

  update_from_model(actualModel);
  check_submodel_compatibility(actualModel);

  // for ApproximationInterface to be able to look up actualModel eval records
  // within data_pairs, the actualModel must have an active evaluation cache
  // and derivative estimation (which causes consolidation of Interface evals
  // within Model evals, breaking Model eval lookups) must be off.
  bool cache = ( actualModel.evaluation_cache(false) &&
		!actualModel.derivative_estimation() );
  // assign the ApproximationInterface instance which manages the
  // local/multipoint/global approximation.  By instantiating with assign_rep(),
  // Interface::get_interface() does not need special logic for approximations.
  approxInterface.assign_rep(std::make_shared<ApproximationInterface>(
    /*dfs_view,*/ approx_type, approx_order, actualModel.current_variables(), // ***
    cache, actualModel.interface_id(), numFns, data_order, outputLevel));

  if (!daceIterator.is_null()) // global DACE approximations
    daceIterator.sub_iterator_flag(true);
  //else { // local/multipoint approximation
  //}

  // initialize the DiscrepancyCorrection instance
  deltaCorr.initialize(*this, surrogateFnIndices, corr_type, corr_order);

  // to define derivative settings, we use incoming ASV to define requests
  // and surrogate type to determine analytic derivative support.
  const ShortArray& asv = dfs_set.request_vector();
  size_t i, num_fns = asv.size();
  bool grad_flag = false, hess_flag = false;
  for (i=0; i<num_fns; ++i) {
    if (asv[i] & 2) grad_flag = true;
    if (asv[i] & 4) hess_flag = true;
  }
  if (grad_flag)
    gradientType = (approx_type == "global_polynomial" ||
      approx_type == "global_gaussian" || approx_type == "global_kriging" ||
      approx_type == "global_moving_least_squares" ||
      strends(approx_type, "_orthogonal_polynomial") ||
      strends(approx_type, "_interpolation_polynomial") ||
      strbegins(approx_type, "local_") ||
      strbegins(approx_type, "multipoint_")) ? "analytic" : "numerical";
  else 
    gradientType = "none";
  if (hess_flag)
    hessianType = ( strbegins(approx_type, "local_") ||
      approx_type == "global_polynomial" || approx_type == "global_kriging" ||
      strends(approx_type, "_orthogonal_polynomial"))
    //strends(approx_type, "_interpolation_polynomial")) // TO DO
      ? "analytic" : "numerical";
  else
    hessianType = "none";

  if (outputLevel > NORMAL_OUTPUT)
    Cout << "DFS gradientType = " << gradientType 
	 << " DFS hessianType = " << hessianType << std::endl;

  // Promote fdGradStepSize/fdHessByFnStepSize/fdHessByGradStepSize to
  // defaults if needed.
  if (gradientType == "numerical") { // mixed not supported for this Model
    methodSource = "dakota"; intervalType = "central";
    fdGradStepType = "relative";
    fdGradStepSize.resize(1); fdGradStepSize[0] = 0.001;
  }
  if (hessianType == "numerical") { // mixed not supported for this Model
    if (gradientType == "numerical") {
      fdHessStepType = "relative";
      fdHessByFnStepSize.resize(1); fdHessByFnStepSize[0] = 0.002;
    }
    else
      { fdHessByGradStepSize.resize(1); fdHessByGradStepSize[0] = 0.001; }
  }

  // ignore bounds when finite differencing on data fits, since the bounds are
  // artificial in this case (and reflecting the stencil degrades accuracy)
  ignoreBounds = true;

  // TODO: pass import_use_var_labels from lightweight DFSModel ctor
  bool import_use_var_labels = false;
  if (import_pts) import_points(import_build_format, import_use_var_labels,
				import_build_active_only);
  if (export_pts) initialize_export();
  if (import_pts || export_pts) manage_data_recastings();

  currentResponse.reshape_metadata(0);
}


void DataFitSurrModel::check_submodel_compatibility(const Model& sub_model)
{
  bool err1 = check_active_variables(sub_model), err2 = false, err3 = false;

  // cases not covered by the SurrogateModel check are disallowed for DFSModel
  short active_view = currentVariables.view().first,
     sm_active_view = sub_model.current_variables().view().first;
  if ( !( active_view == sm_active_view ||
	( ( sm_active_view == RELAXED_ALL || sm_active_view == MIXED_ALL ) &&
	  active_view >= RELAXED_DESIGN ) ||
	( ( active_view == RELAXED_ALL || active_view == MIXED_ALL ) &&
	  sm_active_view >= RELAXED_DESIGN ) ) ) {
    Cerr << "Error: unsupported variable view differences between approximate "
	 << "and actual models within DataFitSurrModel." << std::endl;
    err2 = true;
  }

  // Check for compatible array sizing between sub_model and currentResponse.
  // EnsembleSurrModel creates aggregations and DataFitSurrModel consumes them.
  // For now, allow either a factor of 2 or 1 from aggregation or not.  In the
  // future, aggregations may span a broader model hierarchy (e.g., factor =
  // orderedModels.size()).  In general, the fn count check needs to be
  // specialized in the derived classes.
  size_t sm_qoi = sub_model.qoi();
  if ( numFns != sm_qoi) {
    Cerr << "Error: incompatibility between approximate and actual model "
	 << "response function sets\n       within DataFitSurrModel: " << numFns
	 << " approximate and " << sm_qoi << " actual functions.\n       "
	 << "Check consistency of responses specifications." << std::endl;
    err3 = true;
  }

  if (err1 || err2 || err3)
    abort_handler(MODEL_ERROR);
}


bool DataFitSurrModel::initialize_mapping(ParLevLIter pl_iter)
{
  Model::initialize_mapping(pl_iter);

  if (!actualModel.is_null()) {
    actualModel.initialize_mapping(pl_iter);
    // push data that varies per iterator execution rather than per-evaluation
    // from currentVariables and userDefinedConstraints into actualModel
    init_model(actualModel);
  }

  return false; // no change to problem size
}


/** Inactive variables must be propagated when a EnsembleSurrModel
    is employed by a sub-iterator (e.g., OUU with MLMC or MLPCE).
    In current use cases, this can occur once per sub-iterator
    execution within Model::initialize_mapping(). */
bool DataFitSurrModel::finalize_mapping()
{
  if (!actualModel.is_null()) actualModel.finalize_mapping();
  Model::finalize_mapping();

  return false; // no change to problem size
}


/*
void DataFitSurrModel::init_model(Model& model)
{
  SurrogateModel::init_model(model);

  // See concern in SurrogateModel::init_model():
  //init_model_inactive_variables(model);
  // This may be preferred:
  //init_model_mapped_variables(model);

  // retained for now since deactivated at base level for EnsembleSurrModel
  //init_model_inactive_labels(model);
}
*/


void DataFitSurrModel::update_model(Model& model)
{
  if (model.is_null()) return;
  update_model_active_variables(model);
  update_model_active_constraints(model);
  update_model_distributions(model);
}


void DataFitSurrModel::update_from_model(const Model& model)
{
  if (model.is_null()) return;
  update_variables_from_model(model);
  update_distributions_from_model(model);
  update_response_from_model(model);
}


/** asynchronous flags need to be initialized for the sub-models.  In addition,
    max_eval_concurrency is the outer level iterator concurrency, not the
    DACE concurrency that actualModel will see, and recomputing the
    message_lengths on the sub-model is probably not a bad idea either.
    Therefore, recompute everything on actualModel using init_communicators. */
void DataFitSurrModel::
derived_init_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
			   bool recurse_flag)
{
  // initialize approxInterface (for serial operations).
  // Note: this is where max_eval_concurrency would be used.
  //approxInterface.init_serial();

  // initialize actualModel for parallel operations
  if (recurse_flag && !actualModel.is_null()) {

    // minimum_points() returns the minimum number of points needed to build
    // approxInterface (global and local approximations) without any numerical
    // derivatives multiplier.  Obtain the deriv multiplier from actualModel.
    // min_points does not account for reuse_points or anchor, since these
    // will vary, and min_points must remain constant among ctor/run/dtor.
    int min_conc = approxInterface.minimum_points(false)
                 * actualModel.derivative_concurrency();
    // as for constructors, we recursively set and restore DB list nodes
    // (initiated from the restored starting point following construction)
    size_t model_index = probDescDB.get_db_model_node(); // for restoration
    if (daceIterator.is_null()) {
      // store within empty envelope for later use in derived_{set,free}_comms
      daceIterator.maximum_evaluation_concurrency(min_conc);
      daceIterator.iterated_model(actualModel);
      // init comms for actualModel
      probDescDB.set_db_model_nodes(actualModel.model_id());
      actualModel.init_communicators(pl_iter, min_conc);
    }
    else {
      // daceIterator.maximum_evaluation_concurrency() includes user-specified
      // samples for building a global approx & any numerical deriv multiplier.
      // Analyzer::maxEvalConcurrency must remain constant for ctor/run/dtor.

      // The concurrency for global/local surrogate construction is defined by
      // the greater of the dace samples user-specification and the min_points
      // approximation requirement.
      if (min_conc > daceIterator.maximum_evaluation_concurrency())
	daceIterator.maximum_evaluation_concurrency(min_conc); // update

      // init comms for daceIterator
      size_t method_index = probDescDB.get_db_method_node(); // for restoration
      probDescDB.set_db_list_nodes(daceIterator.method_id());
      daceIterator.init_communicators(pl_iter);
      probDescDB.set_db_method_node(method_index); // restore method only
    }
    probDescDB.set_db_model_nodes(model_index); // restore all model nodes
  }
}


/** This function constructs a new approximation, discarding any
    previous data.  It constructs any required data for
    SurrogateData::{vars,resp}Data and does not define an anchor point
    for SurrogateData::anchor{Vars,Resp}, so is an unconstrained build. */
void DataFitSurrModel::build_approximation()
{
  Cout << "\n>>>>> Building " << surrogateType << " approximations.\n";

  // update actualModel w/ variable values/bounds/labels
  update_model(actualModel);

  // build a local, multipoint, or global data fit approximation.
  if (strbegins(surrogateType, "local_") ||
      strbegins(surrogateType, "multipoint_")) { // NOTE: branch used by TRMM
    update_local_reference();
    build_local_multipoint();
  }
  else { // global approximation.  NOTE: branch not used by TRMM.
    update_global_reference();
    clear_approx_interface();
    build_global();
  }

  Cout << "\n<<<<< " << surrogateType << " approximation builds completed.\n";
}


/** This function constructs a new approximation, discarding any
    previous data.  It uses the passed data to populate
    SurrogateData::anchor{Vars,Resp} and constructs any required data
    points for SurrogateData::{vars,resp}Data. */
bool DataFitSurrModel::
build_approximation(const Variables& vars, const IntResponsePair& response_pr)
{
  // Usage notes:
  // > not used by SBLM local/multipoint
  // > used by SBLM global *with* persistent center vars,response
  // > used by NonDLocal *without* persistent vars,response

  Cout << "\n>>>>> Building " << surrogateType << " approximations.\n";

  // update actualModel w/ variable values/bounds/labels
  update_model(actualModel);

  // build a local, multipoint, or global data fit approximation.
  if (strbegins(surrogateType, "local_") ||
      strbegins(surrogateType, "multipoint_")) {// NOTE: branch not used by TRMM
    update_local_reference();
    build_local_multipoint(vars, response_pr);
  }
  else { // global approximation.  NOTE: branch used by TRMM.
    update_global_reference();
    update_approx_interface(vars, response_pr);
    build_global();
  }

  Cout << "\n<<<<< " << surrogateType << " approximation builds completed.\n";

  // return a bool indicating whether the incoming data defines an embedded
  // correction (hard constraint) or just another data point.  It would be
  // preferable to flow this up from the surrogate, but keep it simple for now.
  return (strbegins(surrogateType, "local_") ||
	  strbegins(surrogateType, "multipoint_") ||
	  surrogateType == "global_polynomial");
}


/** This function updates an existing approximation, by appending new data.
    It does not define an anchor point, so is an unconstrained build. */
void DataFitSurrModel::rebuild_approximation()
{
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n>>>>> Rebuilding " << surrogateType << " approximations.\n";

  // update actualModel w/ variable values/bounds/labels
  update_model(actualModel);

  // rebuild a local, multipoint, or global data fit approximation
  if (strbegins(surrogateType, "local_") ||
      strbegins(surrogateType, "multipoint_")) {
    //update_local_reference();//updates from build_approximation() remain valid
    build_local_multipoint(); // no change for build vs. rebuild
  }
  else { // global approximation
    //update_global_reference();// updates from build_approximation remain valid
    rebuild_global();
  }

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n<<<<< "<< surrogateType <<" approximation rebuilds completed.\n";
}


/** This function populates/replaces SurrogateData::anchor{Vars,Resp}
    and rebuilds the approximation, if requested.  It does not clear
    other data (i.e., SurrogateData::{vars,resp}Data) and does not
    update the actualModel with revised bounds, labels, etc.  Thus, it
    updates data from a previous call to build_approximation(), and is
    not intended to be used in isolation. */
void DataFitSurrModel::update_approximation(bool rebuild_flag)
{
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n>>>>> Updating " << surrogateType << " approximations.\n";

  // replace the current points for each approximation
  //daceIterator.run(pl_iter);
  const IntResponseMap& all_resp = daceIterator.all_responses();
  if (daceIterator.compact_mode())
    approxInterface.update_approximation(daceIterator.all_samples(),  all_resp);
  else
    approxInterface.update_approximation(daceIterator.all_variables(),all_resp);

  if (rebuild_flag) // update the coefficients for each approximation
    rebuild_approximation(all_resp);

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n<<<<< " << surrogateType
	 << " approximation updates completed.\n";
}


/** This function populates/replaces SurrogateData::anchor{Vars,Resp}
    and rebuilds the approximation, if requested.  It does not clear
    other data (i.e., SurrogateData::{vars,resp}Data) and does not
    update the actualModel with revised bounds, labels, etc.  Thus, it
    updates data from a previous call to build_approximation(), and is
    not intended to be used in isolation. */
void DataFitSurrModel::
update_approximation(const Variables& vars, const IntResponsePair& response_pr,
		     bool rebuild_flag)
{
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n>>>>> Updating " << surrogateType << " approximations.\n";

  // populate/replace the anchor point for each approximation
  approxInterface.update_approximation(vars, response_pr); // update anchor pt

  if (rebuild_flag)
    rebuild_approximation(response_pr);

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n<<<<< " << surrogateType
	 << " approximation updates completed.\n";
}


/** This function populates/replaces SurrogateData::{vars,resp}Data
    and rebuilds the approximation, if requested.  It does not clear
    other data (i.e., SurrogateData::anchor{Vars,Resp}) and does not
    update the actualModel with revised bounds, labels, etc.  Thus, it
    updates data from a previous call to build_approximation(), and is
    not intended to be used in isolation. */
void DataFitSurrModel::
update_approximation(const VariablesArray& vars_array,
		     const IntResponseMap& resp_map, bool rebuild_flag)
{
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n>>>>> Updating " << surrogateType << " approximations.\n";

  // populate/replace the current points for each approximation
  approxInterface.update_approximation(vars_array, resp_map);

  if (rebuild_flag)
    rebuild_approximation(resp_map);

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n<<<<< " << surrogateType
	 << " approximation updates completed.\n";
}


/** This function populates/replaces SurrogateData::{vars,resp}Data
    and rebuilds the approximation, if requested.  It does not clear
    other data (i.e., SurrogateData::anchor{Vars,Resp}) and does not
    update the actualModel with revised bounds, labels, etc.  Thus, it
    updates data from a previous call to build_approximation(), and is
    not intended to be used in isolation. */
void DataFitSurrModel::
update_approximation(const RealMatrix& samples, const IntResponseMap& resp_map,
		     bool rebuild_flag)
{
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n>>>>> Updating " << surrogateType << " approximations.\n";

  // populate/replace the current points for each approximation
  approxInterface.update_approximation(samples, resp_map);

  if (rebuild_flag)
    rebuild_approximation(resp_map);

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n<<<<< " << surrogateType
	 << " approximation updates completed.\n";
}


/** This function appends all{Samples,Variables,Responses} to
    SurrogateData::{vars,resp}Data and rebuilds the approximation,
    if requested. */
void DataFitSurrModel::append_approximation(bool rebuild_flag)
{
  // append to the current points for each approximation
  //daceIterator.run(pl_iter);
  const IntResponseMap& all_resp = daceIterator.all_responses();
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n>>>>> Appending " << all_resp.size() << " points to "
	 << surrogateType << " approximations.\n";
  if (daceIterator.compact_mode())
    approxInterface.append_approximation(daceIterator.all_samples(),  all_resp);
  else
    approxInterface.append_approximation(daceIterator.all_variables(),all_resp);

  if (rebuild_flag)
    rebuild_approximation(all_resp); // all_resp used to define build_fns

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n<<<<< " << surrogateType
	 << " approximation updates completed.\n";
}


/** This function appends one point to SurrogateData::{vars,resp}Data
    and rebuilds the approximation, if requested.  It does not modify
    other data (i.e., SurrogateData::anchor{Vars,Resp}) and does not
    update the actualModel with revised bounds, labels, etc.  Thus, it
    appends to data from a previous call to build_approximation(), and
    is not intended to be used in isolation. */
void DataFitSurrModel::
append_approximation(const Variables& vars, const IntResponsePair& response_pr,
		     bool rebuild_flag)
{
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n>>>>> Appending to " << surrogateType << " approximations.\n";

  // append to the current points for each approximation
  approxInterface.append_approximation(vars, response_pr);

  if (rebuild_flag)
    rebuild_approximation(response_pr); // response_pr used to define build_fns

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n<<<<< " << surrogateType
	 << " approximation updates completed.\n";
}


/** This function appends multiple points to SurrogateData::{vars,resp}Data
    and rebuilds the approximation, if requested.  It does not modify other 
    data (i.e., SurrogateData::anchor{Vars,Resp}) and does not update the
    actualModel with revised bounds, labels, etc.  Thus, it appends to data
    from a previous call to build_approximation(), and is not intended to
    be used in isolation. */
void DataFitSurrModel::
append_approximation(const RealMatrix& samples, const IntResponseMap& resp_map,
		     bool rebuild_flag)
{
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n>>>>> Appending to " << surrogateType << " approximations.\n";

  // append to the current points for each approximation
  approxInterface.append_approximation(samples, resp_map);

  if (rebuild_flag)
    rebuild_approximation(resp_map);

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n<<<<< " << surrogateType
	 << " approximation updates completed.\n";
}


/** This function appends multiple points to SurrogateData::{vars,resp}Data
    and rebuilds the approximation, if requested.  It does not modify other 
    data (i.e., SurrogateData::anchor{Vars,Resp}) and does not update the
    actualModel with revised bounds, labels, etc.  Thus, it appends to data
    from a previous call to build_approximation(), and is not intended to
    be used in isolation. */
void DataFitSurrModel::
append_approximation(const VariablesArray& vars_array,
		     const IntResponseMap& resp_map, bool rebuild_flag)
{
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n>>>>> Appending to " << surrogateType << " approximations.\n";

  // append to the current points for each approximation
  approxInterface.append_approximation(vars_array, resp_map);

  if (rebuild_flag)
    rebuild_approximation(resp_map); // resp_map used to define build_fns

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n<<<<< " << surrogateType
	 << " approximation updates completed.\n";
}


/** This function appends multiple points to SurrogateData::{vars,resp}Data
    and rebuilds the approximation, if requested.  It does not modify other 
    data (i.e., SurrogateData::anchor{Vars,Resp}) and does not update the
    actualModel with revised bounds, labels, etc.  Thus, it appends to data
    from a previous call to build_approximation(), and is not intended to
    be used in isolation. */
void DataFitSurrModel::
append_approximation(const IntVariablesMap& vars_map,
		     const IntResponseMap&  resp_map, bool rebuild_flag)
{
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n>>>>> Appending to " << surrogateType << " approximations.\n";

  // append to the current points for each approximation
  approxInterface.append_approximation(vars_map, resp_map);

  if (rebuild_flag)
    rebuild_approximation(resp_map); // resp_map used to define build_fns

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n<<<<< " << surrogateType
	 << " approximation updates completed.\n";
}


void DataFitSurrModel::
replace_approximation(const IntResponsePair& response_pr, bool rebuild_flag)
{
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n>>>>> Replacing response id " << response_pr.first << " in "
	 << surrogateType << " approximations.\n";

  // append to the current points for each approximation
  approxInterface.replace_approximation(response_pr);

  if (rebuild_flag)
    rebuild_approximation(response_pr); // response_pr used to define build_fns

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n<<<<< " << surrogateType
	 << " approximation data replacement completed.\n";
}


void DataFitSurrModel::
replace_approximation(const IntResponseMap& resp_map, bool rebuild_flag)
{
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n>>>>> Replacing response data in " << surrogateType
	 << " approximations.\n";

  // append to the current points for each approximation
  approxInterface.replace_approximation(resp_map);

  if (rebuild_flag)
    rebuild_approximation(resp_map);

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n<<<<< " << surrogateType
	 << " approximation data replacements completed.\n";
}


void DataFitSurrModel::pop_approximation(bool save_surr_data, bool rebuild_flag)
{
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n>>>>> Popping data from " << surrogateType
	 << " approximations.\n";

  // remove the most recent data appends from each approximation, where the
  // number of points to pop is tracked by pop counts at a lower level.
  // Typical use is to pop a candidate refinement following its evaluation.
  approxInterface.pop_approximation(save_surr_data);

  if (rebuild_flag) { // update the coefficients for each approximation
    BitArray rebuild_fns; // empty: default rebuild of all fns
    approxInterface.rebuild_approximation(rebuild_fns);
    ++approxBuilds;
  }

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n<<<<< " << surrogateType
         << " approximation data removal completed.\n";
}


void DataFitSurrModel::push_approximation()//(bool rebuild_flag)
{
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n>>>>> Retrieving " << surrogateType << " approximation data.\n";

  // restore one of the previously popped data sets for each approximation,
  // where the data set to restore is tracked by the push index at a lower
  // level.  Typical use is to select the best candidate refinement from
  // previously popped data sets.
  approxInterface.push_approximation();

  /*
  if (rebuild_flag) { // update the coefficients for each approximation
    BitArray rebuild_fns; // empty: default rebuild of all fns
    approxInterface.rebuild_approximation(rebuild_fns);
    ++approxBuilds;
  }
  */

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n<<<<< " << surrogateType << " approximation data retrieved.\n";
}


void DataFitSurrModel::finalize_approximation()//(bool rebuild_flag)
{
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n>>>>> Finalizing " << surrogateType << " approximations.\n";

  // restore all remaining popped data sets to finalize each approximation
  approxInterface.finalize_approximation();

  /*
  if (rebuild_flag) { // update the coefficients for each approximation
    BitArray rebuild_fns; // empty: default rebuild of all fns
    approxInterface.rebuild_approximation(rebuild_fns);
    ++approxBuilds;
  }
  */

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n<<<<< " << surrogateType << " approximation finalized.\n";
}


void DataFitSurrModel::combine_approximation()
{
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n>>>>> Combining " << surrogateType << " approximations.\n";

  // Manage swap detection here or within Pecos::sharedPolyApproxData?
  // Note: access to spec sequences are on Dakota side, but need to reach
  // into Pecos driver levels (TPQ, SSG) and approx orders (regression) for
  // access to final refinement levels (GSG starting levels could be the same).
  //NonDIntegration* nond_int = (NonDIntegration*)daceIterator.iterator_rep();
  //bool swap = !nond_int->maximal_grid();

  approxInterface.combine_approximation();
}


void DataFitSurrModel::combined_to_active(bool clear_combined)
{
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n>>>>> Promoting combined " << surrogateType << " approximation "
	 << "to active approximation.\n";

  approxInterface.combined_to_active(clear_combined);
}


void DataFitSurrModel::update_local_reference()
{
  // Store the actualModel inactive variable values for use in force_rebuild()
  // for determining whether an automatic approximation rebuild is required.

  // the actualModel data has been updated by update_model(), which precedes
  // update_local_reference()

  const Variables& actual_vars = actualModel.current_variables();
  if (actual_vars.view().first >= RELAXED_DESIGN) { // Distinct view
    copy_data(actual_vars.inactive_continuous_variables(),    referenceICVars);
    copy_data(actual_vars.inactive_discrete_int_variables(),  referenceIDIVars);
    copy_data(actual_vars.inactive_discrete_real_variables(), referenceIDRVars);
  }
}


void DataFitSurrModel::update_global_reference()
{
  // Store the actualModel active variable bounds and inactive variable values
  // for use in force_rebuild() to determine whether an automatic approximation
  // rebuild is required.

  // the actualModel data has been updated by update_model(), which precedes
  // update_global_reference().

  const Variables& vars = (actualModel.is_null()) ? currentVariables :
    actualModel.current_variables();
  if (vars.view().first >= RELAXED_DESIGN) { // Distinct view
    copy_data(vars.inactive_continuous_variables(),    referenceICVars);
    copy_data(vars.inactive_discrete_int_variables(),  referenceIDIVars);
    copy_data(vars.inactive_discrete_real_variables(), referenceIDRVars);
  }

  if (!actualModel.is_null() && actualModel.model_type() == "recast") {
    // dive through Model recursion to bypass recasting
    Model sub_model = actualModel.subordinate_model();
    while (sub_model.model_type() == "recast")
      sub_model = sub_model.subordinate_model();
    // update referenceCLBnds/referenceCUBnds/referenceDLBnds/referenceDUBnds
    copy_data(sub_model.continuous_lower_bounds(),    referenceCLBnds);
    copy_data(sub_model.continuous_upper_bounds(),    referenceCUBnds);
    copy_data(sub_model.discrete_int_lower_bounds(),  referenceDILBnds);
    copy_data(sub_model.discrete_int_upper_bounds(),  referenceDIUBnds);
    copy_data(sub_model.discrete_real_lower_bounds(), referenceDRLBnds);
    copy_data(sub_model.discrete_real_upper_bounds(), referenceDRUBnds);
  }
  else {
    const Constraints& cons = (actualModel.is_null()) ? userDefinedConstraints :
      actualModel.user_defined_constraints();
    copy_data(cons.continuous_lower_bounds(),    referenceCLBnds);
    copy_data(cons.continuous_upper_bounds(),    referenceCUBnds);
    copy_data(cons.discrete_int_lower_bounds(),  referenceDILBnds);
    copy_data(cons.discrete_int_upper_bounds(),  referenceDIUBnds);
    copy_data(cons.discrete_real_lower_bounds(), referenceDRLBnds);
    copy_data(cons.discrete_real_upper_bounds(), referenceDRUBnds);
  }
}


void DataFitSurrModel::clear_approx_interface()
{
  // fresh build: clear out previous data, but preserve history if needed
  // (multipoint preserves history for both {build,rebuild}_approximation())
  approxInterface.clear_current_active_data();
}


void DataFitSurrModel::
update_approx_interface(const Variables& vars,
			const IntResponsePair& response_pr)
{
  // fresh build: clear out previous data, but preserve history if needed
  // (multipoint preserves history for both {build,rebuild}_approximation())
  approxInterface.clear_current_active_data();
  // populate/replace the anchor data.  When supported by the surrogate type
  // (local, multipoint, equality-constrained global regression), this is
  // enforced as a hard constraint. Otherwise, it is just another data point.
  approxInterface.update_approximation(vars, response_pr);
}


void DataFitSurrModel::build_approx_interface()
{
  if (actualModel.is_null())
    approxInterface.build_approximation(
      userDefinedConstraints.continuous_lower_bounds(),
      userDefinedConstraints.continuous_upper_bounds(),
      userDefinedConstraints.discrete_int_lower_bounds(),
      userDefinedConstraints.discrete_int_upper_bounds(),
      userDefinedConstraints.discrete_real_lower_bounds(),
      userDefinedConstraints.discrete_real_upper_bounds());
  else { // employ sub-model vars view, if available
    approxInterface.build_approximation(
      actualModel.continuous_lower_bounds(),
      actualModel.continuous_upper_bounds(),
      actualModel.discrete_int_lower_bounds(),
      actualModel.discrete_int_upper_bounds(),
      actualModel.discrete_real_lower_bounds(),
      actualModel.discrete_real_upper_bounds());
  }
  if (exportSurrogate) {
    // skip the ApproximationInterface layer and go directly to
    // Approximations this could pass in the response name too,
    // presumably, but the API for export_model requires all
    // {resp_label, prefix, format} parameters or none to handle
    // whether comes from shared data vs. Model...
    for (auto approx : approximations())
      approx.export_model(currentVariables);
  }
}


/** Evaluate the value, gradient, and possibly Hessian needed for a
    local or multipoint approximation using actualModel. */
void DataFitSurrModel::build_local_multipoint()
{
  // set DataFitSurrModel parallelism mode to actualModel
  component_parallel_mode(TRUTH_MODEL_MODE);

  // Define the data requests
  short asv_value = 3;
  if (strbegins(surrogateType, "local_") &&
      actualModel.hessian_type() != "none")
    asv_value += 4;
  ShortArray orig_asv(numFns), actual_asv;
  StSIter it;
  for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it)
    orig_asv[*it] = asv_value;
  asv_inflate_build(orig_asv, actual_asv);

  // Evaluate value and derivatives using actualModel
  ActiveSet set = actualModel.current_response().active_set(); // copy
  set.request_vector(actual_asv);
  set.derivative_vector(actualModel.current_variables().continuous_variable_ids());
  actualModel.evaluate(set);

  // construct a new approximation using this actualModel evaluation
  build_local_multipoint(actualModel.current_variables(),
			 IntResponsePair(actualModel.evaluation_id(),
					 actualModel.current_response()));
}


void DataFitSurrModel::
build_local_multipoint(const Variables& vars,
		       const IntResponsePair& response_pr)
{
  // push the anchor data to approxInterface
  update_approx_interface(vars, response_pr);
  // construct the new local/multipoint approximation
  build_approx_interface();
  ++approxBuilds;
}


/** Determine points to use in building the approximation and then
    evaluate them on actualModel using daceIterator.  Any changes to
    the bounds should be performed by setting them at a higher level
    (e.g., SurrBasedOptStrategy). */
void DataFitSurrModel::build_global()
{
  // build_global() follows update_model() so we may use
  // actualModel.continuous_(lower/upper)_bounds() to avoid view
  // conversions and allow pass-by-reference.

  // **************************************************************************
  // Check data_pairs and importPointsFile for any existing evaluations to reuse
  // **************************************************************************
  size_t i, j, reuse_points = 0;
  int fn_index = *surrogateFnIndices.begin();
  const Pecos::SurrogateData& approx_data
    = approxInterface.approximation_data(fn_index);
  bool anchor = approx_data.anchor();
  if (pointReuse == "all" || pointReuse == "region") {

    size_t num_c_vars, num_di_vars, num_dr_vars;
    if (actualModel.is_null()) {
      num_c_vars  = currentVariables.cv();
      num_di_vars = currentVariables.div();
      num_dr_vars = currentVariables.drv();
    }
    else {
      num_c_vars  = actualModel.current_variables().cv();
      num_di_vars = actualModel.current_variables().div();
      num_dr_vars = actualModel.current_variables().drv();
    }

    // Process PRPCache using default iterators (index 0 = ordered_non_unique).
    // This includes evals from current run, evals imported from restart, and 
    // evals imported from a tabular file (DataFitSurrModel::import_points()).
    // Each of these data_pairs sources is in "user-space" (e.g., generated by
    // an ApplicationInterface).  To compare with DataFitSurrModel values and
    // bounds, any recastings within the model recursion must be managed.
    String am_interface_id;
    if (!actualModel.is_null())  am_interface_id = actualModel.interface_id();
    if (am_interface_id.empty()) am_interface_id = "NO_ID";
    ModelLRevIter ml_rit; PRPCacheCIter prp_iter;
    Variables db_vars; Response db_resp;
    bool map_to_iter_space = recastings();
    for (prp_iter=data_pairs.begin(); prp_iter!=data_pairs.end(); ++prp_iter) {

      const Variables& prp_vars = prp_iter->variables();
      const Response&  prp_resp = prp_iter->response();
      if (prp_iter->interface_id() == am_interface_id && consistent(prp_vars)) {
	// apply any recastings below this level: we perform these recastings at
	// run time (instead of once in import_points()) to support any updates
	// to the transformations (e.g., distribution parameter updates).
	if (map_to_iter_space)
	  user_space_to_iterator_space(prp_vars, prp_resp, db_vars, db_resp);
	else
	  { db_vars = prp_vars; db_resp = prp_resp; }

	// Note: since SurrBasedLocalMinimizer currently evaluates the trust
	// region center first, we must take care to not include this point
	// in the point reuse, since this would cause it to be used twice.
	// Note: for NonD uses with u-space models, the global_bounds boolean
	// in ProbabilityTransformModel ctor needs to be set in order to allow
	// test of transformed bounds in "region" reuse case.  For "all" reuse
	// case typically used with data import, this is not necessary.
	if ( inside(db_vars) && !(anchor && // avoid anchor duplic
	     active_vars_compare(db_vars, approx_data.anchor_variables())) ) {
	  // Eval id definitions:
	  //   id > 0 for unique evals from current execution
	  //   id = 0 for evals from file import --> data_pairs
	  //   id < 0 for non-unique evals from restart
	  // update one point at a time since accumulation within an
	  // IntResponseMap requires unique id's
	  approxInterface.append_approximation(db_vars,
	    std::make_pair(prp_iter->eval_id(), db_resp));
	  ++reuse_points;

	  if (outputLevel >= DEBUG_OUTPUT) {
	    if (map_to_iter_space) Cout <<   "Transformed ";
	    else                   Cout << "Untransformed ";
	    Cout << "data for DB eval " << prp_iter->eval_id() << ":\n"
		 << db_vars << db_resp;
	  }
	}
      }
    }
  }

  // *******************************************
  // Evaluate new data points using daceIterator
  // *******************************************
  int new_points = 0;
  if (daceIterator.is_null()) { // reused/imported data only (no new data)
    // check for sufficient data
    int min_points = approxInterface.minimum_points(true);
    if (reuse_points < min_points) {
      Cerr << "Error: a minimum of " << min_points << " points is required by "
	   << "DataFitSurrModel::build_global.\n" << reuse_points
	   << " were provided." << std::endl;
      abort_handler(MODEL_ERROR);
    }
  }
  else { // new data

    // set DataFitSurrModel parallelism mode to actualModel
    component_parallel_mode(TRUTH_MODEL_MODE);

    // daceIterator must generate at least diff_points samples, should
    // populate allData lists (allDataFlag = true), and should bypass
    // statistics computation (statsFlag = false).
    int diff_points = std::max(0, required_points() - (int)reuse_points);
    daceIterator.sampling_reset(diff_points, true, false);// update s.t. lwr bnd
    // The DACE iterator's samples{Spec,Ref} value provides a lower bound on
    // the number of samples generated: new_points = max(diff_points,reference).
    new_points = daceIterator.num_samples();

    // only run the iterator if work to do
    if (new_points) {
      run_dace();
      append_approximation(false); // append new data sets; defer build
    }
    else if (outputLevel >= DEBUG_OUTPUT)
      Cout << "DataFitSurrModel: No samples needed from DACE iterator."
	   << std::endl;
  }

  //deltaCorr.compute(...need data...);
  // could add deltaCorr.compute() here and in EnsembleSurrModel::
  // build_approximation if global approximations had easy access
  // to the truth/approx responses.  Instead, it is called from
  // SurrBasedLocalMinimizer using data from the trust region center.

  // **********************************
  // Now build the global approximation
  // **********************************
  String anchor_str = (anchor) ? "one" : "no";
  Cout << "Constructing global approximations with " << anchor_str
       << " anchor, " << new_points << " DACE samples, and " << reuse_points
       << " reused points.\n";
  if (autoRefine) refine_surrogate(); // BMA TODO: Move to an external refiner
  else            build_approx_interface();
  ++approxBuilds; // Note: auto-refined surrogate counts as 1 approx build
}


/** Determine points to use in rebuilding the approximation and
    then evaluate them on actualModel using daceIterator.  Assumes
    data imports/reuse have been handled previously within build_global(). */
void DataFitSurrModel::rebuild_global()
{
  // rebuild_global() follows update_model() so we may use
  // actualModel.continuous_(lower/upper)_bounds() to avoid view
  // conversions and allow pass-by-reference.

  // *******************************************
  // Evaluate new data points using daceIterator
  // *******************************************
  size_t pts_i, curr_points = SZ_MAX;
  StSIter it;
  for (it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it) {
    pts_i = approxInterface.approximation_data(*it).points();
    if (pts_i < curr_points) curr_points = pts_i;
  }
  int new_points = 0;
  if (daceIterator.is_null()) { // reused/imported data only (no new data)
    // check for sufficient data
    int min_points = approxInterface.minimum_points(true);
    if (curr_points < min_points) {
      Cerr << "Error: a minimum of " << min_points << " points is required by "
	   << "DataFitSurrModel::build_global.\n" << curr_points
	   << " were provided." << std::endl;
      abort_handler(MODEL_ERROR);
    }
  }
  else { // new data

    // set DataFitSurrModel parallelism mode to actualModel
    component_parallel_mode(TRUTH_MODEL_MODE);

    int diff_points = std::max(0, required_points() - (int)curr_points);
    if (diff_points) { // only run the iterator if work to do
      // For a rebuild, we do not enforce a lower bound as in build_global():
      // daceIterator's samples{Spec,Ref} is intended to overlay minimum user
      // spec with imported/reqd data, by defining a lower bound on the number
      // of generated samples, e.g. new_points = max(diff_points, samplesRef)
      daceIterator.sampling_reference(0); // make new points = diff points
      // daceIterator generates diff_points samples, populates allData arrays
      // (allDataFlag = true), bypasses stats computation (statsFlag = false)
      daceIterator.sampling_reset(diff_points, true, false);
      run_dace();
      // append new data sets, rebuild approximation, increment approxBuilds
      append_approximation(true);
    }
    else if (approxInterface.formulation_updated()) {
      // Rebuild the approximation for updated formulation with existing data

      // This approach currently assumes an increment to data and coeffs:
      //BitArray rebuild_fns; // empty: default rebuild of all fns
      //approxInterface.rebuild_approximation(rebuild_fns);

      // Overwrite previous build without assumed increment to data/coeffs:
      build_approx_interface();
      ++approxBuilds; // Note: this is a replacement rather than an increment...
    }
    else if (outputLevel >= DEBUG_OUTPUT)
      Cout << "DataFitSurrModel: no rebuild as no new data and same surrogate "
	   << "formulation." << std::endl;
  }
}


void DataFitSurrModel::run_dace()
{
  // Execute the daceIterator

  // daceIterator activeSet gets resized in DataFitSurrModel::
  // resize_from_subordinate_model(), but can be overwritten by top-level
  // Iterator (e.g., NonDExpansion::compute_expansion()
  const ShortArray& dace_asv = daceIterator.active_set_request_vector();
  if (dace_asv.size() != actualModel.response_size()) {
    ShortArray actual_asv;
    asv_inflate_build(dace_asv, actual_asv);
    daceIterator.active_set_request_vector(actual_asv);
  }

  // prepend hierarchical tag before running
  if (hierarchicalTagging) {
    String eval_tag = evalTagPrefix + '.' + std::to_string(surrModelEvalCntr+1);
    daceIterator.eval_tag_prefix(eval_tag);
  }

  // run the iterator
  ParLevLIter pl_iter = modelPCIter->mi_parallel_level_iterator(miPLIndex);
  daceIterator.run(pl_iter);
}


void DataFitSurrModel::refine_surrogate()
{
  StringArray diag_metrics(1, refineCVMetric);
  size_t curr_iter = 0; // initial surrogate build is iteration 0

  // accumulate num_samples in each iteration in total_evals.
  int total_evals = 0, num_samples;

  // accumulator for rolling average of length softConvergenceLimit
  using namespace boost::accumulators;
  accumulator_set<Real, stats<tag::rolling_mean> >
    mean_err(tag::rolling_window::window_size = softConvergenceLimit);

  num_samples = daceIterator.num_samples();
  total_evals += num_samples;

  // build surrogate from initial sample
  build_approx_interface();
  Real2DArray cv_diags = 
    approxInterface.cv_diagnostics(diag_metrics, refineCVFolds);
  size_t resp_fns = currentResponse.num_functions();
  RealArray cv_per_fn(resp_fns);
  for (size_t i=0; i<resp_fns; ++i)
    cv_per_fn[i] = cv_diags[i][0];
  Real curr_err = *std::max_element(cv_per_fn.begin(), cv_per_fn.end());
  // keep prev_err to calculate improvement
  Real prev_err = curr_err;
  Cout << "\n------------\nAuto-refinement initial error (" << refineCVMetric 
    << "): " << curr_err << std::endl;
  while (true) {
    // convergence conditions. messages will need to be fixed if we add
    // challenge error
    if (curr_err <= convergenceTolerance) {
      Cout << "\n------------\nAuto-refined surrogate(s) converged: "
	   << "Cross-validation error criterion met.\n";
      break;
    } else if (curr_iter++ == maxIterations) {
      Cout << "\n------------\nAuto-refinment halted: Maximum iterations "
	   << "met or exceeded.\n";
      break;
    } else if (softConvergenceLimit && curr_iter >= softConvergenceLimit &&
	       rolling_mean(mean_err) < convergenceTolerance) {
      Cout << "\n------------\nAuto-refinment halted: Average reduction in "
	   << "cross-validation error over\nprevious " << softConvergenceLimit
	   << " iterations less than " << "convergence_tolerance ("
	   << convergenceTolerance << ")\n";
      break;
    } else if (total_evals >= maxFuncEvals) {
      Cout << "\n------------\nAuto-refinment halted: Maximum function "
	   << "evaluations met or exceeded.\n";
      break;
    }

    // sampling reset only resets the base numSamples; if there are
    // refinement samples, increment them here and add points
    daceIterator.sampling_increment();
    num_samples = daceIterator.num_samples();
    total_evals += num_samples;
    Cout << "\n------------\nRefining surrogate(s) with " << num_samples 
	 << " samples (iteration " << curr_iter << ")\n";
    run_dace();
    append_approximation(false); // append new data sets; don't rebuild

    // build and check diagnostics
    build_approx_interface();
    Real2DArray cv_diags = 
      approxInterface.cv_diagnostics(diag_metrics, refineCVFolds);
    RealArray cv_per_fn(resp_fns);
    for (size_t i=0; i<resp_fns; ++i)
      cv_per_fn[i] = cv_diags[i][0];
    curr_err = *std::max_element(cv_per_fn.begin(), cv_per_fn.end());
    Cout << "\n------------\nAuto-refinement iteration " << curr_iter 
      << " complete.\n";
    Cout << "Cross-validation error (" << refineCVMetric << "): " << curr_err
	 << std::endl;
    mean_err(prev_err-curr_err);
    prev_err = curr_err;
    Cout << "Mean reduction in CV error: " << rolling_mean(mean_err)
	 << std::endl;
  }
}


bool DataFitSurrModel::consistent(const Variables& vars) const
{
  size_t i, num_acv = vars.acv(), num_adiv = vars.adiv(),
    num_adsv = vars.adsv(), num_adrv = vars.adrv(), cv_start = vars.cv_start(),
    div_start = vars.div_start(), dsv_start = vars.dsv_start(),
    drv_start = vars.drv_start(), num_cv = vars.cv(), num_div = vars.div(),
    num_dsv = vars.dsv(), num_drv = vars.drv();

  const Variables& am_vars = (actualModel.is_null()) ?
    currentVariables : actualModel.current_variables();

  if (am_vars.acv()       != num_acv   || am_vars.adiv()      != num_adiv || 
      am_vars.adsv()      != num_adsv  || am_vars.adrv()      != num_adrv || 
      am_vars.cv_start()  != cv_start  || am_vars.div_start() != div_start ||
      am_vars.dsv_start() != dsv_start || am_vars.drv_start() != drv_start ||
      am_vars.cv()        != num_cv    || am_vars.div()       != num_div  ||
      am_vars.dsv()       != num_dsv   || am_vars.drv()       != num_drv ) {
    Cerr << "Warning: inconsistent variable counts in DataFitSurrModel::"
	 << "consistent().  Excluding candidate data point.\n";
    return false;
  }

  size_t cv_end =  cv_start + num_cv,    div_end = div_start + num_div,
        dsv_end = dsv_start + num_dsv,   drv_end = drv_start + num_drv,
    num_post_cv = num_acv - cv_end, num_post_drv = num_adrv - drv_end;

  // This tolerance is important since imported data may lack full precision
  Real rel_tol = 1.e-10; // hardwired for now

  // complement of active cont vars must be identical (within rel tol)
  const RealVector&    acv =    vars.all_continuous_variables();
  const RealVector& am_acv = am_vars.all_continuous_variables();
  RealVector pre_cv(Teuchos::View, acv.values(),           cv_start),
            post_cv(Teuchos::View, acv.values()+cv_end,    num_post_cv),
          pre_am_cv(Teuchos::View, am_acv.values(),        cv_start),
         post_am_cv(Teuchos::View, am_acv.values()+cv_end, num_post_cv);
  if ( !nearby(pre_cv,  pre_am_cv,  rel_tol) ||
       !nearby(post_cv, post_am_cv, rel_tol) )
    return false;
  // for (i=0; i<cv_start; ++i)
  //   if (acv[i] != am_acv[i])
  //     return false;
  // for (i=cv_end; i<num_acv; ++i)
  //   if (acv[i] != am_acv[i])
  //     return false;
  // complement of active discrete int vars must be identical
  const IntVector& adiv = vars.all_discrete_int_variables();
  const IntVector& am_adiv = am_vars.all_discrete_int_variables();
  for (i=0; i<div_start; ++i)
    if (adiv[i] != am_adiv[i])
      return false;
  for (i=div_end; i<num_adiv; ++i)
    if (adiv[i] != am_adiv[i])
      return false;
  // complement of active discrete string vars must be identical
  StringMultiArrayConstView adsv = vars.all_discrete_string_variables();
  StringMultiArrayConstView am_adsv = am_vars.all_discrete_string_variables();
  for (i=0; i<dsv_start; ++i)
    if (adsv[i] != am_adsv[i])
      return false;
  for (i=dsv_end; i<num_adsv; ++i)
    if (adsv[i] != am_adsv[i])
      return false;
  // complement of active discrete real vars must be identical (within rel tol)
  const RealVector& adrv = vars.all_discrete_real_variables();
  const RealVector& am_adrv = am_vars.all_discrete_real_variables();
  RealVector pre_drv(Teuchos::View, adrv.values(),            drv_start),
            post_drv(Teuchos::View, adrv.values()+drv_end,    num_post_drv),
          pre_am_drv(Teuchos::View, am_adrv.values(),         drv_start),
         post_am_drv(Teuchos::View, am_adrv.values()+drv_end, num_post_drv);
  if ( !nearby(pre_drv,  pre_am_drv,  rel_tol) ||
       !nearby(post_drv, post_am_drv, rel_tol) )
    return false;
  // for (i=0; i<drv_start; ++i)
  //   if (adrv[i] != am_adrv[i])
  //     return false;
  // for (i=drv_end; i<num_adrv; ++i)
  //   if (adrv[i] != am_adrv[i])
  //     return false;

  return true;
}


bool DataFitSurrModel::inside(const Variables& vars) const
{
  // additionally check if within current bounds for "region" case
  if (pointReuse == "region") {

    const Constraints& am_cons = (actualModel.is_null()) ?
      userDefinedConstraints : actualModel.user_defined_constraints();
    const RealVector&  cv = vars.continuous_variables();
    const IntVector&  div = vars.discrete_int_variables();
    const RealVector& drv = vars.discrete_real_variables();
    size_t i, num_cv = cv.length(), num_div = div.length(),
      num_drv = drv.length();

    const RealVector& c_l_bnds = am_cons.continuous_lower_bounds();
    const RealVector& c_u_bnds = am_cons.continuous_upper_bounds();
    for (i=0; i<num_cv; ++i)
      if (cv[i] < c_l_bnds[i] || cv[i] > c_u_bnds[i])
	return false;

    const IntVector& di_l_bnds = am_cons.discrete_int_lower_bounds();
    const IntVector& di_u_bnds = am_cons.discrete_int_upper_bounds();
    for (i=0; i<num_div; ++i)
      if (div[i] < di_l_bnds[i] || div[i] > di_u_bnds[i])
	return false;

    // No check for active string variable bounds

    const RealVector& dr_l_bnds = am_cons.discrete_real_lower_bounds();
    const RealVector& dr_u_bnds = am_cons.discrete_real_upper_bounds();
    for (i=0; i<num_drv; ++i)
      if (drv[i] < dr_l_bnds[i] || drv[i] > dr_u_bnds[i])
	return false;
  }

  return true;
}


/** Compute the response synchronously using actualModel, approxInterface,
    or both (mixed case).  For the approxInterface portion, build the
    approximation if needed, evaluate the approximate response, and apply 
    correction (if active) to the results. */
void DataFitSurrModel::derived_evaluate(const ActiveSet& set)
{
  ++surrModelEvalCntr;

  ShortArray approx_asv, actual_asv; bool actual_eval, approx_eval, mixed_eval;
  Response actual_response, approx_response; // empty handles
  switch (responseMode) {
  case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
    asv_split(set.request_vector(), approx_asv, actual_asv);
    approx_eval = !approx_asv.empty(); actual_eval = !actual_asv.empty();
    mixed_eval = (approx_eval && actual_eval); break;
  case BYPASS_SURROGATE:
    approx_eval = false; actual_eval = true;   break;
  case MODEL_DISCREPANCY: case AGGREGATED_MODEL_PAIR:
    approx_eval = actual_eval = true;          break;
  }

  if (hierarchicalTagging) {
    String eval_tag = evalTagPrefix + '.' + std::to_string(surrModelEvalCntr+1);
    if (actual_eval)
      actualModel.eval_tag_prefix(eval_tag);
  }

  // -----------------------------
  // Compute actual model response
  // -----------------------------
  if (actual_eval) {
    component_parallel_mode(TRUTH_MODEL_MODE);
    update_model(actualModel); // update variables/bounds/labels in actualModel
    switch (responseMode) {
    case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE: {
      ActiveSet actual_set = set;
      actual_set.request_vector(actual_asv);
      actualModel.evaluate(actual_set);
      if (mixed_eval)
	actual_response = actualModel.current_response(); // shared rep
      else {
	currentResponse.active_set(actual_set);
	currentResponse.update(actualModel.current_response(), true);//pull meta
      }
      break;
    }
    case BYPASS_SURROGATE:
      actualModel.evaluate(set);
      currentResponse.active_set(set);
      currentResponse.update(actualModel.current_response(), true); // pull meta
      // TODO: Add to surrogate build data
      //      add_tabular_data(....)
      break;
    case MODEL_DISCREPANCY: case AGGREGATED_MODEL_PAIR:
      actualModel.evaluate(set);
      break;
    }
  }

  // ---------------------------------
  // Compute approx interface response
  // ---------------------------------
  if (approx_eval) { // normal case: evaluation of approxInterface
    // pre-process
    switch (responseMode) {
    case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
      // if build_approximation has not yet been called, call it now
      if (!approxBuilds || force_rebuild())
	build_approximation();
      break;
    }

    // compute the approximate response
    //component_parallel_mode(SURROGATE_MODEL_MODE); // does not use parallelism
    //ParConfigLIter pc_iter = parallelLib.parallel_configuration_iterator();
    //parallelLib.parallel_configuration_iterator(modelPCIter);
    if (interfEvaluationsDBState == EvaluationsDBState::UNINITIALIZED)
      interfEvaluationsDBState = evaluationsDB.interface_allocate(modelId, 
        approxInterface.interface_id(), "approximation", currentVariables,
	currentResponse, default_interface_active_set(),
	approxInterface.analysis_components());
    
    switch (responseMode) {
    case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE: {
      ActiveSet approx_set = set;
      approx_set.request_vector(approx_asv);
      approx_response = (mixed_eval) ? currentResponse.copy() : currentResponse;
      approxInterface.map(currentVariables, approx_set, approx_response);
      if (interfEvaluationsDBState == EvaluationsDBState::ACTIVE) {
        evaluationsDB.store_interface_variables(modelId,
	  approxInterface.interface_id(), approxInterface.evaluation_id(),
	  approx_set, currentVariables);
        evaluationsDB.store_interface_response(modelId,
	  approxInterface.interface_id(), approxInterface.evaluation_id(),
	  approx_response);
      }
      break;
    }
    case MODEL_DISCREPANCY: case AGGREGATED_MODEL_PAIR:
      approx_response = currentResponse.copy(); // TO DO
      approxInterface.map(currentVariables, set, approx_response);
      if (interfEvaluationsDBState == EvaluationsDBState::ACTIVE) {
        evaluationsDB.store_interface_variables(modelId,
	  approxInterface.interface_id(), approxInterface.evaluation_id(),
	  set, currentVariables);
        evaluationsDB.store_interface_response(modelId,
	  approxInterface.interface_id(), approxInterface.evaluation_id(),
	  approx_response);
      }
      break;
    }

    //parallelLib.parallel_configuration_iterator(pc_iter); // restore


    // export data (optional)
    if (!exportPointsFile.empty() || !exportVarianceFile.empty())
      export_point(surrModelEvalCntr, currentVariables, approx_response);

    // post-process
    switch (responseMode) {
    case AUTO_CORRECTED_SURROGATE: {
      bool quiet_flag = (outputLevel < NORMAL_OUTPUT);
      //if (!deltaCorr.computed())
      //  deltaCorr.compute(currentVariables, centerResponse, approx_response,
      //                    quiet_flag);
      deltaCorr.apply(currentVariables, approx_response, quiet_flag);
      break;
    }
    }
  }

  // --------------------------------------
  // perform any actual/approx aggregations
  // --------------------------------------
  switch (responseMode) {
  case MODEL_DISCREPANCY: {
    // don't update surrogate data within deltaCorr's Approximations; just
    // update currentResponse (managed as surrogate data at a higher level)
    bool quiet_flag = (outputLevel < NORMAL_OUTPUT);
    deltaCorr.compute(actualModel.current_response(), approx_response,
		      currentResponse, quiet_flag);
    break;
  }
  case AGGREGATED_MODEL_PAIR:
    aggregate_response(approx_response, actualModel.current_response(),
		       currentResponse);
    break;
  case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
    if (mixed_eval) {
      currentResponse.active_set(set);
      response_combine(actual_response, approx_response, currentResponse);
    }
    break;
  }
}


/** Compute the response asynchronously using actualModel,
    approxInterface, or both (mixed case).  For the approxInterface
    portion, build the approximation if needed and evaluate the
    approximate response in a quasi-asynchronous approach
    (ApproximationInterface::map() performs the map synchronously and
    bookkeeps the results for return in derived_synchronize() below). */
void DataFitSurrModel::derived_evaluate_nowait(const ActiveSet& set)
{
  ++surrModelEvalCntr;

  ShortArray approx_asv, actual_asv; bool approx_eval, actual_eval;
  switch (responseMode) {
  case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
    asv_split(set.request_vector(), approx_asv, actual_asv);
    approx_eval = !approx_asv.empty(); actual_eval = !actual_asv.empty(); break;
  case BYPASS_SURROGATE:
    approx_eval = false; actual_eval = true;                              break;
  case MODEL_DISCREPANCY: case AGGREGATED_MODEL_PAIR:
    approx_eval = actual_eval = true;                                     break;
  }

  if (hierarchicalTagging) {
    String eval_tag = evalTagPrefix + '.' + std::to_string(surrModelEvalCntr+1);
    if (actual_eval)
      actualModel.eval_tag_prefix(eval_tag);
  }

  // -----------------------------
  // Compute actual model response
  // -----------------------------
  if (actual_eval) {
    // don't need to set component parallel mode since this only queues the job
    update_model(actualModel); // update variables/bounds/labels in actualModel
    switch (responseMode) {
    case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE: {
      ActiveSet actual_set = set;
      actual_set.request_vector(actual_asv);
      actualModel.evaluate_nowait(actual_set); break;
    }
    case BYPASS_SURROGATE: case MODEL_DISCREPANCY: case AGGREGATED_MODEL_PAIR:
      actualModel.evaluate_nowait(set);        break;
    }
    // store mapping from actualModel eval id to DataFitSurrModel id
    truthIdMap[actualModel.evaluation_id()] = surrModelEvalCntr;
  }

  // ---------------------------------
  // Compute approx interface response
  // ---------------------------------
  if (approx_eval) { // normal case: evaluation of approxInterface
    // pre-process
    switch (responseMode) {
    case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
      // if build_approximation has not yet been called, call it now
      if (!approxBuilds || force_rebuild())
	build_approximation();
      break;
    }

    if (interfEvaluationsDBState == EvaluationsDBState::ACTIVE)
      evaluationsDB.interface_allocate(modelId, approxInterface.interface_id(),
	"approximation", currentVariables, currentResponse,
	default_interface_active_set(), approxInterface.analysis_components());

    // compute the approximate response
    // don't need to set component parallel mode since this only queues the job
    switch (responseMode) {
    case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE: {
      ActiveSet approx_set = set;
      approx_set.request_vector(approx_asv);
      approxInterface.map(currentVariables, approx_set, currentResponse, true);
      if (interfEvaluationsDBState == EvaluationsDBState::ACTIVE)
        evaluationsDB.store_interface_variables(modelId,
	  approxInterface.interface_id(), approxInterface.evaluation_id(),
	  approx_set, currentVariables);
      break;
    }
    case MODEL_DISCREPANCY: case AGGREGATED_MODEL_PAIR:
      approxInterface.map(currentVariables,        set, currentResponse, true);
      if(interfEvaluationsDBState == EvaluationsDBState::ACTIVE)
        evaluationsDB.store_interface_variables(modelId,
	  approxInterface.interface_id(), approxInterface.evaluation_id(),
	  set, currentVariables);
      break;
    }

    // post-process
    switch (responseMode) {
    case AUTO_CORRECTED_SURROGATE:
      rawVarsMap[surrModelEvalCntr] = currentVariables.copy(); break;
    default:
      if (!exportPointsFile.empty() || !exportVarianceFile.empty())
	rawVarsMap[surrModelEvalCntr] = currentVariables.copy();
      break;
    }
    // store map from approxInterface eval id to DataFitSurrModel id
    surrIdMap[approxInterface.evaluation_id()] = surrModelEvalCntr;
  }
}


/** Blocking retrieval of asynchronous evaluations from actualModel,
    approxInterface, or both (mixed case).  For the approxInterface
    portion, apply correction (if active) to each response in the array.
    derived_synchronize() is designed for the general case where
    derived_evaluate_nowait() may be inconsistent in its use
    of actual evaluations, approximate evaluations, or both. */
const IntResponseMap& DataFitSurrModel::derived_synchronize()
{
  surrResponseMap.clear();
  bool actual_evals = !truthIdMap.empty(), approx_evals = !surrIdMap.empty(),
    block = true;

  // -----------------------------
  // synchronize actualModel evals
  // -----------------------------
  IntResponseMap actual_resp_map_rekey;
  if (actual_evals) {
    component_parallel_mode(TRUTH_MODEL_MODE);

    // update map keys to use surrModelEvalCntr
    if (approx_evals)
      rekey_synch(actualModel, block, truthIdMap, actual_resp_map_rekey);
    else {
      rekey_synch(actualModel, block, truthIdMap, surrResponseMap);
      return surrResponseMap; // if no approx evals, return actual results
    }
  }

  // ---------------------------------
  // synchronize approxInterface evals
  // ---------------------------------
  IntResponseMap approx_resp_map_rekey;
  if (approx_evals) {
    // derived_synchronize() and derived_synchronize_nowait() share code since
    // approx_resp_map is complete in both cases
    if (actual_evals)
      derived_synchronize_approx(block, approx_resp_map_rekey);
    else {
      derived_synchronize_approx(block, surrResponseMap);
      return surrResponseMap; // if no approx evals, return actual results
    }
  }

  // --------------------------------------
  // perform any actual/approx aggregations
  // --------------------------------------
  // Both actual and approx evals are present: {actual,approx}_resp_map_rekey
  // may be partial sets (partial surrogateFnIndices in
  // {UN,AUTO_}CORRECTED_SURROGATE) or full sets (MODEL_DISCREPANCY).
  Response empty_resp;
  IntRespMCIter act_it = actual_resp_map_rekey.begin(),
                app_it = approx_resp_map_rekey.begin();
  switch (responseMode) {
  case MODEL_DISCREPANCY: {
    bool quiet_flag = (outputLevel < NORMAL_OUTPUT);
    for (; act_it != actual_resp_map_rekey.end() && 
	   app_it != approx_resp_map_rekey.end(); ++act_it, ++app_it) {
      check_key(act_it->first, app_it->first);
      deltaCorr.compute(act_it->second, app_it->second,
			surrResponseMap[act_it->first], quiet_flag);
    }
    break;
  }
  case AGGREGATED_MODEL_PAIR:
    for (; act_it != actual_resp_map_rekey.end() && 
	   app_it != approx_resp_map_rekey.end(); ++act_it, ++app_it) {
      check_key(act_it->first, app_it->first);
      aggregate_response(app_it->second, act_it->second,
			 surrResponseMap[act_it->first]);
    }
    break;
  default: // {UN,AUTO_}CORRECTED_SURROGATE modes
    // process any combination of HF and LF completions
    while (act_it != actual_resp_map_rekey.end() ||
	   app_it != approx_resp_map_rekey.end()) {
      int act_eval_id = (act_it == actual_resp_map_rekey.end()) ?
	INT_MAX : act_it->first;
      int app_eval_id = (app_it == approx_resp_map_rekey.end()) ?
	INT_MAX : app_it->first;

      if (act_eval_id < app_eval_id) // only HF available
	{ response_combine(act_it->second, empty_resp,
			   surrResponseMap[act_eval_id]); ++act_it; }
      else if (app_eval_id < act_eval_id) // only LF available
	{ response_combine(empty_resp, app_it->second,
			   surrResponseMap[app_eval_id]); ++app_it; }
      else // both LF and HF available
	{ response_combine(act_it->second, app_it->second,
			   surrResponseMap[act_eval_id]); ++act_it; ++app_it; }
    }
    break;
  }

  return surrResponseMap;
}


/** Nonblocking retrieval of asynchronous evaluations from
    actualModel, approxInterface, or both (mixed case).  For the
    approxInterface portion, apply correction (if active) to each
    response in the map.  derived_synchronize_nowait() is designed for
    the general case where derived_evaluate_nowait() may be
    inconsistent in its use of actual evals, approx evals, or both. */
const IntResponseMap& DataFitSurrModel::derived_synchronize_nowait()
{
  surrResponseMap.clear();
  bool actual_evals = !truthIdMap.empty(), approx_evals = !surrIdMap.empty(),
    block = false;

  // -----------------------------
  // synchronize actualModel evals
  // -----------------------------
  IntResponseMap actual_resp_map_rekey;
  if (actual_evals) {
    component_parallel_mode(TRUTH_MODEL_MODE);

    // update map keys to use surrModelEvalCntr
    if (approx_evals)
      rekey_synch(actualModel, block, truthIdMap, actual_resp_map_rekey);
    else {
      rekey_synch(actualModel, block, truthIdMap, surrResponseMap);
      return surrResponseMap; // if no approx evals, return actual results
    }
  }

  // ---------------------------------
  // synchronize approxInterface evals
  // ---------------------------------
  IntResponseMap approx_resp_map_rekey;
  if (approx_evals) {
    // derived_synchronize() and derived_synchronize_nowait() share code since
    // approx_resp_map is complete in both cases
    if (actual_evals)
      derived_synchronize_approx(block, approx_resp_map_rekey);
    else {
      derived_synchronize_approx(block, surrResponseMap);
      return surrResponseMap; // if no approx evals, return actual results
    }
  }

  // --------------------------------------
  // perform any approx/actual aggregations
  // --------------------------------------
  // Both actual and approx evals are present:
  // > actual_resp_map_rekey may be a partial set of evals (partial
  //   surrogateFnIndices in {UN,AUTO_}CORRECTED_SURROGATE modes) or
  //   full sets (MODEL_DISCREPANCY mode)
  // > approx_resp_map_rekey is a complete set of evals
  Response empty_resp;
  IntRespMCIter act_it = actual_resp_map_rekey.begin(),
                app_it = approx_resp_map_rekey.begin();
  bool quiet_flag = (outputLevel < NORMAL_OUTPUT);
  // remaining values from truthIdMap key-value pairs
  IntSet remain_truth_ids; IntIntMIter im_it;
  for (im_it=truthIdMap.begin(); im_it!=truthIdMap.end(); ++im_it)
    remain_truth_ids.insert(im_it->second);
  // process any combination of actual and approx completions
  while (act_it != actual_resp_map_rekey.end() ||
	 app_it != approx_resp_map_rekey.end()) {
    int act_eval_id = (act_it == actual_resp_map_rekey.end()) ?
      INT_MAX : act_it->first;
    int app_eval_id = (app_it == approx_resp_map_rekey.end()) ?
      INT_MAX : app_it->first;
    // process approx/actual results or cache them for next pass
    if (act_eval_id < app_eval_id) { // only actual available
      switch (responseMode) {
      case MODEL_DISCREPANCY: case AGGREGATED_MODEL_PAIR:
	Cerr << "Error: approx eval missing in DataFitSurrModel::"
	     << "derived_synchronize_nowait()" << std::endl;
	abort_handler(MODEL_ERROR); break;
      default: // {UN,AUTO_}CORRECTED_SURROGATE modes
	// there is no approx component to this response
	response_combine(act_it->second, empty_resp,
			 surrResponseMap[act_eval_id]);
	break;
      }
      ++act_it;
    }
    else if (app_eval_id < act_eval_id) { // only approx available
      switch (responseMode) {
      case MODEL_DISCREPANCY: case AGGREGATED_MODEL_PAIR:
	// cache approx response since actual contribution not yet available
	cachedApproxRespMap[app_eval_id] = app_it->second; break;
      default: // {UN,AUTO_}CORRECTED_SURROGATE modes
	if (remain_truth_ids.find(app_eval_id) != remain_truth_ids.end())
	  // cache approx response since actual contribution still pending
	  cachedApproxRespMap[app_eval_id] = app_it->second;
	else // response complete: there is no actual contribution
	  response_combine(empty_resp, app_it->second, 
			   surrResponseMap[app_eval_id]);
	break;
      }
      ++app_it;
    }
    else { // both approx and actual available
      switch (responseMode) {
      case MODEL_DISCREPANCY:
	deltaCorr.compute(act_it->second, app_it->second,
			  surrResponseMap[act_eval_id], quiet_flag); break;
      case AGGREGATED_MODEL_PAIR:
	aggregate_response(app_it->second, act_it->second,
			   surrResponseMap[act_eval_id]);            break;
      default: // {UN,AUTO_}CORRECTED_SURROGATE modes
	response_combine(act_it->second, app_it->second,
			 surrResponseMap[act_eval_id]);              break;
      }
      ++act_it; ++app_it;
    }
  }

  return surrResponseMap;
}


void DataFitSurrModel::
derived_synchronize_approx(bool block, IntResponseMap& approx_resp_map_rekey)
{
  bool actual_evals = !truthIdMap.empty();

  //component_parallel_mode(SURROGATE_MODEL_MODE); // does not use parallelism
  //ParConfigLIter pc_iter = parallelLib.parallel_configuration_iterator();
  //parallelLib.parallel_configuration_iterator(modelPCIter);

  // synchronize and rekey to use surrModelEvalCntr
  rekey_synch(approxInterface, block, surrIdMap, approx_resp_map_rekey);

  //parallelLib.parallel_configuration_iterator(pc_iter); // restore

  IntRespMIter r_it;
  bool export_pts = !exportPointsFile.empty() || !exportVarianceFile.empty();
  if (responseMode == AUTO_CORRECTED_SURROGATE && corrType) {
    // Interface::rawResponseMap can be corrected directly in the case of an
    // ApproximationInterface since data_pairs is not used (not true for
    // EnsembleSurrModel::derived_synchronize()/derived_synchronize_nowait()).
    // The response map from ApproximationInterface's quasi-asynch mode is
    // complete and in order.

    bool quiet_flag = (outputLevel < NORMAL_OUTPUT);
    //if (!deltaCorr.computed() && !approx_resp_map_rekey.empty())
    //  deltaCorr.compute(rawVarsMap.begin()->second, ...,
    //                    approx_resp_map_rekey.begin()->second, quiet_flag);
    IntVarsMIter v_it;
    for (r_it  = approx_resp_map_rekey.begin(), v_it = rawVarsMap.begin();
	 r_it != approx_resp_map_rekey.end(); ++r_it, ++v_it) {
      deltaCorr.apply(v_it->second,//rawVarsMap[r_it->first],
		      r_it->second, quiet_flag);
      // decided to export auto-corrected approx response
      if (export_pts)
	export_point(r_it->first, v_it->second, r_it->second);
    }
    rawVarsMap.clear();
  }
  else if (export_pts) {
    IntVarsMIter v_it;
    for (r_it  = approx_resp_map_rekey.begin(), v_it = rawVarsMap.begin();
	 r_it != approx_resp_map_rekey.end(); ++r_it, ++v_it)
      export_point(r_it->first, v_it->second, r_it->second);
    rawVarsMap.clear();
  }

  // add cached evals (synchronized approx evals that could not be returned
  // since truth eval portions were still pending) for processing.  Do not
  // correct them a second time.
  for (r_it  = cachedApproxRespMap.begin();
       r_it != cachedApproxRespMap.end(); ++r_it)
    approx_resp_map_rekey[r_it->first] = r_it->second;
  cachedApproxRespMap.clear();
}


void DataFitSurrModel::
asv_inflate_build(const ShortArray& orig_asv, ShortArray& actual_asv)
{
  // DataFitSurrModel consumes replicates from any response aggregations
  // occurring in actualModel
  size_t num_orig = orig_asv.size(), num_actual = actualModel.response_size();
  if (num_actual < num_orig || num_actual % num_orig) {
    Cerr << "Error: ASV size mismatch in DataFitSurrModel::asv_inflate_build()."
	 << std::endl;
    abort_handler(MODEL_ERROR);
  }

  if (surrogateFnIndices.size() == numFns) {
    if (num_actual > num_orig) { // inflate actual_asv if needed
      actual_asv.resize(num_actual);
      for (size_t i=0; i<num_actual; ++i)
	actual_asv[i] = orig_asv[i % num_orig];
    }
    else
      actual_asv = orig_asv;
  }
  else { // mixed response set
    size_t i, index; short orig_asv_val;
    actual_asv.assign(num_actual, 0);
    for (StSIter it=surrogateFnIndices.begin();
	 it!=surrogateFnIndices.end(); ++it) {
      index = *it; orig_asv_val = orig_asv[index];
      if (orig_asv_val)
	for (i=index; i<num_actual; i+=num_orig) // inflate actual_asv
	  actual_asv[i] = orig_asv_val;
    }
  }
}


void DataFitSurrModel::
asv_split(const ShortArray& orig_asv, ShortArray& approx_asv,
	  ShortArray& actual_asv)
{
  if (actualModel.is_null() || surrogateFnIndices.size() == numFns)
    { approx_asv = orig_asv; return; } // don't inflate approx_asv
  // else mixed response set

  // DataFitSurrModel consumes replicates from any response aggregations
  // occurring in actualModel
  size_t num_orig = orig_asv.size(), num_actual = actualModel.response_size();
  if (num_orig != numFns || num_actual < num_orig || num_actual % num_orig) {
    Cerr << "Error: ASV size mismatch in DataFitSurrModel::asv_split()."
	 << std::endl;
    abort_handler(MODEL_ERROR);
  }
  int index; short orig_asv_val;
  for (index=0; index<num_orig; ++index) {
    orig_asv_val = orig_asv[index];
    if (orig_asv_val) {
      if (surrogateFnIndices.count(index)) {
	if (approx_asv.empty()) // keep empty if no active requests
	  approx_asv.assign(num_orig, 0);
	approx_asv[index] = orig_asv_val; // don't inflate approx_asv
      }
      else {
	if (actual_asv.empty()) // keep empty if no active requests
	  actual_asv.assign(num_actual, 0);
	for (size_t i=index; i<num_actual; i+=num_orig) // inflate actual_asv
	  actual_asv[i] = orig_asv_val;
      }
    }
  }
}


/** Constructor helper to read the points file once, if provided, and
    then reuse its data as appropriate within build_global().
    Surrogate data imports default to active/inactive variables, but
    user can override to active only */
void DataFitSurrModel::
import_points(unsigned short tabular_format, bool use_var_labels,
	      bool active_only)
{
  // Temporary objects to use to read correct size vars/resp; use copies
  // so that read_data_tabular() does not alter state of vars/resp objects
  // in Models (especially important for non-active variables).
  Variables vars = actualModel.is_null() ? currentVariables.copy() : 
    actualModel.current_variables().copy(); 
  Response  resp = actualModel.is_null() ? currentResponse.copy() : 
    actualModel.current_response().copy();
  size_t num_vars = active_only ? vars.total_active() : vars.tv();

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "Surrogate model retrieving points with " << num_vars
	 << " variables and " << numFns << " response\nfunctions from file '"
	 << importPointsFile << "'\n";
  // Preserves eval and interface ids in the PRPList, if annotated format
  // If no eval ID, will number successively from 1
  PRPList import_prp_list;
  bool verbose = (outputLevel > NORMAL_OUTPUT);
  String context_msg = "Surrogate model with id '" + model_id() +
    "' import_build_points";
  TabularIO::read_data_tabular(importPointsFile, context_msg, vars, resp,
			       import_prp_list, tabular_format, verbose,
			       use_var_labels, active_only);
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "Surrogate model retrieved " << import_prp_list.size()
	 << " total points." << std::endl;

  // For consistency with restart read, import_points (as well as post_input)
  // should also promote imported data to current eval cache/restart file.
  PRPLIter prp_it; String am_iface_id;
  bool cache = true, restart = true; // default on (with empty id) if no Model
  if (!actualModel.is_null()) {
    am_iface_id = actualModel.interface_id(); // default (Note: no recurse!)
    cache       = actualModel.evaluation_cache(); // recurse_flag = true
    restart     = actualModel.restart_file();     // recurse_flag = true
  }
  if (cache || restart) {
    // For negated sequence that continues from most negative id 
    //int cache_id = -1;
    //if (cache && !data_pairs.empty()) {
    //  int first_id = data_pairs.front().evaluation_id();
    //  if (first_id < 0) cache_id = first_id - 1;
    //}
    /// process arrays of data from TabularIO::read_data_tabular() above
    for (prp_it =import_prp_list.begin();
	 prp_it!=import_prp_list.end(); ++prp_it) {
      ParamResponsePair& pr = *prp_it;
      //if ( (tabular_format & TABULAR_EVAL_ID) == 0 )  // not imported
      pr.eval_id(0); // always override eval id to 0 for imported data
      if ( (tabular_format & TABULAR_IFACE_ID) == 0  && !am_iface_id.empty()) {// not imported: dangerous!
          pr.interface_id(am_iface_id); // assign best guess / default
      }

      if (restart) parallelLib.write_restart(pr); // preserve eval id
      if (cache)   data_pairs.insert(pr); // duplicate ids OK for PRPCache
      //if (cache) // for negated sequence
      //  { pr.evaluation_id(cache_id); data_pairs.insert(pr); --cache_id; }
    }
  }
}


/** Constructor helper to export approximation-based evaluations to a file. */
void DataFitSurrModel::initialize_export()
{
  if (!exportPointsFile.empty()) {
    TabularIO::open_file(exportFileStream, exportPointsFile,
			 "DataFitSurrModel export");
    TabularIO::write_header_tabular(exportFileStream, currentVariables,
				    currentResponse, "eval_id", "interface",
				    exportFormat);
  }
  if (!exportVarianceFile.empty()) {
    StringArray variance_labels;
    for (const auto& label : currentResponse.function_labels())
      variance_labels.push_back(label + "_variance");
    TabularIO::open_file(exportVarianceFileStream, exportVarianceFile,
			 "DataFitSurrModel variance export");
    TabularIO::write_header_tabular(exportVarianceFileStream, currentVariables,
				    variance_labels, "eval_id", "interface",
				    exportVarianceFormat);
  }
}


/** Constructor helper to export approximation-based evaluations to a file. */
void DataFitSurrModel::finalize_export()
{
  if (!exportPointsFile.empty())
    TabularIO::close_file(exportFileStream, exportPointsFile,
			  "DataFitSurrModel export");
  if (!exportVarianceFile.empty())
    TabularIO::close_file(exportVarianceFileStream, exportVarianceFile,
			  "DataFitSurrModel variance export");
}


/** Constructor helper to export approximation-based evaluations to a
    file. Exports all variables, so it's clear at what values of
    inactive it was built at */
void DataFitSurrModel::
export_point(int eval_id, const Variables& vars, const Response& resp)
{
  Response response_variance;
  if (!exportVarianceFile.empty()) {
    RealVector approx_var = approximation_variances(vars);
    response_variance = resp.copy();
    response_variance.function_values(approx_var);
  }

  if (recastings()) {
    Variables export_vars; Response export_resp;
    iterator_space_to_user_space(vars, resp, export_vars, export_resp);
    if (!exportPointsFile.empty())
      TabularIO::write_data_tabular(exportFileStream, export_vars, interface_id(),
				    export_resp, eval_id, exportFormat);
    if (!exportVarianceFile.empty()) {
      // BMA TODO WARN or SKIP: exported variance not transformed?!?
      // Can't in general map it through a recast model...
      TabularIO::write_data_tabular(exportVarianceFileStream, export_vars, interface_id(),
				    response_variance, eval_id, exportVarianceFormat);
    }

  }
  else {
    if (!exportPointsFile.empty())
      TabularIO::write_data_tabular(exportFileStream, vars, interface_id(), resp, 
				    eval_id, exportFormat);
    if (!exportVarianceFile.empty()) {
      TabularIO::write_data_tabular(exportVarianceFileStream, vars, interface_id(),
				    response_variance, eval_id, exportVarianceFormat);
    }
  }
}


void DataFitSurrModel::component_parallel_mode(short mode)
{
  // mode may be correct, but can't guarantee active parallel config is in sync
  //if (componentParallelMode == mode)
  //  return; // already in correct parallel mode

  /* Moved up a level so that config can be restored after optInterface usage
  //if (mode == TRUTH_MODEL_MODE) {
    // ParallelLibrary::currPCIter activation delegated to subModel
  //}
  //else 
  if (mode == SURROGATE_MODEL_MODE)
    parallelLib.parallel_configuration_iterator(modelPCIter);
  //else if (mode == 0)
  */

  componentParallelMode = mode;
}


void DataFitSurrModel::declare_sources()
{
  switch (responseMode) {
  case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
    if(actualModel.is_null() || surrogateFnIndices.size() == numFns) {
      evaluationsDB.declare_source(modelId, "surrogate", approxInterface.interface_id(),
        "approximation");
    } else if(surrogateFnIndices.empty()) { // don't know if this can happen.
      evaluationsDB.declare_source(modelId, "surrogate", actualModel.model_id(),
        actualModel.model_type());
    } else {
      evaluationsDB.declare_source(modelId, "surrogate", approxInterface.interface_id(),
        "approximation");
      evaluationsDB.declare_source(modelId, "surrogate", actualModel.model_id(),
        actualModel.model_type());
    }
    break;
  case BYPASS_SURROGATE:
    evaluationsDB.declare_source(modelId, "surrogate", actualModel.model_id(),
        actualModel.model_type());
    break;
  case MODEL_DISCREPANCY: case AGGREGATED_MODEL_PAIR:
    evaluationsDB.declare_source(modelId, "surrogate", actualModel.model_id(),
        actualModel.model_type());
    evaluationsDB.declare_source(modelId, "surrogate", approxInterface.interface_id(),
        "approximation");
    break;
  }
}


ActiveSet DataFitSurrModel::default_interface_active_set()
{
  // The ApproximationInterface may provide just a subset
  // of the responses, with the balance coming from the
  // actualModel.
  ActiveSet set;
  set.derivative_vector(currentVariables.all_continuous_variable_ids());
  bool has_deriv_vars = set.derivative_vector().size() != 0;
  ShortArray asv(numFns);
  const bool has_gradients = gradientType != "none" && has_deriv_vars &&
    (gradientType == "analytic" || supportsEstimDerivs);
  const bool has_hessians = hessianType != "none" &&  has_deriv_vars &&
    (hessianType == "analytic" || supportsEstimDerivs);
  // Most frequent case: build surrogates for all responses
  if (responseMode == MODEL_DISCREPANCY || responseMode == AGGREGATED_MODEL_PAIR
      || actualModel.is_null() || surrogateFnIndices.size() == numFns) {
    std::fill(asv.begin(), asv.end(), 1);
    if(has_gradients)
      for(auto &a : asv)
        a |=  2;
    if(has_hessians)
      for(auto &a : asv)
	a |=  4;
  } else {
    std::fill(asv.begin(), asv.end(), 0);
    for(size_t i = 0; i < numFns; ++i) {
      if(surrogateFnIndices.count(i)) {
        asv[i] = 1;
        if(has_gradients)
          asv[i] |= 2;
        if(has_hessians)
          asv[i] |= 4;
      }
    }
  }
  set.request_vector(asv);
  return set;
}

} // namespace Dakota
