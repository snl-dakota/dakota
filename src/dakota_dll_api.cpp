/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Description: An API for launching DAKOTA from a DLL.
//- Owner:       Bill Hart
//- Checked by:
//- Version: $Id$

/** \file dakota_dll_api.cpp
    \brief This file contains a DakotaRunner class, which launches DAKOTA. */

#include "dakota_windows.h"
#include "dakota_system_defs.hpp"
#include "ProgramOptions.hpp"
#include "LibraryEnvironment.hpp"
#include "ProblemDescDB.hpp"
#include "PRPMultiIndex.hpp"
#include "DakotaModel.hpp"
#include "DakotaInterface.hpp"
#include "PluginSerialDirectApplicInterface.hpp"
#include "dakota_global_defs.hpp"
#include "dakota_dll_api.h"
#include <string>

#if defined(_WIN32) || defined(_MSC_VER) || defined(__MINGW32__)
#define BUILDING_DAKOTA_DLL
#endif


namespace Dakota {
  extern PRPCache data_pairs;
}

using namespace Dakota;

namespace {

/// initialize signal handlers (not using Dakota's helper function
/// since DLL may need different behavior.)
void signal_init()
{
#if defined(__MINGW32__) || defined(_MSC_VER)
  std::signal(SIGBREAK, abort_handler);
#else
  std::signal(SIGKILL, abort_handler);
#endif
  std::signal(SIGTERM, abort_handler);
  std::signal(SIGINT,  abort_handler);
}

/// Class to manage an instance of Dakota's library interface for
/// presentation to the Dakota DLL interface.
class DakotaRunner
{
public:

  /// Construct a runner object, setting output/error file names with logname
  DakotaRunner(std::string logname)
    : dakotaEnv(NULL), numVars(0), varNames(NULL), numResp(0), respNames(NULL)
  {
    signal_init();

    // Add -output and -error arguments to the command line with
    // filename.log and filename.err, respectively.
    progOpts.output_file(logname + ".log");
    progOpts.error_file(logname + ".err");
  }

  /// Destroy the runner object, freeing any allocated memory
  ~DakotaRunner()
  {
    if (respNames) {
      for (size_t i=0; i<numResp; i++) {
	// memory allocated with strdup requires free, not delete
	std::free(respNames[i]);
      }
      delete[] respNames;
    }

    if (varNames) {
      for (size_t i=0; i<numVars; i++) {
	// memory allocated with strdup requires free, not delete
	std::free(varNames[i]);
      }
      delete[] varNames;
    }

    if (dakotaEnv) {
      delete dakotaEnv;
      dakotaEnv = NULL;
    }
  }

  /// Set the input file and parse it, creating a Dakota
  /// LibraryEnvironment instance
  void read_input(const char* dakota_input)
  {
    progOpts.input_file(dakota_input);

    /// this shouldn't happen, but was a safeguard in historical code
    if (dakotaEnv)
      delete dakotaEnv;
    dakotaEnv = new LibraryEnvironment(progOpts);

    if (!dakotaEnv)
      throw std::logic_error("DakotaRunner: could not instantiate LibraryEnvironment");
      
    // initialize variable and response names
    initialize_names();
  }

  void initialize_names()
  {

    ProblemDescDB& problem_db = dakotaEnv->problem_description_db();

    // set the variable names
    const VariablesList& vlist = problem_db.variables_list();
    VariablesList::const_iterator vlist_it;
    VariablesList::const_iterator vlist_end = vlist.end();

    // calculate total number of vars by iterating over each set
    // overly cautious check for non-empty labels (shouldn't they have
    // defaults?)
    numVars = 0;
    for (vlist_it = vlist.begin(); vlist_it != vlist_end; ++vlist_it)
      numVars += vlist_it->all_continuous_variable_labels().size() + 
	vlist_it->all_discrete_int_variable_labels().size() + 
	vlist_it->all_discrete_real_variable_labels().size();
    // if appropriate, populate name array
    if (numVars > 0) {
      varNames = new char* [numVars]; 
      size_t j, idx = 0;
      for (vlist_it = vlist.begin(); vlist_it != vlist_end; ++vlist_it) {
	StringMultiArrayConstView acv_labels
	  = vlist_it->all_continuous_variable_labels();
	StringMultiArrayConstView adiv_labels
	  = vlist_it->all_discrete_int_variable_labels();
	StringMultiArrayConstView adrv_labels
	  = vlist_it->all_discrete_real_variable_labels();
	for (j=0; j<acv_labels.size(); ++j, ++idx)
	  varNames[idx] = strdup(acv_labels[j].c_str());
	for (j=0; j<adiv_labels.size(); ++j, ++idx)
	  varNames[idx] = strdup(adiv_labels[j].c_str());
	for (j=0; j<adrv_labels.size(); ++j, ++idx)
	  varNames[idx] = strdup(adrv_labels[j].c_str());
      }
    }

    // set the response names
    const ResponseList& rlist = problem_db.response_list();
    ResponseList::const_iterator rlist_it;
    ResponseList::const_iterator rlist_end = rlist.end();

    // calculate total number of responses by iterating over each set
    numResp = 0;
    for (rlist_it = rlist.begin(); rlist_it != rlist_end; ++rlist_it)
      numResp += rlist_it->function_labels().size();
    // if appropriate, populate name array
    if (numResp > 0) {
      respNames = new char* [numResp]; 
      size_t j, idx = 0;
      for (rlist_it = rlist.begin(); rlist_it != rlist_end; ++rlist_it) {
	const StringArray& fn_labels = rlist_it->function_labels();
	for (j=0; j<fn_labels.size(); ++j, ++idx)
	  respNames[idx] = strdup(fn_labels[j].c_str());
      }
    }

  }

