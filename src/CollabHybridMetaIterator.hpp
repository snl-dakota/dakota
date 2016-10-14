/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       CollabHybridMetaIterator
//- Description: A hybrid method involving several collaborating iterators
//- Owner:       Patty Hough/John Siirola
//- Checked by:
//- Version: $Id: CollabHybridMetaIterator.hpp 6492 2009-12-19 00:04:28Z briadam $

#ifndef COLLAB_HYBRID_META_ITERATOR_H
#define COLLAB_HYBRID_META_ITERATOR_H

#include "MetaIterator.hpp"


namespace Dakota {


/// Meta-iterator for hybrid iteration using multiple collaborating
/// optimization and nonlinear least squares methods.

/** This meta-iterator has two approaches to hybrid iteration: (1)
    agent-based using the ABO framework; (2) nonagent-based using the
    HOPSPACK framework. */

class CollabHybridMetaIterator: public MetaIterator
{
public:
  
  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  CollabHybridMetaIterator(ProblemDescDB& problem_db);
  /// alternate constructor
  CollabHybridMetaIterator(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~CollabHybridMetaIterator();
    
protected:
  
  //
  //- Heading: Member functions
  //

  /// Performs the collaborative hybrid iteration
  void core_run();

  void derived_init_communicators(ParLevLIter pl_iter);
  void derived_set_communicators(ParLevLIter pl_iter);
  void derived_free_communicators(ParLevLIter pl_iter);

  IntIntPair estimate_partition_bounds();

  /// return the final solution from the collaborative iteration (variables)
  const Variables& variables_results() const;
  /// return the final solution from the collaborative iteration (response)
  const Response&  response_results() const;

private:

  //
  //- Heading: Data members
  //

  String hybridCollabType; ///< abo or hops

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
  /// the set of models, one for each iterator
  ModelArray selectedModels;

  // In this hybrid, the best results are not just the final results of the
  // final iterator (they must be captured independently of the iterators)

  Variables bestVariables; ///< best variables found in collaborative iteration
  Response  bestResponse;  ///< best response  found in collaborative iteration
};


inline IntIntPair CollabHybridMetaIterator::estimate_partition_bounds()
{
  int min_procs = INT_MAX, max_procs = 0;       IntIntPair min_max;
  size_t i, num_meth = selectedIterators.size();  String empty_str;
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


inline const Variables& CollabHybridMetaIterator::variables_results() const
{ return bestVariables; }


inline const Response& CollabHybridMetaIterator::response_results() const
{ return bestResponse; }

} // namespace Dakota

#endif
