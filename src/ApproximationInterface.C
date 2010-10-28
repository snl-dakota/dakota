/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ApproximationInterface
//- Description:  Implementation of base class for approximation interfaces
//- Owner:        Mike Eldred

#include "system_defs.h"
#include "ApproximationInterface.H"
#include "DakotaVariables.H"
#include "DakotaResponse.H"
#include "ProblemDescDB.H"


namespace Dakota {

ApproximationInterface::
ApproximationInterface(ProblemDescDB& problem_db,
		       const Variables& actual_model_vars, size_t num_fns):
  Interface(BaseConstructor(), problem_db), 
  approxFnIndices(problem_db.get_dis("model.surrogate.function_indices")),
  graph3DFlag(problem_db.get_bool("strategy.graphics")),
  diag_list(problem_db.get_dsa("model.diagnostics")),
  actualModelVars(actual_model_vars.copy())
{
  // Some specification-based attributes inherited from Interface may
  // be incorrect since there is no longer an approximation interface
  // specification (assign_rep() is used from DataFitSurrModel).
  // Override these inherited settings.
  idInterface   = "APPROX_INTERFACE";
  interfaceType = "approximation";
  algebraicMappings = false; // for now; *** TO DO ***
  coreMappings      = true;

  // process approxFnIndices.  IntSets are sorted and unique.  Error checking
  // is performed in SurrogateModel and does not need to be replicated here.
  if (approxFnIndices.empty()) // default: all fns are approximated
    for (int i=0; i<num_fns; i++)
      approxFnIndices.insert(i);

  // This section moved to the constructor so that ApproxInterfaces can be
  // queried for their state prior to build_approximation() (e.g., to configure
  // parallelism w/ max_concurrency).  Also, Approximation classes now
  // use the problem_db, so their instantiation must occur in constructors to 
  // assure proper list node settings.
  functionSurfaces.resize(num_fns);
  // despite view mappings, x in map() always = size of active actualModelVars
  size_t num_vars = actualModelVars.cv();
  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it)
    functionSurfaces[*it] = Approximation(problem_db, num_vars);

  /*
  // Old approach for scaling/offsetting approximation results read the data
  // from a hardwired filename (approx_scale_offset.in).  This capability had
  // been used (~Spring 2000) to reuse existing surrogates with modified design
  // goals, particularly modified constraint allowables via approximation
  // offsets.  The intent was to eventually make this a supported capability
  // within the interface keyword of the input specification, e.g.:
  //		[approx_scale  = <LISTof><REAL>]
  //		[approx_offset = <LISTof><REAL>]
  // An outstanding question was which approximation types to support, i.e. 
  // (1) global: primary use, (2) local/multipoint: somewhat useful, and
  // (3) hierarchical:  not supported in HierarchSurrModel and not that useful
  // since correction is required.
  //
  // Since that time, the capability for specifying inequality constraint bounds
  // and equality constraint targets (~Spring 2001) has largely superceded the
  // capability to scale/offset approximation results, since modifying the
  // constraint bounds/targets for subsequent surrogate-based runs has the same
  // effect.  However, if the need to offset/scale surrogate objective functions
  // or least squares terms arises, then this is not supported in the existing
  // code and the block below (as well as corresponding lines in map()) could be
  // reactivated.
  ifstream scale_offset("approx_scale_offset.in");
  if (scale_offset) {
    approxScale.reshape(num_fns);
    approxOffset.reshape(num_fns);
    approxScale  = 1.; // default
    approxOffset = 0.; // default
    int start_id, end_id;
    Real scale, offset;
    while (!scale_offset.eof()) {
      // Read a record with format: <start id> <end id> <scale> <offset>
      scale_offset >> start_id >> end_id >> scale >> offset;
      Cout << "approx_scale_offset.in: " << start_id << ' ' << end_id << ' '
           << scale << ' '  << offset << '\n';
      // Check that start_id/end_id are reasonable
      if ( start_id < 1 || start_id > num_fns || end_id < 1
        || end_id > num_fns || start_id > end_id) {
        Cerr << "Error: start/end id's from approx_scale_offset.in do not "
	     << "define a valid range\n       within the " << num_fns 
             << " response functions." << std::endl;
        abort_handler(-1);
      }
      // Set scale and offset for the start_id/end_id range
      for (int i=start_id-1; i<=end_id-1; i++) { // 1-based
        approxScale[i]  = scale;
        approxOffset[i] = offset;
      }
    }
  }
  */
}


