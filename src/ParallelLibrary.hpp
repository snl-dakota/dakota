/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
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

  //bool own_communicators() const;              // return ownCommFlag
  bool dedicated_master() const;               ///< return dedicatedMasterFlag
  //bool communicator_split() const;             // return commSplitFlag
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

  /// read a ParallelLevel object from a packed MPI buffer
  void read(MPIUnpackBuffer& s);
  /// write a ParallelLevel object to a packed MPI buffer
  void write(MPIPackBuffer& s) const;

  /// test comm for MPI_COMM_NULL
  bool null(const MPI_Comm& comm);
  /// test comm for special identity that cannot be deallocated
  bool special(const MPI_Comm& comm);

  /// deallocate the communicators in this ParallelLevel
  /** This appears to be more robust outside of the destructor due to
      interactions among managed deallocation and default deallocation
      (e.g., explicitly freeing a communicator and then default
      deallocating its handle). */
  void clear();

  /// assign the attributes of the incoming pl to this object.  For
  /// communicators, this is a lightweight copy which assigns the same
  /// pointer values as the incoming pl, resulting in the same context.
  void alias(const ParallelLevel& pl);
  /// deep copy the attributes of the incoming pl to this object using
  /// MPI_Comm_dup to create equivalent communicators with a unique context.
  void copy(const ParallelLevel& pl);
  /// copy the scalar attributes of the incoming pl to this object,
  /// omitting communicators
  void copy_config(const ParallelLevel& pl);

private:

  //
  //- Heading: Data
  //

  bool ownCommFlag;              ///< signals Comm ownership for deallocation

  bool dedicatedMasterFlag;      ///< signals dedicated master partitioning
  bool commSplitFlag;            ///< signals a communicator split was used
  bool serverMasterFlag;         ///< identifies master server processors
  bool messagePass;              ///< flag for message passing at this level,
                                 ///< indicating work assignment among servers
  bool idlePartition;            ///< identifies presence of an idle processor
                                 ///< partition at this level

  int numServers;                ///< number of servers
  int procsPerServer;            ///< processors per server
  int procRemainder;             ///< proc remainder after equal distribution
  int serverId;                  ///< server identifier

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
};


inline ParallelLevel::ParallelLevel(): ownCommFlag(true),
  dedicatedMasterFlag(false), commSplitFlag(false), serverMasterFlag(true),
  messagePass(false), idlePartition(false), numServers(0), procsPerServer(0),
  procRemainder(0), serverId(0), serverIntraComm(MPI_COMM_NULL),
  serverCommRank(0), serverCommSize(1), hubServerIntraComm(MPI_COMM_NULL),
  hubServerCommRank(0), hubServerCommSize(1), hubServerInterComm(MPI_COMM_NULL),
  hubServerInterComms(NULL)
{ }

inline ParallelLevel::~ParallelLevel()
{
  // Since MPI_Finalize occurs in the MPIManager dtor and is preceded by the
  // ParallelLibrary dtor, we should be able to deallocate all of the comms
  // here, but memory issues have been observed with this approach.  So we
  // resort to an explicit .clear() on all levels from ~ParallelLibrary.

  // There are temporaries that go out of scope (list insertions by value
  // and print_configuration()) so dellocating here generally requires a
  // deep copy() approach with MPI_Comm_dup().
}

inline bool ParallelLevel::special(const MPI_Comm& comm)
{
  return (comm == MPI_COMM_NULL || comm == MPI_COMM_SELF ||
	  comm == MPI_COMM_WORLD);
}

inline bool ParallelLevel::null(const MPI_Comm& comm)
{ return (comm == MPI_COMM_NULL); }

inline void ParallelLevel::clear()
{
#ifdef DAKOTA_HAVE_MPI
  // ownCommFlag needs to prevent multiple deallocations (e.g., shallow
  // inherits).  MPI_Comm_free sets the current MPI_Comm pointer to 
  // MPI_COMM_NULL but it can't do so for other pointers pointing to the
  // same memory (that has now been deallocated...)
  if (ownCommFlag) { // dealloc intra/inter comms
    if (!special(serverIntraComm))    MPI_Comm_free(&serverIntraComm);
    if (!special(hubServerIntraComm)) MPI_Comm_free(&hubServerIntraComm);
    if (dedicatedMasterFlag) { // master-slave interComms
      if (serverId == 0 && hubServerInterComms) { // if dedicated master
	int i;
	for(i=0; i<numServers; ++i)
	  if (!special(hubServerInterComms[i]))
	    MPI_Comm_free(&hubServerInterComms[i]);
	// trailing server of idle processors
	if (idlePartition && !special(hubServerInterComms[i]))
	  MPI_Comm_free(&hubServerInterComms[i]);
	delete [] hubServerInterComms;
	hubServerInterComms = NULL;
      }
      else if (!special(hubServerInterComm)) // servers 1 through n
	MPI_Comm_free(&hubServerInterComm);
    }
    else { // peer interComms
      if (serverId == 1 && hubServerInterComms) { // 1st peer
	int i;
	for(i=0; i<numServers-1; ++i) 
	  if (!special(hubServerInterComms[i]))
	    MPI_Comm_free(&hubServerInterComms[i]);
	// trailing server of idle processors
	if (idlePartition && !special(hubServerInterComms[i]))
	  MPI_Comm_free(&hubServerInterComms[i]);
	delete [] hubServerInterComms;
	hubServerInterComms = NULL;
      }
      else if (!special(hubServerInterComm)) // peers 2 through n
	MPI_Comm_free(&hubServerInterComm);
    }
  }
#endif // DAKOTA_HAVE_MPI
}

inline void ParallelLevel::copy_config(const ParallelLevel& pl)
{
  // This function copies scalar config settings without copying MPI_Comms

  // these are shared configuration attributes (also passed in read/write)
  dedicatedMasterFlag = pl.dedicatedMasterFlag;
  commSplitFlag = pl.commSplitFlag; serverMasterFlag = pl.serverMasterFlag;
  messagePass   = pl.messagePass;   idlePartition    = pl.idlePartition;
  numServers    = pl.numServers;    procsPerServer   = pl.procsPerServer;
  procRemainder = pl.procRemainder;

  // these are local configuration attributes (not passed in read/write)
  serverId          = pl.serverId;
  serverCommRank    = pl.serverCommRank;
  serverCommSize    = pl.serverCommSize;
  hubServerCommRank = pl.hubServerCommRank;
  hubServerCommSize = pl.hubServerCommSize;
}

