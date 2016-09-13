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

class HierarchSurrBasedLocalMinimizerHelper
{
public:
  /// constructor
  HierarchSurrBasedLocalMinimizerHelper();
  /// destructor
  ~HierarchSurrBasedLocalMinimizerHelper();

  const Response& response_star(bool return_corrected) const;

  const Response& response_center(bool return_corrected) const;

  void response_star_corrected(const Response& resp);
  void response_star_uncorrected(const Response& resp);
  void response_center_corrected(const Response& resp);
  void response_center_uncorrected(const Response& resp);

  Real trust_region_factor();
  void trust_region_factor(Real val);

  bool new_center();
  void new_center(bool val);

  const Variables& vars_center() const;
  void vars_center(const Variables& val);

private:
  Response responseStarUncorrected;
  Response responseStarCorrected;

  Response responseCenterUncorrected;
  Response responseCenterCorrected;

  Variables varsCenter;          ///< variables at the trust region centers
  
  Real trustRegionFactor;

  /// flags the acceptance of a candidate point and the existence of
  /// a new trust region center
  bool newCenterFlag;
};


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

  void pre_run();

  /// reset convergence controls in case of multiple MLMF executions
  void reset();

  /// update the trust region bounds, strictly contained within global bounds
  void update_trust_region();

  void verify();
  void minimize();
  void build();

private:

  //
  //- Heading: Convenience member functions
  //

  void find_center(size_t lf_model_form, size_t hf_model_form);

  void hard_convergence_check(const Response& response_truth,
                         const RealVector& c_vars,
                         const RealVector& lower_bnds,
                         const RealVector& upper_bnds);

  void tr_ratio_check();

  //
  //- Heading: Data members
  //

  size_t numFid;
  SizetArray numLev;

  std::vector<HierarchSurrBasedLocalMinimizerHelper> trustRegions;

  /// pointer to MLMF instance used in static member functions
  static HierarchSurrBasedLocalMinimizer* mlmfInstance;
};

} // namespace Dakota

#endif
