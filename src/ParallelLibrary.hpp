/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ParallelLibrary
//- Description:  Encapsulates management of parallel message passing libraries
//-               such as MPI.
//-
//- Owner:        Mike Eldred
//- Version: $Id: ParallelLibrary.hpp 6731 2010-04-16 03:42:16Z wjbohnh $

#ifndef PARALLEL_LIBRARY_H
#define PARALLEL_LIBRARY_H

#include "dakota_system_defs.hpp"
#include "dakota_global_defs.hpp"  // for Cout
#include "dakota_data_types.hpp"
#include "MPIPackBuffer.hpp"
#include "MPIManager.hpp"
#include "ProgramOptions.hpp"
#include "OutputManager.hpp"

namespace Dakota {

/// Container class for the data associated with a single level of 
/// communicator partitioning.

/** A list of these levels is contained in ParallelLibrary
    (ParallelLibrary::parallelLevels), which defines all of the
    parallelism levels across one or more multilevel parallelism
    configurations. */

class ParallelLevel
{
  //
  //- Heading: Friends
  //

  /// the ParallelLibrary class has special access priveleges in order to
  /// streamline implementation
  friend class ParallelLibrary;

public:

  //
  //- Heading: Constructors, destructor, and operators
  //

  ParallelLevel();                        ///< default constructor
  ParallelLevel(const ParallelLevel& pl); ///< copy constructor
  ~ParallelLevel();                       ///< destructor

  /// assignment operator
  ParallelLevel& operator=(const ParallelLevel& pl);

  //
  //- Heading: Member functions
  //

  bool dedicated_master() const;               ///< return dedicatedMasterFlag
  bool communicator_split() const;             ///< return commSplitFlag
  bool server_master() const;                  ///< return serverMasterFlag
  bool message_pass() const;                   ///< return messagePass
  bool idle_partition() const;                 ///< return idlePartition
  int num_servers() const;                     ///< return numServers
  int processors_per_server() const;           ///< return procsPerServer
  int processor_remainder() const;             ///< return procRemainder
  const MPI_Comm& server_intra_communicator() const; ///< return serverIntraComm
  int server_communicator_rank() const;        ///< return serverCommRank
  int server_communicator_size() const;        ///< return serverCommSize
  const MPI_Comm& hub_server_intra_communicator() const;
                                               ///< return hubServerIntraComm
  int hub_server_communicator_rank() const;    ///< return hubServerCommRank
  int hub_server_communicator_size() const;    ///< return hubServerCommSize
  const MPI_Comm& hub_server_inter_communicator() const;
                                               ///< return hubServerInterComm
  MPI_Comm* hub_server_inter_communicators() const;
                                               ///< return hubServerInterComms
  int server_id() const;                       ///< return serverId

private:

  //
  //- Heading: Convenience functions
  //

  /// assign the attributes of the incoming pl to this object
  void assign(const ParallelLevel& pl);

  //
  //- Heading: Data
  //

  bool dedicatedMasterFlag;      ///< signals dedicated master partitioning
  bool commSplitFlag;            ///< signals a communicator split was used
  bool serverMasterFlag;         ///< identifies master server processors
  bool messagePass;              ///< flag for message passing at this level
  bool idlePartition;            ///< identifies presence of an idle processor
                                 ///< partition at this level
  int numServers;                ///< number of servers
  int procsPerServer;            ///< processors per server
  int procRemainder;             ///< proc remainder after equal distribution
  MPI_Comm serverIntraComm;      ///< intracomm. for each server partition
  int serverCommRank;            ///< rank in serverIntraComm
  int serverCommSize;            ///< size of serverIntraComm
  MPI_Comm hubServerIntraComm;   ///< intracomm for all serverCommRank==0
                                 ///< w/i next higher level serverIntraComm
  int hubServerCommRank;         ///< rank in hubServerIntraComm
  int hubServerCommSize;         ///< size of hubServerIntraComm
  MPI_Comm hubServerInterComm;   ///< intercomm. between a server & the hub  
                                 ///< (on server partitions only)
  MPI_Comm* hubServerInterComms; ///< intercomm. array on hub processor
  int serverId;                  ///< server identifier
};


inline ParallelLevel::ParallelLevel(): dedicatedMasterFlag(false),
  commSplitFlag(false), serverMasterFlag(false), messagePass(false),
  idlePartition(false), numServers(0), procsPerServer(0), procRemainder(0),
  serverIntraComm(MPI_COMM_NULL), serverCommRank(0), serverCommSize(1),
  hubServerIntraComm(MPI_COMM_NULL), hubServerCommRank(0), hubServerCommSize(1),
  hubServerInterComm(MPI_COMM_NULL), hubServerInterComms(NULL), serverId(0)
{ }

inline ParallelLevel::~ParallelLevel()
{ }

inline void ParallelLevel::assign(const ParallelLevel& pl)
{
  dedicatedMasterFlag = pl.dedicatedMasterFlag;
  commSplitFlag   = pl.commSplitFlag;   serverMasterFlag = pl.serverMasterFlag;
  messagePass     = pl.messagePass;     idlePartition    = pl.idlePartition;
  numServers      = pl.numServers;      procsPerServer   = pl.procsPerServer;
  procRemainder   = pl.procRemainder;   serverIntraComm  = pl.serverIntraComm;
  serverCommRank  = pl.serverCommRank;  serverCommSize   = pl.serverCommSize;
  hubServerIntraComm  = pl.hubServerIntraComm;
  hubServerCommRank   = pl.hubServerCommRank;
  hubServerCommSize   = pl.hubServerCommSize; 
  hubServerInterComm  = pl.hubServerInterComm;
  hubServerInterComms = pl.hubServerInterComms; serverId = pl.serverId;
}

inline ParallelLevel::ParallelLevel(const ParallelLevel& pl)
{ assign(pl); }

inline ParallelLevel& ParallelLevel::operator=(const ParallelLevel& pl)
{ assign(pl); return *this; }

inline bool ParallelLevel::dedicated_master() const
{ return dedicatedMasterFlag; }

inline bool ParallelLevel::communicator_split() const
{ return commSplitFlag; }

inline bool ParallelLevel::server_master() const
{ return serverMasterFlag; }

inline bool ParallelLevel::message_pass() const
{ return messagePass; }

inline bool ParallelLevel::idle_partition() const
{ return idlePartition; }

inline int ParallelLevel::num_servers() const
{ return numServers; }

inline int ParallelLevel::processors_per_server() const
{ return procsPerServer; }

inline int ParallelLevel::processor_remainder() const
{ return procRemainder; }

inline const MPI_Comm& ParallelLevel::server_intra_communicator() const
{ return serverIntraComm; }

inline int ParallelLevel::server_communicator_rank() const
{ return serverCommRank; }

inline int ParallelLevel::server_communicator_size() const
{ return serverCommSize; }

inline const MPI_Comm& ParallelLevel::hub_server_intra_communicator() const
{ return hubServerIntraComm; }

inline int ParallelLevel::hub_server_communicator_rank() const
{ return hubServerCommRank; }

inline int ParallelLevel::hub_server_communicator_size() const
{ return hubServerCommSize; }

inline const MPI_Comm& ParallelLevel::hub_server_inter_communicator() const
{ return hubServerInterComm; }

inline MPI_Comm* ParallelLevel::hub_server_inter_communicators() const
{ return hubServerInterComms; }

inline int ParallelLevel::server_id() const
{ return serverId; }


/// Container class for a set of ParallelLevel list iterators that
/// collectively identify a particular multilevel parallel configuration.

/** Rather than containing the multilevel parallel configuration directly,
    ParallelConfiguration instead provides a set of list iterators which
    point into a combined list of ParallelLevels.  This approach allows
    different configurations to reuse ParallelLevels without copying them.
    A list of ParallelConfigurations is contained in ParallelLibrary
    (ParallelLibrary::parallelConfigurations). */

class ParallelConfiguration
{
  //
  //- Heading: Friends
  //

