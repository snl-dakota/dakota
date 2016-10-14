/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       EmbedHybridMetaIterator
//- Description: A meta-iterator for tightly-coupled global-local hybrids
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: EmbedHybridMetaIterator.hpp 6492 2009-12-19 00:04:28Z briadam $

#ifndef EMBED_HYBRID_META_ITERATOR_H
#define EMBED_HYBRID_META_ITERATOR_H

#include "MetaIterator.hpp"


namespace Dakota {


/// Meta-iterator for closely-coupled hybrid iteration, typically
/// involving the embedding of local search methods within global
/// search methods.

/** This meta-iterator uses multiple methods in close coordination,
    generally using a local search minimizer repeatedly within a
    global minimizer (the local search minimizer refines candidate
    minima which are fed back to the global minimizer). */

class EmbedHybridMetaIterator: public MetaIterator
{
public:
  
  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  EmbedHybridMetaIterator(ProblemDescDB& problem_db);
  /// alternate constructor
  EmbedHybridMetaIterator(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~EmbedHybridMetaIterator();
    
protected:
  
  //
  //- Heading: Member functions
  //

  /// Performs the hybrid iteration by executing global and local
  /// iterators, using a set of models that may vary in fidelity
  void core_run();

  void derived_init_communicators(ParLevLIter pl_iter);
  void derived_set_communicators(ParLevLIter pl_iter);
  void derived_free_communicators(ParLevLIter pl_iter);

  IntIntPair estimate_partition_bounds();

  /// return the final solution from the embedded hybrid (variables)
  const Variables& variables_results() const;
  /// return the final solution from the embedded hybrid (response)
  const Response&  response_results() const;

private:

  //
  //- Heading: Data members
  //

  /// the top-level outer iterator (e.g., global minimizer)
  Iterator globalIterator;
  /// the model employed by the top-level outer iterator
  Model globalModel;

  /// the inner iterator (e.g., local minimizer)
  Iterator localIterator;
  /// the model employed by the inner iterator
  Model localModel;

  /// use of constructor that enforces use of a single passed Model
  bool singlePassedModel;

  /// the probability of running a local search refinement within
  /// phases of the global minimization for tightly-coupled hybrids
  Real localSearchProb;
};


inline IntIntPair EmbedHybridMetaIterator::estimate_partition_bounds()
{
  // Note: EmbedHybridMetaIterator::derived_init_communicators() calls
  // IteratorScheduler::configure() to estimate_partition_bounds() on the
  // subIterator, not the MetaIterator.  When EmbedHybridMetaIterator is a
  // sub-iterator, we augment the subIterator concurrency with the MetaIterator
  // concurrency.  [Thus, this is not redundant with configure().]

  const String& global_method_ptr
    = probDescDB.get_string("method.hybrid.global_method_pointer");
  const String& global_model_ptr
    = probDescDB.get_string("method.hybrid.global_model_pointer");
  const String& local_method_ptr
    = probDescDB.get_string("method.hybrid.local_method_pointer");
  const String& local_model_ptr
    = probDescDB.get_string("method.hybrid.local_model_pointer");

  Model& global_model = (singlePassedModel) ? iteratedModel : globalModel;
  Model& local_model  = (singlePassedModel) ? iteratedModel :  localModel;

  iterSched.construct_sub_iterator(probDescDB, globalIterator, global_model,
    global_method_ptr,probDescDB.get_string("method.hybrid.global_method_name"),
    global_model_ptr);
  iterSched.construct_sub_iterator(probDescDB, localIterator, local_model,
    local_method_ptr, probDescDB.get_string("method.hybrid.local_method_name"),
    local_model_ptr);

  IntIntPair global_min_max = globalIterator.estimate_partition_bounds(),
    local_min_max = localIterator.estimate_partition_bounds(), min_max;
  int min_procs = std::min(global_min_max.first,  local_min_max.first),
      max_procs = std::max(global_min_max.second, local_min_max.second);

  // now apply scheduling data for this level (recursion is complete)
  min_max.first  = ProblemDescDB::min_procs_per_level(min_procs,
    iterSched.procsPerIterator,	iterSched.numIteratorServers);
  min_max.second = ProblemDescDB::max_procs_per_level(max_procs,
    iterSched.procsPerIterator, iterSched.numIteratorServers,
    iterSched.iteratorScheduling, 1, false, maxIteratorConcurrency);
  return min_max;
}


inline const Variables& EmbedHybridMetaIterator::variables_results() const
{ return globalIterator.variables_results(); }


inline const Response& EmbedHybridMetaIterator::response_results() const
{ return globalIterator.response_results(); }

} // namespace Dakota

#endif
