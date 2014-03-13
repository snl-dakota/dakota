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
#include "LibraryEnvironment.hpp"
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
extern "C" int  nidr_save_exedir(const char*, int);

void run_dakota_parse(const char* dakota_input_file);
//namespace Dakota {
void run_dakota_data();
//} // namespace Dakota
void run_dakota_mixed(const char* dakota_input_file);
void interface_plugins(Dakota::LibraryEnvironment& env);
void model_interface_plugins(Dakota::LibraryEnvironment& env);


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
        (a) via the DB prior to Environment instantiation, and 
        (b) via Iterators/Models following Environment instantiation.
    Usage: dakota_library_mode [-m] [dakota.in] */

int main(int argc, char* argv[])
{
  // 3 ==> add both the directory containing this binary and . to the end
  // of $PATH if not already on $PATH.
  nidr_save_exedir(argv[0], 3);
  bool parallel = Dakota::MPIManager::detect_parallel_launch(argc, argv);

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
    /*Dakota::*/run_dakota_data();       // mode 2: data

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
  // Parse input and construct all Dakota objects.
  Dakota::ProgramOptions opts;
  opts.inputFile = dakota_input_file;
  Dakota::LibraryEnvironment env(opts);

  if (env.mpi_manager().world_rank() == 0)
    Cout << "Library mode 1: run_dakota_parse()\n";

  // convenience function for iterating over models and performing any
  // interface plug-ins
  model_interface_plugins(env);

  // Execute the environment
  env.execute();
}


/// Function to encapsulate the DAKOTA object instantiations for
/// mode 2: direct Data class instantiation.

/** Rather than parsing from an input file, this function populates
    Data class objects directly using a minimal specification and
    relies on constructor defaults and post-processing in
    post_process() to fill in the rest. */