  /// the ParallelLibrary class has special access priveleges in order to
  /// streamline implementation
  friend class ParallelLibrary;

public:

  //
  //- Heading: Constructors, destructor, and operators
  //

  /// default constructor
  ParallelConfiguration();
  /// copy constructor
  ParallelConfiguration(const ParallelConfiguration& pl);
  /// destructor
  ~ParallelConfiguration();

  /// assignment operator
  ParallelConfiguration& operator=(const ParallelConfiguration& pl);

  //
  //- Heading: Member functions
  //

  /// return the ParallelLevel corresponding to wPLIter
  const ParallelLevel&  w_parallel_level() const;
  /// return the ParallelLevel corresponding to siPLIter
  const ParallelLevel& si_parallel_level() const;
  /// return the ParallelLevel corresponding to iePLIter
  const ParallelLevel& ie_parallel_level() const;
  /// return the ParallelLevel corresponding to eaPLIter
  const ParallelLevel& ea_parallel_level() const;

private:

  //
  //- Heading: Convenience functions
  //

  /// assign the attributes of the incoming pl to this object
  void assign(const ParallelConfiguration& pl);

  //
  //- Heading: Data
  //

  short numParallelLevels; ///< number of parallel levels

  /// list iterator for MPI_COMM_WORLD (not strictly required, but
  /// improves modularity by avoiding explicit usage of MPI_COMM_WORLD)
  ParLevLIter wPLIter;

  /// list iterator for concurrent iterator partitions
  /// (there may be more than one per parallel configuration instance)
  ParLevLIter siPLIter;
  //std::list<ParLevLIter> siPLIters;

  /// list iterator identifying the iterator-evaluation parallelLevel 
  /// (there can only be one)
  ParLevLIter iePLIter;

  /// list iterator identifying the evaluation-analysis parallelLevel 
  /// (there can only be one)
  ParLevLIter eaPLIter;
};


inline ParallelConfiguration::ParallelConfiguration(): numParallelLevels(0)
  //wPLIter(NULL), siPLIter(NULL), iePLIter(NULL), eaPLIter(NULL)
{ }

inline ParallelConfiguration::~ParallelConfiguration()
{ }

inline void ParallelConfiguration::assign(const ParallelConfiguration& pc)
{
  numParallelLevels = pc.numParallelLevels;
  wPLIter  = pc.wPLIter;
  siPLIter = pc.siPLIter;
  iePLIter = pc.iePLIter;
  eaPLIter = pc.eaPLIter;
}

inline ParallelConfiguration::
ParallelConfiguration(const ParallelConfiguration& pc)
{ assign(pc); }

inline ParallelConfiguration& ParallelConfiguration::
operator=(const ParallelConfiguration& pc)
{ assign(pc); return *this; }

inline const ParallelLevel& ParallelConfiguration::w_parallel_level()  const
{ return *wPLIter; }

inline const ParallelLevel& ParallelConfiguration::si_parallel_level() const
{ return *siPLIter; }

inline const ParallelLevel& ParallelConfiguration::ie_parallel_level() const
{ return *iePLIter; }

inline const ParallelLevel& ParallelConfiguration::ea_parallel_level() const
{ return *eaPLIter; }


/// Class for partitioning multiple levels of parallelism and managing
/// message passing within these levels.

/** The ParallelLibrary class encapsulates all of the details of
    performing message passing within multiple levels of parallelism.
    It provides functions for partitioning of levels according to user
    configuration input and functions for passing messages within and
    across MPI communicators for each of the parallelism levels.  If
    support for other message-passing libraries beyond MPI becomes
    needed (PVM, ...), then ParallelLibrary would be promoted to a
    base class with virtual functions to encapsulate the
    library-specific syntax. */

class ParallelLibrary
{
public:

  //
  //- Heading: Constructors and Destructor
  //

  /// default constructor (used for dummy_lib)
  ParallelLibrary();
  /// stand-alone and default library mode constructor; don't require options
  //  ParallelLibrary(const ProgramOptions& prog_opts = ProgramOptions());
  /// library mode constructor accepting communicator
  ParallelLibrary(const MPIManager& mpi_mgr, ProgramOptions& prog_opts,
		  OutputManager& output_mgr);
  /// destructor
  ~ParallelLibrary();

  //
  //- Heading: Member functions
  //

  /// split MPI_COMM_WORLD into iterator communicators
  const ParallelLevel& init_iterator_communicators(int iterator_servers,
    int procs_per_iterator, int min_procs_per_iterator,
    int max_procs_per_iterator, int max_iterator_concurrency,
    short default_config, short iterator_scheduling, bool peer_dynamic_avail);

  /// split an iterator communicator into evaluation communicators
  const ParallelLevel& init_evaluation_communicators(int evaluation_servers,
    int procs_per_evaluation, int min_procs_per_eval, int max_procs_per_eval,
    int max_evaluation_concurrency, int asynch_local_evaluation_concurrency,
    short default_config, short evaluation_scheduling, bool peer_dynamic_avail);

  /// split an evaluation communicator into analysis communicators
  const ParallelLevel& init_analysis_communicators(int analysis_servers,
    int procs_per_analysis, int min_procs_per_analysis,
    int max_procs_per_analysis, int max_analysis_concurrency,
    int asynch_local_analysis_concurrency, short default_config,
    short analysis_scheduling, bool peer_dynamic_avail);

  /// deallocate iterator communicators
  void free_iterator_communicators();
  /// deallocate evaluation communicators
  void free_evaluation_communicators();
  /// deallocate analysis communicators
  void free_analysis_communicators();

  /// print the parallel level settings for a particular parallel configuration
  void print_configuration();

  /// manage output streams and restart file(s) (both modes)
  void manage_outputs_restart(const ParallelLevel& pl);

  /// write a parameter/response set to the restart file
  void write_restart(const ParamResponsePair& prp);

  /// return programOptions reference
  ProgramOptions& program_options();
  /// return outputManager reference
  OutputManager& output_manager();

  /// terminate ModelCenter if running
  void terminate_modelcenter();

  /// finalize MPI with correct communicator for abort
  void abort_helper(int code);

  // Functions to get run_mode data
  // BMA TODO: consider moving all to ProgramOptions, possibly with
  // delegation from here.
  bool command_line_check() const;      ///< return checkFlag
  bool command_line_pre_run() const;    ///< return preRunFlag
  bool command_line_run() const;        ///< return runFlag
  bool command_line_post_run() const;   ///< return postRunFlag
  bool command_line_user_modes() const; ///< return userModesFlag
  const String& command_line_pre_run_input() const;   ///< preRunInput filename
  const String& command_line_pre_run_output() const;  ///< preRunOutput filename
  const String& command_line_run_input() const;       ///< runInput filename
  const String& command_line_run_output() const;      ///< runOutput filename
  const String& command_line_post_run_input() const;  ///< postRunInput filename
  const String& command_line_post_run_output() const; ///< postRunOutput fname

