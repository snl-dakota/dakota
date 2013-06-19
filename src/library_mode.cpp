/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Description: A mock simulator main for testing DAKOTA in library mode.
//-              Uses alternative instantiation syntax as described in the
//-              library mode docs within the Developers Manual.
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: library_mode.cpp 5063 2008-06-05 02:08:06Z mseldre $

/** \file library_mode.cpp
    \brief file containing a mock simulator main for testing DAKOTA in
    library mode */

#include "ParallelLibrary.hpp"
#include "ProblemDescDB.hpp"
#include "DakotaStrategy.hpp"
#include "DakotaModel.hpp"
#include "DakotaInterface.hpp"
#include "PluginSerialDirectApplicInterface.hpp"
#include "PluginParallelDirectApplicInterface.hpp"

//#define MPI_DEBUG
#if defined(MPI_DEBUG) && defined(MPICH2)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

/// Set input to NIDR via string argument instead of input file
extern "C" void nidr_set_input_string(const char *);
extern "C" int nidr_save_exedir(const char*, int);

void run_dakota_parse(const char* dakota_input_file);
namespace Dakota {
void run_dakota_data();
} // namespace Dakota
void run_dakota_mixed(const char* dakota_input_file);
void model_interface_plugins(Dakota::ProblemDescDB& problem_db);


/// A mock simulator main for testing DAKOTA in library mode.

/** Uses alternative instantiation syntax as described in the library
    mode documentation within the Developers Manual.  Tests several
    problem specification modes:
    (1) run_dakota_parse: reads all problem specification data from an
        input file
    (2) run_dakota_data:  creates all problem specification from direct
        Data instance instantiations.
    (3) run_dakota_mixed: a mixture of input parsing (by file or default 
        string) and direct data updates, where the data updates occur:
        (a) via the DB prior to Strategy instantiation, and 
        (b) via Iterators/Models following Strategy instantiation.
    Usage: dakota_library_mode [-m] [dakota.in] */

int main(int argc, char* argv[])
{
  // 3 ==> add both the directory containing this binary and . to the end
  // of $PATH if not already on $PATH.
  nidr_save_exedir(argv[0], 3);
  bool parallel = Dakota::ParallelLibrary::detect_parallel_launch(argc, argv);

#ifdef MPI_DEBUG
  // hold parallel job prior to MPI_Init() in order to attach debugger to
  // master process.  Then step past ParallelLibrary instantiation and attach
  // debugger to other processes.
#ifdef MPICH2
  // To use this approach, set $DAKOTA_DEBUGPIPE to a suitable name,
  // and create $DAKOTA_DEBUGPIPE by executing "mkfifo $DAKOTA_DEBUGPIPE".
  // After invoking "mpirun ... dakota ...", find the processes, invoke
  // a debugger on them, set breakpoints, and execute "echo >$DAKOTA_DEBUGPIPE"
  // to write something to $DAKOTA_DEBUGPIPE, thus releasing dakota from
  // a wait at the open invocation below.
  char *pname; int dfd;
  if ( ( pname = getenv("DAKOTA_DEBUGPIPE") ) &&
       ( dfd = open(pname,O_RDONLY) ) > 0 ) {
    char buf[80];
    read(dfd,buf,sizeof(buf));
    close(dfd);
  }
#else
  // This simple scheme has been observed to fail with MPICH2
  int test;
  std::cin >> test;
#endif // MPICH2
#endif // MPI_DEBUG

#ifdef DAKOTA_HAVE_MPI
  if (parallel)
    MPI_Init(&argc, &argv); // initialize MPI
#endif // DAKOTA_HAVE_MPI

  // Allow MPI to extract its command line arguments first,
  // then detect "-m" and dakota_input_file
  bool mixed_input = false;
  const char *dakota_input_file = NULL;
  if (argc > 1) {
    if (!strcmp(argv[1],"-m")) {
      mixed_input = true;
      if (argc > 2)
	dakota_input_file = argv[2];
    }
    else
      dakota_input_file = argv[1];
  }

  // DAKOTA objects need to go out of scope prior to MPI_Finalize so
  // that MPI code in destructors works properly in library mode.
  if (mixed_input)
    run_dakota_mixed(dakota_input_file); // mode 3: mixed
  else if (dakota_input_file)
    run_dakota_parse(dakota_input_file); // mode 1: parse
  else
    Dakota::run_dakota_data();           // mode 2: data

#ifdef DAKOTA_HAVE_MPI
  if (parallel)
    MPI_Finalize(); // finalize MPI
#endif // DAKOTA_HAVE_MPI

  return 0;
}


