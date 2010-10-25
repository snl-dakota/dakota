/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       EmbeddedHybridStrategy
//- Description: A strategy for tightly-coupled global-local hybrids
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: EmbeddedHybridStrategy.H 6492 2009-12-19 00:04:28Z briadam $

#ifndef EMBEDDED_HYBRID_STRATEGY_H
#define EMBEDDED_HYBRID_STRATEGY_H

#include "data_types.h"
#include "HybridStrategy.H"


namespace Dakota {


/// Strategy for closely-coupled hybrid minimization, typically
/// involving the embedding of local search methods within global
/// search methods.

/** This strategy uses multiple methods in close coordination,
    generally using a local search minimizer repeatedly within a
    global minimizer (the local search minimizer refines candidate
    minima which are fed back to the global minimizer). */

class EmbeddedHybridStrategy: public HybridStrategy
{
public:
  
  //
  //- Heading: Constructors and destructor
  //

  EmbeddedHybridStrategy(ProblemDescDB& problem_db); ///< constructor
  ~EmbeddedHybridStrategy();                         ///< destructor
    
protected:
  
  //
  //- Heading: Member functions
  //

  /// Performs the hybrid minimization strategy by executing multiple
  /// iterators on different models of varying fidelity
  void run_strategy();

  /// return the final solution from selectedIterators (variables)
  const Variables& variables_results() const;
  /// return the final solution from selectedIterators (response)
  const Response&  response_results() const;

private:

  //
  //- Heading: Data members
  //

  /// the probability of running a local search refinement within
  /// phases of the global minimization for coupled hybrids
  Real localSearchProb;
};


inline const Variables& EmbeddedHybridStrategy::variables_results() const
{ return selectedIterators[1].variables_results(); }


inline const Response& EmbeddedHybridStrategy::response_results() const
{ return selectedIterators[1].response_results(); }

} // namespace Dakota

#endif
