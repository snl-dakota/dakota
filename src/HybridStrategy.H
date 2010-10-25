/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       HybridStrategy
//- Description: A multi-level hybrid strategy which invokes several iterators
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: HybridStrategy.H 6492 2009-12-19 00:04:28Z briadam $

#ifndef HYBRID_STRATEGY_H
#define HYBRID_STRATEGY_H

#include "data_types.h"
#include "DakotaStrategy.H"
#include "DakotaIterator.H"
#include "DakotaModel.H"


namespace Dakota {


/// Base class for hybrid minimization strategies.

/** This base class shares code for three approaches to hybrid
    minimization: (1) the sequential hybrid; (2) the embedded hybrid;
    and (3) the collaborative hybrid. */

class HybridStrategy: public Strategy
{
public:
  
protected:
  
  //
  //- Heading: Constructors and destructor
  //

  HybridStrategy(ProblemDescDB& problem_db); ///< constructor
  ~HybridStrategy();                         ///< destructor

  //
  //- Heading: Convenience member functions
  //

  /// initialize selectedIterators and userDefinedModels
  void allocate_methods();
  /// free communicators for selectedIterators and userDefinedModels
  void deallocate_methods();

  //
  //- Heading: Data members
  //

  StringArray methodList;   ///< the list of method identifiers
  int         numIterators; ///< number of methods in methodList

  /// the set of iterators, one for each entry in methodList
  IteratorArray selectedIterators;
  /// the set of models, one for each iterator
  ModelArray userDefinedModels;
};

} // namespace Dakota

#endif
