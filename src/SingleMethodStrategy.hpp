/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SingleMethodStrategy
//- Description: A fall-through strategy which invokes a single iterator
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: SingleMethodStrategy.hpp 6492 2009-12-19 00:04:28Z briadam $

#ifndef SINGLE_METHOD_STRATEGY_H
#define SINGLE_METHOD_STRATEGY_H

#include "DakotaStrategy.hpp"
#include "DakotaModel.hpp"
#include "DakotaIterator.hpp"


namespace Dakota {

/// Simple fall-through strategy for running a single iterator on a
/// single model.

/** This strategy executes a single iterator on a single model.  Since
    it does not provide coordination for multiple iterators and
    models, it can considered to be a "fall-through" strategy in that
    it allows control to fall through immediately to the iterator. */

class SingleMethodStrategy: public Strategy
{
public:
  
  //
  //- Heading: Constructors and destructor
  //

  SingleMethodStrategy(ProblemDescDB& problem_db); ///< constructor
  ~SingleMethodStrategy();                         ///< destructor
    
  //
  //- Heading: Member functions
  //

  /// Perform the strategy by executing selectedIterator on userDefinedModel
  void run_strategy();

  /// return the final solution from selectedIterator (variables)
  const Variables& variables_results() const;
  /// return the final solution from selectedIterator (response)
  const Response&  response_results() const;

private:

  //
  //- Heading: Convenience member functions
  //
    
  //
  //- Heading: Data members
  //

  /// the model to be iterated
  Model userDefinedModel;

  /// the iterator
  Iterator selectedIterator;
};


inline const Variables& SingleMethodStrategy::variables_results() const
{ return selectedIterator.variables_results(); }


inline const Response& SingleMethodStrategy::response_results() const
{ return selectedIterator.response_results(); }

} // namespace Dakota

#endif