inline void ParallelLevel::alias(const ParallelLevel& pl)
{
  // This function is invoked by the copy constructor and operator=, as
  // "aliasing" of MPI comms (sharing the pointer to struct) seems sufficient
  // in practice and avoids the collective communication necessary for
  // MPI_Comm_dup().  Since the copy ctor does not define an initializer list
  // (to avoid initializing data that will be immediately copied over), assign
  // to all values.

  copy_config(pl);

  // For the MPI communicators below, this is a shallow copy.  For many MPI
  // implementations, an MPI_Comm is a pointer to struct so assuming these 
  // pointer values requires care on deallocation (see ownCommFlag protection
  // in ParallelLibrary::clear()).
  ownCommFlag         = false; // shallow copy
  serverIntraComm     = pl.serverIntraComm;     // MPI_Comm is pointer to struct
  hubServerIntraComm  = pl.hubServerIntraComm;  // MPI_Comm is pointer to struct
  hubServerInterComm  = pl.hubServerInterComm;  // MPI_Comm is pointer to struct
  hubServerInterComms = pl.hubServerInterComms; // MPI_Comm*
}

inline void ParallelLevel::copy(const ParallelLevel& pl)
{
  // This function is _not_ invoked by the copy constructor and operator=.
  // It is for explicit use when the communication config is the same but
  // a separate communication context is required.

  // MPI_Comm_dup guarantees a new context that prevents misassociation of
  // messages with the same source/dest and tag (e.g., reuse of fn_eval_id
  // tag across LF and HF models when using nonblocking scheduling of both).

#ifdef DAKOTA_HAVE_MPI
  copy_config(pl);

  //if (mpiManager.mpirun_flag()) { // not accessible from ParallelLevel
    ownCommFlag = true; // deep copy

    if (null(pl.serverIntraComm))    serverIntraComm    = MPI_COMM_NULL;
    else MPI_Comm_dup(pl.serverIntraComm,    &serverIntraComm);

    if (null(pl.hubServerIntraComm)) hubServerIntraComm = MPI_COMM_NULL;
    else MPI_Comm_dup(pl.hubServerIntraComm, &hubServerIntraComm);

    if (null(pl.hubServerInterComm)) hubServerInterComm = MPI_COMM_NULL;
    else MPI_Comm_dup(pl.hubServerInterComm, &hubServerInterComm);

    if (pl.hubServerInterComms == NULL) hubServerInterComms = NULL;
    else { // reallocate + MPI_Comm_dup()
      int i, num_hs_ic = (dedicatedMasterFlag) ? numServers : numServers - 1;
      if (idlePartition) ++num_hs_ic;
      hubServerInterComms = new MPI_Comm [num_hs_ic];
      for (i=0; i<num_hs_ic; ++i)
	if (null(pl.hubServerInterComms[i]))
	  hubServerInterComms[i] = MPI_COMM_NULL;
	else
	  MPI_Comm_dup(pl.hubServerInterComms[i], &hubServerInterComms[i]);
    }
  //}
  //else
  //  alias(pl);
#else
  alias(pl);
#endif
}

inline ParallelLevel::ParallelLevel(const ParallelLevel& pl)
{ alias(pl); }

inline ParallelLevel& ParallelLevel::operator=(const ParallelLevel& pl)
{ alias(pl); return *this; }

//inline bool ParallelLevel::own_communicators() const
//{ return ownCommFlag; }

inline bool ParallelLevel::dedicated_master() const
{ return dedicatedMasterFlag; }

//inline bool ParallelLevel::communicator_split() const
//{ return commSplitFlag; }

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

inline void ParallelLevel::read(MPIUnpackBuffer& s)
{
  // pass configuration settings (which are relevant on other procs),
  // but not specific comm/rank/size/id
  s >> dedicatedMasterFlag >> commSplitFlag >> serverMasterFlag >> messagePass
    >> idlePartition >> numServers >> procsPerServer >> procRemainder;
  //>> serverIntraComm >> serverCommRank >> serverCommSize
  //>> hubServerIntraComm >> hubServerCommRank >> hubServerCommSize
  //>> hubServerInterComm >> hubServerInterComms >> serverId;
}

inline void ParallelLevel::write(MPIPackBuffer& s) const
{
  // pass configuration settings (which are relevant on other procs),
  // but not specific comm/rank/size/id
  s << dedicatedMasterFlag << commSplitFlag << serverMasterFlag << messagePass
    << idlePartition << numServers << procsPerServer << procRemainder;
  //<< serverIntraComm << serverCommRank << serverCommSize
  //<< hubServerIntraComm << hubServerCommRank << hubServerCommSize
  //<< hubServerInterComm << hubServerInterComms << serverId;
}

/// MPIUnpackBuffer extraction operator for ParallelLevel.  Calls
/// read(MPIUnpackBuffer&).
inline MPIUnpackBuffer& 
operator>>(MPIUnpackBuffer& s, ParallelLevel& pl)
{ pl.read(s); return s; }

/// MPIPackBuffer insertion operator for ParallelLevel.  Calls
/// write(MPIPackBuffer&).
inline MPIPackBuffer& 
operator<<(MPIPackBuffer& s, const ParallelLevel& pl)
{ pl.write(s); return s; }


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

  /// return the ParallelLevel corresponding to miPLIters.front()
  const ParallelLevel&  w_parallel_level() const;
  /// return the ParallelLevel corresponding to miPLIters[index]
  const ParallelLevel& mi_parallel_level(size_t index = _NPOS) const;
  /// return the ParallelLevel corresponding to iePLIter
  const ParallelLevel& ie_parallel_level() const;
  /// return the ParallelLevel corresponding to eaPLIter
  const ParallelLevel& ea_parallel_level() const;

  /// test for definition of world parallel level
  bool  w_parallel_level_defined() const;
  /// test for definition of meta-iterator-iterator parallel level
  bool mi_parallel_level_defined(size_t index = _NPOS) const;
  /// test for definition of iterator-evaluation parallel level
  bool ie_parallel_level_defined() const;
  /// test for definition of evaluation-analysis parallel level
  bool ea_parallel_level_defined() const;

  /// return miPLIters.front()
  ParLevLIter  w_parallel_level_iterator() const;
  /// return miPLIters[index]
  ParLevLIter mi_parallel_level_iterator(size_t index = _NPOS) const;
  /// return iePLIter
  ParLevLIter ie_parallel_level_iterator() const;
  /// return eaPLIter
  ParLevLIter ea_parallel_level_iterator() const;

  /// return the index within miPLIters corresponding to pl_iter
  size_t mi_parallel_level_index(ParLevLIter pl_iter) const;
  /// return the index of the last entry in miPLIters
  size_t mi_parallel_level_last_index() const;

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

  /// list iterator for world level followed by any concurrent iterator
  /// partitions (there may be multiple per parallel configuration instance)
  std::vector<ParLevLIter> miPLIters;
  /// list iterator identifying the iterator-evaluation parallelLevel 
  /// (there can only be one)
  ParLevLIter iePLIter;
  /// list iterator identifying the evaluation-analysis parallelLevel 
  /// (there can only be one)
  ParLevLIter eaPLIter;

  /// snapshot of the end of ParallelLibrary::parallelLevels; used for detecting
  /// when a component of the parallel configuration has been initialized
  ParLevLIter endPLIter;
};


