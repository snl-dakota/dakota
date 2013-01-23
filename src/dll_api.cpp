/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Description: An API for launching DAKOTA from a DLL.
//- Owner:       Bill Hart
//- Checked by:
//- Version: $Id$

/** \file dll_api.cpp
    \brief This file contains a DakotaRunner class, which launches DAKOTA. */

// eventually use only _WIN32 here
#if defined(_WIN32) || defined(_MSC_VER) || defined(__MINGW32__)
#define BUILDING_DAKOTA_DLL
#include <windows.h>
#endif
#include "dakota_system_defs.hpp"
#include "ParallelLibrary.hpp"
#include "CommandLineHandler.hpp"
#include "ProblemDescDB.hpp"
#include "DakotaStrategy.hpp"
#include "PRPMultiIndex.hpp"
#ifdef DAKOTA_PLUGIN
#include "DakotaModel.hpp"
#include "DakotaInterface.hpp"
#include "PluginSerialDirectApplicInterface.hpp"
#endif
#include "dakota_global_defs.hpp"
#include "dakota_dll_api.h"
#include <string>

namespace Dakota {
  extern PRPCache data_pairs;
}

using namespace Dakota;

namespace {

/// initialize signal handlers
void signal_init()
{
// likely need more general _WIN32 here (or see active bug on Win32 defs)
#ifdef __MINGW32__
  signal(WM_QUIT, abort_handler);
  signal(WM_CHAR, abort_handler);
#else
  signal(SIGKILL, abort_handler);
  signal(SIGTERM, abort_handler);
#endif
  signal(SIGINT,  abort_handler);
}


class DakotaRunner
{
public:

  /// Construct a runner object, currently no use cases for argc > 0
  DakotaRunner(int _argc, char** _argv)
    : parallel_lib(0), argsPassed(_argc), numVars(0), varNames(NULL), numResp(0), respNames(NULL)
	{
	signal_init();
	// allocate additional pointers for the log and error files and the input argument
	//protect against possible usage without args
	// (need argv[0] for dakota parsing)
	if (_argc < 1) {
	  argc = 7;
	  argv = new char* [argc];
	  argv[0] = strdup("dakota_dll");
	}
	else {
	  argc = _argc+6;
	  argv = new char* [argc];
	  for (int i=0; i<_argc; i++) {
	    argv[i] = new char [strlen(_argv[i])+1];
	    strcpy(argv[i], _argv[i]);
	  }
	}
	parallel_lib=0;
	}

  ///
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

	if (parallel_lib) {
	  delete selected_strategy;
	  delete problem_db;
	  delete cmd_line_handler;
	  delete parallel_lib;
	}

	if (argv) {
	  if (argsPassed < 1) {
	    // all argv done with strdup (0 plus supplemental 1--6)
	    // memory allocated with strdup requires free, not delete
	    for (int i=0; i<argc; ++i)
	      std::free(argv[i]);
	  }
	  else {
	    // argv[0:argsPassed-1] new/strcpy
	    // new/strcp memory must be deleted
	    // remaining 6 strdup must be free-ed
	    int i=0;
	    for ( ; i<argsPassed; ++i)
	      delete argv[i];
	    for ( ; i<argc; ++i)
	      std::free(argv[i]);
	  }
	  delete[] argv;
	}

	}

  /// Add -output and -error arguments to the command line
  void set_output_file(const char* filename)
  {
    assert(filename);

    argv[argc-6] = strdup("-output");
    std::string tmps1(filename);
    tmps1 += ".log";
    argv[argc-5] = strdup(tmps1.c_str());

    argv[argc-4] = strdup("-error");
    std::string tmps2(filename);
    tmps2 += ".err";
    argv[argc-3] = strdup(tmps2.c_str());

  }


