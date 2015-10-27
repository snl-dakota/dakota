/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DirectApplicInterface
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DirectApplicInterface.hpp"
#include "ParamResponsePair.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include <algorithm>

namespace Dakota {

DirectApplicInterface::
DirectApplicInterface(const ProblemDescDB& problem_db):
  ApplicationInterface(problem_db),
  iFilterName(problem_db.get_string("interface.application.input_filter")),
  oFilterName(problem_db.get_string("interface.application.output_filter")),
  gradFlag(false), hessFlag(false), numFns(0), numVars(0), numDerivVars(0),
  analysisDrivers(
    problem_db.get_sa("interface.application.analysis_drivers")),
  analysisComponents(
    problem_db.get_s2a("interface.application.analysis_components"))
{
  // "interface direct" always instantiates a TestDriverInterface, but
  // eventually support "interface plugin", which would
  // instantiate only this base class, to be replaced by the plug-in

  // Provide defaults for the registered drivers.  Derived classes,
  // e.g., TestDriverInterface, can supplement with additional options for
  // driverType, such as a tester or request different active data views

  // register the missing driver type with the empty driver name string 
  std::string empty;
  driverTypeMap[empty] = NO_DRIVER;

  // the default data view is an ordered variables vector, but derived
  // classes can override; really only want to set in plug-in case
  localDataView = VARIABLES_VECTOR;

  // initialize all drivers to none
  iFilterType = NO_DRIVER;
  analysisDriverTypes.resize(numAnalysisDrivers);
  for (size_t i=0; i<numAnalysisDrivers; ++i)
    analysisDriverTypes[i] = NO_DRIVER;
  oFilterType = NO_DRIVER;

  //if (outputLevel > NORMAL_OUTPUT)
  //  Cerr << "No specific direct interface requested; assuming subsequent "
  //       << "plug-in" << std::endl;
     
}


DirectApplicInterface::~DirectApplicInterface()
{ 
}


void DirectApplicInterface::
derived_map(const Variables& vars, const ActiveSet& set, Response& response,
	    int fn_eval_id)
{
  // Check for erroneous concurrent analysis specification:
  if (asynchLocalAnalysisFlag && evalCommRank == 0 && evalServerId == 1)
    Cerr << "Warning: multiple threads not yet supported in direct interfaces."
	 << "\n         Asynchronous analysis request will be ignored.\n";

  if (evalCommRank == 0 && !suppressOutput && outputLevel > SILENT_OUTPUT) {

    bool curly_braces = ( numAnalysisDrivers > 1 || iFilterType || oFilterType )
      ? true : false;

    // A printing-friendly (capitalized) name for the interface type
    String interface_type(interface_enum_to_string(interfaceType));
    interface_type.replace(0, 1, 1, std::toupper(*interface_type.begin()));

    if (eaDedMasterFlag)
      Cout << interface_type << " interface: self-scheduling ";
    else if (numAnalysisServers > 1)
      Cout << interface_type << " interface: static scheduling ";
    else
      Cout << interface_type << " interface: invoking ";
    if (curly_braces)
      Cout << "{ ";
    if (iFilterType)
      Cout << iFilterName << ' ';
    for (size_t i=0; i<numAnalysisDrivers; ++i)
      Cout << analysisDrivers[i] << ' ';
    if (oFilterType)
      Cout << oFilterName << ' ';
    if (curly_braces)
      Cout << "} ";
    if (numAnalysisServers > 1)
      Cout << "among " << numAnalysisServers << " analysis servers.";
    Cout << std::endl;

  }

  // Goes before input filter to set up variables data:
  // TODO: consider eliminating in some direct interfaces
  set_local_data(vars, set, response);

  // --------------------
  // Input filter portion
  // --------------------
  // Use ifilter just once per evaluation.  This provides a separate identity
  // for the ifilter: instead of just being part of each analysis process (and
  // therefore not having much justification for being separate from the
  // analysis), it is now the *non-replicated* portion of setting up the
  // analysis drivers.  For example, remeshing might be performed once per
  // evaluation and would be part of derived_map_if, whereas aprepro might be
  // used for each analysis and would be part of derived_map_ac.
  if (iFilterType && evalCommRank == 0)
    derived_map_if(iFilterName);

  // ----------------
  // Analysis portion
  // ----------------
  if (eaDedMasterFlag) { // set up master-slave
    if (evalCommRank == 0)
      master_dynamic_schedule_analyses();
    else
      serve_analyses_synch();
  }
  else { // simple static schedule (all peer cases including single analysis)
#ifdef MPI_DEBUG
    Cout << "analysisServerId = "    << analysisServerId
	 << " numAnalysisDrivers = " << numAnalysisDrivers
	 << " numAnalysisServers = " << numAnalysisServers << std::endl;
#endif // MPI_DEBUG

    // For execution of local jobs on a dedicated master (e.g., for a serial
    // value computation amongst parallel numerical gradient computations),
    // ApplicationInterface::init_serial_analyses() updates numAnalysisServers
    // from its default (0) to a serial setting (1).
    for (analysisDriverIndex =  analysisServerId-1;
	 analysisDriverIndex <  numAnalysisDrivers;
	 analysisDriverIndex += numAnalysisServers)
      derived_map_ac(analysisDrivers[analysisDriverIndex]);
    // NOTE: no synchronization enforced in static case (some procs may lag)
  }

  // ---------------------
  // Output filter portion
  // ---------------------
  // As with ifilter, use ofilter just once per evaluation to accomodate any
  // non-replicated portions of post-processing (e.g., max response, sqrt of
  // sum of squares, other reductions which don't fit the overlay concept).
  if (oFilterType) {
    // Provide synchronization if there's an oFilter and a static schedule
    // was used since evalCommRank 0 must postprocess all results (MPI_Reduce
    // provides synchronization in overlay() for the case of no oFilter).
    if (evalCommSize > 1 && !eaDedMasterFlag) // static schedule
      parallelLib.barrier_e(); // evalCommRank 0 waits for analyses to complete
    if (evalCommRank == 0)
      derived_map_of(oFilterName);
  }
  // OLD: Goes after ofilter since ofilter maps raw analysis data to
  // Response data, which is then overlaid into a combined response below.
  // NEW: in the concurrent analysis fork case, ofilter and overlay are
  // mutually exclusive, i.e., the nonreplicated operations of the ofilter
  // can be used to enact nonstandard overlays (e.g., sqrt(sum_sq)) and are
  // therefore responsible for mapping results.out.[eval#].[1->numPrograms] to
  // results.out.[eval#]
  else // mutually exclusive overlay/ofilter
    overlay_response(response); // MPI_Reduce provides synchronization
}


int DirectApplicInterface::derived_map_if(const String& if_name)
{
  int fail_code = 0;
  //if (if_name == "text_book_if") {
    //fail_code = text_book_if();
  //}
  //else {
    Cerr << if_name << " is not available as an input filter within "
         << "DirectApplicInterface." << std::endl;
    abort_handler(INTERFACE_ERROR);
  //}

  //std::map<String, driver_t>::iterator sd_iter = driverTypeMap.find(if_name);
  //driver_t if_type
  //  = (sd_iter!=driverTypeMap.end()) ? sd_iter->second : NO_DRIVER;
  //switch (if_type) {
  //case TEXT_BOOK_IF:
  //  fail_code = text_book_if(); break;
  //default:
  //  Cerr << "Bad if_type..." << std::endl;
  //  abort_handler(-1);
  //}

  // Failure capturing
    if (fail_code) {
      std::string err_msg("Error evaluating direct input filter ");
      err_msg += if_name;
      throw FunctionEvalFailure(err_msg);
    }

  return 0;
}


/** When a direct analysis/filter is a member function, the (vars,set,response)
    data does not need to be passed through the API.  If, however, non-member
    analysis/filter functions are added, then pass (vars,set,response) through
    to the non-member fns:
    \code
    // API declaration
    int sim(const Variables& vars, const ActiveSet& set, Response& response);
    // use of API within derived_map_ac()
    if (ac_name == "sim")
      fail_code = sim(directFnVars, directFnActSet, directFnResponse);
    \endcode */
int DirectApplicInterface::derived_map_ac(const String& ac_name)
{
  // NOTE: a Factory pattern might be appropriate in the future to manage the
  // conditional presence of linked subroutines in DirectApplicInterface.

#ifdef MPI_DEBUG
    Cout << "analysis server " << analysisServerId << " invoking " << ac_name
         << " within DirectApplicInterface." << std::endl;
#endif // MPI_DEBUG
  int fail_code = 0;
  Cerr << ac_name << " is not available as an analysis driver within "
       << "DirectApplicInterface." << std::endl;
  abort_handler(INTERFACE_ERROR);

  // Failure capturing
  if (fail_code)  {
    std::string err_msg("Error evaluating direct analysis_driver ");
    err_msg += ac_name;
    throw FunctionEvalFailure(err_msg);
  }
  
  return 0;
}


int DirectApplicInterface::derived_map_of(const String& of_name)
{
  int fail_code = 0;
  //if (of_name == "text_book_of") {
    //fail_code = text_book_of();
  //}
  //else {
    Cerr << of_name << " is not available as an output filter within "
         << "DirectApplicInterface." << std::endl;
    abort_handler(INTERFACE_ERROR);
  //}

  //std::map<String, driver_t>::iterator sd_iter = driverTypeMap.find(of_name);
  //driver_t of_type
  //  = (sd_iter!=driverTypeMap.end()) ? sd_iter->second : NO_DRIVER;
  //switch (of_type) {
  //case TEXT_BOOK_OF:
  //  fail_code = text_book_of(); break;
  //default:
  //  Cerr << "Bad of_type..." << std::endl;
  //  abort_handler(-1);
  //}

  // Failure capturing
  if (fail_code) {
    std::string err_msg("Error evaluating direct output filter ");
    err_msg += of_name;
    throw FunctionEvalFailure(err_msg);
  }
 
  return 0;
}


void DirectApplicInterface::derived_map_asynch(const ParamResponsePair& pair)
{
  Cerr << "Error: asynchronous capability (multiple threads) not installed in"
       << "\nDirectApplicInterface." << std::endl;
  abort_handler(-1);

  //pthread_create/thr_create(derived_map(...)) launches a new thread
  //threadIdMap[tid] = fn_eval_id;

  // Design note, 10/2013: the heavy fork() approach that already exists in
  // ProcessHandleApplicInterface::create_evaluation_process() would be a
  // quick and easy substitute for POSIX threads.  While this would work
  // nicely for managing job concurrency with code that's already in hand
  // (assuming sufficient memory for the required heavy fork() concurrency),
  // there is no simple way to get response results back to the parent
  // process from the child.  Short of resorting to the file system, there
  // are examples of using pipes to accomplish this, but this moves the
  // idea away from the state of low risk / high payoff.
}


void DirectApplicInterface::wait_local_evaluations(PRPQueue& prp_queue)
{
  Cerr << "Error: asynchronous capability (multiple threads) not installed in"
       << "\nDirectApplicInterface." << std::endl;
  abort_handler(-1);

  /*
  PRPQueueIter queue_it = lookup_by_eval_id(prp_queue, fn_eval_id);
  if (queue_it == prp_queue.end()) {
    Cerr << "Error: failure in queue lookup within DirectApplicInterface::"
	 << "wait_local_evaluations()." << std::endl;
    abort_handler(-1);
  }
  int fail_code = 0, id = queue_it->eval_id();
  Variables    vars = queue_it->variables(); // shallow copy
  Response response = queue_it->response();  // shallow copy

  // pthread_join/thr_join(target_thread, ..., status) recovers threads.
  // status provides a mechanism to return failure codes from analyses.
  // The Solaris thr_join allows completion of any thread in the set if 
  // target_thread is set to 0; unfortunately, this does not appear to be part
  // of the POSIX standard (pthread_join must complete a valid target_thread).

  // Possible better solution: look at OpenMP ??

  // For the asynch case, Direct (unlike SysCall) can manage failures w/o 
  // throwing exceptions.  See ApplicationInterface::manage_failure for notes.
  if (fail_code)
    manage_failure(vars, response.active_set(), response, id);

  Cout << "Thread for evaluation " << id << " captured.\n";
  queue_it->response(response);
  */
}


void DirectApplicInterface::test_local_evaluations(PRPQueue& prp_queue)
{
  Cerr << "Error: asynchronous capability (multiple threads) not installed in"
       << "\nDirectApplicInterface." << std::endl;
  abort_handler(-1);
}


// -----------------------------------
// Begin utilities used by derived_map
// -----------------------------------
void DirectApplicInterface::
set_local_data(const Variables& vars, const ActiveSet& set)
{
  // This function is performed once per evaluation, which may involve multiple
  // analyses.  Since the data has class scope, it has persistence from one
  // function evaluation to the next.  Old data must be zeroed, current 
  // variable values must be assigned to xC/xD, and variable and response 
  // arrays are resized (if necessary).

  // ------------------------
  // Set local variables data
  // ------------------------

  // As for ApproximationInterface::map(), it does not make sense to evaluate
  // the direct functions on an active subset of vars (for which the effect of
  // inactive vars would not be properly captured); rather, all of vars must be
  // mapped through.  This is important in particular for OUU since the inactive
  // variables are carrying data from the outer loop.
  numACV = vars.acv();
  numADIV = vars.adiv(); 
  numADRV = vars.adrv();
  numADSV = vars.adsv();
  numVars = numACV + numADIV + numADRV + numADSV;

  // Initialize copies of incoming data
  //directFnVars = vars; // shared rep
  if (localDataView & VARIABLES_MAP) {
    size_t i;
    // set labels once (all processors)
    if (xCMLabels.size()  != numACV || xDIMLabels.size() != numADIV ||
	xDRMLabels.size() != numADRV || xDSMLabels.size() != numADSV) {
      StringMultiArrayConstView acv_labels
	= vars.all_continuous_variable_labels();
      StringMultiArrayConstView adiv_labels
	= vars.all_discrete_int_variable_labels();
      StringMultiArrayConstView adrv_labels
	= vars.all_discrete_real_variable_labels();
      StringMultiArrayConstView adsv_labels
	= vars.all_discrete_string_variable_labels();
      xCMLabels.resize(numACV);
      xDIMLabels.resize(numADIV);
      xDRMLabels.resize(numADRV);
      xDSMLabels.resize(numADSV);
      //String label_i;
      // Map labels in a*v_labels to var_t enum in x*Labels through varTypeMap
      map_labels_to_enum(acv_labels,xCMLabels);
      map_labels_to_enum(adiv_labels,xDIMLabels);
      map_labels_to_enum(adrv_labels,xDRMLabels);
      map_labels_to_enum(adsv_labels,xDSMLabels);
    }
    // set variable values on every evaluation
    const RealVector& acv  = vars.all_continuous_variables();
    const IntVector&  adiv = vars.all_discrete_int_variables();
    const RealVector& adrv = vars.all_discrete_real_variables();
    StringMultiArrayConstView adsv = vars.all_discrete_string_variables();
    xCM.clear(); xDIM.clear(); xDRM.clear(); xDSM.clear(); // more rigorous than overwrite
    for (i=0; i<numACV; ++i)
      xCM[xCMLabels[i]] = acv[i];
    for (i=0; i<numADIV; ++i)
      xDIM[xDIMLabels[i]] = adiv[i];
    for (i=0; i<numADRV; ++i)
      xDRM[xDRMLabels[i]] = adrv[i];
    for (i=0; i<numADSV; ++i)
      xDSM[xDSMLabels[i]] = adsv[i];
  }
  if (localDataView & VARIABLES_VECTOR) {
    // set labels once (all processors)
    if (xCLabels.size()  != numACV || xDILabels.size() != numADIV ||
	xDRLabels.size() != numADRV || xDSLabels.size() != numADSV) {
      xCLabels.resize(boost::extents[numACV]);
      xCLabels = vars.all_continuous_variable_labels();
      xDILabels.resize(boost::extents[numADIV]);
      xDILabels = vars.all_discrete_int_variable_labels();
      xDRLabels.resize(boost::extents[numADRV]);
      xDRLabels = vars.all_discrete_real_variable_labels();
      xDSLabels.resize(boost::extents[numADSV]);
      xDSLabels = vars.all_discrete_string_variable_labels();
    }
    xC  = vars.all_continuous_variables();    // view OK
    xDI = vars.all_discrete_int_variables();  // view OK
    xDR = vars.all_discrete_real_variables(); // view OK
    xDS.resize(boost::extents[numADSV]);
    xDS = vars.all_discrete_string_variables(); // view OK
  }

  // -------------------------
  // Set local active set data
  // -------------------------
  //directFnActSet = set;                // copy
  directFnASV = set.request_vector();    // copy
  directFnDVV = set.derivative_vector(); // copy
  numDerivVars = directFnDVV.size();
  if (localDataView & VARIABLES_MAP) {
    SizetMultiArrayConstView acv_ids = vars.all_continuous_variable_ids();
    varTypeDVV.resize(numDerivVars);
    for (size_t i=0; i<numDerivVars; ++i) {
      size_t acv_index = find_index(acv_ids, directFnDVV[i]);
      if (acv_index == _NPOS) {
	Cerr << "Error: dvv value " << directFnDVV[i] << " not present in all "
	     << "continuous variable ids." << std::endl;
	abort_handler(INTERFACE_ERROR);
      }
      else
	varTypeDVV[i] = xCMLabels[acv_index];
    }
  }
}


void DirectApplicInterface::set_local_data(const Response& response)
{
  // -----------------------
  // Set local response data
  // -----------------------
  //directFnResponse = response; // shared rep
  numFns = directFnASV.size();
  gradFlag = false;
  hessFlag = false;
  size_t i;
  for (i=0; i<numFns; ++i) {
    if (directFnASV[i] & 2)
      gradFlag = true;
    if (directFnASV[i] & 4)
      hessFlag = true;
  }

  // Resize and clear required data constructs
  if (fnVals.length() != numFns)
    fnVals.resize(numFns);
  fnVals = 0.;

  if (gradFlag) {
    if (fnGrads.numCols() != numFns || fnGrads.numRows() != numDerivVars)
      fnGrads.shapeUninitialized(numDerivVars, numFns);
    fnGrads = 0.;
  }

  if (hessFlag) {
    if (fnHessians.size() != numFns) 
      fnHessians.resize(numFns);
    for (i=0; i<numFns; ++i) {
      if (fnHessians[i].numRows() != numDerivVars)
        fnHessians[i].reshape(numDerivVars);
      fnHessians[i] = 0.;
    }
  }
  // set labels once (all processors)
  if (fnLabels.empty())
    fnLabels = response.function_labels();
}


void DirectApplicInterface::overlay_response(Response& response)
{
  // Individual analysis servers are allowed to divide up the function 
  // evaluation in whatever way is convenient.  It need not be by function 
  // number (although this simple approach is used in text_book1/2/3).  The 
  // overlay_response function uses MPI_Reduce over evalAnalysisIntraComm to
  // add all contributions to the response object from analysisComm leaders.

  // If not rank 0 within an analysisComm, then nothing to contribute to the
  // total response.  Note that an evaluation dedicated master must participate
  // in the reduction (even though it contributes no response data) since it is
  // the final destination of the evaluation result.
  if (analysisCommRank)
    return;

  // set response data for analysisComm leaders (excluding master if present)
  if (analysisServerId) {
    ActiveSet set;
    set.request_vector(directFnASV);
    set.derivative_vector(directFnDVV);
    response.update(fnVals, fnGrads, fnHessians, set);
  }

  // For all master-slave cases & for peer cases in which numAnalysisServers>1,
  // response components from analysis servers must be overlaid using 
  // MPI_Reduce(..., MPI_SUM, ...).  This is performed using a mapping of 
  // response->double*->Reduce(double*)->response.
  if (numAnalysisServers > 1 || eaDedMasterFlag) {
    int num_doubles   = response.data_size();
    double* local_fns = new double [num_doubles];
    if (analysisServerId) // analysis leaders
      response.write_data(local_fns);
    else { // evaluation dedicated master (if present)
      for (size_t i=0; i<num_doubles; ++i)
        local_fns[i] = 0.0;
    }
    // sum response data over evalAnalysisIntraComm.  This is more efficient
    // than performing the reduction over evalComm since only analysisComm
    // leaders have data to reduce.  evalCommRank 0 then returns the results
    // to the iterator in ApplicationInterface::serve_evaluations().
    double* sum_fns = (evalCommRank) ? NULL : new double [num_doubles];
    parallelLib.reduce_sum_ea(local_fns, sum_fns, num_doubles);
    delete [] local_fns;
    if (evalCommRank == 0) {
      response.read_data(sum_fns);
      delete [] sum_fns;
    }
  }
}


void DirectApplicInterface::map_labels_to_enum(StringMultiArrayConstView &src, 
  std::vector<var_t> &dest) {
  // Helper to map variable labels (in src) to var_t enums (in dest); dest
  // used to set mapped variable values for use in test functions
  // See, e.g., cantilever.
  size_t num_vars = dest.size();
  std::map<String, var_t>::iterator v_iter;
  for (size_t i=0; i<num_vars; ++i) {
    //label_i = toLower(acv_labels[i]);
    v_iter = varTypeMap.find(src[i]);//(label_i);
    if (v_iter == varTypeMap.end()) {
      Cerr << "Error: label \"" << src[i]//label_i
           << "\" not supported in analysis driver." << std::endl;
      abort_handler(INTERFACE_ERROR);
    }
    else
      dest[i] = v_iter->second;
  }
}

} // namespace Dakota
