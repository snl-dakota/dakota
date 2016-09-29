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
initialize_responses(const Response& approx_resp, const Response& truth_resp,
		     bool uncorr)
{
  responseStarApproxCorrected   = approx_resp.copy();
  responseCenterApproxCorrected = approx_resp.copy();

  responseStarTruthCorrected    = truth_resp.copy();
  responseCenterTruthCorrected  = truth_resp.copy();

  if (uncorr) {
    responseStarApproxUncorrected   = approx_resp.copy();
    responseCenterApproxUncorrected = approx_resp.copy();

    responseStarTruthUncorrected    = truth_resp.copy();
    responseCenterTruthUncorrected  = truth_resp.copy();
  }
}


void SurrBasedLevelData::
initialize_indices(size_t approx_form,  size_t truth_form,
		   size_t approx_level, size_t truth_level)
{
  approxModelIndices.first  = approx_form;
  approxModelIndices.second = approx_level;

  truthModelIndices.first   = truth_form;
  truthModelIndices.second  = truth_level;
}


void SurrBasedLevelData::
active_set_star(const ActiveSet& set, short response_type, bool uncorr)
{
  switch (response_type) {
  case TRUTH_RESPONSE:
    responseStarTruthCorrected.active_set(set);
    if (uncorr) responseStarTruthUncorrected.active_set(set);
    break;
  case APPROX_RESPONSE:
    responseStarApproxCorrected.active_set(set);
    if (uncorr) responseStarApproxUncorrected.active_set(set);
    break;
  }
}


const ActiveSet& SurrBasedLevelData::active_set_star(short response_type) const
{
  switch (response_type) {
  case TRUTH_RESPONSE:  return responseStarTruthCorrected.active_set();  break;
  case APPROX_RESPONSE: return responseStarApproxCorrected.active_set(); break;
  }
}


void SurrBasedLevelData::
active_set_center(const ActiveSet& set, short response_type, bool uncorr)
{
  switch (response_type) {
  case TRUTH_RESPONSE:
    responseCenterTruthCorrected.active_set(set);
    if (uncorr) responseCenterTruthUncorrected.active_set(set);
    break;
  case APPROX_RESPONSE:
    responseCenterApproxCorrected.active_set(set);
    if (uncorr) responseCenterApproxUncorrected.active_set(set);
    break;
  }
}


const ActiveSet& SurrBasedLevelData::
active_set_center(short response_type) const
{
  switch (response_type) {
  case TRUTH_RESPONSE:  return responseCenterTruthCorrected.active_set(); break;
  case APPROX_RESPONSE: return responseCenterApproxCorrected.active_set();break;
  }
}


/*
const Response& SurrBasedLevelData::
response_star(short corr_response_type) const
{
  switch (corr_response_type) {
  case CORR_TRUTH_RESPONSE:    return responseStarTruthCorrected;    break;
  case UNCORR_TRUTH_RESPONSE:  return responseStarTruthUncorrected;  break;
  case CORR_APPROX_RESPONSE:   return responseStarApproxCorrected;   break;
  case UNCORR_APPROX_RESPONSE: return responseStarApproxUncorrected; break;
  }
}


const Response& SurrBasedLevelData::
response_center(short corr_response_type) const
{
  switch (corr_response_type) {
  case CORR_TRUTH_RESPONSE:    return responseCenterTruthCorrected;    break;
  case UNCORR_TRUTH_RESPONSE:  return responseCenterTruthUncorrected;  break;
  case CORR_APPROX_RESPONSE:   return responseCenterApproxCorrected;   break;
  case UNCORR_APPROX_RESPONSE: return responseCenterApproxUncorrected; break;
  }
}
*/


Response& SurrBasedLevelData::
response_star(short corr_response_type)
{
  switch (corr_response_type) {
  case CORR_TRUTH_RESPONSE:    return responseStarTruthCorrected;    break;
  case UNCORR_TRUTH_RESPONSE:  return responseStarTruthUncorrected;  break;
  case CORR_APPROX_RESPONSE:   return responseStarApproxCorrected;   break;
  case UNCORR_APPROX_RESPONSE: return responseStarApproxUncorrected; break;
  }
}


Response& SurrBasedLevelData::
response_center(short corr_response_type)
{
  switch (corr_response_type) {
  case CORR_TRUTH_RESPONSE:    return responseCenterTruthCorrected;    break;
  case UNCORR_TRUTH_RESPONSE:  return responseCenterTruthUncorrected;  break;
  case CORR_APPROX_RESPONSE:   return responseCenterApproxCorrected;   break;
  case UNCORR_APPROX_RESPONSE: return responseCenterApproxUncorrected; break;
  }
}


void SurrBasedLevelData::
response_star(const Response& resp, short corr_response_type)
{
  switch (corr_response_type) {
  case CORR_TRUTH_RESPONSE:   responseStarTruthCorrected.update(resp);   break;
  case UNCORR_TRUTH_RESPONSE: responseStarTruthUncorrected.update(resp); break;
  case CORR_APPROX_RESPONSE:  responseStarApproxCorrected.update(resp);  break;
  case UNCORR_APPROX_RESPONSE:
    responseStarApproxUncorrected.update(resp); break;
  }
}


void SurrBasedLevelData::
response_center(const Response& resp, short corr_response_type)
{
  switch (corr_response_type) {
  case CORR_TRUTH_RESPONSE:   responseCenterTruthCorrected.update(resp);  break;
  case UNCORR_TRUTH_RESPONSE: responseCenterTruthUncorrected.update(resp);break;
  case CORR_APPROX_RESPONSE:  responseCenterApproxCorrected.update(resp); break;
  case UNCORR_APPROX_RESPONSE:
    responseCenterApproxUncorrected.update(resp); break;
  }
}

} // namespace Dakota