inline ParallelConfiguration::ParallelConfiguration(): numParallelLevels(0)
  //iePLIter(NULL), eaPLIter(NULL)
{ }


inline ParallelConfiguration::~ParallelConfiguration()
{ }


inline void ParallelConfiguration::assign(const ParallelConfiguration& pc)
{
  numParallelLevels = pc.numParallelLevels;
  miPLIters = pc.miPLIters;
  iePLIter  = pc.iePLIter;
  eaPLIter  = pc.eaPLIter;
  endPLIter = pc.endPLIter;
}


inline ParallelConfiguration::
ParallelConfiguration(const ParallelConfiguration& pc)
{ assign(pc); }


inline ParallelConfiguration& ParallelConfiguration::
operator=(const ParallelConfiguration& pc)
{ assign(pc); return *this; }


inline const ParallelLevel& ParallelConfiguration::w_parallel_level() const
{ return *miPLIters.front(); }


/** If a meaningful index is not provided, return the last mi parallel
    level.  This is useful within the Model context, for which we need
    the lowest level partition after any meta-iterator recursions. */
inline const ParallelLevel& ParallelConfiguration::
mi_parallel_level(size_t index) const
{ return (index == _NPOS) ? *miPLIters.back() : *miPLIters[index]; }


inline size_t ParallelConfiguration::
mi_parallel_level_index(ParLevLIter pl_iter) const
{
  size_t i, len = miPLIters.size();
  for (i=0; i<len; ++i)
    if (miPLIters[i] == pl_iter)
      return i;
  return _NPOS;
}


inline size_t ParallelConfiguration::mi_parallel_level_last_index() const
{ return (miPLIters.empty()) ? _NPOS : miPLIters.size() - 1; }


inline const ParallelLevel& ParallelConfiguration::ie_parallel_level() const
{ return *iePLIter; }


inline const ParallelLevel& ParallelConfiguration::ea_parallel_level() const
{ return *eaPLIter; }


inline bool ParallelConfiguration::w_parallel_level_defined() const
{ return !miPLIters.empty(); }


inline bool ParallelConfiguration::mi_parallel_level_defined(size_t index) const
{
  if (index == _NPOS) // check for trailing entry (default=last partition)
    return ( !miPLIters.empty() && miPLIters.back() != endPLIter );
  else // specific mi parallel level from index
    return ( index < miPLIters.size() && miPLIters[index] != endPLIter );
}


inline bool ParallelConfiguration::ie_parallel_level_defined() const
{ return ( iePLIter != endPLIter ); }


inline bool ParallelConfiguration::ea_parallel_level_defined() const
{ return ( eaPLIter != endPLIter ); }


inline ParLevLIter ParallelConfiguration::w_parallel_level_iterator() const
{ return miPLIters.front(); }


/** If a meaningful index is not provided, return the last mi parallel
    level.  This is useful within the Model context, for which we need
    the lowest level partition after any meta-iterator recursions. */
inline ParLevLIter ParallelConfiguration::
mi_parallel_level_iterator(size_t index) const
{ return (index == _NPOS) ? miPLIters.back() : miPLIters[index]; }


inline ParLevLIter ParallelConfiguration::ie_parallel_level_iterator() const
{ return iePLIter; }


