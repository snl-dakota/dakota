/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       HierarchSurrBasedLocalMinimizer
//- Description: Implementation code for the HierarchSurrBasedLocalMinimizer class
//- Owner:       Mike Eldred, Sandia National Laboratories
//- Checked by:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "HierarchSurrBasedLocalMinimizerHelper.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "ParamResponsePair.hpp"
#include "PRPMultiIndex.hpp"
#include "DakotaGraphics.hpp"
#include "DiscrepancyCorrection.hpp"
#ifdef HAVE_NPSOL
#include "NPSOLOptimizer.hpp"
#endif // HAVE_NPSOL

//#define DEBUG
#define TRUTH_MODEL 2
#define APPROX_MODEL 3


namespace Dakota
{

HierarchSurrBasedLocalMinimizerHelper::
HierarchSurrBasedLocalMinimizerHelper(const Response& initial_resp, size_t approx_form, size_t truth_form, size_t approx_level, size_t truth_level)
  : 
  responseApproxStarUncorrected(initial_resp.copy()), 
  responseApproxStarCorrected(initial_resp.copy()), 
  responseApproxCenterUncorrected(initial_resp.copy()), 
  responseApproxCenterCorrected(initial_resp.copy()), 
  responseTruthStarUncorrected(initial_resp.copy()), 
  responseTruthStarCorrected(initial_resp.copy()), 
  responseTruthCenterUncorrected(initial_resp.copy()), 
  responseTruthCenterCorrected(initial_resp.copy()),
  approxModelForm(approx_form), truthModelForm(truth_form), approxModelLevel(approx_level), truthModelLevel(truth_level)
{
  // empty
}

HierarchSurrBasedLocalMinimizerHelper::
~HierarchSurrBasedLocalMinimizerHelper()
{
  // empty
}


size_t HierarchSurrBasedLocalMinimizerHelper::
approx_model_form() { return approxModelForm; }
size_t HierarchSurrBasedLocalMinimizerHelper::
truth_model_form() { return truthModelForm; }
size_t HierarchSurrBasedLocalMinimizerHelper::
approx_model_level() { return approxModelLevel; }
size_t HierarchSurrBasedLocalMinimizerHelper::
truth_model_level() { return truthModelLevel; }


const ActiveSet& HierarchSurrBasedLocalMinimizerHelper::active_set() const
{
  return responseTruthCenterUncorrected.active_set();
}

void HierarchSurrBasedLocalMinimizerHelper::active_set(const ActiveSet& set)
{
  responseApproxStarUncorrected.active_set(set);
  responseApproxStarCorrected.active_set(set);
  responseApproxCenterUncorrected.active_set(set);
  responseApproxCenterCorrected.active_set(set);

  responseTruthStarUncorrected.active_set(set);
  responseTruthStarCorrected.active_set(set);
  responseTruthCenterUncorrected.active_set(set);
  responseTruthCenterCorrected.active_set(set);
}

Real HierarchSurrBasedLocalMinimizerHelper::
trust_region_factor() { return trustRegionFactor; }

void HierarchSurrBasedLocalMinimizerHelper::
trust_region_factor(Real val) { trustRegionFactor = val; }

void HierarchSurrBasedLocalMinimizerHelper::
scale_trust_region_factor(Real val) { trustRegionFactor *= val; }

bool HierarchSurrBasedLocalMinimizerHelper::
new_center() { return newCenterFlag; }

void HierarchSurrBasedLocalMinimizerHelper::
new_center(bool val) { newCenterFlag = val; }

const Variables& HierarchSurrBasedLocalMinimizerHelper::
vars_center() const { return varsCenter; }

Variables& HierarchSurrBasedLocalMinimizerHelper::
vars_center() { return varsCenter; }

void HierarchSurrBasedLocalMinimizerHelper::
vars_center(const Variables& val) { varsCenter = val; }

const Variables& HierarchSurrBasedLocalMinimizerHelper::
vars_star() const { return varsStar; }

Variables& HierarchSurrBasedLocalMinimizerHelper::
vars_star() { return varsStar; }

void HierarchSurrBasedLocalMinimizerHelper::
vars_star(const Variables& val) { varsStar = val; }

Response HierarchSurrBasedLocalMinimizerHelper::
response_star(short response_type, bool return_corrected) const
{
  switch (response_type) {
    case TRUTH_MODEL:
      if (return_corrected)
        return responseTruthStarCorrected.copy();
      else
        return responseTruthStarUncorrected.copy();
      break;
    case APPROX_MODEL:
      if (return_corrected)
        return responseApproxStarCorrected.copy();
      else
        return responseApproxStarUncorrected.copy();
      break;
  }
}

Response HierarchSurrBasedLocalMinimizerHelper::
response_center(short response_type, bool return_corrected) const
{
  switch (response_type) {
    case TRUTH_MODEL:
      if (return_corrected)
        return responseTruthCenterCorrected.copy();
      else
        return responseTruthCenterUncorrected.copy();
      break;
    case APPROX_MODEL:
      if (return_corrected)
        return responseApproxCenterCorrected.copy();
      else
        return responseApproxCenterUncorrected.copy();
      break;
  }
}

Response HierarchSurrBasedLocalMinimizerHelper::
response_star(short response_type, bool return_corrected)
{
  switch (response_type) {
    case TRUTH_MODEL:
      if (return_corrected)
        return responseTruthStarCorrected.copy();
      else
        return responseTruthStarUncorrected.copy();
      break;
    case APPROX_MODEL:
      if (return_corrected)
        return responseApproxStarCorrected.copy();
      else
        return responseApproxStarUncorrected.copy();
      break;
  }
}

Response HierarchSurrBasedLocalMinimizerHelper::
response_center(short response_type, bool return_corrected)
{
  switch (response_type) {
    case TRUTH_MODEL:
      if (return_corrected)
        return responseTruthCenterCorrected.copy();
      else
        return responseTruthCenterUncorrected.copy();
      break;
    case APPROX_MODEL:
      if (return_corrected)
        return responseApproxCenterCorrected.copy();
      else
        return responseApproxCenterUncorrected.copy();
      break;
  }
}

void HierarchSurrBasedLocalMinimizerHelper::
response_star(const Response& resp, short response_type, bool return_corrected)
{
  switch (response_type) {
    case TRUTH_MODEL:
      if (return_corrected)
        responseTruthStarCorrected.update(resp);
      else
        responseTruthStarUncorrected.update(resp);
      break;
    case APPROX_MODEL:
      if (return_corrected)
        responseApproxStarCorrected.update(resp);
      else
        responseApproxStarUncorrected.update(resp);
      break;
  }
}

void HierarchSurrBasedLocalMinimizerHelper::
response_center(const Response& resp, short response_type, bool return_corrected)
{
  switch (response_type) {
    case TRUTH_MODEL:
      if (return_corrected)
        responseTruthCenterCorrected.update(resp);
      else
        responseTruthCenterUncorrected.update(resp);
      break;
    case APPROX_MODEL:
      if (return_corrected)
        responseApproxCenterCorrected.update(resp);
      else
        responseApproxCenterUncorrected.update(resp);
      break;
  }
}

void HierarchSurrBasedLocalMinimizerHelper::
tr_lower_bnds(const RealVector& bounds)
{
   trLowerBounds = bounds;
}

void HierarchSurrBasedLocalMinimizerHelper::
tr_upper_bnds(const RealVector& bounds)
{
   trUpperBounds = bounds;
}

const RealVector& HierarchSurrBasedLocalMinimizerHelper::
tr_lower_bnds() const { return trLowerBounds; }

const RealVector& HierarchSurrBasedLocalMinimizerHelper::
tr_upper_bnds() const { return trUpperBounds; }


} // namespace Dakota
