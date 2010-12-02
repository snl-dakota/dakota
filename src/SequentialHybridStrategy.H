/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SequentialHybridStrategy
//- Description: A hybrid strategy that sequentially invokes several iterators
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: SequentialHybridStrategy.H 7029 2010-10-22 00:17:02Z mseldre $

#ifndef SEQUENTIAL_HYBRID_STRATEGY_H
#define SEQUENTIAL_HYBRID_STRATEGY_H

#include "data_types.h"
#include "HybridStrategy.H"


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

  /// convert job_index into a start_index and job_size for insertion to or
  /// extraction from prpResults
  void partition_results(int job_index, size_t& start_index, size_t& job_size);
  /// extract partial_param_sets from prpResults based on job_index
  void extract_parameter_sets(int job_index,VariablesArray& partial_param_sets);
  /// extract partial_prp_results from prpResults based on job_index
  void extract_results_sets(int job_index, PRPArray& partial_prp_results);
  /// merge partial_prp_results into prpResults based on job_index
  void merge_results_sets(int job_index, PRPArray& partial_prp_results);
  /// update the partial set of final results from the local iterator execution
  void update_local_results(PRPArray& partial_prp_results, int job_id);
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
};


inline const Variables& SequentialHybridStrategy::variables_results() const
{ return selectedIterators[numIterators-1].variables_results(); }


inline const Response& SequentialHybridStrategy::response_results() const
{ return selectedIterators[numIterators-1].response_results(); }


inline void SequentialHybridStrategy::
partition_results(int job_index, size_t& start_index, size_t& job_size)
{
  size_t num_sets = prpResults.size(), 
    set_remainder = num_sets % numIteratorJobs;
  job_size = num_sets / numIteratorJobs;
  start_index = job_index * job_size;
  if (set_remainder) { // allocate 1 addtnl job to first set_remainder jobs
    if (set_remainder > job_index) { // this job is offset and grown
      start_index += job_index;
      job_size++;
    }
    else // this job is only offset
      start_index += set_remainder;
  }
}


/** This convenience function is executed on an iterator master (static
    scheduling) or a strategy master (self scheduling) at run
    initialization time and has access to the full prpResults array
    (this is All-Reduced for all peers at the completion of each cycle
    in run_sequential()). */
inline void SequentialHybridStrategy::
extract_parameter_sets(int job_index, VariablesArray& partial_param_sets)
{
  size_t start_index, job_size;
  partition_results(job_index, start_index, job_size);
  if (partial_param_sets.size() != job_size)
    partial_param_sets.resize(job_size);
  for (size_t i=0; i<job_size; ++i)
    partial_param_sets[i] = prpResults[start_index+i].prp_parameters();
}


/** This convenience function is executed on iterator servers 2 through n
    (peer partition) following iterator executions and prior to prpResults
    All-Reduce at bottom of run_sequential().  Therefore, some prpResults 
    entries may be empty. */
inline void SequentialHybridStrategy::
extract_results_sets(int job_index, PRPArray& partial_prp_results)
{
  size_t start_index, job_size;
  partition_results(job_index, start_index, job_size);
  if (partial_prp_results.size() != job_size)
    partial_prp_results.resize(job_size);
  for (size_t i=0; i<job_size; ++i)
    partial_prp_results[i] = prpResults[start_index+i];
}


/** This convenience function may be executed on either an iterator
    server (access to only a partial prpResults array) or the strategy
    master (access to full prpResults array). */
inline void SequentialHybridStrategy::
merge_results_sets(int job_index, PRPArray& partial_prp_results)
{
  size_t start_index, job_size;
  partition_results(job_index, start_index, job_size);
  copy_data_partial(partial_prp_results, prpResults, start_index);
}


inline void SequentialHybridStrategy::
update_local_results(PRPArray& partial_prp_results, int job_id)
{
  Iterator& curr_iterator = selectedIterators[seqCount];
  Model&    curr_model    = userDefinedModels[seqCount];
  // Analyzers do not currently support returns_multiple_points() since the
  // distinction between Hybrid sampling and Multistart sampling is that
  // the former performs fn evals and processes the data (and current
  // implementations of update_best() only log a single best point).
  if (curr_iterator.returns_multiple_points()) {
    const VariablesArray& vars_results
      = curr_iterator.variables_array_results();
    const ResponseArray& resp_results = curr_iterator.response_array_results();
    // workaround: some methods define vars_results, but not resp_results
    size_t num_vars_results = vars_results.size(),
           num_resp_results = resp_results.size(),
           num_results      = std::max(num_vars_results, num_resp_results);
    partial_prp_results.resize(num_results);
    Variables dummy_vars; Response dummy_resp;
    for (size_t i=0; i<num_results; ++i) {
      const Variables& vars = (num_vars_results) ? vars_results[i] : dummy_vars;
      const Response&  resp = (num_resp_results) ? resp_results[i] : dummy_resp;
      // need a deep copy for case where multiple instances of
      // best{Variables,Response}Array will be assimilated
      ParamResponsePair prp_star(vars, curr_model.interface_id(), resp, job_id);
      partial_prp_results[i] = prp_star;
    }
  }
  else {
    // need a deep copy for case where multiple instances of
    // best{Variables,Response}Array.front() will be assimilated
    ParamResponsePair prp_star(curr_iterator.variables_results(),
      curr_model.interface_id(), curr_iterator.response_results(), job_id);
    partial_prp_results.resize(1);
    partial_prp_results[0] = prp_star;
  }
}


inline void SequentialHybridStrategy::update_local_results(int job_index)
{
  PRPArray partial_prp_results;
  update_local_results(partial_prp_results, job_index+1);
  merge_results_sets(job_index, partial_prp_results);
}


inline void SequentialHybridStrategy::
initialize_iterator(const VariablesArray& param_sets)
{
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
{
  PRPArray partial_prp_results;
  extract_results_sets(job_index, partial_prp_results);
  send_buffer << partial_prp_results;
}


inline void SequentialHybridStrategy::
unpack_results_buffer(MPIUnpackBuffer& recv_buffer, int job_index)
{
  PRPArray partial_prp_results;
  recv_buffer >> partial_prp_results;
  merge_results_sets(job_index, partial_prp_results);
}

} // namespace Dakota

#endif
