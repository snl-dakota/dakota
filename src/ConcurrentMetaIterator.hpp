/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       ConcurrentMetaIterator
//- Description: Class for concurrent iteration: pareto_set and multi_start
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: ConcurrentMetaIterator.hpp 6492 2009-12-19 00:04:28Z briadam $

#ifndef CONCURRENT_META_ITERATOR_H
#define CONCURRENT_META_ITERATOR_H

#include "MetaIterator.hpp"
#include "dakota_data_io.hpp"
#include "ParamResponsePair.hpp"


namespace Dakota {

/// Meta-iterator for multi-start iteration or pareto set optimization.

/** This meta-iterator maintains two concurrent iterator capabilities.
    First, a general capability for running an iterator multiple times
    from different starting points is provided (often used for
    multi-start optimization, but not restricted to optimization).
    Second, a simple capability for mapping the "pareto frontier" (the
    set of optimal solutions in multiobjective formulations) is
    provided.  This pareto set is mapped through running an optimizer
    multiple times for different sets of multiobjective weightings. */

class ConcurrentMetaIterator: public MetaIterator
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
  ConcurrentMetaIterator(ProblemDescDB& problem_db);
  /// alternate constructor
  ConcurrentMetaIterator(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~ConcurrentMetaIterator();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void pre_run();
  /// Performs the concurrent iteration by executing selectedIterator on
  /// iteratedModel multiple times in parallel for different parameter sets
  void core_run();
  void print_results(std::ostream& s);

  void derived_init_communicators(ParLevLIter pl_iter);
  void derived_set_communicators(ParLevLIter pl_iter);
  void derived_free_communicators(ParLevLIter pl_iter);

  IntIntPair estimate_partition_bounds();

  void initialize_iterator(int job_index);
  void pack_parameters_buffer(MPIPackBuffer& send_buffer, int job_index);
  void unpack_parameters_initialize(MPIUnpackBuffer& recv_buffer,
				    int job_index);
  void pack_results_buffer(MPIPackBuffer& send_buffer, int job_index);
  void unpack_results_buffer(MPIUnpackBuffer& recv_buffer, int job_index);
  void update_local_results(int job_index);

  const Model& algorithm_space_model() const;

private:

  //
  //- Heading: Convenience member functions
  //

  /// called by unpack_parameters_initialize(MPIUnpackBuffer) and
  /// initialize_iterator(int) to update iteratedModel and selectedIterator
  void initialize_iterator(const RealVector& param_set);

  /// initialize the iterated Model prior to Iterator instantiation
  /// and define param_set_len
  void initialize_model();

  //
  //- Heading: Data members
  //

  Iterator selectedIterator; ///< the iterator selected for concurrent iteration

  /// the initial continuous variables for restoring the starting
  /// point in the Pareto set minimization
  RealVector initialPt;

  /// an array of parameter set vectors (either multistart variable
  /// sets or pareto multi-objective/least squares weighting sets) to
  /// be performed.
  RealVectorArray parameterSets;
  /// length of each of the parameter sets associated with an iterator job
  /// (number of continuous variables for MULTI_START, number of objective
  /// fns for PARETO_SET)
  int paramSetLen;
  /// number of randomly-generated parameter sets to evaluate
  int numRandomJobs;
  /// seed for random number generator for random samples
  int randomSeed;
  /// 1-d array of ParamResponsePair results corresponding to numIteratorJobs
  PRPArray prpResults;
};


inline const Model& ConcurrentMetaIterator::algorithm_space_model() const
{ return iteratedModel; }


inline IntIntPair ConcurrentMetaIterator::estimate_partition_bounds()
{
  // Note: ConcurrentMetaIterator::derived_init_communicators() calls
  // IteratorScheduler::configure() to estimate_partition_bounds() on the
  // subIterator, not the MetaIterator.  When ConcurrentMetaIterator is a
  // sub-iterator, we augment the subIterator concurrency with the MetaIterator
  // concurrency.  [Thus, this is not redundant with configure().]

  // This function is already rank protected as far as partitioning has occurred
  // to this point.  However, this call may precede derived_init_communicators
  // when the ConcurrentMetaIterator is a sub-iterator.
  iterSched.construct_sub_iterator(probDescDB, selectedIterator, iteratedModel,
    probDescDB.get_string("method.sub_method_pointer"),
    probDescDB.get_string("method.sub_method_name"),
    probDescDB.get_string("method.sub_model_pointer"));
  IntIntPair min_max, si_min_max = selectedIterator.estimate_partition_bounds();

  // now apply scheduling data for this level (recursion is complete)
  min_max.first = ProblemDescDB::min_procs_per_level(si_min_max.first,
    iterSched.procsPerIterator, iterSched.numIteratorServers);
  min_max.second = ProblemDescDB::max_procs_per_level(si_min_max.second,
    iterSched.procsPerIterator, iterSched.numIteratorServers,
    iterSched.iteratorScheduling, 1, false, maxIteratorConcurrency);
  return min_max;
}


inline void ConcurrentMetaIterator::initialize_model()
{
  if (methodName == PARETO_SET) {
    paramSetLen = probDescDB.get_sizet("responses.num_objective_functions");
    // define dummy weights to trigger model recasting in iterator construction
    // (replaced at run-time with weight sets from specification)
    if (iteratedModel.primary_response_fn_weights().empty()) {
      RealVector initial_wts(paramSetLen, false);
      initial_wts = 1./(Real)paramSetLen;
      iteratedModel.primary_response_fn_weights(initial_wts); // trigger recast
    }
  }
  else
    paramSetLen = iteratedModel.cv();
}


inline void ConcurrentMetaIterator::
initialize_iterator(const RealVector& param_set)
{
  if (methodName == MULTI_START)
    iteratedModel.continuous_variables(param_set);
  else {
    iteratedModel.continuous_variables(initialPt); // reset
    iteratedModel.primary_response_fn_weights(param_set);
  }
}


inline void ConcurrentMetaIterator::initialize_iterator(int job_index)
{ initialize_iterator(parameterSets[job_index]); }


inline void ConcurrentMetaIterator::
pack_parameters_buffer(MPIPackBuffer& send_buffer, int job_index)
{ send_buffer << parameterSets[job_index]; }


inline void ConcurrentMetaIterator::
unpack_parameters_initialize(MPIUnpackBuffer& recv_buffer, int job_index)
{
  RealVector param_set;
  recv_buffer >> param_set; // job_index can be ignored
  initialize_iterator(param_set);
}


inline void ConcurrentMetaIterator::
pack_results_buffer(MPIPackBuffer& send_buffer, int job_index)
{ send_buffer << prpResults[job_index]; }


inline void ConcurrentMetaIterator::
unpack_results_buffer(MPIUnpackBuffer& recv_buffer, int job_index)
{ recv_buffer >> prpResults[job_index]; }


inline void ConcurrentMetaIterator::update_local_results(int job_index)
{
  prpResults[job_index]
    = ParamResponsePair(selectedIterator.variables_results(),
			iteratedModel.interface_id(),
			selectedIterator.response_results(),
			job_index+1); // deep copy
}

} // namespace Dakota

#endif