inline ParLevLIter ParallelConfiguration::ea_parallel_level_iterator() const
{ return eaPLIter; }


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

  /// print the parallel level settings for a particular parallel configuration
  void print_configuration();

  /// conditionally append an iterator server id tag to the
  /// hierarchical output tag, manage restart, and rebind cout/cerr
  void push_output_tag(const ParallelLevel& pl);

  /// pop the last output tag and rebind streams as needed; pl isn't
  /// yet used, but may be in the future when we generalize to
  /// arbitrary output context switching
  void pop_output_tag(const ParallelLevel& pl);

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

  /// blocking buffer send at the current communication level
  void  send(MPIPackBuffer& send_buff, int dest, int tag,
	     const ParallelLevel& parent_pl, const ParallelLevel& child_pl);
  /// blocking integer send at the current communication level
  void  send(int& send_int, int dest, int tag,
	     const ParallelLevel& parent_pl, const ParallelLevel& child_pl);

  /// nonblocking buffer send at the current communication level
  void isend(MPIPackBuffer& send_buff, int dest, int tag,
	     MPI_Request& send_req, const ParallelLevel& parent_pl,
	     const ParallelLevel& child_pl);
  /// nonblocking integer send at the current communication level
  void isend(int& send_int, int dest, int tag,
	     MPI_Request& send_req, const ParallelLevel& parent_pl,
	     const ParallelLevel& child_pl);

  /// blocking buffer receive at the current communication level
  void  recv(MPIUnpackBuffer& recv_buff, int source, int tag,
	     MPI_Status& status, const ParallelLevel& parent_pl,
	     const ParallelLevel& child_pl);
  /// blocking integer receive at the current communication level
  void  recv(int& recv_int, int source, int tag,
	     MPI_Status& status, const ParallelLevel& parent_pl,
	     const ParallelLevel& child_pl);

  /// nonblocking buffer receive at the current communication level
  void irecv(MPIUnpackBuffer& recv_buff, int source, int tag,
	     MPI_Request& recv_req, const ParallelLevel& parent_pl,
	     const ParallelLevel& child_pl);
  /// nonblocking integer receive at the current communication level
  void irecv(int& recv_int, int source, int tag,
	     MPI_Request& recv_req, const ParallelLevel& parent_pl,
	     const ParallelLevel& child_pl);

  /// process _NPOS default and perform error checks
  void check_mi_index(size_t& index) const;

  /// blocking send at the metaiterator-iterator communication level
  void  send_mi(int& send_int, int dest, int tag, size_t index = _NPOS);
  /// nonblocking send at the metaiterator-iterator communication level
  void isend_mi(int& send_int, int dest, int tag, MPI_Request& send_req,
		size_t index = _NPOS);
  /// blocking receive at the metaiterator-iterator communication level
  void  recv_mi(int& recv_int, int source, int tag, MPI_Status& status,
		size_t index = _NPOS);
  /// nonblocking receive at the metaiterator-iterator communication level
  void irecv_mi(int& recv_int, int source, int tag, MPI_Request& recv_req,
		size_t index = _NPOS);

  /// blocking send at the metaiterator-iterator communication level
  void  send_mi(MPIPackBuffer& send_buff, int dest, int tag,
		size_t index = _NPOS);
  /// nonblocking send at the metaiterator-iterator communication level
  void isend_mi(MPIPackBuffer& send_buff, int dest, int tag, 
		MPI_Request& send_req, size_t index = _NPOS);
  /// blocking receive at the metaiterator-iterator communication level
  void  recv_mi(MPIUnpackBuffer& recv_buff, int source, int tag, 
		MPI_Status& status, size_t index = _NPOS);
  /// nonblocking receive at the metaiterator-iterator communication level
  void irecv_mi(MPIUnpackBuffer& recv_buff, int source, int tag, 
		MPI_Request& recv_req, size_t index = _NPOS);

  /// blocking send at the iterator-evaluation communication level
  void  send_ie(int& send_int, int dest, int tag);
  /// nonblocking send at the iterator-evaluation communication level
  void isend_ie(int& send_int, int dest, int tag, MPI_Request& send_req);
  /// blocking receive at the iterator-evaluation communication level
  void  recv_ie(int& recv_int, int source, int tag, MPI_Status& status);
  /// nonblocking receive at the iterator-evaluation communication level
  void irecv_ie(int& recv_int, int source, int tag, MPI_Request& recv_req);

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
  /// broadcast an integer across the serverIntraComm of a ParallelLevel
  void bcast(short& data, const ParallelLevel& pl);
  /// broadcast a MPIPackBuffer across the serverIntraComm of a ParallelLevel
  void bcast(MPIPackBuffer& send_buff, const ParallelLevel& pl);
  /// broadcast a MPIUnpackBuffer across the serverIntraComm of a ParallelLevel
  void bcast(MPIUnpackBuffer& recv_buff, const ParallelLevel& pl);
  /// broadcast an integer across the hubServerIntraComm of a ParallelLevel
  void bcast_hs(int& data, const ParallelLevel& pl);
  /// broadcast a MPIPackBuffer across the hubServerIntraComm of a ParallelLevel
  void bcast_hs(MPIPackBuffer& send_buff, const ParallelLevel& pl);
  /// broadcast a MPIUnpackBuffer across the hubServerIntraComm of a
  /// ParallelLevel
  void bcast_hs(MPIUnpackBuffer& recv_buff, const ParallelLevel& pl);

  /// broadcast an integer across MPI_COMM_WORLD
  void bcast_w(int& data);
  /// broadcast an integer across an iterator communicator
  void bcast_i(int& data, size_t index = _NPOS);
  /// broadcast a short integer across an iterator communicator
  void bcast_i(short& data, size_t index = _NPOS);
  /// broadcast an integer across an evaluation communicator
  void bcast_e(int& data);
  /// broadcast an integer across an analysis communicator
  void bcast_a(int& data);
  /// broadcast an integer across a metaiterator-iterator intra communicator
  void bcast_mi(int& data, size_t index = _NPOS);
  /// broadcast a packed buffer across MPI_COMM_WORLD
  void bcast_w(MPIPackBuffer& send_buff);
  /// broadcast a packed buffer across an iterator communicator
  void bcast_i(MPIPackBuffer& send_buff, size_t index = _NPOS);
  /// broadcast a packed buffer across an evaluation communicator
  void bcast_e(MPIPackBuffer& send_buff);
  /// broadcast a packed buffer across an analysis communicator
  void bcast_a(MPIPackBuffer& send_buff);
  /// broadcast a packed buffer across a metaiterator-iterator intra
  /// communicator
  void bcast_mi(MPIPackBuffer& send_buff, size_t index = _NPOS);
  /// matching receive for packed buffer broadcast across MPI_COMM_WORLD
  void bcast_w(MPIUnpackBuffer& recv_buff);
  /// matching receive for packed buffer bcast across an iterator communicator
  void bcast_i(MPIUnpackBuffer& recv_buff, size_t index = _NPOS);
  /// matching receive for packed buffer bcast across an evaluation communicator
  void bcast_e(MPIUnpackBuffer& recv_buff);
  /// matching receive for packed buffer bcast across an analysis communicator
  void bcast_a(MPIUnpackBuffer& recv_buff);
  /// matching recv for packed buffer bcast across a
  /// metaiterator-iterator intra comm
  void bcast_mi(MPIUnpackBuffer& recv_buff, size_t index = _NPOS);

  /// enforce MPI_Barrier on MPI_COMM_WORLD
  void barrier_w();
  /// enforce MPI_Barrier on an iterator communicator
  void barrier_i(size_t index = _NPOS);
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
  int world_size() const; ///< return MPIManager::worldSize
  int world_rank() const; ///< return MPIManager::worldRank
  bool mpirun_flag() const;   ///< return MPIManager::mpirunFlag
  bool is_null() const;       ///< return dummyFlag
  Real parallel_time() const; ///< returns current MPI wall clock time

  /// set the current ParallelConfiguration node
  void parallel_configuration_iterator(ParConfigLIter pc_iter);
  /// return the current ParallelConfiguration node
  ParConfigLIter parallel_configuration_iterator() const;
  /// return the current ParallelConfiguration instance
  const ParallelConfiguration& parallel_configuration() const;

  /// returns the number of entries in parallelConfigurations
  size_t num_parallel_configurations() const;
  /// identifies if the current ParallelConfiguration has been fully populated
  bool parallel_configuration_is_complete();
  /// add a new node to parallelConfigurations and increment currPCIter;
  /// limit miPLIters within new configuration to mi_pl_iter level
  void increment_parallel_configuration(ParLevLIter mi_pl_iter);
  /// add a new node to parallelConfigurations and increment currPCIter;
  /// copy all of miPLIters into new configuration
  void increment_parallel_configuration();
  // decrement currPCIter
  //void decrement_parallel_configuration();

  /// test current parallel configuration for definition of world parallel level
  bool  w_parallel_level_defined() const;
  /// test current parallel configuration for definition of
  /// meta-iterator-iterator parallel level
  bool mi_parallel_level_defined(size_t index = _NPOS) const;
  /// test current parallel configuration for definition of
  /// iterator-evaluation parallel level
  bool ie_parallel_level_defined() const;
  /// test current parallel configuration for definition of
  /// evaluation-analysis parallel level
  bool ea_parallel_level_defined() const;

  /// for this level, access through ParallelConfiguration is not necessary
  ParLevLIter w_parallel_level_iterator();

  /// return the index within parallelLevels corresponding to pl_iter
  size_t parallel_level_index(ParLevLIter pl_iter);
  // return the index within currPCIter->miPLIters corresponding to pl_iter
  //size_t mi_parallel_level_index(ParLevLIter pl_iter) const;
  // return index of trailing entry in currPCIter->miPLIters
  //size_t mi_parallel_level_last_index() const;

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
  /// parent serverIntraComm (shallow Comm copy)
  void alias_as_server_comm(const ParallelLevel& parent_pl,
			    ParallelLevel& child_pl);
  /// convenience function for updating child serverIntraComm from
  /// parent serverIntraComm (deep Comm copy)
  void copy_as_server_comm(const ParallelLevel& parent_pl,
			   ParallelLevel& child_pl);
  /// convenience function for updating child hubServerIntraComm from
  /// parent serverIntraComm (shallow Comm copy)
  void alias_as_hub_server_comm(const ParallelLevel& parent_pl,
				ParallelLevel& child_pl);
  /// convenience function for updating child hubServerIntraComm from
  /// parent serverIntraComm (deep Comm copy)
  void copy_as_hub_server_comm(const ParallelLevel& parent_pl,
			       ParallelLevel& child_pl);

  //
  //- Heading: Data
  //
  
  /// reference to the MPI manager with Dakota's MPI options
  const MPIManager& mpiManager;
  /// programOptions is non-const due to updates from broadcast
  ProgramOptions& programOptions;
  /// Non-const output handler to help with file redirection
  OutputManager& outputManager;

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

  /// list iterator identifying the current node in parallelConfigurations
  ParConfigLIter currPCIter;
};


