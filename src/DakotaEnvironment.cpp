/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       Environment
//- Description: Implementation code for the Environment class
//- Owner:       Mike Eldred
//- Checked by:

#include "DakotaEnvironment.hpp"
#include "ExecutableEnvironment.hpp"
#include "LibraryEnvironment.hpp"
#include "WorkdirHelper.hpp"
#include "ProblemDescDB.hpp"
#include "IteratorScheduler.hpp"
#include "dakota_preproc_util.hpp"

static const char rcsId[]="@(#) $Id: DakotaEnvironment.cpp 6749 2010-05-03 17:11:57Z briadam $";

using std::cout;

namespace Dakota {


/** This letter constructor initializes base class data for inherited
    environments that are default constructed.  Since the letter IS
    the representation, its representation pointer is set to NULL.

    Use cases:
    * library with no options, no MPI comm

*/
Environment::Environment(BaseConstructor):
  mpiManager(), programOptions(mpiManager.world_rank()), outputManager(),
  parallelLib(mpiManager, programOptions, outputManager),
  probDescDB(parallelLib), usageTracker(mpiManager.world_rank())
{
  // set exit mode as early as possible
  if (!programOptions.exit_mode().empty())
    exit_mode(programOptions.exit_mode());

  // Initialize paths used by WorkdirHelper
  WorkdirHelper::initialize();
}


/** This letter constructor initializes base class data for inherited
    environments: instantiate/initialize the environment, options,
    parallel library, and problem description database objects.  Since
    the letter IS the representation, its representation pointer is set
    to NULL.
    
    Use cases:
    * executable with command-line args

*/
Environment::Environment(BaseConstructor, int argc, char* argv[]):
  // MPIManager potentially parses off MPI options from argc/argv via MPI_Init
  mpiManager(argc, argv),    
  // extract program options from command line and environment
  programOptions(argc, argv, mpiManager.world_rank()),
  // assign streams right away based on programOtions if needed, possibly
  // updated at runtime to tagged files; streams are closed via destructor
  outputManager(programOptions, mpiManager.world_rank(),
		mpiManager.mpirun_flag()),
  // now instantiate the parallel library and problem description DB
  parallelLib(mpiManager, programOptions, outputManager),
  probDescDB(parallelLib), usageTracker(mpiManager.world_rank())
{
  // set exit mode as early as possible
  if (!programOptions.exit_mode().empty())
    exit_mode(programOptions.exit_mode());

  // Initialize paths used by WorkdirHelper and NIDR

  // these data were previously statically initialized, so perform first
  WorkdirHelper::initialize();
}


/** This letter constructor initializes base class data for inherited
    environments.  Since the letter IS the representation, its
    representation pointer is set to NULL.

    Use cases: 
     * library with program options
     * library with program options and MPI comm
*/
Environment::Environment(BaseConstructor, ProgramOptions prog_opts,
			 MPI_Comm dakota_mpi_comm):
  mpiManager(dakota_mpi_comm), programOptions(prog_opts),
  outputManager(programOptions, mpiManager.world_rank(),
		mpiManager.mpirun_flag()), 
  parallelLib(mpiManager, programOptions, outputManager),
  probDescDB(parallelLib), usageTracker(mpiManager.world_rank())
{
  // set exit mode as early as possible
  if (!programOptions.exit_mode().empty())
    exit_mode(programOptions.exit_mode());

  // Initialize paths used by WorkdirHelper
  WorkdirHelper::initialize();
}


/** Default envelope constructor.  environmentRep is NULL in this
    case. */
Environment::Environment()
{ /* empty ctor */ }


/** Envelope constructor for ExecutableEnvironment.  Selection of
    derived type by get_environment() is not necessary in this case. */
Environment::Environment(int argc, char* argv[]):
  // set the rep pointer to the appropriate environment type
  environmentRep(std::make_shared<ExecutableEnvironment>(argc, argv))
{
  if ( !environmentRep ) // insufficient memory
    abort_handler(-1);
}


/** Envelope constructor for LibraryEnvironment.  Selection of
    derived type by get_environment() is not necessary in this case. */
Environment::
Environment(ProgramOptions prog_opts): 
  // set the rep pointer to the appropriate environment type
  environmentRep(std::make_shared<LibraryEnvironment>(prog_opts))
{
  if ( !environmentRep ) // insufficient memory
    abort_handler(-1);
}


/** Envelope constructor for LibraryEnvironment.  Selection of
    derived type by get_environment() is not necessary in this case. */
Environment::
Environment(MPI_Comm dakota_mpi_comm, ProgramOptions prog_opts): 
  // set the rep pointer to the appropriate environment type
  environmentRep(std::make_shared<LibraryEnvironment>(dakota_mpi_comm, prog_opts))
{
  if ( !environmentRep ) // insufficient memory
    abort_handler(-1);
}


/** Alternate construction by String. Envelope constructor invokes
    get_environment() which instantiates a derived class letter; the
    derived constructor selects a BaseConstructor constructor in its
    initialization list to avoid the recursion of a base class
    constructor calling get_environment() again. */
Environment::Environment(const String& env_type): 
  // set the rep pointer to the appropriate environment type
  environmentRep(get_environment(env_type))
{
  if ( !environmentRep ) // bad name or insufficient memory
    abort_handler(-1);
}


/** Used only by the envelope constructor to initialize environmentRep to the 
    appropriate derived type, as given by the environmentName attribute. */
std::shared_ptr<Environment> Environment::get_environment(const String& env_type)
{
  if (env_type == "executable")
    return std::make_shared<ExecutableEnvironment>();
  else if (env_type == "library")
    return std::make_shared<LibraryEnvironment>();
  else
    Cerr << "Invalid environment type: " << env_type << std::endl;

  return std::shared_ptr<Environment>();
}


/** Copy constructor manages sharing of environmentRep. */
Environment::Environment(const Environment& env):
  environmentRep(env.environmentRep)
{ /* empty ctor */ }


Environment Environment::operator=(const Environment& env)
{
  environmentRep = env.environmentRep;
  return *this; // calls copy constructor since returned by value
}


Environment::~Environment()
{ 
  if (!environmentRep) // letter: base class destruction
    destruct();
}


/** Set the global variable controlling Dakota's exit behavior. Call
    with no arguments to reset to default behavior. */
void Environment::exit_mode(const String& mode)
{
  if (mode == "exit")
    abort_mode = ABORT_EXITS;
  else if (mode == "throw")
    abort_mode = ABORT_THROWS;
  else {
    Cerr << "\nError: unknown exit mode; options are \"exit\" and \"throw\""
	 << std::endl;
    abort_handler(-1);
  }
}


void Environment::preprocess_inputs() {

  // Only the leader parses inputs
  if (parallelLib.world_rank() != 0)
    return;

  if ( !programOptions.input_file().empty() &&
       !programOptions.input_string().empty() ) {
    Cerr << "\nError: preprocess_inputs called with both input file and input "
	 << "string." << std::endl;
    abort_handler(PARSE_ERROR);
  }

  // Read the input from stdin if the user provided "-" as the filename
  if(programOptions.input_file() == "-") {
    Cout << "Reading Dakota input from standard input" << std::endl;
    String stdin_string;
    char in = std::cin.get();
    while(std::cin.good()) {
      stdin_string.push_back(in);
      in = std::cin.get();
    }
    programOptions.input_file("");
    programOptions.input_string(stdin_string);
  }

  if (programOptions.preproc_input()) {
    std::string tmpl_file = programOptions.input_file();
    if (!programOptions.input_string().empty())
      // must generate to file on disk for pyprepro
      tmpl_file = string_to_tmpfile(programOptions.input_string());

    // run the pre-processor on the file
    std::string preproc_file = pyprepro_input(tmpl_file,
					      programOptions.preproc_cmd());
    programOptions.preprocessed_file(preproc_file);

    if (!programOptions.input_string().empty())
      boost::filesystem::remove(tmpl_file);

    // once pre-processed, check for any input file redirs
    outputManager.check_input_redirs(programOptions, preproc_file,
				     std::string());
  }
  else {
    outputManager.check_input_redirs(programOptions,
				     programOptions.input_file(),
				     programOptions.input_string());
  }
}



/** Parse input file and invoked any callbacks, then optionally check
    and sync database if check_bcast_database = true */
void Environment::parse(bool check_bcast_database,
			DbCallbackFunctionPtr callback, void* callback_data)
{
  // Called only by letter instances, no Rep forward required

  // Manage input file parsing, output redirection, and restart processing.
  // Since all processors need the database, manage_inputs() does not require
  // iterator partitions and it can precede init_iterator_communicators()
  // (a simple world bcast is sufficient).

  // Output/restart management utilizes iterator partitions, so calls to
  // push_output_tag() follow ParallelLibrary::init_iterator_communicators()
  // within IteratorScheduler::partition().

  // ProblemDescDB requires cmd line information, so pass programOptions

  // parse input and callback functions
  if ( !programOptions.input_file().empty() || 
       !programOptions.input_string().empty())
    probDescDB.parse_inputs(programOptions, callback, callback_data);

  // check if true, otherwise caller assumes responsibility  
  if (check_bcast_database) 
    probDescDB.check_and_broadcast(programOptions);

}


void Environment::construct()
{
  // Called only by letter instances, no Rep forward required

  // extract global output specification data (environment keyword
  // spec) right before run time in case of late library updates to DB
  programOptions.parse(probDescDB);
  // user might have requested output/error redirection in environment block;
  // check and update redirects
  outputManager.parse(programOptions, probDescDB);

  // With respect to Environment interaction with the probDescDB linked lists,
  // the current design allows the user to either fully specify the method to
  // be used (which may itself contain pointers to variables, interface, and
  // responses) or to rely on the default behavior in which the last data
  // populated in the calls to the keyword handlers is used to build the
  // model and the iterator.
  const String& method_ptr
    = probDescDB.get_string("environment.top_method_pointer");
  // The method pointer is optional and some detective work may be required to
  // resolve which method sits on top of a recursion.  Only set the method node:
  // leave model nodes locked for meta-iterators; standard iterators set model
  // nodes separately within else block below.
  if (method_ptr.empty()) probDescDB.resolve_top_method(false); // no model set
  else                    probDescDB.set_db_method_node(method_ptr);
  // augment setting of method node with model nodes according to top method
  // (std iterators need model; meta-iterators may need a default model spec)
  if (assign_model_pointer())
    probDescDB.set_db_model_nodes(
      probDescDB.get_string("method.model_pointer"));

  // w_pl is the same for all parallel configurations
  ParLevLIter w_pl_iter = parallelLib.w_parallel_level_iterator();
  // initialize/increment hierarchical output/restart streams
  // (w_pl does not induce a tag)
  parallelLib.push_output_tag(*w_pl_iter);// from init_serial_iterators

  // Instantiate topLevelIterator in parallel
  // (invoke ProblemDescDB ctor chain on all processors)
  IteratorScheduler::init_iterator(probDescDB, topLevelIterator, w_pl_iter);
  // Notfiy the iterator that it is the top level
  topLevelIterator.top_level(true);
}


void Environment::execute()
{
  if (environmentRep)
    environmentRep->execute();
  else {

    // must wait for iterators to be instantiated; positive side effect is that 
    // we don't track dakota -version, -help, and errant usage
    usageTracker.post_start(probDescDB);

    bool output_rank = (parallelLib.world_rank() == 0);
    if (output_rank)
      Cout << "\n>>>>> Executing environment.\n";

    probDescDB.lock(); // prevent run-time DB queries

    outputManager.init_results_db();
    if(output_rank)
      outputManager.archive_input(programOptions);

    // set up plotting and data tabulation
    // > MetaIterators delegate graphics initialization
    // > topLevelIterator's methodName must be defined on all ranks
    if ( (topLevelIterator.method_name() & PARALLEL_BIT) == 0 && output_rank )
      topLevelIterator.initialize_graphics(); // default to server_id = 1

    ParLevLIter w_pl_iter = parallelLib.w_parallel_level_iterator();
    IteratorScheduler::run_iterator(topLevelIterator, w_pl_iter);

    if (output_rank)
      Cout << "<<<<< Environment execution completed.\n";
  
    usageTracker.post_finish();

  }
}


bool Environment::check() const
{
  // don't proceed to execute for help and version 
  if (programOptions.help() || programOptions.version())
    return true;

  if (programOptions.check()) {
    if (parallelLib.world_rank() == 0)
      Cout << "\nInput check completed successfully (input parsed and objects "
	   << "instantiated).\n" << std::endl;
    return true;
  }

  return false;
}


void Environment::destruct()
{
  // Called only by letter instances, no Rep forward required

  if (topLevelIterator.is_null()) // help and version invocations
    return;

  ParLevLIter w_pl_iter = parallelLib.w_parallel_level_iterator();
  IteratorScheduler::free_iterator(topLevelIterator, w_pl_iter);

  // decrement hierarchical output/restart streams (w_pl does not induce a tag)
  parallelLib.pop_output_tag(*w_pl_iter);
}

} // namespace Dakota
