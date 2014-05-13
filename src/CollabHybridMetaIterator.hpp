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

  /// return the final solution from the collaborative iteration (variables)
  const Variables& variables_results() const;
  /// return the final solution from the collaborative iteration (response)
  const Response&  response_results() const;

private:

  //
  //- Heading: Data members
  //

  String hybridCollabType; ///< abo or hops
  StringArray methodList;  ///< the list of method name identifiers
  bool        lightwtCtor; ///< indicates use of lightweight Iterator ctors

  /// the set of iterators, one for each entry in methodList
  IteratorArray selectedIterators;
  /// the set of models, one for each iterator
  ModelArray selectedModels;

  // In this hybrid, the best results are not just the final results of the
  // final iterator (they must be captured independently of the iterators)

  Variables bestVariables; ///< best variables found in collaborative iteration
  Response  bestResponse;  ///< best response  found in collaborative iteration
};


inline const Variables& CollabHybridMetaIterator::variables_results() const
{ return bestVariables; }


inline const Response& CollabHybridMetaIterator::response_results() const
{ return bestResponse; }

} // namespace Dakota

#endif