inline ProgramOptions& ParallelLibrary::program_options()
{ return programOptions; }


inline OutputManager& ParallelLibrary::output_manager()
{ return outputManager; }


inline int ParallelLibrary::world_rank() const
{ return mpiManager.world_rank(); }


inline int ParallelLibrary::world_size() const
{ return mpiManager.world_size(); }


inline bool ParallelLibrary::mpirun_flag() const
{ return mpiManager.mpirun_flag(); }


inline bool ParallelLibrary::is_null() const
{ return dummyFlag; }


inline Real ParallelLibrary::parallel_time() const
{
#ifdef DAKOTA_HAVE_MPI
  return (mpiManager.mpirun_flag()) ? MPI_Wtime() - startMPITime : 0.;
#else
  return 0.;
#endif // DAKOTA_HAVE_MPI
}


inline void ParallelLibrary::
parallel_configuration_iterator(ParConfigLIter pc_iter)
{ currPCIter = pc_iter; }


inline ParConfigLIter ParallelLibrary::parallel_configuration_iterator() const
{ return currPCIter; }


inline const ParallelConfiguration& ParallelLibrary::
parallel_configuration() const
{ return *currPCIter; }


inline size_t ParallelLibrary::num_parallel_configurations() const
{ return parallelConfigurations.size(); }


inline bool ParallelLibrary::parallel_configuration_is_complete()
{
  size_t i, num_mipl = currPCIter->miPLIters.size();
  if (!num_mipl) return false; // PC incomplete if w level undefined
  bool prev_pl_rank0 = (mpiManager.world_rank() == 0);
  for (i=1; i<num_mipl; ++i) { // skip w_pl (dedicated master not supported)
    const ParallelLevel& mi_pl = *currPCIter->miPLIters[i];
    if (mi_pl.dedicatedMasterFlag && prev_pl_rank0)
      return true; // PC complete at mi level for ded master
    else
      prev_pl_rank0 = (mi_pl.serverCommRank == 0);
  }

  if (!currPCIter->ie_parallel_level_defined())
    return false; // PC incomplete for remaining procs if ie level undefined

  const ParallelLevel& ie_pl = currPCIter->ie_parallel_level();
  if (ie_pl.dedicatedMasterFlag && prev_pl_rank0)
    return true;  // PC complete at ie level for iterator ded master
  else // PC incomplete for remaining procs if ea level undefined
    return currPCIter->ea_parallel_level_defined();
}


/** Called from the ParallelLibrary ctor and from Model::init_communicators().
    An increment is performed for each Model initialization except the first
    (which inherits the world level from the first partial configuration). */
inline void ParallelLibrary::
increment_parallel_configuration(ParLevLIter mi_pl_iter)
{
  // The world level is set in the ParallelLib ctor, mi levels are set in
  // MetaIterators and NestedModels, and the ie and ea levels are defined in
  // ApplicationInterface::init_communicators().  Any undefined iterators
  // are initialized to their "singular values" (NULL is not used).
  ParallelConfiguration pc;

  // initial configuration: copy all parallel levels into miPLIters.  In
  // current usage, there should only be one parallel level (world level)
  // from ParallelLibrary::init_mpi_comm()
  size_t i, num_mi_pl;
  if (parallelConfigurations.empty()) {
    for (ParLevLIter pl_iter=parallelLevels.begin();
	 pl_iter!=parallelLevels.end(); ++pl_iter) {
      pc.miPLIters.push_back(pl_iter);
      if (pl_iter == mi_pl_iter) break;
    }
  }
  // incrementing from an existing configuration: inherit a subset of
  // mi parallel levels configured to this point
  else {
    //pc.miPLIters = currPCIter->miPLIters; // before passing of num_mipl
    const std::vector<ParLevLIter>& curr_mipl_iters = currPCIter->miPLIters;
    num_mi_pl = curr_mipl_iters.size();
    for (i=0; i<num_mi_pl; ++i) {
      pc.miPLIters.push_back(curr_mipl_iters[i]);
      if (curr_mipl_iters[i] == mi_pl_iter) break;
    }
  }
  // update numParallelLevels to correspond to miPLIters starting point
  num_mi_pl = pc.miPLIters.size();
  for (i=0; i<num_mi_pl; ++i)
    if (pc.miPLIters[i]->messagePass)
      ++pc.numParallelLevels;

  // ie & ea levels to be defined by ApplicationInterface::init_communicators()
  pc.iePLIter = pc.eaPLIter = pc.endPLIter = parallelLevels.end();

  parallelConfigurations.push_back(pc);
  currPCIter = --parallelConfigurations.end();
}


inline void ParallelLibrary::increment_parallel_configuration()
{
  ParLevLIter pl_iter = (parallelConfigurations.empty()) ?
    --parallelLevels.end() : currPCIter->miPLIters.back(); // last to include
  increment_parallel_configuration(pl_iter);
}


//inline void ParallelLibrary::decrement_parallel_configuration()
//{ --currPCIter; }


