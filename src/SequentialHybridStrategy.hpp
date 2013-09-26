/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SequentialHybridStrategy
//- Description: A hybrid strategy that sequentially invokes several iterators
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: SequentialHybridStrategy.hpp 7029 2010-10-22 00:17:02Z mseldre $

#ifndef SEQUENTIAL_HYBRID_STRATEGY_H
#define SEQUENTIAL_HYBRID_STRATEGY_H

#include "dakota_data_types.hpp"
#include "HybridStrategy.hpp"


namespace Dakota {


/// Strategy for sequential hybrid minimization using multiple
/// optimization and nonlinear least squares methods on multiple
/// models of varying fidelity.

/** The sequential hybrid minimization strategy has two approaches:
    (1) the non-adaptive sequential hybrid runs one method to
    completion, passes its best results as the starting point for a
    subsequent method, and continues this succession until all methods
    have been executed (the stopping rules are controlled internally
    by each minimizer), and (2) the adaptive sequential hybrid uses
    adaptive stopping rules for the minimizers that are controlled
    externally by the strategy.  Note that while the strategy is
    targeted at minimizers, any iterator may be used so long as it
    defines the notion of a final solution which can be passed as the
    starting point for subsequent iterators. */

class SequentialHybridStrategy: public HybridStrategy
{
public:
  
  //
  //- Heading: Constructors and destructor
  //

  SequentialHybridStrategy(ProblemDescDB& problem_db); ///< constructor
  ~SequentialHybridStrategy();                         ///< destructor

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  /// Performs the hybrid minimization strategy by executing multiple
  /// iterators on different models of varying fidelity
  void run_strategy();

  /// return the final solution from selectedIterators (variables)
  const Variables& variables_results() const;
  /// return the final solution from selectedIterators (response)
  const Response&  response_results() const;

  void initialize_iterator(int job_index);
  void pack_parameters_buffer(MPIPackBuffer& send_buffer, int job_index);
  void unpack_parameters_buffer(MPIUnpackBuffer& recv_buffer);
  void pack_results_buffer(MPIPackBuffer& send_buffer, int job_index);
  void unpack_results_buffer(MPIUnpackBuffer& recv_buffer, int job_index);
  void update_local_results(int job_index);

private:

  //
  //- Heading: Convenience member functions
  //

  void run_sequential();          ///< run a sequential hybrid
  void run_sequential_adaptive(); ///< run a sequential adaptive hybrid

  /// convert num_sets and job_index into a start_index and job_size for
  /// extraction from parameterSets
  void partition_sets(size_t num_sets, int job_index, size_t& start_index,
		      size_t& job_size);
  /// extract partial_param_sets from parameterSets based on job_index
  void extract_parameter_sets(int job_index,VariablesArray& partial_param_sets);
  /// update the partial set of final results from the local iterator execution
  void update_local_results(PRPArray& prp_results, int job_id);
  /// called by unpack_parameters_buffer(MPIUnpackBuffer) and
  /// initialize_iterator(int) to update the active Model and Iterator
  void initialize_iterator(const VariablesArray& param_sets);

  //
  //- Heading: Data members
  //

  String hybridType; ///< sequential or sequential_adaptive

  /// hybrid sequence counter: 0 to numIterators-1
  size_t seqCount;
  /// the amount of progress made in a single iterator++ cycle within
  /// a sequential adaptive hybrid
  Real progressMetric;
  /// when the progress metric falls below this threshold, the
  /// sequential adaptive hybrid switches to the next method
  Real progressThreshold;

  /// 2-D array of results corresponding to numIteratorJobs, one set
  /// of results per job (iterators may return multiple final solutions)
  PRP2DArray prpResults;
  /// 1-D array of variable starting points for the iterator jobs
  VariablesArray parameterSets;
};


inline const Variables& SequentialHybridStrategy::variables_results() const
{ return selectedIterators[numIterators-1].variables_results(); }


inline const Response& SequentialHybridStrategy::response_results() const
{ return selectedIterators[numIterators-1].response_results(); }


inline void SequentialHybridStrategy::
partition_sets(size_t num_sets, int job_index, size_t& start_index,
	       size_t& job_size)
{
  size_t set_remainder = num_sets % numIteratorJobs;
  job_size = num_sets / numIteratorJobs;
  start_index = job_index * job_size;
  if (set_remainder) { // allocate 1 addtnl job to first set_remainder jobs
    if (set_remainder > job_index) { // this job is offset and grown
      start_index += job_index;
      ++job_size;
    }
    else // this job is only offset
      start_index += set_remainder;
  }
}


/** This convenience function is executed on an iterator master (static
    scheduling) or a strategy master (self scheduling) at run initialization
    time and has access to the full parameterSets array (this is All-Reduced
    for all peers at the completion of each cycle in run_sequential()). */
inline void SequentialHybridStrategy::
extract_parameter_sets(int job_index, VariablesArray& partial_param_sets)
{
  size_t start_index, job_size;
  partition_sets(parameterSets.size(), job_index, start_index, job_size);
  if (partial_param_sets.size() != job_size)
    partial_param_sets.resize(job_size);
  for (size_t i=0; i<job_size; ++i)
    partial_param_sets[i] = parameterSets[start_index+i];
}


inline void SequentialHybridStrategy::update_local_results(int job_index)
{ update_local_results(prpResults[job_index], job_index+1); }


inline void SequentialHybridStrategy::
initialize_iterator(const VariablesArray& param_sets)
{
  // Note: in current usage, we update an iterator with either:
  // > 1 set from parameterSets (numIteratorJobs == parameterSets.size())
  // > all of parameterSets (numIteratorJobs == 1)
  size_t num_param_sets = param_sets.size();
  if (num_param_sets == 1)
    userDefinedModels[seqCount].active_variables(param_sets[0]);
  else if (selectedIterators[seqCount].accepts_multiple_points())
    selectedIterators[seqCount].initial_points(param_sets);
  else {
    std::cerr << "Error: bad parameter sets array in SequentialHybridStrategy::"
	      << "initialize_iterator()" << std::endl;
    abort_handler(-1);
  }
}


inline void SequentialHybridStrategy::initialize_iterator(int job_index)
{
  if (seqCount) { // else default initialization is used
    VariablesArray partial_param_sets;
    extract_parameter_sets(job_index, partial_param_sets);
    initialize_iterator(partial_param_sets);
  }
}


inline void SequentialHybridStrategy::
pack_parameters_buffer(MPIPackBuffer& send_buffer, int job_index)
{
  if (seqCount) { // else default initialization is used
    VariablesArray partial_param_sets;
    extract_parameter_sets(job_index, partial_param_sets);
    send_buffer << partial_param_sets;
  }
}


inline void SequentialHybridStrategy::
unpack_parameters_buffer(MPIUnpackBuffer& recv_buffer)
{
  if (seqCount) { // else default initialization is used
    VariablesArray param_sets;
    recv_buffer >> param_sets;
    initialize_iterator(param_sets);
  }
}


inline void SequentialHybridStrategy::
pack_results_buffer(MPIPackBuffer& send_buffer, int job_index)
{ send_buffer << prpResults[job_index]; }


inline void SequentialHybridStrategy::
unpack_results_buffer(MPIUnpackBuffer& recv_buffer, int job_index)
{ recv_buffer >> prpResults[job_index]; }

} // namespace Dakota

#endif
