/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Description: A mock simulator main for testing Dakota in library mode.
//-              Uses alternative instantiation syntax as described in the
//-              library mode docs within the Developers Manual.
//- Owner:       Mike Eldred
//- Checked by:  Brian Adams
//- Version: $Id: library_mode.cpp 5063 2008-06-05 02:08:06Z mseldre $

/** \file library_mode.cpp
    \brief file containing a mock simulator main for testing Dakota in
    library mode */

#include "ParallelLibrary.hpp"
#include "ProblemDescDB.hpp"
#include "LibraryEnvironment.hpp"
#include "DakotaModel.hpp"
#include "DakotaInterface.hpp"
#include "PluginSerialDirectApplicInterface.hpp"
#include "PluginParallelDirectApplicInterface.hpp"

#ifdef HAVE_AMPL 
/** Floating-point initialization from AMPL: switch to 53-bit rounding
    if appropriate, to eliminate some cross-platform differences. */
extern "C" void fpinit_ASL(); 
#endif 

#ifndef DAKOTA_HAVE_MPI
#define MPI_COMM_WORLD 0
#endif // not DAKOTA_HAVE_MPI

/// Run a Dakota LibraryEnvironment, mode 1: parsing an input file
void run_dakota_parse(const char* dakota_input_file);

//namespace Dakota {
/// Run a Dakota LibraryEnvironment, mode 2: from C++ API inserted data
void run_dakota_data();
//} // namespace Dakota

/// Run a Dakota LibraryEnvironment, from string or input file input,
/// supplemented with additional C++ API adjustments
void run_dakota_mixed(const char* dakota_input_file, bool mpirun_flag);

/// Convenience function with simplest example of interface plugin: plugin a serial
/// DirectApplicInterface that can be constructed independent of Dakota's 
/// configuration details.
void serial_interface_plugin(Dakota::LibraryEnvironment& env);

/// Convenience function to plug a library client's interface into the appropriate 
/// model, demonstrating use of Dakota parallel configuration in constructing the 
/// plugin Interface on the right MPI_Comm
void parallel_interface_plugin(Dakota::LibraryEnvironment& env);

/** Data structure to pass application-specific values through Dakota
    back to the callback function, for example to convey late updates
    to bounds, initial points, etc., to Dakota. */
struct callback_data {
  /// upper bound value to pass through parser to callback function
  double rosen_cdv_upper_bd;
};

/// Example: user-provided post-parse callback (Dakota::DbCallbackFunction)
static void callback_function(Dakota::ProblemDescDB* db, void *ptr);


/// A mock simulator main for testing Dakota in library mode.

/** Overall Usage: dakota_library_mode [-mixed] [dakota.in] 

    Uses alternative instantiation syntax as described in the library
    mode documentation within the Developers Manual.  Tests several
    problem specification modes:

    (1) run_dakota_parse: reads all problem specification data from a
        Dakota input file.  Usage:
	  dakota_library_mode dakota.in

    (2) run_dakota_data: creates all problem specification from direct
        Data instance instantiations in the C++ code. Usage:
	  dakota_library_mode

    (3) run_dakota_mixed: a mixture of input parsing and direct data updates,
        where the data updates occur:
        (a) via the DB during Environment instantiation, and 
        (b) via Iterators/Models following Environment instantiation.
        Usage:
	  dakota_library_mode -mixed            (input from default string)
	  dakota_library_mode -mixed dakota.in  (input from specified file)

    Serial cases use a plugin rosenbrock model, while parallel cases
    use textbook.
*/
int main(int argc, char* argv[])
{
#ifdef HAVE_AMPL
  // Switch to 53-bit rounding if appropriate, to eliminate some
  // cross-platform differences.
  fpinit_ASL();	
#endif

  // whether running in parallel
  bool parallel = Dakota::MPIManager::detect_parallel_launch(argc, argv);

  // Define MPI_DEBUG in dakota_global_defs.cpp to cause a hold here
  Dakota::mpi_debug_hold();

#ifdef DAKOTA_HAVE_MPI
  if (parallel)
    MPI_Init(&argc, &argv); // initialize MPI
#endif // DAKOTA_HAVE_MPI

  // Allow MPI to extract its command line arguments first in detect above,
  // then detect "-mixed" and dakota_input_file
  bool mixed_input = false;
  const char *dakota_input_file = NULL;
  if (argc > 1) {
    if (!strcmp(argv[1],"-mixed")) {
      mixed_input = true;
      if (argc > 2)
	dakota_input_file = argv[2];
    }
    else
      dakota_input_file = argv[1];
  }

  if (mixed_input)
    run_dakota_mixed(dakota_input_file, parallel); // mode 3: mixed
  else if (dakota_input_file)
    run_dakota_parse(dakota_input_file); // mode 1: parse
  else
    /*Dakota::*/run_dakota_data();       // mode 2: data

  // Note: Dakota objects created in above function calls need to go
  // out of scope prior to MPI_Finalize so that MPI code in
  // destructors works properly in library mode.

#ifdef DAKOTA_HAVE_MPI
  if (parallel)
    MPI_Finalize(); // finalize MPI
#endif // DAKOTA_HAVE_MPI

  return 0;
}