inline bool ParallelLibrary::w_parallel_level_defined() const
{
  return (  currPCIter != parallelConfigurations.end() &&
	    currPCIter->w_parallel_level_defined() );
}


inline bool ParallelLibrary::mi_parallel_level_defined(size_t index) const
{
  return (  currPCIter != parallelConfigurations.end() &&
	    currPCIter->mi_parallel_level_defined(index) );
}


inline bool ParallelLibrary::ie_parallel_level_defined() const
{
  return ( currPCIter != parallelConfigurations.end() &&
	   currPCIter->ie_parallel_level_defined() );
}


inline bool ParallelLibrary::ea_parallel_level_defined() const
{
  return ( currPCIter != parallelConfigurations.end() &&
	   currPCIter->ea_parallel_level_defined() );
}


inline ParLevLIter ParallelLibrary::w_parallel_level_iterator()
{ return parallelLevels.begin(); }


inline size_t ParallelLibrary::parallel_level_index(ParLevLIter pl_iter)
{
  return (parallelLevels.empty()) ? _NPOS :
    std::distance(parallelLevels.begin(), pl_iter);
}


/* These are error-prone due to requirement to update currPCIter
   --> force usage of {method,model}PCIter in clients...
inline size_t ParallelLibrary::
mi_parallel_level_index(ParLevLIter pl_iter) const
{
  if (currPCIter == parallelConfigurations.end() ||
      currPCIter->miPLIters.empty()) {
    Cerr << "Error: mi_parallel_level_index() called with no active mi "
	 << "parallelism levels defined." << std::endl;
    abort_handler(-1);
    return _NPOS;
  }
  else 
    return currPCIter->mi_parallel_level_index(pl_iter);
}


inline size_t ParallelLibrary::mi_parallel_level_last_index() const
{
  if (currPCIter == parallelConfigurations.end() ||
      currPCIter->miPLIters.empty()) {
    Cerr << "Error: mi_parallel_level_last_index() called with no active mi "
	 << "parallelism levels defined." << std::endl;
    abort_handler(-1);
    return _NPOS;
  }
  else 
    return currPCIter->mi_parallel_level_last_index();
}
*/


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
  init_communicators(*currPCIter->miPLIters.back(), iterator_servers,
		     procs_per_iterator, min_procs_per_iterator,
		     max_procs_per_iterator, max_iterator_concurrency,
		     asynch_local_iterator_concurrency, default_config,
		     iterator_scheduling, peer_dynamic_avail);
  ParLevLIter last = --parallelLevels.end();

  // unconditionally update miPLIters
  currPCIter->miPLIters.push_back(last);
  return *last; // same as parallelLevels.back()

  /*
  // update miPLIters iff new partition.  This complicates deallocation (the new
  // ParallelLevel does not correpond to a ParLevLIter) and requires the client
  // to detect and manage when the higher level config has not been updated
  // (requiring a serialization step to ensure that the previous level settings
  // are not adopted).  Therefore, the benefits of this streamlining are dubious
  // given the need to recreate serial settings from the new parallel level.
  if (last->messagePass || last->idlePartition)
    currPCIter->miPLIters.push_back(last);
  else
    parallelLevels.pop_back(); // assuming comm deallocation in pl destructor
  return *currPCIter->miPLIters.back();
  */
}


inline const ParallelLevel& ParallelLibrary::
init_evaluation_communicators(int evaluation_servers, int procs_per_evaluation,
			      int min_procs_per_eval, int max_procs_per_eval,
			      int max_evaluation_concurrency,
			      int asynch_local_evaluation_concurrency,
			      short default_config, short evaluation_scheduling,
			      bool peer_dynamic_avail)
{
  init_communicators(*currPCIter->miPLIters.back(), evaluation_servers,
		     procs_per_evaluation, min_procs_per_eval,
		     max_procs_per_eval, max_evaluation_concurrency,
		     asynch_local_evaluation_concurrency, default_config,
		     evaluation_scheduling, peer_dynamic_avail);
  currPCIter->iePLIter = --parallelLevels.end();
  return *currPCIter->iePLIter;
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
  currPCIter->eaPLIter = --parallelLevels.end();
  return *currPCIter->eaPLIter;
}


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
	 << " on processor " << mpiManager.world_rank() << std::endl;
    abort_handler(-1);
  }
}


inline void ParallelLibrary::
send(MPIPackBuffer& send_buff, int dest, int tag,
     const ParallelLevel& parent_pl, const ParallelLevel& child_pl)
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
send(int& send_int, int dest, int tag, const ParallelLevel& parent_pl,
     const ParallelLevel& child_pl)
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


inline void ParallelLibrary::check_mi_index(size_t& index) const
{
  size_t num_mi_pl = currPCIter->miPLIters.size();
  if (!num_mi_pl) {
    Cerr << "Error: mi level send/recv called with no mi parallelism levels "
	 << "defined." << std::endl;
    abort_handler(-1);
  }
  if (index == _NPOS) index = num_mi_pl - 1; // last entry
  else if (index >= num_mi_pl) {
    Cerr << "Error: mi level send/recv called with index out of bounds."
	 << std::endl;
    abort_handler(-1);
  }
}


inline void ParallelLibrary::
send_mi(int& send_int, int dest, int tag, size_t index)
{
  check_mi_index(index);
  const ParallelLevel& parent_pl = (index) ?
    *currPCIter->miPLIters[index-1] : *parallelLevels.begin();
  send(send_int, dest, tag, parent_pl, *currPCIter->miPLIters[index]);
}


inline void ParallelLibrary::
send_mi(MPIPackBuffer& send_buff, int dest, int tag, size_t index)
{
  check_mi_index(index);
  const ParallelLevel& parent_pl = (index) ?
    *currPCIter->miPLIters[index-1] : *parallelLevels.begin();
  send(send_buff, dest, tag, parent_pl, *currPCIter->miPLIters[index]);
}


inline void ParallelLibrary::send_ie(int& send_int, int dest, int tag)
{
  send(send_int, dest, tag, *currPCIter->miPLIters.back(),
       *currPCIter->iePLIter);
}


inline void ParallelLibrary::
send_ie(MPIPackBuffer& send_buff, int dest, int tag)
{
  send(send_buff, dest, tag, *currPCIter->miPLIters.back(),
       *currPCIter->iePLIter);
}


inline void ParallelLibrary::send_ea(int& send_int, int dest, int tag)
{ send(send_int, dest, tag, *currPCIter->iePLIter, *currPCIter->eaPLIter); }