ApproximationInterface::
ApproximationInterface(const String& approx_type,
		       const UShortArray& approx_order,
		       const Variables& actual_model_vars, size_t num_fns,
		       unsigned short data_order):
  Interface(NoDBBaseConstructor(), num_fns), graph3DFlag(false),
  actualModelVars(actual_model_vars.copy())
{
  idInterface   = "APPROX_INTERFACE";
  interfaceType = "approximation";

  functionSurfaces.resize(num_fns);
  // despite view mappings, x in map() always = size of active actualModelVars
  size_t num_vars = actualModelVars.cv();
  for (int i=0; i<num_fns; i++) {
    approxFnIndices.insert(i);
    functionSurfaces[i]
      = Approximation(approx_type, approx_order, num_vars, data_order);
  }
}


void ApproximationInterface::
map(const Variables& vars, const ActiveSet& set, Response& response,
    const bool asynch_flag)
{
  fnEvalId++;    // all calls to map (used throughout as eval id)
  newFnEvalId++; // nonduplicate evaluations (used ONLY in fn. eval. summary)
  if (fineGrainEvalCounters) { // detailed evaluation reporting
    const ShortArray& asv = set.request_vector();
    size_t i, num_fns = asv.size();
    for (i=0; i<num_fns; ++i) {
      short asv_val = asv[i];
      if (asv_val & 1) { fnValCounter[i]++;  newFnValCounter[i]++;  }
      if (asv_val & 2) { fnGradCounter[i]++; newFnGradCounter[i]++; }
      if (asv_val & 4) { fnHessCounter[i]++; newFnHessCounter[i]++; }
    }
    if (fnLabels.empty())
      fnLabels = response.function_labels();
  }

  // output the current parameter values prior to running the Analysis
  if (outputLevel > NORMAL_OUTPUT) {
    Cout << "\n------------------------------------\n"
         <<   "Begin Approximate Fn Evaluation " << std::setw(4) << fnEvalId;
    // This may be more confusing than helpful:
    //if (fnEvalIdRefPt)
    //  Cout << " (local evaluation " << fnEvalId - fnEvalIdRefPt << ")";
    Cout << "\n------------------------------------\nParameters for "
         << "approximate fn evaluation " << fnEvalId << ":\n" << vars << '\n';
  }
  else if (fnEvalId == 1)
    Cout << "Beginning Approximate Fn Evaluations..." << std::endl;

  //if (asvControlFlag) // else leave response ActiveSet as initialized
  response.active_set(set); // set the current ActiveSet within the response

  // Subdivide set for algebraic_mappings() and derived_map()
  Response algebraic_response, core_response; // empty handles
  ActiveSet core_set;

  if (algebraicMappings) {
    if (fnEvalId == 1)
      init_algebraic_mappings(vars, response);
    if (coreMappings) { // both mappings
      ActiveSet algebraic_set;
      asv_mapping(set, algebraic_set, core_set);
      algebraic_response = Response(algebraic_set);
      algebraic_mappings(vars, algebraic_set, algebraic_response);
      // separate core_response from response
      core_response = response.copy();
      core_response.active_set(core_set);
    }
    else // algebraic mappings only
      algebraic_mappings(vars, set, response);
  }
  else if (coreMappings) { // analysis_driver mappings only
    core_set      = set;
    core_response = response; // shared rep
  }

  if (coreMappings) {

    // Evaluate functionSurfaces at vars and populate core_response.
    const ShortArray& core_asv = core_set.request_vector();
    size_t i, num_core_fns = core_asv.size();
    if ( num_core_fns != functionSurfaces.size() ) {
      Cerr << "Error: mismatch in number of functions in ApproximationInterface"
	   << "::map()" << std::endl;
      abort_handler(-1);
    }
    // perform any conversions necessary to map between the approximation and
    // underlying actualModel variables views.  Within SurrogateModel,
    // check_submodel_compatibility() is responsible for verifying that the
    // views are compatible and force_rebuild() is responsible for verifying
    // that no unapproximated variables have changed (which would invalidate
    // the approximation).
    short approx_active_view = vars.view().first,
          actual_active_view = actualModelVars.view().first;
    RealVector x; // view
    if (approx_active_view == actual_active_view)
      x = vars.continuous_variables();
    else if ( ( actual_active_view == MERGED_ALL ||
		actual_active_view == MIXED_ALL ) &&
	      approx_active_view >= MERGED_DISTINCT_DESIGN ) // Distinct to All
      //actualModelVars.continuous_variables(vars.all_continuous_variables());
      x = vars.all_continuous_variables();
    else if ( ( approx_active_view == MERGED_ALL ||
		approx_active_view == MIXED_ALL ) &&
	      actual_active_view >= MERGED_DISTINCT_DESIGN) { // All to Distinct
      actualModelVars.all_continuous_variables(vars.continuous_variables());
      x = actualModelVars.continuous_variables();
    }
    // TO DO: extend for aleatory/epistemic uncertain views
    else {
      Cerr << "Error: unsupported variable view differences in "
	   << "ApproximationInterface::map()" << std::endl;
      abort_handler(-1);
    }
    // avoid unnecessary data copying
    //const RealVector& x = (approx_active_view == actual_active_view) ?
    //  vars.continuous_variables() :          //Distinct to Distinct,All to All
    //  actualModelVars.continuous_variables();//Distinct to All,All to Distinct

    //size_t num_core_vars = x.length(), 
    //bool approx_scale_len  = (approxScale.length())  ? true : false;
    //bool approx_offset_len = (approxOffset.length()) ? true : false;
    for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
      int index = *it;
      if (core_asv[index] & 1) {
	const Real& fn_val = functionSurfaces[index].get_value(x);
	//if (approx_scale_len)
	//  fn_val *= approxScale[index];
	//if (approx_offset_len)
	//  fn_val += approxOffset[index];
	core_response.function_value(fn_val, index);
      }
      if (core_asv[index] & 2) { // TO DO: ACV->DVV
	const RealVector& fn_grad= functionSurfaces[index].get_gradient(x);
	//if (approx_scale_len)
	//  for (size_t j=0; j<num_core_vars; j++)
	//    fn_grad[j] *= approxScale[index];
	core_response.function_gradient(fn_grad, index);
      }
      if (core_asv[index] & 4) { // TO DO: ACV->DVV
	const RealSymMatrix& fn_hess = functionSurfaces[index].get_hessian(x);
	//if (approx_scale_len)
	//  for (size_t j=0; j<num_core_vars; j++)
	//    for (size_t k=0; k<num_core_vars; k++)
	//      fn_hess[j][k] *= approxScale[index];
	core_response.function_hessian(fn_hess, index);
      }
    }
  }

  if (algebraicMappings && coreMappings)
    response_mapping(algebraic_response, core_response, response);

  if (outputLevel > NORMAL_OUTPUT)
    Cout << "\nActive response data for approximate fn evaluation " << fnEvalId
	 << ":\n" << response << '\n';
  if (asynch_flag)
    beforeSynchResponseMap[fnEvalId] = response.copy();
}