  /// blocking send at the strategy-iterator communication level
  void  send_si(int& send_int, int dest, int tag);
  /// blocking receive at the strategy-iterator communication level
  void  recv_si(int& recv_int, int source, int tag, MPI_Status& status);

  /// blocking send at the strategy-iterator communication level
  void  send_si(MPIPackBuffer& send_buff, int dest, int tag);
  /// nonblocking send at the strategy-iterator communication level
  void isend_si(MPIPackBuffer& send_buff, int dest, int tag, 
		MPI_Request& send_req);
  /// blocking receive at the strategy-iterator communication level
  void  recv_si(MPIUnpackBuffer& recv_buff, int source, int tag, 
		MPI_Status& status);
  /// nonblocking receive at the strategy-iterator communication level
  void irecv_si(MPIUnpackBuffer& recv_buff, int source, int tag, 
		MPI_Request& recv_req);

  /// blocking send at the iterator-evaluation communication level
  void  send_ie(MPIPackBuffer& send_buff, int dest, int tag);
  /// nonblocking send at the iterator-evaluation communication level
  void isend_ie(MPIPackBuffer& send_buff, int dest, int tag, 
		MPI_Request& send_req);
  /// blocking receive at the iterator-evaluation communication level
  void  recv_ie(MPIUnpackBuffer& recv_buff, int source, int tag, 
		MPI_Status& status);
  /// nonblocking receive at the iterator-evaluation communication level
  void irecv_ie(MPIUnpackBuffer& recv_buff, int source, int tag, 
		MPI_Request& recv_req);

  /// blocking send at the evaluation-analysis communication level
  void  send_ea(int& send_int, int dest, int tag);
  /// nonblocking send at the evaluation-analysis communication level
  void isend_ea(int& send_int, int dest, int tag, MPI_Request& send_req);
  /// blocking receive at the evaluation-analysis communication level
  void  recv_ea(int& recv_int, int source, int tag, MPI_Status& status);
  /// nonblocking receive at the evaluation-analysis communication level
  void irecv_ea(int& recv_int, int source, int tag, MPI_Request& recv_req);

  /// broadcast an integer across the serverIntraComm of a ParallelLevel
  void bcast(int& data, const ParallelLevel& pl);
  /// broadcast an integer across MPI_COMM_WORLD
  void bcast_w(int& data);
  /// broadcast an integer across an iterator communicator
  void bcast_i(int& data);
  /// broadcast a short integer across an iterator communicator
  void bcast_i(short& data);
  /// broadcast an integer across an evaluation communicator
  void bcast_e(int& data);
  /// broadcast an integer across an analysis communicator
  void bcast_a(int& data);
  /// broadcast an integer across a strategy-iterator intra communicator
  void bcast_si(int& data);
  /// broadcast a packed buffer across MPI_COMM_WORLD
  void bcast_w(MPIPackBuffer& send_buff);
  /// broadcast a packed buffer across an iterator communicator
  void bcast_i(MPIPackBuffer& send_buff);
  /// broadcast a packed buffer across an evaluation communicator
  void bcast_e(MPIPackBuffer& send_buff);
  /// broadcast a packed buffer across an analysis communicator
  void bcast_a(MPIPackBuffer& send_buff);
  /// broadcast a packed buffer across a strategy-iterator intra communicator
  void bcast_si(MPIPackBuffer& send_buff);
  /// matching receive for packed buffer broadcast across MPI_COMM_WORLD
  void bcast_w(MPIUnpackBuffer& recv_buff);
  /// matching receive for packed buffer bcast across an iterator communicator
  void bcast_i(MPIUnpackBuffer& recv_buff);
  /// matching receive for packed buffer bcast across an evaluation communicator
  void bcast_e(MPIUnpackBuffer& recv_buff);
  /// matching receive for packed buffer bcast across an analysis communicator
  void bcast_a(MPIUnpackBuffer& recv_buff);
  /// matching recv for packed buffer bcast across a strat-iterator intra comm
  void bcast_si(MPIUnpackBuffer& recv_buff);

  /// enforce MPI_Barrier on MPI_COMM_WORLD
  void barrier_w();
  /// enforce MPI_Barrier on an iterator communicator
  void barrier_i();
  /// enforce MPI_Barrier on an evaluation communicator
  void barrier_e();
  /// enforce MPI_Barrier on an analysis communicator
  void barrier_a();

  /// compute a sum over an eval-analysis intra-communicator using MPI_Reduce
  void reduce_sum_ea(double* local_vals, double* sum_vals, int num_vals);
  /// compute a sum over an analysis communicator using MPI_Reduce
  void reduce_sum_a(double* local_vals, double* sum_vals, int num_vals);

  /// test a nonblocking send/receive request for completion
  void test(MPI_Request& request, int& test_flag, MPI_Status& status);

  /// wait for a nonblocking send/receive request to complete
  void wait(MPI_Request& request, MPI_Status& status);
  /// wait for all messages from a series of nonblocking receives
  void waitall(int num_recvs, MPI_Request*& recv_reqs);
  /// wait for at least one message from a series of nonblocking receives
  /// but complete all that are available
  void waitsome(int num_sends, MPI_Request*& recv_requests,
		int& num_recvs, int*& index_array, MPI_Status*& status_array);

  /// free an MPI_Request
  void free(MPI_Request& request);

  //
  //- Heading: Set/Inquire functions
  //

  // TODO: rename to reflect dakotaMPIComm
  int world_size() const;     ///< return Dakota's worldSize
  int world_rank() const;     ///< return Dakota's worldRank
  bool mpirun_flag() const;   ///< return mpirunFlag
  bool is_null() const;       ///< return dummyFlag
  Real parallel_time() const; ///< returns current MPI wall clock time

  /// set the current ParallelConfiguration node
  void parallel_configuration_iterator(const ParConfigLIter& pc_iter);
  /// return the current ParallelConfiguration node
  const ParConfigLIter& parallel_configuration_iterator() const;
  /// return the current ParallelConfiguration instance
  const ParallelConfiguration& parallel_configuration() const;

  /// returns the number of entries in parallelConfigurations
  size_t num_parallel_configurations() const;
  /// identifies if the current ParallelConfiguration has been fully populated
  bool parallel_configuration_is_complete();
  /// add a new node to parallelConfigurations and increment currPCIter
  void increment_parallel_configuration();
  // decrement currPCIter
  //void decrement_parallel_configuration();
  /// test current parallel configuration for definition of world
  /// parallel level
  bool  w_parallel_level_defined() const;
  /// test current parallel configuration for definition of
  /// strategy-iterator parallel level
  bool si_parallel_level_defined() const;
  /// test current parallel configuration for definition of
  /// iterator-evaluation parallel level
  bool ie_parallel_level_defined() const;
  /// test current parallel configuration for definition of
  /// evaluation-analysis parallel level
  bool ea_parallel_level_defined() const;

  /// return the set of analysis intra communicators for all parallel
  /// configurations (used for setting up direct simulation interfaces
  /// prior to execution time).
  std::vector<MPI_Comm> analysis_intra_communicators();

private:

  //
  //- Heading: Convenience member functions
  //

  /// convenience function for initializing DAKOTA's top-level MPI
  /// communicators, based on dakotaMPIComm
  void init_mpi_comm();

  /// initialize DAKOTA and UTILIB timers
  void initialize_timers();

  /// conditionally output timers in destructor
  void output_timers();

