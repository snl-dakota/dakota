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
initialize_data(const Variables& vars, const Response& approx_resp,
		const Response& truth_resp, bool uncorr)
{
  // Initialize with deep Variables copies to avoid representation sharing;
  // Variables::active_variables() is then used for run time assignments.
  varsCenter = vars.copy();
  varsStar   = vars.copy();

  // Initialize with deep Response copies to avoid representation sharing;
  // Response::update() is then used for run time assignments.

  responseStarApproxCorrected   = approx_resp.copy();
  responseCenterApproxCorrected = approx_resp.copy();

  responseStarTruthCorrected.second   = truth_resp.copy();
  responseCenterTruthCorrected.second = truth_resp.copy();

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
active_set_center(const ActiveSet& set, short response_type, bool uncorr)
{
  switch (response_type) {
  case TRUTH_RESPONSE:
    responseCenterTruthCorrected.second.active_set(set);
    if (uncorr) responseCenterTruthUncorrected.active_set(set);
    break;
  case APPROX_RESPONSE:
    responseCenterApproxCorrected.active_set(set);
    if (uncorr) responseCenterApproxUncorrected.active_set(set);
    break;
  }
}


void SurrBasedLevelData::
active_set_star(const ActiveSet& set, short response_type, bool uncorr)
{
  switch (response_type) {
  case TRUTH_RESPONSE:
    responseStarTruthCorrected.second.active_set(set);
    if (uncorr) responseStarTruthUncorrected.active_set(set);
    break;
  case APPROX_RESPONSE:
    responseStarApproxCorrected.active_set(set);
    if (uncorr) responseStarApproxUncorrected.active_set(set);
    break;
  }
}


/*
const Response& SurrBasedLevelData::
response_star(short corr_response_type) const
{
  switch (corr_response_type) {
  case CORR_TRUTH_RESPONSE:    return responseStarTruthCorrected.second; break;
  case UNCORR_TRUTH_RESPONSE:  return responseStarTruthUncorrected;      break;
  case CORR_APPROX_RESPONSE:   return responseStarApproxCorrected;       break;
  case UNCORR_APPROX_RESPONSE: return responseStarApproxUncorrected;     break;
  }
}


const Response& SurrBasedLevelData::
response_center(short corr_response_type) const
{
  switch (corr_response_type) {
  case CORR_TRUTH_RESPONSE:   return responseCenterTruthCorrected.second; break;
  case UNCORR_TRUTH_RESPONSE:  return responseCenterTruthUncorrected;     break;
  case CORR_APPROX_RESPONSE:   return responseCenterApproxCorrected;      break;
  case UNCORR_APPROX_RESPONSE: return responseCenterApproxUncorrected;    break;
  }
}
*/


Response& SurrBasedLevelData::
response_star(short corr_response_type)
{
  switch (corr_response_type) {
  case CORR_TRUTH_RESPONSE:    return responseStarTruthCorrected.second; break;
  case UNCORR_TRUTH_RESPONSE:  return responseStarTruthUncorrected;      break;
  case CORR_APPROX_RESPONSE:   return responseStarApproxCorrected;       break;
  case UNCORR_APPROX_RESPONSE: return responseStarApproxUncorrected;     break;
  }
}


Response& SurrBasedLevelData::
response_center(short corr_response_type)
{
  switch (corr_response_type) {
  case CORR_TRUTH_RESPONSE:
    return responseCenterTruthCorrected.second;                        break;
  case UNCORR_TRUTH_RESPONSE:  return responseCenterTruthUncorrected;  break;
  case CORR_APPROX_RESPONSE:   return responseCenterApproxCorrected;   break;
  case UNCORR_APPROX_RESPONSE: return responseCenterApproxUncorrected; break;
  }
}


void SurrBasedLevelData::
response_star(const Response& resp, short corr_response_type)
{
  switch (corr_response_type) {
  case CORR_TRUTH_RESPONSE:
    responseStarTruthCorrected.second.update(resp);                      break;
  case UNCORR_TRUTH_RESPONSE: responseStarTruthUncorrected.update(resp); break;
  case CORR_APPROX_RESPONSE:  responseStarApproxCorrected.update(resp);  break;
  case UNCORR_APPROX_RESPONSE:
    responseStarApproxUncorrected.update(resp);                          break;
  }
}


void SurrBasedLevelData::
response_center(const Response& resp, short corr_response_type)
{
  switch (corr_response_type) {
  case CORR_TRUTH_RESPONSE:
    responseCenterTruthCorrected.second.update(resp);                     break;
  case UNCORR_TRUTH_RESPONSE: responseCenterTruthUncorrected.update(resp);break;
  case CORR_APPROX_RESPONSE:  responseCenterApproxCorrected.update(resp); break;
  case UNCORR_APPROX_RESPONSE:
    responseCenterApproxUncorrected.update(resp);                         break;
  }
}


IntResponsePair& SurrBasedLevelData::
response_star_pair(short corr_response_type)
{
  if (corr_response_type != CORR_TRUTH_RESPONSE) {
    Cerr << "Error: IntResponsePair return not supported in SurrBasedLevelData "
	 << "for this response type" << std::endl;
    abort_handler(METHOD_ERROR);
  }

  return responseStarTruthCorrected;
}


IntResponsePair& SurrBasedLevelData::
response_center_pair(short corr_response_type)
{
  if (corr_response_type != CORR_TRUTH_RESPONSE) {
    Cerr << "Error: IntResponsePair return not supported in SurrBasedLevelData "
	 << "for this response type" << std::endl;
    abort_handler(METHOD_ERROR);
  }

  return responseCenterTruthCorrected;
}


void SurrBasedLevelData::
response_star_id(int eval_id, short corr_response_type)
{
  switch (corr_response_type) {
  case CORR_TRUTH_RESPONSE: responseStarTruthCorrected.first = eval_id; break;
  default:
    Cerr << "Error: eval_id assignment not supported in SurrBasedLevelData for "
	 << "this response type" << std::endl;
    abort_handler(METHOD_ERROR);
    break;
  }
}


void SurrBasedLevelData::
response_center_id(int eval_id, short corr_response_type)
{
  switch (corr_response_type) {
  case CORR_TRUTH_RESPONSE: responseCenterTruthCorrected.first = eval_id; break;
  default:
    Cerr << "Error: eval_id assignment not supported in SurrBasedLevelData for "
	 << "this response type" << std::endl;
    abort_handler(METHOD_ERROR);
    break;
  }
}


void SurrBasedLevelData::
response_star_pair(int eval_id, const Response& resp, short corr_response_type)
{
  if (corr_response_type != CORR_TRUTH_RESPONSE) {
    Cerr << "Error: eval_id + response assignment not supported in "
	 << "SurrBasedLevelData for this response type" << std::endl;
    abort_handler(METHOD_ERROR);
  }

  responseStarTruthCorrected.first = eval_id;
  responseStarTruthCorrected.second.update(resp);
}


void SurrBasedLevelData::
response_center_pair(int eval_id, const Response& resp,
		     short corr_response_type)
{
  if (corr_response_type != CORR_TRUTH_RESPONSE) {
    Cerr << "Error: eval_id + response assignment not supported in "
	 << "SurrBasedLevelData for this response type" << std::endl;
    abort_handler(METHOD_ERROR);
  }

  responseCenterTruthCorrected.first = eval_id;
  responseCenterTruthCorrected.second.update(resp);
}


bool SurrBasedLevelData::update_filter(Real new_f, Real new_g)
{
  // test new point against current filter
  RRPSIter filt_it = paretoFilter.begin();
  Real filt_f, filt_g, gamma = 1.e-5, beta = 1. - gamma;

  // we queue dominated pt removals to ensure that iterate rejection by
  // slanting filter and filter pruning by std filter are exclusive
  std::list<RRPSIter> rm_queue;

  for (; filt_it != paretoFilter.end(); ++filt_it) {
    filt_f = filt_it->first; filt_g = filt_it->second;

    // Simple filter (no gamma, beta):
    //if (new_f >= filt_f && new_g >= filt_g)
    //  return false;              // new point dominated: reject iterate
    //else if (new_f < filt_f && new_g < filt_g)
    //  paretoFilter.erase(filt_it++); // old dominated by new: remove old
    //else ++filt_it;

    // Slanting filter: Fletcher, Leyffer, and Toint (SIAM J. Optim., 2002).
    // The slanting logic is applied to new iterate acceptance, but the
    // simple filter logic is used for the pruning of old points which are
    // dominated by the new iterate.  This is due to the inclusion property
    // of the slanting filter (the envelope for an accepted iterate includes
    // the envelope for any filter point it dominates).
    if (new_f + gamma*new_g > filt_f && new_g > beta*filt_g) // slanting logic
      return false;                // new pt unacceptable: reject iterate
    else if (new_f < filt_f && new_g < filt_g)                 // simple logic
      rm_queue.push_back(filt_it); // old dominated by new: queue old for rm
  }

  // iterate is acceptable: process any removals and add new pt to filter
  // invalidation rules: only iters/refs to erased elements are invalidated
  for (std::list<RRPSIter>::iterator rm_it=rm_queue.begin();
       rm_it!=rm_queue.end(); ++rm_it)
    paretoFilter.erase(*rm_it);
  paretoFilter.insert(RealRealPair(new_f, new_g));
#ifdef DEBUG
  Cout << "Filter:\n";
  for (filt_it=paretoFilter.begin(); filt_it!=paretoFilter.end(); ++filt_it)
    Cout << *filt_it << '\n';
#endif
  return true;
}

  
bool SurrBasedLevelData::update_filter(Real new_f)
{
  // retain only one point for unconstrained filter
  RRPSIter filt_it = paretoFilter.begin();
  if (filt_it == paretoFilter.end())    // insert first iterate
    paretoFilter.insert(RealRealPair(new_f, 0.));
  else if (new_f >= filt_it->first) // new f is dominated: reject iterate
    return false;             
  else {                            // old f dominated by new: replace old
    //filt_it->first = new_f; // not allowed for ordered set
    paretoFilter.clear();
    paretoFilter.insert(RealRealPair(new_f, 0.));
  }
  return true;
}

} // namespace Dakota
