/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SurrBasedLevelData
//- Description: Implementation code for the SurrBasedLevelData class
//- Owner:       Jason Monschke
//- Checked by:

#include "SurrBasedLevelData.hpp"

namespace Dakota {


void SurrBasedLevelData::
initialize_responses(const Response& initial_resp, bool uncorr)
{
  responseApproxStarCorrected   = initial_resp.copy();
  responseApproxCenterCorrected = initial_resp.copy();
  responseTruthStarCorrected    = initial_resp.copy();
  responseTruthCenterCorrected  = initial_resp.copy();

  if (uncorr) {
    responseApproxStarUncorrected   = initial_resp.copy();
    responseApproxCenterUncorrected = initial_resp.copy();
    responseTruthStarUncorrected    = initial_resp.copy();
    responseTruthCenterUncorrected  = initial_resp.copy();
  }
}


void SurrBasedLevelData::
initialize_indices(size_t approx_form,  size_t truth_form,
		   size_t approx_level, size_t truth_level)
{
  approxModelForm  = approx_form;  truthModelForm  = truth_form;
  approxModelLevel = approx_level; truthModelLevel = truth_level;
}


void SurrBasedLevelData::
active_set_star(const ActiveSet& set, short response_type, bool uncorr)
{
  switch (response_type) {
  case TRUTH_RESPONSE:
    responseTruthStarCorrected.active_set(set);
    if (uncorr) responseTruthStarUncorrected.active_set(set);
    break;
  case APPROX_RESPONSE:
    responseApproxStarCorrected.active_set(set);
    if (uncorr) responseApproxStarUncorrected.active_set(set);
    break;
  }
}


const ActiveSet& SurrBasedLevelData::active_set_star(short response_type) const
{
  switch (response_type) {
  case TRUTH_RESPONSE:  return responseTruthStarCorrected.active_set();  break;
  case APPROX_RESPONSE: return responseApproxStarCorrected.active_set(); break;
  }
}


void SurrBasedLevelData::
active_set_center(const ActiveSet& set, short response_type, bool uncorr)
{
  switch (response_type) {
  case TRUTH_RESPONSE:
    responseTruthCenterCorrected.active_set(set);
    if (uncorr) responseTruthCenterUncorrected.active_set(set);
    break;
  case APPROX_RESPONSE:
    responseApproxCenterCorrected.active_set(set);
    if (uncorr) responseApproxCenterUncorrected.active_set(set);
    break;
  }
}


const ActiveSet& SurrBasedLevelData::
active_set_center(short response_type) const
{
  switch (response_type) {
  case TRUTH_RESPONSE:  return responseTruthCenterCorrected.active_set(); break;
  case APPROX_RESPONSE: return responseApproxCenterCorrected.active_set();break;
  }
}


const Response& SurrBasedLevelData::
response_star(short corr_response_type) const
{
  switch (corr_response_type) {
  case CORR_TRUTH_RESPONSE:    return responseTruthStarCorrected;    break;
  case UNCORR_TRUTH_RESPONSE:  return responseTruthStarUncorrected;  break;
  case CORR_APPROX_RESPONSE:   return responseApproxStarCorrected;   break;
  case UNCORR_APPROX_RESPONSE: return responseApproxStarUncorrected; break;
  }
}


const Response& SurrBasedLevelData::
response_center(short corr_response_type) const
{
  switch (corr_response_type) {
  case CORR_TRUTH_RESPONSE:    return responseTruthCenterCorrected;    break;
  case UNCORR_TRUTH_RESPONSE:  return responseTruthCenterUncorrected;  break;
  case CORR_APPROX_RESPONSE:   return responseApproxCenterCorrected;   break;
  case UNCORR_APPROX_RESPONSE: return responseApproxCenterUncorrected; break;
  }
}


Response& SurrBasedLevelData::
response_star(short corr_response_type)
{
  switch (corr_response_type) {
  case CORR_TRUTH_RESPONSE:    return responseTruthStarCorrected;    break;
  case UNCORR_TRUTH_RESPONSE:  return responseTruthStarUncorrected;  break;
  case CORR_APPROX_RESPONSE:   return responseApproxStarCorrected;   break;
  case UNCORR_APPROX_RESPONSE: return responseApproxStarUncorrected; break;
  }
}


Response& SurrBasedLevelData::
response_center(short corr_response_type)
{
  switch (corr_response_type) {
  case CORR_TRUTH_RESPONSE:    return responseTruthCenterCorrected;    break;
  case UNCORR_TRUTH_RESPONSE:  return responseTruthCenterUncorrected;  break;
  case CORR_APPROX_RESPONSE:   return responseApproxCenterCorrected;   break;
  case UNCORR_APPROX_RESPONSE: return responseApproxCenterUncorrected; break;
  }
}


void SurrBasedLevelData::
response_star(const Response& resp, short corr_response_type)
{
  switch (corr_response_type) {
  case CORR_TRUTH_RESPONSE:   responseTruthStarCorrected.update(resp);   break;
  case UNCORR_TRUTH_RESPONSE: responseTruthStarUncorrected.update(resp); break;
  case CORR_APPROX_RESPONSE:  responseApproxStarCorrected.update(resp);  break;
  case UNCORR_APPROX_RESPONSE:
    responseApproxStarUncorrected.update(resp); break;
  }
}


void SurrBasedLevelData::
response_center(const Response& resp, short corr_response_type)
{
  switch (corr_response_type) {
  case CORR_TRUTH_RESPONSE:   responseTruthCenterCorrected.update(resp);  break;
  case UNCORR_TRUTH_RESPONSE: responseTruthCenterUncorrected.update(resp);break;
  case CORR_APPROX_RESPONSE:  responseApproxCenterCorrected.update(resp); break;
  case UNCORR_APPROX_RESPONSE:
    responseApproxCenterUncorrected.update(resp); break;
  }
}

} // namespace Dakota
