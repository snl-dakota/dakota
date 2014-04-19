/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ParallelLibrary
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include <boost/lexical_cast.hpp>
#include "ParallelLibrary.hpp"
#include "ProblemDescDB.hpp"
#include "ProgramOptions.hpp"
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

// on by default from consensus among library users Collis/Drake/Keiter/Salinger
#define COMM_SPLIT_TO_SINGLE

//#define MPI_DEBUG

static const char rcsId[]="@(#) $Id: ParallelLibrary.cpp 7013 2010-10-08 01:03:38Z wjbohnh $";


namespace Dakota {

extern MPIManager     dummy_mpi_mgr; // defined in dakota_global_defs.cpp
extern ProgramOptions dummy_prg_opt; // defined in dakota_global_defs.cpp
extern OutputManager  dummy_out_mgr; // defined in dakota_global_defs.cpp


/** This constructor is used for creation of the global dummy_lib
    object, which is used to satisfy initialization requirements when
    the real ParallelLibrary object is not available. */
ParallelLibrary::ParallelLibrary(): 
  mpiManager(dummy_mpi_mgr), programOptions(dummy_prg_opt),
  outputManager(dummy_out_mgr), dummyFlag(true), outputTimings(false)
{ }


/** TODO: Update comment.

    Same constructor is used for executable and
    library environments and sequencing of object construction is
    ordered, so no need to separately get updates off command line
    (programOptions)
 */
ParallelLibrary::
ParallelLibrary(const MPIManager& mpi_mgr, ProgramOptions& prog_opts,
		OutputManager& output_mgr): 
  mpiManager(mpi_mgr), programOptions(prog_opts), outputManager(output_mgr),
  dakotaMPIComm(mpiManager.dakota_mpi_comm()),
  worldRank(mpiManager.world_rank()), worldSize(mpiManager.world_size()), 
  mpirunFlag(mpiManager.mpirun_flag()), dummyFlag(false), 
  outputTimings(programOptions.proceed_to_run()), startClock(0),
  currPLIter(parallelLevels.end()), currPCIter(parallelConfigurations.end())
{
  initialize_timers();
  init_mpi_comm();
}


/// shared function for initializing based on passed MPI_Comm
void ParallelLibrary::init_mpi_comm()
{
  // do not initialize MPI.  Get worldRank/worldSize if available
  ParallelLevel pl;
  String start_msg("Running Dakota executable.");
#ifdef DAKOTA_HAVE_MPI // mpi available
  if (mpirunFlag) {
    pl.serverIntraComm = dakotaMPIComm;
    pl.serverCommRank  = worldRank;
    pl.serverCommSize  = worldSize;

    startMPITime = MPI_Wtime();
  }
  else
    pl.serverIntraComm = MPI_COMM_NULL;
  
  if (worldSize > 1) {
    start_msg = "Running MPI Dakota executable in parallel on ";
    start_msg += boost::lexical_cast<std::string>(worldSize) + 
      " processors.";
  }
  else
    start_msg = "Running MPI Dakota executable in serial mode.";
#else // mpi not available
  if (mpirunFlag) {
    Cerr << "Error: Attempting to run serial executable in parallel."
	 << std::endl;
    abort_handler(-1);
  }
  else { // use defaults: worldRank = 0, worldSize = 1
    pl.serverIntraComm = MPI_COMM_NULL;
    start_msg = "Running serial Dakota executable in serial mode.";
  }
#endif // DAKOTA_HAVE_MPI
  outputManager.startup_message(start_msg);

  parallelLevels.push_back(pl);
  currPLIter = parallelLevels.begin();
  increment_parallel_configuration();
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


/** Split parent communicator into concurrent child server partitions
    as specified by the passed parameters.  This constructs new child
    intra-communicators and parent-child inter-communicators.  This
    function is called from the Strategy constructor for the concurrent
    iterator level and from ApplicationInterface::init_communicators()
    for the concurrent evaluation and concurrent analysis levels. */
void ParallelLibrary::
init_communicators(const ParallelLevel& parent_pl, int num_servers,
		   int procs_per_server, int max_concurrency, 
		   int asynch_local_concurrency, short default_config,
		   short scheduling_override, bool peer_dynamic_avail)
{
  ParallelLevel child_pl;
  child_pl.numServers     = num_servers;      // request/default to be updated
  child_pl.procsPerServer = procs_per_server; // request/default to be updated

  int capacity_multiplier = std::max(asynch_local_concurrency, 1);
  bool print_rank         = (parent_pl.serverCommRank == 0);

  // resolve_inputs selects master vs. peer scheduling
  child_pl.dedicatedMasterFlag = resolve_inputs(child_pl.numServers,
    child_pl.procsPerServer, parent_pl.serverCommSize, child_pl.procRemainder,
    max_concurrency, capacity_multiplier, default_config, scheduling_override,
    peer_dynamic_avail, print_rank);

  child_pl.commSplitFlag = (child_pl.dedicatedMasterFlag) ?
    split_communicator_dedicated_master(parent_pl, child_pl) :
    split_communicator_peer_partition(parent_pl,   child_pl);

  // update number of parallelism levels
  if ( child_pl.commSplitFlag && child_pl.numServers > 1 &&
       ( child_pl.procsPerServer > 1 || child_pl.procRemainder ) )
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
resolve_inputs(int& num_servers, int& procs_per_server, int avail_procs, 
               int& proc_remainder, int max_concurrency,
	       int capacity_multiplier, short default_config,
	       short scheduling_override, bool peer_dynamic_avail,
	       bool print_rank)
{
#ifdef MPI_DEBUG
  if (print_rank)
    Cout << "ParallelLibrary::resolve_inputs() called with num_servers = "
	 << num_servers << " procs_per_server = " << procs_per_server
	 << " avail_procs = " << avail_procs << " max_concurrency = "
	 << max_concurrency << " capacity_multiplier = " << capacity_multiplier
	 << " default_config = " << default_config << " scheduling_override = "
	 << scheduling_override << std::endl;
#endif

  const bool master_override = (scheduling_override == MASTER_SCHEDULING);
  const bool peer_override
    = (scheduling_override == PEER_SCHEDULING ||
       scheduling_override == PEER_DYNAMIC_SCHEDULING ||
       scheduling_override == PEER_STATIC_SCHEDULING);

  bool ded_master;
  if (avail_procs <= 1) {
    // ------------------------
    // insufficient avail_procs
    // ------------------------
    if (procs_per_server > 1 && print_rank) // TO DO: consider hard error
      Cerr << "\nWarning: not enough available processors to support "
           << procs_per_server << " procs_per_server.\n         "
	   << "Reducing to 1.\n\n";
    if (num_servers > 1 && print_rank) // TO DO: consider hard error
      Cerr << "\nWarning: not enough available processors to support " 
           << num_servers << " servers.\n         Reducing to 1.\n\n";
    num_servers = 1; procs_per_server = 1; ded_master = false; // peer partition
  }
  else if (num_servers > 0 && procs_per_server > 0) { // Data defaults are 0
    // ------------------------------------------
    // num_servers and procs_per_server specified
    // ------------------------------------------
    // in this case, we are tightly constrained to respecting both overrides

    int total_request = num_servers * procs_per_server;
    if (total_request == avail_procs) {
      ded_master = false;
      if (master_override && print_rank) // TO DO: consider hard error
	Cerr << "\nWarning: user selection of master scheduling cannot be "
	     << "supported in this partition.  Overriding to peer partition."
	     << "\n\n";
    }
    else if (total_request < avail_procs) {
      ded_master = !peer_override; // dedicate a master if no peer override
      if (ded_master) ++total_request;
      if (total_request < avail_procs && print_rank)
	Cerr << "\nWarning: user override of servers and partition size "
	     << "results in idle processors\n         (request: "
	     << total_request << " avail: " << avail_procs << " idle: "
	     << avail_procs - total_request <<")\n\n";
    }
    else { // hard error if insufficient avail_procs
      if (print_rank)
	Cerr << "\nError: insufficient available processors (" << avail_procs
	     << ") to support user overrides of servers (" << num_servers
	     << ")\n       and partition size (" << procs_per_server
	     << ").  Please adjust total allocation or overrides." << std::endl;
      abort_handler(-1);
    }
  }
  else if (num_servers > 0) { // Data default is 0: user request of 1 
                              // executes this block as a manual override
    // --------------------------
    // only num_servers specified
    // --------------------------
    // in this case, we honor the override, but have freedom to vary server
    // size in order to avoid idle processors

    if (num_servers > avail_procs) {
      if (print_rank)
	Cerr << "\nError: insufficient available processors (" << avail_procs
	     << ") to support user override of servers (" << num_servers
	     << ")\n.  Please adjust total allocation or overrides."<<std::endl;
      abort_handler(-1);
    }
    else if (num_servers == avail_procs) {
      ded_master = false;
      if (master_override && print_rank) // TO DO: consider hard error
	Cerr << "\nWarning: user selection of master scheduling cannot be "
	     << "supported in this partition.  Overriding to peer partition."
	     << "\n\n";
    }
    else if (master_override)
      ded_master = true;
    else if (peer_override || num_servers == 1 ||
	     max_concurrency <= num_servers * capacity_multiplier)
      ded_master = false;
    // peer dynamic requires pps = 1 with no remainder (else use remainder to
    // dedicate a master).  This turns out to be redundant of the check above.
    //else if (peer_dynamic_avail && num_servers == avail_procs)
    //  ded_master = false;
    else
      // since idle procs can be reallocated in this case, we don't need to be
      // too concerned about a ded master partition inducing addtl remainder
      ded_master = true;

    int server_procs = (ded_master) ? avail_procs - 1 : avail_procs;
    procs_per_server = server_procs / num_servers;
    proc_remainder   = server_procs % num_servers;
  }
  else if (procs_per_server > 0) { // Data default is 0: user request of 1
                                   // executes this block as a manual override
    // -------------------------------
    // only procs_per_server specified
    // -------------------------------
    // in this case, we have less freedom than the previous num_servers case
    // and strictly honoring the override may result in idle procs.  We could
    // consider a less strict policy, at least for higher parallelism levels
    // where excess could be repurposed without causing problems with analysis
    // decompositions, but for now we will treat a user override at any level
    // as gospel.  Also, peer_dynamic_avail is restricted to pps = 1.

    proc_remainder = 0; // all cases, no freedom to increment server sizes
    if (procs_per_server > avail_procs) {
      if (print_rank)
	Cerr << "\nError: insufficient available processors (" << avail_procs
	     << ") to support user override of procs_per_server ("
	     << procs_per_server << ")\n.  Please adjust total allocation or "
	     << "overrides." << std::endl;
      abort_handler(-1);
    }
    else if (procs_per_server == avail_procs) {
      ded_master = false;
      if (master_override && print_rank) // TO DO: consider hard error
	Cerr << "\nWarning: user selection of master scheduling cannot be "
	     << "supported in this partition.  Overriding to peer partition."
	     << "\n\n";
    }
    else if (master_override)
      ded_master = true;
    else if (peer_override)
      ded_master = false;
    else { // neither override: try peer, then master
      int peer_servers   = avail_procs / procs_per_server,
	  peer_remainder = avail_procs % procs_per_server;
      // Since we will not carry a proc_remainder, the logic below currently
      // dedicates a master if there are excess processors, even if not needed
      // to cover max_concurrency.  We also use special logic to avoid inducing
      // high idle proc counts due to selection of a master partition in the
      // case of excess concurrency.  For example:
      // > pps = 5, avail = 16: always dedicate master from remainder
      // > pps = 5, avail = 15: avoid inducing 4 idle due to master selection
      if (peer_remainder)
	ded_master = true; // if any remainder, dedicate 1 for scheduling
      else if ( peer_servers == 1 ||// avail_procs not enough for 2 full servers
		max_concurrency <= peer_servers * capacity_multiplier ||
		( peer_dynamic_avail && procs_per_server == 1 ) )
	ded_master = false;
      else {
	// num_servers could drop from 2 to 1 with one less processor, and 2
	// peer servers will always be better than 1 ded master server:
	// > we generally choose n ded master servers over n+1 peer servers for
	//   n>1 when max_concurrency indicates need for dynamic scheduling
	// > but we try to avoid bad cases, when master selection is wasteful
	//   due to significant increase in idle procs
	int master_servers   = (avail_procs - 1) / procs_per_server,
	    master_remainder = (avail_procs - 1) % procs_per_server;
	// We seek a simple heuristic that disallows wasteful master partitions:
	// > simple enforcement of !master_remainder is insufficient since this
	//   would eval to true for any pps > 1 (peer_remainder is 0; therefore,
	//   master_remainder = pps-1)
	// > enforcement that we don't waste some portion (half) of a server is
	//   also insufficiently general: master_remainder > pps / 2 is true
	//   for all pps > 2 (since master_remainder = pps-1).
	// > It appears we need to define wasteful more globally, e.g., if
	//   idle / available > some %.  In the example above, 4 idle / 15 avail
	//   increases idle percentage from 0% to 27% due to a master partition.
	// > A more complicated formula would need to estimate the value to be
	//   derived from dynamic scheduling in term of an excess concurrency
	//   factor (# of scheduling passes), expected job heterogeneity, etc.
	ded_master
	  = (master_servers > 1 && master_remainder <= avail_procs / 10);
      }
    }

    int server_procs = (ded_master) ? avail_procs - 1 : avail_procs;
    num_servers = server_procs / procs_per_server;
    if (server_procs % procs_per_server && print_rank)
      Cerr << "\nWarning: user override of partition size results in idle "
	   << "processors\n         (partition size request: "
	   << procs_per_server << " avail: " << server_procs << " idle: "
	   << server_procs - num_servers * procs_per_server << ")\n\n";
  }
  else {
    // --------------------------------------------------
    // neither num_servers nor procs_per_server specified -> auto config
    // --------------------------------------------------
    if (default_config == PUSH_DOWN) { // default for this parallelism level
      // is concurrency pushed down.  default_config taking precedence over a
      // scheduling_override request makes sense from an efficiency standpoint,
      // but may be neglecting a user request in lieu of a built-in default.
      // Output a warning if neglecting a user request:
      if (master_override && print_rank)
        Cerr << "\nWarning: default_config takes precendence over a master "
	     << "scheduling request\n         when neither num_servers nor "
	     << "procs_per_server is specified.\n\n";
      // TO DO: neglects available concurrency at lower levels (would
      // require two passes through parallelism hierarchy to resolve)
      procs_per_server = avail_procs;
      num_servers = 1;
      ded_master = false; // tau_i = 1 -> static scheduling
    }
    else { // concurrency pushed up
      // round up loaded servers but avoid std::ceil((Real)numer/(Real)denom)
      int loaded_servers = max_concurrency / capacity_multiplier;
      if (max_concurrency % capacity_multiplier) ++loaded_servers;

      if (master_override) // master partition (dynamic scheduling)
	ded_master = true;
      else if (peer_override || loaded_servers <= avail_procs)
        ded_master = false; // tau_i = n_ij_max -> static scheduling

      // We have excess concurrency (loaded_servers > avail_procs yields
      // num_servers = server_procs, procs_per_server = 1, remainder = 0):
      // use peer dynamic if available, then master if num_servers > 1,
      // then revert to peer (static)
      else if (peer_dynamic_avail)
	ded_master = false;
      else // ded_master if num_servers > 1 in calculation to follow
	ded_master = (avail_procs > 2);// && loaded_servers > 1);

      // As for the num_servers specification case, we don't need to be as
      // concerned with selection of a wasteful master partition (increased
      // idleness), since any partition remainder gets reallocated.

      int server_procs = (ded_master) ? avail_procs - 1 : avail_procs;
      num_servers = std::min(server_procs, loaded_servers);
      procs_per_server = server_procs / num_servers;
      proc_remainder   = server_procs % num_servers;
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

  // special case for which we assign child = parent without partitioning
  if (parent_pl.serverId > parent_pl.numServers) { // parent is idle partition
    inherit_as_server_comm(parent_pl, child_pl);
    child_pl.serverMasterFlag = (parent_pl.serverCommRank == 0);
    child_pl.serverId = child_pl.numServers + 1; // trip at next level as well
    return false; // set split flag to false in calling routine
  }

#ifndef COMM_SPLIT_TO_SINGLE
  // In some direct interfacing cases, the simulation requires its own comm
  // even if the comm is single processor.  In this case, the code block below
  // is bypassed and the additional comm split overhead is incurred.
  if (child_pl.procsPerServer == 1 && !child_pl.procRemainder) {//1-proc servers
    inherit_as_hub_server_comm(parent_pl, child_pl);
    child_pl.messagePass = true;
    child_pl.serverMasterFlag = (parent_pl.serverCommRank) ? true : false;
    child_pl.serverId = parent_pl.serverCommRank;// 0 = master, 1/2/... = slaves
    return false; // set split flag to false in calling routine
  }
#endif

  // ------------------------------------------------------
  // Split parent Comm to create new intra- and inter-comms
  // ------------------------------------------------------

  IntArray start_rank(child_pl.numServers);
  int color = 0; // reassigned unless master proc.
  // addtl_procs manages case where procRemainder > numServers that can occur
  // for large procsPerServer --> ensures that proc_rem_cntr < numServers
  int i, color_cntr = 1, end_rank = 0,
    addtl_procs   = child_pl.procRemainder / child_pl.numServers, // truncated
    proc_rem_cntr = child_pl.procRemainder - addtl_procs*child_pl.numServers;
  for (i=0; i<child_pl.numServers; ++i) {
    start_rank[i] = end_rank + 1;
    end_rank = start_rank[i] + child_pl.procsPerServer + addtl_procs - 1;
    if (proc_rem_cntr > 0)
      { ++end_rank; --proc_rem_cntr; }
    if (parent_pl.serverCommRank >= start_rank[i] &&
	parent_pl.serverCommRank <= end_rank)
      color = color_cntr;
#ifdef MPI_DEBUG
    if (parent_pl.serverCommRank == 0)
      Cout << "group " << i << " has processors " << start_rank[i]
	   << " through " << end_rank << " with color = " << color_cntr << '\n';
#endif // MPI_DEBUG
    ++color_cntr;
  }

  // address any idle processors not included in child_pl.procRemainder
  // (resolve_inputs() must honor procs_per_server and will not augment with
  // remainders in this case).  These processors get a color assignment for
  // purposes of the split, but are not part of a child server.
  if (parent_pl.serverCommRank > end_rank)
    color = color_cntr;
  if (end_rank+1 < parent_pl.serverCommSize)
    child_pl.idlePartition = true;

  // verify that all processors except master have a color
  if (parent_pl.serverCommRank && !color) {
    Cerr << "\nError: slave processor " << parent_pl.serverCommRank 
         << " missing color assignment" << std::endl;
    abort_handler(-1);
  }

  // special case for which we assign child = parent without partitioning
  if (child_pl.numServers < 1) { // no check on idlePartition for ded master
    inherit_as_server_comm(parent_pl, child_pl);
    child_pl.serverMasterFlag = (parent_pl.serverCommRank == 0);
    child_pl.serverId = 0;
    return false; // set split flag to false in calling routine
  }

  child_pl.messagePass = true;
#ifdef DAKOTA_HAVE_MPI
  MPI_Comm_split(parent_pl.serverIntraComm, color, parent_pl.serverCommRank,
		 &child_pl.serverIntraComm);
  MPI_Comm_rank(child_pl.serverIntraComm, &child_pl.serverCommRank);
  MPI_Comm_size(child_pl.serverIntraComm, &child_pl.serverCommSize);
  child_pl.serverId = color; // master = 0, slaves = 1/2/3/.../n, idle = n+1

  // Create intercommunicators.  All processors in both intracommunicators 
  // (child_pl.serverIntraComm for master and slaves) must participate in call
  // with matching tags (tag = color on slave side and = i+1 on master side).
  // See example on p. 252 of MPI: The Complete Reference.
  if (parent_pl.serverCommRank == 0) {
    int num_hs_ic = child_pl.numServers;
    if (child_pl.idlePartition) ++num_hs_ic;
    child_pl.hubServerInterComms = new MPI_Comm [num_hs_ic];
    for (i=0; i<child_pl.numServers; ++i)
      MPI_Intercomm_create(child_pl.serverIntraComm, 0,
			   parent_pl.serverIntraComm, start_rank[i], i+1,
			   &child_pl.hubServerInterComms[i]);
    if (child_pl.idlePartition)
      MPI_Intercomm_create(child_pl.serverIntraComm, 0,
			   parent_pl.serverIntraComm, end_rank+1, i+1,
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
  // Exclude any idle partition.
  color = (child_pl.serverCommRank == 0 &&
	   child_pl.serverId <= child_pl.numServers) ? 1 : 0;
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
  // child_pl.serverIntraComm == parent_pl.serverIntraComm.

  // special case for which we assign child = parent without partitioning
  if (parent_pl.serverId > parent_pl.numServers) { // parent is idle partition
    inherit_as_server_comm(parent_pl, child_pl);
    child_pl.serverMasterFlag = (parent_pl.serverCommRank == 0);
    child_pl.serverId = child_pl.numServers + 1; // trip at next level as well
    return false; // Set split flag to false in calling routine
  }

#ifndef COMM_SPLIT_TO_SINGLE
  // In some direct interfacing cases, the simulation requires its own comm
  // even if the comm is single processor.  In this case, the code block below
  // is bypassed and the additional comm split overhead is incurred.
  if (child_pl.procsPerServer == 1 && !child_pl.procRemainder) {// 1-proc. peers
    inherit_as_hub_server_comm(parent_pl, child_pl);
    child_pl.messagePass = child_pl.serverMasterFlag = true;
    child_pl.serverId = parent_pl.serverCommRank+1; // peer id's = 1/2/3/.../n
    return false; // Set split flag to false in calling routine
  }
#endif

  // -----------------------------------------------------------
  // Split parent Comm to create new peer intra- and inter-comms
  // -----------------------------------------------------------

  IntArray start_rank(child_pl.numServers);
  int color = 0; // reassigned for all procs. in peer case
  // addtl_procs manages case where procRemainder > numServers that can occur
  // for large procsPerServer --> ensures that proc_rem_cntr < numServers
  int i, color_cntr = 1, end_rank = -1,
    addtl_procs   = child_pl.procRemainder / child_pl.numServers, // truncated
    proc_rem_cntr = child_pl.procRemainder - addtl_procs*child_pl.numServers;
  for (i=0; i<child_pl.numServers; ++i) {
    start_rank[i] = end_rank + 1;
    end_rank = start_rank[i] + child_pl.procsPerServer + addtl_procs - 1;
    // don't assign remainder to first peer, since this interferes with possible
    // usage of dynamic peer scheduling (requires procs_per_server == 1)
    if (proc_rem_cntr > 0 && i)
      { ++end_rank; --proc_rem_cntr; }
    if (parent_pl.serverCommRank >= start_rank[i] &&
	parent_pl.serverCommRank <= end_rank)
      color = color_cntr;
#ifdef MPI_DEBUG
    if (parent_pl.serverCommRank == 0)
      Cout << "group " << i << " has processors " << start_rank[i] 
           << " through " << end_rank << " with color = " << color_cntr << '\n';
#endif // MPI_DEBUG
    ++color_cntr;
  }

  // address any idle processors not included in child_pl.procRemainder
  // (resolve_inputs() must honor procs_per_server and will not augment with
  // remainders in this case).  These processors get a color assignment for
  // purposes of the split, but are not part of a child server.
  if (parent_pl.serverCommRank > end_rank)
    color = color_cntr;
  if (end_rank+1 < parent_pl.serverCommSize)
    child_pl.idlePartition = true;

  // verify that all processors except master have a color
  if (!color) {
    Cerr << "\nError: processor " << parent_pl.serverCommRank
	 << " missing color assignment" << std::endl;
    abort_handler(-1);
  }

  // special case for which we assign child = parent without partitioning
  if (child_pl.numServers < 2 && !child_pl.idlePartition) { // 1 peer, no idle
    inherit_as_server_comm(parent_pl, child_pl);
    child_pl.serverMasterFlag = (parent_pl.serverCommRank == 0);
    child_pl.serverId = 1; // one server, peer id = 1
    return false; // Set split flag to false in calling routine
  }

  child_pl.messagePass = true;
#ifdef DAKOTA_HAVE_MPI
  MPI_Comm_split(parent_pl.serverIntraComm, color, parent_pl.serverCommRank,
		 &child_pl.serverIntraComm);
  MPI_Comm_rank(child_pl.serverIntraComm, &child_pl.serverCommRank);
  MPI_Comm_size(child_pl.serverIntraComm, &child_pl.serverCommSize);
  child_pl.serverId = color; // peer id's = 1/2/3/.../n, idle id = n+1

  // Create intercommunicators.  Current implementation is very similar to
  // master-slave in that only the 1st server has an array of intercomms.  This
  // reflects the current ApplicationInterface::static_schedule_message_passing
  // design.  A more general implementation would give each peer an array of 
  // intercomms, and could easily be supported in the future if needed.
  if (child_pl.serverId == 1) {
    int num_hs_ic = child_pl.numServers - 1;
    if (child_pl.idlePartition) ++num_hs_ic;
    child_pl.hubServerInterComms = new MPI_Comm [num_hs_ic];
    for (i=0; i<child_pl.numServers-1; ++i)
      MPI_Intercomm_create(child_pl.serverIntraComm, 0,
			   parent_pl.serverIntraComm, start_rank[i+1], i+2,
			   &child_pl.hubServerInterComms[i]);
    if (child_pl.idlePartition)
      MPI_Intercomm_create(child_pl.serverIntraComm, 0,
			   parent_pl.serverIntraComm, end_rank+1, i+2,
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
  // Exclude any idle partition.
  color = (child_pl.serverCommRank == 0 &&
	   child_pl.serverId <= child_pl.numServers) ? 1 : 0;
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
  // stratDedicatedMasterFlag in IteratorScheduler::self_schedule_iterators().

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
	 << "Level\t\t\tnum_servers    procs_per_server    partition\n"
	 << "-----\t\t\t-----------    ----------------    ---------\n"
	 << "concurrent iterators\t  " << std::setw(4)
         << si_pl.numServers << "\t\t   " << std::setw(4)
	 << si_pl.procsPerServer << "\t\t   ";
    if (si_pl.dedicatedMasterFlag) Cout << "ded. master\n";
    else                           Cout << "peer\n";

    // Iterator diagnostics
    Cout << "concurrent evaluations\t  " << std::setw(4) << num_eval_srv
	 << "\t\t   " << std::setw(4) << p_per_eval << "\t\t   ";
    if (iterator_ded_master_flag)  Cout << "ded. master\n";
    else                           Cout << "peer\n";

    // Evaluation diagnostics
    Cout << "concurrent analyses\t  " << std::setw(4) << num_anal_srv
	 << "\t\t   " << std::setw(4) << p_per_anal << "\t\t   ";
    if (eval_ded_master_flag)      Cout << "ded. master\n";
    else                           Cout << "peer\n";

    // Analysis diagnostics
    Cout << "multiprocessor analysis\t  " << std::setw(4) << p_per_anal
         << "\t\t     N/A\t   N/A\n\nTotal parallelism levels =   " 
         << par_levels << "\n-------------------------------------------------"
	 << "----------------------------" << std::endl;
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
void ParallelLibrary::manage_outputs_restart(const ParallelLevel& pl)
{
  // Design rationale: this class should manage parallel-related
  // issues including broadcast, delegating filename management and
  // redirection to OutputManager (with help from ProgramOptions).

  // The incoming pl should be the lowest of the concurrent iterator levels

  // If not rank 0 within an iteratorComm, then no output handling to
  // manage.  Note that a strategy dedicated master must participate
  // in the broadcasts, but returns thereafter since it does not
  // redirect its output, read from restart files, etc.
  if (pl.serverCommRank > 0)
    return;

  // Synchronize necessary data from worldRank 0 to iterator masters
  bool stdout_redirect_required;
  if (worldRank == 0) {

    // If iterator servers are in use, then always segregate the std
    // output.  However, for std error, assume that this should remain
    // directed to the screen unless an explicit "-e" command line
    // option has been given.
    stdout_redirect_required = (pl.numServers > 1 || pl.dedicatedMasterFlag);

    // All in the server comm need the data to manage the files
    if (pl.hubServerCommSize > 1) {
      MPIPackBuffer send_buffer;
      send_buffer << stdout_redirect_required
		  << programOptions
		  << outputManager.tabularDataFlag 
		  << outputManager.tabularDataFile
		  << outputManager.resultsOutputFlag 
		  << outputManager.resultsOutputFile;
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
    recv_buffer >> stdout_redirect_required 
		>> programOptions
		>> outputManager.tabularDataFlag 
		>> outputManager.tabularDataFile
		>> outputManager.resultsOutputFlag 
		>> outputManager.resultsOutputFile;
  }

  // After this communication, we can rely on programOptions and
  // certain outputManager fields in the following.  We do not
  // broadcast the whole OutputManager as it may be managing open file
  // streams and the state would be hard to preserve.  (May later
  // encapsulate the broadcase there though.)

  // This returns a strategy dedicated master processor, if present.
  if (!pl.serverMasterFlag)
    return;

  // Tag cout/cerr/read_restart/write_restart for case of concurrent iterators
  String ctr_tag;
  if (pl.numServers > 1 || pl.dedicatedMasterFlag) {
    // could change to numServers>0 since it would still be nice to organize
    // the output for 1 server in BranchBndStrategy/ConcurrentStrategy
    ctr_tag += "." + boost::lexical_cast<std::string>(pl.serverId);
  }
  outputManager.file_tag(ctr_tag);

  // Now that all iterator masters have the output filename settings
  // and local tags have been added, initialize output streams,
  // restart, databases, etc., for each iterator master.  Note that
  // the opening of files on processors for which there is no output
  // is avoided.
  outputManager.redirect_cout(programOptions, stdout_redirect_required);
  outputManager.redirect_cerr(programOptions);
  outputManager.init_resultsdb(programOptions);
  outputManager.init_restart(programOptions);
}


void ParallelLibrary::write_restart(const ParamResponsePair& prp)
{
  // delegate restart write to outputManager
  outputManager.append_restart(prp);
}


/** Close streams associated with manage_outputs and manage_restart
    and terminate any additional services that may be active. */
void ParallelLibrary::terminate_modelcenter()
{
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


void ParallelLibrary::abort_helper(int code) {
  
  outputManager.close_streams();

  // Abort the process(es)
#ifdef DAKOTA_HAVE_MPI
#ifdef HAVE_AIX_MPI
  // AIX POE utility function for signal handling w/MPI to exit normally.
  pm_child_sig_handler(code, NULL, NULL);
#endif
  // BMA TODO: more graceful handling (though MPIManager) for library customers?
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


void ParallelLibrary::free_communicators(ParallelLevel& pl)
{
#ifdef DAKOTA_HAVE_MPI
  if (pl.commSplitFlag) { // deallocate intra/inter comms.
    MPI_Comm_free(&pl.serverIntraComm);
    MPI_Comm_free(&pl.hubServerIntraComm);
    if (pl.dedicatedMasterFlag) { // master-slave interComms
      if (pl.serverId == 0) { // if dedicated master
	int i;
        for(i=0; i<pl.numServers; ++i) 
          MPI_Comm_free(&pl.hubServerInterComms[i]);
	if (pl.idlePartition) // trailing server of idle processors
          MPI_Comm_free(&pl.hubServerInterComms[i]);
        delete [] pl.hubServerInterComms;
      }
      else // servers 1 through n
        MPI_Comm_free(&pl.hubServerInterComm);
    }
    else { // peer interComms
      if (pl.serverId == 1) { // 1st peer
	int i;
        for(i=0; i<pl.numServers-1; ++i) 
          MPI_Comm_free(&pl.hubServerInterComms[i]);
	if (pl.idlePartition) // trailing server of idle processors
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
  if (!dummyFlag) { // protect some finalizations in case of dummy_lib

    // Output timings before closing streams, in case of redirection
    output_timers();

    // BMA TODO: may need this depending on destructor order
    // Close the output and restart streams
    //outputManager.close_streams();

    // close any other services
    terminate_modelcenter();

    // No longer protecting MPI_Finalize for dummy_lib, as MPIManager tracks 

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
