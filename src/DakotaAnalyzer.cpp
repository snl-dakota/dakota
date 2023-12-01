/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <stdexcept>
#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"
#include "RecastModel.hpp"
#include "DakotaAnalyzer.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "IteratorScheduler.hpp"
#include "PRPMultiIndex.hpp"

static const char rcsId[]="@(#) $Id: DakotaAnalyzer.cpp 7035 2010-10-22 21:45:39Z mseldre $";

//#define DEBUG

namespace Dakota {

extern PRPCache data_pairs;


Analyzer::
Analyzer(ProblemDescDB& problem_db, Model& model):
  Iterator(BaseConstructor(), problem_db), compactMode(true),
  numObjFns(0), numLSqTerms(0), // default: no best data tracking
  vbdFlag(problem_db.get_bool("method.variance_based_decomp")),
  writePrecision(problem_db.get_int("environment.output_precision"))
{
  // set_db_list_nodes() is set by a higher context
  iteratedModel = model;
  update_from_model(iteratedModel); // variable/response counts & checks

  // assign context-specific defaults
  if (convergenceTol < 0.) convergenceTol = 1.e-4; // historical default
  //if (maxIterations    == SZ_MAX) maxIterations    = 100;
  //if (maxFunctionEvals == SZ_MAX) maxFunctionEvals = 1000;

  if (model.primary_fn_type() == OBJECTIVE_FNS)
    numObjFns = model.num_primary_fns();
  else if (model.primary_fn_type() == CALIB_TERMS)
    numLSqTerms = model.num_primary_fns();
  else if (model.primary_fn_type() != GENERIC_FNS) {
    Cerr << "\nError: Unknown primary function type in Analyzer." << std::endl;
    abort_handler(METHOD_ERROR);
  }
  
  if (vbdFlag) 
    vbdDropTol = probDescDB.get_real("method.vbd_drop_tolerance");

  if (!numFinalSolutions)  // default is zero
    numFinalSolutions = 1; // iterator-specific default assignment
}


Analyzer::
Analyzer(unsigned short method_name, Model& model):
  Iterator(NoDBBaseConstructor(), method_name, model), compactMode(true),
  numObjFns(0), numLSqTerms(0), // default: no best data tracking
  vbdFlag(false), vbdDropTol(-1.),
  writePrecision(0)
{
  update_from_model(iteratedModel); // variable/response counts & checks
}


Analyzer::
Analyzer(unsigned short method_name, Model& model,
	 const ShortShortPair& view_override):
  Iterator(NoDBBaseConstructor(), method_name, model), compactMode(true),
  numObjFns(0), numLSqTerms(0), // default: no best data tracking
  writePrecision(0)
{
  if (view_override != iteratedModel.current_variables().view())
    recast_model_view(view_override);
  update_from_model(iteratedModel); // variable/response counts & checks
}


Analyzer::Analyzer(unsigned short method_name):
  Iterator(NoDBBaseConstructor(), method_name), compactMode(true),
  numObjFns(0), numLSqTerms(0), // default: no best data tracking
  writePrecision(0)
{ }


bool Analyzer::resize()
{
  bool parent_reinit_comms = Iterator::resize();

  numContinuousVars     = iteratedModel.cv();
  numDiscreteIntVars    = iteratedModel.div();
  numDiscreteStringVars = iteratedModel.dsv();
  numDiscreteRealVars   = iteratedModel.drv();
  numFunctions          = iteratedModel.response_size();

  return parent_reinit_comms;
}


void Analyzer::recast_model_view(const ShortShortPair& view_override)
{
  iteratedModel.assign_rep(
    std::make_shared<RecastModel>(iteratedModel, view_override));
}


void Analyzer::update_from_model(const Model& model)
{
  Iterator::update_from_model(model);

  numContinuousVars     = model.cv();  numDiscreteIntVars  = model.div();
  numDiscreteStringVars = model.dsv(); numDiscreteRealVars = model.drv();
  numFunctions          = model.response_size();

  bool err_flag = false;
  // Check for correct bit associated within methodName
  if ( !(methodName & ANALYZER_BIT) ) {
    Cerr << "\nError: analyzer bit not activated for method instantiation "
	 << "(case " << methodName << ") within Analyzer branch." << std::endl;
    err_flag = true;
  }
  // Check for active design variables and discrete variable support
  if (methodName == CENTERED_PARAMETER_STUDY ||
      methodName == LIST_PARAMETER_STUDY     ||
      methodName == MULTIDIM_PARAMETER_STUDY ||
      methodName == VECTOR_PARAMETER_STUDY   || methodName == RANDOM_SAMPLING ||
      methodName == GLOBAL_INTERVAL_EST      || methodName == GLOBAL_EVIDENCE ||
      methodName == ADAPTIVE_SAMPLING ) {
    if (!numContinuousVars && !numDiscreteIntVars && !numDiscreteStringVars &&
	!numDiscreteRealVars) {
      Cerr << "\nError: " << method_enum_to_string(methodName)
	   << " requires active variables." << std::endl;
      err_flag = true;
    }
  }
  else { // methods supporting only continuous design variables
    if (!numContinuousVars) {
      Cerr << "\nError: " << method_enum_to_string(methodName)
	   << " requires active continuous variables." << std::endl;
      err_flag = true;
    }
    if (numDiscreteIntVars || numDiscreteStringVars || numDiscreteRealVars)
      Cerr << "\nWarning: active discrete variables ignored by "
	   << method_enum_to_string(methodName) << std::endl;
  }
  // Check for response functions
  if ( numFunctions <= 0 ) {
    Cerr << "\nError: number of response functions must be greater than zero."
	 << std::endl;
    err_flag = true;
  }

  if (err_flag)
    abort_handler(METHOD_ERROR);
}


void Analyzer::initialize_run()
{
  // Verify that iteratedModel is not null (default ctor and some
  // NoDBBaseConstructor ctors leave iteratedModel uninitialized).
  if (!iteratedModel.is_null()) {
    // update context data that is outside scope of local DB specifications.
    // This is needed for reused objects.
    //iteratedModel.db_scope_reset(); // TO DO: need better name?

    // This is to catch un-initialized models used by local iterators that
    // are not called through IteratorScheduler::run_iterator().  Within a
    // recursion, it will correspond to the first initialize_run() with an
    // uninitialized mapping, such as the outer-iterator on the first pass
    // of a recursion.  On subsequent passes, it may correspond to the inner
    // iterator.  The Iterator scope should not matter for the iteratedModel
    // mapping initialize/finalize.
    if (!iteratedModel.mapping_initialized()) {
      ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator();
      bool var_size_changed = iteratedModel.initialize_mapping(pl_iter);
      if (var_size_changed)
        /*bool reinit_comms =*/ resize(); // Ignore return value
    }

    // Do not reset the evaluation reference for sub-iterators
    // (previously managed via presence/absence of ostream)
    //if (!subIteratorFlag)
    if (summaryOutputFlag)
      iteratedModel.set_evaluation_reference();
  }
}


void Analyzer::pre_run()
{ bestVarsRespMap.clear(); }


void Analyzer::post_run(std::ostream& s)
{
  if (summaryOutputFlag) {
    // Print the function evaluation summary for all Iterators
    if (!iteratedModel.is_null())
      iteratedModel.print_evaluation_summary(s); // full hdr, relative counts

    // The remaining final results output varies by iterator branch
    print_results(s);
  }
}


void Analyzer::finalize_run()
{
  // Finalize an initialized mapping.  This will correspond to the first
  // finalize_run() with an uninitialized mapping, such as the inner-iterator
  // in a recursion.
  if (!iteratedModel.is_null() && iteratedModel.mapping_initialized()) {
    bool var_size_changed = iteratedModel.finalize_mapping();
    if (var_size_changed)
      /*bool reinit_comms =*/ resize(); // Ignore return value
  }
}


/** Convenience function for derived classes with sets of function
    evaluations to perform (e.g., NonDSampling, DDACEDesignCompExp,
    FSUDesignCompExp, ParamStudy). */
void Analyzer::
evaluate_parameter_sets(Model& model, bool log_resp_flag, bool log_best_flag)
{
  // This protected fn is only called by derived classes / letter instances

  // allVariables or allSamples defines the set of fn evals to be performed
  size_t i, num_evals
    = (compactMode) ? allSamples.numCols() : allVariables.size();
  bool header_flag = (allHeaders.size() == num_evals),
       asynch_flag = model.asynch_flag();

  if (!asynch_flag && log_resp_flag) allResponses.clear();

  // Loop over parameter sets and compute responses.  Collect data
  // and track best evaluations based on flags.
  for (i=0; i<num_evals; i++) {
    // output the evaluation header (if present)
    if (header_flag) Cout << allHeaders[i];

    if (compactMode) update_model_from_sample(model,    allSamples[i]);
    else             update_model_from_variables(model, allVariables[i]);

    // compute the response
    if (asynch_flag)
      model.evaluate_nowait(activeSet);
    else {
      model.evaluate(activeSet);
      log_response(model, allResponses, i, log_resp_flag, log_best_flag);
    }
    archive_model_variables(model, i);
  }

  // synchronize a non-blocking evaluation of all{Samples,Variables}
  // Note: multiple batches can be synchronized using evaluate_batch()
  // and synchronize_batches()
  if (asynch_flag) {
    const IntResponseMap& resp_map = model.synchronize();
    if (log_resp_flag) // log response data
      allResponses = resp_map;
    if (compactMode) log_response_map(allSamples,   resp_map, log_best_flag);
    else             log_response_map(allVariables, resp_map, log_best_flag);
  }
}


void Analyzer::evaluate_batch(Model& model, int batch_id, bool log_best_flag)
{
  // This function does not need an iteratorRep fwd because it is a
  // protected fn only called by letter classes.

  // allVariables or allSamples defines the set of fn evals to be performed
  size_t i, num_cv, batch_size;
  bool asynch_flag = model.asynch_flag();
  IntResponse2DMap::iterator r2_it;
  IntVariables2DMap::iterator v2_it;  IntRealVector2DMap::iterator rv2_it;
  if (!asynch_flag) {
    std::pair<int, IntResponseMap> resp_map_pr(batch_id, IntResponseMap());
    std::pair<IntResponse2DMap::iterator, bool> rtn_pr
      = batchResponsesMap.insert(resp_map_pr);
    r2_it = rtn_pr.first;
    if (!rtn_pr.second) // not inserted since batch_id already present
      r2_it->second.clear();
  }
  if (compactMode) {
    std::pair<int, IntRealVectorMap> map_pr(batch_id, IntRealVectorMap());
    std::pair<IntRealVector2DMap::iterator, bool> rtn_pr
      = batchSamplesMap.insert(map_pr);
    rv2_it = rtn_pr.first;
    if (!rtn_pr.second) // not inserted since batch_id already present
      rv2_it->second.clear();
    num_cv = allSamples.numRows();  batch_size = allSamples.numCols();
  }
  else {
    std::pair<int,  IntVariablesMap> map_pr(batch_id,  IntVariablesMap());
    std::pair<IntVariables2DMap::iterator, bool> rtn_pr
      = batchVariablesMap.insert(map_pr);
    v2_it = rtn_pr.first;
    if (!rtn_pr.second) // not inserted since batch_id already present
      v2_it->second.clear();
    batch_size = allVariables.size();
  }
  bool header_flag = (allHeaders.size() == batch_size);

  // Loop over parameter sets and compute responses.  Collect data
  // and track best evaluations based on flags.
  for (i=0; i<batch_size; i++) {
    // output the evaluation header (if present)
    if (header_flag) Cout << allHeaders[i];

    if (compactMode) update_model_from_sample(model,    allSamples[i]);
    else             update_model_from_variables(model, allVariables[i]);
    archive_model_variables(model, i);

    if (asynch_flag)
      model.evaluate_nowait(activeSet);
    else {
      model.evaluate(activeSet);
      log_response(model, r2_it->second, i, true, log_best_flag);
    }

    // transfer all{Samples,Variables} into keyed batch{Samples,Responses}Map
    int eval_id = model.evaluation_id();
    if (compactMode)
      copy_data(allSamples[i], num_cv, rv2_it->second[eval_id]);
    else
      v2_it->second[eval_id] = allVariables[i]; // *** shallow copy is safe unless allVariables gets updated in place
  }
}


const IntResponse2DMap& Analyzer::
synchronize_batches(Model& model, bool log_best_flag)
{
  // synchronize all batches at once

  const IntResponseMap& full_resp_map = model.synchronize();
  batchResponsesMap.clear();
  //size_t i, num_batches = (compactMode) ?
  //  batchSamplesMap.size() : batchVariablesMap.size();
  int batch_id, first_id, last_id;

  // for each batch id, extract eval_ids from full response map
  if (compactMode)
    for (IntRealVector2DMap::iterator s_it=batchSamplesMap.begin();
	 s_it!=batchSamplesMap.end(); ++s_it) {
      batch_id = s_it->first;
      IntRealVectorMap&       rv_map = s_it->second;
      IntResponseMap& batch_resp_map = batchResponsesMap[batch_id];
      // Copy one by one:
      //for (rv_it=rv_map.begin(); rv_it!=rv_map.end(); ++rv_it) {
      //  eval_id = rv_it->first;
      //  batch_resp_map[eval_id] = full_resp_map[eval_id];
      //}
      // Range copy: batches are well ordered in eval_id's
      first_id = rv_map.begin()->first; last_id = (--rv_map.end())->first;
      batch_resp_map.insert(full_resp_map.find(first_id), // start (include)
			  ++full_resp_map.find(last_id)); // stop  (omit)
      log_response_map(rv_map, batch_resp_map, log_best_flag);      
    }
  else {
    for (IntVariables2DMap::iterator v_it=batchVariablesMap.begin();
	 v_it!=batchVariablesMap.end(); ++v_it){
      batch_id = v_it->first;
      IntVariablesMap&      vars_map = v_it->second;
      IntResponseMap& batch_resp_map = batchResponsesMap[batch_id];
      first_id = vars_map.begin()->first; last_id = (--vars_map.end())->first;
      batch_resp_map.insert(full_resp_map.find(first_id), // start (include)
			  ++full_resp_map.find(last_id)); // stop  (omit)
      log_response_map(vars_map, batch_resp_map, log_best_flag);      
    }
  }
  return batchResponsesMap;
}


void Analyzer::update_model_from_variables(Model& model, const Variables& vars)
{
  // default implementation is sufficient in current uses, but could
  // be overridden in future cases where a view discrepancy can exist
  // between model and vars.
  model.active_variables(vars);
}

// ***************************************************
// MSE TO DO: generalize for all active variable types
// NonDSampling still overrrides in the case of samplingVarsMode != active view
// ***************************************************

void Analyzer::update_model_from_sample(Model& model, const Real* sample_vars)
{
  // default implementation is sufficient for FSUDesignCompExp and
  // NonD{Quadrature,SparseGrid,Cubature}, but NonDSampling overrides.
  size_t i, num_cv = model.cv();
  for (i=0; i<num_cv; ++i)
    model.continuous_variable(sample_vars[i], i);
}


/** Default mapping that maps into continuous part of Variables only */
void Analyzer::
sample_to_variables(const Real* sample_c_vars, Variables& vars)
{
  // pack sample_matrix into vars_array
  const Variables& model_vars = iteratedModel.current_variables();
  if (vars.is_null()) // use minimal data ctor
    vars = Variables(model_vars.shared_data());
  for (size_t i=0; i<numContinuousVars; ++i)
    vars.continuous_variable(sample_c_vars[i], i); // ith row
  // BMA: this may be needed if vars wasn't initialized off the model
  vars.inactive_continuous_variables(
    model_vars.inactive_continuous_variables());
  // preserve any active discrete vars (unsupported by sample_matrix)
  size_t num_adiv = model_vars.adiv(), num_adrv = model_vars.adrv();
  if (num_adiv)
    vars.all_discrete_int_variables(model_vars.all_discrete_int_variables());
  if (num_adrv)
    vars.all_discrete_real_variables(model_vars.all_discrete_real_variables());
}


void Analyzer::
samples_to_variables_array(const RealMatrix& sample_matrix,
			   VariablesArray& vars_array)
{
  // pack sample_matrix into vars_array
  size_t i, num_samples = sample_matrix.numCols(); // #vars by #samples
  if (vars_array.size() != num_samples)
    vars_array.resize(num_samples);
  for (i=0; i<num_samples; ++i)
    sample_to_variables(sample_matrix[i], vars_array[i]);
}


/** Default implementation maps active continuous variables only */
void Analyzer::
variables_to_sample(const Variables& vars, Real* sample_c_vars)
{
  const RealVector& c_vars = vars.continuous_variables();
  for (size_t i=0; i<numContinuousVars; ++i)
    sample_c_vars[i] = c_vars[i]; // ith row of samples_matrix
}


void Analyzer::
variables_array_to_samples(const VariablesArray& vars_array,
			   RealMatrix& sample_matrix)
{
  // pack vars_array into sample_matrix
  size_t i, num_samples = vars_array.size();
  if (sample_matrix.numRows() != numContinuousVars ||
      sample_matrix.numCols() != num_samples)
    sample_matrix.reshape(numContinuousVars, num_samples); // #vars by #samples
  // populate each colum of the sample matrix (one col per sample)
  for (i=0; i<num_samples; ++i)
    variables_to_sample(vars_array[i], sample_matrix[i]);
}



/** Generate (numvars + 2)*num_samples replicate sets for VBD,
    populating allSamples( numvars, (numvars + 2)*num_samples ) */
void Analyzer::get_vbd_parameter_sets(Model& model, size_t num_samples)
{
  if (!compactMode) {
    Cerr << "\nError: get_vbd_parameter_sets requires compactMode.\n";
    abort_handler(METHOD_ERROR);
  }

  // BMA TODO: This may not be right for all LHS active/inactive
  // sampling modes, but is equivalent to previous code.
  size_t num_vars = numContinuousVars + numDiscreteIntVars + 
    numDiscreteStringVars + numDiscreteRealVars;
  size_t num_replicates = num_vars + 2;

  allSamples.shape(num_vars, (num_vars+2)*num_samples);

  // run derived sampling routine generate two initial matrices
  vary_pattern(true);

  // populate the first num_samples cols of allSamples
  RealMatrix sample_1(Teuchos::View, allSamples, num_vars, num_samples, 0, 0);
  get_parameter_sets(model, num_samples, sample_1);
  if (sample_1.numCols() != num_samples) {
    Cerr << "\nError in Analyzer::variance_based_decomp(): Expected "
	 << num_samples << " variable samples; received "
	 << sample_1.numCols() << std::endl;
    abort_handler(METHOD_ERROR);
  }
  
  // populate the second num_samples cols of allSamples
  RealMatrix sample_2(Teuchos::View, allSamples, num_vars, num_samples, 0, 
		     num_samples);
  get_parameter_sets(model, num_samples, sample_2);
  if (sample_2.numCols() != num_samples) {
    Cerr << "\nError in Analyzer::variance_based_decomp(): Expected "
	 << num_samples << " variable samples; received "
	 << sample_2.numCols() << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // one additional replicate per variable
  for (int i=0; i<num_vars; ++i) {
    int replicate_index = i+2;
    RealMatrix sample_r(Teuchos::View, allSamples, num_vars, num_samples, 0, 
			replicate_index * num_samples);
    // initialize the replicate to the second sample
    sample_r.assign(sample_2);
    // now swap in a row from the first sample
    for (int j=0; j<num_samples; ++j)
      sample_r(i, j) = sample_1(i, j);
  }
}

/** Generate tabular output with active variables (compactMode) or all
    variables with their labels and response labels, with no data.
    Variables are sequenced {cv, div, drv} */
void Analyzer::pre_output()
{
  // distinguish between defaulted pre-run and user-specified
  if (!parallelLib.command_line_user_modes())
    return;

  const String& filename = parallelLib.command_line_pre_run_output();
  if (filename.empty()) {
    if (outputLevel > QUIET_OUTPUT)
      Cout << "\nPre-run phase complete: no output requested.\n" << std::endl;
    return;
  }

  size_t num_evals = compactMode ? allSamples.numCols() : allVariables.size();
  if (num_evals == 0) {
    if (outputLevel > QUIET_OUTPUT)
      Cout << "\nPre-run phase complete: no variables to output.\n"
	   << std::endl;
    return;
  }

  std::ofstream tabular_file;
  TabularIO::open_file(tabular_file, filename, "pre-run output");

  // try to mitigate errors resulting from lack of precision in output
  // the full 17 digits might surprise users, but will reduce
  // numerical errors between pre/post phases
  // TODO: consider passing precision to helper functions instead of using global
  int save_precision;
  if (writePrecision == 0) {
    save_precision = write_precision;
    write_precision = 17;
  }

  // Write all variables in input spec ordering; always annotated.
  // When in compactMode, get the inactive variables off the Model and
  // use sample_to_variables to set the discrete variables not treated
  // by allSamples.
  unsigned short tabular_format = 
    parallelLib.program_options().pre_run_output_format();
  TabularIO::write_header_tabular(tabular_file,
				  iteratedModel.current_variables(), 
				  iteratedModel.current_response(),
				  "eval_id", "interface", tabular_format);

  tabular_file << std::setprecision(write_precision) 
	       << std::resetiosflags(std::ios::floatfield);

  Variables vars = iteratedModel.current_variables().copy();
  for (size_t eval_index = 0; eval_index < num_evals; eval_index++) {

    TabularIO::write_leading_columns(tabular_file, eval_index+1, 
				     iteratedModel.interface_id(),
				     tabular_format);
    if (compactMode) {
      // allSamples num_vars x num_evals, so each col becomes tabular file row
      // populate the active discrete variables that aren't in sample_matrix
      size_t num_vars = allSamples.numRows();
      sample_to_variables(allSamples[eval_index], vars);
      vars.write_tabular(tabular_file);
    }
    else
      allVariables[eval_index].write_tabular(tabular_file);
    // no response data, so terminate the record
    tabular_file << '\n';
  }

  tabular_file.flush();
  tabular_file.close();

  if (writePrecision == 0)
    write_precision = save_precision;
  if (outputLevel > QUIET_OUTPUT)
    Cout << "\nPre-run phase complete: variables written to tabular file "
	 << filename << ".\n" << std::endl;
}


/// read num_evals variables/responses from file
void Analyzer::read_variables_responses(int num_evals, size_t num_vars)
{
  // distinguish between defaulted post-run and user-specified
  if (!parallelLib.command_line_user_modes())
    return;

  const String& filename = parallelLib.command_line_post_run_input();
  if (filename.empty()) {
    if (outputLevel > QUIET_OUTPUT)
      Cout << "\nPost-run phase initialized: no input requested.\n" 
	   << std::endl;
    return;
  }

  if (num_evals == 0) {
    if (outputLevel > QUIET_OUTPUT)
      Cout << "\nPost-run phase initialized: zero samples specified.\n" 
	   << std::endl;
    return;
  }

  // pre/post only supports annotated; could detect
  unsigned short tabular_format = 
    parallelLib.program_options().post_run_input_format();

  // Define modelList and recastFlags to support any recastings within
  // a model recursion
  bool map_to_iter_space = iteratedModel.manage_data_recastings();

  // TO DO: validate/accommodate incoming num_vars since it may be defined
  // from a local sampling mode (see NonDSampling) that differs from active;
  // support for active discrete also varies across the post-run Iterators.
  bool active_only = true; // consistent with PStudyDACE use cases
  Variables vars(iteratedModel.current_variables().copy());
  Response  resp(iteratedModel.current_response().copy());

  PRPList import_prp_list;
  bool verbose = (outputLevel > NORMAL_OUTPUT);
  // This reader will either get the eval ID from the file, or number
  // the IDs starting from 1
  TabularIO::read_data_tabular(filename, "post-run input", vars, resp,
			       import_prp_list, tabular_format, verbose,
			       active_only);
  size_t num_imported = import_prp_list.size();
  if (num_imported < num_evals) {
    Cerr << "Error: number of imported evaluations (" << num_imported
	 << ") less than expected (" << num_evals << ")." << std::endl;
    abort_handler(METHOD_ERROR);
  }
  else if (verbose) {
    Cout << "\nRead " << num_imported << " samples from file " << filename;
    if (num_imported > num_evals)
      Cout << " of which " << num_evals << " will be used." << std::endl;
    else Cout << std::endl;
  }

  if (compactMode) allSamples.shapeUninitialized(num_vars, num_evals);
  else             allVariables.resize(num_evals);

  size_t i; PRPLIter prp_it;
  bool cache = iteratedModel.evaluation_cache(), // recurse_flag = true
     restart = iteratedModel.restart_file();     // recurse_flag = true
  Variables iter_vars; Response iter_resp;
  for (i=0, prp_it=import_prp_list.begin(); i<num_evals; ++i, ++prp_it) {

    ParamResponsePair& pr = *prp_it;

    // insert imported user-space data into evaluation cache (for consistency)
    // and restart (more likely to be useful).  Unlike DataFitSurrModel, we
    // will preserve the incoming eval id in the post-input file import case.
    if (restart) parallelLib.write_restart(pr); // preserve eval id
    if (cache)   data_pairs.insert(pr); // duplicate ids OK for PRPCache

    // manage any model recastings to promote from user-space to iterator-space
    if (map_to_iter_space)
      iteratedModel.user_space_to_iterator_space(pr.variables(), pr.response(),
						 iter_vars, iter_resp);
    else
      { iter_vars = pr.variables(); iter_resp = pr.response(); }

    // update allVariables,allSamples
    if (compactMode) variables_to_sample(iter_vars, allSamples[i]);
    else             allVariables[i] = iter_vars;
    // update allResponses (requires unique eval IDs)
    allResponses[pr.eval_id()] = iter_resp;

    // mirror any post-processing in Analyzer::evaluate_parameter_sets()
    if (numObjFns || numLSqTerms)
      update_best(iter_vars, i+1, iter_resp);
  }

  if (outputLevel > QUIET_OUTPUT)
    Cout << "\nPost-run phase initialized: variables / responses read from "
	 << "tabular\nfile " << filename << ".\n" << std::endl;
}

void Analyzer::compute_best_metrics(const Response& response,
				    std::pair<Real,Real>& metrics)
{
  size_t i, constr_offset;
  const RealVector& fn_vals = response.function_values();
  const RealVector& primary_wts
    = iteratedModel.primary_response_fn_weights();
  Real& obj_fn = metrics.second; obj_fn = 0.0;
  if (numObjFns) {
    constr_offset = numObjFns;
    if (primary_wts.empty()) {
      for (i=0; i<numObjFns; i++)
	obj_fn += fn_vals[i];
      if (numObjFns > 1)
	obj_fn /= (Real)numObjFns;
    }
    else
      for (i=0; i<numObjFns; i++)
	obj_fn += primary_wts[i] * fn_vals[i];
  }
  else if (numLSqTerms) {
    constr_offset = numLSqTerms;
    if (primary_wts.empty())
      for (i=0; i<numLSqTerms; i++)
	obj_fn += std::pow(fn_vals[i], 2);
    else
      for (i=0; i<numLSqTerms; i++)
	obj_fn += std::pow(primary_wts[i]*fn_vals[i], 2);
  }
  else // no "best" metric currently defined for generic response fns
    return;
  Real& constr_viol   = metrics.first; constr_viol = 0.0;
  size_t num_nln_ineq = iteratedModel.num_nonlinear_ineq_constraints(),
         num_nln_eq   = iteratedModel.num_nonlinear_eq_constraints();
  const RealVector& nln_ineq_lwr_bnds
    = iteratedModel.nonlinear_ineq_constraint_lower_bounds();
  const RealVector& nln_ineq_upr_bnds
    = iteratedModel.nonlinear_ineq_constraint_upper_bounds();
  const RealVector& nln_eq_targets
    = iteratedModel.nonlinear_eq_constraint_targets();
  for (i=0; i<num_nln_ineq; i++) { // ineq constraint violation (default tol=0)
    size_t index = i + constr_offset;
    Real ineq_con = fn_vals[index];
    if (ineq_con > nln_ineq_upr_bnds[i])
      constr_viol += std::pow(ineq_con - nln_ineq_upr_bnds[i],2);
    else if (ineq_con < nln_ineq_lwr_bnds[i])
      constr_viol += std::pow(nln_ineq_lwr_bnds[i] - ineq_con,2);
  }
  for (i=0; i<num_nln_eq; i++) { // eq constraint violation (default tol=0)
    size_t index = i + constr_offset + num_nln_ineq;
    Real eq_con = fn_vals[index];
    if (std::fabs(eq_con - nln_eq_targets[i]) > 0.)
      constr_viol += std::pow(eq_con - nln_eq_targets[i], 2);
  }
}


void Analyzer::
update_best(const Real* sample_c_vars, int eval_id, const Response& response)
{
  RealRealPair metrics; 
  compute_best_metrics(response, metrics);
#ifdef DEBUG
  Cout << "Best metrics: " << metrics.first << ' ' << metrics.second
       << std::endl;
#endif

  size_t num_best_map = bestVarsRespMap.size();
  if (num_best_map < numFinalSolutions) { // initialization of best map
    Variables vars = iteratedModel.current_variables().copy();
    sample_to_variables(sample_c_vars, vars); // copy sample only when needed
    Response copy_resp = response.copy();
    ParamResponsePair prp(vars, iteratedModel.interface_id(), copy_resp,
			  eval_id, false); // shallow copy since previous deep
    std::pair<RealRealPair, ParamResponsePair> new_pr(metrics, prp);
    bestVarsRespMap.insert(new_pr);
  }
  else {
    RealPairPRPMultiMap::iterator it = --bestVarsRespMap.end();
    //   Primary criterion: constraint violation must be <= stored violation
    // Secondary criterion: for equal (or zero) constraint violation, objective
    //                      must be < stored objective
    if (metrics < it->first) { // new best
      bestVarsRespMap.erase(it);
      Variables vars = iteratedModel.current_variables().copy();
      sample_to_variables(sample_c_vars, vars); // copy sample only when needed
      Response copy_resp = response.copy();
      ParamResponsePair prp(vars, iteratedModel.interface_id(), copy_resp,
			    eval_id, false); // shallow copy since previous deep
      std::pair<RealRealPair, ParamResponsePair> new_pr(metrics, prp);
      bestVarsRespMap.insert(new_pr);
    }
  }
}


void Analyzer::
update_best(const Variables& vars, int eval_id, const Response& response)
{
  RealRealPair metrics; 
  compute_best_metrics(response, metrics);
#ifdef DEBUG
  Cout << "Best metrics: " << metrics.first << ' ' << metrics.second
       << std::endl;
#endif

  size_t num_best_map = bestVarsRespMap.size();
  if (num_best_map < numFinalSolutions) { // initialization of best map
    ParamResponsePair prp(vars, iteratedModel.interface_id(),
			  response, eval_id); // deep copy
    std::pair<RealRealPair, ParamResponsePair> new_pr(metrics, prp);
    bestVarsRespMap.insert(new_pr);
  }
  else {
    RealPairPRPMultiMap::iterator it = --bestVarsRespMap.end();
    //   Primary criterion: constraint violation must be <= stored violation
    // Secondary criterion: for equal (or zero) constraint violation, objective
    //                      must be < stored objective
    if (metrics < it->first) { // new best
      bestVarsRespMap.erase(it);
      ParamResponsePair prp(vars, iteratedModel.interface_id(),
			    response, eval_id); // deep copy
      std::pair<RealRealPair, ParamResponsePair> new_pr(metrics, prp);
      bestVarsRespMap.insert(new_pr);
    }
  }
}


void Analyzer::print_results(std::ostream& s, short results_state)
{
  if (!numObjFns && !numLSqTerms) {
    s << "<<<<< Best parameters/responses are not reported for generic response_functions" << std::endl;
    return;
  }

  // -------------------------------------
  // Single and Multipoint results summary
  // -------------------------------------
  RealPairPRPMultiMap::iterator it = bestVarsRespMap.begin();
  size_t i, offset, num_fns, num_best_map = bestVarsRespMap.size();
  for (i=1; it!=bestVarsRespMap.end(); ++i, ++it) {
    const ParamResponsePair& best_pr = it->second;
    const Variables&  best_vars = best_pr.variables();
    const RealVector& best_fns  = best_pr.response().function_values();
    s << "<<<<< Best parameters          ";
    if (num_best_map > 1) s << "(set " << i << ") ";
    s << "=\n" << best_vars;
    num_fns = best_fns.length(); offset = 0;
    if (numObjFns) {
      if (numObjFns > 1) s << "<<<<< Best objective functions ";
      else               s << "<<<<< Best objective function  ";
      if (num_best_map > 1) s << "(set " << i << ") "; s << "=\n";
      write_data_partial(s, offset, numObjFns, best_fns);
      offset = numObjFns;
    }
    else if (numLSqTerms) {
      s << "<<<<< Best residual terms      ";
      if (num_best_map > 1) s << "(set " << i << ") "; s << "=\n";
      write_data_partial(s, offset, numLSqTerms, best_fns);
      offset = numLSqTerms;
    }
    if (num_fns > offset) {
      s << "<<<<< Best constraint values   ";
      if (num_best_map > 1) s << "(set " << i << ") "; s << "=\n";
      write_data_partial(s, offset, num_fns - offset, best_fns);
    }
    s << "<<<<< Best evaluation ID: " << best_pr.eval_id() << std::endl;
  }
}


void Analyzer::vary_pattern(bool pattern_flag)
{
  Cerr << "Error: Analyzer lacking redefinition of virtual vary_pattern() "
       << "function.\n       This analyzer does not support pattern variance."
       << std::endl;
  abort_handler(METHOD_ERROR);
}


void Analyzer::get_parameter_sets(Model& model)
{
  Cerr << "Error: Analyzer lacking redefinition of virtual get_parameter_sets"
       << "(1) function.\n       This analyzer does not support parameter sets."
       << std::endl;
  abort_handler(METHOD_ERROR);
}

void Analyzer::get_parameter_sets(Model& model, const size_t num_samples, 
				  RealMatrix& design_matrix)
{
  Cerr << "Error: Analyzer lacking redefinition of virtual get_parameter_sets"
       << "(3) function.\n       This analyzer does not support parameter sets."
       << std::endl;
  abort_handler(METHOD_ERROR);
}

} // namespace Dakota