inline void ParallelLibrary::
isend(MPIPackBuffer& send_buff, int dest, int tag,
      MPI_Request& send_req, const ParallelLevel& parent_pl,
      const ParallelLevel& child_pl)
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
      const ParallelLevel& parent_pl, const ParallelLevel& child_pl)
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
isend_mi(int& send_int, int dest, int tag, MPI_Request& send_req, size_t index)
{
  check_mi_index(index);
  const ParallelLevel& parent_pl = (index) ?
    *currPCIter->miPLIters[index-1] : *parallelLevels.begin();
  isend(send_int, dest, tag, send_req, parent_pl,
	*currPCIter->miPLIters[index]);
}


inline void ParallelLibrary::
isend_mi(MPIPackBuffer& send_buff, int dest, int tag, MPI_Request& send_req,
	 size_t index)
{
  check_mi_index(index);
  const ParallelLevel& parent_pl = (index) ?
    *currPCIter->miPLIters[index-1] : *parallelLevels.begin();
  isend(send_buff, dest, tag, send_req, parent_pl,
	*currPCIter->miPLIters[index]);
}


inline void ParallelLibrary::
isend_ie(int& send_int, int dest, int tag, MPI_Request& send_req)
{
  isend(send_int, dest, tag, send_req, *currPCIter->miPLIters.back(),
	*currPCIter->iePLIter);
}


inline void ParallelLibrary::
isend_ie(MPIPackBuffer& send_buff, int dest, int tag, MPI_Request& send_req)
{
  isend(send_buff, dest, tag, send_req, *currPCIter->miPLIters.back(),
	*currPCIter->iePLIter);
}


inline void ParallelLibrary::
isend_ea(int& send_int, int dest, int tag, MPI_Request& send_req)
{
  isend(send_int, dest, tag, send_req, *currPCIter->iePLIter,
	*currPCIter->eaPLIter);
}


inline void ParallelLibrary::
recv(MPIUnpackBuffer& recv_buff, int source, int tag, MPI_Status& status,
     const ParallelLevel& parent_pl, const ParallelLevel& child_pl)
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
     const ParallelLevel& parent_pl, const ParallelLevel& child_pl)
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
recv_mi(int& recv_int, int source, int tag, MPI_Status& status, size_t index)
{
  check_mi_index(index);
  const ParallelLevel& parent_pl = (index) ?
    *currPCIter->miPLIters[index-1] : *parallelLevels.begin();
  recv(recv_int, source, tag, status, parent_pl, *currPCIter->miPLIters[index]);
}


inline void ParallelLibrary::
recv_mi(MPIUnpackBuffer& recv_buff, int source, int tag, MPI_Status& status,
	size_t index)
{
  check_mi_index(index);
  const ParallelLevel& parent_pl = (index) ?
    *currPCIter->miPLIters[index-1] : *parallelLevels.begin();
  recv(recv_buff, source, tag, status, parent_pl,
       *currPCIter->miPLIters[index]);
}


inline void ParallelLibrary::
recv_ie(int& recv_int, int source, int tag, MPI_Status& status)
{
  recv(recv_int, source, tag, status, *currPCIter->miPLIters.back(),
       *currPCIter->iePLIter);
}


inline void ParallelLibrary::
recv_ie(MPIUnpackBuffer& recv_buff, int source, int tag, MPI_Status& status)
{
  recv(recv_buff, source, tag, status, *currPCIter->miPLIters.back(),
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
      MPI_Request& recv_req, const ParallelLevel& parent_pl,
      const ParallelLevel& child_pl)
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
      const ParallelLevel& parent_pl, const ParallelLevel& child_pl)
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
irecv_mi(int& recv_int, int source, int tag, MPI_Request& recv_req,
	 size_t index)
{
  check_mi_index(index);
  const ParallelLevel& parent_pl = (index) ?
    *currPCIter->miPLIters[index-1] : *parallelLevels.begin();
  irecv(recv_int, source, tag, recv_req, parent_pl,
	*currPCIter->miPLIters[index]);
}


inline void ParallelLibrary::
irecv_mi(MPIUnpackBuffer& recv_buff, int source, int tag, MPI_Request& recv_req,
	 size_t index)
{
  check_mi_index(index);
  const ParallelLevel& parent_pl = (index) ?
    *currPCIter->miPLIters[index-1] : *parallelLevels.begin();
  irecv(recv_buff, source, tag, recv_req, parent_pl,
	*currPCIter->miPLIters[index]);
}


inline void ParallelLibrary::
irecv_ie(int& recv_int, int source, int tag, MPI_Request& recv_req)
{
  irecv(recv_int, source, tag, recv_req, *currPCIter->miPLIters.back(),
	*currPCIter->iePLIter);
}


