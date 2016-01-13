/** \file library_split.cpp
    \brief file containing a mock simulator main for testing DAKOTA in
    library mode on a split communicator */ 

#include <boost/lexical_cast.hpp>
#include "mpi.h"
#include "LibraryEnvironment.hpp"
#include "ParallelLibrary.hpp"
#include "ProblemDescDB.hpp" 
#include "DakotaModel.hpp"
#include "DakotaInterface.hpp"
#include "PluginParallelDirectApplicInterface.hpp"

// for Sleep or sleep
#ifdef _WIN32
#include "dakota_windows.h"
#else
#include <unistd.h>
#endif

/// Split MPI_COMM_WORLD, returning the comm and color
void manage_mpi(MPI_Comm& my_comm, int& color);

/// Return the appropriate DAKOTA input based on color (1 or 2)
void gen_dakota_input(const int& color, std::string& input);

/// Launch DAKOTA on passed communicator, tagging output/error with color
void run_dakota(const MPI_Comm& comm, const std::string& input, 
		const int& color);

/// Wait for and collect results from DAKOTA runs
void collect_results();


/// Driver routine for testing library mode with partitioned
/// MPI_Comm. This test fixture requires MPI and can be run on 3--8
/// processors
int main(int argc, char* argv[])
{
  
  MPI_Init(&argc, &argv);

  // manage MPI split
  MPI_Comm my_comm;
  int color;
  manage_mpi(my_comm, color);

  // cleanup output files; avoid race condition
  std::remove("dakota.o.1");
  std::remove("dakota.o.2");
  // remove("dakota.o");
  std::remove("dakota.e.1");
  std::remove("dakota.e.2");
  //remove("dakota.e");
#ifdef _WIN32
  Sleep(1000); // milliseconds
#else
  sleep(1);    // seconds
#endif

  // colors 1 and 2 run DAKOTA
  if (color != 0) {
    std::string input;
    gen_dakota_input(color, input);
    run_dakota(my_comm, input, color);
  }
  
  // ideally color 0 would do something concurrently...
  MPI_Barrier(MPI_COMM_WORLD);

  if (color == 0)
    collect_results();

  // ideally color 0 would do something concurrently...
  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Comm_free(&my_comm);
  MPI_Finalize();

  return 0;
}


void manage_mpi(MPI_Comm& my_comm, int& color) 
{
  int world_rank, world_size;

  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  if (world_size < 3) {
    std::cerr << "At least 3 processors required; " << world_size 
	      << " specified." << std::endl;
    MPI_Abort(MPI_COMM_WORLD, 1);
  } else if (world_size > 8) {
    std::cerr << "At most 8 processors allowed; " << world_size 
	      << " specified." << std::endl;
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  std::cout << "Rank " << world_rank << " of " << world_size << " starting."
	    << std::endl;

  // calculate size of DAKOTA job 1 and 2
  int s1 = static_cast<int>(std::ceil((world_size-1)/2.0));
  int s2 = static_cast<int>(std::floor((world_size-1)/2.0));

  if (world_rank == 0) {
    std::cout << "Number of processes: \n  collector: 1\n";
    std::cout << "  DAKOTA 1: " << s1 << "\n";
    std::cout << "  DAKOTA 2: " << s2 << std::endl;
  }

  if (world_rank == 0)
    color = 0;
  else if (world_rank <= s1)
    color = 1;
  else if (world_rank <= (s1 + s2))
    color = 2;
  else {
    std::cerr << "Error in color calculation; exiting." << std::endl;
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  MPI_Comm_split(MPI_COMM_WORLD, color, world_rank, &my_comm);

  int newrank;
  MPI_Comm_rank(my_comm, &newrank);
  
  std::cout << "Was rank " << world_rank << " in MPI_Comm = " << MPI_COMM_WORLD
	    << "; now rank " << newrank << " in MPI_Comm = " << my_comm 
	    << std::endl;
}


void gen_dakota_input(const int& color, std::string& input)
{

  // TODO: diagnose problems with ded. master/self scheduling which hangs
  //       mandating use of evaluation_scheduling peer static
  switch(color) {

  case 1:
    // CONMIN MFD on 3 variables
    input = 
      "	method,"
      "		conmin_frcg"
      "		  max_iterations = 50"
      "		  convergence_tolerance = 1e-4"
      "	variables,"
      "		continuous_design = 3"
      "		  descriptors 'x1' 'x2' 'x3'"
      "	interface,"
      "		direct"
      "		  analysis_driver = 'plugin_text_book'"
      "           evaluation_scheduling peer static"
      "	responses,"
      "		num_objective_functions = 1"
      "		numerical_gradients"
      "		no_hessians";
    break;

  case 2:
    // OPT++ QN on 2 variables
    input = 
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
      "           evaluation_scheduling peer static"
      "	responses,"
      "		num_objective_functions = 1"
      "		num_nonlinear_inequality_constraints = 2"
      "		numerical_gradients"
      "		no_hessians";
    break;

  default:
    std::cerr << "Unknown color " << color << "; exiting" << std::endl;
    MPI_Abort(MPI_COMM_WORLD, 1);
    break;

  }
}


void run_dakota(const MPI_Comm& my_comm, const std::string& input, 
		const int& color)
{
  std::cout << "*** Starting DAKOTA run " << color << std::endl;
  
  // BMA TODO: get right behavior across ranks here:

  // override output, error, and write restart files, but not read restart
  std::string ofile("dakota.o." + boost::lexical_cast<std::string>(color));
  std::string efile("dakota.e." + boost::lexical_cast<std::string>(color));
  std::string wfile("dakota.rst." + boost::lexical_cast<std::string>(color));

  Dakota::ProgramOptions prog_opts;
  prog_opts.input_string(input);
  prog_opts.output_file(ofile);
  prog_opts.error_file(efile);
  prog_opts.write_restart_file(wfile);

  // Create LibraryEnvironment
  Dakota::LibraryEnvironment env(my_comm, prog_opts);
  Dakota::ProblemDescDB& problem_db = env.problem_description_db();

  // Perform interface plug-ins.
  // retrieve the currently active analysisComm from the Model.  In the most
  // general case, need an array of Comms to cover all Model configurations.

  // Get only the models that match our plugin criteria
  Dakota::ModelList models = 
    env.filtered_model_list("simulation", "direct", "plugin_text_book");
  Dakota::ModelLIter ml_iter = models.begin(), ml_end = models.end();
  for ( ; ml_iter != ml_end; ++ml_iter) {
    Dakota::Interface& model_iface = ml_iter->derived_interface();
    const Dakota::ParallelLevel& ea_level
      = ml_iter->parallel_configuration_iterator()->ea_parallel_level();
    const MPI_Comm& analysis_comm = ea_level.server_intra_communicator();
    model_iface.assign_rep(new
      SIM::ParallelDirectApplicInterface(problem_db, analysis_comm), false);
  }

  // Execute the Environment
  env.execute();

  std::cout << "*** Finished DAKOTA run " << color << std::endl;
}


void collect_results()
{
  // avoid file race condition
#ifdef _WIN32
  Sleep(1000); // milliseconds
#else
  sleep(1);    // seconds
#endif
  // for dakota_test.perl benefit; no easy way to sequence output and error
  std::system("cat dakota.o.1");
  std::system("cat dakota.o.2");
  //system("cat dakota.o.1 > dakota.o");
  //system("cat dakota.o.2 >> dakota.o");
  //system("cat dakota.e.1 > dakota.e");
  //system("cat dakota.e.2 >> dakota.e");
}