/// Function to encapsulate the DAKOTA object instantiations for
/// mode 1: parsing an input file.

/** This function parses from an input file to define the
    ProblemDescDB data. */

void run_dakota_parse(const char* dakota_input_file)
{
  // Instantiate/initialize the parallel library and problem description
  // database objects.
  Dakota::ParallelLibrary parallel_lib;
  Dakota::ProblemDescDB problem_db(parallel_lib);

  if (parallel_lib.world_rank() == 0)
    Cout << "Library mode 1: run_dakota_parse()\n";

  // specify_outputs_restart() is only necessary if specifying non-defaults
  //parallel_lib.specify_outputs_restart("dakota.dak", NULL, NULL, NULL);

  // Manage input file parsing, output redirection, and restart processing
  // without a CommandLineHandler.  This version relies on parsing of an
  // input file.
  problem_db.manage_inputs(dakota_input_file);

  // Instantiate the Strategy object (which instantiates all Model and
  // Iterator objects) using the parsed information in problem_db.
  Dakota::Strategy selected_strategy(problem_db);

  // convenience function for iterating over models and performing any
  // interface plug-ins
  model_interface_plugins(problem_db);

  // Execute the strategy
  problem_db.lock(); // prevent run-time DB queries
  selected_strategy.run_strategy();
}


/// Function to encapsulate the DAKOTA object instantiations for
/// mode 2: direct Data class instantiation.

/** Rather than parsing from an input file, this function populates
    Data class objects directly using a minimal specification and
    relies on constructor defaults and post-processing in
    post_process() to fill in the rest. */

void Dakota::run_dakota_data()
{
  // Instantiate/initialize the parallel library and problem description
  // database objects.
  Dakota::ParallelLibrary parallel_lib;
  Dakota::ProblemDescDB problem_db(parallel_lib);

  // Manage output redirection & restart processing w/o a CommandLineHandler
  // (only necessary if specifying non-defaults):
  //parallel_lib.specify_outputs_restart(NULL, NULL, NULL, NULL);

  if (parallel_lib.world_rank() == 0) {
    Cout << "Library mode 2: run_dakota_data()\n";
    // This version uses direct Data instance population.  Initial instantiation
    // populates all the defaults.  Default Strategy and Model data are used.
    Dakota::DataMethod    dm;  Dakota::DataMethodRep*    dmr = dm.dataMethodRep;
    Dakota::DataVariables dv;  Dakota::DataVariablesRep* dvr = dv.dataVarsRep;
    Dakota::DataInterface di;  Dakota::DataInterfaceRep* dir = di.dataIfaceRep;
    Dakota::DataResponses dr;  Dakota::DataResponsesRep* drr = dr.dataRespRep;
    // Set any non-default values: mimic default_input
    dmr->methodName = "optpp_q_newton";
    dvr->numContinuousDesVars = 2;
    dir->interfaceType = "direct";
    if (parallel_lib.mpirun_flag()) {
      dir->analysisDrivers.push_back("plugin_text_book");
      drr->numObjectiveFunctions = 1;
      drr->numNonlinearIneqConstraints = 2;
    }
    else {
      dir->analysisDrivers.push_back("plugin_rosenbrock");
      drr->numObjectiveFunctions = 1;
    }
    drr->gradientType = "analytic";
    drr->hessianType  = "none";
    // Push instances into the DB
    problem_db.insert_node(dm);
    problem_db.insert_node(dv);
    problem_db.insert_node(di);
    problem_db.insert_node(dr);
    // Sanity checking on minimal specification
    problem_db.check_input();
  }
  // broadcast minimal DB specification to other processors (if needed)
  problem_db.broadcast();
  // Perform post-processing of minimal specification on all processors
  problem_db.post_process();

  // Instantiate the Strategy object (which instantiates all Model and
  // Iterator objects) using the parsed information in problem_db.
  Dakota::Strategy selected_strategy(problem_db);

  // convenience function for iterating over models and performing any
  // interface plug-ins
  model_interface_plugins(problem_db);

  // Execute the strategy
  problem_db.lock(); // prevent run-time DB queries
  selected_strategy.run_strategy();
}


struct my_callback_data {
  Dakota::ProblemDescDB *db;
  // and in general other application-specific stuff needed
  // for passing bounds, initial points, etc., to DAKOTA.
};

/** Example of user-provided callback function to override input
    specified and managed by NIDR, e.g., from an input deck. */