void /*Dakota::*/run_dakota_data()
{
  // Instantiate the parallel library and problem description database objects
  Dakota::LibraryEnvironment env;

  // Specify output redirection & restart processing (if non-defaults)
  //env.specify_outputs_restart(NULL, NULL, NULL, NULL, NULL);

  Dakota::DataMethod   dme; Dakota::DataModel    dmo;
  Dakota::DataVariables dv; Dakota::DataInterface di; Dakota::DataResponses dr;
  Dakota::ParallelLibrary& parallel_lib = env.parallel_library();
  if (parallel_lib.world_rank() == 0) {
    Cout << "Library mode 2: run_dakota_data()\n";
    // This version uses direct Data instance population.  Initial instantiation
    // populates all the defaults.  Default Environment and Model data are used.
    Dakota::DataMethodRep*    dmr = dme.data_rep();
    Dakota::DataVariablesRep* dvr =  dv.data_rep();
    Dakota::DataInterfaceRep* dir =  di.data_rep();
    Dakota::DataResponsesRep* drr =  dr.data_rep();
    // Set any non-default values: mimic default_input
    dmr->methodName = Dakota::OPTPP_Q_NEWTON;
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
  }
  env.insert_nodes(dme, dmo, dv, di, dr);
  // BMA: temporary workaround until discuss design with Mike
  bool run_parser = false;
  env.construct(run_parser);

  // convenience function for iterating over models and performing any
  // interface plug-ins
  model_interface_plugins(env);

  // Execute the environment
  env.execute();
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
  my_callback_data *data = (my_callback_data*)ptr;
  Dakota::ProblemDescDB *db = data->db;

  // Do something to put the DB in a usable set/get state (unlock and set list
  // iterators).  The approach below is sufficient for simple input files, but
  // more advanced usage would require set_db_list_nodes() or equivalent.
  db->resolve_top_method();

  if (db->get_string("interface.type") != "direct")
    return;

  // supply labels, initial_point, and bounds
  // Both Rosenbrock and text_book have the same number of variables (2).
  Dakota::RealVector rv(2);
  const Dakota::StringArray& drivers
    = db->get_sa("interface.application.analysis_drivers");
  if (Dakota::contains(drivers, "plugin_rosenbrock")) {
    // Rosenbrock
    rv[0] = -1.2; rv[1] =  1.;
    db->set("variables.continuous_design.initial_point", rv);
    rv[0] = -2.;  rv[1] = -2.;
    db->set("variables.continuous_design.lower_bounds", rv);
    rv[0] =  2.;
    rv[1] =  2.;
    db->set("variables.continuous_design.upper_bounds", rv);
  }
  else if (Dakota::contains(drivers, "plugin_text_book")) {
    // text_book
    rv[0] =  0.2;  rv[1] =  1.1;
    db->set("variables.continuous_design.initial_point", rv);
    rv[0] =  0.5;  rv[1] = -2.9;
    db->set("variables.continuous_design.lower_bounds", rv);
    rv[0] =  5.8;  rv[1] =  2.9;
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
    Environment instantiation, (3) updates directly to Iterators/Models
    following Environment instantiation. */

void run_dakota_mixed(const char* dakota_input_file)
{
  // Instantiate the parallel library and problem description database objects
  Dakota::ProgramOptions opts;
  // in this use case, input file may be null:
  if (dakota_input_file)
    opts.inputFile = dakota_input_file;
  Dakota::LibraryEnvironment env(opts);

  // Specify output redirection & restart processing (if non-defaults)
  //env.specify_outputs_restart(NULL, NULL, NULL, NULL, NULL);

  Dakota::ProblemDescDB&   problem_db   = env.problem_description_db();
  Dakota::ParallelLibrary& parallel_lib = env.parallel_library();
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

  // BMA TODO: decide flow when using input file vs. string and encapsulate in LibraryEnvironment.  This second parse is only necessary if there's a callback function or data in a string...  Also don't want to call bcast, post_proc above...
  bool echo_input = true;
  problem_db.parse_inputs(dakota_input_file, std::string(), echo_input, 
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
      ip[0] =  1.1;  ip[1] = -1.3;
      problem_db.set("variables.continuous_design.initial_point", ip);
    }
  }
  // parse_inputs() used above in order to defer post-processing (especially any
  // large default vector creation) until after all inputs have been provided.
  problem_db.broadcast();
  problem_db.post_process();

  // instantiate the top-level Iterator and its Model
  // BMA: workaround until discuss design with Mike; in this case don't want to broadcast or post proc again either...
  bool run_parser = false;
  env.construct(run_parser);

  // Perform interface plug-ins.
  model_interface_plugins(env);

  // Demonstrate changes to data after the Environment has been instantiated.
  // In this case, the DB is not updated since its data has already been
  // extracted; rather, we must update the Environment's Iterators and Models
  // directly.  Iterator updates should be performed only on the Iterator
  // master processor, but Model updates are performed on all processors.
  Dakota::ModelList models
    = env.filtered_model_list("single", "direct", "plugin_text_book");
  Dakota::ModelLIter ml_iter;
  for (ml_iter = models.begin(); ml_iter != models.end(); ml_iter++) {
    const Dakota::StringArray& drivers
      = ml_iter->derived_interface().analysis_drivers();
    if (drivers.size() == 1 && drivers[0] == "plugin_text_book") {
      // Change initial guess:
      //ml_iter->continuous_variables(T);
      // Change a lower bound:
      Dakota::RealVector lb(ml_iter->continuous_lower_bounds()); // copy
      lb[0] += 0.1;
      ml_iter->continuous_lower_bounds(lb);
    }
  }

  // Execute the environment
  env.execute();
}


/** From a filtered list of Interface candidates, plug-in a derived
    direct application interface instance ("plugin_rosenbrock").  This
    approach is for use when you do not need access to Model-based
    parallel configuration information. */
void interface_plugins(Dakota::LibraryEnvironment& env)
{
  Dakota::InterfaceList filt_interfs
    = env.filtered_interface_list("direct", "plugin_rosenbrock");
  if (filt_interfs.empty()) {
    Cerr << "Error: no interface plugin performed.  Check compatibility "
	 << "between parallel configuration and selected analysis_driver."
	 << std::endl;
    Dakota::abort_handler(-1);
  }

  Dakota::ProblemDescDB& problem_db = env.problem_description_db();
  for (Dakota::InterfLIter il_iter = filt_interfs.begin();
       il_iter != filt_interfs.end(); ++il_iter) {
    // set DB nodes to input specification for this Interface
    problem_db.set_db_interface_node(il_iter->interface_id());
    // assign representation
    il_iter->assign_rep(new SIM::SerialDirectApplicInterface(problem_db),false);
  }
}


/** From a filtered list of Model candidates, plug-in a derived direct
    application interface instance ("plugin_rosenbrock" for serial,
    "plugin_text_book" for parallel).  This approach provides more complete
    access to the Model, e.g., for access to analysis communicators. */
void model_interface_plugins(Dakota::LibraryEnvironment& env)
{
  int initialized = 0;
#ifdef DAKOTA_HAVE_MPI
  MPI_Initialized(&initialized);
#endif // DAKOTA_HAVE_MPI

  Dakota::ModelList filt_models = (initialized) ?
    env.filtered_model_list("single", "direct", "plugin_text_book") :
    env.filtered_model_list("single", "direct", "plugin_rosenbrock");
  if (filt_models.empty()) {
    Cerr << "Error: no interface plugin performed.  Check compatibility "
	 << "between parallel configuration and selected analysis_driver."
	 << std::endl;
    Dakota::abort_handler(-1);
  }

  Dakota::ProblemDescDB& problem_db = env.problem_description_db();
  Dakota::ModelLIter ml_iter;
  for (ml_iter = filt_models.begin(); ml_iter != filt_models.end(); ++ml_iter) {
    // set DB nodes to input specification for this Model
    problem_db.set_db_model_nodes(ml_iter->model_id());

    Dakota::Interface& interface = ml_iter->derived_interface();
    if (initialized) {
      // Parallel case: plug in derived Interface object with an analysisComm.
      // Note: retrieval and passing of analysisComm is necessary only if
      // parallel operations will be performed in the derived constructor.

      // retrieve the currently active analysisComm from the Model.  In the most
      // general case, need an array of Comms to cover all Model configurations.
      const MPI_Comm& analysis_comm = ml_iter->parallel_configuration_iterator()
	->ea_parallel_level().server_intra_communicator();

      interface.assign_rep(new
	SIM::ParallelDirectApplicInterface(problem_db, analysis_comm), false);
    }
    else // Serial case: plug in derived Interface object w/o an analysisComm
      interface.assign_rep(new
	SIM::SerialDirectApplicInterface(problem_db), false);
  }
}
