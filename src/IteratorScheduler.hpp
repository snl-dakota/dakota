/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       IteratorScheduler
//- Description: 
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: IteratorScheduler.hpp 6492 2009-12-19 00:04:28Z briadam $

#ifndef ITERATOR_SCHEDULER_H
#define ITERATOR_SCHEDULER_H

//#include "Scheduler.hpp"
#include "DataMethod.hpp"


namespace Dakota {

class Iterator;
class Model;
class ParallelLibrary;
class ProblemDescDB;


/// Environment corresponding to execution as a stand-alone application.

/** This environment corresponds to a stand-alone executable program,
    e.g., main.cpp.  It sets up the ParallelLibrary,
    and ProblemDescDB objects based on access to command line arguments. */

class IteratorScheduler //: public Scheduler
{
public:
  
  //
  //- Heading: Constructors and destructor
  //

  // default constructor
  //IteratorScheduler();
  /// constructor
  IteratorScheduler(ParallelLibrary& parallel_lib, int num_servers,
		    int procs_per_iterator = 0, int min_procs_per_iterator = 1,
		    short scheduling = DEFAULT_SCHEDULING);
  /// destructor
  ~IteratorScheduler();
    
  //
  //- Heading: Static member functions
  //

  /// convenience function for serializing the concurrent iterator
  /// parallelism level
  static void init_serial_iterators(ParallelLibrary& parallel_lib);

  /// convenience function for deallocating the concurrent iterator
  /// parallelism level
  static void free_iterator_parallelism(ParallelLibrary& parallel_lib);

  /// convenience function for allocation of an iterator and (parallel)
  /// initialization of its comms
  static void init_iterator(ProblemDescDB& problem_db, Iterator& the_iterator,
			    Model& the_model, const ParallelLevel& pl);
  /// convenience function for lightweight allocation of an iterator
  /// and (parallel) initialization of its comms
  static void init_iterator(const String& method_string, Iterator& the_iterator,
			    Model& the_model, const ParallelLevel& pl);

  /// Convenience function for invoking an iterator and managing parallelism.
  /// This version omits communicator repartitioning. Function must be public
  /// due to use by MINLPNode.
  static void run_iterator(Iterator& the_iterator, const ParallelLevel& pl);

  /// convenience function for deallocating comms after running an iterator
  static void free_iterator(Iterator& the_iterator, const ParallelLevel& pl);

  //
  //- Heading: Member functions
  //

  /// convenience function for initializing iterator communicators, setting
  /// parallel configuration attributes, and managing outputs and restart.
  void init_iterator_parallelism(int max_concurrency,
				 short default_config = PUSH_DOWN);

  /// convenience function for deallocating the concurrent iterator
  /// parallelism level
  void free_iterator_parallelism();

  /// short convenience function for distributing control among
  /// master_dynamic_schedule_iterators(), serve_iterators(), and
  /// peer_static_schedule_iterators()
  void schedule_iterators(Iterator& meta_iterator, Iterator& sub_iterator);
  /// executed by the scheduler master to manage a dynamic schedule of
  /// iterator jobs among slave iterator servers
  void master_dynamic_schedule_iterators(Iterator& meta_iterator);
  /// executed by the scheduler master to terminate slave iterator servers
  void stop_iterator_servers();
  /// executed on the slave iterator servers to perform iterator jobs
  /// assigned by the scheduler master
  void serve_iterators(Iterator& meta_iterator, Iterator& sub_iterator);
  /// executed on iterator peers to manage a static schedule of iterator jobs
  void peer_static_schedule_iterators(Iterator& meta_iterator,
				      Iterator& sub_iterator);

  /// update paramsMsgLen and resultsMsgLen
  void iterator_message_lengths(int params_msg_len, int results_msg_len);

  /// determines if current processor is rank 0 of the parent comm
  bool lead_rank() const;

  //
  //- Heading: Data members
  //

  ParallelLibrary& parallelLib; ///< reference to the ParallelLibrary instance

  int   numIteratorJobs;     ///< number of iterator executions to schedule
  int   numIteratorServers;  ///< number of concurrent iterator partitions
  int   procsPerIterator;    ///< partition size request
  int   minProcsPerIterator; ///< lower bound on partition size
  int   iteratorCommRank;    ///< processor rank in iteratorComm
  int   iteratorCommSize;    ///< number of processors in iteratorComm
  int   iteratorServerId;    ///< identifier for an iterator server

  bool  messagePass;         ///< flag for message passing at si level
  short iteratorScheduling;  ///< {DEFAULT,MASTER,PEER}_SCHEDULING
  //int maxIteratorConcurrency; ///< max concurrency possible in meta-algorithm

private:

  //
  //- Heading: Convenience member functions
  //

  //
  //- Heading: Data members
  //

  int  paramsMsgLen; ///< length of MPI buffer for parameter input instance(s)
  int resultsMsgLen; ///< length of MPI buffer for results  output instance(s)
};


//inline IteratorScheduler::IteratorScheduler():
//  numIteratorServers(1), numIteratorJobs(1), maxIteratorConcurrency(1)
//{ }


inline IteratorScheduler::~IteratorScheduler()
{ }


inline bool IteratorScheduler::lead_rank() const
{
  return
    ( iteratorCommRank == 0 &&
      ( ( iteratorScheduling == MASTER_SCHEDULING && iteratorServerId == 0 ) ||
	( iteratorScheduling == PEER_SCHEDULING   && iteratorServerId == 1 )));
}


inline void IteratorScheduler::
iterator_message_lengths(int params_msg_len, int results_msg_len)
{ paramsMsgLen = params_msg_len; resultsMsgLen = results_msg_len; }

} // namespace Dakota

#endif