// Little distinction between blocking and nonblocking synch since all 
// responses are completed.
const IntResponseMap& ApproximationInterface::synch()
{
  // move data from beforeSynch map to completed map
  rawResponseMap = beforeSynchResponseMap;
  beforeSynchResponseMap.clear();
  return rawResponseMap;
}


const IntResponseMap& ApproximationInterface::synch_nowait()
{
  // move data from beforeSynch map to completed map
  rawResponseMap = beforeSynchResponseMap;
  beforeSynchResponseMap.clear();
  return rawResponseMap;
}


/** This function populates/replaces each Approximation::anchorPoint
    with the incoming variables/response data point. */
void ApproximationInterface::
update_approximation(const Variables& vars, const Response& response)
{
  // NOTE: variable sets passed in from DataFitSurrModel::build_approximation()
  // correspond to the active continuous variables for either the top level
  // model or sub-model (DataFitSurrModel::currentVariables or
  // DataFitSurrModel::actualModel::currentVariables) since the view is the same
  // in the local case.  This can be inconsistent with the use of all continuous
  // variables above (in constructor and map()) if there are inactive variables
  // which are changing (e.g. OUU w/ surrogate at UQ level).  Currently, the
  // Taylor series does not compute response derivs w.r.t. inactive variables
  // and the approximation therefore cannot capture any changes in the inactive
  // variable values.  For this reason, DataFitSurrModel::force_rebuild() forces
  // a Taylor series rebuild whenever the inactive variable values change.

  // rather than unrolling the response (containing all response functions)
  // into per-response function arrays for input to functionSurfaces[i], pass
  // the complete response along with a response function index.
  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    int index = *it;
    // populate/replace Approximation::anchorPoint
    functionSurfaces[index].update(vars, response, index);
  }
}


/** This function populates/replaces each Approximation::currentPoints
    with the incoming variables/response arrays. */
