/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DirectApplicInterface
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DirectApplicInterface.H"
#include "ParamResponsePair.H"
#include "ProblemDescDB.H"
#include "ParallelLibrary.H"
//#include <unistd.h> // for sleep(int)
#ifdef DAKOTA_MODELCENTER
#include "PHXCppApi.h"
#endif
#ifdef DAKOTA_MATLAB
#include "engine.h"
#endif
#ifdef DAKOTA_PYTHON
#include <Python.h>
#ifdef DAKOTA_PYTHON_NUMPY
#include <arrayobject.h>
#endif
#endif
#include <algorithm>

namespace Dakota {

#define POW_VAL 1.0 // text_book: 1.0 is nominal, 1.4 used for B&B testing

#ifdef DAKOTA_SALINAS
/// subroutine interface to SALINAS simulation code
int salinas_main(int argc, char *argv[], MPI_Comm* comm);
#endif // DAKOTA_SALINAS

#ifdef DAKOTA_MATLAB
#ifndef MWSIZE_MAX
// Older Matlab versions used int for sizes.  Newer versions
// (7.3/R2006b and later) typedef mwSize as int or size_t in
// tmwtypes.h.  Provide definition for mwSize if necessary:
#define mwSize int
#endif
/** fields to pass to Matlab in Dakota structure */
const char *FIELD_NAMES[] = { "numFns", "numVars", "numACV", "numADIV",  // 0
			      "numADRV", "numDerivVars", "xC", "xDI",    // 4
			      "xDR", "xCLabels", "xDILabels",            // 8
			      "xDRLabels", "directFnASV", "directFnASM", // 11
			      "directFnDVV", "directFnDVV_bool",         // 14
			      "fnFlag", "gradFlag", "hessFlag",          // 16
			      "fnVals",  "fnGrads",  "fnHessians",       // 19
			      "fnLabels", "failure", "currEvalId" };     // 22
/** number of fields in above structure */
const int NUMBER_OF_FIELDS = 25;
#endif


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
  // define driverMap for string to enum mapping
  std::string empty;
  driverTypeMap[empty]                    = NO_DRIVER;
  driverTypeMap["cantilever"]             = CANTILEVER_BEAM;
  driverTypeMap["mod_cantilever"]         = MOD_CANTILEVER_BEAM;
  driverTypeMap["cyl_head"]               = CYLINDER_HEAD;
  driverTypeMap["extended_rosenbrock"]    = EXTENDED_ROSENBROCK;
  driverTypeMap["generalized_rosenbrock"] = GENERALIZED_ROSENBROCK;
  driverTypeMap["lf_rosenbrock"]          = LF_ROSENBROCK;
  driverTypeMap["rosenbrock"]             = ROSENBROCK;
  driverTypeMap["gerstner"]               = GERSTNER;
  driverTypeMap["scalable_gerstner"]      = SCALABLE_GERSTNER;
  driverTypeMap["log_ratio"]              = LOGNORMAL_RATIO;
  driverTypeMap["multimodal"]             = MULTIMODAL;
  driverTypeMap["lf_short_column"]        = LF_SHORT_COLUMN;
  driverTypeMap["short_column"]           = SHORT_COLUMN;
  driverTypeMap["side_impact_cost"]       = SIDE_IMPACT_COST;
  driverTypeMap["side_impact_perf"]       = SIDE_IMPACT_PERFORMANCE;
  driverTypeMap["sobol_rational"]         = SOBOL_RATIONAL;
  driverTypeMap["sobol_g_function"]       = SOBOL_G_FUNCTION;
  driverTypeMap["sobol_ishigami"]         = SOBOL_ISHIGAMI;
  driverTypeMap["steel_column_cost"]      = STEEL_COLUMN_COST;
  driverTypeMap["steel_column_perf"]      = STEEL_COLUMN_PERFORMANCE;
  driverTypeMap["text_book"]              = TEXT_BOOK;
  driverTypeMap["text_book1"]             = TEXT_BOOK1;
  driverTypeMap["text_book2"]             = TEXT_BOOK2;
  driverTypeMap["text_book3"]             = TEXT_BOOK3;
  driverTypeMap["text_book_ouu"]          = TEXT_BOOK_OUU;
  driverTypeMap["scalable_text_book"]     = SCALABLE_TEXT_BOOK;
  driverTypeMap["scalable_monomials"]     = SCALABLE_MONOMIALS;
  driverTypeMap["herbie"]                 = HERBIE;
  driverTypeMap["smooth_herbie"]          = SMOOTH_HERBIE;
  driverTypeMap["shubert"]                = SHUBERT;
  driverTypeMap["salinas"]                = SALINAS;
  driverTypeMap["mc_api_run"]             = MODELCENTER;
  driverTypeMap["modelcenter"]            = MODELCENTER;
  driverTypeMap["matlab"]                 = MATLAB;
  driverTypeMap["python"]                 = PYTHON;

  // convert strings to enums for analysisDriverTypes, iFilterType, oFilterType
  analysisDriverTypes.resize(numAnalysisDrivers);
  std::map<String, driver_t>::iterator sd_iter;
  for (size_t i=0; i<numAnalysisDrivers; ++i) {
    sd_iter = driverTypeMap.find(analysisDrivers[i]);//toLower(Drivers[i]));
    if (sd_iter == driverTypeMap.end()) {
#ifdef DAKOTA_PLUGIN
      if (outputLevel > NORMAL_OUTPUT)
	Cerr << "Warning: analysis_driver \"" << analysisDrivers[i] << "\" not "
	     << "available at construct time in DirectApplicInterface.\n       "
	     << "  Subsequent interface plug-in may resolve." << std::endl;
      analysisDriverTypes[i] = NO_DRIVER;
#else
      Cerr << "Error: analysis_driver \"" << analysisDrivers[i]
	   << "\" not supported in DirectApplicInterface." << std::endl;
      abort_handler(-1);
#endif
    }
    else
      analysisDriverTypes[i] = sd_iter->second;
  }
  sd_iter = driverTypeMap.find(iFilterName);//toLower(iFilterName));
  if (sd_iter == driverTypeMap.end()) {
#ifdef DAKOTA_PLUGIN
    if (outputLevel > NORMAL_OUTPUT)
      Cerr << "Warning: input filter \"" << iFilterName << "\" not available at"
	   << " construct time in DirectApplicInterface.\n         Subsequent "
	   << "interface plug-in may resolve." << std::endl;
    iFilterType = NO_DRIVER;
#else
    Cerr << "Error: input filter \"" << iFilterName << "\" not supported in "
	 << "DirectApplicInterface." << std::endl;
    abort_handler(-1);
#endif
  }
  else
    iFilterType = sd_iter->second;
  sd_iter = driverTypeMap.find(oFilterName);//toLower(oFilterName));
  if (sd_iter == driverTypeMap.end()) {
#ifdef DAKOTA_PLUGIN
    if (outputLevel > NORMAL_OUTPUT)
      Cerr << "Warning: output filter \"" << oFilterName << "\" not available "
	   << "at construct time in DirectApplicInterface.\n         Subsequent"
	   << " interface plug-in may resolve." << std::endl;
    oFilterType = NO_DRIVER;
#else
    Cerr << "Error: output filter \"" << oFilterName << "\" not supported in "
	 << "DirectApplicInterface." << std::endl;
    abort_handler(-1);
#endif
  }
  else
    oFilterType = sd_iter->second;

  // define localDataView from analysisDriverTypes
  localDataView = 0;
  for (size_t i=0; i<numAnalysisDrivers; ++i)
    switch (analysisDriverTypes[i]) {
    case CANTILEVER_BEAM: case MOD_CANTILEVER_BEAM: case ROSENBROCK:
    case LF_ROSENBROCK:  case SHORT_COLUMN:      case LF_SHORT_COLUMN:
    case SOBOL_ISHIGAMI: case STEEL_COLUMN_COST: case STEEL_COLUMN_PERFORMANCE:
      localDataView |= VARIABLES_MAP;    break;
    case NO_DRIVER: // assume VARIABLES_VECTOR approach for plug-ins for now
    case CYLINDER_HEAD:       case LOGNORMAL_RATIO:     case MULTIMODAL:
    case GERSTNER:            case SCALABLE_GERSTNER:
    case EXTENDED_ROSENBROCK: case GENERALIZED_ROSENBROCK:
    case SIDE_IMPACT_COST:    case SIDE_IMPACT_PERFORMANCE:
    case SOBOL_G_FUNCTION:    case SOBOL_RATIONAL:
    case TEXT_BOOK:     case TEXT_BOOK1: case TEXT_BOOK2: case TEXT_BOOK3:
    case TEXT_BOOK_OUU: case SCALABLE_TEXT_BOOK: case SCALABLE_MONOMIALS:
    case HERBIE:        case SMOOTH_HERBIE:      case SHUBERT:
    case SALINAS:       case MODELCENTER:        case MATLAB: case PYTHON:
      localDataView |= VARIABLES_VECTOR; break;
    }

  // define varTypeMap for analysis drivers based on xCM/XDM maps
  if (localDataView & VARIABLES_MAP) {
    // define the string to enumeration map
    //switch (ac_name) {
    //case ROSENBROCK: case LF_ROSENBROCK: case SOBOL_ISHIGAMI:
      varTypeMap["x1"] = VAR_x1; varTypeMap["x2"] = VAR_x2;
      varTypeMap["x3"] = VAR_x3; //varTypeMap["x4"]  = VAR_x4;
      //varTypeMap["x5"] = VAR_x5; varTypeMap["x6"]  = VAR_x6;
      //varTypeMap["x7"] = VAR_x7; varTypeMap["x8"]  = VAR_x8;
      //varTypeMap["x9"] = VAR_x9; varTypeMap["x10"] = VAR_x10; break;
    //case SHORT_COLUMN: case LF_SHORT_COLUMN:
      varTypeMap["b"] = VAR_b; varTypeMap["h"] = VAR_h;
      varTypeMap["P"] = VAR_P; varTypeMap["M"] = VAR_M; varTypeMap["Y"] = VAR_Y;
      //break;
    //case CANTILEVER_BEAM: case MOD_CANTILEVER_BEAM:
      varTypeMap["w"] = VAR_w; varTypeMap["t"] = VAR_t; varTypeMap["R"] = VAR_R;
      varTypeMap["E"] = VAR_E; varTypeMap["X"] = VAR_X;
      //varTypeMap["Y"] = VAR_Y; break;
    //case STEEL_COLUMN:
      varTypeMap["Fs"] = VAR_Fs; varTypeMap["P1"] = VAR_P1;
      varTypeMap["P2"] = VAR_P2; varTypeMap["P3"] = VAR_P3;
      varTypeMap["B"]  = VAR_B;  varTypeMap["D"]  = VAR_D;
      varTypeMap["H"]  = VAR_H;  //varTypeMap["b"] = VAR_b;
      varTypeMap["d"]  = VAR_d;  //varTypeMap["h"] = VAR_h;
      varTypeMap["F0"] = VAR_F0; //varTypeMap["E"] = VAR_E; break;
    //}
  }

#ifdef DAKOTA_MATLAB
  if (contains(analysisDriverTypes, MATLAB)) {
    std::string engine_cmd;
    const char* env_engine_cmd = getenv("DAKOTA_MATLAB_ENGINE_CMD");
    // eventually just use _WIN32 and __CYGWIN__ here
#if defined(__CYGWIN__) || defined(_WIN32) || defined(__MINGW32__) || defined(_MSC_VER)
    engine_cmd = "\0";
    if (env_engine_cmd)
      Cerr << "\nWarning: DAKOTA_MATLAB_ENGINE_CMD ignored on Windows."
	   << std::endl;
#else
    engine_cmd = env_engine_cmd ? env_engine_cmd :
      "matlab -nodesktop -nosplash";
#endif
    if (matlabEngine = engOpen(engine_cmd.c_str())) {
      if (outputLevel >= NORMAL_OUTPUT)
	Cout << "Matlab engine initialized for direct function evaluation."
	     << std::endl;
      const char * env_matlab_startup = getenv("DAKOTA_MATLAB_STARTUP");
      if (env_matlab_startup)
	engEvalString(matlabEngine, env_matlab_startup);      
    }
    else {
      Cerr << "Error (Direct:Matlab): Could not initialize Matlab engine "
	   << "for direct fn. eval." << std::endl;
      abort_handler(-1);
    }
    mxArray *dakota_matlab = NULL;  // MATLAB Dakota structure
    /* fields to pass to Matlab in Dakota structure */
    const mwSize ndim = 2;
    const mwSize dims[2] = {1, 1};
    /* Create Dakota variable structure in the engine workspace */
    dakota_matlab = mxCreateStructArray(ndim, dims, NUMBER_OF_FIELDS,
					FIELD_NAMES);
    engPutVariable(matlabEngine, "Dakota", dakota_matlab);
  }
#endif

#ifdef DAKOTA_PYTHON
  if (contains(analysisDriverTypes, PYTHON)) {
    Py_Initialize();
    if (Py_IsInitialized()) {
      if (outputLevel >= NORMAL_OUTPUT)
	Cout << "Python interpreter initialized for direct function evaluation."
	     << std::endl;
    }
    else {
      Cerr << "Error: Could not initialize Python for direct function "
	   << "evaluation." << std::endl;
      abort_handler(-1);
    }
#ifdef DAKOTA_PYTHON_NUMPY
    import_array();
    //      userNumpyFlag = problem_db.get_bool("python_numpy");
    userNumpyFlag = true;
#else
    //      if (problem_db.get_bool("python_numpy")) {
    Cout << "Warning: Python numpy not available, ignoring user request."
	 << std::endl;
    userNumpyFlag = false;
    //}
#endif
  }
#endif
}


DirectApplicInterface::~DirectApplicInterface()
{ 
#ifdef DAKOTA_MATLAB
  if (contains(analysisDriverTypes, MATLAB) && matlabEngine)
    if ( engClose(matlabEngine) ) {
      Cerr << "Error (Direct:Matlab): Couldn't terminate Matlab engine post "
	   << "direct fn. eval." << std::endl;
      abort_handler(-1);
    } 
    else if (outputLevel >= NORMAL_OUTPUT)
      Cout << "Matlab engine terminated." << std::endl;
#endif
#ifdef DAKOTA_PYTHON
  if (Py_IsInitialized()) {
    Py_Finalize();
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "Python interpreter terminated." << std::endl;
  }
#endif
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
    if (eaDedMasterFlag)
      Cout << "Direct function: self-scheduling ";
    else if (numAnalysisServers > 1)
      Cout << "Direct function: static scheduling ";
    else
      Cout << "Direct function: invoking ";
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
      self_schedule_analyses();
    else
      serve_analyses_synch();
  }
  else // simple static schedule (all peer cases including single analysis)
    for (analysisDriverIndex =  analysisServerId-1;
	 analysisDriverIndex <  numAnalysisDrivers;
	 analysisDriverIndex += numAnalysisServers)
      derived_map_ac(analysisDrivers[analysisDriverIndex]);
  // NOTE: no synchronization enforced in static case (some processors may lag)

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
    abort_handler(-1);
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
  if (fail_code)
    throw fail_code;

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
  std::map<String, driver_t>::iterator sd_iter = driverTypeMap.find(ac_name);
  driver_t ac_type
    = (sd_iter!=driverTypeMap.end()) ? sd_iter->second : NO_DRIVER;
  switch (ac_type) {
  case CANTILEVER_BEAM:
    fail_code = cantilever(); break;
  case MOD_CANTILEVER_BEAM:
    fail_code = mod_cantilever(); break;
  case CYLINDER_HEAD:
    fail_code = cyl_head(); break;
  case ROSENBROCK:
    fail_code = rosenbrock(); break;
  case GENERALIZED_ROSENBROCK:
    fail_code = generalized_rosenbrock(); break;
  case EXTENDED_ROSENBROCK:
    fail_code = extended_rosenbrock(); break;
  case LF_ROSENBROCK:
    fail_code = lf_rosenbrock(); break;
  case GERSTNER:
    fail_code = gerstner(); break;
  case SCALABLE_GERSTNER:
    fail_code = scalable_gerstner(); break;
  case LOGNORMAL_RATIO:
    fail_code = log_ratio(); break;
  case MULTIMODAL:
    fail_code = multimodal(); break;
  case SHORT_COLUMN:
    fail_code = short_column(); break;
  case LF_SHORT_COLUMN:
    fail_code = lf_short_column(); break;
  case SIDE_IMPACT_COST:
    fail_code = side_impact_cost(); break;
  case SIDE_IMPACT_PERFORMANCE:
    fail_code = side_impact_perf(); break;
  case SOBOL_RATIONAL:
    fail_code = sobol_rational(); break;
  case SOBOL_G_FUNCTION:
    fail_code = sobol_g_function(); break;
  case SOBOL_ISHIGAMI:
    fail_code = sobol_ishigami(); break;
  case STEEL_COLUMN_COST:
    fail_code = steel_column_cost(); break;
  case STEEL_COLUMN_PERFORMANCE:
    fail_code = steel_column_perf(); break;
  case TEXT_BOOK:
    fail_code = text_book(); break;
  case TEXT_BOOK1: // for testing concurrent analyses
    fail_code = text_book1(); break;
  case TEXT_BOOK2: // for testing concurrent analyses
    fail_code = text_book2(); break;
  case TEXT_BOOK3: // for testing concurrent analyses
    fail_code = text_book3(); break;
  case TEXT_BOOK_OUU:
    fail_code = text_book_ouu(); break;
  case SCALABLE_TEXT_BOOK:
    fail_code = scalable_text_book(); break;
  case SCALABLE_MONOMIALS:
    fail_code = scalable_monomials(); break;
  case HERBIE:
    fail_code = herbie(); break;
  case SMOOTH_HERBIE:
    fail_code = smooth_herbie(); break;
  case SHUBERT:
    fail_code = shubert(); break;
#ifdef DAKOTA_SALINAS
  case SALINAS:
    fail_code = salinas(); break;
#endif // DAKOTA_SALINAS
#ifdef DAKOTA_MODELCENTER
  case MODELCENTER: //case MC_API_RUN:
    fail_code = mc_api_run(); break;
#endif // DAKOTA_MODELCENTER
#ifdef DAKOTA_MATLAB
  case MATLAB:
    fail_code = matlab_engine_run(); break;
#endif // DAKOTA_MATLAB
#ifdef DAKOTA_PYTHON
  case PYTHON:
    fail_code = python_run(); break;
#endif // DAKOTA_PYTHON
  default: {
    Cerr << ac_name << " is not available as an analysis driver within "
         << "DirectApplicInterface." << std::endl;
    abort_handler(-1);
  }
  }

  // Failure capturing
  if (fail_code)
    throw fail_code;

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
    abort_handler(-1);
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
  if (fail_code)
    throw fail_code;

  return 0;
}


void DirectApplicInterface::derived_map_asynch(const ParamResponsePair& pair)
{
  Cerr << "Error: asynchronous capability (multiple threads) not installed in"
       << "\nDirectApplicInterface." << std::endl;
  abort_handler(-1);

  //pthread_create/thr_create(derived_map(...)) launches a new thread
  //threadIdMap[tid] = fn_eval_id;
}


void DirectApplicInterface::derived_synch(PRPQueue& prp_queue)
{
  Cerr << "Error: asynchronous capability (multiple threads) not installed in"
       << "\nDirectApplicInterface." << std::endl;
  abort_handler(-1);

  /*
  ParamResponsePair pr_pair;
  bool found = lookup_by_eval_id(prp_queue, fn_eval_id, pr_pair);
  if (!found) {
    Cerr << "Error: failure in queue lookup within DirectApplicInterface::"
	 << "derived_synch()." << std::endl;
    abort_handler(-1);
  }
  int fail_code = 0, id = pr_pair.eval_id();
  Variables vars    = pr_pair.prp_parameters(); // shallow copy
  Response response = pr_pair.prp_response();   // shallow copy

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
  pr_pair.prp_response(response);
  */
}


