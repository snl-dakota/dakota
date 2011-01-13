/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DataStrategy
//- Description:
//-
//-
//- Owner:        Mike Eldred
//- Version: $Id: DataStrategy.H 6740 2010-04-30 16:12:00Z briadam $

#ifndef DATA_STRATEGY_H
#define DATA_STRATEGY_H

#include "system_defs.h"
#include "data_types.h"
#include "DakotaString.H"

namespace Dakota {


/// Body class for strategy specification data.

/** The DataStrategyRep class is used to contain the data from the
    strategy keyword specification.  Default values are managed in the
    DataStrategyRep constructor.  Data is public to avoid maintaining
    set/get functions, but is still encapsulated within ProblemDescDB
    since ProblemDescDB::strategySpec is private (a similar approach
    is used with SurrogateDataPoint objects contained in
    Dakota::Approximation). */

class DataStrategyRep
{
  //
  //- Heading: Friends
  //

  /// the handle class can access attributes of the body class directly
  friend class DataStrategy;

public:

  //
  //- Heading: Data
  //

  /// the strategy selection: hybrid, multi_start, pareto_set, or single_method
  String strategyType;
  /// flags use of graphics by the strategy (from the \c graphics
  /// specification in \ref StratIndControl)
  bool graphicsFlag;
  /// flags tabular data collection by the strategy (from
  /// the tabular_graphics_data specification in \ref StratIndControl)
  bool tabularDataFlag;
  /// the filename used for tabular data collection by the strategy (from
  /// the tabular_graphics_file specification in \ref StratIndControl)
  String tabularDataFile;
  /// output precision for tabular and screen output
  int outputPrecision;
  /// number of servers for concurrent iterator parallelism (from
  /// the iterator_servers specification in \ref StratIndControl)
  int iteratorServers;
  /// type of scheduling (self or static) used in concurrent
  /// iterator parallelism (from the \c iterator_self_scheduling and
  /// \c iterator_static_scheduling specifications in \ref StratIndControl)
  String iteratorScheduling;
  /// method identifier for the strategy (from the \c opt_method_pointer
  /// specifications in \ref StratParetoSet and \c method_pointer
  /// specifications in \ref StratSingle and \ref StratMultiStart)
  String methodPointer;

  /// array of methods for the sequential and collaborative hybrid
  /// optimization strategies (from the \c method_list specification
  /// in \ref StratHybrid)
  StringArray hybridMethodList;
  /// the type of hybrid optimization strategy: collaborative, embedded,
  /// sequential, or sequential_adaptive (from the \c collaborative,
  /// \c embedded, and \c sequential specifications in \ref StratHybrid)
  String hybridType;
  // progress threshold for sequential_adaptive hybrids (from the \c
  // progress_threshold specification in \ref StratHybrid)
  //Real hybridProgThresh;
  /// global method pointer for embedded hybrids (from the \c
  /// global_method_pointer specification in \ref StratHybrid)
  String hybridGlobalMethodPointer;
  /// local method pointer for embedded hybrids (from the \c
  /// local_method_pointer specification in \ref StratHybrid)
  String hybridLocalMethodPointer;
  /// local search probability for embedded hybrids (from the \c
  /// local_search_probability specification in \ref StratHybrid)
  Real hybridLSProb;

  /// number of random jobs to perform in the concurrent strategy
  /// (from the \c random_starts and \c random_weight_sets specifications
  /// in \ref StratMultiStart and \ref StratParetoSet)
  int concurrentRandomJobs;
  /// seed for the selected random jobs within the concurrent
  /// strategy (from the \c seed specification in \ref StratMultiStart
  /// and \ref StratParetoSet)
  int concurrentSeed;
  /// user-specified (i.e., nonrandom) parameter sets to evaluate in
  /// the concurrent strategy (from the \c starting_points and
  /// \c multi_objective_weight_sets specifications in \ref
  /// StratMultiStart and \ref StratParetoSet)
  RealVector concurrentParameterSets;

private:

  //
  //- Heading: Constructors, destructor, operators
  //

  DataStrategyRep();  ///< constructor
  ~DataStrategyRep(); ///< destructor

  //
  //- Heading: Member methods
  //

  /// write a DataStrategyRep object to an std::ostream
  void write(std::ostream& s) const;

  /// read a DataStrategyRep object from a packed MPI buffer
  void read(MPIUnpackBuffer& s);
  /// write a DataStrategyRep object to a packed MPI buffer
  void write(MPIPackBuffer& s) const;

  //
  //- Heading: Private data members
  //

  /// number of handle objects sharing this dataStrategyRep
  int referenceCount;
};


inline DataStrategyRep::~DataStrategyRep() { }


/// Handle class for strategy specification data.

/** The DataStrategy class is used to provide a memory management
    handle for the data in DataStrategyRep.  It is populated by
    IDRProblemDescDB::strategy_kwhandler() and is queried by the
    ProblemDescDB::get_<datatype>() functions.  A single DataStrategy
    object is maintained in ProblemDescDB::strategySpec. */

class DataStrategy
{
  //
  //- Heading: Friends
  //

  // the problem description database
  friend class ProblemDescDB;
  // the NIDR derived problem description database
  friend class NIDRProblemDescDB;

public:

  //
  //- Heading: Constructors, destructor, operators
  //

  DataStrategy();                                ///< constructor
  DataStrategy(const DataStrategy&);             ///< copy constructor
  ~DataStrategy();                               ///< destructor

  DataStrategy& operator=(const DataStrategy&); ///< assignment operator

  //
  //- Heading: Member methods
  //

  /// write a DataStrategy object to an std::ostream
  void write(std::ostream& s) const;

  /// read a DataStrategy object from a packed MPI buffer
  void read(MPIUnpackBuffer& s);

  /// write a DataStrategy object to a packed MPI buffer
  void write(MPIPackBuffer& s) const;

private:

  //
  //- Heading: Data
  //

  /// pointer to the body (handle-body idiom)
  DataStrategyRep* dataStratRep;
};


/// MPIPackBuffer insertion operator for DataStrategy
inline MPIPackBuffer& operator<<(MPIPackBuffer& s, const DataStrategy& data)
{ data.write(s); return s;}


/// MPIUnpackBuffer extraction operator for DataStrategy
inline MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s, DataStrategy& data)
{ data.read(s); return s;}


/// std::ostream insertion operator for DataStrategy
inline std::ostream& operator<<(std::ostream& s, const DataStrategy& data)
{ data.write(s); return s;}


inline void DataStrategy::write(std::ostream& s) const
{ dataStratRep->write(s); }


inline void DataStrategy::read(MPIUnpackBuffer& s)
{ dataStratRep->read(s); }


inline void DataStrategy::write(MPIPackBuffer& s) const
{ dataStratRep->write(s); }

} // namespace Dakota

#endif