// Default input for mixed cases where input file not used.  The strings may 
// include comments, provided a \n follows each comment.  Before each new 
// keyword, some white space (a blank or newline) must appear.

/// Default Dakota input string for serial case (rosenbrock):
static const char serial_input[] = 
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

/// Default Dakota input string for parallel case (text_book)
static const char parallel_input[] = 
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


/** Simplest library case: this function parses from an input file to define the
    ProblemDescDB data. */
void run_dakota_parse(const char* dakota_input_file)
{
  // Parse input and construct Dakota LibraryEnvironment, performing
  // input data checks
  Dakota::ProgramOptions opts;
  opts.input_file(dakota_input_file);

  // Defaults constructs the MPIManager, which assumes COMM_WORLD
  Dakota::LibraryEnvironment env(opts);

  if (env.mpi_manager().world_rank() == 0)
    Cout << "Library mode 1: run_dakota_parse()\n";

  // plug the client's interface (function evaluator) into the Dakota
  // environment; in serial case, demonstrate the simpler plugin method
  if (env.mpi_manager().mpirun_flag())
    parallel_interface_plugin(env);
  else
    serial_interface_plugin(env);

  // Execute the environment
  env.execute();
}


/** Rather than parsing from an input file, this function populates
    Data class objects directly using a minimal specification and
    relies on constructor defaults and post-processing in
    post_process() to fill in the rest. */
void /*Dakota::*/run_dakota_data()
{
  // Instantiate the LibraryEnvironment and underlying ProblemDescDB

  // No input file set --> no parsing.  Could set other command line
  // options such as restart in opts:
  Dakota::ProgramOptions opts;

  // delay validation/sync of the Dakota database and iterator
  // construction to allow update after all data is populated
  bool check_bcast_construct = false;

  // set up a Dakota instance, with the right MPI configuration if a
  // parallel run (don't need to pass the MPI comm here, just doing to
  // demonstrate/test).
  Dakota::LibraryEnvironment env(MPI_COMM_WORLD, opts, check_bcast_construct);

  // configure Dakota to throw a std::runtime_error instead of calling exit
  env.exit_mode("throw");

  // Now set the various data to specify the Dakota study
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
    dir->interfaceType = Dakota::TEST_INTERFACE;
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

  // once done with changes: check database, broadcast, and construct iterators
  env.done_modifying_db();

  // plug the client's interface (function evaluator) into the Dakota
  // environment; in serial case, demonstrate the simpler plugin method
  if (env.mpi_manager().mpirun_flag())
    parallel_interface_plugin(env);
  else
    serial_interface_plugin(env);

  // Execute the environment
  env.execute();
}



/// Function to encapsulate the Dakota object instantiations for
/// mode 3: mixed parsing and direct updating

/** This function showcases multiple features.  For parsing, either an
    input file (dakota_input_file != NULL) or a default input string
    (dakota_input_file == NULL) are shown.  This parsed input is then
    mixed with input from three sources: (1) input from a
    user-supplied callback function, (2) updates to the DB prior to
    Environment instantiation, (3) updates directly to Iterators/Models
    following Environment instantiation. */