void DirectApplicInterface::derived_synch_nowait(PRPQueue& prp_queue)
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
  numACV = vars.acv(); numADIV = vars.adiv(); numADRV = vars.adrv();
  numVars = numACV + numADIV + numADRV;

  // Initialize copies of incoming data
  //directFnVars = vars; // shared rep
  if (localDataView & VARIABLES_MAP) {
    size_t i;
    // set labels once (all processors)
    if (xCMLabels.size()  != numACV || xDIMLabels.size() != numADIV ||
	xDRMLabels.size() != numADRV) {
      StringMultiArrayConstView acv_labels
	= vars.all_continuous_variable_labels();
      StringMultiArrayConstView adiv_labels
	= vars.all_discrete_int_variable_labels();
      StringMultiArrayConstView adrv_labels
	= vars.all_discrete_real_variable_labels();
      xCMLabels.resize(numACV);
      xDIMLabels.resize(numADIV);
      xDRMLabels.resize(numADRV);
      //String label_i;
      std::map<String, var_t>::iterator v_iter;
      for (i=0; i<numACV; ++i) {
	//label_i = toLower(acv_labels[i]);
	v_iter = varTypeMap.find(acv_labels[i]);//(label_i);
	if (v_iter == varTypeMap.end()) {
	  Cerr << "Error: label \"" << acv_labels[i]//label_i
	       << "\" not supported in analysis driver." << std::endl;
	  abort_handler(-1);
	}
	else
	  xCMLabels[i] = v_iter->second;
      }
      for (i=0; i<numADIV; ++i) {
	//label_i = toLower(adiv_labels[i]);
	v_iter = varTypeMap.find(adiv_labels[i]);//(label_i);
	if (v_iter == varTypeMap.end()) {
	  Cerr << "Error: label \"" << adiv_labels[i]//label_i
	       << "\" not supported in analysis driver." << std::endl;
	  abort_handler(-1);
	}
	else
	  xDIMLabels[i] = v_iter->second;
      }
      for (i=0; i<numADRV; ++i) {
	//label_i = toLower(adrv_labels[i]);
	v_iter = varTypeMap.find(adrv_labels[i]);//(label_i);
	if (v_iter == varTypeMap.end()) {
	  Cerr << "Error: label \"" << adrv_labels[i]//label_i
	       << "\" not supported in analysis driver." << std::endl;
	  abort_handler(-1);
	}
	else
	  xDRMLabels[i] = v_iter->second;
      }
    }
    // set variable values on every evaluation
    const RealVector& acv  = vars.all_continuous_variables();
    const IntVector&  adiv = vars.all_discrete_int_variables();
    const RealVector& adrv = vars.all_discrete_real_variables();
    xCM.clear(); xDIM.clear(); xDRM.clear(); // more rigorous than overwrite
    for (i=0; i<numACV; ++i)
      xCM[xCMLabels[i]] = acv[i];
    for (i=0; i<numADIV; ++i)
      xDIM[xDIMLabels[i]] = adiv[i];
    for (i=0; i<numADRV; ++i)
      xDRM[xDRMLabels[i]] = adrv[i];
  }
  if (localDataView & VARIABLES_VECTOR) {
    // set labels once (all processors)
    if (xCLabels.size()  != numACV || xDILabels.size() != numADIV ||
	xDRLabels.size() != numADRV) {
      xCLabels.resize(boost::extents[numACV]);
      xCLabels = vars.all_continuous_variable_labels();
      xDILabels.resize(boost::extents[numADIV]);
      xDILabels = vars.all_discrete_int_variable_labels();
      xDRLabels.resize(boost::extents[numADRV]);
      xDRLabels = vars.all_discrete_real_variable_labels();
    }
    xC  = vars.all_continuous_variables();    // view OK
    xDI = vars.all_discrete_int_variables();  // view OK
    xDR = vars.all_discrete_real_variables(); // view OK
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
	abort_handler(-1);
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


// -----------------------------------------
// Begin direct interfaces to test functions
// -----------------------------------------
int DirectApplicInterface::cantilever()
{
  using std::pow;

  if (multiProcAnalysisFlag) {
    Cerr << "Error: cantilever direct fn does not support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  // cantilever normally has 6 variables: 2 design + 4 uncertain
  // If, however, design variables are _inserted_ into the uncertain variable
  // distribution parameters (e.g., dakota_rbdo_cantilever_mapvars.in) instead
  // of augmenting the uncertain variables, then the number of variables is 4.
  // Design gradients are not supported for the case of design var insertion.
  if ( (numVars != 4 && numVars != 6) || numADIV || numADRV ||//var count, no dv
       (gradFlag && numVars == 4 && numDerivVars != 4) ) { // design insertion
    Cerr << "Error: Bad number of variables in cantilever direct fn."
	 << std::endl;
    abort_handler(-1);
  }
   if (numFns != 3) {
    Cerr << "Error: Bad number of functions in cantilever direct fn."
	 << std::endl;
    abort_handler(-1);
  }

  // Compute the cross-sectional area, stress, and displacement of the
  // cantilever beam.  This simulator is unusual in that it must support both
  // the case of design variable insertion and the case of design variable
  // augmentation.  It does not support mixed insertion/augmentation.  In
  // the 6 variable case, w,t,R,E,X,Y are all passed in; in the 4 variable
  // case, w,t assume local values.
  std::map<var_t, Real>::iterator m_iter = xCM.find(VAR_w);
  Real w = (m_iter == xCM.end()) ? 2.5 : m_iter->second; // beam width
  m_iter = xCM.find(VAR_t);
  Real t = (m_iter == xCM.end()) ? 2.5 : m_iter->second; // beam thickness
  Real R = xCM[VAR_R], // yield strength
       E = xCM[VAR_E], // Young's modulus
       X = xCM[VAR_X], // horizontal load
       Y = xCM[VAR_Y]; // vertical load

  // optimization inequality constraint: <= 0 and scaled O(1)
  //Real g_stress = stress/R - 1.0;
  //Real g_disp   = disp/D0  - 1.0;

  Real D0 = 2.2535, L = 100., area = w*t, w_sq = w*w, t_sq = t*t,
       R_sq = R*R, X_sq = X*X, Y_sq = Y*Y;
  Real stress = 600.*Y/w/t_sq + 600.*X/w_sq/t;
  Real D1 = 4.*pow(L,3)/E/area,  D2 = pow(Y/t_sq, 2)+pow(X/w_sq, 2),
       D3 = D1/std::sqrt(D2)/D0, D4 = D1*std::sqrt(D2)/D0;

  // **** f:
  if (directFnASV[0] & 1)
    fnVals[0] = area;

  // **** c1:
  if (directFnASV[1] & 1)
    fnVals[1] = stress/R - 1.;

  // **** c2:
  if (directFnASV[2] & 1)
    fnVals[2] = D4 - 1.;

  // **** df/dx:
  if (directFnASV[0] & 2)
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_w:  fnGrads[0][i] = t;  break; // design var derivative
      case VAR_t:  fnGrads[0][i] = w;  break; // design var derivative
      default: fnGrads[0][i] = 0.; break; // uncertain var derivative
      }

  // **** dc1/dx:
  if (directFnASV[1] & 2)
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_w: fnGrads[1][i] = -600.*(Y/t + 2.*X/w)/w_sq/t/R; break;//des var
      case VAR_t: fnGrads[1][i] = -600.*(2.*Y/t + X/w)/w/t_sq/R; break;//des var
      case VAR_R: fnGrads[1][i] = -stress/R_sq;  break; // uncertain var deriv
      case VAR_E: fnGrads[1][i] = 0.;            break; // uncertain var deriv
      case VAR_X: fnGrads[1][i] = 600./w_sq/t/R; break; // uncertain var deriv
      case VAR_Y: fnGrads[1][i] = 600./w/t_sq/R; break; // uncertain var deriv
      }

  // **** dc2/dx:
  if (directFnASV[2] & 2)
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_w: fnGrads[2][i] = -D3*2.*X_sq/w_sq/w_sq/w - D4/w; break;//desvar
      case VAR_t: fnGrads[2][i] = -D3*2.*Y_sq/t_sq/t_sq/t - D4/t; break;//desvar
      case VAR_R: fnGrads[2][i] = 0.;             break; // unc var deriv
      case VAR_E: fnGrads[2][i] = -D4/E;          break; // unc var deriv
      case VAR_X: fnGrads[2][i] = D3*X/w_sq/w_sq; break; // unc var deriv
      case VAR_Y: fnGrads[2][i] = D3*Y/t_sq/t_sq; break; // unc var deriv
      }

  // **** d^2f/dx^2:
  if (directFnASV[0] & 4)
    for (size_t i=0; i<numDerivVars; ++i)
      for (size_t j=0; j<=i; ++j)
	fnHessians[0](i,j)
	  = ( (varTypeDVV[i] == VAR_w && varTypeDVV[j] == VAR_t) ||
	      (varTypeDVV[i] == VAR_t && varTypeDVV[j] == VAR_w) ) ? 1. : 0.;

  // **** d^2c1/dx^2:
  if (directFnASV[1] & 4) {
    for (size_t i=0; i<numDerivVars; ++i)
      for (size_t j=0; j<=i; ++j)
	if (varTypeDVV[i] == VAR_w && varTypeDVV[j] == VAR_w)
	  fnHessians[1](i,j) = 1200.*(Y/t + 3.*X/w)/w_sq/area/R;
	else if (varTypeDVV[i] == VAR_t && varTypeDVV[j] == VAR_t)
	  fnHessians[1](i,j) = 1200.*(3.*Y/t + X/w)/t_sq/area/R;
	else if (varTypeDVV[i] == VAR_R && varTypeDVV[j] == VAR_R)
	  fnHessians[1](i,j) = 2.*stress/pow(R, 3);
	else if ( (varTypeDVV[i] == VAR_w && varTypeDVV[j] == VAR_t) ||
		  (varTypeDVV[i] == VAR_t && varTypeDVV[j] == VAR_w) )
	  fnHessians[1](i,j) = 1200.*(Y/t + X/w)/w_sq/t_sq/R;
	else if ( (varTypeDVV[i] == VAR_w && varTypeDVV[j] == VAR_R) ||
		  (varTypeDVV[i] == VAR_R && varTypeDVV[j] == VAR_w) )
	  fnHessians[1](i,j) = 600.*(Y/t + 2.*X/w)/w_sq/t/R_sq;
	else if ( (varTypeDVV[i] == VAR_w && varTypeDVV[j] == VAR_X) ||
		  (varTypeDVV[i] == VAR_X && varTypeDVV[j] == VAR_w) )
	  fnHessians[1](i,j) = -1200./w_sq/w/t/R;
	else if ( (varTypeDVV[i] == VAR_w && varTypeDVV[j] == VAR_Y) ||
		  (varTypeDVV[i] == VAR_Y && varTypeDVV[j] == VAR_w) )
	  fnHessians[1](i,j) = -600./w_sq/t_sq/R;
	else if ( (varTypeDVV[i] == VAR_t && varTypeDVV[j] == VAR_R) ||
		  (varTypeDVV[i] == VAR_R && varTypeDVV[j] == VAR_t) )
	  fnHessians[1](i,j) = 600.*(2.*Y/t + X/w)/w/t_sq/R_sq;
	else if ( (varTypeDVV[i] == VAR_t && varTypeDVV[j] == VAR_X) ||
		  (varTypeDVV[i] == VAR_X && varTypeDVV[j] == VAR_t) )
	  fnHessians[1](i,j) = -600./w_sq/t_sq/R;
	else if ( (varTypeDVV[i] == VAR_t && varTypeDVV[j] == VAR_Y) ||
		  (varTypeDVV[i] == VAR_Y && varTypeDVV[j] == VAR_t) )
	  fnHessians[1](i,j) = -1200./w/t_sq/t/R;
	else if ( (varTypeDVV[i] == VAR_R && varTypeDVV[j] == VAR_X) ||
		  (varTypeDVV[i] == VAR_X && varTypeDVV[j] == VAR_R) )
	  fnHessians[1](i,j) = -600./w_sq/t/R_sq;
	else if ( (varTypeDVV[i] == VAR_R && varTypeDVV[j] == VAR_Y) ||
		  (varTypeDVV[i] == VAR_Y && varTypeDVV[j] == VAR_R) )
	  fnHessians[1](i,j) = -600./w/t_sq/R_sq;
	else
	  fnHessians[1](i,j) = 0.;
  }

  // **** d^2c2/dx^2:
  if (directFnASV[2] & 4) {
    Real D5 = 1./std::sqrt(D2)/D0, D6 = -D1/2./D0/pow(D2,1.5);
    Real D7 = std::sqrt(D2)/D0,    D8 =  D1/2./D0/std::sqrt(D2);
    Real dD2_dX = 2.*X/w_sq/w_sq, dD3_dX = D6*dD2_dX, dD4_dX = D8*dD2_dX;
    Real dD2_dY = 2.*Y/t_sq/t_sq, dD3_dY = D6*dD2_dY, dD4_dY = D8*dD2_dY;
    Real dD1_dw = -D1/w, dD2_dw = -4.*X_sq/w_sq/w_sq/w,
      dD3_dw = D5*dD1_dw + D6*dD2_dw, dD4_dw = D7*dD1_dw + D8*dD2_dw;
    Real dD1_dt = -D1/t, dD2_dt = -4.*Y_sq/t_sq/t_sq/t,
      dD3_dt = D5*dD1_dt + D6*dD2_dt, dD4_dt = D7*dD1_dt + D8*dD2_dt;
    for (size_t i=0; i<numDerivVars; ++i)
      for (size_t j=0; j<=i; ++j)
	if (varTypeDVV[i] == VAR_w && varTypeDVV[j] == VAR_w)
	  fnHessians[2](i,j) = D3*10.*X_sq/pow(w_sq,3)
	    - 2.*X_sq/w_sq/w_sq/w*dD3_dw + D4/w_sq - dD4_dw/w;
	else if (varTypeDVV[i] == VAR_t && varTypeDVV[j] == VAR_t)
	  fnHessians[2](i,j) = D3*10.*Y_sq/pow(t_sq,3)
	    - 2.*Y_sq/t_sq/t_sq/t*dD3_dt + D4/t_sq - dD4_dt/t;
	else if (varTypeDVV[i] == VAR_E && varTypeDVV[j] == VAR_E) {
	  Real dD1_dE = -D1/E, dD4_dE = D7*dD1_dE;
	  fnHessians[2](i,j) = D4/E/E - dD4_dE/E;
	}
	else if (varTypeDVV[i] == VAR_X && varTypeDVV[j] == VAR_X)
	  fnHessians[2](i,j) = D3/w_sq/w_sq + X/w_sq/w_sq*dD3_dX;
	else if (varTypeDVV[i] == VAR_Y && varTypeDVV[j] == VAR_Y)
	  fnHessians[2](i,j) = D3/t_sq/t_sq + Y/t_sq/t_sq*dD3_dY;
	else if ( (varTypeDVV[i] == VAR_w && varTypeDVV[j] == VAR_t) ||
		  (varTypeDVV[i] == VAR_t && varTypeDVV[j] == VAR_w) )
	  fnHessians[2](i,j) = -2.*X_sq/w_sq/w_sq/w*dD3_dt - dD4_dt/w;
	else if ( (varTypeDVV[i] == VAR_w && varTypeDVV[j] == VAR_E) ||
		  (varTypeDVV[i] == VAR_E && varTypeDVV[j] == VAR_w) )
	  fnHessians[2](i,j) = -dD4_dw/E;
	else if ( (varTypeDVV[i] == VAR_w && varTypeDVV[j] == VAR_X) ||
		  (varTypeDVV[i] == VAR_X && varTypeDVV[j] == VAR_w) )
	  fnHessians[2](i,j) = -4.*X*D3/w_sq/w_sq/w + X/w_sq/w_sq*dD3_dw;
	else if ( (varTypeDVV[i] == VAR_w && varTypeDVV[j] == VAR_Y) ||
		  (varTypeDVV[i] == VAR_Y && varTypeDVV[j] == VAR_w) )
	  fnHessians[2](i,j) = Y/t_sq/t_sq*dD3_dw;
	else if ( (varTypeDVV[i] == VAR_t && varTypeDVV[j] == VAR_E) ||
		  (varTypeDVV[i] == VAR_E && varTypeDVV[j] == VAR_t) )
	  fnHessians[2](i,j) = -dD4_dt/E;
	else if ( (varTypeDVV[i] == VAR_t && varTypeDVV[j] == VAR_X) ||
		  (varTypeDVV[i] == VAR_X && varTypeDVV[j] == VAR_t) )
	  fnHessians[2](i,j) = X/w_sq/w_sq*dD3_dt;
	else if ( (varTypeDVV[i] == VAR_t && varTypeDVV[j] == VAR_Y) ||
		  (varTypeDVV[i] == VAR_Y && varTypeDVV[j] == VAR_t) )
	  fnHessians[2](i,j) = -4.*Y*D3/t_sq/t_sq/t + Y/t_sq/t_sq*dD3_dt;
	else if ( (varTypeDVV[i] == VAR_E && varTypeDVV[j] == VAR_X) ||
		  (varTypeDVV[i] == VAR_X && varTypeDVV[j] == VAR_E) )
	  fnHessians[2](i,j) = -dD4_dX/E;
	else if ( (varTypeDVV[i] == VAR_E && varTypeDVV[j] == VAR_Y) ||
		  (varTypeDVV[i] == VAR_Y && varTypeDVV[j] == VAR_E) )
	  fnHessians[2](i,j) = -dD4_dY/E;
	else if ( (varTypeDVV[i] == VAR_X && varTypeDVV[j] == VAR_Y) ||
		  (varTypeDVV[i] == VAR_Y && varTypeDVV[j] == VAR_X) )
	  fnHessians[2](i,j) = X/w_sq/w_sq*dD3_dY;
	else
	  fnHessians[2](i,j) = 0.;
  }

  return 0; // no failure
}


int DirectApplicInterface::mod_cantilever()
{
  using std::pow;

  if (multiProcAnalysisFlag) {
    Cerr << "Error: cantilever direct fn does not support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  // cantilever normally has 6 variables: 2 design + 4 uncertain
  // If, however, design variables are _inserted_ into the uncertain variable
  // distribution parameters (e.g., dakota_rbdo_cantilever_mapvars.in) instead
  // of augmenting the uncertain variables, then the number of variables is 4.
  // Design gradients are not supported for the case of design var insertion.
  if ( (numVars != 4 && numVars != 6) || numADIV || numADRV ||//var count, no dv
       (gradFlag && numVars == 4 && numDerivVars != 4) ) { // design insertion
    Cerr << "Error: Bad number of variables in cantilever direct fn."
	 << std::endl;
    abort_handler(-1);
  }
   if (numFns != 3) {
    Cerr << "Error: Bad number of functions in cantilever direct fn."
	 << std::endl;
    abort_handler(-1);
  }

  // Compute the cross-sectional area, stress, and displacement of the
  // cantilever beam.  This simulator is unusual in that it must support both
  // the case of design variable insertion and the case of design variable
  // augmentation.  It does not support mixed insertion/augmentation.  In
  // the 6 variable case, w,t,R,E,X,Y are all passed in; in the 4 variable
  // case, w,t assume local values.
  std::map<var_t, Real>::iterator m_iter = xCM.find(VAR_w);
  Real w = (m_iter == xCM.end()) ? 2.5 : m_iter->second; // beam width
  m_iter = xCM.find(VAR_t);
  Real t = (m_iter == xCM.end()) ? 2.5 : m_iter->second; // beam thickness
  Real R = xCM[VAR_R], // yield strength
       E = xCM[VAR_E], // Young's modulus
       X = xCM[VAR_X], // horizontal load
       Y = xCM[VAR_Y]; // vertical load

  // UQ limit state <= 0: don't scale stress by random variable r
  //double g_stress = stress - r;
  //double g_disp   = displ  - D0;

  Real D0 = 2.2535, L = 100., area = w*t, w_sq = w*w, t_sq = t*t,
       R_sq = R*R, X_sq = X*X, Y_sq = Y*Y;
  Real stress = 600.*Y/w/t_sq + 600.*X/w_sq/t;
  Real D1 = 4.*pow(L,3)/E/area, D2 = pow(Y/t_sq, 2)+pow(X/w_sq, 2),
       D3 = D1/std::sqrt(D2),   displ = D1*std::sqrt(D2);

  // **** f:
  if (directFnASV[0] & 1)
    fnVals[0] = area;

  // **** c1:
  if (directFnASV[1] & 1)
    fnVals[1] = stress - R;

  // **** c2:
  if (directFnASV[2] & 1)
    fnVals[2] = displ - D0;

  // **** df/dx:
  if (directFnASV[0] & 2)
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_w:  fnGrads[0][i] = t;  break; // design var derivative
      case VAR_t:  fnGrads[0][i] = w;  break; // design var derivative
      default: fnGrads[0][i] = 0.; break; // uncertain var derivative
      }

  // **** dc1/dx:
  if (directFnASV[1] & 2)
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_w: fnGrads[1][i] = -600.*(Y/t + 2.*X/w)/w_sq/t; break;//des var
      case VAR_t: fnGrads[1][i] = -600.*(2.*Y/t + X/w)/w/t_sq; break;//des var
      case VAR_R: fnGrads[1][i] = -1.;          break; // uncertain var deriv
      case VAR_E: fnGrads[1][i] =  0.;          break; // uncertain var deriv
      case VAR_X: fnGrads[1][i] =  600./w_sq/t; break; // uncertain var deriv
      case VAR_Y: fnGrads[1][i] =  600./w/t_sq; break; // uncertain var deriv
      }

  // **** dc2/dx:
  if (directFnASV[2] & 2)
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_w: fnGrads[2][i] = -D3*2.*X_sq/w_sq/w_sq/w - displ/w; break;// dv
      case VAR_t: fnGrads[2][i] = -D3*2.*Y_sq/t_sq/t_sq/t - displ/t; break;// dv
      case VAR_R: fnGrads[2][i] =  0.;             break; // unc var deriv
      case VAR_E: fnGrads[2][i] = -displ/E;        break; // unc var deriv
      case VAR_X: fnGrads[2][i] =  D3*X/w_sq/w_sq; break; // unc var deriv
      case VAR_Y: fnGrads[2][i] =  D3*Y/t_sq/t_sq; break; // unc var deriv
      }

  /* Alternative modification: take E out of displ denominator to remove
     singularity in tail (at 20 std deviations).  In PCE/SC testing, this
     had minimal impact and did not justify the nonstandard form.

  Real D0 = 2.2535, L = 100., area = w*t, w_sq = w*w, t_sq = t*t,
       R_sq = R*R, X_sq = X*X, Y_sq = Y*Y;
  Real stress = 600.*Y/w/t_sq + 600.*X/w_sq/t;
  Real D1 = 4.*pow(L,3)/area, D2 = pow(Y/t_sq, 2)+pow(X/w_sq, 2),
       D3 = D1/std::sqrt(D2), D4 = D1*std::sqrt(D2);

  // **** c2:
  if (directFnASV[2] & 1)
    fnVals[2] = D4 - D0*E;

  // **** dc2/dx:
  if (directFnASV[2] & 2)
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_w: fnGrads[2][i] = -D3*2.*X_sq/w_sq/w_sq/w - D4/w; break;//desvar
      case VAR_t: fnGrads[2][i] = -D3*2.*Y_sq/t_sq/t_sq/t - D4/t; break;//desvar
      case VAR_R: fnGrads[2][i] =  0.;             break; // unc var deriv
      case VAR_E: fnGrads[2][i] = -D0;             break; // unc var deriv
      case VAR_X: fnGrads[2][i] =  D3*X/w_sq/w_sq; break; // unc var deriv
      case VAR_Y: fnGrads[2][i] =  D3*Y/t_sq/t_sq; break; // unc var deriv
      }
  */

  return 0; // no failure
}


int DirectApplicInterface::cyl_head()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: cyl_head direct fn does not yet support "
	 << "multiprocessor analyses." << std::endl;
    abort_handler(-1);
  }
  if (numVars != 2 || numADIV || numADRV || (gradFlag && numDerivVars != 2)) {
    Cerr << "Error: Bad number of variables in cyl_head direct fn." <<std::endl;
    abort_handler(-1);
  }
  if (numFns != 4) {
    Cerr << "Error: Bad number of functions in cyl_head direct fn." <<std::endl;
    abort_handler(-1);
  }
  if (hessFlag) {
    Cerr << "Error: Hessians not supported in cyl_head direct fn." << std::endl;
    abort_handler(-1);
  }

  Real exhaust_offset = 1.34;
  Real exhaust_dia    = 1.556;
  Real intake_offset  = 3.25;
  // Use nondimensional xC[1]: 
  // (0. <= nondimensional <= 4.), (0. in <= dimensional <= 0.004 in)
  Real warranty       = 100000. + 15000. * (4. - xC[1]);
  Real cycle_time     = 45. + 4.5*std::pow(4. - xC[1], 1.5);
  Real wall_thickness = intake_offset - exhaust_offset - (xC[0]+exhaust_dia)/2.;
  Real horse_power    = 250.+200.*(xC[0]/1.833-1.);
  Real max_stress     = 750. + std::pow(std::fabs(wall_thickness),-2.5);

  // **** f:
  if (directFnASV[0] & 1)
    fnVals[0] =  -1.*(horse_power/250.+warranty/100000.);

  // **** c1:
  if (directFnASV[1] & 1)
    fnVals[1] = max_stress/1500.-1.;

  // **** c2:
  if (directFnASV[2] & 1)
    fnVals[2] = 1.-warranty/100000.;

  // **** c3:
  if (directFnASV[3] & 1)
    fnVals[3] = cycle_time/60. - 1.;

  // **** c4: (Unnecessary if intake_dia upper bound reduced to 2.164)
  //if (directFnASV[4] & 1)
  //  fnVals[4] = 1.-20.*wall_thickness;

  // **** df/dx:
  if (directFnASV[0] & 2) {
    fnGrads[0][0] = -.8/1.833;
    fnGrads[0][1] = 0.15;
  }

  // **** dc1/dx:
  if (directFnASV[1] & 2) {
    fnGrads[1][0] = 1.25/1500*std::pow(wall_thickness, -3.5);
    fnGrads[1][1] = 0.;
  }

  // **** dc2/dx:
  if (directFnASV[2] & 2) {
    fnGrads[2][0] = 0.;
    fnGrads[2][1] = 0.15;
  }

  // **** dc3/dx:
  if (directFnASV[3] & 2) {
    fnGrads[3][0] = 0.;
    fnGrads[3][1] = -0.1125*std::sqrt(4. - xC[1]);
  }

  return 0; // no failure
}


