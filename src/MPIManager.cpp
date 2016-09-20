/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       MPIManager
//- Description: Implementation code for the MPIManager class
//- Owner:       Brian Adams
//- Checked by:

#include <cctype>
#include "dakota_system_defs.hpp"
#include "MPIManager.hpp"
#include "dakota_data_types.hpp"
#include "dakota_global_defs.hpp"

namespace Dakota {


MPIManager::MPIManager():
  dakotaMPIComm(MPI_COMM_WORLD), dakotaWorldRank(0), dakotaWorldSize(1),
  mpirunFlag(false), ownMPIFlag(false)
{
  // MPI check for library clients not passing any options or MPI
  // comm, since that will invoke this default ctor.  
  // BMA: This shouldn't cause problems for dummy objects as
  // ownMPIFlag will be false.
#ifdef DAKOTA_HAVE_MPI
  // Do not initialize MPI, but check if initialized and get data on rank/size.
  int initialized = 0;
  MPI_Initialized(&initialized);
  if (initialized) {
    mpirunFlag = true;
    MPI_Comm_rank(dakotaMPIComm, &dakotaWorldRank);
    MPI_Comm_size(dakotaMPIComm, &dakotaWorldSize);
  }
#endif
}


MPIManager::MPIManager(int& argc, char**& argv):
  dakotaMPIComm(MPI_COMM_WORLD), dakotaWorldRank(0), dakotaWorldSize(1),
  mpirunFlag(false), ownMPIFlag(false)
{
  // detect parallel launch of DAKOTA using mpirun/mpiexec/poe/etc.
  mpirunFlag = detect_parallel_launch(argc, argv);
  
#ifdef DAKOTA_HAVE_MPI
  // Initialize MPI if and only if DAKOTA launched in parallel from
  // the command-line (mpirunFlag).  Here MPI_Init pulls off the MPI
  // arguments and Dakota owns MPI.
  if (mpirunFlag) {
    MPI_Init(&argc, &argv); // See comment above regarding argv and argc
    ownMPIFlag = true; // own MPI_Init, so call MPI_Finalize in destructor 
    MPI_Comm_rank(dakotaMPIComm, &dakotaWorldRank);
    MPI_Comm_size(dakotaMPIComm, &dakotaWorldSize);
  }
#endif
}


MPIManager::MPIManager(MPI_Comm dakota_mpi_comm):
  dakotaMPIComm(dakota_mpi_comm), dakotaWorldRank(0), dakotaWorldSize(1),
  mpirunFlag(false), ownMPIFlag(false)
{
#ifdef DAKOTA_HAVE_MPI
  // Do not initialize MPI, but check if initialized and get data on rank/size.
  int initialized = 0;
  MPI_Initialized(&initialized);
  if (initialized) {
    mpirunFlag = true;
    MPI_Comm_rank(dakotaMPIComm, &dakotaWorldRank);
    MPI_Comm_size(dakotaMPIComm, &dakotaWorldSize);
  }
#endif
}


MPIManager::~MPIManager()
{
#ifdef DAKOTA_HAVE_MPI
  // call MPI_Finalize only if DAKOTA called MPI_Init
  if (mpirunFlag && ownMPIFlag)
    MPI_Finalize();
#endif // DAKOTA_HAVE_MPI
}


// Consider having the output manager queue up any messages prior to
// rebinding cout/cerr
bool MPIManager::detect_parallel_launch(int& argc, char**& argv)
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
  // 
  // The logic falls through and sets mpi_launch true if Dakota was configured 
  // with MPI but lacks detection for the platform/MPI implementation. 
  //
  // The environment variable DAKOTA_RUN_PARALLEL overrides automatic
  // detection. If it is set and begins with 1, t, or T, mpi_launch is set 
  // true (run in parallel mode). If it is defined and set to anything else, 
  // mpi_launch remains false (run in serial mode).
  
#ifdef DAKOTA_HAVE_MPI
  // Check DAKOTA_RUN_PARALLEL override.
  char* parallel_override = std::getenv("DAKOTA_RUN_PARALLEL");
  if(parallel_override) {
    if(parallel_override[0] == '1' || toupper(parallel_override[0]) == 'T') {
#ifdef MPI_DEBUG
    Cout << "Parallel run forced by DAKOTA_RUN_PARALLEL" << std::endl;
#endif
      mpi_launch = true;
    }
#ifdef MPI_DEBUG
    else {
      Cout << "Serial run forced by DAKOTA_RUN_PARALLEL" << std::endl;
    }
#endif
    return mpi_launch;
  }
#endif

  // Begin checks for automatic detection
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

#elif defined(MPICH_NAME) || defined(MPICH_VERSION)
  // MPICH1 defines MPICH_NAME, MPICH_VERSION
  // MPICH2 defines MPICH_NAME, MPICH2
  // MPICH3 defines MPICH_NAME, MPICH_VERSION
  // test for command-line p4 device arguments
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
  if (!mpi_launch) {
    // Command-line content didn't suffice.  Check environment variables.
    // MPICH environment variables in preferred order (roughly newest to oldest)
    StringArray env_vars;
    env_vars.push_back("ALPS_APP_PE");              // MPICH on Cray Linux Environment
    env_vars.push_back("PMI_SIZE");                 // MPICH3
    env_vars.push_back("MPICH_NP");                 // MPICH2, incl shmem
    env_vars.push_back("MPICH_INTERFACE_HOSTNAME"); // MPICH2, old p4 check
    env_vars.push_back("MPIRUN_NPROCS");            // MPICH1, Myrinet/all
    env_vars.push_back("GMPI_NP");                  // MPICH1, Myrinet GM
    // Also historically validated GMPI_NP > 1.
    // For MPICH2, can't check MPIRUN_DEVICE; only set on master.

    StringArray::const_iterator ev_it = env_vars.begin();
    StringArray::const_iterator ev_end = env_vars.end();
    for ( ; ev_it != ev_end; ++ev_it) {
      char* env_val = std::getenv(ev_it->c_str());
      if (env_val) {
#ifdef MPI_DEBUG
	Cout << "Parallel run detected via MPICH env test: " << env_val 
		  << std::endl;
#endif
	mpi_launch = true;
	break;
      }
    }
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
  // For platforms w/o runtime detection above (e.g., TFLOPS_COMPUTE,
  // CPLANT_COMPUTE), we historically assumed that if Dakota was built
  // with DAKOTA_HAVE_MPI, it is run with MPI.  May want to reconsider
  // this, though may be useful for IBM, Cray.
#ifdef DAKOTA_HAVE_MPI
#ifdef MPI_DEBUG
  Cout << "Parallel run enabled via fall-through default" << std::endl;
#endif
  mpi_launch = true;
#endif // DAKOTA_HAVE_MPI

#endif // OPEN_MPI

  return mpi_launch;
}

}  // namespace Dakota
