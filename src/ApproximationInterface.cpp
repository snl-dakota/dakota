/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ApproximationInterface
//- Description:  Implementation of base class for approximation interfaces
//- Owner:        Mike Eldred

#include "dakota_system_defs.hpp"
#include "dakota_tabular_io.hpp"
#include "ApproximationInterface.hpp"
#include "DakotaVariables.hpp"
#include "DakotaResponse.hpp"
#include "ProblemDescDB.hpp"
//#include "PRPMultiIndex.hpp"

//#define DEBUG


namespace Dakota {

extern PRPCache data_pairs;

size_t ApproximationInterface::approxIdNum = 0;

ApproximationInterface::
ApproximationInterface(ProblemDescDB& problem_db, const Variables& am_vars,
		       bool am_cache, const String& am_interface_id,
		       const StringArray& fn_labels):
  Interface(BaseConstructor(), problem_db), 
  approxFnIndices(problem_db.get_szs("model.surrogate.function_indices")),
  trackEvalIds(false),
  //graph3DFlag(problem_db.get_bool("environment.graphics")),
  challengeFile(problem_db.get_string("model.surrogate.challenge_points_file")),
  challengeFormat(
    problem_db.get_ushort("model.surrogate.challenge_points_file_format")),
  challengeUseVarLabels(
    problem_db.get_bool("model.surrogate.challenge_use_variable_labels")),
  challengeActiveOnly(
    problem_db.get_bool("model.surrogate.challenge_points_file_active")),
  actualModelVars(am_vars.copy()), actualModelCache(am_cache),
  actualModelInterfaceId(am_interface_id)
{
  // Some specification-based attributes inherited from Interface may
  // be incorrect since there is no longer an approximation interface
  // specification (assign_rep() is used from DataFitSurrModel).
  // Override these inherited settings.
  interfaceId = String("APPROX_INTERFACE_") + std::to_string(++approxIdNum);
  interfaceType = APPROX_INTERFACE;
  algebraicMappings = false; // for now; *** TO DO ***

  // process approxFnIndices.  SizetSets are sorted and unique.  Error checking
  // is performed in SurrogateModel and does not need to be replicated here.
  size_t i, num_fns = fn_labels.size();
  if (approxFnIndices.empty()) // default: all fns are approximated
    for (i=0; i<num_fns; i++)
      approxFnIndices.insert(i);

  // This section moved to the constructor so that ApproxInterfaces can be
  // queried for their state prior to build_approximation() (e.g., to configure
  // parallelism w/ max_concurrency).  Also, Approximation classes now
  // use the problem_db, so their instantiation must occur in constructors to 
  // assure proper list node settings.
  functionSurfaces.resize(num_fns);
  // despite view mappings, x in map() always = size of active actualModelVars
  size_t num_vars = actualModelVars.cv()  + actualModelVars.div()
                  + actualModelVars.dsv() + actualModelVars.drv();
  sharedData = SharedApproxData(problem_db, num_vars);
  for (StSIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it)
    functionSurfaces[*it] = Approximation(problem_db, sharedData,
                                          fn_labels[*it]);

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
  // (3) hierarchical:  not supported in EnsembleSurrModel and not that useful
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
		       const Variables& am_vars, bool am_cache,
		       const String& am_interface_id, size_t num_fns,
		       short data_order, short output_level):
  Interface(NoDBBaseConstructor(), num_fns, output_level), //graph3DFlag(false),
  trackEvalIds(false), challengeFormat(TABULAR_ANNOTATED),
  challengeActiveOnly(false), actualModelVars(am_vars.copy()),
  actualModelCache(am_cache), actualModelInterfaceId(am_interface_id)
{
  interfaceId = String("APPROX_INTERFACE_") + std::to_string(++approxIdNum);
  interfaceType = APPROX_INTERFACE;

  functionSurfaces.resize(num_fns);
  // despite view mappings, x in map() always = size of active actualModelVars
  size_t i, num_vars = actualModelVars.cv()  + actualModelVars.div()
                     + actualModelVars.dsv() + actualModelVars.drv();
  sharedData = SharedApproxData(approx_type, approx_order, num_vars,
				data_order, output_level);
  for (i=0; i<num_fns; i++) {
    approxFnIndices.insert(i);
    functionSurfaces[i] = Approximation(sharedData);
  }
}


void ApproximationInterface::
map(const Variables& vars, const ActiveSet& set, Response& response,
    bool asynch_flag)
{
  ++evalIdCntr;    // all calls to map (used throughout as eval id)
  ++newEvalIdCntr; // nonduplicate evaluations (used ONLY in fn. eval. summary)
  if (fineGrainEvalCounters) { // detailed evaluation reporting
    const ShortArray& asv = set.request_vector();
    size_t i, num_fns = asv.size();
    init_evaluation_counters(num_fns);
    for (i=0; i<num_fns; ++i) {
      short asv_val = asv[i];
      if (asv_val & 1) { ++fnValCounter[i];  ++newFnValCounter[i];  }
      if (asv_val & 2) { ++fnGradCounter[i]; ++newFnGradCounter[i]; }
      if (asv_val & 4) { ++fnHessCounter[i]; ++newFnHessCounter[i]; }
    }
    if (fnLabels.empty())
      fnLabels = response.function_labels();
  }

  // output the current parameter values prior to running the Analysis
  if (outputLevel > NORMAL_OUTPUT) {
    Cout << "\n------------------------------------\n"
         <<   "Begin Approximate Fn Evaluation " << std::setw(4) << evalIdCntr;
    // This may be more confusing than helpful:
    //if (evalIdRefPt)
    //  Cout << " (local evaluation " << evalIdCntr - evalIdRefPt << ")";
    Cout << "\n------------------------------------\nParameters for "
         << "approximate fn evaluation " << evalIdCntr << ":\n" << vars << '\n';
  }
  else if (evalIdCntr == 1)
    Cout << "Beginning Approximate Fn Evaluations..." << std::endl;

  //if (asvControlFlag) // else leave response ActiveSet as initialized
  response.active_set(set); // set the current ActiveSet within the response

  // Subdivide set for algebraic_mappings() and derived_map()
  Response algebraic_response, core_response; // empty handles
  ActiveSet core_set;

  if (algebraicMappings) {
    if (evalIdCntr == 1)
      init_algebraic_mappings(vars, response);
    if (coreMappings) { // both mappings
      ActiveSet algebraic_set;
      asv_mapping(set, algebraic_set, core_set);
      algebraic_response = Response(SIMULATION_RESPONSE, algebraic_set);
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
    bool am_vars = true;
    if (approx_active_view == actual_active_view)
      am_vars = false;
    else if ( ( actual_active_view == RELAXED_ALL ||
		actual_active_view == MIXED_ALL ) &&
	      approx_active_view >= RELAXED_DESIGN ) { // Distinct to All
      if (vars.acv())
	actualModelVars.continuous_variables(vars.all_continuous_variables());
      if (vars.adiv())
	actualModelVars.discrete_int_variables(
	  vars.all_discrete_int_variables());
      if (vars.adsv())
	actualModelVars.discrete_string_variables(
	  vars.all_discrete_string_variables());
      if (vars.adrv())
	actualModelVars.discrete_real_variables(
	  vars.all_discrete_real_variables());
    }
    else if ( ( approx_active_view == RELAXED_ALL ||
		approx_active_view == MIXED_ALL ) &&
	      actual_active_view >= RELAXED_DESIGN) { // All to Distinct
      if (vars.cv())
	actualModelVars.all_continuous_variables(vars.continuous_variables());
      if (vars.div())
	actualModelVars.all_discrete_int_variables(
	  vars.discrete_int_variables());
      if (vars.dsv())
	actualModelVars.all_discrete_string_variables(
	  vars.discrete_string_variables());
      if (vars.drv())
	actualModelVars.all_discrete_real_variables(
	  vars.discrete_real_variables());
    }
    // TO DO: extend for aleatory/epistemic uncertain views
    else {
      Cerr << "Error: unsupported variable view differences in "
	   << "ApproximationInterface::map()" << std::endl;
      abort_handler(-1);
    }
    // active subsets of actualModelVars are used in surrogate construction
    // and evaluation
    const Variables& surf_vars = (am_vars) ? actualModelVars : vars;

    //size_t num_core_vars = x.length(), 
    //bool approx_scale_len  = (approxScale.length())  ? true : false;
    //bool approx_offset_len = (approxOffset.length()) ? true : false;
    for (StSIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
      size_t index = *it;
      if (core_asv[index] & 1) {
	Real fn_val = functionSurfaces[index].value(surf_vars);
	//if (approx_scale_len)
	//  fn_val *= approxScale[index];
	//if (approx_offset_len)
	//  fn_val += approxOffset[index];
	core_response.function_value(fn_val, index);
      }
      if (core_asv[index] & 2) { // TO DO: ACV->DVV
	const RealVector& fn_grad = functionSurfaces[index].gradient(surf_vars);
	//if (approx_scale_len)
	//  for (size_t j=0; j<num_core_vars; j++)
	//    fn_grad[j] *= approxScale[index];
	core_response.function_gradient(fn_grad, index);
      }
      if (core_asv[index] & 4) { // TO DO: ACV->DVV
	const RealSymMatrix& fn_hess
	  = functionSurfaces[index].hessian(surf_vars);
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
    Cout << "\nActive response data for approximate fn evaluation "
	 << evalIdCntr << ":\n" << response << '\n';
  if (asynch_flag)
    beforeSynchResponseMap[evalIdCntr] = response.copy();
}


// Little distinction between blocking and nonblocking synch since all 
// responses are completed.
const IntResponseMap& ApproximationInterface::synchronize()
{
  // move data from beforeSynch map to completed map
  rawResponseMap.clear();
  std::swap(beforeSynchResponseMap, rawResponseMap);

  // augment with any cached evals
  rawResponseMap.insert(cachedResponseMap.begin(), cachedResponseMap.end());
  cachedResponseMap.clear();

  return rawResponseMap;
}


const IntResponseMap& ApproximationInterface::synchronize_nowait()
{
  // move data from beforeSynch map to completed map
  rawResponseMap.clear();
  std::swap(beforeSynchResponseMap, rawResponseMap);

  // augment with any cached evals
  rawResponseMap.insert(cachedResponseMap.begin(), cachedResponseMap.end());
  cachedResponseMap.clear();

  return rawResponseMap;
}


/** This function populates/replaces each Approximation::anchorPoint
    with the incoming variables/response data point. */
void ApproximationInterface::
update_approximation(const Variables& vars, const IntResponsePair& response_pr)
{
  // NOTE: variable sets passed in from DataFitSurrModel::build_approximation()
  // correspond to the active continuous variables for either the top level
  // model or sub-model (DataFitSurrModel::currentVariables or
  // DataFitSurrModel::actualModel::currentVariables) since the view is the same
  // in the local case.  This can be inconsistent with the use of all continuous
  // variables above (in constructor and map()) if there are inactive variables
  // which are changing (e.g. OUU w/ surrogate at UQ level).  Currently, a
  // Taylor series does not compute response derivs w.r.t. inactive variables
  // and the approximation therefore cannot capture any changes in the inactive
  // variable values.  For this reason, DataFitSurrModel::force_rebuild() forces
  // a Taylor series rebuild whenever the inactive variable values change.

  // add/replace SurrogateData::anchor{Vars,Resp}
  if (actualModelCache) {
    // anchor vars/resp are not sufficiently persistent for use in shallow
    // copies.  Therefore, use ordered id lookup in global PRPCache.
    PRPCacheCIter p_it
      = cache_lookup(vars, response_pr.first, response_pr.second);
    if (p_it == data_pairs.end()) // deep response copies with vars sharing
      mixed_add(vars, response_pr, true);
    else                          // shallow copies of cached vars/resp data
      shallow_add(p_it->variables(), p_it->response_pair(), true);
  }
  else                            // deep response copies with vars sharing
    mixed_add(vars, response_pr, true);

  // reset active approxData key using SharedApproxData::activeKey
  restore_data_key();
}


/** This function populates/replaces each Approximation::currentPoints
    with the incoming variables/response arrays. */
void ApproximationInterface::
update_approximation(const RealMatrix& samples, const IntResponseMap& resp_map)
{
  size_t i, num_pts = resp_map.size();
  if (samples.numCols() != num_pts) {
    Cerr << "Error: mismatch in variable and response set lengths in "
	 << "ApproximationInterface::update_approximation()." << std::endl;
    abort_handler(-1);
  }
  // clear active SurrogateData::{vars,resp}Data
  StSIter a_it; IntRespMCIter r_it;
  for (a_it=approxFnIndices.begin(); a_it!=approxFnIndices.end(); ++a_it)
    functionSurfaces[*a_it].clear_active_data();
  // replace active SurrogateData::{vars,resp}Data
  if (actualModelCache) {
    PRPCacheCIter p_it; size_t num_cv = samples.numRows();
    for (i=0, r_it=resp_map.begin(); i<num_pts; ++i, ++r_it) {
      // allVariables/allResponses are not sufficiently persistent for use in
      // shallow copies.  Therefore, use ordered id lookup in global PRPCache.
      p_it = cache_lookup(samples[i], num_cv, r_it->first, r_it->second);
      if (p_it == data_pairs.end()) // deep response copies with vars sharing
	mixed_add(samples[i], *r_it, false);
      else                          // shallow copies of cached vars/resp data
	shallow_add(p_it->variables(), p_it->response_pair(), false);
    }
  }
  else                              // deep response copies with vars sharing
    for (i=0, r_it=resp_map.begin(); i<num_pts; ++i, ++r_it)
      mixed_add(samples[i], *r_it, false);

  // reset active approxData key using SharedApproxData::activeKey
  restore_data_key();
}


/** This function populates/replaces each Approximation::currentPoints
    with the incoming variables/response arrays. */
void ApproximationInterface::
update_approximation(const VariablesArray& vars_array,
		     const IntResponseMap& resp_map)
{
  size_t i, num_pts = resp_map.size();
  if (vars_array.size() != num_pts) {
    Cerr << "Error: mismatch in variable and response set lengths in "
	 << "ApproximationInterface::update_approximation()." << std::endl;
    abort_handler(-1);
  }
  // clear active SurrogateData::{vars,resp}Data
  StSIter a_it; IntRespMCIter r_it;
  for (a_it=approxFnIndices.begin(); a_it!=approxFnIndices.end(); ++a_it)
    functionSurfaces[*a_it].clear_active_data();
  // replace active SurrogateData::{vars,resp}Data
  if (actualModelCache) {
    PRPCacheCIter p_it;
    for (i=0, r_it=resp_map.begin(); i<num_pts; ++i, ++r_it) {
      // allVariables/allResponses are not sufficiently persistent for use in
      // shallow copies.  Therefore, use ordered id lookup in global PRPCache.
      p_it = cache_lookup(vars_array[i], r_it->first, r_it->second);
      if (p_it == data_pairs.end()) // deep response copies with vars sharing
	mixed_add(vars_array[i], *r_it, false);
      else                          // shallow copies of cached vars/resp data
	shallow_add(p_it->variables(), p_it->response_pair(), false);
    }
  }
  else                            // deep response copies with vars sharing
    for (i=0, r_it=resp_map.begin(); i<num_pts; ++i, ++r_it)
      mixed_add(vars_array[i], *r_it, false);

  // reset active approxData key using SharedApproxData::activeKey
  restore_data_key();
}


/** This function appends to each Approximation::currentPoints with
    one incoming variables/response data point. */
void ApproximationInterface::
append_approximation(const Variables& vars, const IntResponsePair& response_pr)
{
  // append a single point to SurrogateData::{vars,resp}Data
  if (actualModelCache) {
    // anchor vars/resp are not sufficiently persistent for use in shallow
    // copies.  Therefore, use ordered id lookup in global PRPCache.
    PRPCacheCIter p_it
      = cache_lookup(vars, response_pr.first, response_pr.second);
    if (p_it == data_pairs.end()) // deep response copies with vars sharing
      mixed_add(vars, response_pr, false);
    else                          // shallow copies of cached vars/resp data
      shallow_add(p_it->variables(), p_it->response_pair(), false);
  }
  else                            // deep response copies with vars sharing
    mixed_add(vars, response_pr, false);

  update_pop_counts(response_pr);

  // reset active approxData key using SharedApproxData::activeKey
  restore_data_key();
}


/** This function appends to each Approximation::currentPoints with
    multiple incoming variables/response data points. */
void ApproximationInterface::
append_approximation(const RealMatrix& samples, const IntResponseMap& resp_map)
{
  size_t i, num_pts = resp_map.size();
  if (samples.numCols() != num_pts) {
    Cerr << "Error: mismatch in variable and response set lengths in "
	 << "ApproximationInterface::append_approximation()." << std::endl;
    abort_handler(-1);
  }
  // append multiple points to SurrogateData::{vars,resp}Data
  IntRespMCIter r_it;
  if (actualModelCache) {
    PRPCacheCIter p_it; size_t num_cv = samples.numRows();
    for (i=0, r_it=resp_map.begin(); i<num_pts; ++i, ++r_it) {
      // allVariables/allResponses are not sufficiently persistent for use in
      // shallow copies.  Therefore, use ordered id lookup in global PRPCache.
      p_it = cache_lookup(samples[i], num_cv, r_it->first, r_it->second);
      if (p_it == data_pairs.end()) // deep response copies with vars sharing
	mixed_add(samples[i], *r_it, false);
      else                          // shallow copies of cached vars/resp data
	shallow_add(p_it->variables(), p_it->response_pair(), false);
    }
  }
  else                            // deep response copies with vars sharing
    for (i=0, r_it=resp_map.begin(); i<num_pts; ++i, ++r_it)
      mixed_add(samples[i], *r_it, false);

  update_pop_counts(resp_map);

  // reset active approxData key using SharedApproxData::activeKey
  restore_data_key();
}


/** This function appends to each Approximation::currentPoints with
    multiple incoming variables/response data points. */
void ApproximationInterface::
append_approximation(const VariablesArray& vars_array,
		     const IntResponseMap& resp_map)
{
  size_t i, num_pts = resp_map.size();
  if (vars_array.size() != num_pts) {
    Cerr << "Error: mismatch in variable and response set lengths in "
	 << "ApproximationInterface::append_approximation()." << std::endl;
    abort_handler(-1);
  }
  // append multiple points to SurrogateData::{vars,resp}Data
  IntRespMCIter r_it;
  if (actualModelCache) {
    PRPCacheCIter p_it;
    for (i=0, r_it=resp_map.begin(); i<num_pts; ++i, ++r_it) {
      // allVariables/allResponses are not sufficiently persistent for use in
      // shallow copies.  Therefore, use ordered id lookup in global PRPCache.
      p_it = cache_lookup(vars_array[i], r_it->first, r_it->second);
      if (p_it == data_pairs.end()) // deep response copies with vars sharing
	mixed_add(vars_array[i], *r_it, false);
      else                          // shallow copies of cached vars/resp data
	shallow_add(p_it->variables(), p_it->response_pair(), false);
    }
  }
  else                            // deep response copies with vars sharing
    for (i=0, r_it=resp_map.begin(); i<num_pts; ++i, ++r_it)
      mixed_add(vars_array[i], *r_it, false);

  update_pop_counts(resp_map);

  // reset active approxData key using SharedApproxData::activeKey
  restore_data_key();
}


/** This function appends to each Approximation::currentPoints with
    multiple incoming variables/response data points. */
void ApproximationInterface::
append_approximation(const IntVariablesMap& vars_map,
		     const IntResponseMap&  resp_map)
{
  size_t i, num_pts = resp_map.size();
  if (vars_map.size() != num_pts) {
    Cerr << "Error: mismatch in variable and response set lengths in "
	 << "ApproximationInterface::append_approximation()." << std::endl;
    abort_handler(-1);
  }
  // append multiple points to SurrogateData::{vars,resp}Data
  IntVarsMCIter v_it; IntRespMCIter r_it; int eval_id;
  if (actualModelCache) {
    PRPCacheCIter p_it;
    for (v_it =vars_map.begin(), r_it =resp_map.begin();
	 v_it!=vars_map.end() && r_it!=resp_map.end(); ++v_it, ++r_it) {
      // allVariables/allResponses are not sufficiently persistent for use in
      // shallow copies.  Therefore, use ordered id lookup in global PRPCache.
      eval_id = r_it->first;
      check_id(v_it->first, eval_id);
      p_it = cache_lookup(v_it->second, eval_id, r_it->second);
      if (p_it == data_pairs.end()) // deep response copies with vars sharing
	mixed_add(v_it->second, *r_it, false);
      else                          // shallow copies of cached vars/resp data
	shallow_add(p_it->variables(), p_it->response_pair(), false);
    }
  }
  else                            // deep response copies with vars sharing
    for (v_it =vars_map.begin(), r_it =resp_map.begin();
	 v_it!=vars_map.end() && r_it!=resp_map.end(); ++v_it, ++r_it) {
      check_id(v_it->first, r_it->first);
      mixed_add(v_it->second, *r_it, false);
    }

  update_pop_counts(resp_map);

  // reset active approxData key using SharedApproxData::activeKey
  restore_data_key();
}


void ApproximationInterface::
replace_approximation(const IntResponsePair& response_pr)
{
  size_t fn_index;
  for (StSIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    fn_index = *it;
    functionSurfaces[fn_index].replace(response_pr, fn_index); // --> active approxData
  }
}


void ApproximationInterface::
replace_approximation(const IntResponseMap& resp_map)
{
  size_t fn_index;
  for (StSIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    fn_index = *it;
    Approximation& fn_surf = functionSurfaces[fn_index];
    for (IntRespMCIter r_it=resp_map.begin(); r_it!=resp_map.end(); ++r_it)
      fn_surf.replace(*r_it, fn_index); // --> active approxData
  }
}


/** This function finds the coefficients for each Approximation based
    on the data passed through update_approximation() calls.  The
    bounds are used only for graphics visualization. */
void ApproximationInterface::
build_approximation(const RealVector&  c_l_bnds, const RealVector&  c_u_bnds,
		    const IntVector&  di_l_bnds, const IntVector&  di_u_bnds,
		    const RealVector& dr_l_bnds, const RealVector& dr_u_bnds)
{
  // initialize the data shared among approximation instances
  sharedData.set_bounds(c_l_bnds, c_u_bnds, di_l_bnds, di_u_bnds,
			dr_l_bnds, dr_u_bnds);
  sharedData.build();
  // build the approximation surface instances
  for (StSIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    size_t fn_index = *it;
    // construct the approximation
    functionSurfaces[fn_index].build();

    // manage diagnostics
    if (functionSurfaces[fn_index].diagnostics_available()) {
      // print default or user-requested metrics and cross-validation
      functionSurfaces[fn_index].primary_diagnostics(fn_index);
      // for user-provided challenge data, we assume there are
      // function values for all functions in the analysis, not just
      // the indices for which surrogates are being built
      
      // BMA TODO: can this move to ctor?
      if (!challengeFile.empty()) {
        if (challengePoints.empty())
          read_challenge_points();
        functionSurfaces[fn_index].challenge_diagnostics(fn_index,
	  challengePoints, Teuchos::getCol(Teuchos::View, challengeResponses,
					   (int)fn_index));
      }
    }
  }

  /* Old 3D graphics capability:
  size_t fn_index = *approxFnIndices.begin();
  // if graphics is on for 2 variables, plot first functionSurface in 3D
  if (graph3DFlag && sharedData.num_variables() == 2) {
    functionSurfaces[fn_index].draw_surface();
  }
  */
}


/** This function calls export on each approximation */
void ApproximationInterface::export_approximation()
{
  for (StSIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it)
    functionSurfaces[*it].export_model();
}
   

/** This function updates the coefficients for each Approximation based
    on data increments provided by {update,append}_approximation(). */
void ApproximationInterface::rebuild_approximation(const BitArray& rebuild_fns)
{
  // rebuild data shared among approximation instances
  sharedData.rebuild();
  // rebuild the approximation surfaces
  for (StSIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    size_t fn_index = *it;
    // check for rebuild request (defaults to true if no deque defined)
    if (rebuild_fns.empty() || rebuild_fns[fn_index]) {
      // approx bounds not updated as in build_approximation()

      // invokes increment_coefficients()
      functionSurfaces[fn_index].rebuild();

      // diagnostics not currently active on rebuild
    }
  }
}


PRPCacheCIter ApproximationInterface::
cache_lookup(const Variables& vars, int eval_id, const Response& response)
{
  PRPCacheCIter p_it;
  IntStringPair ids(eval_id, actualModelInterfaceId);
  // sign indicates dataset source (see DataFitSurrModel::build_global()):
  //   eval id > 0 for unique evals from current execution (in data_pairs)
  //   eval id = 0 for evals from file import (not in data_pairs)
  //   eval id < 0 for non-unique evals from restart (in data_pairs)
  if (eval_id > 0) // unique evals: current run
    p_it = lookup_by_ids(data_pairs, ids);
  else { // nonunique eval ids from restart/file import
    // rather than resorting to lookup_by_val(), use a two-pass approach
    // to process multiple returns from equal_range(search_ids)
    if (actualModelInterfaceId.empty()) {
      ParamResponsePair search_pr(vars, "NO_ID", response);
      p_it = lookup_by_ids(data_pairs, ids, search_pr);
    }
    else {
      ParamResponsePair search_pr(vars, actualModelInterfaceId, response);
      p_it = lookup_by_ids(data_pairs, ids, search_pr);
    }
  }
  return p_it;
}


PRPCacheCIter ApproximationInterface::
cache_lookup(const Real* vars, size_t num_v, int eval_id,
	     const Response& response)
{
  PRPCacheCIter p_it;
  IntStringPair ids(eval_id, actualModelInterfaceId);
  // sign indicates dataset source (see DataFitSurrModel::build_global()):
  //   eval id > 0 for unique evals from current execution (in data_pairs)
  //   eval id = 0 for evals from file import (not in data_pairs)
  //   eval id < 0 for non-unique evals from restart (in data_pairs)
  if (eval_id > 0) // unique evals: current run
    p_it = lookup_by_ids(data_pairs, ids);
  else { // nonunique eval ids from restart/file import
    // rather than resorting to lookup_by_val(), use a two-pass approach
    // to process multiple returns from equal_range(search_ids)
    sample_to_variables(vars, num_v, actualModelVars);
    if (actualModelInterfaceId.empty()) {
      ParamResponsePair search_pr(actualModelVars, "NO_ID", response);
      p_it = lookup_by_ids(data_pairs, ids, search_pr);
    }
    else {
      ParamResponsePair search_pr(actualModelVars, actualModelInterfaceId,
				  response);
      p_it = lookup_by_ids(data_pairs, ids, search_pr);
    }
  }
  return p_it;
}


void ApproximationInterface::
mixed_add(const Variables& vars, const IntResponsePair& response_pr,
	  bool anchor)
{
  int           eval_id = (trackEvalIds) ? response_pr.first : INT_MAX;
  const Response&  resp = response_pr.second;
  const ShortArray& asv = resp.active_set_request_vector();
  size_t i, fn_index, num_fns = functionSurfaces.size(),
    num_asv = asv.size(), qoi_set, key_index;
  Pecos::SurrogateDataVars sdv; bool first_vars = true;
  for (StSIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    fn_index = *it;
    Approximation& fn_surf = functionSurfaces[fn_index];
    // asv may be larger than num_fns due to response aggregation modes
    // (e.g., multifidelity) --> use num_fns as stride and support add()
    // to vector of SurrogateData
    for (i=fn_index, qoi_set=0; i<num_asv; i+=num_fns, ++qoi_set)
      if (asv[i]) {
	// mapping required for advanced cases, such as recursive emulation
	qoi_set_to_key_index(qoi_set, key_index);

	// rather than unrolling the response (containing all response fns)
	// into per-response function arrays for input to fn_surf, pass the
	// complete response along with a response function index.
	if (first_vars) { // vars,resp copy = deep,deep
	  fn_surf.add(vars, true, resp, i, true, anchor, eval_id, key_index);
	  // carry newly added sdv over to other approx fn indices:
	  sdv = (anchor) ? fn_surf.surrogate_data().anchor_variables() :
	                   fn_surf.surrogate_data().variables_data().back();
	  first_vars = false;
	}
	else // vars,resp copy = shallow,deep
	  fn_surf.add(sdv, false, resp, i, true, anchor, eval_id, key_index);
      }
  }
}


void ApproximationInterface::
mixed_add(const Real* c_vars, const IntResponsePair& response_pr, bool anchor)
{
  int           eval_id = (trackEvalIds) ? response_pr.first : INT_MAX;
  const Response&  resp = response_pr.second;
  const ShortArray& asv = resp.active_set_request_vector();
  size_t i, fn_index, num_fns = functionSurfaces.size(),
    num_asv = asv.size(), qoi_set, key_index;
  Pecos::SurrogateDataVars sdv; bool first_vars = true;
  for (StSIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    fn_index = *it;
    Approximation& fn_surf = functionSurfaces[fn_index];
    // asv may be larger than num_fns due to response aggregation modes
    // (e.g., multifidelity) --> use num_fns as stride and support add()
    // to vector of SurrogateData
    for (i=fn_index, qoi_set=0; i<num_asv; i+=num_fns, ++qoi_set)

      if (asv[i]) {
	// mapping required for advanced cases, such as recursive emulation
	qoi_set_to_key_index(qoi_set, key_index);

	// rather than unrolling the response (containing all response fns)
	// into per-response function arrays for input to fn_surf, pass the
	// complete response along with a response function index.
	if (first_vars) { // vars,resp copy = deep,deep
	  fn_surf.add(c_vars, true, resp, i, true, anchor, eval_id, key_index);
	  // carry newly added sdv over to other approx fn indices:
	  sdv = (anchor) ? fn_surf.surrogate_data().anchor_variables() :
	                   fn_surf.surrogate_data().variables_data().back();
	  first_vars = false;
	}
	else // vars,resp copy = shallow,deep
	  fn_surf.add(sdv, false, resp, i, true, anchor, eval_id, key_index);
      }
  }
}


void ApproximationInterface::
shallow_add(const Variables& vars, const IntResponsePair& response_pr,
	    bool anchor)
{
  int           eval_id = (trackEvalIds) ? response_pr.first : INT_MAX;
  const Response&  resp = response_pr.second;
  const ShortArray& asv = resp.active_set_request_vector();
  size_t i, fn_index, num_fns = functionSurfaces.size(),
    num_asv = asv.size(), qoi_set, key_index;
  for (StSIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    fn_index = *it;
    Approximation& fn_surf = functionSurfaces[fn_index];
    // asv may be larger than num_fns due to response aggregation modes
    // (e.g., multifidelity) --> use num_fns as stride and support add()
    // to vector of SurrogateData
    for (i=fn_index, qoi_set=0; i<num_asv; i+=num_fns, ++qoi_set)
      if (asv[i]) { // vars,resp copy = shallow,shallow
	// mapping required for advanced cases, such as recursive emulation
	qoi_set_to_key_index(qoi_set, key_index);
	fn_surf.add(vars, false, resp, i, false, anchor, eval_id, key_index);
      }
  }
}


void ApproximationInterface::
update_pop_counts(const IntResponsePair& response_pr)
{
  // update pop counts for data in response_pr
  const ShortArray& asv = response_pr.second.active_set_request_vector();
  size_t i, qoi_set, key_index, fn_index, num_fns = functionSurfaces.size(),
    num_asv = asv.size(), count;
  StSIter fn_it;
  for (fn_it=approxFnIndices.begin(); fn_it!=approxFnIndices.end(); ++fn_it) {
    fn_index = *fn_it;
    // asv may be larger than num_fns due to response aggregation modes
    for (i=fn_index, qoi_set=0; i<num_asv; i+=num_fns, ++qoi_set) {
      count = (asv[i]) ? 1 : 0; // either one or no pts appended
      qoi_set_to_key_index(qoi_set, key_index);
      functionSurfaces[fn_index].pop_count(count, key_index);
    }
  }
}


void ApproximationInterface::update_pop_counts(const IntResponseMap& resp_map)
{
  StSIter fn_it; IntRespMCIter r_it = resp_map.begin();
  size_t i, count, qoi_set, key_index, fn_index,
    num_fns = functionSurfaces.size(),
    num_asv = r_it->second.active_set_request_vector().size();
  for (fn_it=approxFnIndices.begin(); fn_it!=approxFnIndices.end(); ++fn_it) {
    fn_index = *fn_it;
    // asv may be larger than num_fns due to response aggregation modes
    for (i=fn_index, qoi_set=0; i<num_asv; i+=num_fns, ++qoi_set) {
      for (r_it=resp_map.begin(), count=0; r_it!=resp_map.end(); ++r_it)
	if (r_it->second.active_set_request_vector()[i])
	  ++count;
      qoi_set_to_key_index(qoi_set, key_index);
      functionSurfaces[fn_index].pop_count(count, key_index);
    }
  }
}


Real2DArray ApproximationInterface::
cv_diagnostics(const StringArray& metric_types, unsigned num_folds)
{
  Real2DArray cv_diags;
  StSIter a_it = approxFnIndices.begin(), a_end = approxFnIndices.end();
  for ( ; a_it != a_end; ++a_it) {
    size_t index = *a_it;
    cv_diags.push_back(functionSurfaces[index].
		       cv_diagnostic(metric_types, num_folds));
  }
  return cv_diags;
}


Real2DArray ApproximationInterface::
challenge_diagnostics(const StringArray& metric_types, 
		      const RealMatrix& challenge_pts,
                      const RealVector& challenge_resps)
{
  Real2DArray chall_diags;
  StSIter a_it = approxFnIndices.begin(), a_end = approxFnIndices.end();
  for ( ; a_it != a_end; ++a_it) {
    size_t index = *a_it;
    chall_diags.push_back(functionSurfaces[index].
			  challenge_diagnostic(metric_types, challenge_pts,
                                               challenge_resps));
  }
  return chall_diags;
}



const RealVectorArray& ApproximationInterface::
approximation_coefficients(bool normalized)
{
  // only assign the functionSurfaceCoeffs array if it's requested
  // (i.e., do it here rather than in build/update functions above).
  if (functionSurfaceCoeffs.empty())
    functionSurfaceCoeffs.resize(functionSurfaces.size());
  for (StSIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    size_t index = *it;
    functionSurfaceCoeffs[index]
      = functionSurfaces[index].approximation_coefficients(normalized);
  }
  return functionSurfaceCoeffs;
}


void ApproximationInterface::
approximation_coefficients(const RealVectorArray& approx_coeffs,
			   bool normalized)
{
  for (StSIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    size_t index = *it;
    functionSurfaces[index].approximation_coefficients(approx_coeffs[index],
						       normalized);
  }
  //functionSurfaceCoeffs = approx_coeffs;
}


const RealVector& ApproximationInterface::
approximation_variances(const Variables& vars)
{
  // only assign the functionSurfaceVariances array if it's requested
  // (i.e., do it here rather than in build/update functions above).
  if (functionSurfaceVariances.empty())
    functionSurfaceVariances.sizeUninitialized(functionSurfaces.size());
  for (StSIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    size_t index = *it;
    functionSurfaceVariances[index]
      = functionSurfaces[index].prediction_variance(vars);
  }
  return functionSurfaceVariances;
}


// TODO: What does it even mean to challenge at index or string
// data?!?  Is a Response object available too?
/** Challenge data defaults to active/inactive, but user can override
    to active only.  */
void ApproximationInterface::read_challenge_points()
{
  size_t num_fns = functionSurfaces.size();
  String context_msg = "Surrogate model, interface id '" + interface_id() +
    "' import_challenge_points_file";
  bool verbose = (outputLevel > NORMAL_OUTPUT);
  // use a Variables object to easily read active vs. all
  TabularIO::read_data_tabular(challengeFile, context_msg,
			       actualModelVars.copy(), num_fns, challengePoints,
                               challengeResponses, challengeFormat,
			       verbose, challengeUseVarLabels,
			       challengeActiveOnly);
}

} // namespace Dakota