int DirectApplicInterface::multimodal()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: multimodal direct fn does not support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  if ( numVars != 2 || numADIV || numADRV ||
       ( ( gradFlag || hessFlag ) && numDerivVars != 2 ) ) {
    Cerr << "Error: Bad number of variables in multimodal direct fn."
	 << std::endl;
    abort_handler(-1);
  }
  if (numFns != 1) {
    Cerr << "Error: Bad number of functions in multimodal direct fn."
	 << std::endl;
    abort_handler(-1);
  }

  // **** f:
  if (directFnASV[0] & 1)
    fnVals[0] = (xC[0]*xC[0]+4)*(xC[1]-1)/20 - std::sin(5*xC[0]/2) - 2;

  // **** df/dx:
  if (directFnASV[0] & 2) {
    fnGrads[0][0] = xC[0]*(xC[1]-1)/10 - (5/2)*std::cos(5*xC[0]/2);
    fnGrads[0][1] = (xC[0]*xC[0]+4)/20;
  }

  // **** d^2f/dx^2:
  if (directFnASV[0] & 4) {
    fnHessians[0](0,0) = (xC[1]-1)/10 + (25/4)*std::sin(5*xC[0]/2);
    fnHessians[0](0,1) = fnHessians[0](1,0) = xC[0]/10;
    fnHessians[0](1,1) = 0.0;
  }

  return 0; // no failure
}


int DirectApplicInterface::rosenbrock()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: rosenbrock direct fn does not yet support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  if (numVars != 2 || numADIV || numADRV) {
    Cerr << "Error: Bad number of variables in rosenbrock direct fn."
	 << std::endl;
    abort_handler(-1);
  }
  if (numFns > 2) { // 1 fn -> opt, 2 fns -> least sq
    Cerr << "Error: Bad number of functions in rosenbrock direct fn."
	 << std::endl;
    abort_handler(-1);
  }

  bool least_sq_flag = (numFns > 1) ? true : false;
  Real x1 = xCM[VAR_x1], x2 = xCM[VAR_x2], f1 = x2-x1*x1, f2 = 1.-x1;

  if (least_sq_flag) {
    // **** Residual R1:
    if (directFnASV[0] & 1)
      fnVals[0] = 10*f1;
    // **** Residual R2:
    if (directFnASV[1] & 1)
      fnVals[1] = f2;

    // **** dR1/dx:
    if (directFnASV[0] & 2)
      for (size_t i=0; i<numDerivVars; ++i)
	switch (varTypeDVV[i]) {
	case VAR_x1: fnGrads[0][i] = -20.*x1; break;
	case VAR_x2: fnGrads[0][i] =  10.;    break;
	}
    // **** dR2/dx:
    if (directFnASV[1] & 2)
      for (size_t i=0; i<numDerivVars; ++i)
	switch (varTypeDVV[i]) {
	case VAR_x1: fnGrads[1][i] = -1.; break;
	case VAR_x2: fnGrads[1][i] =  0.; break;
	}

    // **** d^2R1/dx^2:
    if (directFnASV[0] & 4)
      for (size_t i=0; i<numDerivVars; ++i)
	for (size_t j=0; j<=i; ++j)
	  if (varTypeDVV[i] == VAR_x1 && varTypeDVV[j] == VAR_x1)
	    fnHessians[0](i,j) = -20.;
	  else
	    fnHessians[0](i,j) =   0.;
    // **** d^2R2/dx^2:
    if (directFnASV[1] & 4)
      fnHessians[1] = 0.;
  }
  else {
    // **** f:
    if (directFnASV[0] & 1)
      fnVals[0] = 100.*f1*f1+f2*f2;

    // **** df/dx:
    if (directFnASV[0] & 2)
      for (size_t i=0; i<numDerivVars; ++i)
	switch (varTypeDVV[i]) {
	case VAR_x1: fnGrads[0][i] = -400.*f1*x1 - 2.*f2; break;
	case VAR_x2: fnGrads[0][i] =  200.*f1;            break;
	}

    // **** d^2f/dx^2:
    if (directFnASV[0] & 4)
      for (size_t i=0; i<numDerivVars; ++i)
	for (size_t j=0; j<=i; ++j)
	  if (varTypeDVV[i] == VAR_x1 && varTypeDVV[j] == VAR_x1)
	    fnHessians[0](i,j) = -400.*(x2 - 3.*x1*x1) + 2.;
	  else if ( (varTypeDVV[i] == VAR_x1 && varTypeDVV[j] == VAR_x2) ||
		    (varTypeDVV[i] == VAR_x2 && varTypeDVV[j] == VAR_x1) )
	    fnHessians[0](i,j) = -400.*x1;
	  else if (varTypeDVV[i] == VAR_x2 && varTypeDVV[j] == VAR_x2)
	    fnHessians[0](i,j) =  200.;
  }

  return 0; // no failure
}


int DirectApplicInterface::generalized_rosenbrock()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: generalized_rosenbrock direct fn does not support "
	 << "multiprocessor analyses." << std::endl;
    abort_handler(-1);
  }
  if (numADIV || numADRV) {
    Cerr << "Error: discrete variables not supported in generalized_rosenbrock "
	 << "direct fn." << std::endl;
    abort_handler(-1);
  }
  if ( (directFnASV[0] & 6) && numVars != numDerivVars ) {
    Cerr << "Error: DVV subsets not supported in generalized_rosenbrock direct "
	 << "fn." << std::endl;
    abort_handler(-1);
  }
  if (numFns > 1) {
    Cerr << "Error: Bad number of functions in generalized_rosenbrock direct "
	 << "fn." << std::endl;
    abort_handler(-1);
  }

  for (size_t i=1; i<numVars; ++i) {
    size_t index_ip1 = i, index_i = i-1; // offset by 1
    const Real& x_ip1 = xC[index_ip1];
    const Real& x_i   = xC[index_i];
    Real f1 = x_ip1 - x_i*x_i, f2 = 1. - x_i;

    // **** f:
    if (directFnASV[0] & 1)
      fnVals[0] += 100.*f1*f1 + f2*f2;

    // **** df/dx:
    if (directFnASV[0] & 2) {
      fnGrads[0][index_i]   += -400.*f1*x_i - 2.*f2;
      fnGrads[0][index_ip1] +=  200.*f1;
    }

    // **** d^2f/dx^2:
    if (directFnASV[0] & 4) {
      Real fx = x_ip1 - 3.*x_i*x_i;
      fnHessians[0](index_i,index_i)     += -400.*fx + 2.0;
      fnHessians[0](index_i,index_ip1)   += -400.*x_i;
      fnHessians[0](index_ip1,index_i)   += -400.*x_i;
      fnHessians[0](index_ip1,index_ip1) +=  200.;
    }
  }

  return 0; // no failure
}


int DirectApplicInterface::extended_rosenbrock()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: extended_rosenbrock direct fn does not support "
	 << "multiprocessor analyses." << std::endl;
    abort_handler(-1);
  }
  if (numADIV || numADRV) {
    Cerr << "Error: discrete variables not supported in extended_rosenbrock "
	 << "direct fn." << std::endl;
    abort_handler(-1);
  }
  if ( (directFnASV[0] & 6) && numVars != numDerivVars ) {
    Cerr << "Error: DVV subsets not supported in extended_rosenbrock direct fn."
	 << std::endl;
    abort_handler(-1);
  }
  if (numFns > 1) {
    Cerr << "Error: Bad number of functions in extended_rosenbrock direct fn."
	 << std::endl;
    abort_handler(-1);
  }

  const Real alpha = 100.;
  for (size_t i=1; i<=numVars/2; ++i) {
    size_t index_2i = 2*i-1, index_2im1 = 2*i-2; // offset by 1
    const Real& x_2i   = xC[index_2i];
    const Real& x_2im1 = xC[index_2im1];
    Real f1 = x_2i - x_2im1*x_2im1, f2 = 1. - x_2im1;

    // **** f:
    if (directFnASV[0] & 1)
      fnVals[0] += alpha*f1*f1 + f2*f2;

    // **** df/dx:
    if (directFnASV[0] & 2) {
      fnGrads[0][index_2im1] += -4.*alpha*f1*x_2im1 - 2.*f2;
      fnGrads[0][index_2i]   +=  2.*alpha*f1;
    }

    // **** d^2f/dx^2:
    if (directFnASV[0] & 4) {
      Real fx = x_2i - 3.*x_2im1*x_2im1;
      fnHessians[0](index_2im1,index_2im1) += -4.*alpha*fx + 2.0;
      fnHessians[0](index_2im1,index_2i)   += -4.*alpha*x_2im1;
      fnHessians[0](index_2i,index_2im1)   += -4.*alpha*x_2im1;
      fnHessians[0](index_2i,index_2i)     +=  2.*alpha;
    }
  }

  return 0; // no failure
}


int DirectApplicInterface::lf_rosenbrock()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: lf_rosenbrock direct fn does not support "
	 << "multiprocessor analyses." << std::endl;
    abort_handler(-1);
  }
  if (numVars != 2 || numADIV || numADRV) {
    Cerr << "Error: Bad number of variables in lf_rosenbrock direct fn."
	 << std::endl;
    abort_handler(-1);
  }
  if (numFns > 1) {
    Cerr << "Error: Bad number of functions in lf_rosenbrock direct fn."
	 << std::endl;
    abort_handler(-1);
  }

  Real x1 = xCM[VAR_x1],     x2 = xCM[VAR_x2],
       f1 = x2 - x1*x1 + .2, f2 = 0.8 - x1; // terms offset by +/- .2

  // **** f:
  if (directFnASV[0] & 1)
    fnVals[0] = 100.*f1*f1+f2*f2;

  // **** df/dx:
  if (directFnASV[0] & 2)
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_x1: fnGrads[0][i] = -400.*f1*x1 - 2.*f2; break;
      case VAR_x2: fnGrads[0][i] =  200.*f1;            break;
      }

  // **** d^2f/dx^2:
  if (directFnASV[0] & 4)
    for (size_t i=0; i<numDerivVars; ++i)
      for (size_t j=0; j<=i; ++j)
	if (varTypeDVV[i] == VAR_x1 && varTypeDVV[j] == VAR_x1)
	  fnHessians[0](i,j) = -400.*(x2 - 3.*x1*x1 + .2) + 2.;
	else if ( (varTypeDVV[i] == VAR_x1 && varTypeDVV[j] == VAR_x2) ||
		  (varTypeDVV[i] == VAR_x2 && varTypeDVV[j] == VAR_x1) )
	  fnHessians[0](i,j) = -400.*x1;
	else if (varTypeDVV[i] == VAR_x2 && varTypeDVV[j] == VAR_x2)
	  fnHessians[0](i,j) =  200.;

  return 0; // no failure
}


int DirectApplicInterface::gerstner()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: gerstner direct fn does not support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  if (numVars != 2 || numADIV || numADRV || (gradFlag && numDerivVars != 2)) {
    Cerr << "Error: Bad number of variables in gerstner direct fn."<< std::endl;
    abort_handler(-1);
  }
  if (numFns != 1) {
    Cerr << "Error: Bad number of functions in gerstner direct fn."<< std::endl;
    abort_handler(-1);
  }
  if (hessFlag) {
    Cerr << "Error: Hessians not supported in gerstner direct fn." << std::endl;
    abort_handler(-1);
  }

  const Real& x = xC[0]; const Real& y = xC[1];
  String an_comp = (!analysisComponents.empty() && 
		    !analysisComponents[analysisDriverIndex].empty()) ?
    analysisComponents[analysisDriverIndex][0] : "iso1";
  short test_fn; Real x_coeff, y_coeff, xy_coeff;
  if (an_comp        == "iso1")
    { test_fn = 1; x_coeff = y_coeff = 10.; }
  else if (an_comp   == "iso2")
    { test_fn = 2; x_coeff = y_coeff = xy_coeff = 1.; }
  else if (an_comp   == "iso3")
    { test_fn = 3; x_coeff = y_coeff = 10.; }
  else if (an_comp == "aniso1")
    { test_fn = 1; x_coeff =  1.; y_coeff = 10.; }
  else if (an_comp == "aniso2")
    { test_fn = 2; x_coeff =  1.; y_coeff = xy_coeff = 10.; }
  else if (an_comp == "aniso3")
    { test_fn = 3; x_coeff = 10.; y_coeff = 5.; }
  else {
    Cerr << "Error: analysis component specification required in gerstner "
	 << "direct fn." << std::endl;
    abort_handler(-1);
  }

  // **** f:
  if (directFnASV[0] & 1) {
    switch (test_fn) {
    case 1:
      fnVals[0] = x_coeff*std::exp(-x*x) + y_coeff*std::exp(-y*y); break;
    case 2:
      fnVals[0]	=  x_coeff*std::exp(x) + y_coeff*std::exp(y)
	        + xy_coeff*std::exp(x*y);                          break;
    case 3:
      fnVals[0] = std::exp(-x_coeff*x*x - y_coeff*y*y);            break;
    }
  }

  // **** df/dx:
  if (directFnASV[0] & 2) {
    Real val;
    switch (test_fn) {
    case 1:
      fnGrads[0][0] = -2.*x*x_coeff*std::exp(-x*x);
      fnGrads[0][1] = -2.*y*y_coeff*std::exp(-y*y); break;
    case 2:
      val = xy_coeff*std::exp(x*y);
      fnGrads[0][0] = x_coeff*std::exp(x) + val*y;
      fnGrads[0][1] = y_coeff*std::exp(y) + val*x;  break;
    case 3:
      val = std::exp(-x_coeff*x*x - y_coeff*y*y);
      fnGrads[0][0] = -2.*x*x_coeff*val;
      fnGrads[0][1] = -2.*y*y_coeff*val;            break;
    }
  }

  return 0; // no failure
}


int DirectApplicInterface::scalable_gerstner()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: scalable_gerstner direct fn does not support "
	 << "multiprocessor analyses." << std::endl;
    abort_handler(-1);
  }
  if (numADIV || numADRV) {
    Cerr << "Error: Bad variable types in scalable_gerstner direct fn."
	 << std::endl;
    abort_handler(-1);
  }
  if (numFns != 1) {
    Cerr << "Error: Bad number of functions in scalable_gerstner direct fn."
	 << std::endl;
    abort_handler(-1);
  }
  if (hessFlag) {
    Cerr << "Error: Hessians not supported in scalable_gerstner direct fn."
	 << std::endl;
    abort_handler(-1);
  }

  String an_comp = (!analysisComponents.empty() && 
		    !analysisComponents[analysisDriverIndex].empty()) ?
    analysisComponents[analysisDriverIndex][0] : "iso1";
  short test_fn; Real even_coeff, odd_coeff, inter_coeff;
  if (an_comp        == "iso1")
    { test_fn = 1; even_coeff = odd_coeff = 10.; }
  else if (an_comp   == "iso2")
    { test_fn = 2; even_coeff = odd_coeff = inter_coeff = 1.; }
  else if (an_comp   == "iso3")
    { test_fn = 3; even_coeff = odd_coeff = 10.; }
  else if (an_comp == "aniso1")
    { test_fn = 1; even_coeff =  1.; odd_coeff = 10.; }
  else if (an_comp == "aniso2")
    { test_fn = 2; even_coeff =  1.; odd_coeff = inter_coeff = 10.; }
  else if (an_comp == "aniso3")
    { test_fn = 3; even_coeff = 10.; odd_coeff = 5.; }
  else {
    Cerr << "Error: analysis component specification required in gerstner "
	 << "direct fn." << std::endl;
    abort_handler(-1);
  }

  // **** f:
  if (directFnASV[0] & 1) {
    switch (test_fn) {
    case 1:
      fnVals[0] = 0.;
      for (size_t i=0; i<numVars; ++i)
	fnVals[0] += (i%2) ? odd_coeff*std::exp(-xC[i]*xC[i]) :
	                    even_coeff*std::exp(-xC[i]*xC[i]); break;
    case 2:
      fnVals[0] = 0.;
      for (size_t i=0; i<numVars; ++i)
	if (i%2)
	  fnVals[0] +=  odd_coeff*std::exp(xC[i])
	            + inter_coeff*std::exp(xC[i]*xC[i-1]);
	else
	  fnVals[0] += even_coeff*std::exp(xC[i]);
      break;
    case 3: {
      Real sum = 0;
      for (size_t i=0; i<numVars; ++i)
	sum -= (i%2) ? odd_coeff*xC[i]*xC[i] : even_coeff*xC[i]*xC[i];
      fnVals[0] = std::exp(sum); break;
    }
    }
  }

  // **** df/dx:
  if (directFnASV[0] & 2) {
    Real val;
    switch (test_fn) {
    case 1:
      for (size_t i=0; i<numVars; ++i)
	fnGrads[0][i] = (i%2) ? -2.*xC[i]* odd_coeff*std::exp(-xC[i]*xC[i])
	                      : -2.*xC[i]*even_coeff*std::exp(-xC[i]*xC[i]);
      break;
    case 2:
      for (size_t i=0; i<numVars; ++i)
	fnGrads[0][i] = (i%2) ?
	  odd_coeff*std::exp(xC[i])
	  + inter_coeff*xC[i-1]*std::exp(xC[i]*xC[i-1]) :
	  even_coeff*std::exp(xC[i])
	  + inter_coeff*xC[i+1]*std::exp(xC[i]*xC[i+1]);
      break;
    case 3:
      if (directFnASV[0] & 1)
	val = fnVals[0];
      else {
	Real sum = 0;
	for (size_t i=0; i<numVars; ++i)
	  sum -= (i%2) ? odd_coeff*xC[i]*xC[i] : even_coeff*xC[i]*xC[i];
	val = std::exp(sum);
      }
      for (size_t i=0; i<numVars; ++i)
	fnGrads[0][i] = (i%2) ? -2.*xC[i]* odd_coeff*val
	                      : -2.*xC[i]*even_coeff*val;
      break;
    }
  }

  return 0; // no failure
}


int DirectApplicInterface::log_ratio()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: log_ratio direct fn does not support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  if ( numVars != 2 || numADIV || numADRV ||
       ( ( gradFlag || hessFlag ) && numDerivVars != 2 ) ) {
    Cerr << "Error: Bad number of variables in log_ratio direct fn."
	 << std::endl;
    abort_handler(-1);
  }
  if (numFns != 1) {
    Cerr << "Error: Bad number of functions in log_ratio direct fn."
	 << std::endl;
    abort_handler(-1);
  }

  const Real& x1 = xC[0]; const Real& x2 = xC[1];

  // **** f:
  if (directFnASV[0] & 1)
    fnVals[0] = x1/x2;

  // **** df/dx:
  if (directFnASV[0] & 2) {
    fnGrads[0][0] = 1./x2;
    fnGrads[0][1] = -x1/(x2*x2);
  }

  // **** d^2f/dx^2:
  if (directFnASV[0] & 4) {
    fnHessians[0](0,0) = 0.0;
    fnHessians[0](0,1) = fnHessians[0](1,0) = -1./(x2*x2);
    fnHessians[0](1,1) = 2.*x1/std::pow(x2,3);
  }

  return 0; // no failure
}


