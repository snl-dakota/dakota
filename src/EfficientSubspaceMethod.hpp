/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef EFFICIENT_SUBSPACE_METHOD_H
#define EFFICIENT_SUBSPACE_METHOD_H

#include "DakotaNonD.hpp"

namespace Dakota {

// BMA: Redesign wishlist / notes:
//
//  * Use subspace models with arbitrary Iterators.  To do this, need
//    to be able to construct the Iterator without knowing the size of
//    the variable domain, then later update it with the subspace
//    model once built.
// 
//  * For now, use method strings or method pointer and construct the
//    outer solver at run-time.
//
//  * Consider moving to inherit from Iterator instead of NonD if possible

/// Efficient Subspace Method (ESM), as proposed by Hany S. Abdel-Khalik; 
/// updated to include ideas from Paul G. Constantine and others

/** ESM uses random sampling of the model gradient to construct a
    low-dimensional subspace of the full dimensional parameter space,
    then performs UQ in the reduced space */
class EfficientSubspaceMethod: public NonD
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// Standard, model-based constructor
  EfficientSubspaceMethod(ProblemDescDB& problem_db, Model& model);
  /// Destructor
  ~EfficientSubspaceMethod();

  //
  //- Heading: Virtual function redefinitions
  //

  /// specialization of init comms due to varied use of the original model
  // void derived_init_communicators(ParLevLIter pl_iter);
  /// specialization of init comms due to varied use of the original model
  void derived_set_communicators(ParLevLIter pl_iter);
  /// specialization of free comms due to varied use of the original model
  // void derived_free_communicators(ParLevLIter pl_iter);

  /// ESM re-implementation of the virtual UQ iterator function
  void core_run();

private:

  // Initialization methods 
  
  /// validate user-supplied input values, setting defaults, aborting on error
  void validate_inputs();

  /// experimental method to demonstrate creating a RecastModel and
  /// perform sampling-based UQ in the reduced space
  void reduced_space_uq(Model& subspace_model);

  // Data controlling iteration and status

  /// seed controlling all samplers
  int seedSpec;

  /// number of UQ samples to perform in the reduced space
  int subspaceSamples;

  /// index for the active ParallelLevel within ParallelConfiguration::miPLIters
  size_t miPLIndex;

  /// Flag determining if uncertain variables are transformed prior to
  /// active subspace identification.
  bool transformVars;

  /// Model including full space uncertain variables. Transformed to standard
  /// normal variables if transformVars == true.
  Model fullSpaceModel;
};

}

#endif  // EFFICIENT_SUBSPACE_METHOD_H
