/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SurrBasedLocalMinimizer
//- Description: A local surrogate-based algorithm which successively invokes
//-              a minimizer on an approximate model within a trust region.
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: SurrBasedLocalMinimizer.hpp 6879 2010-07-30 01:05:11Z mseldre $

#ifndef SURR_BASED_LOCAL_MINIMIZER_H
#define SURR_BASED_LOCAL_MINIMIZER_H

#include "SurrBasedMinimizer.hpp"
#include "DakotaModel.hpp"

namespace Dakota {


/// Class for provably-convergent local surrogate-based optimization
/// and nonlinear least squares.

/** This minimizer uses a SurrogateModel to perform minimization based
    on local, global, or hierarchical surrogates. It achieves provable
    convergence through the use of a sequence of trust regions and the
    application of surrogate corrections at the trust region centers. */

class SurrBasedLocalMinimizer: public SurrBasedMinimizer
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// constructor
  SurrBasedLocalMinimizer(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~SurrBasedLocalMinimizer();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void pre_run();

  /// Performs local surrogate-based minimization by minimizing local,
  /// global, or hierarchical surrogates over a series of trust regions.
  void core_run();

  void post_run(std::ostream& s);

  void reset();

  //
  //- Heading: New Virtual functions
  //

  /// update the trust region bounds, strictly contained within global bounds
  virtual void update_trust_region();

  virtual void verify();
  virtual void minimize();
  virtual void build();

  //
  //- Heading: Data members
  //

  // the +/- offsets for each of the variables in the current trust region
  //RealVector trustRegionOffset;
  /// original user specification for trustRegionFactor
  Real origTrustRegionFactor;
  /// the trust region factor is used to compute the total size of the trust
  /// region -- it is a percentage, e.g. for trustRegionFactor = 0.1, the
  /// actual size of the trust region will be 10% of the global bounds (upper
  /// bound - lower bound for each design variable).
  Real trustRegionFactor;
  /// a soft convergence control: stop SBLM when the trust region
  /// factor is reduced below the value of minTrustRegionFactor
  Real minTrustRegionFactor;
  /// trust region ratio min value: contract tr if ratio below this value
  Real trRatioContractValue;
  /// trust region ratio sufficient value: expand tr if ratio above this value
  Real trRatioExpandValue;
  /// trust region contraction factor
  Real gammaContract;
  /// trust region expansion factor
  Real gammaExpand;

  /// code indicating satisfaction of hard or soft convergence conditions
  short convergenceFlag;
  /// number of consecutive candidate point rejections.  If the
  /// count reaches softConvLimit, stop SBLM.
  unsigned short softConvCount;
  /// the limit on consecutive candidate point rejections.  If
  /// exceeded by softConvCount, stop SBLM.
  unsigned short softConvLimit;

  /// flags the use/availability of truth gradients within the SBLM process
  bool truthGradientFlag;
  /// flags the use/availability of surrogate gradients within the SBLM process
  bool approxGradientFlag;
  /// flags the use/availability of truth Hessians within the SBLM process
  bool truthHessianFlag;
  /// flags the use/availability of surrogate Hessians within the SBLM process
  bool approxHessianFlag;
  /// flags the use of surrogate correction techniques at the center
  /// of each trust region
  short correctionType;

  /// Global lower bounds
  RealVector globalLowerBnds;
  /// Global Upper bounds
  RealVector globalUpperBnds;

  ActiveSet valSet;
  ActiveSet fullApproxSet;
  ActiveSet fullTruthSet;
};

} // namespace Dakota

#endif