int DirectApplicInterface::short_column()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: short_column direct fn does not support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  if (numVars != 5 || numADIV || numADRV) {
    Cerr << "Error: Bad number of variables in short_column direct fn."
	 << std::endl;
    abort_handler(-1);
  }
  size_t ai, lsi;
  if (numFns == 1)      // option for limit state only
    lsi = 0;
  else if (numFns == 2) // option for area + limit state
    { ai = 0; lsi = 1; }
  else {
    Cerr << "Error: Bad number of functions in short_column direct fn."
	 << std::endl;
    abort_handler(-1);
  }

  // b = xC[0] = column base   (design var.)
  // h = xC[1] = column height (design var.)
  // P = xC[2] (normal uncertain var.)
  // M = xC[3] (normal uncertain var.)
  // Y = xC[4] (lognormal uncertain var.)
  Real b = xCM[VAR_b], h = xCM[VAR_h], P = xCM[VAR_P], M = xCM[VAR_M],
       Y = xCM[VAR_Y], b_sq = b*b, h_sq = h*h, P_sq = P*P, Y_sq = Y*Y;

  // **** f (objective = bh = cross sectional area):
  if (numFns > 1 && (directFnASV[ai] & 1))
    fnVals[ai] = b*h;

  // **** g (limit state = short column response):
  if (directFnASV[lsi] & 1)
    fnVals[lsi] = 1. - 4.*M/(b*h_sq*Y) - P_sq/(b_sq*h_sq*Y_sq);

  // **** df/dx (w.r.t. active variables):
  if (numFns > 1 && (directFnASV[ai] & 2))
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_b: // design variable derivative
	fnGrads[ai][i] = h;  break;
      case VAR_h: // design variable derivative
	fnGrads[ai][i] = b;  break;
      default: // uncertain variable derivative
	fnGrads[ai][i] = 0.; break;
      }

  // **** dg/dx (w.r.t. active variables):
  if (directFnASV[lsi] & 2)
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_b: // design variable derivative
	fnGrads[lsi][i] = 4.*M/(b_sq*h_sq*Y) + 2.*P_sq/(b_sq*b*h_sq*Y_sq);break;
      case VAR_h: // design variable derivative
	fnGrads[lsi][i] = 8.*M/(b*h_sq*h*Y)  + 2.*P_sq/(b_sq*h_sq*h*Y_sq);break;
      case VAR_P: // uncertain variable derivative
	fnGrads[lsi][i] = -2.*P/(b_sq*h_sq*Y_sq);                         break;
      case VAR_M: // uncertain variable derivative
	fnGrads[lsi][i] = -4./(b*h_sq*Y);                                 break;
      case VAR_Y: // uncertain variable derivative
	fnGrads[lsi][i] = 4.*M/(b*h_sq*Y_sq) + 2.*P_sq/(b_sq*h_sq*Y_sq*Y);break;
      }

  // **** d^2f/dx^2: (SORM)
  if (numFns > 1 && (directFnASV[ai] & 4))
    for (size_t i=0; i<numDerivVars; ++i)
      for (size_t j=0; j<=i; ++j)
	fnHessians[ai](i,j)
	  = ( (varTypeDVV[i] == VAR_b && varTypeDVV[j] == VAR_h) ||
	      (varTypeDVV[i] == VAR_h && varTypeDVV[j] == VAR_b) ) ? 1. : 0.;

  // **** d^2g/dx^2: (SORM)
  if (directFnASV[lsi] & 4)
    for (size_t i=0; i<numDerivVars; ++i)
      for (size_t j=0; j<=i; ++j)
	if (varTypeDVV[i] == VAR_b && varTypeDVV[j] == VAR_b)
	  fnHessians[lsi](i,j) = -8.*M/(b_sq*b*h_sq*Y)
	    - 6.*P_sq/(b_sq*b_sq*h_sq*Y_sq);
	else if ( (varTypeDVV[i] == VAR_b && varTypeDVV[j] == VAR_h) ||
		  (varTypeDVV[i] == VAR_h && varTypeDVV[j] == VAR_b) )
	  fnHessians[lsi](i,j) = -8.*M/(b_sq*h_sq*h*Y)
	    - 4.*P_sq/(b_sq*b*h_sq*h*Y_sq);
	else if (varTypeDVV[i] == VAR_h && varTypeDVV[j] == VAR_h)
	  fnHessians[lsi](i,j) = -24.*M/(b*h_sq*h_sq*Y)
	    - 6.*P_sq/(b_sq*h_sq*h_sq*Y_sq);
	else if (varTypeDVV[i] == VAR_P && varTypeDVV[j] == VAR_P)
	  fnHessians[lsi](i,j) = -2./(b_sq*h_sq*Y_sq);
	else if ( (varTypeDVV[i] == VAR_P && varTypeDVV[j] == VAR_M) ||
		  (varTypeDVV[i] == VAR_M && varTypeDVV[j] == VAR_P) )
	  fnHessians[lsi](i,j) = 0.;
	else if ( (varTypeDVV[i] == VAR_P && varTypeDVV[j] == VAR_Y) ||
		  (varTypeDVV[i] == VAR_Y && varTypeDVV[j] == VAR_P) )
	  fnHessians[lsi](i,j) = 4.*P/(b_sq*h_sq*Y_sq*Y);
	else if (varTypeDVV[i] == VAR_M && varTypeDVV[j] == VAR_M)
	  fnHessians[lsi](i,j) = 0.;
	else if ( (varTypeDVV[i] == VAR_M && varTypeDVV[j] == VAR_Y) ||
		  (varTypeDVV[i] == VAR_Y && varTypeDVV[j] == VAR_M) )
	  fnHessians[lsi](i,j) = 4./(b*h_sq*Y_sq);
	else if (varTypeDVV[i] == VAR_Y && varTypeDVV[j] == VAR_Y)
	  fnHessians[lsi](i,j) = -8.*M/(b*h_sq*Y_sq*Y)
	    - 6.*P_sq/(b_sq*h_sq*Y_sq*Y_sq);
	else { // unsupported cross-derivative
	  Cerr << "Error: unsupported Hessian cross term in short_column."
	       << std::endl;
	  abort_handler(-1);
	}

  return 0; // no failure
}


int DirectApplicInterface::lf_short_column()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: lf_short_column direct fn does not support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  if (numVars != 5 || numADIV || numADRV) {
    Cerr << "Error: Bad number of variables in lf_short_column direct fn."
	 << std::endl;
    abort_handler(-1);
  }
  size_t ai, lsi;
  if (numFns == 1)      // option for limit state only
    lsi = 0;
  else if (numFns == 2) // option for area + limit state
    { ai = 0; lsi = 1; }
  else {
    Cerr << "Error: Bad number of functions in lf_short_column direct fn."
	 << std::endl;
    abort_handler(-1);
  }

  // b = xC[0] = column base   (design var.)
  // h = xC[1] = column height (design var.)
  // P = xC[2] (normal uncertain var.)
  // M = xC[3] (normal uncertain var.)
  // Y = xC[4] (lognormal uncertain var.)
  Real b = xCM[VAR_b], h = xCM[VAR_h], P = xCM[VAR_P], M = xCM[VAR_M],
       Y = xCM[VAR_Y], b_sq = b*b, h_sq = h*h, P_sq = P*P, M_sq = M*M,
       Y_sq = Y*Y;
  short test_fn = 1;
  if (!analysisComponents.empty() && 
      !analysisComponents[analysisDriverIndex].empty()) {
    const String& an_comp = analysisComponents[analysisDriverIndex][0];
    if (an_comp      == "lf1") test_fn = 1;
    else if (an_comp == "lf2") test_fn = 2;
    else if (an_comp == "lf3") test_fn = 3;
  }

  // **** f (objective = bh = cross sectional area):
  if (numFns > 1 && (directFnASV[ai] & 1))
    fnVals[ai] = b*h;

  // **** g (limit state = short column response):
  if (directFnASV[lsi] & 1) {
    switch (test_fn) {
    case 1:
      fnVals[lsi] = 1. - 4.*P/(b*h_sq*Y) - P_sq/(b_sq*h_sq*Y_sq); break;
    case 2:
      fnVals[lsi] = 1. - 4.*M/(b*h_sq*Y) - M_sq/(b_sq*h_sq*Y_sq); break;
    case 3:
      fnVals[lsi] = 1. - 4.*M/(b*h_sq*Y) - P_sq/(b_sq*h_sq*Y_sq)
	               - 4.*(P - M)/(b*h*Y);                      break;
    }
  }

  return 0; // no failure
}


int DirectApplicInterface::side_impact_cost()
{
  if (numVars != 7 || numFns != 1) {
    Cerr << "Error: wrong number of inputs/outputs in side_impact_cost."
	 << std::endl;
    abort_handler(-1);
  }

  // **** f:
  if (directFnASV[0] & 1)
    fnVals[0] = 1.98 + 4.9*xC[0] + 6.67*xC[1] + 6.98*xC[2] + 4.01*xC[3]
              + 1.78*xC[4] + 2.73*xC[6];

  // **** df/dx:
  if (directFnASV[0] & 2) {
    Real* fn_grad = fnGrads[0];
    fn_grad[0] = 4.9;  fn_grad[1] = 6.67; fn_grad[2] = 6.98;
    fn_grad[3] = 4.01; fn_grad[4] = 1.78; fn_grad[5] = 0.;
    fn_grad[6] = 2.73;
  }

  // **** d^2f/dx^2:
  if (directFnASV[0] & 4)
    fnHessians[0] = 0.;

  return 0; // no failure
}


int DirectApplicInterface::side_impact_perf()
{
  if (numVars != 11 || numFns != 10) {
    Cerr << "Error: wrong number of inputs/outputs in side_impact_perf."
	 << std::endl;
    abort_handler(-1);
  }

  // **** f:
  if (directFnASV[0] & 1)
    fnVals[0] = 1.16 - 0.3717*xC[1]*xC[3] - 0.00931*xC[1]*xC[9]
              - 0.484*xC[2]*xC[8] + 0.01343*xC[5]*xC[9];
  if (directFnASV[1] & 1)
    fnVals[1] = 28.98 + 3.818*xC[2] - 4.2*xC[0]*xC[1] + 0.0207*xC[4]*xC[9]
              + 6.63*xC[5]*xC[8] - 7.7*xC[6]*xC[7] + 0.32*xC[8]*xC[9];
  if (directFnASV[2] & 1)
    fnVals[2] = 33.86 + 2.95*xC[2] + 0.1792*xC[9] - 5.057*xC[0]*xC[1]
              - 11*xC[1]*xC[7] - 0.0215*xC[4]*xC[9] - 9.98*xC[6]*xC[7]
              + 22*xC[7]*xC[8];
  if (directFnASV[3] & 1)
    fnVals[3] = 46.36 - 9.9*xC[1] - 12.9*xC[0]*xC[7] + 0.1107*xC[2]*xC[9];
  if (directFnASV[4] & 1)
    fnVals[4] = 0.261 - 0.0159*xC[0]*xC[1] - 0.188*xC[0]*xC[7]
              - 0.019*xC[1]*xC[6] + 0.0144*xC[2]*xC[4] + 0.0008757*xC[4]*xC[9]
              + 0.08045*xC[5]*xC[8] + 0.00139*xC[7]*xC[10]
              + 0.00001575*xC[9]*xC[10];
  if (directFnASV[5] & 1)
    fnVals[5] = 0.214 + 0.00817*xC[4] - 0.131*xC[0]*xC[7] - 0.0704*xC[0]*xC[8]
              + 0.03099*xC[1]*xC[5] - 0.018*xC[1]*xC[6] + 0.0208*xC[2]*xC[7]
              + 0.121*xC[2]*xC[8] - 0.00364*xC[4]*xC[5] + 0.0007715*xC[4]*xC[9]
              - 0.0005354*xC[5]*xC[9] + 0.00121*xC[7]*xC[10];
  if (directFnASV[6] & 1)
    fnVals[6] = 0.74 - 0.61*xC[1] - 0.163*xC[2]*xC[7] + 0.001232*xC[2]*xC[9]
              - 0.166*xC[6]*xC[8] + 0.227*xC[1]*xC[1];
  if (directFnASV[7] & 1)
    fnVals[7] = 4.72 - 0.5*xC[3] - 0.19*xC[1]*xC[2] - 0.0122*xC[3]*xC[9]
              + 0.009325*xC[5]*xC[9] + 0.000191*xC[10]*xC[10];
  if (directFnASV[8] & 1)
    fnVals[8] = 10.58 - 0.674*xC[0]*xC[1] - 1.95*xC[1]*xC[7]
              + 0.02054*xC[2]*xC[9] - 0.0198*xC[3]*xC[9] + 0.028*xC[5]*xC[9];
  if (directFnASV[9] & 1)
    fnVals[9] = 16.45 - 0.489*xC[2]*xC[6] - 0.843*xC[4]*xC[5]
              + 0.0432*xC[8]*xC[9] - 0.0556*xC[8]*xC[10]
              - 0.000786*xC[10]*xC[10];

  // **** df/dx and d^2f/dx^2:
  bool grad_flag = false, hess_flag = false;
  for (size_t i=0; i<numFns; ++i) {
    if (directFnASV[i] & 2) grad_flag = true;
    if (directFnASV[i] & 4) hess_flag = true;
  }
  if (grad_flag)
    Cerr << "Error: gradients not currently supported in side_impact_perf()."
	 << std::endl;
  if (hess_flag)
    Cerr << "Error: Hessians not currently supported in side_impact_perf()."
	 << std::endl;
  if (grad_flag || hess_flag)
    abort_handler(-1);

  return 0; // no failure
}


int DirectApplicInterface::steel_column_cost()
{
  if (numVars != 3 || numFns != 1) {
    Cerr << "Error: wrong number of inputs/outputs in steel_column_cost."
	 << std::endl;
    abort_handler(-1);
  }

  // In the steel column description in Kuschel & Rackwitz, Cost is _not_
  // defined as a random variable.  That is Cost is not a fn(B, D, H), but
  // is rather defined as a fn(b, d, h).  Since dCost/dX|_{X=mean} is not the
  // same as dCost/dmean for non-normal X (jacobian_dX_dS is not 1), dCost/dX
  // may not be used and an optional interface must be defined for Cost.

  // b  = xC[0] = flange breadth   (lognormal unc. var., mean is design var.)
  // d  = xC[1] = flange thickness (lognormal unc. var., mean is design var.)
  // h  = xC[2] = profile height   (lognormal unc. var., mean is design var.)

  Real b = xCM[VAR_b], d = xCM[VAR_d], h = xCM[VAR_h];

  // **** f (objective = bd + 5h = cost of column):
  if (directFnASV[0] & 1)
    fnVals[0] = b*d + 5.*h;

  // **** df/dx:
  if (directFnASV[0] & 2)
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_b:
	fnGrads[0][i] = d;  break;
      case VAR_d:
	fnGrads[0][i] = b;  break;
      case VAR_h:
	fnGrads[0][i] = 5.; break;
      }

  // **** d^2f/dx^2:
  if (directFnASV[0] & 4)
    for (size_t i=0; i<numDerivVars; ++i)
      for (size_t j=0; j<=i; ++j)
	fnHessians[0](i,j)
	  = ( (varTypeDVV[i] == VAR_b && varTypeDVV[j] == VAR_d) ||
	      (varTypeDVV[i] == VAR_d && varTypeDVV[j] == VAR_b) ) ? 1. : 0.;

  return 0; // no failure
}


int DirectApplicInterface::steel_column_perf()
{
  if (numVars != 9 || numFns != 1) {
    Cerr << "Error: wrong number of inputs/outputs in steel_column_perf."
	 << std::endl;
    abort_handler(-1);
  }

  // In the steel column description in Kuschel & Rackwitz, Cost is _not_
  // defined as a random variable.  That is Cost is not a fn(B, D, H), but
  // is rather defined as a fn(b, d, h).  Since dCost/dX|_{X=mean} is not the
  // same as dCost/dmean for non-normal X (jacobian_dX_dS is not 1), dCost/dX
  // may not be used and an optional interface must be defined for Cost.

  // set effective length s based on assumed boundary conditions
  // actual length of the column is 7500 mm
  Real s = 7500;

  // Fs = yield stress     (lognormal unc. var.)
  // P1 = dead weight load (normal    unc. var.)
  // P2 = variable load    (gumbel    unc. var.)
  // P3 = variable load    (gumbel    unc. var.)
  // B  = flange breadth   (lognormal unc. var., mean is design var.)
  // D  = flange thickness (lognormal unc. var., mean is design var.)
  // H  = profile height   (lognormal unc. var., mean is design var.)
  // F0 = init. deflection (normal    unc. var.)
  // E  = elastic modulus  (weibull   unc. var.)

  Real F0 = xCM[VAR_F0], B = xCM[VAR_B], D = xCM[VAR_D], H = xCM[VAR_H],
    Fs = xCM[VAR_Fs], E = xCM[VAR_E], P = xCM[VAR_P1]+xCM[VAR_P2]+xCM[VAR_P3],
    Pi = 3.1415926535897932385, Pi2 = Pi*Pi, Pi4 = Pi2*Pi2, Pi6 = Pi2*Pi4,
    B2 = B*B, D2 = D*D, H2 = H*H, H3 = H*H2, H5 = H2*H3, E2 = E*E, E3 = E*E2,
    s2 = s*s, X = Pi2*E*B*D*H2 - 2.*s2*P, X2 = X*X, X3 = X*X2;

  // **** g (limit state):
  if (directFnASV[0] & 1)
    fnVals[0] = Fs - P*(1./2./B/D + Pi2*F0*E*H/X);

  // **** dg/dx (w.r.t. active/uncertain variables):
  if (directFnASV[0] & 2)
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_F0: // df/dF0
	fnGrads[0][i] = -E*H*P*Pi2/X;                       break;
      case VAR_P1: case VAR_P2: case VAR_P3: // df/dP1, df/dP2, df/dP3
	fnGrads[0][i] = -1./2./B/D - B*D*E2*F0*H3*Pi4/X2;   break;
      case VAR_B: // df/dB
	fnGrads[0][i] = P*(1./2./B2/D + D*E2*F0*H3*Pi4/X2); break;
      case VAR_D: // df/dD
	fnGrads[0][i] = P*(1./2./B/D2 + B*E2*F0*H3*Pi4/X2); break;
      case VAR_H: // df/dH
	fnGrads[0][i] = E*F0*P*Pi2*(X + 4.*P*s2)/X2;        break;
      case VAR_Fs: // df/dFs
	fnGrads[0][i] = 1.;	                            break;
      case VAR_E: // df/dE
	fnGrads[0][i] = 2.*F0*H*P*P*Pi2*s2/X2;	            break;
      }

  // **** d^2g/dx^2: (SORM)
  if (directFnASV[0] & 4)
    for (size_t i=0; i<numDerivVars; ++i)
      for (size_t j=0; j<=i; ++j)
	if (varTypeDVV[i] == VAR_Fs || varTypeDVV[j] == VAR_Fs)
	  fnHessians[0](i,j) = 0.;
	else if ( (varTypeDVV[i] == VAR_P1 && varTypeDVV[j] == VAR_P1) ||
                  (varTypeDVV[i] == VAR_P1 && varTypeDVV[j] == VAR_P2) ||
                  (varTypeDVV[i] == VAR_P1 && varTypeDVV[j] == VAR_P3) ||
		  (varTypeDVV[i] == VAR_P2 && varTypeDVV[j] == VAR_P1) ||
		  (varTypeDVV[i] == VAR_P2 && varTypeDVV[j] == VAR_P2) ||
		  (varTypeDVV[i] == VAR_P2 && varTypeDVV[j] == VAR_P3) ||
		  (varTypeDVV[i] == VAR_P3 && varTypeDVV[j] == VAR_P1) ||
		  (varTypeDVV[i] == VAR_P3 && varTypeDVV[j] == VAR_P2) ||
		  (varTypeDVV[i] == VAR_P3 && varTypeDVV[j] == VAR_P3) )
	  fnHessians[0](i,j) = -4.*B*D*E2*F0*H3*Pi4*s2/X3;
	else if ( (varTypeDVV[i] == VAR_P1 && varTypeDVV[j] == VAR_B) ||
 		  (varTypeDVV[i] == VAR_P2 && varTypeDVV[j] == VAR_B) ||
		  (varTypeDVV[i] == VAR_P3 && varTypeDVV[j] == VAR_B) ||
		  (varTypeDVV[i] == VAR_B  && varTypeDVV[j] == VAR_P1) ||
		  (varTypeDVV[i] == VAR_B  && varTypeDVV[j] == VAR_P2) ||
		  (varTypeDVV[i] == VAR_B  && varTypeDVV[j] == VAR_P3) )
	  fnHessians[0](i,j)
	    = 1./2./B2/D + D*E2*F0*H3*Pi4/X2*(2.*B*D*E*H2*Pi2/X - 1.);
	else if ( (varTypeDVV[i] == VAR_P1 && varTypeDVV[j] == VAR_D) ||
 		  (varTypeDVV[i] == VAR_P2 && varTypeDVV[j] == VAR_D) ||
		  (varTypeDVV[i] == VAR_P3 && varTypeDVV[j] == VAR_D) ||
		  (varTypeDVV[i] == VAR_D  && varTypeDVV[j] == VAR_P1) ||
		  (varTypeDVV[i] == VAR_D  && varTypeDVV[j] == VAR_P2) ||
		  (varTypeDVV[i] == VAR_D  && varTypeDVV[j] == VAR_P3) )
	  fnHessians[0](i,j)
	    = 1./2./B/D2 + B*E2*F0*H3*Pi4/X2*(2.*B*D*E*H2*Pi2/X - 1.);
	else if ( (varTypeDVV[i] == VAR_P1 && varTypeDVV[j] == VAR_H) ||
 		  (varTypeDVV[i] == VAR_P2 && varTypeDVV[j] == VAR_H) ||
		  (varTypeDVV[i] == VAR_P3 && varTypeDVV[j] == VAR_H) ||
		  (varTypeDVV[i] == VAR_H  && varTypeDVV[j] == VAR_P1) ||
		  (varTypeDVV[i] == VAR_H  && varTypeDVV[j] == VAR_P2) ||
		  (varTypeDVV[i] == VAR_H  && varTypeDVV[j] == VAR_P3) )
	  fnHessians[0](i,j) = B*D*E2*F0*H2*Pi4*(X+8.*P*s2)/X3;
	else if ( (varTypeDVV[i] == VAR_P1 && varTypeDVV[j] == VAR_F0) ||
 		  (varTypeDVV[i] == VAR_P2 && varTypeDVV[j] == VAR_F0) ||
		  (varTypeDVV[i] == VAR_P3 && varTypeDVV[j] == VAR_F0) ||
		  (varTypeDVV[i] == VAR_F0 && varTypeDVV[j] == VAR_P1) ||
		  (varTypeDVV[i] == VAR_F0 && varTypeDVV[j] == VAR_P2) ||
		  (varTypeDVV[i] == VAR_F0 && varTypeDVV[j] == VAR_P3) )
	  fnHessians[0](i,j) = -B*D*E2*H3*Pi4/X2;
	else if ( (varTypeDVV[i] == VAR_P1 && varTypeDVV[j] == VAR_E) ||
 		  (varTypeDVV[i] == VAR_P2 && varTypeDVV[j] == VAR_E) ||
		  (varTypeDVV[i] == VAR_P3 && varTypeDVV[j] == VAR_E) ||
		  (varTypeDVV[i] == VAR_E  && varTypeDVV[j] == VAR_P1) ||
		  (varTypeDVV[i] == VAR_E  && varTypeDVV[j] == VAR_P2) ||
		  (varTypeDVV[i] == VAR_E  && varTypeDVV[j] == VAR_P3) )
	  fnHessians[0](i,j) = 4.*B*D*E*F0*H3*P*Pi4*s2/X3;
	else if (varTypeDVV[i] == VAR_B && varTypeDVV[j] == VAR_B)
	  fnHessians[0](i,j) = -P*(1./B2/B/D + 2.*D2*E3*F0*H5*Pi6/X3);
	else if ( (varTypeDVV[i] == VAR_B && varTypeDVV[j] == VAR_D) ||
		  (varTypeDVV[i] == VAR_D && varTypeDVV[j] == VAR_B) )
	  fnHessians[0](i,j)
	    = -P*(1./2./B2/D2 + E2*F0*H3*Pi4/X2*(2.*B*D*E*H2*Pi2/X - 1.));
	else if ( (varTypeDVV[i] == VAR_B && varTypeDVV[j] == VAR_H) ||
		  (varTypeDVV[i] == VAR_H && varTypeDVV[j] == VAR_B) )
	  fnHessians[0](i,j) = -D*E2*F0*H2*P*Pi4*(X + 8.*P*s2)/X3;
	else if ( (varTypeDVV[i] == VAR_F0 && varTypeDVV[j] == VAR_B) ||
		  (varTypeDVV[i] == VAR_B  && varTypeDVV[j] == VAR_F0) )
	  fnHessians[0](i,j) = D*E2*H3*P*Pi4/X2;
	else if ( (varTypeDVV[i] == VAR_B && varTypeDVV[j] == VAR_E) ||
		  (varTypeDVV[i] == VAR_E && varTypeDVV[j] == VAR_B) )
	  fnHessians[0](i,j) = -4.*D*E*F0*H3*P*P*Pi4*s2/X3;
	else if (varTypeDVV[i] == VAR_D && varTypeDVV[j] == VAR_D)
	  fnHessians[0](i,j) = -P*(1./B/D2/D + 2.*B2*E3*F0*H5*Pi6/X3);
	else if ( (varTypeDVV[i] == VAR_D && varTypeDVV[j] == VAR_H) ||
		  (varTypeDVV[i] == VAR_H && varTypeDVV[j] == VAR_D) )
	  fnHessians[0](i,j) = -B*E2*F0*H2*P*Pi4*(X + 8.*P*s2)/X3;
	else if ( (varTypeDVV[i] == VAR_F0 && varTypeDVV[j] == VAR_D) ||
		  (varTypeDVV[i] == VAR_D  && varTypeDVV[j] == VAR_F0) )
	  fnHessians[0](i,j) = B*E2*H3*P*Pi4/X2;
	else if ( (varTypeDVV[i] == VAR_D && varTypeDVV[j] == VAR_E) ||
		  (varTypeDVV[i] == VAR_E && varTypeDVV[j] == VAR_D) )
	  fnHessians[0](i,j) = -4.*B*E*F0*H3*P*P*Pi4*s2/X3;
	else if (varTypeDVV[i] == VAR_H && varTypeDVV[j] == VAR_H)
	  fnHessians[0](i,j) = -2.*B*D*E2*F0*H*P*Pi4*(X + 8.*P*s2)/X3;
	else if ( (varTypeDVV[i] == VAR_F0 && varTypeDVV[j] == VAR_H) ||
		  (varTypeDVV[i] == VAR_H  && varTypeDVV[j] == VAR_F0) )
	  fnHessians[0](i,j) = E*P*Pi2*(X + 4.*P*s2)/X2;
	else if ( (varTypeDVV[i] == VAR_H && varTypeDVV[j] == VAR_E) ||
		  (varTypeDVV[i] == VAR_E && varTypeDVV[j] == VAR_H) )
	  fnHessians[0](i,j) = -2.*F0*P*P*Pi2*s2*(3.*X + 8.*P*s2)/X3;
	else if (varTypeDVV[i] == VAR_F0 && varTypeDVV[j] == VAR_F0)
	  fnHessians[0](i,j) = 0.;
	else if ( (varTypeDVV[i] == VAR_F0 && varTypeDVV[j] == VAR_E) ||
		  (varTypeDVV[i] == VAR_E  && varTypeDVV[j] == VAR_F0) )
	  fnHessians[0](i,j) = 2.*H*P*P*Pi2*s2/X2;
	else if (varTypeDVV[i] == VAR_E && varTypeDVV[j] == VAR_E)
	  fnHessians[0](i,j) = -4.*B*D*F0*H3*P*P*Pi4*s2/X3;
	else { // unsupported derivative
	  Cerr << "Error: unsupported Hessian cross term in steel_column."
	       << std::endl;
	  abort_handler(-1);
	}

  return 0; // no failure
}


