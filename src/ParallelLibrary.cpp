/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ParallelLibrary
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "ParallelLibrary.hpp"
#include "DakotaBinStream.hpp"
#include "ProblemDescDB.hpp"
#include "CommandLineHandler.hpp"
#include "ParamResponsePair.hpp"
#include "DakotaGraphics.hpp"
#include "PRPMultiIndex.hpp"
#include "ResultsManager.hpp"
#ifdef DAKOTA_UTILIB
#include <utilib/exception_mngr.h>
#include "utilib/seconds.h"
#endif // DAKOTA_UTILIB
#ifdef DAKOTA_MODELCENTER
#include "PHXCppApi.h"
#endif // DAKOTA_MODELCENTER
#ifdef HAVE_AIX_MPI
#include <pm_util.h>  // for pm_child_sig_handler
#endif // HAVE_AIX_MPI

// on by default from consensus among Collis/Drake/Keiter/Salinger
#define COMM_SPLIT_TO_SINGLE

//#define MPI_DEBUG

static const char rcsId[]="@(#) $Id: ParallelLibrary.cpp 7013 2010-10-08 01:03:38Z wjbohnh $";


namespace Dakota {

// Note: MSVC requires these defined outside any function
extern PRPCache data_pairs;
extern BoStream write_restart;
extern Graphics dakota_graphics;
extern ResultsManager iterator_results_db;

/** Heartbeat function provided by not_executable.C; pass output
    interval in seconds, or -1 to use $DAKOTA_HEARTBEAT */
void start_dakota_heartbeat(int);

extern ParallelLibrary *Dak_pl;

/** This constructor is used for creation of the global dummy_lib
    object, which is used to satisfy initialization requirements when
    the real ParallelLibrary object is not available. */
ParallelLibrary::ParallelLibrary(const std::string& dummy): 
  dummyFlag(true), checkFlag(false),
  preRunFlag(true), runFlag(true), postRunFlag(true), userModesFlag(false),
  outputTimings(false)
{ }


/** This constructor is the one used by main.cpp.  It calls MPI_Init
    conditionally based on whether a parallel launch is detected. */
ParallelLibrary::ParallelLibrary(int& argc, char**& argv): 
  dakotaMPIComm(MPI_COMM_WORLD), worldRank(0), worldSize(1), mpirunFlag(false), 
  ownMPIFlag(false), dummyFlag(false), stdOutputToFile(false),
  stdErrorToFile(false), checkFlag(false), preRunFlag(true), 
  runFlag(true), postRunFlag(true), userModesFlag(false), outputTimings(false),
  startClock(0), stopRestartEvals(0),
  currPLIter(parallelLevels.end()), currPCIter(parallelConfigurations.end())
{
  // detect parallel launch of DAKOTA using mpirun/mpiexec/poe/etc.
  mpirunFlag = detect_parallel_launch(argc, argv);

  initialize_timers();

#ifdef DAKOTA_HAVE_MPI
  // Initialize MPI if and only if DAKOTA launched in parallel from
  // the command-line (mpirunFlag).  Here DAKOTA owns MPI.
  if (mpirunFlag) {
    MPI_Init(&argc, &argv); // See comment above regarding argv and argc
    ownMPIFlag = true; // own MPI_Init, so call MPI_Finalize in destructor 
  }
#endif

  init_mpi_comm();
}


/** This constructor provides a library mode default ParallelLibrary.
    It does not call MPI_Init, but rather gathers data from
    MPI_COMM_WORLD if MPI_Init has been called elsewhere. */
ParallelLibrary::ParallelLibrary(): dakotaMPIComm(MPI_COMM_WORLD), worldRank(0),
  worldSize(1), mpirunFlag(false), ownMPIFlag(false), dummyFlag(false),
  stdOutputToFile(false), stdErrorToFile(false), checkFlag(false),
  preRunFlag(true), runFlag(true), postRunFlag(true), userModesFlag(false),
  outputTimings(true), startClock(0), stopRestartEvals(0),
  currPLIter(parallelLevels.end()), currPCIter(parallelConfigurations.end())
{
  initialize_timers();

#ifdef DAKOTA_HAVE_MPI
  // Do not initialize MPI, but check if initialized.
  int initialized = 0;
  MPI_Initialized(&initialized);
  if (initialized)
    mpirunFlag = true;
#endif

  init_mpi_comm();
}


/** This constructor provides a library mode ParallelLibrary,
    accepting an MPI communicator that might not be MPI_COMM_WORLD.
    It does not call MPI_Init, but rather gathers data from
    dakota_mpi_comm if MPI_Init has been called elsewhere. */
ParallelLibrary::ParallelLibrary(MPI_Comm dakota_mpi_comm): 
  dakotaMPIComm(dakota_mpi_comm), worldRank(0), worldSize(1), 
  mpirunFlag(false), ownMPIFlag(false), dummyFlag(false),
  stdOutputToFile(false), stdErrorToFile(false), checkFlag(false),
  preRunFlag(true), runFlag(true), postRunFlag(true), userModesFlag(false), 
  outputTimings(true), startClock(0), stopRestartEvals(0),
  currPLIter(parallelLevels.end()), currPCIter(parallelConfigurations.end())
{
  initialize_timers();

#ifdef DAKOTA_HAVE_MPI
  // Do not initialize MPI, but check if initialized.
  int initialized = 0;
  MPI_Initialized(&initialized);
  if (initialized)
    mpirunFlag = true;
#endif

  init_mpi_comm();
}


/// shared function for initializing based on passed MPI_Comm
void ParallelLibrary::init_mpi_comm()
{
  // do not initialize MPI.  Get worldRank/worldSize if available
  ParallelLevel pl;
#ifdef DAKOTA_HAVE_MPI // mpi available
  if (mpirunFlag) {
    MPI_Comm_rank(dakotaMPIComm, &worldRank);
    MPI_Comm_size(dakotaMPIComm, &worldSize);

    pl.serverIntraComm = dakotaMPIComm;
    pl.serverCommRank  = worldRank;
    pl.serverCommSize  = worldSize;

    startMPITime = MPI_Wtime();
  }
  else
    pl.serverIntraComm = MPI_COMM_NULL;

  if (worldSize > 1) {
    startupMessage = "Running MPI executable in parallel on ";
    startupMessage += boost::lexical_cast<std::string>(worldSize) + 
      " processors.\n";
  }
  else
    startupMessage = "Running MPI executable in serial mode.\n";
#else // mpi not available
  if (mpirunFlag) {
    Cerr << "Error: Attempting to run serial executable in parallel."
	 << std::endl;
    abort_handler(-1);
  }
  else { // use defaults: worldRank = 0, worldSize = 1
    pl.serverIntraComm = MPI_COMM_NULL;
    startupMessage = "Running serial executable in serial mode.\n";
  }
#endif // DAKOTA_HAVE_MPI

  parallelLevels.push_back(pl);
  currPLIter = parallelLevels.begin();
  increment_parallel_configuration();

  Dak_pl = this;
  if (!mpirunFlag)
    start_dakota_heartbeat(-1); // -1 ==> take interval from $DAKOTA_HEARTBEAT
}


void ParallelLibrary::initialize_timers()
{
  // Initialize timers.  UTILIB start times are logged, rather than using
  // InitializeTiming() & Elapsed functions, since this avoids conflicts w/
  // other calls to these functions performed w/i SGOPT.  NOTES: 
  // (1) local timer uses clock(), which returns the _sum_ of the user & system
  //     times of the calling process _and_ its terminated child processes for
  //     which it has executed the wait() function, the pclose() function, or 
  //     the system() function.
  // (2) utilib uses getrusage on Sun for CPU time.  getrusage records time 
  //     used by the current process _or_ its terminated and waited-for child 
  //     processes (RUSAGE_SELF or RUSAGE_CHILDREN).
  // When no children (direct interface), local & UTILIB CPU times are the same.
  startClock   = clock();
#ifdef DAKOTA_UTILIB
  utilib::exception_mngr::set_mode(utilib::exception_mngr::Standard);
  startCPUTime = CPUSeconds();       // see utilib/src/sys/seconds.cpp
  startWCTime  = WallClockSeconds(); // see utilib/src/sys/seconds.cpp
#endif // DAKOTA_UTILIB
}


bool ParallelLibrary::detect_parallel_launch(int& argc, char**& argv)
{
  bool mpi_launch = false;

  // Command line arguments can be very problematic (more so with mpirun than 
  // yod).  The following procedures for maintaining serial and parallel 
  // capability in a single executable are being considered: (a) always 
  // invoking MPI_Init, even for an executable launched serially (no mpirun), 
  // (b) trigger conditional MPI initialization with a separate command line
  // argument (-mpi, this avoids GetLongOpt::parse problems), and (c) invoking
  // GetLongOpt::parse first so that problem_db settings can be used for 
  // conditional invocation of MPI_Init in init_parallel_lib.  The first case 
  // works with MPICH on Sun (although there have been problems with path 
  // designations to input files) and it will be fine for TFLOP by default 
  // since we will never invoke cougar dakota w/o yod; however, it does not 
  // appear to work for Digital MPI on the Dec.  The second case will be
  // retained as a possible backup and its design merits should be considered 
  // vs. those of the parallel_library IDR input specification.  The last 
  // case fails since MPI args cause GetLongOpt::parse errors for unenrolled 
  // options, the subsequent condition of argc and argv passed to MPI_Init is 
  // currently unknown, and the standard MPI implementation behavior is being 
  // violated - see p.26 in "Using MPI" and the function Get_command_line at 
  // www.mcs.anl.gov/projects/mpi/tutorial/mpiexmpl/src3/jacobi/C/solution.html

  // Detecting a "-mpi" command line argument here (independent of
  // CommandLineHandler - which must only allow it as an enrolled option) is a 
  // safer mechanism for differentiating behavior than using GetLongOpt::parse.
  //bool mpi_flag = false;
  //Cout << argc << std::endl;
  //for (int i=0; i<argc; ++i) {
  //  Cout << argv[i] << std::endl;
  //  if (strcmp(argv[i],"-mpi") == 0) {
  //    Cout << "Found -mpi" << std::endl;
  //    mpi_flag = true;
  //  }
  //}

  // Another alternative: develop a run script which either 
  // (1) calls mpirun -np 1 in the serial case (one MPI executable), or 
  // (2) calls separate serial and parallel executables as appropriate.

  // Current approach: detect MPI launch based on argv or env content.
  // Unfortunately, this is not as simple as monitoring argv[0] since the 
  // launch utility (which would contain mpirun/dmpirun/yod in argv[0]) creates
  // the actual dakota processes as child processes (with different argv's).  
  // MPICH does add "-p4pg" (master) and "-p4amslave" (slaves) command line 
  // arguments which can be used for detection; however other platforms with 
  // proprietary MPI implementations (e.g., DEC, SGI) pass no telltale argv 
  // content.  On DEC/SGI, it's necessary to query environment variables, 
  // e.g. DIGITALMPI_ENV_MAP, MPI_ENVIRONMENT.  These variables are set for all
  // processors on DEC/SGI, but again it is not consistent across platforms 
  // (MPICH sets MPIRUN_DEVICE=ch_p4, but only on the master processor).

#ifdef OPEN_MPI
  // run-time test for OpenMPI v1.2 or greater
  char* ompi_1_2_test = std::getenv("OMPI_MCA_universe");
  // run-time test for OpenMPI v1.3 or greater
  char* ompi_1_3_test = std::getenv("OMPI_COMM_WORLD_SIZE");
  if (ompi_1_2_test || ompi_1_3_test) {
#ifdef MPI_DEBUG
    Cout << "Parallel run detected via OpenMPI test" << std::endl;
#endif
    mpi_launch = true;
  }

#elif defined(HAVE_MPICH)
  // alternately could test for preprocessor defines from the compiler
  // wrappers
  // MPICH_NAME = 1
  // MPICH_NAME = 2 || MPICH2
  //
  // test for p4 device
  //char* test = std::getenv("MPIRUN_DEVICE"); // no good: only set on master
  for (int i=0; i<argc; ++i) {
    std::string test(argv[i]);
    if (test=="-p4pg" || test=="-p4amslave") {
#ifdef MPI_DEBUG
      Cout << "Parallel run detected via MPICH args test" << std::endl;
#endif
      mpi_launch = true;
      break;
    }
  }
  // test for gm device (no command line content, so use GMPI_NP)
  char* gm_test = std::getenv("GMPI_NP"); // returns NULL if not set
  // alternate test for mpirun needed on some platforms with myrinet
  char* mpirun_test = std::getenv("MPIRUN_NPROCS"); // returns NULL if not set
  // alternate test for MPICH2
  if (!mpirun_test)
	mpirun_test = std::getenv("MPICH_INTERFACE_HOSTNAME");
  // alternate test for MPICH shmem comm
  // options for testing: MPICH_NP, MPIRUN_DEVICE=ch_shmem (master only?)
  if (!mpirun_test)
	mpirun_test = std::getenv("MPICH_NP");
  if (gm_test || mpirun_test) { // && atoi(gm_test) > 1)
#ifdef MPI_DEBUG
    Cout << "Parallel run detected via MPICH env test" << std::endl;
#endif
    mpi_launch = true;
  }

#elif defined(HAVE_OSF_MPI) // Digital MPI sets env vars. on all procs.
  char* dec_test = std::getenv("DIGITALMPI_ENV_MAGIC");//returns NULL if not set
  if (dec_test) // non-NULL
    mpi_launch = true;

#elif defined(HAVE_SGI_MPI) // SGI MPI sets env vars. on all procs.
  char* sgi_test = std::getenv("MPI_ENVIRONMENT"); // returns NULL if not set
  if (sgi_test) // non-NULL
    mpi_launch = true;

#elif defined(HAVE_SOLARIS64_MPI) // Sun MPI sets env vars. on all procs.
  char* sun_test = std::getenv("SUNHPC_PROC_RANK"); // returns NULL if not set
  if (sun_test) // non-NULL
    mpi_launch = true;

#elif defined(HAVE_AIX_MPI)
  // check to see if MP_PROCS is set, if so check if greater than 1.
  char* aix_test = std::getenv("MP_PROCS"); // returns NULL if not set
  if (aix_test) // && atoi(aix_test) > 1)
    mpi_launch = true;

#else
  // TFLOPS_COMPUTE & CPLANT_COMPUTE (+ other platforms w/o a special detection
  // routine) have defaults based only on MPI configuration.  Note that
  // TFLOPS_SERVICE is hardwired above and CPLANT_SERVICE is covered by -DMPICH.
#ifdef DAKOTA_HAVE_MPI
  mpi_launch = true;
#endif // DAKOTA_HAVE_MPI

#endif // OPEN_MPI

  return mpi_launch;
}


/** Split parent communicator into concurrent child server partitions
    as specified by the passed parameters.  This constructs new child
    intra-communicators and parent-child inter-communicators.  This
    function is called from the Strategy constructor for the concurrent
    iterator level and from ApplicationInterface::init_communicators()
    for the concurrent evaluation and concurrent analysis levels. */
void ParallelLibrary::
init_communicators(const ParallelLevel& parent_pl, const int& num_servers,
		   const int& procs_per_server, const int& max_concurrency, 
		   const int& asynch_local_concurrency,
		   const std::string& default_config,
		   const std::string& scheduling_override)
{
  ParallelLevel child_pl;
  child_pl.numServers     = num_servers;      // request/default to be updated
  child_pl.procsPerServer = procs_per_server; // request/default to be updated

  int capacity_multiplier = std::max(asynch_local_concurrency, 1);
  bool print_rank         = (parent_pl.serverCommRank == 0);

  // resolve_inputs can adaptively determine static vs. dynamic schedule,
  // but it can't distinguish between self- and distributed scheduling (e.g., 
  // PICO) without additional input.  May need to pass additional setting into 
  // resolve_inputs to communicate the type of scheduling policy a particular 
  // level supports, e.g.: "self"/"static"/"distributed" would be manual
  // selection and "adaptive" would allow auto-selection.  For now, distributed
  // scheduling is supported by using settings which auto-config to a static 
  // schedule & then using the resulting peer partition for distr. scheduling.
  child_pl.dedicatedMasterFlag = resolve_inputs(child_pl.numServers,
    child_pl.procsPerServer, parent_pl.serverCommSize, child_pl.procRemainder,
    max_concurrency, capacity_multiplier, default_config, scheduling_override,
    print_rank);

  child_pl.commSplitFlag = (child_pl.dedicatedMasterFlag) ?
    split_communicator_dedicated_master(parent_pl, child_pl) :
    split_communicator_peer_partition(parent_pl,   child_pl);

  // update number of parallelism levels
#ifdef COMM_SPLIT_TO_SINGLE
  if ( child_pl.commSplitFlag &&
       ( child_pl.procsPerServer > 1 || child_pl.procRemainder ) )
#else
  if (child_pl.commSplitFlag)
#endif
    ++currPCIter->numParallelLevels;

  parallelLevels.push_back(child_pl);
  currPLIter = --parallelLevels.end();
}


// ----------------------------------------------
// Hierarchy of partitioning logic sophistication
// ----------------------------------------------
// low performance: 
//   always use a master (p_min equation from the MDO2000 paper theory)
// 
// medium performance: 
//   only use a master if tau_i > 1 
//   (may be a first step towards distributed scheduling)
// 
// high performance: 
//   only use a master if tau_i > 1 and some n_ij > tau_i
//   (adaptive determination of a static schedule)

/** This function is responsible for the "auto-configure" intelligence of
    DAKOTA.  It resolves a variety of inputs and overrides into a sensible
    partitioning configuration for a particular parallelism level.  It also
    handles the general case in which a user's specification request does
    not divide out evenly with the number of available processors for the
    level.  If num_servers & procs_per_server are both nondefault, then the
    former takes precedence. */
bool ParallelLibrary::
resolve_inputs(int& num_servers, int& procs_per_server, const int& avail_procs, 
               int& proc_remainder, const int& max_concurrency, 
               const int& capacity_multiplier,
	       const std::string& default_config,
               const std::string& scheduling_override, bool print_rank)
{
/*
#ifdef MPI_DEBUG
  if (print_rank)
    Cout << "ParallelLibrary::resolve_inputs() called with num_servers = "
	 << num_servers << " procs_per_server = " << procs_per_server
	 << " avail_procs = " << avail_procs << " max_concurrency = "
	 << max_concurrency << " capacity_multiplier = " << capacity_multiplier
	 << " default_config = " << default_config << " scheduling_override = "
	 << scheduling_override << std::endl;
#endif
*/

  const bool master_override = (scheduling_override == "master");
  const bool peer_override   = strbegins(scheduling_override, "peer");

  bool ded_master;
  if (avail_procs <= 1) {
    // ------------------------
    // insufficient avail_procs
    // ------------------------
    if (procs_per_server > 1 && print_rank)
      Cerr << "Warning: not enough available processors to support "
           << procs_per_server << " procs_per_server.\n         "
	   << "Reducing to 1.\n";
    if (num_servers > 1 && print_rank)
      Cerr << "Warning: not enough available processors to support " 
           << num_servers << " servers.\n         Reducing to 1.\n";
    procs_per_server = 1;
    num_servers = 1;
    ded_master = false; // static schedule
  }
  else if (num_servers > 0) { // needs to be 0 so that a user request of 1 
                              // executes this block as a manual override
    // -------------------------------
    // num_servers (or both) specified
    // -------------------------------

    // First reduce num_servers request if necessary
    if (master_override ||
	(max_concurrency > avail_procs*capacity_multiplier && !peer_override)) {
      // ded. master: cap num_servers at number of slave procs.
      if (num_servers > avail_procs-1) {
        num_servers = avail_procs-1;
        if (print_rank)
          Cerr << "Warning: not enough processors for num_servers request in "
	       << "dedicated master\n         partition.  Reducing num_servers"
	       << " to " << num_servers << '\n';
      }
      if (procs_per_server > 0 && 
          procs_per_server != (avail_procs-1)/num_servers && print_rank)
        Cerr << "Warning: num_servers and procs_per_server specifications are "
    	     << "not equivalent.\n         num_servers takes precedence.\n";
    }
    else {
      // peer partition: cap num_servers at number of avail_procs
      if (num_servers > avail_procs) {
        num_servers = avail_procs;
        if (print_rank)
          Cerr << "Warning: not enough processors for num_servers request in "
	       << "peer partition.\n         Reducing num_servers to " 
               << num_servers << '\n';
      }
      if (procs_per_server > 0 && 
          procs_per_server != avail_procs/num_servers && print_rank)
        Cerr << "Warning: num_servers and procs_per_server specifications are "
    	     << "not equivalent.\n         num_servers takes precedence.\n";
    }
    int max_servers
      = (int)std::ceil((Real)max_concurrency/(Real)capacity_multiplier);
    if (num_servers > max_servers) { // num_servers request exceeds need
      num_servers = max_servers;
      if (print_rank)
        Cerr << "Warning: num_servers capacity exceeds maximum concurrency.\n"
	     << "         reducing num_servers to " << num_servers << '\n';
    }

    // Now that num_servers is reasonable, use it to branch on either static or
    // dynamic scheduling
    if (master_override || (!peer_override && num_servers > 1 &&
	max_concurrency > num_servers*capacity_multiplier)) {
      // dynamic sched. -> self or distributed (self only for now)
      procs_per_server = (avail_procs-1) / num_servers;
      proc_remainder   = (avail_procs-1) % num_servers;
      ded_master = true;
    }
    else { // static sched.
      // num_servers*capacity_multiplier must equal max_concurrency
      procs_per_server = avail_procs / num_servers;
      proc_remainder   = avail_procs % num_servers;
      ded_master = false;
    }
  }
  else if (procs_per_server > 0) { // needs to be 0 so that a user request of 1
                                   // executes this block as a manual override
    // -------------------------------
    // only procs_per_server specified
    // -------------------------------

    // rounding num_servers down means that procs_per_server will be >= request
    // (decreasing below request should be avoided).  Estimating num_servers 
    // using avail_procs (peer partition) provides an upper bound (i.e., 
    // num_servers can only decrease for a dedicated master partition).
    num_servers = avail_procs/procs_per_server; // trial config (not final)
    int max_servers
      = (int)std::ceil((Real)max_concurrency/(Real)capacity_multiplier);
    if (!master_override && (num_servers <= 1 || num_servers >= max_servers ||
			     peer_override)) {
      // static sched.
      if (num_servers < 1) {
	if (print_rank)
	  Cerr << "Warning: not enough processors for procs_per_server request."
	       << "\n         reducing procs_per_server to " << avail_procs 
	       << " and using 1 server.\n";
        num_servers = 1;
      }
      else if (num_servers > max_servers) { // truncate & recalculate
	if (print_rank)
	  Cerr << "Warning: server availability (" << num_servers
	       << ") exceeds maximum server utilization (" << max_servers
	       << ").\n         reducing servers to " << max_servers << ".\n";
        num_servers = max_servers;
      }
      procs_per_server = avail_procs / num_servers;
      proc_remainder   = avail_procs % num_servers;
      ded_master = false;
    }
    else { // dynamic scheduling -> self or distr. (self only for now)
      num_servers = (avail_procs-1)/procs_per_server; // update config
      // no need to check if this num_servers >= max_servers since max_servers
      // would still be greater (num_servers can only decrease).  However, 
      // num_servers can become < 2 (e.g., 8/4 > 1 -> dynamic -> 7/4 = 1 server
      // of 7 procs. -> dedicated master not needed).  In this case, revert 
      // back to a static schedule using 1 server with all procs (unless "self"
      // specifically requested in scheduling_override).  This will mean
      // considerable departure from the procs_per_server request.  Running a
      // static schedule on 2 servers is a good option (achieved with a user
      // request for "static" overriding the capacity logic), and distributed
      // scheduling could be the best option when available.
      if (num_servers < 2 && !master_override) {
        // reducing procs by 1 dropped num_servers back to 1
        procs_per_server = avail_procs;
        num_servers = 1;
        ded_master = false;
      }
      else {
        proc_remainder = (avail_procs-1) % procs_per_server;
        ded_master = true;
      }
    }

    /* To simplify logic & to prevent going below p'_min, reducing 
       procs_per_server below the request in code below will no longer be 
       supported.

    // Since integer division will always be on the conservative side,
    // compare nominal w/ increasing num_servers by 1 to see which
    // resulting procs_per_server provides the best match w/ the request
    num_servers = avail_procs/procs_per_server;
    float ssplus1_ppa = (float)avail_procs/(float)(num_servers+1);
    float nom_ppa = (float)avail_procs/(float)num_servers; // note that
    // this will often increase procs_per_server beyond the request since 
    // more of remainder will be captured (assuming procsPer > numServers).
    //if (print_rank)
    //  Cout << "nom_ppa = " << nom_ppa << " ssplus1_ppa = " << ssplus1_ppa
    //       << "\nnom_ppa delta = " << nom_ppa - procs_per_server
    //       <<" ssplus1_ppa delta = "<<procs_per_server - ssplus1_ppa
    //       << std::endl;
    if (procs_per_server - ssplus1_ppa < nom_ppa - procs_per_server) {
      if (print_rank)
        Cerr << "Warning: reducing procs_per_server for best fit.\n";
      procs_per_server = (int)ssplus1_ppa;
      num_servers += 1;
    }
    else // nominal
      procs_per_server = (int)nom_ppa;
    proc_remainder = avail_procs % num_servers;
    */
  }
  else {
    // --------------------------------------------------
    // neither num_servers nor procs_per_server specified -> auto config
    // --------------------------------------------------
    if (default_config == "push_down") { // default for this parallelism level
      // is concurrency pushed down.  default_config taking precedence over a
      // scheduling_override request makes sense from an efficiency standpoint,
      // but may be neglecting a user request in lieu of a built-in default.
      // Output a warning if neglecting a user request:
      if (master_override && print_rank)
        Cerr << "Warning: default_config takes precendence over a "
	     << "self_scheduling request\n         when neither num_servers "
	     << "nor procs_per_server is specified.\n";
      procs_per_server = avail_procs;
      num_servers = 1;
      ded_master = false; // tau_i = 1 -> static scheduling
    }
    else { // concurrency pushed up
      if ( master_override || ( !peer_override && avail_procs > 2 &&
           max_concurrency > avail_procs*capacity_multiplier ) ) {
        // dynamic sched.: self or distr. (self only for now)
        num_servers = avail_procs-1;
        int max_servers
          = (int)std::ceil((Real)max_concurrency/(Real)capacity_multiplier);
        if (num_servers > max_servers)
          num_servers = max_servers;
        procs_per_server = (avail_procs-1) / num_servers;
        proc_remainder   = (avail_procs-1) % num_servers;    
        ded_master = true;
      }
      else { // static scheduling
        if (max_concurrency <= 1) { // default max_concurrency can be 0 in 
                                    // init_iterator_communicators
          procs_per_server = avail_procs;
          num_servers = 1;
	}
	else { 
          num_servers = avail_procs;
          int max_servers
            = (int)std::ceil((Real)max_concurrency/(Real)capacity_multiplier);
          if (num_servers > max_servers)
            num_servers = max_servers;
          procs_per_server = avail_procs / num_servers;
	  proc_remainder   = avail_procs % num_servers;
	}
        ded_master = false; // tau_i = n_ij_max -> static scheduling
      }
    }
  }

/*
#ifdef MPI_DEBUG
  if (print_rank)
    Cout << "ParallelLibrary::resolve_inputs() returns num_servers = "
	 << num_servers << " procs_per_server = " << procs_per_server
	 << " proc_remainder = " << proc_remainder << " dedicated master = "
	 << ded_master << std::endl;
#endif
*/

  return ded_master;
}


bool ParallelLibrary::
split_communicator_dedicated_master(const ParallelLevel& parent_pl,
				    ParallelLevel& child_pl)
{
  // ----------------------------------------------------------------
  // Check to see if resulting partition sizes require comm splitting
  // ----------------------------------------------------------------

  if (child_pl.numServers < 1) { // no partition: child == parent
    child_pl.serverMasterFlag = (parent_pl.serverCommRank == 0) ? true : false;
    child_pl.serverId = 0;
    child_pl.serverIntraComm = parent_pl.serverIntraComm; // or MPI_Comm_dup()
    child_pl.serverCommRank  = parent_pl.serverCommRank;
    child_pl.serverCommSize  = parent_pl.serverCommSize;
    child_pl.hubServerIntraComm = MPI_COMM_NULL; // or a Comm of only 1 proc.
    // use ctor defaults for child_pl.hubServerCommRank/hubServerCommSize
    return false; // set split flag to false in calling routine
  }
  else
    child_pl.messagePass = true;

#ifndef COMM_SPLIT_TO_SINGLE
  // In some direct interfacing cases, the simulation requires its own comm
  // even if the comm is single processor.  In this case, the code block below
  // is bypassed and the additional comm split overhead is incurred.
  if (child_pl.procsPerServer == 1 && !child_pl.procRemainder) {//1-proc servers
    child_pl.serverMasterFlag = (parent_pl.serverCommRank) ? true : false;
    child_pl.serverId = parent_pl.serverCommRank;// 0 = master, 1/2/... = slaves
    child_pl.serverIntraComm = MPI_COMM_NULL; // prevent further subdivision
    // use ctor defaults for child_pl.serverCommRank/serverCommSize
    child_pl.hubServerIntraComm = parent_pl.serverIntraComm;// or MPI_Comm_dup()
    child_pl.hubServerCommRank  = parent_pl.serverCommRank;
    child_pl.hubServerCommSize  = parent_pl.serverCommSize;
    return false; // set split flag to false in calling routine
  }
#endif

  // ------------------------------------------------------
  // Split parent Comm to create new intra- and inter-comms
  // ------------------------------------------------------

  IntArray start(child_pl.numServers);
  int color = 0; // reassigned unless master proc.
  // addtl_procs manages case where procRemainder > numServers that can occur
  // for large procsPerServer --> ensures that proc_rem_cntr < numServers
  int i, color_cntr = 1, end = 0,
    addtl_procs   = child_pl.procRemainder / child_pl.numServers, // truncated
    proc_rem_cntr = child_pl.procRemainder - addtl_procs*child_pl.numServers;
  for (i=0; i<child_pl.numServers; ++i) {
    start[i] = end + 1;
    end = start[i] + child_pl.procsPerServer + addtl_procs - 1;
    if (proc_rem_cntr > 0)
      { ++end; --proc_rem_cntr; }
    if (parent_pl.serverCommRank >= start[i] &&
	parent_pl.serverCommRank <= end) {
      color = color_cntr;
      //Cout << "Slave processor " << parent_pl.serverCommRank
      //     << " assigned color = " << color << std::endl;
    }
#ifdef MPI_DEBUG
    if (parent_pl.serverCommRank == 0)
      Cout << "group " << i << " has processors " << start[i] 
           << " through " << end << " with color = " << color_cntr << '\n';
#endif // MPI_DEBUG
    ++color_cntr;
  }
  if (parent_pl.serverCommRank && !color) {
    Cerr << "Error: slave processor " << parent_pl.serverCommRank 
         << " missing color assignment" << std::endl;
    abort_handler(-1);
  }

#ifdef DAKOTA_HAVE_MPI
  MPI_Comm_split(parent_pl.serverIntraComm, color, parent_pl.serverCommRank,
		 &child_pl.serverIntraComm);
  MPI_Comm_rank(child_pl.serverIntraComm, &child_pl.serverCommRank);
  MPI_Comm_size(child_pl.serverIntraComm, &child_pl.serverCommSize);
  child_pl.serverId = color; // 0 for master, 1/2/3/.../n for slaves

  // Create intercommunicators.  All processors in both intracommunicators 
  // (child_pl.serverIntraComm for master and slaves) must participate in call
  // with matching tags (tag = color on slave side and = i+1 on master side).
  // See example on p. 252 of MPI: The Complete Reference.
  if (parent_pl.serverCommRank == 0) {
    child_pl.hubServerInterComms = new MPI_Comm [child_pl.numServers];
    for(i=0; i<child_pl.numServers; ++i)
      MPI_Intercomm_create(child_pl.serverIntraComm, 0,
			   parent_pl.serverIntraComm, start[i], i+1,
			   &child_pl.hubServerInterComms[i]);
  }
  else
    MPI_Intercomm_create(child_pl.serverIntraComm, 0, parent_pl.serverIntraComm,
			 0, color, &child_pl.hubServerInterComm);
#ifdef MPI_DEBUG
  Cout << "worldRank = " << worldRank << " child comm rank = " 
       << child_pl.serverCommRank << " child comm size = "
       << child_pl.serverCommSize << '\n';
  int size, remote_size;
  if (parent_pl.serverCommRank==0) { // the dedicated master
    for (int i=0; i<child_pl.numServers; ++i) {
      MPI_Comm_size(child_pl.hubServerInterComms[i], &size);
      MPI_Comm_remote_size(child_pl.hubServerInterComms[i], &remote_size);
      Cout << "Master: size = " << size << " inter_comms[" << i
           << "] remote_size = " << remote_size << '\n';
    }
  }
  else { // slaves
    MPI_Comm_size(child_pl.hubServerInterComm, &size);
    MPI_Comm_remote_size(child_pl.hubServerInterComm, &remote_size);
    Cout << "Slave: size = " << size <<" inter_comm remote_size = " 
         << remote_size << '\n';
  }
  Cout << std::flush;
#endif // MPI_DEBUG

  // Create parent-child intracommunicator used to avoid broadcasting over all
  // of parent Comm when all you really need is the child rank == 0 processors.
  color = (child_pl.serverCommRank == 0) ? 1 : 0;
  MPI_Comm_split(parent_pl.serverIntraComm, color, parent_pl.serverCommRank,
		 &child_pl.hubServerIntraComm);
  if (child_pl.serverCommRank == 0) {
    MPI_Comm_rank(child_pl.hubServerIntraComm, &child_pl.hubServerCommRank);
    MPI_Comm_size(child_pl.hubServerIntraComm, &child_pl.hubServerCommSize);
  }
#endif // DAKOTA_HAVE_MPI

  if (child_pl.serverCommRank == 0 && parent_pl.serverCommRank)
    child_pl.serverMasterFlag = true; // this proc is a child partition master
  return true; // Set split flag to true in calling routine
}


bool ParallelLibrary::
split_communicator_peer_partition(const ParallelLevel& parent_pl, 
				  ParallelLevel& child_pl)
{
  // ----------------------------------------------------------------
  // Check to see if resulting partition sizes require comm splitting
  // ----------------------------------------------------------------

  // Note: requirements for splitting are different in this case.  Since a
  // processor is not dedicated for scheduling, one server means that
  // child_pl.serverIntraComm == parent_pl.serverIntraComm, which is assigned
  // as the default in the init_*_comms functions.
  if (child_pl.numServers < 2) { // one peer partition: child == parent
    child_pl.serverMasterFlag = (parent_pl.serverCommRank == 0) ? true : false;
    child_pl.serverId = 1; // One server, peer id = 1
    child_pl.serverIntraComm = parent_pl.serverIntraComm; // or MPI_Comm_dup()
    child_pl.serverCommRank  = parent_pl.serverCommRank;
    child_pl.serverCommSize  = parent_pl.serverCommSize;
    child_pl.hubServerIntraComm = MPI_COMM_NULL; // or a Comm of only 1 proc.
    // use ctor defaults for child_pl.hubServerCommRank/hubServerCommSize
    return false; // Set split flag to false in calling routine
  }
  else
    child_pl.messagePass = true;

#ifndef COMM_SPLIT_TO_SINGLE
  // In some direct interfacing cases, the simulation requires its own comm
  // even if the comm is single processor.  In this case, the code block below
  // is bypassed and the additional comm split overhead is incurred.
  if (child_pl.procsPerServer == 1 && !child_pl.procRemainder) {// 1-proc. peers
    child_pl.serverMasterFlag = true; // each child is single proc. & a master
    child_pl.serverId = parent_pl.serverCommRank+1; // peer id's = 1/2/3/.../n
    child_pl.serverIntraComm = MPI_COMM_NULL; // prevent further subdivision
    // use ctor defaults for child_pl.serverCommRank/serverCommSize
    child_pl.hubServerIntraComm = parent_pl.serverIntraComm;// or MPI_Comm_dup()
    child_pl.hubServerCommRank  = parent_pl.serverCommRank;
    child_pl.hubServerCommSize  = parent_pl.serverCommSize;
    return false; // Set split flag to false in calling routine
  }
#endif

  // -----------------------------------------------------------
  // Split parent Comm to create new peer intra- and inter-comms
  // -----------------------------------------------------------

  IntArray start(child_pl.numServers);
  int color = 0; // reassigned for all procs. in peer case
  // addtl_procs manages case where procRemainder > numServers that can occur
  // for large procsPerServer --> ensures that proc_rem_cntr < numServers
  int i, color_cntr = 1, end = -1,
    addtl_procs   = child_pl.procRemainder / child_pl.numServers, // truncated
    proc_rem_cntr = child_pl.procRemainder - addtl_procs*child_pl.numServers;
  for (i=0; i<child_pl.numServers; ++i) {
    start[i] = end + 1;
    end = start[i] + child_pl.procsPerServer + addtl_procs - 1;
    if (proc_rem_cntr > 0)
      { ++end; --proc_rem_cntr; }
    if (parent_pl.serverCommRank >= start[i] && parent_pl.serverCommRank <= end)
      color = color_cntr;
#ifdef MPI_DEBUG
    if (parent_pl.serverCommRank == 0)
      Cout << "group " << i << " has processors " << start[i] 
           << " through " << end << " with color = " << color_cntr << '\n';
#endif // MPI_DEBUG
    ++color_cntr;
  }
  if (!color) {
    Cerr << "Error: processor " << parent_pl.serverCommRank
	 << " missing color assignment" << std::endl;
    abort_handler(-1);
  }

#ifdef DAKOTA_HAVE_MPI
  MPI_Comm_split(parent_pl.serverIntraComm, color, parent_pl.serverCommRank,
		 &child_pl.serverIntraComm);
  MPI_Comm_rank(child_pl.serverIntraComm, &child_pl.serverCommRank);
  MPI_Comm_size(child_pl.serverIntraComm, &child_pl.serverCommSize);
  child_pl.serverId = color; // peer id's = 1/2/3/.../n

  // Create intercommunicators.  Current implementation is very similar to
  // master-slave in that only the 1st server has an array of intercomms.  This
  // reflects the current ApplicationInterface::static_schedule_message_passing
  // design.  A more general implementation would give each peer an array of 
  // intercomms, and could easily be supported in the future if needed.
  if (child_pl.serverId == 1) {
    child_pl.hubServerInterComms = new MPI_Comm [child_pl.numServers-1];
    for(i=0; i<child_pl.numServers-1; ++i)
      MPI_Intercomm_create(child_pl.serverIntraComm, 0,
			   parent_pl.serverIntraComm, start[i+1], i+2,
			   &child_pl.hubServerInterComms[i]);
  }
  else
    MPI_Intercomm_create(child_pl.serverIntraComm, 0, parent_pl.serverIntraComm,
			 0, color, &child_pl.hubServerInterComm);
#ifdef MPI_DEBUG
  Cout << "worldRank = " << worldRank << " child comm rank = " 
       << child_pl.serverCommRank << " child comm size = "
       << child_pl.serverCommSize << '\n';
  int size, remote_size;
  if (child_pl.serverId == 1) { // first peer
    for (int i=0; i<child_pl.numServers-1; ++i) {
      MPI_Comm_size(child_pl.hubServerInterComms[i], &size);
      MPI_Comm_remote_size(child_pl.hubServerInterComms[i], &remote_size);
      Cout << "Peer 1: size = " << size << " inter_comms[" << i
           << "] remote_size = " << remote_size << '\n';
    }
  }
  else { // peers 2 through n
    MPI_Comm_size(child_pl.hubServerInterComm, &size);
    MPI_Comm_remote_size(child_pl.hubServerInterComm, &remote_size);
    Cout << "Peer " << child_pl.serverId << ": size = " << size 
         <<" inter_comm remote_size = " << remote_size << '\n';
  }
  Cout << std::flush;
#endif // MPI_DEBUG

  // Create parent-child intracommunicator used to avoid broadcasting over all
  // of parent Comm when all you really need is the child rank == 0 processors.
  color = (child_pl.serverCommRank == 0) ? 1 : 0;
  MPI_Comm_split(parent_pl.serverIntraComm, color, parent_pl.serverCommRank,
		 &child_pl.hubServerIntraComm);
  if (child_pl.serverCommRank == 0) {
    MPI_Comm_rank(child_pl.hubServerIntraComm, &child_pl.hubServerCommRank);
    MPI_Comm_size(child_pl.hubServerIntraComm, &child_pl.hubServerCommSize);
  }
#endif // DAKOTA_HAVE_MPI

  if (child_pl.serverCommRank == 0) // don't exclude parent master 
    child_pl.serverMasterFlag = true; // this proc is a child partition master
  return true; // Set split flag to true in calling routine
}


void ParallelLibrary::print_configuration()
{
  // Print a consolidated report for all ParallelLevels within the current
  // ParallelConfiguration instance.  Calls to this function are protected by
  // worldSize in ApplicationInterface::init_communicators() and by worldRank/
  // stratDedicatedMasterFlag in ConcurrentStrategy::self_schedule_iterators().

  const ParallelConfiguration& pc = *currPCIter;
  const ParallelLevel& si_pl = pc.si_parallel_level();
  const ParallelLevel& ie_pl = pc.ie_parallel_level();
  const ParallelLevel& ea_pl = pc.ea_parallel_level();

  // --------------------------------------------------
  // Send partition info up the chain to worldRank == 0
  // --------------------------------------------------
  // If the strategy has a dedicated master, then only iterator servers 
  // participate in evaluation partitioning.  And if each iterator has a
  // dedicated master, then only evaluation servers participate in analysis
  // partitioning.  This requires up to two messages (within the confines of
  // the existing communicator and send/recv structure) to jump these two gaps
  // in partitioning participation.
  if ( si_pl.serverId == 1 && ie_pl.dedicatedMasterFlag && 
       ie_pl.serverId == 1 && ie_pl.serverCommRank == 0 ) {
    // eval server 1 master sends analysis info to iterator server 1 master
    MPIPackBuffer send_buffer(64);
    send_buffer << ea_pl.numServers << ea_pl.procsPerServer
                << ea_pl.dedicatedMasterFlag << pc.numParallelLevels;
    send_ie(send_buffer, 0, 1001);
  }
  // use local copies for settings from other processors
  // (do not update parallel settings on message recipients).
  int   num_anal_srv = ea_pl.numServers, p_per_anal = ea_pl.procsPerServer;
  bool  eval_ded_master_flag = ea_pl.dedicatedMasterFlag;
  short par_levels = pc.numParallelLevels;
  if (si_pl.serverId == 1 && si_pl.serverCommRank == 0) {
    if (ie_pl.dedicatedMasterFlag) {
      // iterator server 1 master recv's analysis info from eval server 1 master
      MPIUnpackBuffer recv_buffer(64);
      MPI_Status status;
      recv_ie(recv_buffer, 1, 1001, status);
      recv_buffer >> num_anal_srv >> p_per_anal >> eval_ded_master_flag
                  >> par_levels;
    }
    if (si_pl.dedicatedMasterFlag) {
      // iterator server 1 master sends combined info to strategy master
      MPIPackBuffer send_buffer(64);
      send_buffer << ie_pl.numServers << ie_pl.procsPerServer
                  << ie_pl.dedicatedMasterFlag << num_anal_srv << p_per_anal
                  << eval_ded_master_flag << par_levels;
      send_si(send_buffer, 0, 1002);
    }
  }

  // -----------------------------------
  // worldRank == 0 prints configuration
  // -----------------------------------
  if (worldRank == 0) { // does all output
    int  num_eval_srv = ie_pl.numServers, p_per_eval = ie_pl.procsPerServer;
    bool iterator_ded_master_flag = ie_pl.dedicatedMasterFlag;
    if (si_pl.dedicatedMasterFlag) {
      // strategy master receives combined info from iterator server 1 master
      MPIUnpackBuffer recv_buffer(64); // 4 ints+1 short+2 bools < ~20 bytes
      MPI_Status status;
      recv_si(recv_buffer, 1, 1002, status);
      recv_buffer >> num_eval_srv >> p_per_eval >> iterator_ded_master_flag
                  >> num_anal_srv >> p_per_anal >> eval_ded_master_flag
                  >> par_levels;
    }

    // Strategy diagnostics
    Cout << "\n---------------------------------------------------------------"
	 << "--------------\nDAKOTA parallel configuration:\n\n"
	 << "Level\t\t\tnum_servers    procs_per_server    partition/"
	 << "schedule\n-----\t\t\t-----------    ----------------    "
	 << "------------------\nconcurrent iterators\t  " << std::setw(4)
         << si_pl.numServers << "\t\t   " << std::setw(4)
	 << si_pl.procsPerServer << "\t\t       ";
    if (si_pl.dedicatedMasterFlag) Cout << "ded. master\n";
    else                           Cout << "peer\n";

    // Iterator diagnostics
    Cout << "concurrent evaluations\t  " << std::setw(4) << num_eval_srv
	 << "\t\t   " << std::setw(4) << p_per_eval << "\t\t       ";
    if (iterator_ded_master_flag)  Cout << "ded. master\n";
    else                           Cout << "peer\n";

    // Evaluation diagnostics
    Cout << "concurrent analyses\t  " << std::setw(4) << num_anal_srv
	 << "\t\t   " << std::setw(4) << p_per_anal << "\t\t       ";
    if (eval_ded_master_flag)      Cout << "ded. master\n";
    else                           Cout << "peer\n";

    // Analysis diagnostics
    Cout << "multiprocessor analysis\t  " << std::setw(4) << p_per_anal
         << "\t\t     N/A\t       N/A\n\nTotal parallelism levels =   " 
         << par_levels << "\n-------------------------------------------------"
	 << "----------------------------" << std::endl;
  }
}


/** On the rank 0 processor, get the -output, -error, -read_restart,
    and -write_restart filenames and the -stop_restart limit from the
    command line.  Defaults for the filenames from the command line
    handler are NULL for the filenames except write which defaults to
    dakota.rst and 0 for read_restart_evals if no user specification.
    This information is Bcast from rank 0 to all iterator masters in
    manage_outputs_restart(). */
void ParallelLibrary::
specify_outputs_restart(CommandLineHandler& cmd_line_handler)
{
  // This is the command line version of specify_outputs_restart().
  // It retrieves inputs from the command line handler object.  This
  // is the normal mode used by main.cpp within DAKOTA.

  // Need to catch the NULL case; this will clean up when GetLongOpt
  // gets refactored
  if (worldRank == 0) {
    if (cmd_line_handler.retrieve("output"))
      stdOutputFilename = cmd_line_handler.retrieve("output");
    if (cmd_line_handler.retrieve("error"))
      stdErrorFilename = cmd_line_handler.retrieve("error");
    if (cmd_line_handler.retrieve("read_restart")) {
      readRestartFilename  = cmd_line_handler.retrieve("read_restart");
      // check for specified, but without arg; use default restart filename
      if (readRestartFilename.empty())
	readRestartFilename = "dakota.rst";
    }
    if (cmd_line_handler.retrieve("write_restart"))
      writeRestartFilename = cmd_line_handler.retrieve("write_restart");
    stopRestartEvals     = cmd_line_handler.read_restart_evals();

    manage_run_modes(cmd_line_handler);
    assign_streams(true);
  }
}


/** Rather than extracting from the command line, pass the std output,
    std error, read restart, and write restart filenames and the stop
    restart limit directly.  This function only needs to be invoked to
    specify non-default values [defaults for the filenames are NULL
    (resulting in no output redirection, no restart read, and default
    restart write) and 0 for the stop restart limit (resulting in no
    restart read limit)]. */
void ParallelLibrary::
specify_outputs_restart(const char* clh_std_output_filename,
			const char* clh_std_error_filename,
			const char* clh_read_restart_filename,
			const char* clh_write_restart_filename,
			int stop_restart_evals, bool pre_run_flag)
{
  // This is the external input version of specify_outputs_restart that
  // provides a library mode for codes such as SIERRA.  It relies on
  // external retrieval of output settings.

  // Need to catch the NULL case; this will clean up when we change
  // the interface to an std::string
  if (clh_std_output_filename)
    stdOutputFilename = clh_std_output_filename;
  if (clh_std_error_filename)
    stdErrorFilename = clh_std_error_filename;
  if (clh_read_restart_filename)
    readRestartFilename  = clh_read_restart_filename;
  if (clh_write_restart_filename)
    writeRestartFilename = clh_write_restart_filename;
  stopRestartEvals = stop_restart_evals;
  
  // TODO: allow passing any relevant flags; for now emulate historical
  // (only pre-run supported and only by itself)
  if (pre_run_flag) {
    userModesFlag = preRunFlag = true;
    runFlag = postRunFlag = false;
  }
  
  assign_streams(false);
}


void ParallelLibrary::manage_run_modes(CommandLineHandler& cmd_line_handler)
{
  // If filenames empty, do not define defaults; user might not want.

  checkFlag = cmd_line_handler.retrieve("check") != NULL;

  // populate the filenames as necessary
  if ( preRunFlag = (cmd_line_handler.retrieve("pre_run") != NULL))
    split_filenames(cmd_line_handler.retrieve("pre_run"), preRunInput, 
		    preRunOutput);      
  if ( runFlag = (cmd_line_handler.retrieve("run") != NULL))
    split_filenames(cmd_line_handler.retrieve("run"), runInput, runOutput);
  if ( postRunFlag = (cmd_line_handler.retrieve("post_run") != NULL))
    split_filenames(cmd_line_handler.retrieve("post_run"), postRunInput, 
		    postRunOutput);

  // If user didn't specify any specific run modes, default to execute all
  // phases
  if ( !preRunFlag && !runFlag && !postRunFlag ) {
    preRunFlag = runFlag = postRunFlag = true;
    userModesFlag = false; // no active user modes
  }
  else
    userModesFlag = true;  // one or more active user modes

  // override timing output default if proceeding to run
  outputTimings = cmd_line_handler.run_flag();
}


/// Tokenize colon-delimited input and output filenames, returns
/// unchanged strings if tokens not found
void ParallelLibrary::
split_filenames(const char * filenames, std::string& input_filename,
		std::string& output_filename)
{
  // Initial cut doesn't warn/error on invalid input, since pre and post 
  // are special cases in terms of permitted files.  For now admit all specs
  // and silently ignore.
  // NOTE: zero-length (omitted) strings are allowed, but the double colon is 
  // required if the output argument is specified, so, for instance.
  // -pre ::pre.out
  // -post post.in
  if (filenames != NULL) {
    std::string runarg(filenames);
    std::string::size_type pos = runarg.find("::");
    if (pos != std::string::npos) {
      input_filename = runarg.substr(0, pos);
      pos = runarg.find_first_not_of("::", pos);
      if (pos != std::string::npos)
	output_filename = runarg.substr(pos, std::string::npos);
    }
    else
      input_filename = runarg;
  }
}


void ParallelLibrary::assign_streams(bool append)
{
  // Direct output/error to file early, possibly overriding later
  // after Strategy is running and manage_outputs_restart gets called.
  // Would prefer to share with CLH, but don't want tighter coupling.
  
  // Therefore do only on DAKOTA rank 0 for now
  if (worldRank == 0) {

    // If using CLH, file should be created, so append, else write
    if (!stdOutputFilename.empty()) {
      if (append)
	output_ofstream.open(stdOutputFilename.c_str(), std::ios::app);
      else
	output_ofstream.open(stdOutputFilename.c_str(), std::ios::out);
      if (!output_ofstream.good()) {
	Cerr << "\nError opening output file '" << stdOutputFilename << "'" 
	     << std::endl;
	abort_handler(-1);
      }
      // assign global dakota_cout to this ofstream
      dakota_cout = &output_ofstream;
    }
    if (!stdErrorFilename.empty()) {
      if (append)
	error_ofstream.open(stdErrorFilename.c_str(), std::ios::app);
      else
	error_ofstream.open(stdErrorFilename.c_str(), std::ios::out);
      if (!error_ofstream.good()) {
	Cerr << "\nError opening error file '" << stdErrorFilename << "'"
	     << std::endl;
	abort_handler(-1);
      }
      // assign global dakota_cerr to this ofstream
      dakota_cerr = &error_ofstream;
    }

  }

}


/** If the user has specified the use of files for DAKOTA standard
    output and/or standard error, then bind these filenames to the
    Cout/Cerr macros.  In addition, if concurrent iterators are to be
    used, create and tag multiple output streams in order to prevent
    jumbled output. Manage restart file(s) by processing any incoming
    evaluations from an old restart file and by setting up the binary
    output stream for new evaluations.  Only master iterator
    processor(s) read & write restart information.  This function must
    follow init_iterator_communicators so that restart can be managed
    properly for concurrent iterator strategies.  In the case of
    concurrent iterators, each iterator has its own restart file
    tagged with iterator number. */
void ParallelLibrary::
manage_outputs_restart(const ParallelLevel& pl, bool results_output,
		       std::string results_filename)
{
  // The incoming pl should be the lowest of the concurrent iterator levels

  // If not rank 0 within an iteratorComm, then no output/restart to
  // manage.  Note that a strategy dedicated master must participate
  // in the command line bcasts, but returns thereafter since it does
  // not redirect its output or read from restart files.
  if (pl.serverCommRank)
    return;

  // ---------------------------------------------
  // Process strings and Bcast to iterator masters
  // ---------------------------------------------

  // local copies of strings to avoid confusion with user-specified vs.
  //  locally-modified; could consider just overwriting the class data
  std::string std_output_filename, std_error_filename, read_restart_filename, 
    write_restart_filename;
  bool read_restart_flag;
  if (worldRank == 0) {

    // need a default even if user didn't specify due to possible
    // per-server tagging
    std_output_filename    = (stdOutputFilename.empty()) ? 
                             "dakota.out" : stdOutputFilename;
    std_error_filename     = stdErrorFilename;
    read_restart_filename  = readRestartFilename;
    write_restart_filename = (writeRestartFilename.empty()) ? 
                             "dakota.rst" : writeRestartFilename;

    // If iterator servers are in use, then always segregate the std output.
    // However, for std error, assume that this should remain directed to the
    // screen unless an explicit "-e" command line option has been given.
    stdOutputToFile = (pl.numServers > 1 || pl.dedicatedMasterFlag || 
		       !stdOutputFilename.empty()) ? true : false;
    stdErrorToFile  = !stdErrorFilename.empty();
    read_restart_flag = !readRestartFilename.empty();

    if (pl.hubServerCommSize > 1) {
      MPIPackBuffer send_buffer;
      send_buffer << stdOutputToFile << stdErrorToFile 
		  << checkFlag << preRunFlag << runFlag << postRunFlag 
		  << userModesFlag << preRunInput << preRunOutput << runInput
		  << runOutput << postRunInput << postRunOutput
		  << read_restart_flag << std_output_filename
		  << std_error_filename << read_restart_filename
		  << write_restart_filename << stopRestartEvals;
      int buffer_len = send_buffer.size();
      bcast(buffer_len,  pl.hubServerIntraComm);
      bcast(send_buffer, pl.hubServerIntraComm);
    }
  }
  else if (pl.hubServerCommSize > 1) {
    int buffer_len;
    bcast(buffer_len, pl.hubServerIntraComm);
    MPIUnpackBuffer recv_buffer(buffer_len);
    bcast(recv_buffer, pl.hubServerIntraComm);
    recv_buffer >> stdOutputToFile >> stdErrorToFile 
		>> checkFlag >> preRunFlag >> runFlag >> postRunFlag 
		>> userModesFlag >> preRunInput >> preRunOutput >> runInput
		>> runOutput >> postRunInput >> postRunOutput
		>> read_restart_flag >> std_output_filename
		>> std_error_filename >> read_restart_filename
		>> write_restart_filename >> stopRestartEvals;
  }

  // TODO: tagging for pre/run/post I/O files

  // This returns a strategy dedicated master processor, if present.
  if (!pl.serverMasterFlag)
    return;

  // Tag cout/cerr/read_restart/write_restart for case of concurrent iterators
  if (pl.numServers > 1 || pl.dedicatedMasterFlag) {
    // could change to numServers>0 since it would still be nice to organize
    // the output for 1 server in BranchBndStrategy/ConcurrentStrategy
    char si[16];
    std::sprintf(si, ".%d", pl.serverId);
    std::string ctr_tag(si);
    std_output_filename += ctr_tag;     // e.g., "dakota.out.#"
    if (stdErrorToFile)
      std_error_filename += ctr_tag;    // e.g., "dakota.err.#"
    if (read_restart_flag)
      read_restart_filename += ctr_tag; // e.g., "dakota.rst.#"
    write_restart_filename  += ctr_tag; // e.g., "dakota_new.rst.#"
    if (results_output)
      results_filename += ctr_tag;
  }

  // --------------------------------
  // Manage stdout and stderr streams
  // --------------------------------
  // Now that all iterator masters have the output filename settings, open
  // the ofstreams and attach them to Cout/Cerr (if required).  Note that the
  // opening of files on processors for which there is no output is avoided.
  if (stdOutputToFile) {
    // check if there was a file name added or changed
    if (std_output_filename != stdOutputFilename) {
      // close and reopen the stream to a different file, no need to append
      if (output_ofstream.is_open())
	output_ofstream.close();
      output_ofstream.open(std_output_filename.c_str(), std::ios::out);
    }
    else {
      // If already open, just keep writing.  If not, either append
      // (created by CLH) or create new (library) file.  This is safe
      // only because the option to append is handled above, and
      // needed because a library customer may never call
      // specify_outputs_restart.
      if (!output_ofstream.is_open())
	output_ofstream.open(std_output_filename.c_str(), std::ios::out);
    }
    // assign global dakota_cout to this ofstream
    dakota_cout = &output_ofstream;
  }
  if (stdErrorToFile) {
    if (std_error_filename != stdErrorFilename) {
      if (error_ofstream.is_open())
	error_ofstream.close();
      error_ofstream.open(std_error_filename.c_str(), std::ios::out);
    }
    else {
      if (!error_ofstream.is_open())
	error_ofstream.open(std_error_filename.c_str(), std::ios::out);
    }
    // assign global dakota_cerr to this ofstream
    dakota_cerr = &error_ofstream;
  }

  // Manage iterator results output to database
  if (results_output)
    iterator_results_db.initialize(results_filename);

  // ------------
  // Read restart
  // ------------
  // Process the evaluations from the restart file
  if (read_restart_flag) {
    BiStream read_restart(read_restart_filename.c_str());
    if (!read_restart.good()) {
      Cerr << "\nError: could not open restart file " << read_restart_filename
	   << std::endl;
      abort_handler(-1);
    }

    // The -stop_restart input for restricting the number of evaluations read
    // in from the restart file is very useful when the last few evaluations in
    // a run were corrupted.  Note that the desired -stop_restart setting may
    // differ from the evaluation number in the previous run since detected 
    // duplicates are included in Interface::evalIdCntr, but are not written
    // to the restart file!
    if (stopRestartEvals) // cmd_line_handler returns 0 if no cmd line setting
      Cout << "Stopping restart file processing at " << stopRestartEvals 
           << " evaluations." << std::endl;

    int cntr = 0;
    while ( !read_restart.eof() && 
            (!stopRestartEvals || cntr < stopRestartEvals) ) {
      // Use default constr. & rely on Variables::read(BiStream&)
      // & Response::read(BiStream&) to resize vars and response.
      ParamResponsePair current_pair;

      // Exception handling installed since EOF is not captured properly 
      // by BiStream.
      try { read_restart >> current_pair; }

      catch(std::string& err_msg) {
        //Cerr << "Warning: " << err_msg << std::endl;
        break;
      }

      data_pairs.insert(current_pair); ++cntr;
      Cout << "\n------------------------------------------\nRestart record "
	   << std::setw(4) << cntr << "  (evaluation id " << std::setw(4)
	   << current_pair.eval_id() << "):"
	   << "\n------------------------------------------\n" << current_pair;
      // Note: interface id is printed in ParamResponsePair::write(ostream&)
      // if present
    }
    read_restart.close();
    Cout << "Restart file processing completed: " << cntr
	 << " evaluations retrieved and " << data_pairs.size()
	 << " unique evaluations stored.\n";
  }

  // -------------
  // Write restart
  // -------------
  // Always write a restart log file.  Assign the write_restart stream to the
  // filename specified by the user on the dakota command line.  If a 
  // write_restart file is not specified, "dakota.rst" is the default.  It 
  // would be desirable to suppress the creation of the restart file altogether
  // if the user has explicitly deactivated this feature; however this is
  // problematic for 2 reasons: (1) problem_db is not readily available (except
  // in init_iterator_communicators()), and (2) the "deactivate restart_file"
  // specification is linked to the interface and therefore should be enforced
  // per interface, whereas there is only one parallel library instance.
  //if (!deactivateRestartFlag) {
  if (write_restart_filename == read_restart_filename) {
    write_restart.open(write_restart_filename.c_str(), std::ios::app);

    Cout << "Appending new evaluations to existing restart file " 
         << write_restart_filename << std::endl;
    // One possible problem here is the use of stop_restart, since we could
    // be appending new results for the same points back to the same file,
    // such that multiple records (potentially corrupted results followed by
    // good results) exist for the same point.  Need to either (1) remove the
    // unread records (by closing and rewriting the file) before writing new
    // ones or (2) always write a new restart file in the stop_restart case.
    // In case (2), the processed records from the old restart file should be
    // written to the new one (see below).

    // NOTE: It may be preferable to always start over to avoid appending to
    // a damaged file.  Restart is often used when DAKOTA has been killed by a
    // scheduler in which case the file could have a partial evaluation at the
    // end.  If reusing dakota.rst as the write_restart, std::ios::trunc
    // (implied by std::ios::out which is the default) will discard any old file
    // contents and start over [see FSTREAM(3C++) in AT&T C++ Library Manual].
  }
  else {
    write_restart.open(write_restart_filename.c_str());
    Cout << "Writing new restart file " << write_restart_filename << std::endl;
    // Write any processed records from the old restart file to the new file.
    // This prevents the situation where good data from an initial run and a 
    // restart run are in separate files.  By keeping all of the saved data in
    // 1 file, restarts can be chained together indefinitely.
    //
    // "View" the data_pairs cache as an ordered collection (by eval_id)
    PRPCacheCIter cit, cit_end = data_pairs.end();
    for (cit = data_pairs.begin(); cit != cit_end; ++cit)
      write_restart << *cit;
  }
  //}
}


/** Close streams associated with manage_outputs and manage_restart
    and terminate any additional services that may be active. */
void ParallelLibrary::close_streams()
{
  // In the case of -help or -version, this code is reached before the
  // Strategy is constructed, and therefore before any Model
  // constructors call increment_parallel_configuration().  In this
  // scenario, currPCIter->siPLIter points to parallelLevels.end() and
  // can't be dereferenced:
  //   ParallelLevel& pl = *currPCIter->siPLIter; // temp hack

  // The code here does not have to worry about redirection in
  // CommandLineHandler, as it resets when done, closing streams.

  // Therefore only need to close streams if we have a valid
  // parallelLevel and there is a file write occuring; test for end.

  bool server_master_flag = false;  // assume we're not a server master
  if (currPCIter->siPLIter != parallelLevels.end()) {
    ParallelLevel& pl = *currPCIter->siPLIter; // temp hack
    server_master_flag = pl.serverMasterFlag;
  }

  // clean up data from manage_outputs:
  // close ofstreams and reassign pointers for Cout/Cerr so that any subsequent
  // output (e.g., timings in ParallelLibrary destructor) is handled properly.
  if (stdOutputToFile && server_master_flag) {
    output_ofstream.close();
    dakota_cout = &std::cout;
  }
  if (stdErrorToFile && server_master_flag) {
    error_ofstream.close();
    dakota_cerr = &std::cerr;
  }

  // clean up data from manage_restart
  if (server_master_flag) // && !deactivateRestartFlag)
    write_restart.close();

  // terminate any additional services that may be active
#ifdef DAKOTA_MODELCENTER
  int ireturn, iprint = 1; // print debugging info from ModelCenter API calls
  if (mc_ptr_int)
    mc_release_com(ireturn, iprint, mc_ptr_int);
  if (ireturn == -1) {
    Cerr << "Error: mc_release of API pointer unsuccessful." << std::endl;
    abort_handler(-1);
  }
  if (dc_ptr_int)
    mc_release_com(ireturn, iprint, dc_ptr_int);
  if (ireturn == -1) {
    Cerr << "Error: mc_release of DC pointer unsuccessful." << std::endl;
    abort_handler(-1);
  }
#endif // DAKOTA_MODELCENTER
}


void ParallelLibrary::abort_helper(int code) const {
  
  // Abort the process(es)
#ifdef DAKOTA_HAVE_MPI
#ifdef HAVE_AIX_MPI
  // AIX POE utility function for signal handling w/MPI to exit normally.
  pm_child_sig_handler(code, NULL, NULL);
#endif
  int initialized = 0;
  MPI_Initialized(&initialized);
  if (initialized)
    MPI_Abort(dakotaMPIComm, code);
  else
    std::exit(code);
#else

#ifdef DAKOTA_MODELCENTER
  throw(std::logic_error("ModelCenter error"));
#else
  std::exit(code); // or std::exit(EXIT_FAILURE) from /usr/include/stdlib.h
#endif // MODELCENTER

#endif // DAKOTA_HAVE_MPI

}


void ParallelLibrary::
output_helper(const std::string& message, std::ostream &os) const
{
  if (mpirunFlag) {
    if (worldRank == 0)
      os << message << std::endl;
  }
  else 
    os << message << std::endl;
}


void ParallelLibrary::free_communicators(ParallelLevel& pl)
{
#ifdef DAKOTA_HAVE_MPI
  if (pl.commSplitFlag) { // deallocate intra/inter comms.
    MPI_Comm_free(&pl.serverIntraComm);
    MPI_Comm_free(&pl.hubServerIntraComm);
    if (pl.dedicatedMasterFlag) { // master-slave interComms
      if (pl.serverId == 0) { // if dedicated master
        for(int i=0; i<pl.numServers; ++i) 
          MPI_Comm_free(&pl.hubServerInterComms[i]);
        delete [] pl.hubServerInterComms;
      }
      else // servers 1 through n
        MPI_Comm_free(&pl.hubServerInterComm);
    }
    else { // peer interComms
      if (pl.serverId == 1) { // 1st peer
        for(int i=0; i<pl.numServers-1; ++i) 
          MPI_Comm_free(&pl.hubServerInterComms[i]);
        delete [] pl.hubServerInterComms;
      }
      else // peers 2 through n
        MPI_Comm_free(&pl.hubServerInterComm);
    }
  }
#endif // DAKOTA_HAVE_MPI (else no finalization needed)
}


ParallelLibrary::~ParallelLibrary()
{
  if (this == Dak_pl)
    Dak_pl = NULL;

  if (!dummyFlag) { // protect MPI_Finalize in case of dummy_lib

    // Output timings before closing streams, in case of redirection
    output_timers();

    // Close the output and restart streams and any other services
    close_streams();

#ifdef DAKOTA_HAVE_MPI
    // call MPI_Finalize only if DAKOTA called MPI_Init
    if (mpirunFlag && ownMPIFlag)
      MPI_Finalize();
#endif // DAKOTA_HAVE_MPI
      
    if (worldRank == 0)
      dakota_graphics.close(); // after completion of timings

  }
}


void ParallelLibrary::output_timers()
{
  if (!outputTimings)
    return;

  // Compute elapsed times.
  // TODO: sometimes totalCPU is zero, but parent is zero;
  //       need to consistently use system or utilib for this computation
  //       or bound below by zero
  Real totalCPU = (Real)(clock() - startClock)/CLOCKS_PER_SEC;
#ifdef DAKOTA_UTILIB
  Real parentCPU = CPUSeconds() - startCPUTime, 
    childCPU  = totalCPU - parentCPU,
    totalWC   = WallClockSeconds() - startWCTime;
#endif // DAKOTA_UTILIB

  if (mpirunFlag) { // MPI functions are available
#ifdef DAKOTA_HAVE_MPI
    if (worldRank==0) {
      Real runWC = parallel_time();
      Cout << std::setprecision(6) << std::resetiosflags(std::ios::floatfield)
	   << "DAKOTA master processor execution time in seconds:\n"
	   << "  Total CPU        = " << std::setw(10) << totalCPU;
      
#ifdef DAKOTA_UTILIB
      Real initWC = totalWC - runWC;
      Cout << " [parent   = " << std::setw(10) << parentCPU << ", child = " 
	   << std::setw(10) << childCPU << "]\n  Total wall clock = " << std::setw(10)
	   << totalWC << " [MPI_Init = " << std::setw(10) << initWC 
	   << ", run   = " << std::setw(10) << runWC << "]" << std::endl;
#else
      Cout << "\n  MPI wall clock = " << std::setw(10) << runWC << std::endl;
#endif // DAKOTA_UTILIB
    }
#endif // DAKOTA_HAVE_MPI
  }
  else { // MPI functions are not available
      Cout << std::setprecision(6) << std::resetiosflags(std::ios::floatfield)
	   << "DAKOTA execution time in seconds:\n  Total CPU        = " 
	   << std::setw(10) << totalCPU;
#ifdef DAKOTA_UTILIB
      Cout << " [parent = " << std::setw(10) << parentCPU << ", child = "
	   << std::setw(10) << childCPU << "]\n  Total wall clock = "
	   << std::setw(10) << totalWC << std::endl;
#else
      Cout << std::endl;
#endif // DAKOTA_UTILIB
  }
}

} // namespace Dakota