void ApproximationInterface::
update_approximation(const RealMatrix& samples, const ResponseArray& resp_array)
{
  // NOTE: variable sets passed in from DataFitSurrModel::build_approximation()
  // correspond to the all continuous variables at the top level in
  // DataFitSurrModel::currentVariables and the active continuous variables at
  // the sub-model level in DataFitSurrModel::actualModel (daceIterator uses an
  // all variables view in DDACEDesignCompExp, FSUDesignCompExp, or NonDSampling
  // in "all_variables" mode).  Since the ApproximationInterface corresponds to
  // mappings at the top level, the all continuous variables are used above in
  // the constructor and in map().

  // Build the global approximation surfaces
  if (resp_array.size() != samples.numCols()) {
    Cerr << "Error: mismatch in sample and response set lengths in "
	 << "ApproximationInterface::update_approximation()." << std::endl;
    abort_handler(-1);
  }
  // rather than unrolling response arrays (containing all response functions)
  // into per-response function arrays for input to functionSurfaces[i], pass
  // the complete response arrays along with a response function index.
  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    int index = *it;
    // populate/replace Approximation::currentPoints
    functionSurfaces[index].update(samples, resp_array, index);
  }
}


/** This function populates/replaces each Approximation::currentPoints
    with the incoming variables/response arrays. */
void ApproximationInterface::
update_approximation(const VariablesArray& vars_array,
		     const ResponseArray& resp_array)
{
  // NOTE: variable sets passed in from DataFitSurrModel::build_approximation()
  // correspond to the all continuous variables at the top level in
  // DataFitSurrModel::currentVariables and the active continuous variables at
  // the sub-model level in DataFitSurrModel::actualModel (daceIterator uses an
  // all variables view in DDACEDesignCompExp, FSUDesignCompExp, or NonDSampling
  // in "all_variables" mode).  Since the ApproximationInterface corresponds to
  // mappings at the top level, the all continuous variables are used above in
  // the constructor and in map().

  // Build the global approximation surfaces
  if (resp_array.size() != vars_array.size()) {
    Cerr << "Error: mismatch in variable and response set lengths in "
	 << "ApproximationInterface::update_approximation()." << std::endl;
    abort_handler(-1);
  }
  // rather than unrolling response arrays (containing all response functions)
  // into per-response function arrays for input to functionSurfaces[i], pass
  // the complete response arrays along with a response function index.
  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    int index = *it;
    // populate/replace Approximation::currentPoints
    functionSurfaces[index].update(vars_array, resp_array, index);
  }
}


/** This function appends to each Approximation::currentPoints with
    one incoming variables/response data point. */
void ApproximationInterface::
append_approximation(const Variables& vars, const Response& response)
{
  // rather than unrolling the response (containing all response functions)
  // into per-response function arrays for input to functionSurfaces[i], pass
  // the complete response along with a response function index.
  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    int index = *it;
    // append an entry to Approximation::currentPoints
    functionSurfaces[index].append(vars, response, index);
  }
}


/** This function appends to each Approximation::currentPoints with
    multiple incoming variables/response data points. */
void ApproximationInterface::
append_approximation(const RealMatrix& samples, const ResponseArray& resp_array)
{
  if (resp_array.size() != samples.numCols()) {
    Cerr << "Error: mismatch in sample and response set lengths in "
	 << "ApproximationInterface::append_approximation()." << std::endl;
    abort_handler(-1);
  }
  // rather than unrolling response arrays (containing all response functions)
  // into per-response function arrays for input to functionSurfaces[i], pass
  // the complete response arrays along with a response function index.
  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    int index = *it;
    // append an entry to Approximation::currentPoints
    functionSurfaces[index].append(samples, resp_array, index);
  }
}


/** This function appends to each Approximation::currentPoints with
    multiple incoming variables/response data points. */
void ApproximationInterface::
append_approximation(const VariablesArray& vars_array,
		     const ResponseArray& resp_array)
{
  if (resp_array.size() != vars_array.size()) {
    Cerr << "Error: mismatch in variable and response set lengths in "
	 << "ApproximationInterface::append_approximation()." << std::endl;
    abort_handler(-1);
  }
  // rather than unrolling response arrays (containing all response functions)
  // into per-response function arrays for input to functionSurfaces[i], pass
  // the complete response arrays along with a response function index.
  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    int index = *it;
    // append an entry to Approximation::currentPoints
    functionSurfaces[index].append(vars_array, resp_array, index);
  }
}


/** This function finds the coefficients for each Approximation based
    on the data passed through update_approximation() calls.  The
    bounds are used only for graphics visualization. */
