/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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

/// Run a Dakota LibraryEnvironment, mode 1: parsing an input file
void run_dakota_parse(const char* dakota_input_file);
void serial_interface_plugin(Dakota::LibraryEnvironment& env);
void parallel_interface_plugin(Dakota::LibraryEnvironment& env);



/// A mock simulator main for testing Dakota in library mode.

/** Load a complete input file, attach a client interface, and execute. */
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

  int status = 0;
  if (argc != 2 || !strcmp(argv[1], "-mixed")) {
    Cerr << "Usage: dakota_library_mode <dakota-input-file>" << std::endl;
    status = 1;
  }
  else
    run_dakota_parse(argv[1]);

  // Note: Dakota objects created in above function calls need to go
  // out of scope prior to MPI_Finalize so that MPI code in
  // destructors works properly in library mode.

#ifdef DAKOTA_HAVE_MPI
  if (parallel)
    MPI_Finalize(); // finalize MPI
#endif // DAKOTA_HAVE_MPI

  return status;
}


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
  Dakota::ParallelLibrary& parallel_lib = env.parallel_library();
  auto serial_iface = std::make_shared<SIM::SerialDirectApplicInterface>(
    problem_db, parallel_lib);

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
  Dakota::ParallelLibrary& parallel_lib = env.parallel_library();
  Dakota::ModelLIter ml_iter;
  size_t model_index = problem_db.get_db_model_node(); // for restoration
  for (auto& fm : filt_models) {
    // set DB nodes to input specification for this Model
    problem_db.set_db_model_nodes(fm->model_id());

    // Parallel case: plug in derived Interface object with an analysisComm.
    // Note: retrieval and passing of analysisComm is necessary only if
    // parallel operations will be performed in the derived constructor.

    // retrieve the currently active analysisComm from the Model.  In the most
    // general case, need an array of Comms to cover all Model configurations.
    const MPI_Comm& analysis_comm = fm->analysis_comm();

    // don't increment ref count since no other envelope shares this letter
    fm->derived_interface(std::make_shared<SIM::ParallelDirectApplicInterface>
			       (problem_db, parallel_lib, analysis_comm));
  }
  problem_db.set_db_model_nodes(model_index);            // restore
}


