/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SeqHybridMetaIterator
//- Description: A hybrid meta-iterator that sequentially invokes several
//-              methods, initializing subsequent iterators with prior results
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: SeqHybridMetaIterator.hpp 7029 2010-10-22 00:17:02Z mseldre $

#ifndef SEQ_HYBRID_META_ITERATOR_H
#define SEQ_HYBRID_META_ITERATOR_H

#include "MetaIterator.hpp"


namespace Dakota {


/// Method for sequential hybrid iteration using multiple
/// optimization and nonlinear least squares methods on multiple
/// models of varying fidelity.

/** Sequential hybrid meta-iteration supports two approaches: (1) the
    non-adaptive sequential hybrid runs one method to completion,
    passes its best results as the starting point for a subsequent
    method, and continues this succession until all methods have been
    executed (the stopping rules are controlled internally by each
    iterator), and (2) the adaptive sequential hybrid uses adaptive
    stopping rules for the iterators that are controlled externally by
    this method.  Any iterator may be used so long as it defines the
    notion of a final solution which can be passed as starting data
    for subsequent iterators. */

class SeqHybridMetaIterator: public MetaIterator
{
  //
  //- Heading: Friends
  //

  /// protect scheduler callback functions from general access
  friend class IteratorScheduler;

public:
  
  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  SeqHybridMetaIterator(ProblemDescDB& problem_db);
  /// alternate constructor
  SeqHybridMetaIterator(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~SeqHybridMetaIterator();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  /// Performs the hybrid iteration by executing a sequence of iterators,
  /// using a similar sequence of models that may vary in fidelity
  void core_run();
  void print_results(std::ostream& s);

  void derived_init_communicators(ParLevLIter pl_iter);
  void derived_set_communicators(ParLevLIter pl_iter);
  void derived_free_communicators(ParLevLIter pl_iter);

  IntIntPair estimate_partition_bounds();

  /// return the final solution from selectedIterators (variables)
  const Variables& variables_results() const;
  /// return the final solution from selectedIterators (response)
  const Response&  response_results() const;

  void initialize_iterator(int job_index);
  void pack_parameters_buffer(MPIPackBuffer& send_buffer, int job_index);
  void unpack_parameters_initialize(MPIUnpackBuffer& recv_buffer,
				    int job_index);
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
  /// called by unpack_parameters_initialize(MPIUnpackBuffer) and
  /// initialize_iterator(int) to update the active Model and Iterator
  void initialize_iterator(const VariablesArray& param_sets);

  //
  //- Heading: Data members
  //

  String   seqHybridType; ///< empty (default) or "adaptive"

  /// the list of method pointer or method name identifiers
  StringArray methodStrings;
  /// the list of model pointer identifiers for method identification by name
  StringArray modelStrings;

  /// use of lightweight Iterator construction by name
  bool lightwtMethodCtor;
  /// use of constructor that enforces use of a single passed Model
  bool singlePassedModel;

  /// the set of iterators, one for each entry in methodStrings
  IteratorArray selectedIterators;
  /// the set of models, one for each iterator (if not lightweight construction)
  ModelArray selectedModels;

  /// hybrid sequence counter: 0 to numIterators-1
  size_t seqCount;

  /// when the progress metric falls below this threshold, the
  /// sequential adaptive hybrid switches to the next method
  Real progressThreshold;

  /// 2-D array of results corresponding to numIteratorJobs, one set
  /// of results per job (iterators may return multiple final solutions)
  PRP2DArray prpResults;
  /// 1-D array of variable starting points for the iterator jobs
  VariablesArray parameterSets;
};


inline IntIntPair SeqHybridMetaIterator::estimate_partition_bounds()
{
  int min_procs = INT_MAX, max_procs = 0;      IntIntPair min_max;
  size_t i, num_meth = selectedIterators.size(); String empty_str;
  for (i=0; i<num_meth; ++i)  {
    Model& model = (singlePassedModel) ? iteratedModel : selectedModels[i];
    if (lightwtMethodCtor)
      iterSched.construct_sub_iterator(probDescDB, selectedIterators[i], model,
				       empty_str, methodStrings[i], // ptr, name
				       modelStrings[i]); // ptr
    else
      iterSched.construct_sub_iterator(probDescDB, selectedIterators[i], model,
				       methodStrings[i], empty_str, empty_str);

    min_max = selectedIterators[i].estimate_partition_bounds();
    if (min_max.first  < min_procs) min_procs = min_max.first;
    if (min_max.second > max_procs) max_procs = min_max.second;
  }

  // now apply scheduling data for this level (recursion is complete)
  min_max.first = ProblemDescDB::min_procs_per_level(min_procs,
    iterSched.procsPerIterator, iterSched.numIteratorServers);
  min_max.second = ProblemDescDB::max_procs_per_level(max_procs,
    iterSched.procsPerIterator, iterSched.numIteratorServers,
    iterSched.iteratorScheduling, 1, false, maxIteratorConcurrency);
  return min_max;
}


inline const Variables& SeqHybridMetaIterator::variables_results() const
{ return selectedIterators[methodStrings.size()-1].variables_results(); }


inline const Response& SeqHybridMetaIterator::response_results() const
{ return selectedIterators[methodStrings.size()-1].response_results(); }


inline void SeqHybridMetaIterator::
partition_sets(size_t num_sets, int job_index, size_t& start_index,
	       size_t& job_size)
{
  size_t set_remainder = num_sets % iterSched.numIteratorJobs;
  job_size = num_sets / iterSched.numIteratorJobs;
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


/** This convenience function is executed on an iterator master
    (static scheduling) or a meta-iterator master (self scheduling) at
    run initialization time and has access to the full parameterSets
    array (this is All-Reduced for all peers at the completion of each
    cycle in run_sequential()). */
inline void SeqHybridMetaIterator::
extract_parameter_sets(int job_index, VariablesArray& partial_param_sets)
{
  size_t start_index, job_size;
  partition_sets(parameterSets.size(), job_index, start_index, job_size);
  if (partial_param_sets.size() != job_size)
    partial_param_sets.resize(job_size);
  for (size_t i=0; i<job_size; ++i)
    partial_param_sets[i] = parameterSets[start_index+i];
}


inline void SeqHybridMetaIterator::update_local_results(int job_index)
{ update_local_results(prpResults[job_index], job_index+1); }


inline void SeqHybridMetaIterator::
initialize_iterator(const VariablesArray& param_sets)
{
  // BMA TODO: This mixed use of pushing data at the Iterator
  // vs. Models likely indicates we should standardize on pushing to
  // the sub-iterators instead of maintaining a handle to the Model.
  // The Iterator would then have to manage any recursive updates of
  // its underlying models.  In particular, this could break if the
  // Models associated with two Iterators have difference scaling.

  // Note: in current usage, we update an iterator with either:
  // > 1 set from parameterSets (numIteratorJobs == parameterSets.size())
  // > all of parameterSets (numIteratorJobs == 1)
  size_t num_param_sets = param_sets.size();
  if (num_param_sets == 1)
    selectedModels[seqCount].active_variables(param_sets[0]);
  else if (selectedIterators[seqCount].accepts_multiple_points())
    selectedIterators[seqCount].initial_points(param_sets);
  else {
    std::cerr << "Error: bad parameter sets array in SeqHybridMetaIterator::"
	      << "initialize_iterator()" << std::endl;
    abort_handler(-1);
  }
}


inline void SeqHybridMetaIterator::initialize_iterator(int job_index)
{
  if (seqCount) { // else default initialization is used
    VariablesArray partial_param_sets;
    extract_parameter_sets(job_index, partial_param_sets);
    initialize_iterator(partial_param_sets);
  }
}


inline void SeqHybridMetaIterator::
pack_parameters_buffer(MPIPackBuffer& send_buffer, int job_index)
{
  if (seqCount) { // else default initialization is used
    VariablesArray partial_param_sets;
    extract_parameter_sets(job_index, partial_param_sets);
    send_buffer << partial_param_sets;
  }
}


inline void SeqHybridMetaIterator::
unpack_parameters_initialize(MPIUnpackBuffer& recv_buffer, int job_index)
{
  if (seqCount) { // else default initialization is used
    VariablesArray param_sets;
    recv_buffer >> param_sets; // job_index can be ignored
    initialize_iterator(param_sets);
  }
}


inline void SeqHybridMetaIterator::
pack_results_buffer(MPIPackBuffer& send_buffer, int job_index)
{ send_buffer << prpResults[job_index]; }


inline void SeqHybridMetaIterator::
unpack_results_buffer(MPIUnpackBuffer& recv_buffer, int job_index)
{ recv_buffer >> prpResults[job_index]; }

} // namespace Dakota

#endif