int DirectApplicInterface::sobol_rational()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: sobol_rational direct fn does not support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  if (numVars != 2 || numFns != 1) {
    Cerr << "Error: Bad number of inputs/outputs in sobol_rational direct fn."
	 << std::endl;
    abort_handler(-1);
  }

  // f = (x2 + 0.5)^4 / (x1 + 0.5)^2 
  // See Storlie et al. SAND2008-6570

  const Real& x1 = xC[0]; const Real& x2 = xC[1];

  // **** f: 
  if (directFnASV[0] & 1)
    fnVals[0] = std::pow((x2 + 0.5), 4.) / std::pow((x1 + 0.5), 2.);

  // **** df/dx:
  if (directFnASV[0] & 2)
    for (size_t i=0; i<numDerivVars; ++i)
      switch (directFnDVV[i]) {
      case 1: // x1
	fnGrads[0][i] = -2.*std::pow(x2 + 0.5, 4.)/std::pow(x1 + 0.5, 3.);
	break;
      case 2: // x2
	fnGrads[0][i] =  4.*std::pow(x2 + 0.5, 3.)/std::pow(x1 + 0.5, 2.);
	break;
      }

  return 0; // no failure
}


int DirectApplicInterface::sobol_g_function()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: sobol_g_function direct fn does not support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  if (numVars < 1 || numVars > 10 || numFns != 1) {
    Cerr << "Error: Bad number of inputs/outputs in sobol_g_function direct fn."
	 << std::endl;
    abort_handler(-1);
  }

  // Sobol g-Function: see Storlie et al. SAND2008-6570
  int a[] = {0,1,2,4,8,99,99,99,99,99};

  // **** f: 
  if (directFnASV[0] & 1) {
    fnVals[0] = 2.;
    for (int i=0; i<numVars; ++i)
      fnVals[0] *= ( std::abs(4.*xC[i] - 2.) + a[i] ) / ( 1. + a[i] );	
  }
	
  // **** df/dx:
  if (directFnASV[0] & 2) {
    for (size_t i=0; i<numDerivVars; ++i) {
      size_t var_index = directFnDVV[i] - 1;
      Real& fn_grad_0i = fnGrads[0][i];
      if (4.*xC[var_index] == 2.) // zero gradient assumed at discontinuity
	fn_grad_0i = 0.;
      else {
	fn_grad_0i = (4.*xC[var_index] > 2.) ?
	  8. / ( 1. + a[var_index] ) : -8. / ( 1. + a[var_index] );
	for (int j=0; j<numVars; ++j)
	  if (j != var_index)
	    fn_grad_0i *= ( std::abs(4.*xC[j] - 2.) + a[j] ) / ( 1. + a[j] );
      }
    }
  }

  return 0; // no failure
}


int DirectApplicInterface::sobol_ishigami()
{
  using std::pow;
  using std::sin;
  using std::cos;

  if (multiProcAnalysisFlag) {
    Cerr << "Error: sobol_ishigami direct fn does not support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  if (numVars != 3 || numFns != 1) {
    Cerr << "Error: Bad number of inputs/outputs in sobol_ishigami direct fn."
	 << std::endl;
    abort_handler(-1);
  }

  // Ishigami Function: see Storlie et al. SAND2008-6570
  const Real pi = 3.14159265358979324;
  Real x1 = xCM[VAR_x1], x2 = xCM[VAR_x2], x3 = xCM[VAR_x3];

  // **** f: 
  if (directFnASV[0] & 1)
    fnVals[0] = ( 1. + 0.1 * pow(2.*pi*x3 - pi, 4.0) ) *
      sin(2.*pi*x1 - pi) + 7. * pow(sin(2*pi*x2 - pi), 2.0);

  // **** df/dx:
  if (directFnASV[0] & 2)
    for (size_t i=0; i<numDerivVars; ++i)
      switch (varTypeDVV[i]) {
      case VAR_x1:
	fnGrads[0][i]
	  = 2.*pi * ( 1. + 0.1*pow(2.*pi*x3 - pi, 4.) ) * cos(2.*pi*x1 - pi);
	break;
      case VAR_x2:
	fnGrads[0][i] = 28.*pi * sin(2.*pi*x2 - pi) * cos(2.*pi*x2 - pi);
	break;
      case VAR_x3:
	fnGrads[0][i] = 0.8 * pow(2.*pi*x3 - pi, 3.) * sin(2.*pi*x1 - pi);
	break;
      }

  return 0; // no failure
}


int DirectApplicInterface::text_book()
{
  // This version is used when multiple analysis drivers are not employed.
  // In this case, execute text_book1/2/3 sequentially.

  if (numFns > 3) {
    Cerr << "Error: Bad number of functions in text_book direct fn."
	 << std::endl;
    abort_handler(-1);
  }
  // The presence of discrete variables can cause offsets in directFnDVV which
  // the text_book derivative logic does not currently account for.
  if ( (gradFlag || hessFlag) && (numADIV || numADRV) ) {
    Cerr << "Error: text_book direct fn assumes no discrete variables in "
	 << "derivative mode." << std::endl;
    abort_handler(-1);
  }

  text_book1(); // objective fn val/grad/Hessian
  if (numFns > 1)
    text_book2(); // constraint 1 val/grad/Hessian
  if (numFns > 2)
    text_book3(); // constraint 2 val/grad/Hessian

  // Test failure capturing for Direct case
  //int r = rand();
  //if (r < 10000) // RAND_MAX = 32767
  //  return 1; // failure

  //sleep(5); // for faking a more expensive evaluation
  return 0; // no failure
}


// text_book1/2/3 are used when evalComm is split into multiple analysis
// servers.  In this case, the 3 portions are executed in parallel.
int DirectApplicInterface::text_book1()
{
  // **********************************
  // **** f: sum (x[i] - POWVAL)^4 ****
  // **********************************
  size_t i;
  if (directFnASV[0] & 1) {
    Real local_val = 0.0;
    for (i=analysisCommRank; i<numVars; i+=analysisCommSize) {
      // orders all continuous vars followed by all discrete vars.  This is 
      // fine in the direct case so long as everything is self-consistent.
      Real x_i;
      if (i<numACV)
	x_i = xC[i];
      else if (i<numACV+numADIV)
	x_i = (Real)xDI[i-numACV];
      else
	x_i = xDR[i-numACV-numADIV];
      local_val += std::pow(x_i-POW_VAL, 4);
#ifdef TB_EXPENSIVE
      // MDO98/WCSMO99 TFLOP/NOW timings: j<=15000 used for fnVals[0] only
      //   (text_book1 only)
      // StrOpt_2002 TFLOP timings: j<=5000 used for all fnVals, fnGrads, and
      //   fnHessians that are present (text_book1/2/3)
      for (size_t j=1; j<=5000; ++j)
        local_val += 1./(std::pow(x_i-POW_VAL,4)+j/100.)
	               /(std::pow(x_i-POW_VAL,4)+j/100.);
#endif // TB_EXPENSIVE
    }

    if (multiProcAnalysisFlag) {
      Real global_val = 0.0;
      parallelLib.reduce_sum_a(&local_val, &global_val, 1);
      // only analysisCommRank 0 has the correct sum.  This is OK (MPI_Allreduce
      // not needed) since only analysisCommRank 0 updates response for 
      // evalCommRank 0 in overlay_response.  evalCommRank 0 then returns the 
      // results to the iterator in ApplicationInterface::serve_evaluations().
      if (analysisCommRank == 0)
	fnVals[0] = global_val;
    }
    else
      fnVals[0] = local_val;
  }

  // ****************
  // **** df/dx: ****
  // ****************
  if (directFnASV[0] & 2) {
    //for (i=0; i<numDerivVars; ++i)
    //  fnGrads[0][i] = 4.*pow(xC[i]-POW_VAL,3);
    std::fill_n(fnGrads[0], fnGrads.numRows(), 0.);
    for (i=analysisCommRank; i<numDerivVars; i+=analysisCommSize) {
      size_t var_index = directFnDVV[i] - 1; // assumes no discrete vars
      Real x_i = xC[var_index]; // assumes no discrete vars
      fnGrads[0][i] = 4.*std::pow(x_i-POW_VAL,3);
#ifdef TB_EXPENSIVE
      for (size_t j=1; j<=5000; ++j)
        fnGrads[0][i] += 1./(std::pow(x_i-POW_VAL,3)+j/100.)
                           /(std::pow(x_i-POW_VAL,3)+j/100.);
#endif // TB_EXPENSIVE
    }

    if (multiProcAnalysisFlag) {
      Real* sum_fns = (analysisCommRank) ? NULL : new Real [numDerivVars];
      parallelLib.reduce_sum_a((Real*)fnGrads[0], sum_fns, 
			       numDerivVars);
      if (analysisCommRank == 0) {
	RealVector fn_grad_col_vec = Teuchos::getCol(Teuchos::View, fnGrads, 0);
	copy_data(sum_fns, (int)numDerivVars, fn_grad_col_vec);
	delete [] sum_fns;
      }
    }
  }

  // ********************
  // **** d^2f/dx^2: ****
  // ********************
  if (directFnASV[0] & 4) {
    fnHessians[0] = 0.;
    //for (i=0; i<numDerivVars; ++i)
    //  fnHessians[0][i][i] = 12.*pow(xC[i]-POW_VAL,2);
    for (i=analysisCommRank; i<numDerivVars; i+=analysisCommSize) {
      size_t var_index = directFnDVV[i] - 1; // assumes no discrete vars
      Real x_i = xC[var_index]; // assumes no discrete vars
      fnHessians[0](i,i) = 12.*std::pow(x_i-POW_VAL,2);
#ifdef TB_EXPENSIVE
      for (size_t j=0; j<numDerivVars; ++j)
	for (size_t k=1; k<=5000; ++k)
	  fnHessians[0](i,j) += 1./(std::pow(x_i-POW_VAL,2)+k/100.)
                                   /(std::pow(x_i-POW_VAL,2)+k/100.);
#endif // TB_EXPENSIVE
    }

    if (multiProcAnalysisFlag) {
      int num_reals = numDerivVars * numDerivVars;
      Real* local_fns = new Real [num_reals];
      std::copy(fnHessians[0].values(), fnHessians[0].values() + num_reals,
                local_fns);
      Real* sum_fns = (analysisCommRank) ? NULL : new Real [num_reals];
      parallelLib.reduce_sum_a(local_fns, sum_fns, num_reals);
      delete [] local_fns;
      if (analysisCommRank == 0) {
	std::copy(sum_fns, sum_fns + num_reals, fnHessians[0].values());
	delete [] sum_fns;
      }
    }
  }

  //sleep(1);
  return 0;
}


int DirectApplicInterface::text_book2()
{
  // **********************************
  // **** c1: x[0]*x[0] - 0.5*x[1] ****
  // **********************************
  size_t i;
  if (directFnASV[1] & 1) {
    Real local_val = 0.0;
    // Definitely not the most efficient way to do this, but the point is to
    // demonstrate Comm communication.
    for (i=analysisCommRank; i<numVars; i+=analysisCommSize) {
      // orders all continuous vars followed by all discrete vars.  This is 
      // fine in the direct case so long as everything is self-consistent.
      Real x_i;
      if (i<numACV)
	x_i = xC[i];
      else if (i<numACV+numADIV)
	x_i = (Real)xDI[i-numACV];
      else
	x_i = xDR[i-numACV-numADIV];
      if (i==0) // could be changed to i % 2 == 0 to get even vars.
        local_val += x_i*x_i;
      else if (i==1) // could be changed to i % 2 == 1 to get odd vars
        local_val -= 0.5*x_i;
#ifdef TB_EXPENSIVE
      for (size_t j=1; j<=5000; ++j)
        local_val += 1./(std::pow(x_i-POW_VAL,4)+j/100.)
	               /(std::pow(x_i-POW_VAL,4)+j/100.);
#endif // TB_EXPENSIVE
    }

    if (multiProcAnalysisFlag) {
      Real global_val = 0.0;
      parallelLib.reduce_sum_a(&local_val, &global_val, 1);
      // only analysisCommRank 0 has the correct sum.  This is OK (MPI_Allreduce
      // not needed) since only analysisCommRank 0 updates response for 
      // evalCommRank 0 in overlay_response.  evalCommRank 0 then returns the 
      // results to the iterator in ApplicationInterface::serve_evaluations().
      if (analysisCommRank == 0)
	fnVals[1] = global_val;
    }
    else
      fnVals[1] = local_val;
  }

  // *****************
  // **** dc1/dx: ****
  // *****************
  if (directFnASV[1] & 2) {
    std::fill_n(fnGrads[1], fnGrads.numRows(), 0.);

    //fnGrads[1][0] = 2.*xC[0];
    //fnGrads[1][1] = -0.5;
    for (i=analysisCommRank; i<numDerivVars; i+=analysisCommSize) {
      size_t var_index = directFnDVV[i] - 1; // assumes no discrete vars
      if (var_index == 0)
        fnGrads[1][i] = 2.*xC[0];
      else if (var_index == 1)
        fnGrads[1][i] = -0.5;
#ifdef TB_EXPENSIVE
      Real x_i = xC[var_index];
      for (size_t j=1; j<=5000; ++j)
        fnGrads[1][i] += 1./(std::pow(x_i-POW_VAL,3)+j/100.)
                           /(std::pow(x_i-POW_VAL,3)+j/100.);
#endif // TB_EXPENSIVE
    }

    if (multiProcAnalysisFlag) {
      Real* sum_fns = (analysisCommRank) ? NULL : new Real [numDerivVars];
      parallelLib.reduce_sum_a((Real*)fnGrads[1], sum_fns, 
			       numDerivVars);
      if (analysisCommRank == 0) {
	RealVector fn_grad_col_vec = Teuchos::getCol(Teuchos::View, fnGrads, 1);
	copy_data(sum_fns, (int)numDerivVars, fn_grad_col_vec);
	delete [] sum_fns;
      }
    }
  }

  // *********************
  // **** d^2c1/dx^2: ****
  // *********************
  if (directFnASV[1] & 4) {
    fnHessians[1] = 0.;
    //fnHessians[1][0][0] = 2.;
    for (i=analysisCommRank; i<numDerivVars; i+=analysisCommSize) {
      size_t var_index = directFnDVV[i] - 1; // assumes no discrete vars
      if (var_index == 0)
	fnHessians[1](i,i) = 2.;
#ifdef TB_EXPENSIVE
      Real x_i = xC[var_index];
      for (size_t j=0; j<numDerivVars; ++j)
	for (size_t k=1; k<=5000; ++k)
	  fnHessians[1](i,j) += 1./(std::pow(x_i-POW_VAL,2)+k/100.)
                                   /(std::pow(x_i-POW_VAL,2)+k/100.);
#endif // TB_EXPENSIVE
    }

    if (multiProcAnalysisFlag) {
      int num_reals = numDerivVars * numDerivVars;
      Real* local_fns = new Real [num_reals];
      std::copy(fnHessians[1].values(), fnHessians[1].values() + num_reals,
                local_fns);
      Real* sum_fns = (analysisCommRank) ? NULL : new Real [num_reals];
      parallelLib.reduce_sum_a(local_fns, sum_fns, num_reals);
      delete [] local_fns;
      if (analysisCommRank == 0) {
        std::copy( sum_fns, sum_fns + num_reals, fnHessians[1].values() );
	delete [] sum_fns;
      }
    }
  }

  //sleep(1);
  return 0;
}


int DirectApplicInterface::text_book3()
{
  // **********************************
  // **** c2: x[1]*x[1] - 0.5*x[0] ****
  // **********************************
  size_t i;
  if (directFnASV[2] & 1) {
    Real local_val = 0.0;
    // Definitely not the most efficient way to do this, but the point is to
    // demonstrate Comm communication.
    for (i=analysisCommRank; i<numVars; i+=analysisCommSize) {
      // orders all continuous vars followed by all discrete vars.  This is 
      // fine in the direct case so long as everything is self-consistent.
      Real x_i;
      if (i<numACV)
	x_i = xC[i];
      else if (i<numACV+numADIV)
	x_i = (Real)xDI[i-numACV];
      else
	x_i = xDR[i-numACV-numADIV];
      if (i==0) // could be changed to i % 2 == 0 to get even vars.
        local_val -= 0.5*x_i;
      else if (i==1) // could be changed to i % 2 == 1 to get odd vars
        local_val += x_i*x_i;
#ifdef TB_EXPENSIVE
      for (size_t j=1; j<=5000; ++j)
        local_val += 1./(std::pow(x_i-POW_VAL,4)+j/100.)
	               /(std::pow(x_i-POW_VAL,4)+j/100.);
#endif // TB_EXPENSIVE
    }

    if (multiProcAnalysisFlag) {
      Real global_val = 0.0;
      parallelLib.reduce_sum_a(&local_val, &global_val, 1);
      // only analysisCommRank 0 has the correct sum.  This is OK (MPI_Allreduce
      // not needed) since only analysisCommRank 0 updates response for 
      // evalCommRank 0 in overlay_response.  evalCommRank 0 then returns the 
      // results to the iterator in ApplicationInterface::serve_evaluations().
      if (analysisCommRank == 0)
	fnVals[2] = global_val;
    }
    else
      fnVals[2] = local_val;
  }

  // *****************
  // **** dc2/dx: ****
  // *****************
  if (directFnASV[2] & 2) {
    std::fill_n(fnGrads[2], fnGrads.numRows(), 0.);

    //fnGrads[2][0] = -0.5;
    //fnGrads[2][1] = 2.*xC[1];
    for (i=analysisCommRank; i<numDerivVars; i+=analysisCommSize) {
      size_t var_index = directFnDVV[i] - 1; // assumes no discrete vars
      if (var_index == 0)
        fnGrads[2][i] = -0.5;
      else if (var_index == 1)
        fnGrads[2][i] = 2.*xC[1];
#ifdef TB_EXPENSIVE
      Real x_i = xC[var_index];
      for (size_t j=1; j<=5000; ++j)
        fnGrads[2][i] += 1./(std::pow(x_i-POW_VAL,3)+j/100.)
                           /(std::pow(x_i-POW_VAL,3)+j/100.);
#endif // TB_EXPENSIVE
    }

    if (multiProcAnalysisFlag) {
      Real* sum_fns = (analysisCommRank) ? NULL : new Real [numDerivVars];
      parallelLib.reduce_sum_a((Real*)fnGrads[2], sum_fns, 
			       numDerivVars);
      if (analysisCommRank == 0) {
	RealVector fn_grad_col_vec = Teuchos::getCol(Teuchos::View, fnGrads, 2);
	copy_data(sum_fns, (int)numDerivVars, fn_grad_col_vec);
	delete [] sum_fns;
      }
    }
  }

  // *********************
  // **** d^2c2/dx^2: ****
  // *********************
  if (directFnASV[2] & 4) {
    fnHessians[2] = 0.;
    //fnHessians[2][1][1] = 2.;
    for (i=analysisCommRank; i<numDerivVars; i+=analysisCommSize) {
      size_t var_index = directFnDVV[i] - 1; // assumes no discrete vars
      if (var_index == 1)
	fnHessians[2](i,i) = 2.;
#ifdef TB_EXPENSIVE
      Real x_i = xC[var_index];
      for (size_t j=0; j<numDerivVars; ++j)
	for (size_t k=1; k<=5000; ++k)
	  fnHessians[2](i,j) += 1./(std::pow(x_i-POW_VAL,2)+k/100.)
                                   /(std::pow(x_i-POW_VAL,2)+k/100.);
#endif // TB_EXPENSIVE
    }

    if (multiProcAnalysisFlag) {
      int num_reals = numDerivVars * numDerivVars;
      Real* local_fns = new Real [num_reals];
      std::copy(fnHessians[2].values(), fnHessians[2].values() + num_reals,
                local_fns);
      Real* sum_fns = (analysisCommRank) ? NULL : new Real [num_reals];
      parallelLib.reduce_sum_a(local_fns, sum_fns, num_reals);
      delete [] local_fns;
      if (analysisCommRank == 0) {
	std::copy(sum_fns, sum_fns + num_reals, fnHessians[2].values());
	delete [] sum_fns;
      }
    }
  }

  //sleep(1);
  return 0;
}


