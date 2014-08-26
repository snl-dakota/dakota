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

#include "dakota_system_defs.hpp"
#include "MPIManager.hpp"

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

}  // namespace Dakota
