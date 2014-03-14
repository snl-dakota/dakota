/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
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
#include "ProblemDescDB.hpp"
#include "IteratorScheduler.hpp"


static const char rcsId[]="@(#) $Id: DakotaEnvironment.cpp 6749 2010-05-03 17:11:57Z briadam $";

namespace Dakota {


/** This letter constructor initializes base class data for inherited
    environments that are default constructed.  Since the letter IS
    the representation, its representation pointer is set to NULL (an
    uninitialized pointer causes problems in ~Environment). */
Environment::Environment(BaseConstructor):
  mpiManager(), programOptions(), outputManager(),
  parallelLib(mpiManager, programOptions, outputManager),
  probDescDB(parallelLib), environmentRep(NULL), referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  cout << "Environment::Environment(BaseConstructor) called to "
       << "build letter base class." << std::endl;
#endif
}


/** This letter constructor initializes base class data for inherited
    environments: instantiate/initialize the environment, options,
    parallel library, and problem description database objects.  Since
    the letter IS the representation, its representation pointer is set
    to NULL (an uninitialized pointer causes problems in ~Environment). */
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
  probDescDB(parallelLib), environmentRep(NULL), referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  cout << "Environment::Environment(BaseConstructor, int, char*) called "
       << "to build letter base class." << std::endl;
#endif
}


/** This letter constructor initializes base class data for inherited
    environments.  Since the letter IS the representation, its
    representation pointer is set to NULL (an uninitialized pointer
    causes problems in ~Environment). */
Environment::Environment(BaseConstructor, const ProgramOptions& prog_opts):
  mpiManager(), programOptions(prog_opts),
  outputManager(programOptions, mpiManager.world_rank(),
		mpiManager.mpirun_flag()), 
  parallelLib(mpiManager, programOptions, outputManager),
  probDescDB(parallelLib), environmentRep(NULL), referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  cout << "Environment::Environment(BaseConstructor, ProgramOptions&) called "
       << "to build letter base class." << std::endl;
#endif
}


/** Default envelope constructor.  environmentRep is NULL in this
    case, which makes it necessary to check for NULL in the copy
    constructor, assignment operator, and destructor. */
Environment::Environment(): environmentRep(NULL), referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  cout << "Environment::Environment() called to build empty envelope base "
       << "class object." << std::endl;
#endif
}


/** Envelope constructor for ExecutableEnvironment.  Selection of
    derived type by get_environment() is not necessary in this case. */
Environment::Environment(int argc, char* argv[]):
  referenceCount(1) // not used since this is the envelope
{
#ifdef REFCOUNT_DEBUG
  cout << "Environment::Environment(int, char*) called to instantiate "
       << "envelope for executable letter." << std::endl;
#endif

  // set the rep pointer to the appropriate environment type
  environmentRep = new ExecutableEnvironment(argc, argv);
  if ( !environmentRep ) // insufficient memory
    abort_handler(-1);
}


/** Envelope constructor for LibraryEnvironment.  Selection of
    derived type by get_environment() is not necessary in this case. */
Environment::
Environment(const ProgramOptions& prog_opts): 
  referenceCount(1) // not used since this is the envelope
{
#ifdef REFCOUNT_DEBUG
  cout << "Environment::Environment(ProgramOptions&) called to instantiate "
       << "envelope for library letter." << std::endl;
#endif

  // set the rep pointer to the appropriate environment type
  environmentRep = new LibraryEnvironment(prog_opts);
  if ( !environmentRep ) // insufficient memory
    abort_handler(-1);
}


/** Envelope constructor for LibraryEnvironment.  Selection of
    derived type by get_environment() is not necessary in this case. */
Environment::
Environment(MPI_Comm dakota_mpi_comm, const ProgramOptions& prog_opts): 
  referenceCount(1) // not used since this is the envelope
{
#ifdef REFCOUNT_DEBUG
  cout << "Environment::Environment(MPI_Comm, ProgramOptions&) called to "
       << "instantiate envelope for library letter." << std::endl;
#endif

  // set the rep pointer to the appropriate environment type
  environmentRep = new LibraryEnvironment(dakota_mpi_comm, prog_opts);
  if ( !environmentRep ) // insufficient memory
    abort_handler(-1);
}


/** Alternate construction by String. Envelope constructor invokes
    get_environment() which instantiates a derived class letter; the
    derived constructor selects a BaseConstructor constructor in its
    initialization list to avoid the recursion of a base class
    constructor calling get_environment() again. */
Environment::Environment(const String& env_type): 
  referenceCount(1) // not used since this is the envelope
{
#ifdef REFCOUNT_DEBUG
  cout << "Environment::Environment(String&) called to instantiate envelope."
       << std::endl;
#endif

  // set the rep pointer to the appropriate environment type
  environmentRep = get_environment(env_type);
  if ( !environmentRep ) // bad name or insufficient memory
    abort_handler(-1);
}


/** Used only by the envelope constructor to initialize environmentRep to the 
    appropriate derived type, as given by the environmentName attribute. */
Environment* Environment::get_environment(const String& env_type)
{
#ifdef REFCOUNT_DEBUG
  cout << "Envelope instantiating letter: Getting environment " << env_type
       << std::endl;
#endif

  if (env_type == "executable")
    return new ExecutableEnvironment();
  else if (env_type == "library")
    return new LibraryEnvironment();
  else {
    Cerr << "Invalid environment type: " << env_type << std::endl;
    return NULL;
  }
}


/** Copy constructor manages sharing of environmentRep and incrementing of
    referenceCount. */