  /// Plugin interfaces and execute strategy
  void start();

  // for tracking variable and response names
  int numVars;       ///< number of variables active in DAKOTA
  char** varNames;   ///< array of strings of variable names
  int numResp;       ///< number of responses active in DAKOTA
  char** respNames;  ///< array of strings of response names
  
  static int id_ctr; ///< counter for next instance ID to return

private:

  /// don't allow default construction due to memory management concerns
  DakotaRunner();
  // TOOD: disallow copy/assign as well

  /// Options to control the behavior of the Dakota instance
  ProgramOptions progOpts;
  /// Pointer to the Dakota instance
  LibraryEnvironment* dakotaEnv;

};

int DakotaRunner::id_ctr = 0;

void DakotaRunner::start()
{
  // Any library mode plug-ins would go here.
  // Refer to the library mode documentation in the Developers Manual.
  ProblemDescDB& problem_db = dakotaEnv->problem_description_db();
  ModelList& models = problem_db.model_list();
  size_t model_index = problem_db.get_db_model_node(); // for restoration
  for (ModelLIter ml_iter = models.begin(); ml_iter != models.end(); ml_iter++){
    Interface& model_interface = ml_iter->derived_interface();
    if ( (model_interface.interface_type() & DIRECT_INTERFACE_BIT) &&
	 contains(model_interface.analysis_drivers(), "plugin_rosenbrock") ) {
      // set the DB nodes to that of the existing Model specification
      problem_db.set_db_model_nodes(ml_iter->model_id());
      // plug in the new derived Interface object
      model_interface.assign_rep(new SIM::SerialDirectApplicInterface(problem_db), false);
    }
  }
  problem_db.set_db_model_nodes(model_index);            // restore

  // Execute the Dakota environment assume proceeding beyond help/version/check
  if (!dakotaEnv->check()) {

    // In case we're running a sequence of DAKOTA problems, make sure
    // the global evaluation cache is cleared in between runs.
    // Ideally, we'd manage this with interface IDs from the caller
    // instead of this aggressive clear.
    data_pairs.clear();

    dakotaEnv->execute();

  }
}

/// map from DakotaRunner id to instance
std::map<int ,DakotaRunner*> runners;

} // end global namespace

extern "C" void DAKOTA_DLL_FN dakota_create(int* dakota_ptr_int, const char* logname)
{ 
  // logname is the base filename for output and error to .log and .err
  std::string str_logname = logname ? logname : "dakota_dll";
  DakotaRunner* pDakota = new DakotaRunner(str_logname);
  // increment the runner id and return to the caller
  int id = DakotaRunner::id_ctr++;
  runners[id] = pDakota;
  *dakota_ptr_int = id;
}

extern "C" int DAKOTA_DLL_FN dakota_readInput(int id, const char* dakotaInput)
{ 
  try {
    runners[id]->read_input(dakotaInput);
  }
  catch (std::logic_error le) {
    Cout << "Dakota::dll_api readInput caught " << le.what() << std::endl;
    return(-2);
  }
  return(0);
}

extern "C" void DAKOTA_DLL_FN 
dakota_get_variable_info(int id,
			 char*** pVarNames, int* pNumVarNames, 
			 char*** pRespNames, int* pNumRespNames)
{
  *pNumVarNames = runners[id]->numVars;
  *pVarNames = runners[id]->varNames;
  *pNumRespNames = runners[id]->numResp;
  *pRespNames = runners[id]->respNames;
}


extern "C" int DAKOTA_DLL_FN dakota_start(int id)
{
  try {
    runners[id]->start();
  }
  catch (std::logic_error le) {
    Cout << "Dakota::dll_api start caught " << le.what() << std::endl;
    return(-1);
  }
  return(0);
}

extern "C" void DAKOTA_DLL_FN dakota_destroy (int id)
{ 
  delete runners[id];
  runners.erase(id);
}

extern "C" void DAKOTA_DLL_FN dakota_stop(int* id)
{
/** TODO: trick application to quit through the syscall interface or
    throw exception. **/
}

extern "C" const char* DAKOTA_DLL_FN dakota_getStatus(int id)
{
  static std::string tmp;
  tmp = "<DakotaOutput>None</DakotaOutput>";
  return tmp.c_str();
}

extern "C" int get_mc_ptr_int()
{
#ifdef DAKOTA_MODELCENTER
  return Dakota::mc_ptr_int;
#else
  return 0;
#endif
}

extern "C" void set_mc_ptr_int(int ptr_int)
{
#ifdef DAKOTA_MODELCENTER
  Dakota::mc_ptr_int = ptr_int;
#endif
}

extern "C" int get_dc_ptr_int()
{
#ifdef DAKOTA_MODELCENTER
  return Dakota::dc_ptr_int;
#else
  return 0;
#endif
}

extern "C" void set_dc_ptr_int(int ptr_int)
{
#ifdef DAKOTA_MODELCENTER
  Dakota::dc_ptr_int = ptr_int;
#endif
}