static void my_callback_function(void *ptr)
{
  // supply labels, initial_point, and bounds
  // Both Rosenbrock and text_book happen to have
  // the same number of decision variables (2).

  my_callback_data *data = (my_callback_data*)ptr;
  Dakota::ProblemDescDB *db = data->db;

  // Do something to put the DB in a usable set/get state (unlock and set list
  // iterators).  The approach below is sufficient for simple input files, but
  // more advanced usage would require set_db_list_nodes() or equivalent.
  db->resolve_top_method();

  if (db->get_string("interface.type") != "direct")
    return;
  const Dakota::StringArray& drivers
    = db->get_sa("interface.application.analysis_drivers");
  if (Dakota::contains(drivers, "plugin_rosenbrock")) {
    // Rosenbrock
    Dakota::RealVector rv(2);
    rv[0] = -1.2;
    rv[1] =  1.;
    db->set("variables.continuous_design.initial_point", rv);
    rv[0] = -2.;
    rv[1] = -2.;
    db->set("variables.continuous_design.lower_bounds", rv);
    rv[0] =  2.;
    rv[1] =  2.;
    db->set("variables.continuous_design.upper_bounds", rv);
  }
  else if (Dakota::contains(drivers, "plugin_text_book")) {
    // text_book
    Dakota::RealVector rv(2);
    rv[0] =  0.2;
    rv[1] =  1.1;
    db->set("variables.continuous_design.initial_point", rv);
    rv[0] =  0.5;
    rv[1] = -2.9;
    db->set("variables.continuous_design.lower_bounds", rv);
    rv[0] =  5.8;
    rv[1] =  2.9;
    db->set("variables.continuous_design.upper_bounds", rv);
  }
}


/// Function to encapsulate the DAKOTA object instantiations for
/// mode 3: mixed parsing and direct updating

/** This function showcases multiple features.  For parsing, either an
    input file (dakota_input_file != NULL) or a default input string
    (dakota_input_file == NULL) are shown.  This parsed input is then
    mixed with input from three sources: (1) input from a
    user-supplied callback function, (2) updates to the DB prior to
    Strategy instantiation, (3) updates directly to Iterators/Models
    following Strategy instantiation. */

void run_dakota_mixed(const char* dakota_input_file)
{
  // Instantiate/initialize the parallel library and problem description
  // database objects.
  Dakota::ParallelLibrary parallel_lib;
  Dakota::ProblemDescDB problem_db(parallel_lib);

  // specify_outputs_restart() is only necessary if specifying non-defaults
  //parallel_lib.specify_outputs_restart(NULL, NULL, NULL, NULL);

  int world_rank = parallel_lib.world_rank();
  if (world_rank == 0) {
    Cout << "Library mode 3: run_dakota_mixed()\n";

    if (!dakota_input_file) {
      // define a default set of input strings that are used when no
      // dakota_input_file is provided
      if (parallel_lib.mpirun_flag()) {
	// parallel default:
	static const char default_input[] = 
	  "	method,"
	  "		optpp_q_newton"
	  "		  max_iterations = 50"
	  "		  convergence_tolerance = 1e-4"
	  "	variables,"
	  "		continuous_design = 2"
	  "		  descriptors 'x1' 'x2'"
	  "	interface,"
	  "		direct"
	  "		  analysis_driver = 'plugin_text_book'"
	  "	responses,"
	  "		num_objective_functions = 1"
	  "		num_nonlinear_inequality_constraints = 2"
	  "		analytic_gradients"
	  "		no_hessians";
	nidr_set_input_string(default_input);
      }
      else {
	// serial default:
	static const char default_input[] = 
	  "	method,"
	  "		optpp_q_newton"
	  "		  max_iterations = 50"
	  "		  convergence_tolerance = 1e-4"
	  "	variables,"
	  "		continuous_design = 2"
	  "		  descriptors 'x1' 'x2'"
	  "	interface,"
	  "		direct"
	  "		  analysis_driver = 'plugin_rosenbrock'"
	  "	responses,"
	  "		num_objective_functions = 1"
	  "		analytic_gradients"
	  "		no_hessians";
	nidr_set_input_string(default_input);
      }
      // The above strings could have included comments, provided a \n followed
      // each comment.  Before each new keyword, some white space (a blank, as
      // above, or \n) must appear.
    }
  }

  // Manage input file parsing, output redirection, and restart processing
  // without a CommandLineHandle.  This version optionally uses an input file.
  my_callback_data data;
  data.db = &problem_db;
  // In mixed mode, use parse_inputs() instead of manage_inputs() in order to
  // defer broadcast() & post_process() until all DB inputs have been provided.
  bool echo_input = true;
  problem_db.parse_inputs(dakota_input_file, NULL, echo_input, 
			  my_callback_function, &data);

  // Demonstrate changes to DB data initially set by parse_inputs():
  // if we're using rosenbrock, change the initial guess.  This update is
  // performed only on rank 0 and then problem_db.broadcast() is used.
  if (world_rank == 0) {
    problem_db.resolve_top_method(); // allow DB set/get operations
    const Dakota::StringArray& drivers
      = problem_db.get_sa("interface.application.analysis_drivers");
    if (drivers.size() == 1 && drivers[0] == "plugin_rosenbrock") {
      Dakota::RealVector ip(2);
      ip[0] =  1.1;
      ip[1] = -1.3;
      problem_db.set("variables.continuous_design.initial_point", ip);
    }
  }
  // parse_inputs() used above in order to defer post-processing (especially any
  // large default vector creation) until after all inputs have been provided.
  problem_db.broadcast();
  problem_db.post_process();

  // Instantiate the Strategy object (which instantiates all Model and
  // Iterator objects) using the parsed information in problem_db.
  Dakota::Strategy selected_strategy(problem_db);

  // Perform interface plug-ins.
  model_interface_plugins(problem_db);

  // Demonstrate changes to data after the strategy has been instantiated.
  // In this case, the DB is not updated since its data has already been
  // extracted; rather, we must update the Strategy's Iterators and Models
  // directly.  Iterator updates should be performed only on the Iterator
  // master processor, but Model updates are performed on all processors.
  Dakota::ModelList& models = problem_db.model_list();
  Dakota::ModelLIter ml_iter;
  for (ml_iter = models.begin(); ml_iter != models.end(); ml_iter++) {
    const Dakota::StringArray& drivers
      = ml_iter->interface().analysis_drivers();
    if (drivers.size() == 1 && drivers[0] == "plugin_text_book") {
      // Change initial guess:
      //ml_iter->continuous_variables(T);
      // Change a lower bound:
      Dakota::RealVector lb(ml_iter->continuous_lower_bounds()); // copy
      lb[0] += 0.1;
      ml_iter->continuous_lower_bounds(lb);
    }
  }

  // Execute the strategy
  problem_db.lock(); // prevent run-time DB queries
  selected_strategy.run_strategy();
}


