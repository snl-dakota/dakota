/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       DataFitSurrModel
//- Description: Implementation code for the DataFitSurrModel class
//- Owner:       Mike Eldred
//- Checked by:

#include "DataFitSurrModel.hpp"
#include "RecastModel.hpp"
#include "ApproximationInterface.hpp"
#include "ParamResponsePair.hpp"
#include "ProblemDescDB.hpp"
#include "PRPMultiIndex.hpp"
#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/rolling_mean.hpp>


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
  autoRefine(problem_db.get_bool("model.surrogate.auto_refine")),
  maxIterations(problem_db.get_int("model.max_iterations")),
  maxFuncEvals(problem_db.get_int("model.max_function_evals")),
  convergenceTolerance(problem_db.get_real("model.convergence_tolerance")),
  softConvergenceLimit(problem_db.get_int("model.soft_convergence_limit")),
  refineCVMetric(problem_db.get_string("model.surrogate.refine_cv_metric")),
  refineCVFolds(problem_db.get_int("model.surrogate.refine_cv_folds"))
{
  // ignore bounds when finite differencing on data fits, since the bounds are
  // artificial in this case (and reflecting the stencil degrades accuracy)
  ignoreBounds = true;

  // if no user points management spec, assign RECOMMENDED_POINTS as default
  if (pointsManagement == DEFAULT_POINTS)
    pointsManagement = (pointsTotal > 0) ? TOTAL_POINTS : RECOMMENDED_POINTS;

  if (pointReuse.empty()) // assign default
    pointReuse = (importPointsFile.empty()) ? "none" : "all";

  // DataFitSurrModel is allowed to set the db list nodes, so long as it 
  // restores the list nodes to their previous setting.  This removes the need
  // to continuously reset at the strategy level (which would be wasteful
  // since the type of derived model may not be known at the strategy level).
  const String& dace_method_pointer
    = problem_db.get_string("model.dace_method_pointer");
  const String& actual_model_pointer
    = problem_db.get_string("model.surrogate.actual_model_pointer");
  if (!dace_method_pointer.empty()) { // global DACE approximations
    size_t method_index = problem_db.get_db_method_node(); // for restoration
    size_t model_index  = problem_db.get_db_model_node();  // for restoration
    problem_db.set_db_list_nodes(dace_method_pointer);

    // instantiate the DACE iterator, which instantiates the actual model
    daceIterator = problem_db.get_iterator();
    daceIterator.sub_iterator_flag(true);

    // retrieve the actual model from daceIterator (invalid for selected
    // meta-iterators, e.g., hybrids)
    actualModel = daceIterator.iterated_model();
    check_submodel_compatibility(actualModel);
    // if outer level output is verbose/debug and actualModel verbosity is
    // defined by the DACE method spec, request fine-grained evaluation
    // reporting for purposes of the final output summary.  This allows verbose
    // final summaries without verbose output on every dace-iterator completion.
    if (outputLevel > NORMAL_OUTPUT)
      actualModel.fine_grained_evaluation_counters();

    problem_db.set_db_method_node(method_index); // restore method only
    problem_db.set_db_model_nodes(model_index);  // restore all model nodes
  }
  else if (!actual_model_pointer.empty()) { // local/multipoint approximation
    size_t model_index = problem_db.get_db_model_node(); // for restoration
    problem_db.set_db_model_nodes(actual_model_pointer);
    actualModel = problem_db.get_model();
    check_submodel_compatibility(actualModel);
    problem_db.set_db_model_nodes(model_index); // restore
  }
  // else global approx. built solely from reuse_points: daceIterator/
  // actualModel remain empty envelopes.  Verify that there is a data source:
  // this basic check is augmented with a build_global() check which enforces
  // that the total points from both sources be >= minimum required.
  else if ( pointReuse == "none" ) {
    Cerr << "Error: to build an data fit surrogate model, either a global "
	 << "approximation\n       must be specified with reuse_points or "
	 << "dace_method_pointer, or a\n       local/multipoint approximation "
	 << "must be specified with an actual_model_pointer." << std::endl;
    abort_handler(-1);
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
    if ( actualModel.evaluation_cache(false) &&
	!actualModel.derivative_estimation())
      cache = true;
  }
  const StringArray fn_labels = (actualModel.is_null()) ? 
    currentResponse.function_labels() :  actualModel.response_labels();
  approxInterface.assign_rep(new ApproximationInterface(problem_db, vars,
    cache, am_interface_id, fn_labels), false);

  // initialize the DiscrepancyCorrection instance
  deltaCorr.initialize(*this, surrogateFnIndices, corrType,
    problem_db.get_short("model.surrogate.correction_order"));

  import_points(
    problem_db.get_ushort("model.surrogate.import_build_format"),
    problem_db.get_bool("model.surrogate.import_build_active_only"));
  initialize_export();
  if (!importPointsFile.empty() || !exportPointsFile.empty())
    manage_data_recastings();
}


DataFitSurrModel::
DataFitSurrModel(Iterator& dace_iterator, Model& actual_model,
		 //const SharedVariablesData& svd,const SharedResponseData& srd,
		 const ActiveSet& set, const String& approx_type,
		 const UShortArray& approx_order, short corr_type,
		 short corr_order, short data_order, short output_level,
		 const String& point_reuse,
		 const String& import_build_points_file,
		 unsigned short import_build_format,
		 bool import_build_active_only,
		 const String& export_approx_points_file,
		 unsigned short export_approx_format):
  SurrogateModel(actual_model.problem_description_db(),
		 actual_model.parallel_library(),
		 actual_model.current_variables().shared_data(),
		 actual_model.current_response().shared_data(),
		 set, corr_type, output_level),
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
    abort_handler(-1);
  }

  surrogateType = approx_type;

  if (pointReuse.empty()) // assign default
    pointReuse = (importPointsFile.empty()) ? "none" : "all";

  // update constraint counts in userDefinedConstraints.
  userDefinedConstraints.reshape(actualModel.num_nonlinear_ineq_constraints(),
				 actualModel.num_nonlinear_eq_constraints(),
				 actualModel.num_linear_ineq_constraints(),
				 actualModel.num_linear_eq_constraints());
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
  approxInterface.assign_rep(new ApproximationInterface(approx_type,
    approx_order, actualModel.current_variables(), cache,
    actualModel.interface_id(), numFns, data_order, outputLevel), false);

  if (!daceIterator.is_null()) // global DACE approximations
    daceIterator.sub_iterator_flag(true);
  //else { // local/multipoint approximation
  //}

  // initialize the DiscrepancyCorrection instance
  deltaCorr.initialize(*this, surrogateFnIndices, corr_type, corr_order);

  // to define derivative settings, we use incoming ASV to define requests
  // and surrogate type to determine analytic derivative support.
  const ShortArray& asv = set.request_vector();
  size_t i, num_fns = asv.size();
  bool grad_flag = false, hess_flag = false;
  for (i=0; i<num_fns; ++i) {
    if (asv[i] & 2) grad_flag = true;
    if (asv[i] & 4) hess_flag = true;
  }
  if (grad_flag)
    gradientType = (approx_type == "global_polynomial" ||
      approx_type == "global_gaussian" || approx_type == "global_kriging" ||
      strends(approx_type, "_orthogonal_polynomial") ||
      strends(approx_type, "_interpolation_polynomial") ||
      strbegins(approx_type, "local_") ||
      strbegins(approx_type, "multipoint_") ||
      approx_type == "global_moving_least_squares")
      ? "analytic" : "numerical";
  else 
    gradientType = "none";
  if (hess_flag)
    hessianType = (approx_type == "global_polynomial" ||
      approx_type == "global_kriging" ||
      strends(approx_type, "_orthogonal_polynomial") ||
    //strends(approx_type, "_interpolation_polynomial") || // TO DO
		   strbegins(approx_type, "local_"))
      ? "analytic" : "numerical";
  else
    hessianType = "none";

  //Cout << "DFS gradientType = " << gradientType 
  //     << " DFS hessianType = " << hessianType << std::endl;

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

  import_points(import_build_format, import_build_active_only);
  initialize_export();
  if (!importPointsFile.empty() || !exportPointsFile.empty())
    manage_data_recastings();
}


