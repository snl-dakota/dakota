/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       HierarchSurrBasedLocalMinimizer
//- Description: A local optimization algorithm that uses multiple model forms
//-              and discretization levels to accelerate convergence of a high-
//-              fidelity, finely-resolved model.
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: HierarchSurrBasedLocalMinimizer.hpp 6879 2010-07-30 01:05:11Z mseldre $

#ifndef HIERARCH_SURR_BASED_LOCAL_MINIMIZER_H
#define HIERARCH_SURR_BASED_LOCAL_MINIMIZER_H

#include "SurrBasedLocalMinimizer.hpp"
#include "HierarchSurrModel.hpp"
#include "DakotaModel.hpp"

namespace Dakota
{


/// Class for multilevel-multifidelity optimization algorithm

/** This minimizer uses SurrogateModel(s) to perform minimization leveraging
    multiple model forms and discretization levels. */

class HierarchSurrBasedLocalMinimizer: public SurrBasedLocalMinimizer
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// constructor
  HierarchSurrBasedLocalMinimizer(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~HierarchSurrBasedLocalMinimizer();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  /// Performs local multilevel-multifidelity minimization
  void core_run();

  /// reset convergence controls in case of multiple MLMF executions
  void reset();

private:

  //
  //- Heading: Convenience member functions
  //

  void multilevel_multifidelity_opt();
  void multigrid_opt();
  void multifidelity_opt();

  Variables MFOpt(size_t fidelity, const Variables &variables);
  void tr_bounds(RealVectorArray& tr_lower_bnds, RealVectorArray& tr_upper_bnds);
  void find_center(size_t lf_model_form, size_t hf_model_form);
  void hard_convergence_check(const Response& response_truth,
                         const RealVector& c_vars,
                         const RealVector& lower_bnds,
                         const RealVector& upper_bnds);

  void tr_ratio_check();

  //
  //- Heading: Data members
  //

  std::vector<Response> responseStarUncorrected;
  std::vector<Response> responseStarCorrected;

  std::vector<Response> responseCenterUncorrected;
  std::vector<Response> responseCenterCorrected;

  Variables varsCenter;          ///< variables at the trust region center

  /// the trust region factor is used to compute the total size of the trust
  /// region -- it is a percentage, e.g. for trustRegionFactor = 0.1, the
  /// actual size of the trust region will be 10% of the global bounds (upper
  /// bound - lower bound for each design variable).
  RealVector trustRegionFactor;
  /// a soft convergence control: stop SBLM when the trust region
  /// factor is reduced below the value of minTrustRegionFactor
  Real minTrustRegionFactor;
  /// flags the acceptance of a candidate point and the existence of
  /// a new trust region center
  std::vector<bool> newCenterFlag;

  /// pointer to MLMF instance used in static member functions
  static HierarchSurrBasedLocalMinimizer* mlmfInstance;
};

} // namespace Dakota

#endif