/** Iterate over models and plugin appropriate interface: serial
    rosenbrock or parallel textbook. */
void model_interface_plugins(Dakota::ProblemDescDB& problem_db)
{
  int initialized = 0;
#ifdef DAKOTA_HAVE_MPI
  MPI_Initialized(&initialized);
#endif // DAKOTA_HAVE_MPI

  // Library mode interface plug-ins.
  // Model updates are performed on all processors.
  Dakota::ModelList& models = problem_db.model_list();
  Dakota::ModelLIter ml_iter;
  for (ml_iter = models.begin(); ml_iter != models.end(); ml_iter++) {
    Dakota::Interface& interface = ml_iter->interface();
    bool rosen_plugin   = interface.interface_type() == "direct" &&
      Dakota::contains(interface.analysis_drivers(), "plugin_rosenbrock");
    bool textbook_plugin = interface.interface_type() == "direct" &&
      Dakota::contains(interface.analysis_drivers(), "plugin_text_book");
    bool serial_plugin   = !initialized && rosen_plugin;
    bool parallel_plugin =  initialized && textbook_plugin;
    // Either case: set DB nodes to the existing Model specification
    if (serial_plugin || parallel_plugin)
      problem_db.set_db_model_nodes(ml_iter->model_id());
    // Serial case: plug in derived Interface object without an analysisComm
    if (serial_plugin)
      interface.assign_rep(new
	SIM::SerialDirectApplicInterface(problem_db), false);
    // Parallel case: plug in derived Interface object, passing analysisComm(s).
    // Note: retrieval and passing of analysisComm(s) is necessary only if
    // parallel operations will be performed in the derived constructor.
    else if (parallel_plugin) {
      // retrieve the currently active analysisComm from the Model.  In the most
      // general case, need an array of Comms to cover all Model configurations.
      const Dakota::ParallelLevel& ea_level
	= ml_iter->parallel_configuration_iterator()->ea_parallel_level();
      const MPI_Comm& analysis_comm = ea_level.server_intra_communicator();
      interface.assign_rep(new
	SIM::ParallelDirectApplicInterface(problem_db, analysis_comm), false);
    }
    else {
      Cerr << "Error: no interface plugin performed.  Check compatibility "
	   << "between parallel configuration and selected analysis_driver."
	   << std::endl;
      Dakota::abort_handler(-1);
    }
  }
}