Environment::Environment(const Environment& env)
{
  // Increment new (no old to decrement)
  environmentRep = env.environmentRep;
  if (environmentRep) // Check for an assignment of NULL
    environmentRep->referenceCount++;

#ifdef REFCOUNT_DEBUG
  cout << "Environment::Environment(Environment&)" << std::endl;
  if (environmentRep)
    cout << "environmentRep referenceCount = " << environmentRep->referenceCount
	 << std::endl;
#endif
}


/** Assignment operator decrements referenceCount for old environmentRep,
    assigns new environmentRep, and increments referenceCount for new
    environmentRep. */
Environment Environment::operator=(const Environment& env)
{
  if (environmentRep != env.environmentRep) { // normal case: old != new
    // Decrement old
    if (environmentRep) // Check for NULL
      if ( --environmentRep->referenceCount == 0 ) 
	delete environmentRep;
    // Assign new
    environmentRep = env.environmentRep;
  }
  // Increment new (either case: old == new or old != new)
  if (environmentRep) // Check for NULL
    environmentRep->referenceCount++;

#ifdef REFCOUNT_DEBUG
  cout << "Environment::operator=(Environment&)" << std::endl;
  if (environmentRep)
    cout << "environmentRep referenceCount = " << environmentRep->referenceCount
	 << std::endl;
#endif

  return *this; // calls copy constructor since returned by value
}


/** Destructor decrements referenceCount and only deletes environmentRep
    when referenceCount reaches zero. */
Environment::~Environment()
{ 
  // Check for NULL pointer 
  if (environmentRep) { // envelope: manage ref count & delete environmentRep
    --environmentRep->referenceCount;
#ifdef REFCOUNT_DEBUG
    cout << "environmentRep referenceCount decremented to " 
	 << environmentRep->referenceCount << std::endl;
#endif
    if (environmentRep->referenceCount == 0) {
#ifdef REFCOUNT_DEBUG
      cout << "deleting environmentRep" << std::endl;
#endif
      delete environmentRep;
    }
  }
  else // letter: base class destruction
    destruct();
}


void Environment::construct(bool run_parser)
{
  // Called only by letter instances, no Rep forward required

  // Manage input file parsing, output redirection, and restart processing.
  // Since all processors need the database, manage_inputs() does not require
  // iterator partitions and it can precede init_iterator_communicators()
  // (a simple world bcast is sufficient).  Output/restart management does
  // utilize iterator partitions, so manage_outputs_restart() must follow
  // init_iterator_communicators() within the Environment constructor
  // (output/restart options may only be specified at this time).

  // ProblemDescDB requires cmd line information, so pass programOptions
  probDescDB.manage_inputs(programOptions, run_parser);
  // extract global output specification data (environment keyword spec)
  outputManager.parse(probDescDB);

  // With respect to Environment interaction with the probDescDB linked lists,
  // the current design allows the user to either fully specify the method to
  // be used (which may itself contain pointers to variables, interface, and
  // responses) or to rely on the default behavior in which the last data
  // populated in the calls to the keyword handlers is used to build the
  // model and the iterator.
  const String& method_ptr
    = probDescDB.get_string("environment.top_method_pointer");
  // The method pointer is optional and some detective work may be
  // required to resolve which method sits on top of a recursion.
  if (method_ptr.empty()) probDescDB.resolve_top_method();
  else                    probDescDB.set_db_list_nodes(method_ptr);

  // Instantiate the topLevelIterator in parallel (invoke
  // ProblemDescDB ctor chain on all processors)
  if (probDescDB.get_ushort("method.algorithm") & META_BIT) {
    // meta-iterator constructors manage IteratorScheduler::init_iterator()
    // and IteratorScheduler::init_iterator_parallelism()
    topLevelIterator = probDescDB.get_iterator(); // all procs
  }
  else {
    IteratorScheduler::init_serial_iterators(parallelLib); // serialize si_pl
    //topLevelModel = probDescDB.get_model(); // if access needed downstream
    Model top_level_model = probDescDB.get_model();
    IteratorScheduler::init_iterator(probDescDB, topLevelIterator,
      top_level_model, // modelRep gets shared in topLevelIterator
      parallelLib.parallel_configuration().w_parallel_level());
  }
}


void Environment::execute()
{
  if (environmentRep)
    environmentRep->execute();
  else {
    bool output_rank = (parallelLib.world_rank() == 0);
    if (output_rank)
      Cout << "\n>>>>> Executing environment.\n";

    probDescDB.lock(); // prevent run-time DB queries

    if (topLevelIterator.method_name() & META_BIT)
      topLevelIterator.run(Cout);
    else
      IteratorScheduler::run_iterator(topLevelIterator, //topLevelModel,
        parallelLib.parallel_configuration().w_parallel_level());

    if (output_rank)
      Cout << "<<<<< Environment execution completed.\n";
  }
}


void Environment::destruct()
{
  // Called only by letter instances, no Rep forward required

  if (topLevelIterator.is_null()) // help and version invocations
    return;
  else if (topLevelIterator.method_name() & META_BIT) {
    // meta-iterator destructors manage IteratorScheduler::free_iterator()
    // and IteratorScheduler::free_iterator_parallelism()
  }
  else {
    // deallocate communicator partitions for topLevelIterator
    IteratorScheduler::free_iterator(topLevelIterator, //topLevelModel,
      parallelLib.parallel_configuration().w_parallel_level());
    // deallocate the (serialized) si_pl parallelism level
    IteratorScheduler::free_iterator_parallelism(parallelLib);
  }
}

} // namespace Dakota