int DirectApplicInterface::text_book_ouu()
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: text_book_ouu direct fn does not support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }
  // typical usage is 2 design vars + 6 uncertain variables, although the
  // number of uncertain variables can be any factor of two.
  if (numVars < 4 || numVars % 2 || numADIV || numADRV) {
    Cerr << "Error: Bad number of variables in text_book_ouu direct fn."
	 << std::endl;
    abort_handler(-1);
  }
  if (numFns > 3) {
    Cerr << "Error: Bad number of functions in text_book_ouu direct fn."
	 << std::endl;
    abort_handler(-1);
  }
  if (hessFlag) {
    Cerr << "Error: Hessians not supported in text_book_ouu direct fn."
	 << std::endl;
    abort_handler(-1);
  }

  size_t i, split = 2 + (numVars - 2)/2; // split the uncertain vars among d1,d2
  // xC[0], xC[1]     = design
  // xC[2], xC[3],... = uncertain

  // **** f:
  if (directFnASV[0] & 1) {
    Real f = 0.;
    for(i=2; i<split; ++i)
      f += std::pow(xC[i]-10.*xC[0], 4.0);
    for(i=split; i<numVars; ++i)
      f += std::pow(xC[i]-10.*xC[1], 4.0);
    fnVals[0] = f;
  }

  // **** c1:
  if (numFns>1 && (directFnASV[1] & 1))
    fnVals[1] = xC[0]*(xC[2]*xC[2] - 0.5*xC[3]);

  // **** c2:
  if (numFns>2 && (directFnASV[2] & 1))
    fnVals[2] = xC[1]*(xC[3]*xC[3] - 0.5*xC[2]);

  // **** df/dx:
  if (directFnASV[0] & 2)
    for (i=0; i<numDerivVars; ++i)
      switch (directFnDVV[i]) {
      case 1: { // design variable derivative
	Real f0 = 0., xC0 = xC[0];
	for (size_t j=2; j<split; ++j)
	  f0 += -40.*std::pow(xC[j]-10.*xC0, 3.0);
	fnGrads[0][i] = f0;
	break;
      }
      case 2: { // design variable derivative
	Real f1 = 0., xC1 = xC[1];
	for (size_t j=split; j<numVars; ++j)
	  f1 += -40.*std::pow(xC[j]-10.*xC1, 3.0);
	fnGrads[0][i] = f1;
	break;
      }
      default: { // uncertain variable derivative
	size_t var_index = directFnDVV[i] - 1;
	Real xCvi = xC[var_index];
	fnGrads[0][i] = (var_index<split) ? 4.*std::pow(xCvi-10.*xC[0], 3)
                                          : 4.*std::pow(xCvi-10.*xC[1], 3);
	break;
      }
      }

  // **** dc1/dx:
  if (numFns>1 && (directFnASV[1] & 2))
    for (i=0; i<numDerivVars; ++i)
      switch (directFnDVV[i]) {
      case 1: // design variable derivative
	fnGrads[1][i] = xC[2]*xC[2] - 0.5*xC[3]; break;
      case 3: // uncertain variable derivative
	fnGrads[1][i] = 2*xC[0]*xC[2];           break;
      case 4: // uncertain variable derivative
	fnGrads[1][i] = -0.5*xC[0];              break;
      default: // all other derivatives
	fnGrads[1][i] = 0.;                      break;
      }

  // **** dc2/dx:
  if (numFns>2 && (directFnASV[2] & 2))
    for (i=0; i<numDerivVars; ++i)
      switch (directFnDVV[i]) {
      case 2: // design variable derivative
	fnGrads[2][i] = xC[3]*xC[3] - 0.5*xC[2]; break;
      case 3: // uncertain variable derivative
	fnGrads[2][i] = -0.5*xC[1];              break;
      case 4: // uncertain variable derivative
	fnGrads[2][i] = 2*xC[1]*xC[3];           break;
      default: // all other derivative
	fnGrads[2][i] = 0.;                      break;
      }

  //sleep(5); // for faking a more expensive evaluation
  return 0; // no failure
}


int DirectApplicInterface::scalable_text_book()
{
  if (numADIV || numADRV) {
    Cerr << "Error: scalable_text_book direct fn does not support discrete "
	 << "variables." << std::endl;
    abort_handler(-1);
  }

  // **********************************
  // **** f: sum (x[i] - POWVAL)^4 ****
  // **********************************
  size_t i, j;
  if (directFnASV[0] & 1) {
    fnVals[0] = 0.;
    for (i=0; i<numACV; ++i)
      fnVals[0] += std::pow(xC[i]-POW_VAL, 4);
  }

  // ****************
  // **** df/dx: ****
  // ****************
  if (directFnASV[0] & 2) {
    std::fill_n(fnGrads[0], fnGrads.numRows(), 0.);
    for (i=0; i<numDerivVars; ++i) {
      size_t var_index = directFnDVV[i] - 1;
      fnGrads[0][i] = 4.*std::pow(xC[var_index]-POW_VAL,3);
    }
  }

  // ********************
  // **** d^2f/dx^2: ****
  // ********************
  if (directFnASV[0] & 4) {
    fnHessians[0] = 0.;
    for (i=0; i<numDerivVars; ++i) {
      size_t var_index = directFnDVV[i] - 1;
      fnHessians[0](i,i) = 12.*std::pow(xC[var_index]-POW_VAL,2);
    }
  }

  // *********************
  // **** constraints ****
  // *********************
  // "symmetric" constraint pairs are defined from pairs of variables
  // (although odd constraint or variable counts are also allowable):
  // for i=1:num_fns-1, c[i] = x[i-1]^2 - x[i]/2    for  odd i 
  //                    c[i] = x[i-1]^2 - x[i-2]/2  for even i
  for (i=1; i<numFns; ++i) {
    // ************
    // **** c: ****
    // ************
    if (directFnASV[i] & 1) {
      fnVals[i] = (i-1 < numACV) ? xC[i-1]*xC[i-1] : 0.;
      if (i%2) //  odd constraint
	{ if (i   < numACV) fnVals[i] -= xC[i]/2.; }
      else     // even constraint
	{ if (i-2 < numACV) fnVals[i] -= xC[i-2]/2.; }
    }
    // ****************
    // **** dc/dx: ****
    // ****************
    if (directFnASV[i] & 2) {
      std::fill_n(fnGrads[i], fnGrads.numRows(), 0.);
      for (j=0; j<numDerivVars; ++j) {
	size_t var_index = directFnDVV[j] - 1;
	if (i-1 < numACV && var_index == i-1) // both constraints
	  fnGrads[i][j] = 2.*xC[i-1];
	else if (i%2)         //  odd constraint
	  { if (i   < numACV && var_index == i)   fnGrads[i][j] = -0.5; }
	else                  // even constraint
	  { if (i-2 < numACV && var_index == i-2) fnGrads[i][j] = -0.5; }
      }
    }
    // ********************
    // **** d^2c/dx^2: ****
    // ********************
    if (directFnASV[i] & 4) {
      fnHessians[i] = 0.;
      if (i-1 < numACV)
	for (j=0; j<numDerivVars; ++j)
	  if (directFnDVV[j] == i) // both constraints
	    fnHessians[i](j,j) = 2.;
    }
  }

  //sleep(5); // for faking a more expensive evaluation
  return 0; // no failure
}


int DirectApplicInterface::scalable_monomials()
{
  if (numADIV || numADRV) {
    Cerr << "Error: scalable_monomials direct fn does not support discrete "
	 << "variables." << std::endl;
    abort_handler(-1);
  }
  if (numFns != 1) {
    Cerr << "Error: Bad number of functions in scalable_monomials direct fn."
	 << std::endl;
    abort_handler(-1);
  }

  // get power of monomial from analysis components, if available (default to 1)
  int power = 1;
  if (!analysisComponents.empty() && 
      !analysisComponents[analysisDriverIndex].empty())
    power = std::atoi(analysisComponents[analysisDriverIndex][0].c_str());

  // ***************************
  // **** f: sum x[i]^power ****
  // ***************************
  if (directFnASV[0] & 1) {
    fnVals[0] = 0.;
    for (size_t i=0; i<numACV; ++i)
      fnVals[0] += std::pow(xC[i], power);
  }

  // ****************
  // **** df/dx: ****
  // ****************
  if (directFnASV[0] & 2) {
    std::fill_n(fnGrads[0], fnGrads.numRows(), 0.);
    for (size_t i=0; i<numDerivVars; ++i) {
      size_t var_index = directFnDVV[i] - 1;
      switch (power) {
      case 0:  fnGrads[0][i] = 0;                                      break;
      default: fnGrads[0][i] = power*std::pow(xC[var_index], power-1); break;
      }
    }
  }

  // ********************
  // **** d^2f/dx^2: ****
  // ********************
  if (directFnASV[0] & 4) {
    fnHessians[0] = 0.;
    for (size_t i=0; i<numDerivVars; ++i) {
      size_t var_index = directFnDVV[i] - 1;
      switch (power) {
      case 0: case 1:
	fnHessians[0](i,i) = 0; break;
      default:
	fnHessians[0](i,i) = power*(power-1)*std::pow(xC[var_index], power-2);
	break;
      }
    }
  }

  return 0; // no failure
}


/// 1D Herbie function and its derivatives (apart from a multiplicative factor)
void DirectApplicInterface::
herbie1D(size_t der_mode, Real xc_loc, std::vector<Real>& w_and_ders)
{
  w_and_ders[0]=w_and_ders[1]=w_and_ders[2]=0.0;
  
  Real rtemp1=xc_loc-1.0; 
  Real rtemp1_sq=rtemp1*rtemp1;
  Real rtemp2=xc_loc+1.0;
  Real rtemp2_sq=rtemp2*rtemp2;
  Real rtemp3=8.0*(xc_loc+0.1);
  
  if(der_mode & 1) //1=2^0: the 0th derivative of the response (the response itself)
    w_and_ders[0]=
      std::exp(-rtemp1_sq)
      +std::exp(-0.8*rtemp2_sq)
      -0.05*std::sin(rtemp3);
  if(der_mode & 2) //2=2^1: the 1st derivative of the response
    w_and_ders[1]=
      -2.0*rtemp1*std::exp(-rtemp1_sq)
      -1.6*rtemp2*std::exp(-0.8*rtemp2_sq)
      -0.4*std::cos(rtemp3);
  if(der_mode & 4) //4=2^2: the 2nd derivative of the response
    w_and_ders[2]=
      (-2.0+4.0*rtemp1_sq)*std::exp(-rtemp1_sq)
      +(-1.6+2.56*rtemp2_sq)*std::exp(-0.8*rtemp2_sq)
      +3.2*std::sin(rtemp3);
  if(der_mode > 7) {
    std::cerr << "only 0th through 2nd derivatives are implemented for herbie1D()\n";
    assert(false); //should throw an exception get brian to help
  }
}


/// 1D Smoothed Herbie= 1DHerbie minus the high frequency sine term, and its derivatives (apart from a multiplicative factor)
void DirectApplicInterface::
smooth_herbie1D(size_t der_mode, Real xc_loc, std::vector<Real>& w_and_ders)
{
  w_and_ders[0]=w_and_ders[1]=w_and_ders[2]=0.0;
  
  Real rtemp1=xc_loc-1.0; 
  Real rtemp1_sq=rtemp1*rtemp1;
  Real rtemp2=xc_loc+1.0;
  Real rtemp2_sq=rtemp2*rtemp2;
  
  if(der_mode & 1) //1=2^0: the 0th derivative of the response (the response itself)
    w_and_ders[0]=
      std::exp(-rtemp1_sq)
      +std::exp(-0.8*rtemp2_sq);
  if(der_mode & 2) //2=2^1: the 1st derivative of the response
    w_and_ders[1]=
      -2.0*rtemp1*std::exp(-rtemp1_sq)
      -1.6*rtemp2*std::exp(-0.8*rtemp2_sq);
  if(der_mode & 4) //4=2^2: the 2nd derivative of the response
    w_and_ders[2]=
      (-2.0+4.0*rtemp1_sq)*std::exp(-rtemp1_sq)
      +(-1.6+2.56*rtemp2_sq)*std::exp(-0.8*rtemp2_sq);
  if(der_mode > 7) {
    std::cerr << "only 0th through 2nd derivatives are implemented for smooth_herbie1D()\n";
    assert(false); //should throw an exception get brian to help
  }
}


/// 1D Shubert function and its derivatives (apart from a multiplicative factor)
void DirectApplicInterface::
shubert1D(size_t der_mode, Real xc_loc, std::vector<Real>& w_and_ders)
{
  w_and_ders[0]=w_and_ders[1]=w_and_ders[2]=0.0;
  
  size_t k;
  Real k_real;
  
  if(der_mode & 1) {
    for (k=1; k<=5; ++k) {
      k_real=static_cast<Real>(k);
      w_and_ders[0]+=k_real*std::cos(xc_loc*(k_real+1.0)+k_real);	
    }
  }
  if(der_mode & 2) {
    for (k=1; k<=5; ++k) {
      k_real=static_cast<Real>(k);
      w_and_ders[1]+=k_real*(k_real+1.0)*-std::sin(xc_loc*(k_real+1.0)+k_real);
    }
  }
  if(der_mode & 4) {
    for (k=1; k<=5; ++k) {
      k_real=static_cast<Real>(k);
      w_and_ders[2]+=k_real*(k_real+1.0)*(k_real+1.0)*-std::cos(xc_loc*(k_real+1.0)+k_real);
    }
  }
  if(der_mode > 7) {
    std::cerr << "only 0th through 2nd derivatives are implemented for shubert1D()\n";
    assert(false); //should throw an exception get brian to help
  }
}


/// N-D Herbie function and its derivatives
int DirectApplicInterface::herbie()
{
  size_t i;
  std::vector<size_t> der_mode(numVars); 
  for (i=0; i<numVars; ++i)
    der_mode[i]=1;
  if(directFnASV[0] >= 2)
    for (i=0; i<numDerivVars; ++i)
      der_mode[directFnDVV[i]-1]+=2;
  if(directFnASV[0] >= 4)
    for (i=0; i<numDerivVars; ++i)
      der_mode[directFnDVV[i]-1]+=4;
  std::vector<Real> w(numVars);
  std::vector<Real> d1w(numVars);
  std::vector<Real> d2w(numVars);
  std::vector<Real> w_and_ders(3);
  
  for(i=0; i<numVars; ++i) {
    herbie1D(der_mode[i],xC[i],w_and_ders);
    w[i]  =w_and_ders[0];      
    d1w[i]=w_and_ders[1];      
    d2w[i]=w_and_ders[2];
  }      
  
  separable_combine(-1.0,w,d1w,d2w);
  return 0;
}

/// N-D Smoothed Herbie function and its derivatives
int DirectApplicInterface::smooth_herbie()
{
  size_t i;
  std::vector<size_t> der_mode(numVars); 
  for (i=0; i<numVars; ++i)
    der_mode[i]=1;
  if(directFnASV[0] >= 2)
    for (i=0; i<numDerivVars; ++i)
      der_mode[directFnDVV[i]-1]+=2;
  if(directFnASV[0] >= 4)
    for (i=0; i<numDerivVars; ++i)
      der_mode[directFnDVV[i]-1]+=4;
  std::vector<Real> w(numVars);
  std::vector<Real> d1w(numVars);
  std::vector<Real> d2w(numVars);
  std::vector<Real> w_and_ders(3);
  
  for(i=0; i<numVars; ++i) {
    smooth_herbie1D(der_mode[i], xC[i], w_and_ders);
    w[i]  =w_and_ders[0];      
    d1w[i]=w_and_ders[1];      
    d2w[i]=w_and_ders[2];
  }      
  
  separable_combine(-1.0,w,d1w,d2w);
  return 0;
}

int DirectApplicInterface::shubert()
{
  size_t i;
  std::vector<size_t> der_mode(numVars); 
  for (i=0; i<numVars; ++i)
    der_mode[i]=1;
  if(directFnASV[0] >= 2)
    for (i=0; i<numDerivVars; ++i)
      der_mode[directFnDVV[i]-1]+=2;
  if(directFnASV[0] >= 4)
    for (i=0; i<numDerivVars; ++i)
      der_mode[directFnDVV[i]-1]+=4;
  std::vector<Real> w(numVars);
  std::vector<Real> d1w(numVars);
  std::vector<Real> d2w(numVars);
  std::vector<Real> w_and_ders(3);
  
  for(i=0; i<numVars; ++i) {
    shubert1D(der_mode[i],xC[i],w_and_ders);
    w[i]  =w_and_ders[0];      
    d1w[i]=w_and_ders[1];      
    d2w[i]=w_and_ders[2];
  }      
  
  separable_combine(1.0,w,d1w,d2w);
  return 0;
}

/// this function combines N 1D functions and their derivatives to compute a N-D separable function and its derivatives, logic is general enough to support different 1D functions in different dimensions (can mix and match)
void DirectApplicInterface::separable_combine(Real mult_scale_factor, std::vector<Real>& w, std::vector<Real>& d1w, std::vector<Real>& d2w)
{
  // *************************************************************
  // **** now that w(x_i), dw(x_i)/dx_i, and d^2w(x_i)/dx_i^2 ****
  // **** are defined we can calculate the response, gradient ****
  // **** of the response, and Hessian of the response in an  ****
  // **** identical fashion                                   ****
  // *************************************************************
  
  Real local_val;
  size_t i, j, k, i_var_index, j_var_index;
  
  // ****************************************
  // **** response                       ****
  // **** f=\prod_{i=1}^{numVars} w(x_i) ****
  // ****************************************
  if (directFnASV[0] & 1) {
    local_val=mult_scale_factor;
    for (i=0; i<numVars; ++i)
      local_val*=w[i];
    fnVals[0]=local_val;
  }
  
  // **************************************************
  // **** gradient of response                     ****
  // **** df/dx_i=(\prod_{j=1}^{i-1} w(x_j)) ...   ****
  // ****        *(dw(x_i)/dx_i) ...               ****
  // ****        *(\prod_{j=i+1}^{numVars} w(x_j)) ****
  // **************************************************
  if (directFnASV[0] & 2) {
    std::fill_n(fnGrads[0], fnGrads.numRows(), 0.);
    for (i=0; i<numDerivVars; ++i) {
      i_var_index = directFnDVV[i] - 1; 
      local_val=mult_scale_factor*d1w[i_var_index];
      for (j=0; j<i_var_index; ++j)
	local_val*=w[j];
      for (j=i_var_index+1; j<numVars; ++j)
	local_val*=w[j];
      fnGrads[0][i]=local_val;
    }
  }
  
  // ***********************************************************
  // **** Hessian of response                               ****
  // **** if(i==j)                                          ****
  // **** d^2f/dx_i^2=(\prod_{k=1}^{i-1} w(x_k)) ...        ****
  // ****            *(d^2w(x_i)/dx_i^2) ...                ****
  // ****            *(\prod_{k=i+1}^{numVars} w(x_k))      ****
  // ****                                                   ****
  // **** if(i<j)                                           ****
  // **** d^2f/(dx_i*dx_j)=(\prod_{k=1}^{i-1} w(x_k)) ...   ****
  // ****                 *(dw(x_i)/dx_i) ...               ****
  // ****                 *(\prod_{k=i+1}^{j-1} w(x_k)) ... ****
  // ****                 *(dw(x_j)/dx_j) ...               ****
  // ****                 *(\prod_{k=j+1}^{numVars} w(x_j)) ****
  // ***********************************************************
  if (directFnASV[0] & 4) {
    fnHessians[0] = 0.; //what does this do? I think it's vestigal
    for (i=0; i<numDerivVars; ++i) {
      i_var_index = directFnDVV[i] - 1;
      for (j=0; j<numDerivVars; ++j) {
	j_var_index = directFnDVV[j] - 1;
	if (i_var_index==j_var_index ) 
	  local_val = mult_scale_factor*d2w[i_var_index];
	else
	  local_val = mult_scale_factor*d1w[i_var_index]*d1w[j_var_index];
	for (k=0; k<numVars; ++k)
	  if( (k!=i_var_index) && (k!=j_var_index) )
	    local_val*=w[k];
	fnHessians[0](i,j) =local_val;
      }
    }
  }
}