  ///
  void read_input(char* dakotaInput)
	{
	argv[argc-2] = strdup("-input");
	argv[argc-1] = strdup(dakotaInput);

  	// problem description database objects.  The ParallelLibrary constructor
  	// calls MPI_Init() if a parallel launch is detected.  This must precede
  	// CommandLineHandler initialization/parsing so that MPI may extract its
  	// command line arguments first, prior to DAKOTA command line extractions.
	if (parallel_lib) {
		delete parallel_lib;
		delete cmd_line_handler;
		delete problem_db;
		}
	parallel_lib = new ParallelLibrary(argc,argv);
	cmd_line_handler = new CommandLineHandler(argc,argv);
  	problem_db = new ProblemDescDB(*parallel_lib);
  	// Manage input file parsing, output redirection, and restart processing.
  	// Since all processors need the database, manage_inputs() does not require
  	// iterator partitions and it can precede init_iterator_communicators()
  	// (a simple world bcast is sufficient).  Output/restart management does
  	// utilize iterator partitions, so manage_outputs_restart() must follow
  	// init_iterator_communicators() within the Strategy constructor
  	// (output/restart options may only be specified at this time).
  	problem_db->manage_inputs(*cmd_line_handler);
  	parallel_lib->specify_outputs_restart(*cmd_line_handler);

	// Instantiate the Strategy object (which instantiates all Model and Iterator
	// objects) using the parsed information in problem_db.  All MPI communicator
	// partitions are created during strategy construction.
	// Do this here instead of in start, so we can get variable and response names
	// not sure why that's necessary
	selected_strategy = new Strategy(*problem_db);
	// initialize variable and response names
	initialize_names();

	}

  void initialize_names()
  {

    // set the variable names
    const VariablesList& vlist = problem_db->variables_list();
    VariablesList::const_iterator vlist_it;
    VariablesList::const_iterator vlist_end = vlist.end();

    // calculate total number of vars by iterating over each set
    // overly cautious check for non-empty labels (shouldn't they have defaults?)
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
    const ResponseList& rlist = problem_db->response_list();
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

  /** Plugin interfaces and execute strategy */
  void start();

  // pointers to allocated objects
  ParallelLibrary* parallel_lib;        ///< ptr to DAKOTA parallel library
  CommandLineHandler* cmd_line_handler; ///< ptr to DAKOTA command line handler
  ProblemDescDB* problem_db;            ///< ptr to DAKOTA problem DB
  Strategy* selected_strategy;          ///< ptr to DAKOTA strategy

  int argsPassed;    ///< number of args passed to runner constructor
  int argc;          ///< number of command-line args to pass to DAKOTA
  char** argv;       ///< adjusted command-line args to pass to DAKOTA

  static int id_ctr; ///< counter for next instance ID to return

  // for tracking variable and response names
  int numVars;       ///< number of variables active in DAKOTA
  char** varNames;   ///< array of strings of variable names
  int numResp;       ///< number of responses active in DAKOTA
  char** respNames;  ///< array of strings of response names

};

int DakotaRunner::id_ctr = 0;

void DakotaRunner::start()
{
  // Any library mode plug-ins would go here.
  // Refer to the library mode documentation in the Developers Manual.
#ifdef DAKOTA_PLUGIN
  ModelList& models = problem_db->model_list();
  for (ModelLIter ml_iter = models.begin(); ml_iter != models.end(); ml_iter++){
    Interface& interface = ml_iter->interface();
    if ( interface.interface_type() == "direct" &&
	 contains(interface.analysis_drivers(), "plugin_rosenbrock") ) {
      // set the DB nodes to that of the existing Model specification
      problem_db->set_db_model_nodes(ml_iter->model_id());
      // plug in the new derived Interface object
      interface.assign_rep(new SIM::SerialDirectApplicInterface(*problem_db), false);
    }
  }
#endif

  // Run the strategy
  if (cmd_line_handler->retrieve("check"))
    Cout << "\nDry run completed: input parsed and objects instantiated.\n"
	 << std::endl;
  else {

    // In case we're running a sequence of DAKOTA problems, make sure
    // the global evaluation cache is cleared in between runs.
    // Ideally, we'd manage this with interface IDs from the caller
    // instead of this aggressive clear.
    data_pairs.clear();

    problem_db->lock(); // prevent run-time DB queries
    selected_strategy->run_strategy();

  }
}

/// map from DakotaRunner id to instance
std::map<int,DakotaRunner*> runners;

} // end global namespace

extern "C" void DAKOTA_DLL_FN dakota_create(int* dakota_ptr_int, char* logname)
{ 
DakotaRunner* pDakota = new DakotaRunner(0, NULL);
// set logname for outfile, using default if provided NULL
pDakota->set_output_file(logname ? logname : "dakota_dll");
// increment the runner id and return to the caller
int id = DakotaRunner::id_ctr++;
runners[id] = pDakota;
*dakota_ptr_int = id;
}

extern "C" int DAKOTA_DLL_FN dakota_readInput(int id, char* dakotaInput)
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