inline void ParallelLibrary::
irecv_ie(MPIUnpackBuffer& recv_buff, int source, int tag, MPI_Request& recv_req)
{
  irecv(recv_buff, source, tag, recv_req, *currPCIter->miPLIters.back(),
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


inline void ParallelLibrary::bcast(short& data, const ParallelLevel& pl)
{ bcast(data, pl.serverIntraComm); }


inline void ParallelLibrary::
bcast(MPIPackBuffer& send_buff, const ParallelLevel& pl)
{ bcast(send_buff, pl.serverIntraComm); }


inline void ParallelLibrary::
bcast(MPIUnpackBuffer& recv_buff, const ParallelLevel& pl)
{ bcast(recv_buff, pl.serverIntraComm); }


inline void ParallelLibrary::bcast_hs(int& data, const ParallelLevel& pl)
{ bcast(data, pl.hubServerIntraComm); }


inline void ParallelLibrary::
bcast_hs(MPIPackBuffer& send_buff, const ParallelLevel& pl)
{ bcast(send_buff, pl.hubServerIntraComm); }


inline void ParallelLibrary::
bcast_hs(MPIUnpackBuffer& recv_buff, const ParallelLevel& pl)
{ bcast(recv_buff, pl.hubServerIntraComm); }


inline void ParallelLibrary::bcast_w(int& data)
{ bcast(data, currPCIter->miPLIters.front()->serverIntraComm); }


inline void ParallelLibrary::bcast_i(int& data,	size_t index)
{
  check_mi_index(index);
  bcast(data, currPCIter->miPLIters[index]->serverIntraComm);
}


inline void ParallelLibrary::bcast_i(short& data, size_t index)
{
  check_mi_index(index);
  bcast(data, currPCIter->miPLIters[index]->serverIntraComm);
}


inline void ParallelLibrary::bcast_e(int& data)
{ bcast(data, currPCIter->iePLIter->serverIntraComm); }


inline void ParallelLibrary::bcast_a(int& data)
{ bcast(data, currPCIter->eaPLIter->serverIntraComm); }


inline void ParallelLibrary::bcast_mi(int& data, size_t index)
{
  check_mi_index(index);
  bcast_hs(data, *currPCIter->miPLIters[index]);
}


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
{ bcast(send_buff, currPCIter->miPLIters.front()->serverIntraComm); }


inline void ParallelLibrary::bcast_i(MPIPackBuffer& send_buff, size_t index)
{
  check_mi_index(index);
  bcast(send_buff, currPCIter->miPLIters[index]->serverIntraComm);
}


inline void ParallelLibrary::bcast_e(MPIPackBuffer& send_buff)
{ bcast(send_buff, currPCIter->iePLIter->serverIntraComm); }


inline void ParallelLibrary::bcast_a(MPIPackBuffer& send_buff)
{ bcast(send_buff, currPCIter->eaPLIter->serverIntraComm); }


inline void ParallelLibrary::bcast_mi(MPIPackBuffer& send_buff, size_t index)
{
  check_mi_index(index);
  bcast_hs(send_buff, *currPCIter->miPLIters[index]);
}


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
{ bcast(recv_buff, currPCIter->miPLIters.front()->serverIntraComm); }


inline void ParallelLibrary::bcast_i(MPIUnpackBuffer& recv_buff, size_t index)
{
  check_mi_index(index);
  bcast(recv_buff, currPCIter->miPLIters[index]->serverIntraComm);
}


inline void ParallelLibrary::bcast_e(MPIUnpackBuffer& recv_buff)
{ bcast(recv_buff, currPCIter->iePLIter->serverIntraComm); }


inline void ParallelLibrary::bcast_a(MPIUnpackBuffer& recv_buff)
{ bcast(recv_buff, currPCIter->eaPLIter->serverIntraComm); }


inline void ParallelLibrary::bcast_mi(MPIUnpackBuffer& recv_buff, size_t index)
{
  check_mi_index(index);
  bcast_hs(recv_buff, *currPCIter->miPLIters[index]);
}


inline void ParallelLibrary::barrier(const MPI_Comm& comm)
{
#ifdef DAKOTA_HAVE_MPI
  int err_code = MPI_Barrier(comm);
  check_error("MPI_Barrier", err_code);
#endif // DAKOTA_HAVE_MPI
}


inline void ParallelLibrary::barrier_w()
{ barrier(currPCIter->miPLIters.front()->serverIntraComm); }


inline void ParallelLibrary::barrier_i(size_t index)
{
  check_mi_index(index);
  barrier(currPCIter->miPLIters[index]->serverIntraComm);
}


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
alias_as_server_comm(const ParallelLevel& parent_pl, ParallelLevel& child_pl)
{
  // true if servers are created for assignment of distributed work
  child_pl.messagePass   = false;
  // important for correct branch in send/recv
  child_pl.commSplitFlag = false;

  child_pl.ownCommFlag      =  false;
  child_pl.serverIntraComm  =  parent_pl.serverIntraComm; // same comm
  child_pl.serverCommRank   =  parent_pl.serverCommRank;
  child_pl.serverCommSize   =  parent_pl.serverCommSize;
  child_pl.serverMasterFlag = (parent_pl.serverCommRank == 0);

  child_pl.hubServerIntraComm = MPI_COMM_NULL;
  // use ctor defaults for child_pl.hubServerCommRank/hubServerCommSize
}


inline void ParallelLibrary::
copy_as_server_comm(const ParallelLevel& parent_pl, ParallelLevel& child_pl)
{
#ifdef DAKOTA_HAVE_MPI
  if (mpiManager.mpirun_flag() && parent_pl.serverIntraComm != MPI_COMM_NULL) {
    // servers are not created for assignment of distributed work
    child_pl.messagePass   = false;
    // use parent_pl in send/recv (child_pl.hubServerInterComms not defined)
    child_pl.commSplitFlag = false;

    // deep copy ensures inherited comm context is distinct
    child_pl.ownCommFlag = true;
    MPI_Comm_dup(parent_pl.serverIntraComm, &child_pl.serverIntraComm);
    child_pl.serverCommRank   =  parent_pl.serverCommRank;
    child_pl.serverCommSize   =  parent_pl.serverCommSize;
    child_pl.serverMasterFlag = (parent_pl.serverCommRank == 0);

    child_pl.hubServerIntraComm = MPI_COMM_NULL;
    // use ctor defaults for child_pl.hubServerCommRank/hubServerCommSize
  }
  else
    alias_as_server_comm(parent_pl, child_pl);
#else
  alias_as_server_comm(parent_pl, child_pl);
#endif // DAKOTA_HAVE_MPI
}


inline void ParallelLibrary::
alias_as_hub_server_comm(const ParallelLevel& parent_pl,
			 ParallelLevel& child_pl)
{
  // single-processor servers are defined for assignment of distributed work
  child_pl.messagePass   = (parent_pl.serverCommSize > 1);
  // use parent_pl in send/recv (child_pl.hubServerInterComms not defined)
  child_pl.commSplitFlag = false;

  child_pl.ownCommFlag = false;
  child_pl.hubServerIntraComm = parent_pl.serverIntraComm;
  child_pl.hubServerCommRank  = parent_pl.serverCommRank;
  child_pl.hubServerCommSize  = parent_pl.serverCommSize;

  child_pl.serverIntraComm = MPI_COMM_SELF;
  // use ctor defaults for child_pl.serverCommRank/serverCommSize
}


inline void ParallelLibrary::
copy_as_hub_server_comm(const ParallelLevel& parent_pl, ParallelLevel& child_pl)
{
#ifdef DAKOTA_HAVE_MPI
  if (mpiManager.mpirun_flag() && parent_pl.serverIntraComm != MPI_COMM_NULL) {
    // single-processor servers are defined for assignment of distributed work
    child_pl.messagePass   = (parent_pl.serverCommSize > 1);
    // use parent_pl in send/recv (child_pl.hubServerInterComms not defined)
    child_pl.commSplitFlag = false;

    // deep copy ensures inherited comm context is distinct
    child_pl.ownCommFlag = true;
    MPI_Comm_dup(parent_pl.serverIntraComm, &child_pl.hubServerIntraComm);
    child_pl.hubServerCommRank = parent_pl.serverCommRank;
    child_pl.hubServerCommSize = parent_pl.serverCommSize;

    child_pl.serverIntraComm = MPI_COMM_SELF;
    // use ctor defaults for child_pl.serverCommRank/serverCommSize
  }
  else
    alias_as_hub_server_comm(parent_pl, child_pl);
#else
  alias_as_hub_server_comm(parent_pl, child_pl);
#endif // DAKOTA_HAVE_MPI
}

} // namespace Dakota

#endif // PARALLEL_LIBRARY_H
