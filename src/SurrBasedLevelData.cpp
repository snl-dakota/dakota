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

#define APPROX_RESPONSE 1
#define  TRUTH_RESPONSE 2

namespace Dakota {


SurrBasedLevelData::
SurrBasedLevelData(const Response& initial_resp,
		   size_t approx_form,  size_t truth_form,
		   size_t approx_level, size_t truth_level) :
  // TO DO: pass active sets or unsigned short to init responses
  responseApproxStarUncorrected(initial_resp.copy()),
  responseApproxStarCorrected(initial_resp.copy()),
  responseApproxCenterUncorrected(initial_resp.copy()),
  responseApproxCenterCorrected(initial_resp.copy()),
  responseTruthStarUncorrected(initial_resp.copy()),
  responseTruthStarCorrected(initial_resp.copy()),
  responseTruthCenterUncorrected(initial_resp.copy()),
  responseTruthCenterCorrected(initial_resp.copy()),
  approxModelForm(approx_form), truthModelForm(truth_form),
  approxModelLevel(approx_level), truthModelLevel(truth_level)
{ }


void SurrBasedLevelData::active_set(const ActiveSet& set)
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


Response SurrBasedLevelData::
response_star(short response_type, bool return_corrected) const
{
  switch (response_type) {
  case TRUTH_RESPONSE:
    return (return_corrected) ? responseTruthStarCorrected.copy() :
      responseTruthStarUncorrected.copy();
    break;
  case APPROX_RESPONSE:
    return (return_corrected) ? responseApproxStarCorrected.copy() :
      responseApproxStarUncorrected.copy();
    break;
  }
}


Response SurrBasedLevelData::
response_center(short response_type, bool return_corrected) const
{
  switch (response_type) {
  case TRUTH_RESPONSE:
    return (return_corrected) ? responseTruthCenterCorrected.copy() :
      responseTruthCenterUncorrected.copy();
    break;
  case APPROX_RESPONSE:
    return (return_corrected) ? responseApproxCenterCorrected.copy() :
      responseApproxCenterUncorrected.copy();
    break;
  }
}


Response& SurrBasedLevelData::
response_star(short response_type, bool return_corrected)
{
  switch (response_type) {
  case TRUTH_RESPONSE:
    return (return_corrected) ?
      responseTruthStarCorrected : responseTruthStarUncorrected;
    break;
  case APPROX_RESPONSE:
    return (return_corrected) ?
      responseApproxStarCorrected : responseApproxStarUncorrected;
    break;
  }
}


Response& SurrBasedLevelData::
response_center(short response_type, bool return_corrected)
{
  switch (response_type) {
  case TRUTH_RESPONSE:
    return (return_corrected) ? 
      responseTruthCenterCorrected : responseTruthCenterUncorrected;
    break;
  case APPROX_RESPONSE:
    return (return_corrected) ?
      responseApproxCenterCorrected : responseApproxCenterUncorrected;
    break;
  }
}


void SurrBasedLevelData::
response_star(const Response& resp, short response_type, bool return_corrected)
{
  switch (response_type) {
  case TRUTH_RESPONSE:
    if (return_corrected) responseTruthStarCorrected.update(resp);
    else                  responseTruthStarUncorrected.update(resp);
    break;
  case APPROX_RESPONSE:
    if (return_corrected) responseApproxStarCorrected.update(resp);
    else                  responseApproxStarUncorrected.update(resp);
    break;
  }
}


void SurrBasedLevelData::
response_center(const Response& resp, short response_type,
		bool return_corrected)
{
  switch (response_type) {
  case TRUTH_RESPONSE:
    if (return_corrected) responseTruthCenterCorrected.update(resp);
    else                  responseTruthCenterUncorrected.update(resp);
    break;
  case APPROX_RESPONSE:
    if (return_corrected) responseApproxCenterCorrected.update(resp);
    else                  responseApproxCenterUncorrected.update(resp);
    break;
  }
}

} // namespace Dakota