// -------------------------------------------
// Begin direct interfaces to simulation codes
// -------------------------------------------
#ifdef DAKOTA_SALINAS
int DirectApplicInterface::salinas() 
{
  if (numFns < 1 || numFns > 3) {
    Cerr << "Error: Bad number of functions in salinas direct fn." << std::endl;
    abort_handler(-1);
  }
  if (numVars < 1) {
    Cerr << "Error: Bad number of variables in salinas direct fn." << std::endl;
    abort_handler(-1);
  }
  if (gradFlag || hessFlag) {
    Cerr << "Error: analytic derivatives not yet supported in salinas direct "
	 << "fn." <<std::endl;
    abort_handler(-1);
  }

  // ------------------------
  // Salinas input processing
  // ------------------------

  // Set up dummy argc and argv with name of modified input file.
  // NOTE: for concurrent analyses within each fn eval, may want something like
  //       salinas.<eval>.<analysis>.inp (e.g., salinas.3.2.inp)
  int argc = 2;
  char* argv[3];
  argv[0] = "salinas"; // should be ignored
  char si[32];
  std::sprintf(si,"salinas%d.inp", evalServerId); // tag root name in root.inp
  argv[1] = si;
  argv[2] = NULL; // standard requires this, see Kern.&Ritchie, p. 115

  // Insert vars into Salinas input file (Exodus model file avoided for now).
  // Set up loop to process input file and match tokens to variable tags.  The 
  // Salinas parser is not dependent on new lines, so don't worry about 
  // formatting.
  std::ifstream fin("salinas.inp.template");
  std::ofstream fout(argv[1]);
  std::string token;
  while (fin) {
    fin >> token;
    if (token=="//")
      fin.ignore(256, '\n'); // comments will not be replicated in fout
    else if (token=="'./tagged.exo'") {
      // **** Issues with the Exodus input file.  Exodus input files must be 
      //   tagged because the Exodus output uses the same name and must be 
      //   protected from conflict with other concurrent simulations.  This 
      //   requires the creation of these tagged files by DAKOTA or their 
      //   existence a priori (which is a problem when tagging with open ended
      //   indices like evaluation id). A few approaches to handling this:
      // 1.) could use system("cp root.exo root#.exo"), but no good on TFLOP
      // 2.) could tag w/ evalServerId & put sal[0-9].exo out before launching,
      //   but Salinas must overwrite properly (it does) & data loss must be OK
      // 3.) could modify salinas to use (tagged) root.inp i/o root.exo in 
      //   creating root-out.exo, thereby removing the need to tag Exodus input
      char se[32];
      std::sprintf(se,"'./salinas%d.exo' ", evalServerId); // tag root in root.exo
      fout << se;
    }
    else if (localDataView & VARIABLES_MAP) {
      // Use a map-based lookup if tokens of interest are added to varTypeMap
      std::map<String, var_t>::iterator v_it = varTypeMap.find(token);
      if (v_it == varTypeMap.end())
	fout << token << ' ';
      else {
	var_t vt = v_it->second;
	std::map<var_t, Real>::iterator xc_it = xCM.find(vt),
	  xdr_it = xDRM.find(vt);
	std::map<var_t, int>::iterator xdi_it = xDIM.find(vt);
	if (xc_it != xCM.end())
	  fout << xc_it->second << ' ';
	else if (xdi_it != xDIM.end())
	  fout << xdi_it->second << ' ';
	else if (xdr_it != xDRM.end())
	  fout << xdr_it->second << ' ';
	elseint DirectApplicInterface::scalable_text_book()

	  fout << token << ' ';
      }
    }
    else {
      bool found = false;
      size_t i;
      for (i=0; i<numACV; ++i) // loop to remove any order dependency
        if (token == xCLabels[i]) // detect variable label
          { fout << xC[i] << ' '; found = true; break; }
      if (!found)
	for (i=0; i<numADIV; ++i) // loop to remove any order dependency
	  if (token == xDILabels[i]) // detect variable label
	    { fout << xDI[i] << ' '; found = true; break; }
      if (!found)
	for (i=0; i<numADRV; ++i) // loop to remove any order dependency
	  if (token == xDRLabels[i]) // detect variable label
	    { fout << xDR[i] << ' '; found = true; break; }
      if (!found)
        fout << token << ' ';
    }
  }
  fout << std::flush;

  // -----------------
  // Salinas execution
  // -----------------

  // salinas_main is a bare bones wrapper for salinas.  It is provided to 
  // permit calling salinas as a subroutine.

  // analysis_comm may be invalid if multiProcAnalysisFlag is not true!
  const ParallelLevel& ea_pl
    = parallelLib.parallel_configuration().ea_parallel_level();
  MPI_Comm analysis_comm = ea_pl.server_intra_communicator();
  int fail_code = salinas_main(argc, argv, &analysis_comm);
  if (fail_code)
    return fail_code;

  // ---------------------------
  // Salinas response processing
  // ---------------------------

  // Compute margins and return lowest margin as objective function to be 
  // _maximized_ (minimize opposite sign).  Constrain mass to be: 
  // mass_low_bnd <= mass <= mass_upp_bnd
  //Real min_margin = 0.;
  Real lambda1, mass, mass_low_bnd=3., mass_upp_bnd=6.; // 4.608 nominal mass

  // Call EXODUS utilities to retrieve stress & g data 

  // Retrieve data from salinas#.rslt
  char so[32];
  std::sprintf(so,"salinas%d.rslt",evalServerId);
  std::ifstream f2in(so);
  while (f2in) {
    f2in >> token;
    if (token=="Total") {
      for (size_t i=0; i<4; ++i) // After "Total", parse "Mass of Structure is"
        f2in >> token;
      f2in >> mass;
    }
    else if (token=="1:") {
      f2in >> lambda1;
    }
    else 
      f2in.ignore(256, '\n');
  }

  // **** f:
  if (directFnASV[0] & 1)
    fnVals[0] = -lambda1; // max fundamental freq. s.t. mass bounds
    //fnVals[0] = -min_margin;

  // **** c1:
  if (numFns > 1 && (directFnASV[1] & 1))
    fnVals[1] = (mass_low_bnd - mass)/mass_low_bnd;

  // **** c2:
  if (numFns > 2 && (directFnASV[2] & 1))
    fnVals[2] = (mass - mass_upp_bnd)/mass_upp_bnd;

  // **** df/dx:
  //if (directFnASV[0] & 2) {
  //}

  // **** dc1/dx:
  //if (numFns > 1 && (directFnASV[1] & 2)) {
  //}

  // **** dc2/dx:
  //if (numFns > 2 && (directFnASV[2] & 2)) {
  //}

  return 0;
}
#endif // DAKOTA_SALINAS


#ifdef DAKOTA_MODELCENTER
int DirectApplicInterface::mc_api_run()
{
  // ModelCenter interface through direct Dakota interface, HKIM 4/3/03
  // Modified to replace pxcFile with analysisComponents,   MSE 6/20/05

  if (multiProcAnalysisFlag) {
    Cerr << "Error: mc_api_run direct fn does not yet support multiprocessor "
	 << "analyses." << std::endl;
    abort_handler(-1);
  }

  int i, j, ireturn, iprint = 1;

  if(!mc_ptr_int) { // If null, instantiate ModelCenter
    // the pxcFile (Phoenix configuration file) is passed through the
    // analysis_components specification
    if (!analysisComponents.empty() &&
	!analysisComponents[analysisDriverIndex].empty())
      mc_load_model(ireturn,iprint,mc_ptr_int,
		    analysisComponents[analysisDriverIndex][0].c_str());
    else
      ireturn = -1;
    if(ireturn == -1 || mc_ptr_int == 0) abort_handler(-1);
  }

  // continuous variables
  for(i=0; i<numACV; ++i) {
    const char* inStr = xCLabels[i].c_str();
    mc_set_value(ireturn,iprint,mc_ptr_int,xC[i],inStr);
    if(ireturn == -1) abort_handler(-1);
  }

  // discrete, integer-valued variables (actual values sent, not indices)
  for(i=0; i<numADIV; ++i) {
    const char* inStr = xDILabels[i].c_str();
    mc_set_value(ireturn,iprint,mc_ptr_int,xDI[i],inStr);
    if(ireturn == -1) abort_handler(-1);
  }

  // discrete, real-valued variables (actual values sent, not indices)
  for(i=0; i<numADRV; ++i) {
    const char* inStr = xDRLabels[i].c_str();
    mc_set_value(ireturn,iprint,mc_ptr_int,xDR[i],inStr);
    if(ireturn == -1) abort_handler(-1);
  }

  int out_var_act_len = fnLabels.size();
  if (out_var_act_len != numFns) {
    Cerr << "Error: Mismatch in the number of responses in mc_api_run."
	 << std::endl;
    abort_handler(-1);
  }
 
  for (i=0; i<out_var_act_len; ++i) {
    // **** f:
    if (directFnASV[i] & 1) {
      const char* outStr = fnLabels[i].c_str();
      mc_get_value(ireturn,iprint,mc_ptr_int,fnVals[i],outStr);  
      if(ireturn == -1) {
	// Assume this is a failed function evaluation
	// TODO: check correctness / other possible return codes
	return(-1);
      }
      
    }
    // **** df/dx:
    if (directFnASV[i] & 2) {
      Cerr << "Error: Analytic gradients not supported in mc_api_run."
	   << std::endl;
      abort_handler(-1);
    }
    // **** d^2f/dx^2:
    if (directFnASV[i] & 4) {
      Cerr << "Error: Analytic Hessians not supported in mc_api_run."
	   << std::endl;
      abort_handler(-1);
    }
    
  }

  if(dc_ptr_int) {
    mc_store_current_design_point(ireturn,iprint,dc_ptr_int);
    // ignore ireturn for now.
  }

  return 0; // no failure
}
#endif // DAKOTA_MODELCENTER


#ifdef DAKOTA_MATLAB
int DirectApplicInterface::matlab_engine_run()
{
  // direct interface to Matlab through Mathworks external API, BMA 11/28/2005
  // mfile executed is specified through analysisComponents
  // Special thanks to Lee Peterson for substantial enhancements 12/15/2007:
  // - Added output buffer for the MATLAB command response and error messages
  // - Made the Dakota variable persistent in the MATLAB engine workspace
  // - Added robustness to the user deleting required Dakota fields

  const int MATLAB_FAIL = 1;
  const int RESPONSE_TYPES[] = {1, 2, 4};
  int i, j, k, iF;

  /* variables for Matlab data type objects */
  mxArray *dakota_matlab = NULL;
  mxArray *mx_tmp[NUMBER_OF_FIELDS];
  mxArray *mx_string;

  /* temporary variables */
  mwSize dims[3];
  const mwSize *dim_ptr;
  double *ptr;
  std::string analysis_command;
  const int BUFSIZE = 32768;
  char buffer_char[BUFSIZE+1] = {'\0'};
  std::string buffer_string;
  
  int fail_code;
  bool fn_flag;
  
  // make sure the Matlab user did not corrupt the workspace variable Dakota
  if ( (dakota_matlab = engGetVariable(matlabEngine, "Dakota")) == NULL ) {
    Cerr << "Error (Direct:Matlab): Failed to get variable Dakota from "
         << "Matlab." << std::endl;
    return(MATLAB_FAIL);
  }
  if ( mxIsEmpty(dakota_matlab) ) {
    Cerr << "Error (Direct:Matlab): Dakota variable from Matlab "
         << "is empty. " << std::endl;
    return(MATLAB_FAIL);
  }
  if ( !mxIsStruct(dakota_matlab) ) {
    Cerr << "Error (Direct:Matlab): Dakota variable from Matlab "
         << "is not a structure array. " << std::endl;
    return(MATLAB_FAIL);
  }
  if ( (mxGetM(dakota_matlab) != 1) | (mxGetN(dakota_matlab) != 1) ) {
    Cerr << "Error (Direct:Matlab): Dakota variable from Matlab "
         << "is not a scalar. " << std::endl;
    return(MATLAB_FAIL);
  }

  /* Set scalar valued data */
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[0])) >= 0)
    mxSetFieldByNumber(dakota_matlab,0,iF,
		       mxCreateDoubleScalar((double) numFns));
  else
    return(MATLAB_FAIL);
  
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[1])) >= 0)
    mxSetFieldByNumber(dakota_matlab,0,iF,
		       mxCreateDoubleScalar((double) numVars));
  else
    return(MATLAB_FAIL);
  
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[2])) >= 0)
    mxSetFieldByNumber(dakota_matlab,0,iF,
		       mxCreateDoubleScalar((double) numACV));
  else
    return(MATLAB_FAIL);
    
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[3])) >= 0)
    mxSetFieldByNumber(dakota_matlab,0,iF,
		       mxCreateDoubleScalar((double) numADIV));
  else
    return(MATLAB_FAIL);

  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[4])) >= 0)
    mxSetFieldByNumber(dakota_matlab,0,iF,
		       mxCreateDoubleScalar((double) numADRV));
  else
    return(MATLAB_FAIL);
  
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[5])) >= 0)
    mxSetFieldByNumber(dakota_matlab,0,iF, 
		       mxCreateDoubleScalar((double) numDerivVars));
  else
    return(MATLAB_FAIL);
  
  /* Create Matlab matrix, get pointer to it, populate, put in structure 
     NOTE that mxSetFieldByNumber needs to retain allocated memory -- it
     just points to the created mxArray mx_tmp[k], so don't deallocate mx_tmp. 
     MAY be able to use mxSetPr, depending on what types 
     RealVector, IntArray, StringArray are
  */

  /* continuous variables */
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[6])) >= 0) {
    mx_tmp[6] = mxCreateDoubleMatrix(1,numACV,mxREAL);
    ptr = mxGetPr(mx_tmp[6]);
    for( i=0; i<numACV; ++i)  
      *(ptr+i) = xC[i];  
    mxSetFieldByNumber(dakota_matlab, 0, iF, mx_tmp[6]);
  }
  else
    return(MATLAB_FAIL);

  /* discrete integer variables */
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[7])) >= 0) {
    mx_tmp[7] = mxCreateDoubleMatrix(1,numADIV,mxREAL);
    ptr = mxGetPr(mx_tmp[7]);
    for( i=0; i<numADIV; ++i)  
      *(ptr+i) = (double) xDI[i];
    mxSetFieldByNumber(dakota_matlab, 0, iF, mx_tmp[7]);
  }
  else
    return(MATLAB_FAIL);

  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[8])) >= 0) {
    mx_tmp[8] = mxCreateDoubleMatrix(1,numADRV,mxREAL);
    ptr = mxGetPr(mx_tmp[8]);
    for( i=0; i<numADRV; ++i)  
      *(ptr+i) = xDR[i];
    mxSetFieldByNumber(dakota_matlab, 0, iF, mx_tmp[8]);
  }
  else
    return(MATLAB_FAIL);

  /* continuous labels */
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[9])) >= 0) {
    dims[0] = numACV;
    dims[1] = 1;
    mx_tmp[9] = mxCreateCellArray(2,dims);
    for( i=0; i<numACV; ++i)  
      mxSetCell(mx_tmp[9], i, mxCreateString( xCLabels[i] ));
    mxSetFieldByNumber(dakota_matlab, 0, iF, mx_tmp[9]);
  }
  else
    return(MATLAB_FAIL);

  /* discrete integer labels */
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[10])) >= 0) {
    dims[0] = numADIV;
    dims[1] = 1;
    mx_tmp[10] = mxCreateCellArray(2,dims);
    for( i=0; i<numADIV; ++i)
      mxSetCell(mx_tmp[10], i, mxCreateString( xDILabels[i] ));
    mxSetFieldByNumber(dakota_matlab, 0, iF, mx_tmp[10]);
  }
  else
    return(MATLAB_FAIL);

  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[11])) >= 0) {
    dims[0] = numADRV;
    dims[1] = 1;
    mx_tmp[11] = mxCreateCellArray(2,dims);
    for( i=0; i<numADRV; ++i)
      mxSetCell(mx_tmp[11], i, mxCreateString( xDRLabels[i] ));
    mxSetFieldByNumber(dakota_matlab, 0, iF, mx_tmp[11]);
  }
  else
    return(MATLAB_FAIL);

  /* active set vector (ASV) / matrix */
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[12])) >= 0) {
    mx_tmp[12] = mxCreateDoubleMatrix(1,numFns,mxREAL);
    for( i=0; i<numFns; ++i)
      *(mxGetPr(mx_tmp[12])+i) = directFnASV[i];  
    mxSetFieldByNumber(dakota_matlab, 0, iF, mx_tmp[12]);
  }
  else
    return(MATLAB_FAIL);
  
  /* Create boolean version of ASV.  Rows are fnval, grad, hess; col per fn
     CAREFUL -- Matlab stores by column */
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[13])) >= 0) {
    mx_tmp[13] = mxCreateDoubleMatrix(3,numFns,mxREAL);
    for( i=0; i<3; ++i)
      for ( j=0; j<numFns; ++j)
	// FIX THIS MESS!
	(directFnASV[j] & RESPONSE_TYPES[i])
	  ? (*(mxGetPr(mx_tmp[13])+3*j+i) = 1)
	  : (*(mxGetPr(mx_tmp[13])+3*j+i) = 0);
    mxSetFieldByNumber(dakota_matlab, 0, iF, mx_tmp[13]);  
  }
  else
    return(MATLAB_FAIL);

  /* derivative variables vector (DVV) / DVV_bool */
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[14])) >= 0) {
    mx_tmp[14] = mxCreateDoubleMatrix(1,numDerivVars,mxREAL);
    for( i=0; i<numDerivVars; ++i)
      *(mxGetPr(mx_tmp[14])+i) = directFnDVV[i];  
    mxSetFieldByNumber(dakota_matlab, 0, iF, mx_tmp[14]);
  }
  else
    return(MATLAB_FAIL);

  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[15])) >= 0) {
    mx_tmp[15] = mxCreateDoubleMatrix(1,numVars,mxREAL);
    for ( j=0; j<numDerivVars; ++j)
      *(mxGetPr(mx_tmp[15])+directFnDVV[j]-1) = 1;
    mxSetFieldByNumber(dakota_matlab, 0, iF, mx_tmp[15]);
  }
  else
    return(MATLAB_FAIL);
  
  /* fn, grad, hess Flags; as needed, allocate & initialize matrices to zero */
  fn_flag = false;
  for ( j=0; j<numFns; ++j)
    if (directFnASV[j] & 1) {
      fn_flag = true;
      if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[19])) >= 0) {
	mx_tmp[19] = mxCreateDoubleMatrix(1,numFns,mxREAL);
	mxSetFieldByNumber(dakota_matlab, 0, iF, mx_tmp[19]);
      }
      else 
	return(MATLAB_FAIL);
      break;
    }

  if (gradFlag) {
    if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[20])) >= 0) {
      mx_tmp[20] = mxCreateDoubleMatrix(numFns,numDerivVars,mxREAL);
      mxSetFieldByNumber(dakota_matlab, 0, iF, mx_tmp[20]);
    }
    else	
      return(MATLAB_FAIL);
  }

  if (hessFlag) {
    if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[21])) >= 0) {
      dims[0] = numFns;
      dims[1] = numDerivVars;
      dims[2] = numDerivVars;
      mx_tmp[21] = mxCreateNumericArray(3,dims,mxDOUBLE_CLASS,mxREAL);
      mxSetFieldByNumber(dakota_matlab, 0, iF, mx_tmp[21]);
    }
    else 
      return(MATLAB_FAIL);
  }
 
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[16])) >= 0) {
    mxSetFieldByNumber(dakota_matlab,0,iF,
		       mxCreateDoubleScalar((double) fn_flag));
  }
  else	
    return(MATLAB_FAIL);
  
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[17])) >= 0) {
    mxSetFieldByNumber(dakota_matlab,0,iF,
		       mxCreateDoubleScalar((double) gradFlag));
  }
  else	
    return(MATLAB_FAIL);

  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[18])) >= 0) {
    mxSetFieldByNumber(dakota_matlab,0,iF,
		       mxCreateDoubleScalar((double) hessFlag));
  }
  else	
    return(MATLAB_FAIL);

  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[24])) >= 0) {
    mxSetFieldByNumber(dakota_matlab,0,iF,
		       mxCreateDoubleScalar((double) currEvalId));
  }
  else	
    return(MATLAB_FAIL);
 
  /* put the structure into the Matlab workspace, then 
     iterate over provided analysis components, checking for error each time */
  engPutVariable(matlabEngine, "Dakota", dakota_matlab);
  buffer_char[BUFSIZE] = '\0';
  engOutputBuffer(matlabEngine, buffer_char, BUFSIZE);

  if (analysisComponents.empty() ||
      analysisComponents[analysisDriverIndex].empty()) {
    Cerr << "\nError: MATLAB direct interface requires analysis_components "
	 << "specification.\n" << std::endl;
    abort_handler(-1);
  }
  for (int aci=0; aci<analysisComponents[analysisDriverIndex].size(); aci++) {

    // strip away any .m the user might have included
    size_t pos = analysisComponents[analysisDriverIndex][aci].find(".");
    analysis_command = "Dakota = " + 
      analysisComponents[analysisDriverIndex][aci].substr(0,pos) + "(Dakota);";
    engEvalString(matlabEngine, analysis_command.c_str());
  }
  buffer_string = buffer_char;
  if (outputLevel > SILENT_OUTPUT && buffer_string.length() > 3)
    Cout << "Warning (Direct:Matlab): Matlab function output:\n" 
	 << buffer_char+3 << std::endl;  
  
  /* retrieve and parse the response */
  dakota_matlab = NULL;
  if ( (dakota_matlab = engGetVariable(matlabEngine, "Dakota")) == NULL ) {
    Cerr << "Error (Direct:Matlab): Failed to get variable Dakota from " 
         << "Matlab." << std::endl;
    fail_code = MATLAB_FAIL;
  } else if ( (mx_tmp[23] = mxGetField(dakota_matlab, 0, "failure")) != NULL &&
              mxGetClassID(mx_tmp[23]) == mxDOUBLE_CLASS && 
              *(mxGetPr(mx_tmp[23])) != 0 ) {

    /* Matlab user indicated failure, don't process */
    fail_code = (int) *(mxGetPr(mx_tmp[23]));

  } else {
    
    fail_code = 0;
    /* get fields by name in case the user somehow reordered, or decided to
       only return some of the fields to us... update all of
         fns:   1 x numFns
         grads: numFns * numDerivVars
         hess:  numFns * numDerivVars * numDerivVars
       if any of these come back wrong, set fail_code
    */    
     
    if (fn_flag)
      if ( (mx_tmp[19]=mxGetField(dakota_matlab, 0, "fnVals")) == NULL ) {
      
        Cerr << "Error (Direct:Matlab): Failed to get field fnVals from " 
             << "Dakota structure." << std::endl;
        fail_code = MATLAB_FAIL;
      
      } else if ( mxGetM(mx_tmp[19]) != 1 | mxGetN(mx_tmp[19]) != numFns ) {
        
        Cerr << "Error (Direct:Matlab): Dakota.fnVals must be [1 x numFns]." 
             << std::endl;    
        fail_code = MATLAB_FAIL;
      
      } else {
        
        ptr=mxGetPr(mx_tmp[19]);
        for (i=0; i<numFns; ++i)
          fnVals[i] = *(ptr + i);
      }   
  
    if (gradFlag) 
      if ( (mx_tmp[20]=mxGetField(dakota_matlab, 0, "fnGrads")) == NULL ) {
      
        Cerr << "Error (Direct:Matlab): Failed to get field fnGrads from " 
             << "Dakota structure." << std::endl;
        fail_code = MATLAB_FAIL;     
      
      } else if ( mxGetM(mx_tmp[20]) != numFns |
		  mxGetN(mx_tmp[20]) != numDerivVars ) {
      
        Cerr << "Error (Direct:Matlab): Dakota.fnVals must be "
             << "[numFns x numDerivVars]." << std::endl;    
        fail_code = MATLAB_FAIL;
      
      } else  {

	// Matlab stores column major
        ptr=mxGetPr(mx_tmp[20]);
        for (i=0; i<numFns; ++i)
          for (j=0; j<numDerivVars; ++j)
            fnGrads(j,i) = *(ptr + j*numFns + i);
      
      }
  
    if (hessFlag) 
      if ( (mx_tmp[21]=mxGetField(dakota_matlab, 0, "fnHessians")) == NULL ) {
        
        Cerr << "Error (Direct:Matlab): Failed to get field fnHessians from " 
             << "Dakota structure." << std::endl;
        fail_code = MATLAB_FAIL;
      
      } else if ( mxGetNumberOfDimensions(mx_tmp[21]) != 3 ) { 
        
        Cerr << "Error (Direct:Matlab): Dakota.fnHessians must be "
             << "3 dimensional." << std::endl;    
        fail_code = MATLAB_FAIL;
        
      } else {
        
        dim_ptr = mxGetDimensions(mx_tmp[21]);
        if ( dim_ptr[0] != numFns | dim_ptr[1] != numDerivVars | 
             dim_ptr[2] != numDerivVars ) {
          
          Cerr << "Error (Direct:Matlab): Dakota.fnHessians must be "
               << "[numFns x numDerivVars x numDerivVars]." << std::endl;     
          fail_code = MATLAB_FAIL;
          
        } else {
          
	  // Matlab stores column major
          ptr=mxGetPr(mx_tmp[21]);
          for (i=0; i<numFns; ++i)
            for (j=0; j<numDerivVars; ++j)
              for (k=0; k<=j; ++k)
                fnHessians[i](j,k) = 
		  *(ptr + k*numFns*numDerivVars + j*numFns + i);
        }
      
      }
    
    /* get fnLabels -- optional return value */
    if ( (mx_tmp[22]=mxGetField(dakota_matlab, 0, "fnLabels")) != NULL )
      if ( mxGetClassID(mx_tmp[22]) == mxCELL_CLASS &&
           mxGetNumberOfElements(mx_tmp[22]) == numFns )
        for (i=0; i<numFns; ++i) {
        
          mx_string = mxGetCell(mx_tmp[22], i);  
          if ( mxGetClassID(mx_string) != mxCHAR_CLASS ) 
            Cout << "Warning (Direct:Matlab): Dakota.fnLabels{" << i
                 << "} is not a string.  Not returning fnLabel " << i 
                 << "." << std::endl;     
          else {
            
            mxGetString(mx_string, buffer_char, 256);
            fnLabels[i] = buffer_char;
        
          }
        }
   
  } /* end else parse response */

  return(fail_code);    

}