bool DataFitSurrModel::initialize_mapping(ParLevLIter pl_iter)
{
  SurrogateModel::initialize_mapping(pl_iter);
  actualModel.initialize_mapping(pl_iter);

  // push data that varies per iterator execution rather than per-evaluation
  // from currentVariables and userDefinedConstraints into actualModel
  init_model(actualModel);

  return false; // no change to problem size
}


/** Inactive variables must be propagated when a HierarchSurrModel
    is employed by a sub-iterator (e.g., OUU with MLMC or MLPCE).
    In current use cases, this can occur once per sub-iterator
    execution within Model::initialize_mapping(). */
bool DataFitSurrModel::finalize_mapping()
{
  actualModel.finalize_mapping();
  SurrogateModel::finalize_mapping();

  return false; // no change to problem size
}


/** This function constructs a new approximation, discarding any
    previous data.  It constructs any required data for
    SurrogateData::{vars,resp}Data and does not define an anchor point
    for SurrogateData::anchor{Vars,Resp}, so is an unconstrained build. */
void DataFitSurrModel::build_approximation()
{
  Cout << "\n>>>>> Building " << surrogateType << " approximations.\n";

  // clear out previous anchor/data points, but preserve history (if multipoint)
  approxInterface.clear_current();
  // update actualModel w/ variable values/bounds/labels
  update_model(actualModel);

  // build a local, multipoint, or global data fit approximation.
  if (strbegins(surrogateType, "local_") ||
      strbegins(surrogateType, "multipoint_")) {
    // NOTE: branch used by SBO
    update_local_multipoint();
    build_local_multipoint();
    interface_build_approx();
  }
  else { // global approximation.  NOTE: branch not used by SBO.
    update_global();
    build_global();
    //deltaCorr.compute(...need data...);
    // could add deltaCorr.compute() here and in HierarchSurrModel::
    // build_approximation if global approximations had easy access
    // to the truth/approx responses.  Instead, it is called from
    // SurrBasedLocalMinimizer using data from the trust region center.
    if (autoRefine)
      // BMA TODO: Move this to an external refiner
      refine_surrogate();
    else
      interface_build_approx();
  }

  ++approxBuilds;

  Cout << "\n<<<<< " << surrogateType << " approximation builds completed.\n";
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
    previous data.  It uses the passed data to populate
    SurrogateData::anchor{Vars,Resp} and constructs any required data
    points for SurrogateData::{vars,resp}Data. */
bool DataFitSurrModel::
build_approximation(const Variables& vars, const IntResponsePair& response_pr)
{
  Cout << "\n>>>>> Building " << surrogateType << " approximations.\n";

  // clear out previous anchor/data points, but preserve history (if multipoint)
  approxInterface.clear_current();
  // update actualModel w/ variable values/bounds/labels
  update_model(actualModel);
  // populate/replace the anchor point for the approximation.  When supported by
  // the surrogate type (local, multipoint, global polynomial regression), this
  // is enforced as a hard constraint. Otherwise, it is just another data point.
  approxInterface.update_approximation(vars, response_pr);
  // TO DO:
  // > not used by SBLM local/multipoint
  // > used by SBLM global *with* persistent center vars,response
  // > used by NonDLocal *without* persistent vars,response

  // build a local, multipoint, or global data fit approximation.
  if (strbegins(surrogateType, "local_") ||
      strbegins(surrogateType, "multipoint_")) {
    // NOTE: branch not used by SBO
    update_local_multipoint();
    // anchor is given, so no need for build_local_multipoint
    interface_build_approx();
  }
  else { // global approximation.  NOTE: branch used by SBO.
    update_global();
    build_global();
    //deltaCorr.compute(...need data...);
    // could add deltaCorr.compute() here and in HierarchSurrModel::
    // build_approximation if global approximations had easy access
    // to the truth/approx responses.  Instead, it is called from
    // SurrBasedLocalMinimizer using data from the trust region center.
    if (autoRefine)
      // BMA TODO: Move this to an external refiner
      refine_surrogate();
    else
      interface_build_approx();
  }

  ++approxBuilds;

  Cout << "\n<<<<< " << surrogateType << " approximation builds completed.\n";

  // return a bool indicating whether the incoming data defines an embedded
  // correction (hard constraint) or just another data point.  It would be
  // preferable to flow this up from the surrogate, but keep it simple for now.
  return (strbegins(surrogateType, "local_") ||
	  strbegins(surrogateType, "multipoint_") ||
	  surrogateType == "global_polynomial");
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

  if (rebuild_flag) { // update the coefficients for each approximation
    BoolDeque rebuild_deque(numFns, false);
    for (size_t i=0; i<numFns; ++i)
      for (IntRespMCIter r_it=all_resp.begin(); r_it!=all_resp.end(); ++r_it)
	if (r_it->second.active_set_request_vector()[i])
	  { rebuild_deque[i] = true; break; }
    // rebuild the designated surrogates
    approxInterface.rebuild_approximation(rebuild_deque);
    ++approxBuilds;
  }

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

  if (rebuild_flag) { // find the coefficients for each approximation
    // decide which surrogates to rebuild based on response content
    BoolDeque rebuild_deque(numFns);
    const ShortArray& asv = response_pr.second.active_set_request_vector();
    for (size_t i=0; i<numFns; ++i)
      rebuild_deque[i] = (asv[i]) ? true : false;
    // rebuild the designated surrogates
    approxInterface.rebuild_approximation(rebuild_deque);
    ++approxBuilds;
  }

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

  if (rebuild_flag) { // find the coefficients for each approximation
    // decide which surrogates to rebuild based on resp_map content
    BoolDeque rebuild_deque(numFns, false);
    for (size_t i=0; i<numFns; ++i)
      for (IntRespMCIter r_it=resp_map.begin(); r_it!=resp_map.end(); ++r_it)
	if (r_it->second.active_set_request_vector()[i])
	  { rebuild_deque[i] = true; break; }
    // rebuild the designated surrogates
    approxInterface.rebuild_approximation(rebuild_deque);
    ++approxBuilds;
  }

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

  if (rebuild_flag) { // find the coefficients for each approximation
    // decide which surrogates to rebuild based on resp_map content
    BoolDeque rebuild_deque(numFns, false);
    for (size_t i=0; i<numFns; ++i)
      for (IntRespMCIter r_it=resp_map.begin(); r_it!=resp_map.end(); ++r_it)
	if (r_it->second.active_set_request_vector()[i])
	  { rebuild_deque[i] = true; break; }
    // rebuild the designated surrogates
    approxInterface.rebuild_approximation(rebuild_deque);
    ++approxBuilds;
  }

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
void DataFitSurrModel::append_approximation(bool rebuild_flag)
{
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n>>>>> Appending to " << surrogateType << " approximations.\n";

  // append to the current points for each approximation
  //daceIterator.run(pl_iter);
  const IntResponseMap& all_resp = daceIterator.all_responses();
  if (daceIterator.compact_mode())
    approxInterface.append_approximation(daceIterator.all_samples(),  all_resp);
  else
    approxInterface.append_approximation(daceIterator.all_variables(),all_resp);

  if (rebuild_flag) { // update the coefficients for each approximation
    // decide which surrogates to rebuild based on resp_map content
    BoolDeque rebuild_deque(numFns, false);
    for (size_t i=0; i<numFns; ++i)
      for (IntRespMCIter r_it=all_resp.begin(); r_it!=all_resp.end(); ++r_it)
	if (r_it->second.active_set_request_vector()[i])
	  { rebuild_deque[i] = true; break; }
    // rebuild the designated surrogates
    approxInterface.rebuild_approximation(rebuild_deque);
    ++approxBuilds;
  }

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

  if (rebuild_flag) { // find the coefficients for each approximation
    // decide which surrogates to rebuild based on response content
    BoolDeque rebuild_deque(numFns);
    const ShortArray& asv = response_pr.second.active_set_request_vector();
    for (size_t i=0; i<numFns; ++i)
      rebuild_deque[i] = (asv[i]) ? true : false;
    // rebuild the designated surrogates
    approxInterface.rebuild_approximation(rebuild_deque);
    ++approxBuilds;
  }

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

  if (rebuild_flag) { // find the coefficients for each approximation
    // decide which surrogates to rebuild based on resp_map content
    BoolDeque rebuild_deque(numFns, false);
    for (size_t i=0; i<numFns; ++i)
      for (IntRespMCIter r_it=resp_map.begin(); r_it!=resp_map.end(); ++r_it)
	if (r_it->second.active_set_request_vector()[i])
	  { rebuild_deque[i] = true; break; }
    // rebuild the designated surrogates
    approxInterface.rebuild_approximation(rebuild_deque);
    ++approxBuilds;
  }

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

  if (rebuild_flag) { // find the coefficients for each approximation
    // decide which surrogates to rebuild based on resp_map content
    BoolDeque rebuild_deque(numFns, false);
    for (size_t i=0; i<numFns; ++i)
      for (IntRespMCIter r_it=resp_map.begin(); r_it!=resp_map.end(); ++r_it)
	if (r_it->second.active_set_request_vector()[i])
	  { rebuild_deque[i] = true; break; }
    // rebuild the designated surrogates
    approxInterface.rebuild_approximation(rebuild_deque);
    ++approxBuilds;
  }

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n<<<<< " << surrogateType
	 << " approximation updates completed.\n";
}


void DataFitSurrModel::pop_approximation(bool save_surr_data, bool rebuild_flag)
{
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n>>>>> Popping data from " << surrogateType
	 << " approximations.\n";

  // append to the current points for each approximation
  approxInterface.pop_approximation(save_surr_data);

  if (rebuild_flag) { // update the coefficients for each approximation
    BoolDeque rebuild_deque; // empty array: default rebuild of all fns
    approxInterface.rebuild_approximation(rebuild_deque);
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

  // append to the current points for each approximation
  approxInterface.push_approximation();

  /*
  if (rebuild_flag) { // update the coefficients for each approximation
    BoolDeque rebuild_deque; // empty array: default rebuild of all fns
    approxInterface.rebuild_approximation(rebuild_deque);
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

  // append to the current points for each approximation
  approxInterface.finalize_approximation();

  /*
  if (rebuild_flag) { // update the coefficients for each approximation
    BoolDeque rebuild_deque; // empty array: default rebuild of all fns
    approxInterface.rebuild_approximation(rebuild_deque);
    ++approxBuilds;
  }
  */

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n<<<<< " << surrogateType << " approximation finalized.\n";
}


void DataFitSurrModel::store_approximation(size_t index)
{
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n>>>>> Storing " << surrogateType << " approximations.\n";

  // store the current data for each approximation for later combination
  approxInterface.store_approximation(index);

  //Cout << "\n<<<<< " << surrogateType << " approximation stored.\n";
}


void DataFitSurrModel::restore_approximation(size_t index)
{
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n>>>>> Restoring " << surrogateType << " approximations.\n";

  // store the current data for each approximation for later combination
  approxInterface.restore_approximation(index);

  //Cout << "\n<<<<< " << surrogateType << " approximation restored.\n";
}


void DataFitSurrModel::remove_stored_approximation(size_t index)
{
  // If activated, must protect _NPOS:
  //Cout << "\n>>>>> Removing stored approximation " << index+1 << ".\n";

  // store the current data for each approximation for later combination
  approxInterface.remove_stored_approximation(index);

  //Cout << "\n<<<<< Stored approximation removed.\n";
}


void DataFitSurrModel::combine_approximation(short corr_type)
{
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n>>>>> Combining " << surrogateType << " approximations.\n";

  // Manage swap detection here or within Pecos::sharedPolyApproxData?
  // Note: access to spec sequences are on Dakota side, but need to reach
  // into Pecos driver levels (TPQ, SSG) and approx orders (regression) for
  // access to final refinement levels (GSG starting levels could be the same).
  //NonDIntegration* nond_int = (NonDIntegration*)daceIterator.iterator_rep();
  //bool swap = !nond_int->maximal_grid();

  approxInterface.combine_approximation(corr_type);

  //Cout << "\n<<<<< " << surrogateType << " approximation combined.\n";
}


void DataFitSurrModel::update_local_multipoint()
{
  // Store the actualModel inactive variable values for use in force_rebuild()
  // for determining whether an automatic approximation rebuild is required.

  // the actualModel data has been updated by update_model(), which precedes
  // update_local_multipoint()

  const Variables& actual_vars = actualModel.current_variables();
  if (actual_vars.view().first >= RELAXED_DESIGN) { // Distinct view
    copy_data(actual_vars.inactive_continuous_variables(),    referenceICVars);
    copy_data(actual_vars.inactive_discrete_int_variables(),  referenceIDIVars);
    copy_data(actual_vars.inactive_discrete_real_variables(), referenceIDRVars);
  }
}


void DataFitSurrModel::interface_build_approx()
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
    approxInterface.build_approximation(actualModel.continuous_lower_bounds(),
      actualModel.continuous_upper_bounds(),
      actualModel.discrete_int_lower_bounds(),
      actualModel.discrete_int_upper_bounds(),
      actualModel.discrete_real_lower_bounds(),
      actualModel.discrete_real_upper_bounds());
  }
  if (exportSurrogate)
    approxInterface.export_approximation();
}


void DataFitSurrModel::update_global()
{
  // Store the actualModel active variable bounds and inactive variable values
  // for use in force_rebuild() to determine whether an automatic approximation
  // rebuild is required.

  // the actualModel data has been updated by update_model(), which precedes
  // update_global().

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


/** Evaluate the value, gradient, and possibly Hessian needed for a
    local or multipoint approximation using actualModel. */
void DataFitSurrModel::build_local_multipoint()
{
  // set DataFitSurrModel parallelism mode to actualModel
  component_parallel_mode(TRUTH_MODEL);

  // Define the data requests
  short asv_value = 3;
  if (strbegins(surrogateType, "local_") &&
      actualModel.hessian_type() != "none")
    asv_value += 4;
  ShortArray orig_asv(numFns, asv_value), actual_asv, approx_asv;
  asv_mapping(orig_asv, actual_asv, approx_asv, true);

  // Evaluate value and derivatives using actualModel
  ActiveSet set = actualModel.current_response().active_set(); // copy
  set.request_vector(actual_asv);
  set.derivative_vector(actualModel.continuous_variable_ids());
  actualModel.evaluate(set);

  const Variables& curr_vars = actualModel.current_variables();
  IntResponsePair curr_resp_pr(actualModel.evaluation_id(),
			       actualModel.current_response());
  approxInterface.update_approximation(curr_vars, curr_resp_pr);
}


/** Determine points to use in building the approximation and
    then evaluate them on actualModel using daceIterator.  Any changes
    to the bounds should be performed by setting them at a higher
    level (e.g., SurrBasedOptStrategy). */
void DataFitSurrModel::build_global()
{
  // build_global() follows update_model() so we may use
  // actualModel.continuous_(lower/upper)_bounds() to avoid view
  // conversions and allow pass-by-reference.

  // **************************************************************************
  // Check data_pairs and importPointsFile for any existing evaluations to reuse
  // **************************************************************************
  size_t i, j, reuse_points = 0;
  if (pointReuse == "all" || pointReuse == "region") {

    size_t num_c_vars, num_di_vars, num_dr_vars;
    if (actualModel.is_null()) {
      num_c_vars  = currentVariables.cv();
      num_di_vars = currentVariables.div();
      num_dr_vars = currentVariables.drv();
    }
    else {
      num_c_vars  = actualModel.cv();
      num_di_vars = actualModel.div();
      num_dr_vars = actualModel.drv();
    }

    // since SurrBasedLocalMinimizer currently evaluates the trust region center
    // first, we must take care to not include this point in the point reuse,
    // since this would cause it to be used twice.
    int index = *surrogateFnIndices.begin();
    const Pecos::SurrogateDataVars& anchor_vars
      = approxInterface.approximation_data(index).anchor_variables();

    // Process PRPCache using default iterators (index 0 = ordered_non_unique).
    // This includes evals from current run, evals imported from restart, and 
    // evals imported from a tabular file (DataFitSurrModel::import_points()).
    // Each of these data_pairs sources is in "user-space" (e.g., generated by
    // an ApplicationInterface).  To compare with DataFitSurrModel values and
    // bounds, any recastings within the model recursion must be managed.
    String am_interface_id;
    if (!actualModel.is_null()) am_interface_id = actualModel.interface_id();
    ModelLRevIter ml_rit; PRPCacheCIter prp_iter;
    Variables db_vars; Response db_resp;
    bool map_to_iter_space = recastings();
    for (prp_iter=data_pairs.begin(); prp_iter!=data_pairs.end(); ++prp_iter) {
      // apply any recastings below this level: we perform these recastings at
      // run time (instead of once in import_points()) to support any updates
      // to the transformations (e.g., distribution parameter updates).
      if (map_to_iter_space)
	user_space_to_iterator_space(prp_iter->variables(),
				     prp_iter->response(), db_vars, db_resp);
      else
	{ db_vars = prp_iter->variables(); db_resp = prp_iter->response(); }

      // Note: for NonD uses with u-space models, the global_bounds boolean
      // in NonD::transform_model() needs to be set in order to allow test
      // of transformed bounds in "region" reuse case.  For "all" reuse case
      // typically used with data import, this is not necessary.
      if ( prp_iter->interface_id() == am_interface_id &&
	   inside(db_vars.continuous_variables(),
		  db_vars.discrete_int_variables(),
		  db_vars.discrete_real_variables()) &&
	   !vars_exact_compare(db_vars, anchor_vars) ) { // avoid anchor duplic
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
	  if (map_to_iter_space) Cout << "Transformed ";
	  else                   Cout << "Untransformed ";
	  Cout << "data for DB eval " << prp_iter->eval_id() << ":\n"
	       << db_vars << db_resp;
	}
      }
    }
  }

  // *******************************************
  // Evaluate new data points using daceIterator
  // *******************************************
  // minimum points required by the surrogate model
  int min_points = approxInterface.minimum_points(true);// incl constraints

  int new_points = 0;
  if (daceIterator.is_null()) { // reused/imported data only (no new data)
    if (reuse_points < min_points) { // check for sufficient data
      Cerr << "Error: a minimum of " << min_points << " points is required by "
	   << "DataFitSurrModel::build_global.\n" << reuse_points
	   << " were provided." << std::endl;
      abort_handler(-1);
    }
  }
  else { // else use rst info only (no new data)

    // set DataFitSurrModel parallelism mode to actualModel
    component_parallel_mode(TRUTH_MODEL);

    // determine number of points associated with the model specification
    // (min, recommended, or total)
    int model_points;                                
    switch (pointsManagement) {
    case DEFAULT_POINTS: case MINIMUM_POINTS:
      model_points = min_points;                               break;
    case RECOMMENDED_POINTS:
      model_points = approxInterface.recommended_points(true); break;
    case TOTAL_POINTS:
      if (pointsTotal < min_points && outputLevel >= NORMAL_OUTPUT)
	Cout << "\nDataFitSurrModel: Total points specified " << pointsTotal
	     << " is less than minimum required;\n                  "
	     << "increasing to " << min_points << std::endl;
      model_points = std::max(min_points, pointsTotal);        break;
    }

    // daceIterator must generate at least diff_points samples, should
    // populate allData lists (allDataFlag = true), and should bypass
    // statistics computation (statsFlag = false).
    int diff_points = std::max(0, model_points - (int)reuse_points);
    daceIterator.sampling_reset(diff_points, true, false);// update s.t. lwr bnd
    // The DACE iterator's samples{Spec,Ref} value provides a lower bound on
    // the number of samples generated: new_points = max(diff_points,reference).
    new_points = daceIterator.num_samples();

    // only run the iterator if work to do
    if (new_points)
      run_dace_iterator(false); // don't rebuild
    else if (outputLevel >= DEBUG_OUTPUT)
      Cout << "DataFitSurrModel: No samples needed from DACE iterator."
	   << std::endl;

  }

  // *******************************
  // Output counts for data ensemble
  // *******************************
  int index = *surrogateFnIndices.begin();
  String anchor = (approxInterface.approximation_data(index).anchor())
    ? "one" : "no";
  Cout << "Constructing global approximations with " << anchor << " anchor, "
       << new_points << " DACE samples, and " << reuse_points
       << " reused points.\n";
}


void DataFitSurrModel::run_dace_iterator(bool rebuild_flag)
{
  // Define the data requests
  ActiveSet set = daceIterator.active_set(); // copy
  ShortArray actual_asv, approx_asv;
  asv_mapping(set.request_vector(), actual_asv, approx_asv, true);
  set.request_vector(actual_asv);
  daceIterator.active_set(set);
  // prepend hierarchical tag before running
  if (hierarchicalTagging) {
    String eval_tag = evalTagPrefix + '.' + 
      boost::lexical_cast<String>(surrModelEvalCntr+1);
    daceIterator.eval_tag_prefix(eval_tag);
  }
  // run the iterator
  ParLevLIter pl_iter = modelPCIter->mi_parallel_level_iterator(miPLIndex);
  daceIterator.run(pl_iter);
  // append the new data sets and rebuild if indicated
  append_approximation(rebuild_flag);
}


void DataFitSurrModel::refine_surrogate()
{
  StringArray diag_metrics(1, refineCVMetric);
  int curr_iter = 0; // initial surrogate build is iteration 0
    
  // accumulate num_samples in each iteration in total_evals.
  int total_evals = 0, num_samples;

  // Disable the soft convergence limit by setting it to maxIterations if
  // the user didn't set it
  int soft_conv_limit = maxIterations;
  if(softConvergenceLimit != 0)
    soft_conv_limit = softConvergenceLimit;

  // accumulator for rolling average of length soft_conv_limit 
  using namespace boost::accumulators;
  accumulator_set<Real, stats<tag::rolling_mean> > mean_err(
      tag::rolling_window::window_size = soft_conv_limit);

  num_samples = daceIterator.num_samples();
  total_evals += num_samples;

  // build surrogate from initial sample
  interface_build_approx();
  Real2DArray cv_diags = 
    approxInterface.cv_diagnostics(diag_metrics, refineCVFolds);
  RealArray cv_per_fn(currentResponse.num_functions());
  for (size_t i=0; i<currentResponse.num_functions(); ++i)
    cv_per_fn[i] = cv_diags[i][0];
  Real curr_err = *std::max_element(cv_per_fn.begin(), cv_per_fn.end());
  // keep prev_err to calculate improvement
  Real prev_err = curr_err;
  Cout << "\n------------\nAuto-refinement initial error (" << refineCVMetric 
    << "): " << curr_err << std::endl;
  while (true) {
    // convergence conditions. messages will need to be fixed if we add
    // challenge error
    if(curr_err <= convergenceTolerance) {
      Cout << "\n------------\nAuto-refined surrogate(s) converged: "
        << "Cross-validation error criterion met.\n";
      break;
    } else if(curr_iter++ == maxIterations) {
      Cout << "\n------------\nAuto-refinment halted: Maximum iterations "
        << "met or exceeded.\n";
      break;
    } else if( curr_iter >= soft_conv_limit && 
        rolling_mean(mean_err) < convergenceTolerance) {
      Cout << "\n------------\nAuto-refinment halted: Average reduction in "
        << "cross-validation error over\nprevious " << soft_conv_limit 
        << " iterations less than " << "convergence_tolerance (" 
        << convergenceTolerance << ")\n";
      break;
    } else if(total_evals >= maxFuncEvals) {
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
    run_dace_iterator(false); // don't rebuild

    // build and check diagnostics
    interface_build_approx();
    Real2DArray cv_diags = 
      approxInterface.cv_diagnostics(diag_metrics, refineCVFolds);
    RealArray cv_per_fn(currentResponse.num_functions());
    for (size_t i=0; i<currentResponse.num_functions(); ++i)
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


bool DataFitSurrModel::
inside(const RealVector& c_vars, const IntVector& di_vars,
       const RealVector& dr_vars) const
{
  if (pointReuse == "region") { // inside always = TRUE for "all"

    size_t i, num_c_vars = c_vars.length(), num_di_vars = di_vars.length(),
      num_dr_vars = dr_vars.length();

    const RealVector& c_l_bnds = (actualModel.is_null()) ?
      userDefinedConstraints.continuous_lower_bounds() :
      actualModel.continuous_lower_bounds();
    const RealVector& c_u_bnds = (actualModel.is_null()) ?
      userDefinedConstraints.continuous_upper_bounds() :
      actualModel.continuous_upper_bounds();
    const IntVector&  di_l_bnds = (actualModel.is_null()) ?
      userDefinedConstraints.discrete_int_lower_bounds() :
      actualModel.discrete_int_lower_bounds();
    const IntVector&  di_u_bnds = (actualModel.is_null()) ?
      userDefinedConstraints.discrete_int_upper_bounds() :
      actualModel.discrete_int_upper_bounds();
    const RealVector& dr_l_bnds = (actualModel.is_null()) ?
      userDefinedConstraints.discrete_real_lower_bounds() :
      actualModel.discrete_real_lower_bounds();
    const RealVector& dr_u_bnds = (actualModel.is_null()) ?
      userDefinedConstraints.discrete_real_upper_bounds() :
      actualModel.discrete_real_upper_bounds();
    
    if (c_l_bnds.length() != num_c_vars  || c_u_bnds.length()  != num_c_vars  ||
	di_l_bnds.length()!= num_di_vars || di_u_bnds.length() != num_di_vars ||
	dr_l_bnds.length()!= num_dr_vars || dr_u_bnds.length() != num_dr_vars) {
      Cerr << "Warning: inconsistent variable counts in DataFitSurrModel::"
	   << "inside().  Excluding candidate data point.\n";
      return false;
    }

    for (i=0; i<num_c_vars; ++i)
      if (c_vars[i] < c_l_bnds[i] || c_vars[i] > c_u_bnds[i])
	return false;
    for (i=0; i<num_di_vars; ++i)
      if (di_vars[i] < di_l_bnds[i] || di_vars[i] > di_u_bnds[i])
	return false;
    for (i=0; i<num_dr_vars; ++i)
      if (dr_vars[i] < dr_l_bnds[i] || dr_vars[i] > dr_u_bnds[i])
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

  ShortArray actual_asv, approx_asv; bool actual_eval, approx_eval, mixed_eval;
  Response actual_response, approx_response; // empty handles
  switch (responseMode) {
  case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
    asv_mapping(set.request_vector(), actual_asv, approx_asv, false);
    actual_eval = !actual_asv.empty(); approx_eval = !approx_asv.empty();
    mixed_eval = (actual_eval && approx_eval); break;
  case BYPASS_SURROGATE:
    actual_eval = true; approx_eval = false;   break;
  case MODEL_DISCREPANCY:
    actual_eval = approx_eval = true;          break;
  }

  if (hierarchicalTagging) {
    String eval_tag = evalTagPrefix + '.' + 
      boost::lexical_cast<String>(surrModelEvalCntr+1);
    if (actual_eval)
      actualModel.eval_tag_prefix(eval_tag);
  }

  // -----------------------------
  // Compute actual model response
  // -----------------------------
  if (actual_eval) {
    component_parallel_mode(TRUTH_MODEL);
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
	currentResponse.update(actualModel.current_response());
      }
      break;
    }
    case BYPASS_SURROGATE:
      actualModel.evaluate(set);
      currentResponse.active_set(set);
      currentResponse.update(actualModel.current_response());
      // TODO: Add to surrogate build data
      //      add_datapoint(....)
      break;
    case MODEL_DISCREPANCY:
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
    //component_parallel_mode(SURROGATE_MODEL); // does not use parallelism
    //ParConfigLIter pc_iter = parallelLib.parallel_configuration_iterator();
    //parallelLib.parallel_configuration_iterator(modelPCIter);
    switch (responseMode) {
    case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE: {
      ActiveSet approx_set = set;
      approx_set.request_vector(approx_asv);
      approx_response = (mixed_eval) ? currentResponse.copy() : currentResponse;
      approxInterface.map(currentVariables, approx_set, approx_response); break;
    }
    case MODEL_DISCREPANCY:
      approx_response = currentResponse.copy(); // TO DO
      approxInterface.map(currentVariables, set, approx_response);        break;
    }
    //parallelLib.parallel_configuration_iterator(pc_iter); // restore

    // export data (optional)
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
  case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
    if (mixed_eval) {
      currentResponse.active_set(set);
      response_mapping(actual_response, approx_response, currentResponse);
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

  ShortArray actual_asv, approx_asv; bool actual_eval, approx_eval;
  switch (responseMode) {
  case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
    asv_mapping(set.request_vector(), actual_asv, approx_asv, false);
    actual_eval = !actual_asv.empty(); approx_eval = !approx_asv.empty(); break;
  case BYPASS_SURROGATE:
    actual_eval = true; approx_eval = false;                              break;
  case MODEL_DISCREPANCY:
    actual_eval = approx_eval = true;                                     break;
  }

  if (hierarchicalTagging) {
    String eval_tag = evalTagPrefix + '.' + 
      boost::lexical_cast<String>(surrModelEvalCntr+1);
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
    case BYPASS_SURROGATE: case MODEL_DISCREPANCY:
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

    // compute the approximate response
    // don't need to set component parallel mode since this only queues the job
    switch (responseMode) {
    case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE: {
      ActiveSet approx_set = set;
      approx_set.request_vector(approx_asv);
      approxInterface.map(currentVariables, approx_set, currentResponse, true);
      break;
    }
    case MODEL_DISCREPANCY:
      approxInterface.map(currentVariables,        set, currentResponse, true);
      break;
    }

    // post-process
    switch (responseMode) {
    case AUTO_CORRECTED_SURROGATE:
      rawVarsMap[surrModelEvalCntr] = currentVariables.copy(); break;
    default:
      if (!exportPointsFile.empty())
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
    component_parallel_mode(TRUTH_MODEL);

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
  case AGGREGATED_MODELS:
    for (; act_it != actual_resp_map_rekey.end() && 
	   app_it != approx_resp_map_rekey.end(); ++act_it, ++app_it) {
      check_key(act_it->first, app_it->first);
      aggregate_response(act_it->second, app_it->second,
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
	{ response_mapping(act_it->second, empty_resp,
			   surrResponseMap[act_eval_id]); ++act_it; }
      else if (app_eval_id < act_eval_id) // only LF available
	{ response_mapping(empty_resp, app_it->second,
			   surrResponseMap[app_eval_id]); ++app_it; }
      else // both LF and HF available
	{ response_mapping(act_it->second, app_it->second,
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
    component_parallel_mode(TRUTH_MODEL);

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
      case MODEL_DISCREPANCY: case AGGREGATED_MODELS:
	Cerr << "Error: approx eval missing in DataFitSurrModel::"
	     << "derived_synchronize_nowait()" << std::endl;
	abort_handler(-1); break;
      default: // {UN,AUTO_}CORRECTED_SURROGATE modes
	// there is no approx component to this response
	response_mapping(act_it->second, empty_resp,
			 surrResponseMap[act_eval_id]);
	break;
      }
      ++act_it;
    }
    else if (app_eval_id < act_eval_id) { // only approx available
      switch (responseMode) {
      case MODEL_DISCREPANCY: case AGGREGATED_MODELS:
	// cache approx response since actual contribution not yet available
	cachedApproxRespMap[app_eval_id] = app_it->second; break;
      default: // {UN,AUTO_}CORRECTED_SURROGATE modes
	if (remain_truth_ids.find(app_eval_id) != remain_truth_ids.end())
	  // cache approx response since actual contribution still pending
	  cachedApproxRespMap[app_eval_id] = app_it->second;
	else // response complete: there is no actual contribution
	  response_mapping(empty_resp, app_it->second, 
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
      case AGGREGATED_MODELS:
	aggregate_response(act_it->second, app_it->second,
			   surrResponseMap[act_eval_id]);            break;
      default: // {UN,AUTO_}CORRECTED_SURROGATE modes
	response_mapping(act_it->second, app_it->second,
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

  //component_parallel_mode(SURROGATE_MODEL); // does not use parallelism
  //ParConfigLIter pc_iter = parallelLib.parallel_configuration_iterator();
  //parallelLib.parallel_configuration_iterator(modelPCIter);

  // synchronize and rekey to use surrModelEvalCntr
  rekey_synch(approxInterface, block, surrIdMap, approx_resp_map_rekey);

  //parallelLib.parallel_configuration_iterator(pc_iter); // restore

  IntRespMIter r_it;
  if (responseMode == AUTO_CORRECTED_SURROGATE && corrType) {
    // Interface::rawResponseMap can be corrected directly in the case of an
    // ApproximationInterface since data_pairs is not used (not true for
    // HierarchSurrModel::derived_synchronize()/derived_synchronize_nowait()).
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
      export_point(r_it->first, v_it->second, r_it->second);
    }
    rawVarsMap.clear();
  }
  else if (!exportPointsFile.empty()) {
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


/** Constructor helper to read the points file once, if provided, and
    then reuse its data as appropriate within build_global().
    Surrogate data imports default to active/inactive variables, but
    user can override to active only */
void DataFitSurrModel::
import_points(unsigned short tabular_format, bool active_only)
{
  if (importPointsFile.empty())
    return;

  // Temporary objects to use to read correct size vars/resp
  const Variables& vars = actualModel.is_null() ? currentVariables : 
    actualModel.current_variables(); 
  const Response& resp  = actualModel.is_null() ? currentResponse : 
    actualModel.current_response();
  size_t num_vars = active_only ? 
    (vars.cv() + vars.div() + vars.dsv() + vars.drv()) : vars.tv();

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "Surrogate model retrieving points with " << num_vars
	 << " variables and " << numFns << " response functions from file "
	 << importPointsFile << '\n';
  // Preserves eval and interface ids, if annotated format
  PRPList import_prp_list;
  bool verbose = (outputLevel > NORMAL_OUTPUT);
  TabularIO::read_data_tabular(importPointsFile, 
			       "DataFitSurrModel samples file", vars, resp,
			       import_prp_list, tabular_format, verbose,
			       active_only);
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
      if ( (tabular_format & TABULAR_IFACE_ID) == 0 )// not imported: dangerous!
	pr.interface_id(am_iface_id); // assign best guess / default

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
				    currentResponse, "eval_id", exportFormat);
  }
}


/** Constructor helper to export approximation-based evaluations to a file. */
void DataFitSurrModel::finalize_export()
{
  if (!exportPointsFile.empty())
    TabularIO::close_file(exportFileStream, exportPointsFile,
			  "DataFitSurrModel export");
}


/** Constructor helper to export approximation-based evaluations to a
    file. Exports all variables, so it's clear at what values of
    inactive it was built at */
void DataFitSurrModel::
export_point(int eval_id, const Variables& vars, const Response& resp)
{
  if (exportPointsFile.empty())
    return;

  if (recastings()) {
    Variables export_vars; Response export_resp;
    iterator_space_to_user_space(vars, resp, export_vars, export_resp);
    TabularIO::write_data_tabular(exportFileStream, export_vars, interface_id(),
				  export_resp, eval_id, exportFormat);
  }
  else
    TabularIO::write_data_tabular(exportFileStream, vars, interface_id(), resp, 
				  eval_id, exportFormat);
}


void DataFitSurrModel::component_parallel_mode(short mode)
{
  // mode may be correct, but can't guarantee active parallel config is in sync
  //if (componentParallelMode == mode)
  //  return; // already in correct parallel mode

  /* Moved up a level so that config can be restored after optInterface usage
  //if (mode == TRUTH_MODEL) {
    // ParallelLibrary::currPCIter activation delegated to subModel
  //}
  //else 
  if (mode == SURROGATE_MODEL)
    parallelLib.parallel_configuration_iterator(modelPCIter);
  //else if (mode == 0)
  */

  componentParallelMode = mode;
}


/** Update variables and constraints data within model using
    values and labels from currentVariables and bound/linear/nonlinear
    constraints from userDefinedConstraints. */
void DataFitSurrModel::init_model(Model& model)
{
  if (model.is_null())
    return;

  // linear constraints

  if (userDefinedConstraints.num_linear_ineq_constraints()) {
    // the views don't necessarily have to be the same, but the number of
    // active continuous and active discrete variables have to be consistent.
    if (currentVariables.cv()  == model.cv()  &&
	currentVariables.div() == model.div() &&
	currentVariables.drv() == model.drv()) {
      model.linear_ineq_constraint_coeffs(
        userDefinedConstraints.linear_ineq_constraint_coeffs());
      model.linear_ineq_constraint_lower_bounds(
        userDefinedConstraints.linear_ineq_constraint_lower_bounds());
      model.linear_ineq_constraint_upper_bounds(
        userDefinedConstraints.linear_ineq_constraint_upper_bounds());
    }
    else {
      Cerr << "Error: cannot update linear inequality constraints in "
	   << "DataFitSurrModel::init_model() due to inconsistent active "
	   << "variables." << std::endl;
      abort_handler(-1);
    }
  }
  if (userDefinedConstraints.num_linear_eq_constraints()) {
    // the views don't necessarily have to be the same, but the number of
    // active continuous and active discrete variables have to be consistent.
    if (currentVariables.cv()  == model.cv()  &&
	currentVariables.div() == model.div() &&
	currentVariables.drv() == model.drv()) {
      model.linear_eq_constraint_coeffs(
        userDefinedConstraints.linear_eq_constraint_coeffs());
      model.linear_eq_constraint_targets(
        userDefinedConstraints.linear_eq_constraint_targets());
    }
    else {
      Cerr << "Error: cannot update linear equality constraints in "
	   << "DataFitSurrModel::init_model() due to inconsistent active "
	   << "variables." << std::endl;
      abort_handler(-1);
    }
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

  // discrete set values

  if (!discreteDesignSetIntValues.empty())
    model.discrete_design_set_int_values(discreteDesignSetIntValues);
  if (!discreteDesignSetRealValues.empty())
    model.discrete_design_set_real_values(discreteDesignSetRealValues);
  if (!discreteStateSetIntValues.empty())
    model.discrete_state_set_int_values(discreteStateSetIntValues);
  if (!discreteStateSetRealValues.empty())
    model.discrete_state_set_real_values(discreteStateSetRealValues);

  // uncertain variable distribution data

  // Note: Variables instances defined from the same variablesId are not shared
  // (see ProblemDescDB::get_variables()), so we propagate any distribution
  // updates (e.g., NestedModel insertions) up/down the Model recursion.  For
  // differing variablesId, we cannot assume that the distribution information
  // can be mapped, since the distributions used to build may differ from those
  // used to evaluate.   More careful logic may be needed in the future...
  if (currentVariables.shared_data().id() ==
      model.current_variables().shared_data().id()) {
    model.aleatory_distribution_parameters().update(aleatDistParams);
    model.epistemic_distribution_parameters().update(epistDistParams);
  }

  // labels: update model with current{Variables,Response} descriptors

  if (!approxBuilds) {
    model.response_labels(currentResponse.function_labels());

    short approx_active_view = currentVariables.view().first,
          actual_active_view = model.current_variables().view().first;
    if (approx_active_view == actual_active_view) {
      // update active model vars with active currentVariables data
      model.continuous_variable_labels(
        currentVariables.continuous_variable_labels());
      model.discrete_int_variable_labels(
        currentVariables.discrete_int_variable_labels());
      model.discrete_real_variable_labels(
        currentVariables.discrete_real_variable_labels());
      if (approx_active_view >= RELAXED_DESIGN) {
	model.inactive_continuous_variable_labels(
          currentVariables.inactive_continuous_variable_labels());
	model.inactive_discrete_int_variable_labels(
          currentVariables.inactive_discrete_int_variable_labels());
	model.inactive_discrete_real_variable_labels(
          currentVariables.inactive_discrete_real_variable_labels());
      }
    }
    else if ( approx_active_view >= RELAXED_DESIGN &&
	      ( actual_active_view == RELAXED_ALL ||
		actual_active_view == MIXED_ALL ) ) {
      // update active model vars using "All" view of currentVariables data
      model.continuous_variable_labels(
        currentVariables.all_continuous_variable_labels());
      model.discrete_int_variable_labels(
        currentVariables.all_discrete_int_variable_labels());
      model.discrete_real_variable_labels(
        currentVariables.all_discrete_real_variable_labels());
    }
    else if ( actual_active_view >= RELAXED_DESIGN &&
	      ( approx_active_view == RELAXED_ALL ||
		approx_active_view == MIXED_ALL ) ) {
      // update "All" view of model vars using active currentVariables data
      model.all_continuous_variable_labels(
        currentVariables.continuous_variable_labels());
      model.all_discrete_int_variable_labels(
        currentVariables.discrete_int_variable_labels());
      model.all_discrete_real_variable_labels(
        currentVariables.discrete_real_variable_labels());
    }
  }
}


/** Update variables and constraints data within model using
    values and labels from currentVariables and bound/linear/nonlinear
    constraints from userDefinedConstraints. */
void DataFitSurrModel::update_model(Model& model)
{
  if (model.is_null())
    return;

  // vars/bounds/labels

  short approx_active_view = currentVariables.view().first,
        actual_active_view = model.current_variables().view().first;
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
  if (approx_active_view == actual_active_view) {
    // update active model vars/cons with active currentVariables data
    model.continuous_variables(currentVariables.continuous_variables());
    model.discrete_int_variables(
      currentVariables.discrete_int_variables());
    model.discrete_real_variables(
      currentVariables.discrete_real_variables());
    model.continuous_lower_bounds(
      userDefinedConstraints.continuous_lower_bounds());
    model.continuous_upper_bounds(
      userDefinedConstraints.continuous_upper_bounds());
    model.discrete_int_lower_bounds(
      userDefinedConstraints.discrete_int_lower_bounds());
    model.discrete_int_upper_bounds(
      userDefinedConstraints.discrete_int_upper_bounds());
    model.discrete_real_lower_bounds(
      userDefinedConstraints.discrete_real_lower_bounds());
    model.discrete_real_upper_bounds(
      userDefinedConstraints.discrete_real_upper_bounds());
  }
  else if ( approx_active_view >= RELAXED_DESIGN &&
	    ( actual_active_view == RELAXED_ALL ||
	      actual_active_view == MIXED_ALL ) ) {
    // update active model vars/cons using "All" view of
    // currentVariables/userDefinedConstraints data.
    model.continuous_variables(
      currentVariables.all_continuous_variables());
    model.discrete_int_variables(
      currentVariables.all_discrete_int_variables());
    model.discrete_real_variables(
      currentVariables.all_discrete_real_variables());
    model.continuous_lower_bounds(
      userDefinedConstraints.all_continuous_lower_bounds());
    model.continuous_upper_bounds(
      userDefinedConstraints.all_continuous_upper_bounds());
    model.discrete_int_lower_bounds(
      userDefinedConstraints.all_discrete_int_lower_bounds());
    model.discrete_int_upper_bounds(
      userDefinedConstraints.all_discrete_int_upper_bounds());
    model.discrete_real_lower_bounds(
      userDefinedConstraints.all_discrete_real_lower_bounds());
    model.discrete_real_upper_bounds(
      userDefinedConstraints.all_discrete_real_upper_bounds());
  }
  else if ( actual_active_view >= RELAXED_DESIGN &&
	    ( approx_active_view == RELAXED_ALL ||
	      approx_active_view == MIXED_ALL ) ) {
    // update "All" view of model vars/cons using active
    // currentVariables/userDefinedConstraints data.
    model.all_continuous_variables(
      currentVariables.continuous_variables());
    model.all_discrete_int_variables(
      currentVariables.discrete_int_variables());
    model.all_discrete_real_variables(
      currentVariables.discrete_real_variables());
    model.all_continuous_lower_bounds(
      userDefinedConstraints.continuous_lower_bounds());
    model.all_continuous_upper_bounds(
      userDefinedConstraints.continuous_upper_bounds());
    model.all_discrete_int_lower_bounds(
      userDefinedConstraints.discrete_int_lower_bounds());
    model.all_discrete_int_upper_bounds(
      userDefinedConstraints.discrete_int_upper_bounds());
    model.all_discrete_real_lower_bounds(
      userDefinedConstraints.discrete_real_lower_bounds());
    model.all_discrete_real_upper_bounds(
      userDefinedConstraints.discrete_real_upper_bounds());
  }
  // TO DO: extend for aleatory/epistemic uncertain views
  else {
    Cerr << "Error: unsupported variable view differences in "
	 << "DataFitSurrModel::update_model()" << std::endl;
    abort_handler(-1);
  }
}


/** Update values and labels in currentVariables and
    bound/linear/nonlinear constraints in userDefinedConstraints from
    variables and constraints data within model. */
void DataFitSurrModel::update_from_model(const Model& model)
{
  // vars/bounds/labels

  // update vars/bounds/labels with model data using All view for both
  // (since approx arrays are sized but otherwise uninitialized)
  currentVariables.all_continuous_variables(
    model.all_continuous_variables());
  currentVariables.all_discrete_int_variables(
    model.all_discrete_int_variables());
  currentVariables.all_discrete_real_variables(
    model.all_discrete_real_variables());
  userDefinedConstraints.all_continuous_lower_bounds(
    model.all_continuous_lower_bounds());
  userDefinedConstraints.all_continuous_upper_bounds(
    model.all_continuous_upper_bounds());
  userDefinedConstraints.all_discrete_int_lower_bounds(
    model.all_discrete_int_lower_bounds());
  userDefinedConstraints.all_discrete_int_upper_bounds(
    model.all_discrete_int_upper_bounds());
  userDefinedConstraints.all_discrete_real_lower_bounds(
    model.all_discrete_real_lower_bounds());
  userDefinedConstraints.all_discrete_real_upper_bounds(
    model.all_discrete_real_upper_bounds());
  if (!approxBuilds) {
    currentVariables.all_continuous_variable_labels(
      model.all_continuous_variable_labels());
    currentVariables.all_discrete_int_variable_labels(
      model.all_discrete_int_variable_labels());
    currentVariables.all_discrete_real_variable_labels(
      model.all_discrete_real_variable_labels());
    currentResponse.function_labels(model.response_labels());
  }

  if (!model.discrete_design_set_int_values().empty())
    discreteDesignSetIntValues = model.discrete_design_set_int_values();
  if (!model.discrete_design_set_real_values().empty())
    discreteDesignSetRealValues = model.discrete_design_set_real_values();

  // uncertain variable distribution data
  // Note: Variables instances defined from the same variablesId are not shared
  // (see ProblemDescDB::get_variables()), so we propagate any distribution
  // updates (e.g., NestedModel insertions) up/down the Model recursion.  For
  // differing variablesId, we cannot assume that the distribution information
  // can be mapped, since the distributions used to build may differ from those
  // used to evaluate.  More careful logic may be needed in the future...
  if (currentVariables.shared_data().id() ==
      model.current_variables().shared_data().id()) {
    aleatDistParams.update(model.aleatory_distribution_parameters());
    epistDistParams.update(model.epistemic_distribution_parameters());
  }

  if (!model.discrete_state_set_int_values().empty())
    discreteStateSetIntValues = model.discrete_state_set_int_values();
  if (!model.discrete_state_set_real_values().empty())
    discreteStateSetRealValues = model.discrete_state_set_real_values();

  // linear constraints

  if (model.num_linear_ineq_constraints()) {
    // the views don't necessarily have to be the same, but the number of
    // active continuous and active discrete variables have to be consistent.
    if (model.cv()  == currentVariables.cv()  &&
	model.div() == currentVariables.div() &&
	model.drv() == currentVariables.drv()) {
      userDefinedConstraints.linear_ineq_constraint_coeffs(
        model.linear_ineq_constraint_coeffs());
      userDefinedConstraints.linear_ineq_constraint_lower_bounds(
        model.linear_ineq_constraint_lower_bounds());
      userDefinedConstraints.linear_ineq_constraint_upper_bounds(
        model.linear_ineq_constraint_upper_bounds());
    }
    else {
      Cerr << "Error: cannot update linear inequality constraints in "
	   << "DataFitSurrModel::update_from_model() due to inconsistent "
	   << "active variables." << std::endl;
      abort_handler(-1);
    }
  }
  if (model.num_linear_eq_constraints()) {
    // the views don't necessarily have to be the same, but the number of
    // active continuous and active discrete variables have to be consistent.
    if (model.cv()  == currentVariables.cv()  &&
	model.div() == currentVariables.div() &&
	model.drv() == currentVariables.drv()) {
      userDefinedConstraints.linear_eq_constraint_coeffs(
        model.linear_eq_constraint_coeffs());
      userDefinedConstraints.linear_eq_constraint_targets(
        model.linear_eq_constraint_targets());
    }
    else {
      Cerr << "Error: cannot update linear equality constraints in "
	   << "DataFitSurrModel::update_from_model() due to inconsistent "
	   << "active variables." << std::endl;
      abort_handler(-1);
    }
  }

  // weights and sense for primary response functions

  primaryRespFnWts   = model.primary_response_fn_weights();
  primaryRespFnSense = model.primary_response_fn_sense();

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

} // namespace Dakota