void run_dakota_mixed(const char* dakota_input_file, bool mpirun_flag)
{
  Dakota::ProgramOptions opts;
  // Could specify output redirection & restart processing in opts if needed
  opts.echo_input(true);

  // in this use case, input file may be null:
  if (dakota_input_file)
    opts.input_file(dakota_input_file);

  // when no input file, use input string appropraite for MPI mode
  if (!dakota_input_file) {
    if (mpirun_flag)
      opts.input_string(parallel_input);
    else
      opts.input_string(serial_input);
  }
  
  // Setup client data to be available during callback: upper variable bound
  callback_data data;
  data.rosen_cdv_upper_bd = 2.0;

  // Construct library environment, parsing input file or string, then
  // calling back to the callback_function, passing data to it.  

  // However delay braodcast and validation of the db due to further
  // data manipulations below, e.g., to avoid large default vector
  // creation)
  bool done_with_db = false;

  Dakota::LibraryEnvironment env(opts, done_with_db, callback_function, &data);

  Dakota::ParallelLibrary& parallel_lib = env.parallel_library();
  int world_rank = parallel_lib.world_rank();
  if (world_rank == 0)
    Cout << "Library mode 3: run_dakota_mixed()\n";

  // Demonstrate changes to DB data initially set by parse_inputs():
  // if we're using rosenbrock, change the initial guess.  This update is
  // performed only on rank 0
  Dakota::ProblemDescDB&   problem_db   = env.problem_description_db();
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

  // check, broadcast to sync DB data across ranks , and construct
  // iterators/models
  env.done_modifying_db();

  // plug the client's interface (function evaluator) into the Dakota
  // environment; in serial case, demonstrate the simpler plugin method
  if (env.mpi_manager().mpirun_flag())
    parallel_interface_plugin(env);
  else
    serial_interface_plugin(env);

  // Demonstrate changes to data after the Environment has been instantiated.
  // In this case, the DB is not updated since its data has already been
  // extracted; rather, we must update the Environment's Iterators and Models
  // directly.  Iterator updates should be performed only on the Iterator
  // master processor, but Model updates are performed on all processors.
  Dakota::ModelList models
    = env.filtered_model_list("simulation", "direct", "plugin_text_book");
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


/** Demonstration of simple plugin where client code doesn't require
    access to detailed Dakota data (such as Model-based parallel
    configuration information) to construct the DirectApplicInterface.
    This example plugs-in a derived serial direct application
    interface instance ("plugin_rosenbrock"). */
void serial_interface_plugin(Dakota::LibraryEnvironment& env)
{
  std::string model_type(""); // demo: empty string will match any model type
  std::string interf_type("direct");
  std::string an_driver("plugin_rosenbrock");

  Dakota::ProblemDescDB& problem_db = env.problem_description_db();
  Dakota::Interface* serial_iface = 
    new SIM::SerialDirectApplicInterface(problem_db);

  bool plugged_in =
    env.plugin_interface(model_type, interf_type, an_driver, serial_iface);

  if (!plugged_in) {
    Cerr << "Error: no serial interface plugin performed.  Check "
	 << "compatibility between parallel\n       configuration and "
	 << "selected analysis_driver." << std::endl;
    Dakota::abort_handler(-1);
  }
}


/** From a filtered list of Model candidates, plug-in a derived direct
    application interface instance ("plugin_text_book" for parallel).
    This approach provides more complete access to the Model, e.g.,
    for access to analysis communicators. */
void parallel_interface_plugin(Dakota::LibraryEnvironment& env)
{
  // get the list of all models matching the specified model, interface, driver:
  Dakota::ModelList filt_models = 
    env.filtered_model_list("simulation", "direct", "plugin_text_book");
  if (filt_models.empty()) {
    Cerr << "Error: no parallel interface plugin performed.  Check "
	 << "compatibility between parallel\n       configuration and "
	 << "selected analysis_driver." << std::endl;
    Dakota::abort_handler(-1);
  }

  Dakota::ProblemDescDB& problem_db = env.problem_description_db();
  Dakota::ModelLIter ml_iter;
  size_t model_index = problem_db.get_db_model_node(); // for restoration
  for (ml_iter = filt_models.begin(); ml_iter != filt_models.end(); ++ml_iter) {
    // set DB nodes to input specification for this Model
    problem_db.set_db_model_nodes(ml_iter->model_id());

    Dakota::Interface& model_interface = ml_iter->derived_interface();

    // Parallel case: plug in derived Interface object with an analysisComm.
    // Note: retrieval and passing of analysisComm is necessary only if
    // parallel operations will be performed in the derived constructor.

    // retrieve the currently active analysisComm from the Model.  In the most
    // general case, need an array of Comms to cover all Model configurations.
    const MPI_Comm& analysis_comm = ml_iter->analysis_comm();

    // don't increment ref count since no other envelope shares this letter
    model_interface.assign_rep(new
      SIM::ParallelDirectApplicInterface(problem_db, analysis_comm), false);
  }
  problem_db.set_db_model_nodes(model_index);            // restore
}


/** Example of user-provided callback function (an instance of
    Dakota::DbCallbackFunction) to override input provided by parsed Dakota
    input file or input string data.  */
static void callback_function(Dakota::ProblemDescDB* db, void *ptr)
{
  callback_data *my_data = (callback_data*)ptr;
  double my_rosen_ub = my_data->rosen_cdv_upper_bd;

  // Do something to put the DB in a usable set/get state (unlock and set list
  // iterators).  The approach below is sufficient for simple input files, but
  // more advanced usage would require set_db_list_nodes() or equivalent.
  db->resolve_top_method();

  if ( !(db->get_ushort("interface.type") & DIRECT_INTERFACE_BIT) )
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
    rv[0] =  my_rosen_ub;
    rv[1] =  my_rosen_ub;
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


