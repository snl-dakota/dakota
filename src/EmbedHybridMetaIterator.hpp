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

  /// the probability of running a local search refinement within
  /// phases of the global minimization for tightly-coupled hybrids
  Real localSearchProb;
};


inline const Variables& EmbedHybridMetaIterator::variables_results() const
{ return globalIterator.variables_results(); }


inline const Response& EmbedHybridMetaIterator::response_results() const
{ return globalIterator.response_results(); }

} // namespace Dakota

#endif