  /// split a parent communicator into child server communicators
  void init_communicators(const ParallelLevel& parent_pl, int num_servers,
    int procs_per_server, int min_procs_per_server, int max_procs_per_server,
    int max_concurrency, int asynch_local_concurrency, short default_config,
    short scheduling_override, bool peer_dynamic_avail);

  /// deallocate intra/inter communicators for a particular ParallelLevel
  void free_communicators(ParallelLevel& pl);

  /// split a parent communicator into a dedicated master processor
  /// and num_servers child communicators
  void split_communicator_dedicated_master(const ParallelLevel& parent_pl,
					   ParallelLevel& child_pl);

  /// split a parent communicator into num_servers peer child
  /// communicators (no dedicated master processor)
  void split_communicator_peer_partition(const ParallelLevel& parent_pl,
					 ParallelLevel& child_pl);

  /// resolve user inputs into a sensible partitioning scheme
  void resolve_inputs(ParallelLevel& child_pl, int avail_procs,
		      int min_procs_per_server, int max_procs_per_server, 
		      int max_concurrency, int capacity_multiplier,
		      short default_config, short scheduling_override,
		      bool peer_dynamic_avail, bool print_rank);

  /// blocking buffer send at the current communication level
  void  send(MPIPackBuffer& send_buff, int dest, int tag,
	     ParallelLevel& parent_pl, ParallelLevel& child_pl);
  /// blocking integer send at the current communication level
  void  send(int& send_int, int dest, int tag,
	     ParallelLevel& parent_pl, ParallelLevel& child_pl);

  /// nonblocking buffer send at the current communication level
  void isend(MPIPackBuffer& send_buff, int dest, int tag,
	     MPI_Request& send_req, ParallelLevel& parent_pl,
	     ParallelLevel& child_pl);
  /// nonblocking integer send at the current communication level
  void isend(int& send_int, int dest, int tag,
	     MPI_Request& send_req, ParallelLevel& parent_pl,
	     ParallelLevel& child_pl);

  /// blocking buffer receive at the current communication level
  void  recv(MPIUnpackBuffer& recv_buff, int source, int tag,
	     MPI_Status& status, ParallelLevel& parent_pl,
	     ParallelLevel& child_pl);
  /// blocking integer receive at the current communication level
  void  recv(int& recv_int, int source, int tag,
	     MPI_Status& status, ParallelLevel& parent_pl,
	     ParallelLevel& child_pl);

  /// nonblocking buffer receive at the current communication level
  void irecv(MPIUnpackBuffer& recv_buff, int source, int tag,
	     MPI_Request& recv_req, ParallelLevel& parent_pl,
	     ParallelLevel& child_pl);
  /// nonblocking integer receive at the current communication level
  void irecv(int& recv_int, int source, int tag,
	     MPI_Request& recv_req, ParallelLevel& parent_pl,
	     ParallelLevel& child_pl);

  /// broadcast an integer across a communicator
  void bcast(int& data, const MPI_Comm& comm);
  /// broadcast a short integer across a communicator
  void bcast(short& data, const MPI_Comm& comm);
  /// send a packed buffer across a communicator using a broadcast
  void bcast(MPIPackBuffer& send_buff, const MPI_Comm& comm);
  /// matching receive for a packed buffer broadcast
  void bcast(MPIUnpackBuffer& recv_buff, const MPI_Comm& comm);

  /// enforce MPI_Barrier on comm
  void barrier(const MPI_Comm& comm);

  /// compute a sum over comm using MPI_Reduce
  void reduce_sum(double* local_vals, double* sum_vals, int num_vals,
		  const MPI_Comm& comm);

  /// check the MPI return code and abort if error
  void check_error(const String& err_source, int err_code);

  /// convenience function for updating child serverIntraComm from
  /// parent serverIntraComm
  void inherit_as_server_comm(const ParallelLevel& parent_pl,
			      ParallelLevel& child_pl);
  /// convenience function for updating child hubServerIntraComm from
  /// parent serverIntraComm
  void inherit_as_hub_server_comm(const ParallelLevel& parent_pl,
				  ParallelLevel& child_pl);

  //
  //- Heading: Data
  //
  
  const MPIManager& mpiManager;
  /// programOptions is non-const due to updates from broadcast
  ProgramOptions& programOptions;
  /// Non-const output handler to help with file redirection
  OutputManager& outputManager;

  // TODO: Update rank, size, etc. names to reflect DAKOTA's top-level MPI_Comm
  MPI_Comm dakotaMPIComm; ///< MPI_Comm on which DAKOTA is running
  int  worldRank;     ///< rank in MPI_Comm in which DAKOTA is running
  int  worldSize;     ///< size of MPI_Comm in which DAKOTA is running
  bool mpirunFlag;    ///< flag for a parallel mpirun/yod launch

  bool dummyFlag;     ///< prevents multiple MPI_Finalize calls due to dummy_lib

  bool outputTimings; ///< timing info only beyond help/version/check
  Real startCPUTime;  ///< start reference for UTILIB CPU timer
  Real startWCTime;   ///< start reference for UTILIB wall clock timer
  Real startMPITime;  ///< start reference for MPI wall clock timer
  long startClock;    ///< start reference for local clock() timer measuring
                      ///< parent+child CPU

  /// the complete set of parallelism levels for managing multilevel
  /// parallelism among one or more configurations
  std::list<ParallelLevel> parallelLevels;

  /// the set of parallel configurations which manage list iterators for
  /// indexing into parallelLevels
  std::list<ParallelConfiguration> parallelConfigurations;

  /// list iterator identifying the current node in parallelLevels
  ParLevLIter currPLIter;