int DirectApplicInterface::matlab_field_prep(mxArray* dakota_matlab,
					       const char* field_name)
{
  int iF;
  if ( (iF=mxGetFieldNumber(dakota_matlab, field_name)) == -1 )
    if ( (iF=mxAddField(dakota_matlab, field_name)) == -1 ) {
      Cerr << "Error (Direct:Matlab): Cannot init Dakota." << field_name
	   << "."<< std::endl;
      return(iF);
    }
  mxDestroyArray(mxGetFieldByNumber(dakota_matlab,0,iF));
  return(iF);
}
#endif // DAKOTA_MATLAB

#ifdef DAKOTA_PYTHON
int DirectApplicInterface::python_run()
{
  // probably need to convert all of the following with SWIG or Boost!!
  // (there is minimal error checking for now)
  // need to cleanup ref counts on Python objects
  int fail_code = 0;

  // probably want to load the modules and functions at construction time, incl.
  // validation and store the objects for later, but need to resolve use of
  // analysisDriverIndex

  // for now we presume a single analysis component containing module:function
  if (analysisComponents.empty() ||
      analysisComponents[analysisDriverIndex].empty()) {
    Cerr << "\nError: Python direct interface requires analysis_components "
	 << "specification.\n" << std::endl;
    abort_handler(-1);
  }
  const String& an_comp = analysisComponents[analysisDriverIndex][0];
  size_t pos = an_comp.find(":");
  std::string module_name = an_comp.substr(0,pos);
  std::string function_name = an_comp.substr(pos+1);

  // import the module and function and test for callable
  PyObject *pModule = PyImport_Import(PyString_FromString(module_name.c_str()));
  if (pModule == NULL) {
    Cerr << "Error (Direct:Python): Failure importing module " << module_name 
	 << ".\n                       Consider setting PYTHONPATH."
	 << std::endl;
    abort_handler(-1);
  }

  char fn[function_name.size()+1];
  strcpy(fn, function_name.c_str());
  PyObject *pFunc = PyObject_GetAttrString(pModule, fn);
  if (!pFunc || !PyCallable_Check(pFunc)) {
    Cerr << "Error (Direct:Python): Function " << function_name  << "not found "
	 << "or not callable" << std::endl;
    abort_handler(-1);
  }

  // must use empty tuple here to pass to function taking only kwargs
  PyObject *pArgs = PyTuple_New(0);
  PyObject *pDict = PyDict_New();

  // convert DAKOTA data types to Python objects (lists and/or numpy arrays)
  PyObject *cv, *cv_labels, *div, *div_labels, *drv, *drv_labels,
    *av, *av_labels, *asv, *dvv;
  python_convert(xC, &cv);
  python_convert(xCLabels, &cv_labels);
  python_convert_int(xDI, xDI.length(), &div);
  python_convert(xDILabels, &div_labels);
  python_convert(xDR, &drv);
  python_convert(xDRLabels, &drv_labels);
  python_convert(xC, xDI, xDR, &av);
  python_convert(xCLabels, xDILabels, xDRLabels, &av_labels);
  python_convert_int(directFnASV, directFnASV.size(), &asv);
  python_convert_int(directFnDVV, directFnASV.size(), &dvv);
  // TO DO: analysis components

  // assemble everything into a dictionary to pass to user function
  // this should eat references to the objects declared above
  PyDict_SetItem(pDict, PyString_FromString("variables"), 
		 PyInt_FromLong((long) numVars));
  PyDict_SetItem(pDict, PyString_FromString("functions"), 
		 PyInt_FromLong((long) numFns)); 
  PyDict_SetItem(pDict, PyString_FromString("cv"), cv);
  PyDict_SetItem(pDict, PyString_FromString("cv_labels"), cv_labels);
  PyDict_SetItem(pDict, PyString_FromString("div"), div);
  PyDict_SetItem(pDict, PyString_FromString("div_labels"), div_labels);
  PyDict_SetItem(pDict, PyString_FromString("drv"), drv);
  PyDict_SetItem(pDict, PyString_FromString("drv_labels"), drv_labels);
  PyDict_SetItem(pDict, PyString_FromString("av"), av);
  PyDict_SetItem(pDict, PyString_FromString("av_labels"), av_labels);
  PyDict_SetItem(pDict, PyString_FromString("asv"), asv);
  PyDict_SetItem(pDict, PyString_FromString("dvv"), dvv);
  PyDict_SetItem(pDict, PyString_FromString("currEvalId"), 
		 PyInt_FromLong((long) currEvalId));

  // perform analysis
  if (outputLevel > NORMAL_OUTPUT)
    Cout << "Info (Direct:Python): Calling function " << function_name 
	 << " in module " << module_name << "." << std::endl;
  PyObject *retVal = PyObject_Call(pFunc, pArgs, pDict);
  Py_DECREF(pDict);
  Py_DECREF(pArgs);    
  Py_DECREF(pFunc);
  Py_DECREF(pModule);
  if (!retVal) {
    // TODO: better error reporting from Python
    Cerr << "Error (Direct:Python): Unknown error evaluating python "
	 << "function." << std::endl;
    abort_handler(-1);
  }
      
  bool fn_flag = false;
  for (int i=0; i<numFns; ++i)
    if (directFnASV[i] & 1) {
      fn_flag = true;
      break;
    }

  // process return type as dictionary, else assume list of functions only
  if (PyDict_Check(retVal)) {
    // or the user may return a dictionary containing entires fns, fnGrads,
    // fnHessians, fnLabels, failure (int)
    // fnGrads, e.g. is a list of lists of doubles
    // this is where Boost or SWIG could really help
    // making a lot of assumptions on types being returned
    PyObject *obj;
    if (fn_flag) {
      if ( !(obj = PyDict_GetItemString(retVal, "fns")) ) {
	Cerr << "Python dictionary must contain list 'fns'" << std::endl;
	Py_DECREF(retVal);
	abort_handler(-1);
      }
      if (!python_convert(obj, fnVals, numFns)) {
	Py_DECREF(retVal);
	abort_handler(-1);
      }
    }
    if (gradFlag) {
      if ( !(obj = PyDict_GetItemString(retVal, "fnGrads")) ) {
	Cerr << "Python dictionary must contain list 'fnGrads'" << std::endl;
	Py_DECREF(retVal);
	abort_handler(-1);
      }
      if (!python_convert(obj, fnGrads)) {
	Py_DECREF(retVal);
	abort_handler(-1);
      }
    }
    if (hessFlag) {
      if ( !(obj = PyDict_GetItemString(retVal, "fnHessians")) ) {
	Cerr << "Python dictionary must contain list 'fnHessians'" << std::endl;
	Py_DECREF(retVal);
	abort_handler(-1);
      }
      if (!python_convert(obj, fnHessians)){
	Py_DECREF(retVal);
	abort_handler(-1);
      }
    }
    // optional returns
    if (obj = PyDict_GetItemString(retVal, "failure"))
      fail_code = PyInt_AsLong(obj);

    if (obj = PyDict_GetItemString(retVal, "fnLabels")) {
      if (!PyList_Check(obj) || PyList_Size(obj) != numFns) {
	Cerr << "'fnLabels' must be list of length numFns." << std::endl;
	Py_DECREF(retVal);
	abort_handler(-1);
      }
      for (int i=0; i<numFns; ++i)
	fnLabels[i] = PyString_AsString(PyList_GetItem(obj, i));
    }
  }
  else {
    // asssume list/numpy array containing only functions
    if (fn_flag)
      python_convert(retVal, fnVals, numFns);
  }
  Py_DECREF(retVal);

  return(fail_code);
}


/** convert all integer array types including IntVector, ShortArray,
    and SizetArray to Python list of ints or numpy array of ints */
template<class ArrayT, class Size>
bool DirectApplicInterface::
python_convert_int(const ArrayT& src, Size sz, PyObject** dst)
{
#ifdef DAKOTA_PYTHON_NUMPY
  if (userNumpyFlag) {
    npy_intp dims[1];
    dims[0] = sz;
    if (!(*dst = PyArray_SimpleNew(1, dims, PyArray_INT))) {
      Cerr << "Error creating Python numpy array." << std::endl;
      return(false);
    }
    PyArrayObject *pao = (PyArrayObject *) *dst;
    for (int i=0; i<sz; ++i)
      *(int *)(pao->data + i*(pao->strides[0])) = (int) src[i];
  }
  else 
#endif
  {
    if (!(*dst = PyList_New(sz))) {
      Cerr << "Error creating Python list." << std::endl;
      return(false);
    }
    for (int i=0; i<sz; ++i)
      PyList_SetItem(*dst, i, PyInt_FromLong((long) src[i]));
  }
  return(true);
}


// convert RealVector to list of floats or numpy array of doubles
bool DirectApplicInterface::
python_convert(const RealVector& src, PyObject** dst)
{
  int sz = src.length();
#ifdef DAKOTA_PYTHON_NUMPY
  if (userNumpyFlag) {
    npy_intp dims[1];
    dims[0] = sz;
    if (!(*dst = PyArray_SimpleNew(1, dims, PyArray_DOUBLE))) {
      Cerr << "Error creating Python numpy array." << std::endl;
      return(false);
    }
    PyArrayObject *pao = (PyArrayObject *) *dst;
    for (int i=0; i<sz; ++i)
      *(double *)(pao->data + i*(pao->strides[0])) = src[i];
  }
  else
#endif
  {
    if (!(*dst = PyList_New(sz))) {
      Cerr << "Error creating Python list." << std::endl;
      return(false);
    }
    for (int i=0; i<sz; ++i)
      PyList_SetItem(*dst, i, PyFloat_FromDouble(src[i]));
  }
  return(true);
}


// helper for converting xC, xDI, and xDR to single Python array of all variables
bool DirectApplicInterface::
python_convert(const RealVector& c_src, const IntVector& di_src,
	       const RealVector& dr_src, PyObject** dst)
{
  int c_sz = c_src.length();
  int di_sz = di_src.length();
  int dr_sz = dr_src.length();
#ifdef DAKOTA_PYTHON_NUMPY
  if (userNumpyFlag) {
    npy_intp dims[1];
    dims[0] = c_sz + di_sz + dr_sz;
    if (!(*dst = PyArray_SimpleNew(1, dims, PyArray_DOUBLE))) {
      Cerr << "Error creating Python numpy array." << std::endl;
      return(false);
    }
    PyArrayObject *pao = (PyArrayObject *) *dst;
    for (int i=0; i<c_sz; ++i)
      *(double *)(pao->data + i*(pao->strides[0])) = c_src[i];
    for (int i=0; i<di_sz; ++i)
      *(double *)(pao->data + (c_sz+i)*(pao->strides[0])) = (double) di_src[i];
    for (int i=0; i<dr_sz; ++i)
      *(double *)(pao->data + (c_sz+di_sz+i)*(pao->strides[0])) = dr_src[i];
  }
  else
#endif
  {
    if (!(*dst = PyList_New(c_sz + di_sz + dr_sz))) {
      Cerr << "Error creating Python list." << std::endl;
      return(false);
    }
    for (int i=0; i<c_sz; ++i)
      PyList_SetItem(*dst, i, PyFloat_FromDouble(c_src[i]));
    for (int i=0; i<di_sz; ++i)
      PyList_SetItem(*dst, c_sz + i, PyInt_FromLong((long) di_src[i]));
    for (int i=0; i<dr_sz; ++i)
      PyList_SetItem(*dst, c_sz + di_sz + i, 
		     PyFloat_FromDouble(dr_src[i]));
  }
  return(true);
}


// convert StringArray to list of strings
bool DirectApplicInterface::
python_convert(const StringMultiArray& src, PyObject** dst)
{
  int sz = src.size();
  if (!(*dst = PyList_New(sz))) {
      Cerr << "Error creating Python list." << std::endl;
      return(false);
  }
  for (int i=0; i<sz; ++i)
    PyList_SetItem(*dst, i, PyString_FromString(src[i]));

  return(true);
}


// convert continuous and discrete label strings to single list
bool DirectApplicInterface::
python_convert(const StringMultiArray& c_src, const StringMultiArray& di_src,
	       const StringMultiArray& dr_src, PyObject** dst)
{
  int c_sz = c_src.size();
  int di_sz = di_src.size();
  int dr_sz = dr_src.size();
  if (!(*dst = PyList_New(c_sz + di_sz + dr_sz))) {
    Cerr << "Error creating Python list." << std::endl;
    return(false);
  }
  for (int i=0; i<c_sz; ++i)
    PyList_SetItem(*dst, i, PyString_FromString(c_src[i]));
  for (int i=0; i<di_sz; ++i)
    PyList_SetItem(*dst, c_sz+i, PyString_FromString(di_src[i]));
  for (int i=0; i<dr_sz; ++i)
    PyList_SetItem(*dst, c_sz+di_sz+i, PyString_FromString(dr_src[i]));

  return(true);
}


// Accepts python list or numpy array, DAKOTA RealVector,
// expected dimension. Returns false if conversion failed.
bool DirectApplicInterface::
python_convert(PyObject *pyv, RealVector& rv, const int& dim)
{
#ifdef DAKOTA_PYTHON_NUMPY
  // could automatically detect return type instead of throwing error
  if (userNumpyFlag) {
    if (!PyArray_Check(pyv) || PyArray_NDIM(pyv) != 1 || 
	PyArray_DIM(pyv,0) != dim) {
      Cerr << "Python numpy array not 1D of size " << dim << "." << std::endl;
      return(false);
    }
    PyArrayObject *pao = (PyArrayObject *) pyv;
    for (int i=0; i<dim; ++i)
      rv[i] = *(double *)(pao->data + i*(pao->strides[0]));
  }
  else
#endif
  {
    PyObject *val;
    if (!PyList_Check(pyv) || PyList_Size(pyv) != dim) {
      Cerr << "Python vector must have length " << dim << "." << std::endl;
      return(false);
    }
    for (int i=0; i<dim; ++i) {
      val = PyList_GetItem(pyv, i);
      if (PyFloat_Check(val))
	rv[i] = PyFloat_AsDouble(val);
      else if (PyInt_Check(val))
	rv[i] = (double) PyInt_AsLong(val);
      else {
	Cerr << "Unsupported Python data type converting vector." << std::endl;
	Py_DECREF(val);
	return(false);
      }
    }
  }
  return(true);
}

// Accepts python list or numpy array, pointer to double, e.g., view
// of a Teuchose::SerialDenseVector, expected dimension.  Returns
// false if conversion failed.
bool DirectApplicInterface::
python_convert(PyObject *pyv, double *rv, const int& dim)
{
#ifdef DAKOTA_PYTHON_NUMPY
  // could automatically detect return type instead of throwing error
  if (userNumpyFlag) {
    if (!PyArray_Check(pyv) || PyArray_NDIM(pyv) != 1 || 
	PyArray_DIM(pyv,0) != dim) {
      Cerr << "Python numpy array not 1D of size " << dim << "." << std::endl;
      return(false);
    }
    PyArrayObject *pao = (PyArrayObject *) pyv;
    for (int i=0; i<dim; ++i)
      rv[i] = *(double *)(pao->data + i*(pao->strides[0]));
  }
  else
#endif
  {
    PyObject *val;
    if (!PyList_Check(pyv) || PyList_Size(pyv) != dim) {
      Cerr << "Python vector must have length " << dim << "." << std::endl;
      return(false);
    }
    for (int i=0; i<dim; ++i) {
      val = PyList_GetItem(pyv, i);
      if (PyFloat_Check(val))
	rv[i] = PyFloat_AsDouble(val);
      else if (PyInt_Check(val))
	rv[i] = (double) PyInt_AsLong(val);
      else {
	Cerr << "Unsupported Python data type converting vector." << std::endl;
	Py_DECREF(val);
	return(false);
      }
    }
  }
  return(true);
}

// assume we're converting numFns x numDerivVars to numDerivVars x
// numFns (gradients) returns false if conversion failed
bool DirectApplicInterface::python_convert(PyObject *pym, RealMatrix &rm)
{
#ifdef DAKOTA_PYTHON_NUMPY
  if (userNumpyFlag) {
    if (!PyArray_Check(pym) || PyArray_NDIM(pym) != 2 || 
	PyArray_DIM(pym,0) != numFns  ||  PyArray_DIM(pym,1) != numDerivVars) {
      Cerr << "Python numpy array not 2D of size " << numFns << "x"
	   << numDerivVars << "." << std::endl;
      return(false);
    }
    PyArrayObject *pao = (PyArrayObject *) pym;
    for (int i=0; i<numFns; ++i)
      for (int j=0; j<numDerivVars; ++j)
	rm(j,i) = *(double *)(pao->data + i*(pao->strides[0]) + 
			      j*(pao->strides[1]));
  }
  else
#endif
  {
    PyObject *val;
    if (!PyList_Check(pym) || PyList_Size(pym) != numFns) {
      Cerr << "Python matrix must have " << numFns << "rows." << std::endl;
      return(false);
    }
    for (int i=0; i<numFns; ++i) {
      val = PyList_GetItem(pym, i);
      if (PyList_Check(val)) {
	// use the helper to convert this column of the gradients
	if (!python_convert(val, rm[i], numDerivVars))
	  return(false);
      }
      else {
	Cerr << "Each row of Python matrix must be a list." << std::endl;
	Py_DECREF(val);
	return(false);
      }
    }
  }
  return(true);
}

// assume numDerivVars x numDerivVars as helper in Hessian conversion
// and lower triangular storage in Hessians
// returns false if conversion failed
bool DirectApplicInterface::python_convert(PyObject *pym, 
					   RealSymMatrix &rm)
{
  // for now, the numpy case isn't called (since handled in calling
  // Hessian array convert)
#ifdef DAKOTA_PYTHON_NUMPY
  if (userNumpyFlag) {
    if (!PyArray_Check(pym) || PyArray_NDIM(pym) != 2 || 
	PyArray_DIM(pym,0) != numDerivVars  ||
	PyArray_DIM(pym,1) != numDerivVars) {
      Cerr << "Python numpy array not 2D of size " << numDerivVars << "x" 
	   << numDerivVars << "." << std::endl;
      return(false);
    }
    PyArrayObject *pao = (PyArrayObject *) pym;
    for (int i=0; i<numDerivVars; ++i)
      for (int j=0; j<=i; ++j)
	rm(i,j) = *(double *)(pao->data + i*(pao->strides[0]) + 
			       j*(pao->strides[1]));
  }
  else
#endif
  {
    if (!PyList_Check(pym) || PyList_Size(pym) != numDerivVars) {
      Cerr << "Python matrix must have " << numDerivVars << "rows." <<std::endl;
      return(false);
    }
    PyObject *pyv, *val;
    for (int i=0; i<numDerivVars; ++i) {
      pyv = PyList_GetItem(pym, i);
      if (!PyList_Check(pyv) || PyList_Size(pyv) != numDerivVars) {
	Cerr << "Python vector must have length " << numDerivVars << "." 
	     << std::endl;
	return(false);
      }
      for (int j=0; j<=i; ++j) {
	val = PyList_GetItem(pyv, j);
	if (PyFloat_Check(val))
	  rm(i,j) = PyFloat_AsDouble(val);
	else if (PyInt_Check(val))
	  rm(i,j) = (double) PyInt_AsLong(val);
	else {
	  Cerr << "Unsupported Python data type converting vector." 
	       << std::endl;
	  Py_DECREF(val);
	  return(false);
	}
      }
    }

  }
  return(true);
}


// assume numFns x numDerivVars x numDerivVars
// returns false if conversion failed
bool DirectApplicInterface::
python_convert(PyObject *pyma, RealSymMatrixArray &rma)
{
#ifdef DAKOTA_PYTHON_NUMPY
  if (userNumpyFlag) {
    // cannot recurse in this case as we now have a symmetric matrix 
    // (clearer this way anyway)
    if (!PyArray_Check(pyma) || PyArray_NDIM(pyma) != 3 || 
	PyArray_DIM(pyma,0) != numFns || PyArray_DIM(pyma,1) != numDerivVars ||
	PyArray_DIM(pyma,2) != numDerivVars ) {
      Cerr << "Python numpy array not 3D of size " << numFns << "x"
	   << numDerivVars << "x" << numDerivVars << "." << std::endl;
      return(false);
    }
    PyArrayObject *pao = (PyArrayObject *) pyma;
    for (int i=0; i<numFns; ++i)
      for (int j=0; j<numDerivVars; ++j)
	for (int k=0; k<=j; ++k)
	  rma[i](j,k) = *(double *)(pao->data + i*(pao->strides[0]) + 
				    j*(pao->strides[1]) +
				    k*(pao->strides[2]));
  }
  else
#endif
  {
    PyObject *val;
    if (!PyList_Check(pyma) || PyList_Size(pyma) != numFns) {
      Cerr << "Python matrix array must have " << numFns << " rows."
	   << std::endl;
      return(false);
    }
    for (int i=0; i<numFns; ++i) {
      val = PyList_GetItem(pyma, i);
      if (PyList_Check(val)) {
	if (!python_convert(val, rma[i]))
	  return(false);
      }
      else {
	Cerr << "Each row of Python matrix must be a list." << std::endl;
	Py_DECREF(val);
	return(false);
      }
    }
  }
  return(true);
}
#endif // DAKOTA_PYTHON

} // namespace Dakota
