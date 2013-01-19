/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        Strategy
//- Description:  Abstract base class to logically represent a variety
//-               of Dakota strategy objects in a very generic fashion.
//- Owner:        Mike Eldred
//- Version: $Id: DakotaStrategy.hpp 6492 2009-12-19 00:04:28Z briadam $

#ifndef DAKOTA_STRATEGY_H
#define DAKOTA_STRATEGY_H

#include "data_types.hpp"
#include "ParamResponsePair.hpp"


/// The primary namespace for DAKOTA

/** The Dakota namespace encapsulates the core classes of the DAKOTA
    framework and prevents name clashes with third-party libraries
    from methods and packages.  The C++ source files defining these
    core classes reside in Dakota/src as *.[CH]. */

namespace Dakota {

class ProblemDescDB;
class ParallelLibrary;


/// Base class for the strategy class hierarchy.

/** The Strategy class is the base class for the class hierarchy
    providing the top level control in DAKOTA.  The strategy is
    responsible for creating and managing iterators and models.  For
    memory efficiency and enhanced polymorphism, the strategy
    hierarchy employs the "letter/envelope idiom" (see Coplien
    "Advanced C++", p. 133), for which the base class (Strategy)
    serves as the envelope and one of the derived classes (selected in
    Strategy::get_strategy()) serves as the letter. */

class Strategy
{
public:

  //
  //- Heading: Constructors, destructor, assignment operator
  //

  /// default constructor
  Strategy();
  /// envelope constructor
  Strategy(ProblemDescDB& problem_db);
  /// copy constructor
  Strategy(const Strategy& strat);

  /// destructor
  virtual ~Strategy();

  /// assignment operator
  Strategy operator=(const Strategy& strat);

  //
  //- Heading: Virtual functions
  //

  /// the run function for the strategy: invoke the iterator(s) on
  /// the model(s).  Called from main.cpp.
  virtual void run_strategy();

  /// return the final strategy solution (variables)
  virtual const Variables& variables_results() const;
  /// return the final strategy solution (response)
  virtual const Response&  response_results() const;

  //
  //- Heading: Member functions
  //

  /// returns the problem description database (probDescDB)
  ProblemDescDB& problem_description_db() const;

protected:

  //
  //- Heading: Constructors
  //

  /// constructor initializes the base class part of letter classes
  /// (BaseConstructor overloading avoids infinite recursion in the
  /// derived class constructors - Coplien, p. 139)
  Strategy(BaseConstructor, ProblemDescDB& problem_db);

  //
  //- Heading: Virtual functions
  //

  /// initialize the iterator about to be executed within a parallel iterator
  /// scheduling function (serve_iterators() or static_schedule_iterators())
  virtual void initialize_iterator(int index);
  /// pack a send_buffer for assigning an iterator job to a server
  virtual void pack_parameters_buffer(MPIPackBuffer& send_buffer,
				      int job_index);
  /// unpack a recv_buffer for accepting an iterator job from the scheduler
  virtual void unpack_parameters_buffer(MPIUnpackBuffer& recv_buffer);
  /// pack a send_buffer for returning iterator results from a server
  virtual void pack_results_buffer(MPIPackBuffer& send_buffer, int job_index);
  /// unpack a recv_buffer for accepting iterator results from a server
  virtual void unpack_results_buffer(MPIUnpackBuffer& recv_buffer,
				     int job_index);
  /// update local PRP results arrays with current iteration results
  virtual void update_local_results(int job_index);

  //
  //- Heading: Member functions
  //

  /// convenience function for initializing iterator communicators, setting
  /// parallel configuration attributes, and managing outputs and restart.
  void init_iterator_parallelism();

  /// convenience function for allocating comms prior to running an iterator
  void init_iterator(Iterator& the_iterator, Model& the_model);
  /// Convenience function for invoking an iterator and managing parallelism.
  /// This version omits communicator repartitioning. Function must be public
  /// due to use by MINLPNode.
  void run_iterator(Iterator& the_iterator, Model& the_model);
  /// convenience function for deallocating comms after running an iterator
  void free_iterator(Iterator& the_iterator, Model& the_model);

  /// short convenience function for distributing control among
  /// self_schedule_iterators(), serve_iterators(), and
  /// static_schedule_iterators()
  void schedule_iterators(Iterator& the_iterator, Model& the_model);
  /// executed by the strategy master to self-schedule iterator jobs
  /// among slave iterator servers (called by derived run_strategy())
  void self_schedule_iterators(Model& the_model);
  /// executed on the slave iterator servers to perform iterator jobs
  /// assigned by the strategy master (called by derived run_strategy())
  void serve_iterators(Iterator& the_iterator, Model& the_model);
  /// executed on iterator peers to statically schedule iterator jobs
  /// (called by derived run_strategy())
  void static_schedule_iterators(Iterator& the_iterator, Model& the_model);

  //
  //- Heading: Data
  //

  /// class member reference to the problem description database
  ProblemDescDB& probDescDB;

  /// class member reference to the parallel library
  ParallelLibrary& parallelLib;

  /// type of strategy: single_method, hybrid, multi_start, or pareto_set.
  String strategyName;

  bool stratIterMessagePass; ///< flag for message passing at si level
  bool stratIterDedMaster;   ///< flag for dedicated master part. at si level

  int worldRank; ///< processor rank in MPI_COMM_WORLD
  int worldSize; ///< size of MPI_COMM_WORLD

  int iteratorCommRank;   ///< processor rank in iteratorComm
  int iteratorCommSize;   ///< number of processors in iteratorComm
  int numIteratorServers; ///< number of concurrent iterator partitions
  int iteratorServerId;   ///< identifier for an iterator server

  bool   graph2DFlag;     ///< flag for using 2D graphics plots
  bool   tabularDataFlag; ///< flag for file tabulation of graphics data
  String tabularDataFile; ///< filename for tabulation of graphics data
  
  bool resultsOutputFlag;         ///< whether to output results data
  std::string resultsOutputFile;  ///< filename for results data

  int maxConcurrency;  ///< maximum iterator concurrency possible in Strategy
  int numIteratorJobs; ///< number of iterator executions to schedule

  int  paramsMsgLen; ///< length of MPI buffer for parameter input instance(s)
  int resultsMsgLen; ///< length of MPI buffer for results output instance(s)

private:

  //
  //- Heading: Member functions
  //

  /// Used by the envelope to instantiate the correct letter class.
  Strategy* get_strategy();

  //
  //- Heading: Data
  //

  /// pointer to the letter (initialized only for the envelope)
  Strategy* strategyRep;
  /// number of objects sharing strategyRep
  int referenceCount;
};


inline ProblemDescDB& Strategy::problem_description_db() const
{ return (strategyRep) ? strategyRep->probDescDB : probDescDB; }

} // namespace Dakota

#endif
