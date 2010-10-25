/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       CollaborativeHybridStrategy
//- Description: A hybrid strategy involving several collaborating iterators
//- Owner:       Patty Hough/John Siirola
//- Checked by:
//- Version: $Id: CollaborativeHybridStrategy.H 6492 2009-12-19 00:04:28Z briadam $

#ifndef COLLABORATIVE_HYBRID_STRATEGY_H
#define COLLABORATIVE_HYBRID_STRATEGY_H

#include "data_types.h"
#include "HybridStrategy.H"


namespace Dakota {


/// Strategy for hybrid minimization using multiple collaborating
/// optimization and nonlinear least squares methods.

/** This strategy has two approaches to hybrid minimization: (1)
    agent-based using the ABO framework; (2) nonagent-based using the
    HOPSPACK framework. */

class CollaborativeHybridStrategy: public HybridStrategy
{
public:
  
  //
  //- Heading: Constructors and destructor
  //

  CollaborativeHybridStrategy(ProblemDescDB& problem_db); ///< constructor
  ~CollaborativeHybridStrategy();                         ///< destructor
    
protected:
  
  //
  //- Heading: Member functions
  //

  /// Performs the collaborative hybrid minimization strategy
  void run_strategy();

  /// return the final solution from the collaborative minimization (variables)
  const Variables& variables_results() const;
  /// return the final solution from the collaborative minimization (response)
  const Response&  response_results() const;

private:

  //
  //- Heading: Data members
  //

  String hybridCollabType; ///< abo or hops

  // In this hybrid, the best results are not just the final results of the
  // final iterator (they must be captured independently of the iterators)

  Variables bestVariables; ///< best variables found in minimization
  Response  bestResponse;  ///< best response  found in minimization
};


inline const Variables& CollaborativeHybridStrategy::variables_results() const
{ return bestVariables; }


inline const Response& CollaborativeHybridStrategy::response_results() const
{ return bestResponse; }

} // namespace Dakota

#endif