  /// list iterator identifying the current node in parallelConfigurations
  ParConfigLIter currPCIter;
};


inline ProgramOptions& ParallelLibrary::program_options()
{ return programOptions; }


inline OutputManager& ParallelLibrary::output_manager()
{ return outputManager; }


inline int ParallelLibrary::world_rank() const
{ return worldRank; }


inline int ParallelLibrary::world_size() const
{ return worldSize; }


inline bool ParallelLibrary::mpirun_flag() const
{ return mpirunFlag; }


inline bool ParallelLibrary::is_null() const
{ return dummyFlag; }


inline Real ParallelLibrary::parallel_time() const
{
#ifdef DAKOTA_HAVE_MPI
  return (mpirunFlag) ? MPI_Wtime() - startMPITime : 0.;
#else
  return 0.;
#endif // DAKOTA_HAVE_MPI
}


inline void ParallelLibrary::
parallel_configuration_iterator(const ParConfigLIter& pc_iter)
{ currPCIter = pc_iter; }


inline const ParConfigLIter& ParallelLibrary::
parallel_configuration_iterator() const
{ return currPCIter; }


inline const ParallelConfiguration& ParallelLibrary::
parallel_configuration() const
{ return *currPCIter; }


inline size_t ParallelLibrary::num_parallel_configurations() const
{ return parallelConfigurations.size(); }


inline bool ParallelLibrary::parallel_configuration_is_complete()
{
  // All processors invoke init_iterator_comms
  // All strategy procs except ded master invoke init_eval_comms
  // All iterator procs except ded master invoke init_analysis_comms
  if ( currPCIter->siPLIter == parallelLevels.end() )
    return false; // PC incomplete if si level undefined
  else { // si level defined
    const ParallelLevel& si_pl = currPCIter->si_parallel_level();
    if (si_pl.dedicatedMasterFlag && worldRank == 0)
      return true; // PC complete at si level for strategy ded master
    else if ( currPCIter->iePLIter == parallelLevels.end() )
      return false; // PC incomplete for other procs if ie level undefined
    else { // ie level defined
      const ParallelLevel& ie_pl = currPCIter->ie_parallel_level();
      if (ie_pl.dedicatedMasterFlag && si_pl.serverCommRank == 0)
	return true;  // PC complete at ie level for iterator ded master
      else if ( currPCIter->eaPLIter == parallelLevels.end() )
	return false; // PC incomplete for other procs if ea level undefined
      else // ea level defined
	return true;  // PC complete
    }
  }
}


/** Called from the ParallelLibrary ctor and from Model::init_communicators().
    An increment is performed for each Model initialization except the first
    (which inherits the world and strategy-iterator parallel levels from the
    first partial configuration). */
inline void ParallelLibrary::increment_parallel_configuration()
{
  // The world level is set in the ParallelLib ctor, the si level is
  // defined in the Strategy ctor, and the ie and ea levels are defined in
  // ApplicationInterface::init_communicators().  Any undefined iterators
  // are initialized to their "singular values" (NULL should not be used).
  ParallelConfiguration pc;

  // Approach 1 does not hard-wire pc.siPLIter and relies on assignment in
  // init_evaluation_communicators():
  //pc.wPLIter  = parallelLevels.begin();
  //pc.siPLIter = pc.iePLIter = pc.eaPLIter = parallelLevels.end();

  // Approach 2 is more bullet proof, but also less flexible:
  ParLevLIter pl_iter = parallelLevels.begin();
  pc.wPLIter  =   pl_iter;
  // In the first call from the ParallelLibrary ctor, this sets the siPLIter
  // to parallelLevels.end() as there's only one ParallelLevel in the list:
  pc.siPLIter = ++pl_iter;
  // Inherit parallelism level count from si_pl partitioning
  pc.numParallelLevels
    = (pl_iter != parallelLevels.end() && pl_iter->messagePass) ? 1 : 0;
  // ie and ea levels to be defined by Model::init_communicators()
  pc.iePLIter = pc.eaPLIter = parallelLevels.end();

  parallelConfigurations.push_back(pc);
  currPCIter = --parallelConfigurations.end();
}


//inline void ParallelLibrary::decrement_parallel_configuration()
//{ --currPCIter; }


inline bool ParallelLibrary::w_parallel_level_defined() const
{
  return ( currPCIter != parallelConfigurations.end() &&
	   currPCIter->wPLIter != parallelLevels.end() );
}


inline bool ParallelLibrary::si_parallel_level_defined() const
{
  return ( currPCIter != parallelConfigurations.end() &&
	   currPCIter->siPLIter != parallelLevels.end() );
}


inline bool ParallelLibrary::ie_parallel_level_defined() const
{
  return ( currPCIter != parallelConfigurations.end() &&
	   currPCIter->iePLIter != parallelLevels.end() );
}


inline bool ParallelLibrary::ea_parallel_level_defined() const
{
  return ( currPCIter != parallelConfigurations.end() &&
	   currPCIter->eaPLIter != parallelLevels.end() );
}


inline std::vector<MPI_Comm> ParallelLibrary::analysis_intra_communicators()
{
  size_t num_pc = parallelConfigurations.size();
  ParConfigLIter pc_iter = parallelConfigurations.begin();
  std::vector<MPI_Comm> analysis_intra_comms(num_pc);
  for (size_t i=0; i<num_pc; i++, pc_iter++)
    analysis_intra_comms[i]
      = pc_iter->ea_parallel_level().server_intra_communicator();
  return analysis_intra_comms;
}


inline const ParallelLevel& ParallelLibrary::
init_iterator_communicators(int iterator_servers, int procs_per_iterator,
			    int min_procs_per_iterator,
			    int max_procs_per_iterator,
			    int max_iterator_concurrency, short default_config,
			    short iterator_scheduling, bool peer_dynamic_avail)
{
  int asynch_local_iterator_concurrency = 0;
  init_communicators(*parallelLevels.begin(), iterator_servers,
		     procs_per_iterator, min_procs_per_iterator,
		     max_procs_per_iterator, max_iterator_concurrency,
		     asynch_local_iterator_concurrency, default_config,
		     iterator_scheduling, peer_dynamic_avail);
  currPCIter->siPLIter = currPLIter;
  return *currPLIter;
}


inline const ParallelLevel& ParallelLibrary::
init_evaluation_communicators(int evaluation_servers, int procs_per_evaluation,
			      int min_procs_per_eval, int max_procs_per_eval,
			      int max_evaluation_concurrency,
			      int asynch_local_evaluation_concurrency,
			      short default_config, short evaluation_scheduling,
			      bool peer_dynamic_avail)
{
  /*
  // handle case where there is a new parallel configuration instance, but
  // init_iterator_communicators has not been called again.
  if ( parallelLevels.end() == currPCIter->siPLIter ) {
    if (parallelConfigurations.size() > 1) {
      // if used, then this needs to be replaced with a while loop,
      // since the valid siPLIter could be several PC's back
      ParConfigLIter prev_pc_iter = currPCIter; prev_pc_iter--;
      currPCIter->siPLIter = prev_pc_iter->siPLIter;
      if ( currPCIter->siPLIter->communicator_split() )
        ++currPCIter->numParallelLevels;
    }
    else {
      Cerr << "Error: init_evaluation_communicators() called without preceding "
	   << "init_iterator_communicators() call." << std::endl;
      abort_handler(-1);
    }
  }
  */
  init_communicators(*currPCIter->siPLIter, evaluation_servers,
		     procs_per_evaluation, min_procs_per_eval,
		     max_procs_per_eval, max_evaluation_concurrency,
		     asynch_local_evaluation_concurrency, default_config,
		     evaluation_scheduling, peer_dynamic_avail);
  currPCIter->iePLIter = currPLIter;
  return *currPLIter;
}


inline const ParallelLevel& ParallelLibrary::
init_analysis_communicators(int analysis_servers, int procs_per_analysis,
			    int min_procs_per_analysis,
			    int max_procs_per_analysis,
			    int max_analysis_concurrency,
			    int asynch_local_analysis_concurrency,
			    short default_config, short analysis_scheduling,
			    bool peer_dynamic_avail)
{
  init_communicators(*currPCIter->iePLIter, analysis_servers,
		     procs_per_analysis, min_procs_per_analysis,
		     max_procs_per_analysis, max_analysis_concurrency,
		     asynch_local_analysis_concurrency, default_config,
		     analysis_scheduling, peer_dynamic_avail);
  currPCIter->eaPLIter = currPLIter;
  return *currPLIter;
}


inline void ParallelLibrary::free_iterator_communicators()
{ free_communicators(*currPCIter->siPLIter); }


inline void ParallelLibrary::free_evaluation_communicators()
{ free_communicators(*currPCIter->iePLIter); }


inline void ParallelLibrary::free_analysis_communicators()
{ free_communicators(*currPCIter->eaPLIter); }


inline bool ParallelLibrary::command_line_check() const
{ return programOptions.check(); }


inline bool ParallelLibrary::command_line_pre_run() const
{ return programOptions.pre_run(); }


inline bool ParallelLibrary::command_line_run() const
{ return programOptions.run(); }


inline bool ParallelLibrary::command_line_post_run() const
{ return programOptions.post_run(); }


inline bool ParallelLibrary::command_line_user_modes() const
{ return programOptions.user_modes(); }


inline const String& ParallelLibrary::command_line_pre_run_input() const
{ return programOptions.pre_run_input(); }


inline const String& ParallelLibrary::command_line_pre_run_output() const
{ return programOptions.pre_run_output(); }


inline const String& ParallelLibrary::command_line_run_input() const
{ return programOptions.run_input(); }


inline const String& ParallelLibrary::command_line_run_output() const
{ return programOptions.run_output(); }


inline const String& ParallelLibrary::command_line_post_run_input() const
{ return programOptions.post_run_input(); }


inline const String& ParallelLibrary::command_line_post_run_output() const
{ return programOptions.post_run_output(); }


// ---------------------------
// Begin MPI wrapper functions
// ---------------------------
// Notes:
// > values that are inputs (e.g., dest, source, tag) may be passed by value
// > values that are outputs must be passed by ref/ptr/ref-to-ptr
// > ref-to-ptr is used when practical for passing pointers to arrays in the
//   unlikely event that an MPI function modifies the pointer (e.g., allocates
//   new memory) in addition to modifying what the pointer points to.
// > ref-by-ptr cannot be used in cases where a single value can be passed by
//   address (e.g., double*& sum_vals cannot be used in the MPI_Reduce wrappers
//   due to its use for reducing a single value in DirectApplicInterface).
inline void ParallelLibrary::
check_error(const String& err_source, int err_code)
{
  // NOTE: for error code meanings, see mpi/include/mpi_errno.h
  if (err_code) {
    Cerr << "Error: " << err_source << " returns with error code " << err_code
	 << " on processor " << worldRank << std::endl;
    abort_handler(-1);
  }
}


inline void ParallelLibrary::
send(MPIPackBuffer& send_buff, int dest, int tag,
     ParallelLevel& parent_pl, ParallelLevel& child_pl)
{
#ifdef DAKOTA_HAVE_MPI
  int err_code = 0;
  if (child_pl.commSplitFlag)
    err_code = (parent_pl.serverCommRank) ?
      MPI_Send((void*)send_buff.buf(), send_buff.size(), MPI_PACKED, 0, tag,
	       child_pl.hubServerInterComm) : // slave/peers 2 through n
      MPI_Send((void*)send_buff.buf(), send_buff.size(), MPI_PACKED, 0, tag,
	       child_pl.hubServerInterComms[dest-1]); // master/peer 1
  else
    err_code = MPI_Send((void*)send_buff.buf(), send_buff.size(), MPI_PACKED,
			dest, tag, parent_pl.serverIntraComm);
  check_error("MPI_Send(MPIPackBuffer)", err_code);
#endif // DAKOTA_HAVE_MPI
}


inline void ParallelLibrary::
send(int& send_int, int dest, int tag, ParallelLevel& parent_pl,
     ParallelLevel& child_pl)
{
#ifdef DAKOTA_HAVE_MPI
  int err_code = 0;
  if (child_pl.commSplitFlag)
    err_code = (parent_pl.serverCommRank) ?
      MPI_Send((void*)&send_int, 1, MPI_INT, 0, tag,
	       child_pl.hubServerInterComm) : // slaves/peers 2 -> n
      MPI_Send((void*)&send_int, 1, MPI_INT, 0, tag,
	       child_pl.hubServerInterComms[dest-1]); // master/peer 1
  else
    err_code = MPI_Send((void*)&send_int, 1, MPI_INT, dest, tag,
			parent_pl.serverIntraComm);
  check_error("MPI_Send(int)", err_code);
#endif // DAKOTA_HAVE_MPI
}


inline void ParallelLibrary::send_si(int& send_int, int dest, int tag)
{ send(send_int, dest, tag, *parallelLevels.begin(), *currPCIter->siPLIter); }


inline void ParallelLibrary::
send_si(MPIPackBuffer& send_buff, int dest, int tag)
{ send(send_buff, dest, tag, *parallelLevels.begin(), *currPCIter->siPLIter);}


inline void ParallelLibrary::
send_ie(MPIPackBuffer& send_buff, int dest, int tag)
{ send(send_buff, dest, tag, *currPCIter->siPLIter, *currPCIter->iePLIter);}


inline void ParallelLibrary::send_ea(int& send_int, int dest, int tag)
{ send(send_int, dest, tag, *currPCIter->iePLIter, *currPCIter->eaPLIter); }


inline void ParallelLibrary::
isend(MPIPackBuffer& send_buff, int dest, int tag,
      MPI_Request& send_req, ParallelLevel& parent_pl, ParallelLevel& child_pl)
{
#ifdef DAKOTA_HAVE_MPI
  int err_code = 0;
  if (child_pl.commSplitFlag)
    err_code = (parent_pl.serverCommRank) ?
      MPI_Isend((void*)send_buff.buf(), send_buff.size(), MPI_PACKED, 0, tag,
		child_pl.hubServerInterComm, &send_req) : // slave/peers 2 -> n
      MPI_Isend((void*)send_buff.buf(), send_buff.size(), MPI_PACKED, 0, tag,
		child_pl.hubServerInterComms[dest-1], &send_req);//master/peer 1
  else
    err_code = MPI_Isend((void*)send_buff.buf(), send_buff.size(), MPI_PACKED,
			 dest, tag, parent_pl.serverIntraComm, &send_req);
  check_error("MPI_Isend(MPIPackBuffer)", err_code);
#endif // DAKOTA_HAVE_MPI
}


inline void ParallelLibrary::
isend(int& send_int, int dest, int tag, MPI_Request& send_req,
      ParallelLevel& parent_pl, ParallelLevel& child_pl)
{
#ifdef DAKOTA_HAVE_MPI
  int err_code = 0;
  if (child_pl.commSplitFlag)
    err_code = (parent_pl.serverCommRank) ?
      MPI_Isend((void*)&send_int, 1, MPI_INT, 0, tag,
		child_pl.hubServerInterComm, &send_req) : // slaves/peers 2 -> n
      MPI_Isend((void*)&send_int, 1, MPI_INT, 0, tag,
		child_pl.hubServerInterComms[dest-1], &send_req);//master/peer 1
  else
    err_code = MPI_Isend((void*)&send_int, 1, MPI_INT, dest, tag,
			 parent_pl.serverIntraComm, &send_req);
  check_error("MPI_Isend(int)", err_code);
#endif // DAKOTA_HAVE_MPI
}


inline void ParallelLibrary::
isend_si(MPIPackBuffer& send_buff, int dest, int tag, MPI_Request& send_req)
{
  isend(send_buff, dest, tag, send_req, *parallelLevels.begin(),
	*currPCIter->siPLIter);
}


inline void ParallelLibrary::
isend_ie(MPIPackBuffer& send_buff, int dest, int tag, MPI_Request& send_req)
{
  isend(send_buff, dest, tag, send_req, *currPCIter->siPLIter,
	*currPCIter->iePLIter);
}


inline void ParallelLibrary::
isend_ea(int& send_int, int dest, int tag, MPI_Request& send_req)
{
  isend(send_int, dest, tag, send_req, *currPCIter->iePLIter,
	*currPCIter->eaPLIter);
}


inline void ParallelLibrary::
recv(MPIUnpackBuffer& recv_buff, int source, int tag,
     MPI_Status& status, ParallelLevel& parent_pl, ParallelLevel& child_pl)
{
#ifdef DAKOTA_HAVE_MPI
  int err_code = 0;
  if (child_pl.commSplitFlag)
    err_code = (parent_pl.serverCommRank) ?
      MPI_Recv((void*)recv_buff.buf(), recv_buff.size(), MPI_PACKED, 0, tag,
	       child_pl.hubServerInterComm, &status) : // slave/peers 2 -> n
      MPI_Recv((void*)recv_buff.buf(), recv_buff.size(), MPI_PACKED, 0, tag, 
	       child_pl.hubServerInterComms[source-1], &status);// master/peer 1
  else
    err_code = MPI_Recv((void*)recv_buff.buf(), recv_buff.size(), MPI_PACKED,
			source, tag, parent_pl.serverIntraComm, &status);
  check_error("MPI_Recv(MPIUnpackBuffer)", err_code);
#endif // DAKOTA_HAVE_MPI
}


inline void ParallelLibrary::
recv(int& recv_int, int source, int tag, MPI_Status& status,
     ParallelLevel& parent_pl, ParallelLevel& child_pl)
{
#ifdef DAKOTA_HAVE_MPI
  int err_code = 0;
  if (child_pl.commSplitFlag)
    err_code = (parent_pl.serverCommRank) ?
      MPI_Recv((void*)&recv_int, 1, MPI_INT, 0, tag, // slaves/peers 2 -> n
	       child_pl.hubServerInterComm, &status) :
      MPI_Recv((void*)&recv_int, 1, MPI_INT, 0, tag, // master/peer 1
	       child_pl.hubServerInterComms[source-1], &status);
  else
    err_code = MPI_Recv((void*)&recv_int, 1, MPI_INT, source, tag, 
                        parent_pl.serverIntraComm, &status);
  check_error("MPI_Recv(int)", err_code);
#endif // DAKOTA_HAVE_MPI
}


inline void ParallelLibrary::
recv_si(int& recv_int, int source, int tag, MPI_Status& status)
{
  recv(recv_int, source, tag, status, *parallelLevels.begin(),
       *currPCIter->siPLIter);
}


inline void ParallelLibrary::
recv_si(MPIUnpackBuffer& recv_buff, int source, int tag, MPI_Status& status)
{
  recv(recv_buff, source, tag, status, *parallelLevels.begin(),
       *currPCIter->siPLIter);
}


inline void ParallelLibrary::
recv_ie(MPIUnpackBuffer& recv_buff, int source, int tag, MPI_Status& status)
{
  recv(recv_buff, source, tag, status, *currPCIter->siPLIter,
       *currPCIter->iePLIter);
}


inline void ParallelLibrary::
recv_ea(int& recv_int, int source, int tag, MPI_Status& status)
{
  recv(recv_int, source, tag, status, *currPCIter->iePLIter,
       *currPCIter->eaPLIter);
}


inline void ParallelLibrary::
irecv(MPIUnpackBuffer& recv_buff, int source, int tag,
      MPI_Request& recv_req, ParallelLevel& parent_pl, ParallelLevel& child_pl)
{
#ifdef DAKOTA_HAVE_MPI
  int err_code = 0;
  if (child_pl.commSplitFlag)
    err_code = (parent_pl.serverCommRank) ?
      MPI_Irecv((void*)recv_buff.buf(), recv_buff.size(), MPI_PACKED, 0, tag,
		child_pl.hubServerInterComm, &recv_req) : // slave/peers 2 -> n
      MPI_Irecv((void*)recv_buff.buf(), recv_buff.size(), MPI_PACKED, 0, tag, 
		child_pl.hubServerInterComms[source-1], &recv_req);//mast/peer 1
  else
    err_code = MPI_Irecv((void*)recv_buff.buf(), recv_buff.size(), MPI_PACKED,
			 source, tag, parent_pl.serverIntraComm, &recv_req);
  check_error("MPI_Irecv(MPIUnpackBuffer)", err_code);
#endif // DAKOTA_HAVE_MPI
}


inline void ParallelLibrary::
irecv(int& recv_int, int source, int tag, MPI_Request& recv_req,
      ParallelLevel& parent_pl, ParallelLevel& child_pl)
{
#ifdef DAKOTA_HAVE_MPI
  int err_code = 0;
  if (child_pl.commSplitFlag)
    err_code = (parent_pl.serverCommRank) ? 
      MPI_Irecv((void*)&recv_int, 1, MPI_INT, 0, tag, // slaves/peers 2 -> n
		child_pl.hubServerInterComm, &recv_req) :
      MPI_Irecv((void*)&recv_int, 1, MPI_INT, 0, tag, // master/peer 1
		child_pl.hubServerInterComms[source-1], &recv_req);
  else
    err_code = MPI_Irecv((void*)&recv_int, 1, MPI_INT, source, tag, 
                         parent_pl.serverIntraComm, &recv_req);
  check_error("MPI_Irecv(int)", err_code);
#endif // DAKOTA_HAVE_MPI
}


inline void ParallelLibrary::
irecv_si(MPIUnpackBuffer& recv_buff, int source, int tag, MPI_Request& recv_req)
{
  irecv(recv_buff, source, tag, recv_req, *parallelLevels.begin(),
	*currPCIter->siPLIter);
}


inline void ParallelLibrary::
irecv_ie(MPIUnpackBuffer& recv_buff, int source, int tag, MPI_Request& recv_req)
{
  irecv(recv_buff, source, tag, recv_req, *currPCIter->siPLIter,
	*currPCIter->iePLIter);
}


inline void ParallelLibrary::
irecv_ea(int& recv_int, int source, int tag, MPI_Request& recv_req)
{
  irecv(recv_int, source, tag, recv_req, *currPCIter->iePLIter,
	*currPCIter->eaPLIter);
}


// bcast and waitall convenience functions can be used for any level
inline void ParallelLibrary::bcast(int& data, const MPI_Comm& comm)
{
#ifdef DAKOTA_HAVE_MPI
  int err_code = MPI_Bcast(&data, 1, MPI_INT, 0, comm);
  check_error("MPI_Bcast(int)", err_code);
#endif // DAKOTA_HAVE_MPI
}


inline void ParallelLibrary::bcast(short& data, const MPI_Comm& comm)
{
#ifdef DAKOTA_HAVE_MPI
  int err_code = MPI_Bcast(&data, 1, MPI_SHORT, 0, comm);
  check_error("MPI_Bcast(short)", err_code);
#endif // DAKOTA_HAVE_MPI
}


inline void ParallelLibrary::bcast(int& data, const ParallelLevel& pl)
{ bcast(data, pl.serverIntraComm); }


inline void ParallelLibrary::bcast_w(int& data)
{ bcast(data, currPCIter->wPLIter->serverIntraComm); }


inline void ParallelLibrary::bcast_i(int& data)
{ bcast(data, currPCIter->siPLIter->serverIntraComm); }


inline void ParallelLibrary::bcast_i(short& data)
{ bcast(data, currPCIter->siPLIter->serverIntraComm); }


inline void ParallelLibrary::bcast_e(int& data)
{ bcast(data, currPCIter->iePLIter->serverIntraComm); }


inline void ParallelLibrary::bcast_a(int& data)
{ bcast(data, currPCIter->eaPLIter->serverIntraComm); }


inline void ParallelLibrary::bcast_si(int& data)
{ bcast(data, currPCIter->siPLIter->hubServerIntraComm); }


inline void ParallelLibrary::
bcast(MPIPackBuffer& send_buff, const MPI_Comm& comm)
{
#ifdef DAKOTA_HAVE_MPI
  int err_code
    = MPI_Bcast((void*)send_buff.buf(), send_buff.size(), MPI_PACKED, 0, comm);
  check_error("MPI_Bcast(MPIPackBuffer)", err_code);
#endif // DAKOTA_HAVE_MPI
}


inline void ParallelLibrary::bcast_w(MPIPackBuffer& send_buff)
{ bcast(send_buff, currPCIter->wPLIter->serverIntraComm); }


inline void ParallelLibrary::bcast_i(MPIPackBuffer& send_buff)
{ bcast(send_buff, currPCIter->siPLIter->serverIntraComm); }


inline void ParallelLibrary::bcast_e(MPIPackBuffer& send_buff)
{ bcast(send_buff, currPCIter->iePLIter->serverIntraComm); }


inline void ParallelLibrary::bcast_a(MPIPackBuffer& send_buff)
{ bcast(send_buff, currPCIter->eaPLIter->serverIntraComm); }


inline void ParallelLibrary::bcast_si(MPIPackBuffer& send_buff)
{ bcast(send_buff, currPCIter->siPLIter->hubServerIntraComm); }


inline void ParallelLibrary::
bcast(MPIUnpackBuffer& recv_buff, const MPI_Comm& comm)
{
#ifdef DAKOTA_HAVE_MPI
  int err_code
    = MPI_Bcast((void*)recv_buff.buf(), recv_buff.size(), MPI_PACKED, 0, comm);
  check_error("MPI_Bcast(MPIUnpackBuffer)", err_code);
#endif // DAKOTA_HAVE_MPI
}


inline void ParallelLibrary::bcast_w(MPIUnpackBuffer& recv_buff)
{ bcast(recv_buff, currPCIter->wPLIter->serverIntraComm); }


inline void ParallelLibrary::bcast_i(MPIUnpackBuffer& recv_buff)
{ bcast(recv_buff, currPCIter->siPLIter->serverIntraComm); }


inline void ParallelLibrary::bcast_e(MPIUnpackBuffer& recv_buff)
{ bcast(recv_buff, currPCIter->iePLIter->serverIntraComm); }


inline void ParallelLibrary::bcast_a(MPIUnpackBuffer& recv_buff)
{ bcast(recv_buff, currPCIter->eaPLIter->serverIntraComm); }


inline void ParallelLibrary::bcast_si(MPIUnpackBuffer& recv_buff)
{ bcast(recv_buff, currPCIter->siPLIter->hubServerIntraComm); }


inline void ParallelLibrary::barrier(const MPI_Comm& comm)
{
#ifdef DAKOTA_HAVE_MPI
  int err_code = MPI_Barrier(comm);
  check_error("MPI_Barrier", err_code);
#endif // DAKOTA_HAVE_MPI
}


inline void ParallelLibrary::barrier_w()
{ barrier(currPCIter->wPLIter->serverIntraComm); }


inline void ParallelLibrary::barrier_i()
{ barrier(currPCIter->siPLIter->serverIntraComm); }


inline void ParallelLibrary::barrier_e()
{ barrier(currPCIter->iePLIter->serverIntraComm); }


inline void ParallelLibrary::barrier_a()
{ barrier(currPCIter->eaPLIter->serverIntraComm); }


inline void ParallelLibrary::
reduce_sum(double* local_vals, double* sum_vals, int num_vals,
	   const MPI_Comm& comm)
{
#ifdef DAKOTA_HAVE_MPI
  int err_code
    = MPI_Reduce(local_vals, sum_vals, num_vals, MPI_DOUBLE, MPI_SUM, 0, comm);
  check_error("MPI_Reduce", err_code);
#endif // DAKOTA_HAVE_MPI
}


inline void ParallelLibrary::
reduce_sum_ea(double* local_vals, double* sum_vals, int num_vals)
{
  reduce_sum(local_vals, sum_vals, num_vals,
	     currPCIter->eaPLIter->hubServerIntraComm);
}


inline void ParallelLibrary::
reduce_sum_a(double* local_vals, double* sum_vals, int num_vals)
{
  reduce_sum(local_vals, sum_vals, num_vals,
	     currPCIter->eaPLIter->serverIntraComm);
}


inline void ParallelLibrary::
test(MPI_Request& request, int& test_flag, MPI_Status& status)
{
#ifdef DAKOTA_HAVE_MPI
  int err_code = MPI_Test(&request, &test_flag, &status);
  check_error("MPI_Test", err_code);
#endif // DAKOTA_HAVE_MPI
}


inline void ParallelLibrary::wait(MPI_Request& request, MPI_Status& status)
{
#ifdef DAKOTA_HAVE_MPI
  int err_code = MPI_Wait(&request, &status);
  check_error("MPI_Wait", err_code);
#endif // DAKOTA_HAVE_MPI
}


inline void ParallelLibrary::
waitall(int num_recvs, MPI_Request*& recv_reqs)
{
#ifdef DAKOTA_HAVE_MPI
  MPI_Status* status_array = new MPI_Status [num_recvs];
  int err_code = MPI_Waitall(num_recvs, recv_reqs, status_array);
  delete [] status_array;
  check_error("MPI_Waitall", err_code);
#endif // DAKOTA_HAVE_MPI
}


inline void ParallelLibrary::
waitsome(int num_sends, MPI_Request*& recv_requests, int& num_recvs,
	 int*& index_array, MPI_Status*& status_array)
{
#ifdef DAKOTA_HAVE_MPI
  int err_code = MPI_Waitsome(num_sends, recv_requests, &num_recvs, index_array,
			      status_array);
  check_error("MPI_Waitsome", err_code);
#endif // DAKOTA_HAVE_MPI
}


inline void ParallelLibrary::free(MPI_Request& request)
{
#ifdef DAKOTA_HAVE_MPI
  int err_code = MPI_Request_free(&request);
  check_error("MPI_Request_free", err_code);
#endif // DAKOTA_HAVE_MPI
}


inline void ParallelLibrary::
inherit_as_server_comm(const ParallelLevel& parent_pl, ParallelLevel& child_pl)
{
  child_pl.commSplitFlag = false;
  //child_pl.messagePass = false; // default

  child_pl.serverIntraComm = parent_pl.serverIntraComm; // or MPI_Comm_dup()
  child_pl.serverCommRank  = parent_pl.serverCommRank;
  child_pl.serverCommSize  = parent_pl.serverCommSize;

  child_pl.hubServerIntraComm = MPI_COMM_NULL; // or a Comm of only 1 proc.
  // use ctor defaults for child_pl.hubServerCommRank/hubServerCommSize
}


inline void ParallelLibrary::
inherit_as_hub_server_comm(const ParallelLevel& parent_pl,
			   ParallelLevel& child_pl)
{
  child_pl.commSplitFlag = false;
  child_pl.messagePass   = true;

  child_pl.serverIntraComm = MPI_COMM_NULL; // prevent further subdivision
  // use ctor defaults for child_pl.serverCommRank/serverCommSize

  child_pl.hubServerIntraComm = parent_pl.serverIntraComm;// or MPI_Comm_dup()
  child_pl.hubServerCommRank  = parent_pl.serverCommRank;
  child_pl.hubServerCommSize  = parent_pl.serverCommSize;
}

} // namespace Dakota

#endif // PARALLEL_LIBRARY_H
