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

  int estimate_min_processors();
  int estimate_max_processors();

  /// return the final solution from the collaborative iteration (variables)
  const Variables& variables_results() const;
  /// return the final solution from the collaborative iteration (response)
  const Response&  response_results() const;

private:

  //
  //- Heading: Data members
  //

  String hybridCollabType; ///< abo or hops
  StringArray  methodList; ///< the list of method name identifiers
  bool        lightwtCtor; ///< use of lightweight Iterator construction by name

  /// the set of iterators, one for each entry in methodList
  IteratorArray selectedIterators;
  /// the set of models, one for each iterator
  ModelArray selectedModels;

  // In this hybrid, the best results are not just the final results of the
  // final iterator (they must be captured independently of the iterators)

  Variables bestVariables; ///< best variables found in collaborative iteration
  Response  bestResponse;  ///< best response  found in collaborative iteration
};


inline int CollabHybridMetaIterator::estimate_min_processors()
{
  const StringArray& method_ptrs
    = probDescDB.get_sa("method.hybrid.method_pointers");
  const StringArray& method_names
    = probDescDB.get_sa("method.hybrid.method_names");
  const StringArray& model_ptrs
    = probDescDB.get_sa("method.hybrid.model_pointers");
  String empty_str; bool models = !model_ptrs.empty();
  int min_procs = INT_MAX, min_i;
  size_t i, num_meth = selectedIterators.size();
  for (i=0; i<num_meth; ++i) {
    if (lightwtCtor) {
      const String& model_ptr = (models) ? model_ptrs[i] : empty_str;
      Model& model = (new_model(empty_str, model_ptr)) ?
	selectedModels[i] : iteratedModel;
      iterSched.construct_sub_iterator(probDescDB, selectedIterators[i], model,
				       empty_str, methodList[i], model_ptr);
    }
    else {
      Model& model = (new_model(methodList[i], empty_str)) ?
	selectedModels[i] : iteratedModel;
      iterSched.construct_sub_iterator(probDescDB, selectedIterators[i], model,
				       methodList[i], empty_str, empty_str);
    }
    min_i = selectedIterators[i].estimate_min_processors();
    if (min_i < min_procs) min_procs = min_i;
  }

  // now apply scheduling data for this level (recursion is complete)
  return ProblemDescDB::
    min_procs_per_level(min_procs, iterSched.procsPerIterator,
			iterSched.numIteratorServers);
                    //, iterSched.iteratorScheduling);
}


inline int CollabHybridMetaIterator::estimate_max_processors()
{
  const StringArray& method_ptrs
    = probDescDB.get_sa("method.hybrid.method_pointers");
  const StringArray& method_names
    = probDescDB.get_sa("method.hybrid.method_names");
  const StringArray& model_ptrs
    = probDescDB.get_sa("method.hybrid.model_pointers");
  String empty_str; bool models = !model_ptrs.empty();
  int max_procs = 0, max_i;
  size_t i, num_meth = selectedIterators.size();
  for (i=0; i<num_meth; ++i) {
    if (lightwtCtor) {
      const String& model_ptr = (models) ? model_ptrs[i] : empty_str;
      Model& model = (new_model(empty_str, model_ptr)) ?
	selectedModels[i] : iteratedModel;
      iterSched.construct_sub_iterator(probDescDB, selectedIterators[i], model,
				       empty_str, methodList[i], model_ptr);
    }
    else {
      Model& model = (new_model(methodList[i], empty_str)) ?
	selectedModels[i] : iteratedModel;
      iterSched.construct_sub_iterator(probDescDB, selectedIterators[i], model,
				       methodList[i], empty_str, empty_str);
    }

    max_i = selectedIterators[i].estimate_max_processors();
    if (max_i > max_procs) max_procs = max_i;
  }

  // now apply scheduling data for this level (recursion is complete)
  return ProblemDescDB::
    max_procs_per_level(max_procs, iterSched.procsPerIterator,
			iterSched.numIteratorServers,
			iterSched.iteratorScheduling, 1, false,
			maxIteratorConcurrency);
}


inline const Variables& CollabHybridMetaIterator::variables_results() const
{ return bestVariables; }


inline const Response& CollabHybridMetaIterator::response_results() const
{ return bestResponse; }

} // namespace Dakota

#endif
