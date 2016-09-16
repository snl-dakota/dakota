/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       HierarchSurrBasedLocalMinimizerHelper
//- Description: A helper class for HierarchSurrBasedLocalMinimizer
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: HierarchSurrBasedLocalMinimizerHelper.hpp 6879 2010-07-30 01:05:11Z mseldre $

#ifndef HIERARCH_SURR_BASED_LOCAL_MINIMIZER_HELPER_H
#define HIERARCH_SURR_BASED_LOCAL_MINIMIZER_HELPER_H

#include "SurrBasedLocalMinimizer.hpp"
#include "HierarchSurrModel.hpp"
#include "DakotaModel.hpp"

namespace Dakota
{

class HierarchSurrBasedLocalMinimizerHelper
{
public:
  /// constructor
  HierarchSurrBasedLocalMinimizerHelper(const Response& initial_resp, size_t approx_form, size_t truth_form, size_t approx_level = 0, size_t truth_level = 0);
  /// destructor
  ~HierarchSurrBasedLocalMinimizerHelper();

  Response response_star(short response_type, bool return_corrected = false) const;
  Response response_center(short response_type, bool return_corrected = false) const;

  Response response_star(short response_type, bool return_corrected = false);
  Response response_center(short response_type, bool return_corrected = false);

  void response_star(const Response& resp, short response_type, bool return_corrected = false);
  void response_center(const Response& resp, short response_type, bool return_corrected = false);

  Real trust_region_factor();
  void trust_region_factor(Real val);
  void scale_trust_region_factor(Real val);

  bool new_center();
  void new_center(bool val);

  const Variables& vars_center() const;
  Variables& vars_center();
  void vars_center(const Variables& val);

  const Variables& vars_star() const;
  Variables& vars_star();
  void vars_star(const Variables& val);

  const ActiveSet& active_set () const;
  void active_set (const ActiveSet& set);

  size_t approx_model_form();
  size_t truth_model_form();
  size_t approx_model_level();
  size_t truth_model_level();

  void tr_lower_bnds(const RealVector& bounds);
  void tr_upper_bnds(const RealVector& bounds);

  const RealVector& tr_lower_bnds() const;
  const RealVector& tr_upper_bnds() const;

private:
  Response responseApproxStarUncorrected;
  Response responseApproxStarCorrected;

  Response responseApproxCenterUncorrected;
  Response responseApproxCenterCorrected;

  Response responseTruthStarUncorrected;
  Response responseTruthStarCorrected;

  Response responseTruthCenterUncorrected;
  Response responseTruthCenterCorrected;

  Variables varsCenter;
  Variables varsStar;
  
  Real trustRegionFactor;

  /// flags the acceptance of a candidate point and the existence of
  /// a new trust region center
  bool newCenterFlag;

  size_t approxModelForm;
  size_t approxModelLevel;

  size_t truthModelForm;
  size_t truthModelLevel;

  RealVector trLowerBounds;
  RealVector trUpperBounds;
};

} // namespace Dakota

#endif