void ApproximationInterface::
build_approximation(const BoolDeque& rebuild_deque,
		    const RealVector& lower_bnds,
		    const RealVector& upper_bnds)
{
  // build the approximation surfaces
  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    int index = *it;
    // check for rebuild request (defaults to true if no deque defined)
    if (rebuild_deque.empty() || rebuild_deque[index]) {
      functionSurfaces[index].build();
      if (functionSurfaces[index].diagnostics_available()) {
	if (!diag_list.empty()) {
	  int num_diag = diag_list.size();
	  for(int j = 0; j < num_diag; j++)
	    functionSurfaces[index].get_diagnostic(diag_list[j]);
	}
	if (outputLevel > NORMAL_OUTPUT) {
	  functionSurfaces[index].get_diagnostic("rsquared");
	  functionSurfaces[index].get_diagnostic("root_mean_squared");	
	  functionSurfaces[index].get_diagnostic("mean_abs");
	}
      }
    }
  }
  // if graphics is on for 2 variables, plot first functionSurface in 3D
  int index = *approxFnIndices.begin();
  if (graph3DFlag && ( rebuild_deque.empty() || rebuild_deque[index] ) &&
      functionSurfaces[index].num_variables() == 2) {
    functionSurfaces[index].set_bounds(lower_bnds, upper_bnds);
    functionSurfaces[index].draw_surface();
  }
}


/** This function updates the coefficients for each Approximation based
    on data increments provided by {update,append}_approximation(). */
void ApproximationInterface::
rebuild_approximation(const BoolDeque& rebuild_deque)
{
  // rebuild the approximation surfaces
  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it)
    // check for rebuild request (defaults to true if no deque defined)
    if (rebuild_deque.empty() || rebuild_deque[*it])
      functionSurfaces[*it].rebuild(); // invokes increment_coefficients()
}


/** This function removes data provided by a previous call to
    append_approximation(). */
void ApproximationInterface::pop_approximation(bool save_sdp_set)
{
  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it)
    // remove entries from Approximation::currentPoints
    functionSurfaces[*it].pop(save_sdp_set);
}


/** This function updates the coefficients for each Approximation based
    on data increments provided by {update,append}_approximation(). */
void ApproximationInterface::restore_approximation()
{
  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it)
    // remove entries from Approximation::currentPoints
    functionSurfaces[*it].restore();
}


/** This function updates the coefficients for each Approximation based
    on data increments provided by {update,append}_approximation(). */
bool ApproximationInterface::restore_available()
{
  bool avail = true;
  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it)
    // remove entries from Approximation::currentPoints
    if (!functionSurfaces[*it].restore_available())
      { avail = false; break; }
  return avail;
}


void ApproximationInterface::finalize_approximation()
{
  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it)
    // remove entries from Approximation::currentPoints
    functionSurfaces[*it].finalize();
}


const RealVectorArray& ApproximationInterface::approximation_coefficients()
{
  // only assign the functionSurfaceCoeffs array if it's requested
  // (i.e., do it here rather than in build/update functions above).
  if (functionSurfaceCoeffs.empty())
    functionSurfaceCoeffs.resize(functionSurfaces.size());
  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    int index = *it;
    functionSurfaceCoeffs[index]
      = functionSurfaces[index].approximation_coefficients();
  }
  return functionSurfaceCoeffs;
}


void ApproximationInterface::
approximation_coefficients(const RealVectorArray& approx_coeffs)
{
  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    int index = *it;
    functionSurfaces[index].approximation_coefficients(approx_coeffs[index]);
  }
  //functionSurfaceCoeffs = approx_coeffs;
}


const RealVector& ApproximationInterface::
approximation_variances(const RealVector& c_vars)
{
  // only assign the functionSurfaceVariances array if it's requested
  // (i.e., do it here rather than in build/update functions above).
  if (functionSurfaceVariances.empty())
    functionSurfaceVariances.sizeUninitialized(functionSurfaces.size());
  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    int index = *it;
    functionSurfaceVariances[index]
      = functionSurfaces[index].get_prediction_variance(c_vars);
  }
  return functionSurfaceVariances;
}


const SDPList& ApproximationInterface::approximation_data(size_t index)
{
  if (!approxFnIndices.count(index)) {
    Cerr << "Error: index passed to ApproximationInterface::approximation_data"
	 << "() does not correspond to an approximated function." << std::endl;
    abort_handler(-1);
  }
  const Approximation& approx = functionSurfaces[index];
  if (approx.anchor()) {
    // preserve standard order: anchor first, followed by current points
    functionSurfaceDataPoints.clear();
    functionSurfaceDataPoints.push_back(approx.anchor_point());
    const SDPList& current_pts = approx.current_points();
    for (SDPLCIter cit = current_pts.begin(); cit!=current_pts.end(); ++cit)
      functionSurfaceDataPoints.push_back(*cit);
    return functionSurfaceDataPoints;
  }
  else
    return approx.current_points();
}

} // namespace Dakota
