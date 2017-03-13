/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SurrBasedLevelData
//- Description: A helper class for SurrBasedLocalMinimizer
//- Owner:       Jason Monschke
//- Checked by:
//- Version: $Id: SurrBasedLevelData.hpp 6879 2010-07-30 01:05:11Z mseldre $

#ifndef SURR_BASED_LEVEL_DATA_H
#define SURR_BASED_LEVEL_DATA_H

#include "dakota_system_defs.hpp"
#include "DakotaVariables.hpp"
#include "DakotaResponse.hpp"

namespace Dakota {

// values for response type
enum { APPROX_RESPONSE=1, TRUTH_RESPONSE };
// values for corrected response type
enum { CORR_APPROX_RESPONSE=1, UNCORR_APPROX_RESPONSE,
       CORR_TRUTH_RESPONSE,    UNCORR_TRUTH_RESPONSE };
// bits for trust region status
enum { NEW_CANDIDATE      =    1,
       CANDIDATE_ACCEPTED =    2,
     //CANDIDATE_REJECTED =    4,
       CANDIDATE_STATE    = (NEW_CANDIDATE | CANDIDATE_ACCEPTED),// | _REJECTED
       NEW_CENTER         =    8,
       CENTER_BUILT       =   16,
     //CENTER_PENDING     =   32,
       CENTER_STATE       = (NEW_CENTER | CENTER_BUILT),// | CENTER_PENDING
       NEW_TR_FACTOR      =   64,
       NEW_TRUST_REGION   = (NEW_CENTER | NEW_TR_FACTOR),
       HARD_CONVERGED     =  128,
       SOFT_CONVERGED     =  256,
       MIN_TR_CONVERGED   =  512,
       MAX_ITER_CONVERGED = 1024,
       CONVERGED          = (HARD_CONVERGED   | SOFT_CONVERGED |
			     MIN_TR_CONVERGED | MAX_ITER_CONVERGED) };


class SurrBasedLevelData
{
public:

  /// default constructor
  SurrBasedLevelData();
  /// destructor
  ~SurrBasedLevelData();

  /// size the trsut region bound arrays to allow individual updates
  void initialize_bounds(size_t num_c_vars);
  /// initialize response objects via copy
  void initialize_data(const Variables& vars, const Response& approx_resp,
		       const Response& truth_resp, bool uncorr = true);
  /// initialize model forms and discretization levels
  void initialize_indices(size_t approx_form, size_t truth_form,
			  size_t approx_level = _NPOS,
			  size_t truth_level  = _NPOS);

  /// perform several reset operations to restore initialized state
  void reset();

  /// return full status (all bits)
  unsigned short status();
  /// return status of a single bit field
  bool status(unsigned short bit);
  /// activate a status bit
  void set_status_bits(unsigned short bits);
  /// deactivate a status bit
  void reset_status_bits(unsigned short bits);

  /// test for any of the CONVERGED bits
  unsigned short converged();

  const Variables& vars_center() const;
  Variables& vars_center();
  void vars_center(const Variables& vars);

  const RealVector& c_vars_center() const;
  Real c_var_center(size_t i) const;
  void c_vars_center(const RealVector& c_vars);
  void c_var_center(Real c_var, size_t i);

  const Variables& vars_star() const;
  Variables& vars_star();
  void vars_star(const Variables& vars);

  const RealVector& c_vars_star() const;
  Real c_var_star(size_t i) const;
  void c_vars_star(const RealVector& c_vars);
  void c_var_star(Real c_var, size_t i);

  //const Response& response_star(short corr_response_type) const;
  //const Response& response_center(short corr_response_type) const;
  Response& response_star(short corr_response_type);
  Response& response_center(short corr_response_type);

  void response_star(const Response& resp, short corr_response_type);
  void response_center(const Response& resp, short corr_response_type);

  IntResponsePair& response_star_pair(short corr_response_type);
 IntResponsePair& response_center_pair(short corr_response_type);

  void response_star_id(int eval_id, short corr_response_type);
  void response_center_id(int eval_id, short corr_response_type);

  void response_star_pair(IntResponsePair& pair, short corr_response_type);
  void response_center_pair(IntResponsePair& pair, short corr_response_type);

  void response_star_pair(int eval_id, const Response& resp,
			  short corr_response_type);
  void response_center_pair(int eval_id, const Response& resp,
			    short corr_response_type);

  void reset_filter();
  void initialize_filter(Real new_f, Real new_g);
  void initialize_filter(Real new_f);
  bool update_filter(Real new_f, Real new_g);
  bool update_filter(Real new_f);
  size_t filter_size() const;

  Real trust_region_factor();
  void trust_region_factor(Real val);
  void scale_trust_region_factor(Real val);

  unsigned short soft_convergence_count();
  void reset_soft_convergence_count();
  void increment_soft_convergence_count();

  const ActiveSet& active_set_center(short response_type) const;
  void active_set_center(const ActiveSet& set, short response_type,
			 bool uncorr = true);
  void active_set_center(short request, short response_type,
			 bool uncorr = true);

  const ActiveSet& active_set_star(short response_type) const;
  void active_set_star(const ActiveSet& set, short response_type,
		       bool uncorr = true);
  void active_set_star(short request, short response_type, bool uncorr = true);

  size_t approx_model_form();
  size_t approx_model_level();

  size_t truth_model_form();
  size_t truth_model_level();

  SizetSizet2DPair indices();

  const RealVector& tr_lower_bounds() const;
  Real tr_lower_bound(size_t i) const;
  void tr_lower_bounds(const RealVector& bounds);
  void tr_lower_bound(Real bound, size_t i);

  const RealVector& tr_upper_bounds() const;
  Real tr_upper_bound(size_t i) const;
  void tr_upper_bounds(const RealVector& bounds);
  void tr_upper_bound(Real bound, size_t i);

private:

  Variables varsStar;   ///< variables at the new solution iterate
  Variables varsCenter; ///< variables at the trust region center

  /// uncorrected approximate response at the new solution iterate
  Response responseStarApproxUncorrected;
  /// corrected approximate response at the new solution iterate
  Response responseStarApproxCorrected;
  /// uncorrected approximate response at the trust region center
  Response responseCenterApproxUncorrected;
  /// corrected approximate response at the trust region center
  Response responseCenterApproxCorrected;

  /// uncorrected truth response at trust region center
  Response responseStarTruthUncorrected;
  /// corrected truth response at trust region center
  IntResponsePair responseStarTruthCorrected;
  /// uncorrected truth response at new solution iterate
  Response responseCenterTruthUncorrected;
  /// corrected truth response at new solution iterate
  IntResponsePair responseCenterTruthCorrected;

  /// the trust region factor is used to compute the total size of the trust
  /// region -- it is a percentage, e.g. for trustRegionFactor = 0.1, the
  /// actual size of the trust region will be 10% of the global bounds.
  Real trustRegionFactor;

  /// collection of status bits:
  /// NEW_CANDIDATE: indicates the availability of a candidate point that, 
  ///                once verified, can be accepted as a NEW_CENTER.
  /// NEW_CENTER:    indicates the acceptance of a candidate point and the
  ///                existence of a new trust region center
  /// NEW_TR_FACTOR: indicates that trustRegionFactor has been updated,
  ///                requiring a corresponding update to tr{Lower,Upper}Bounds
  /// HARD_CONVERGED: indicates that iteration at this level has hard converged
  ///                 (norm of projected gradient < tol)
  /// SOFT_CONVERGED: indicates that iteration at this level has soft converged
  ///                 (number of unsuccessful consecutive iterations >= limit)
  /// MIN_TR_CONVERGED: indicates that TR size at this level has reached the
  ///                   minimum allowable
  /// MAX_ITER_CONVERGED: indicates that he number of iterations at this level
  ///                     has reached the maximum allowable
  unsigned short trustRegionStatus; // or use BitArray

  /// Pareto set of (objective, constraint violation) pairs defining a
  /// (slanting) filter for iterate selection/rejection
  RealRealPairSet paretoFilter;

  /// number of consecutive candidate point rejections.  If the
  /// count reaches softConvLimit, stop SBLM.
  unsigned short softConvCount;

  /// model form and discretization level indices for the approximate model
  SizetSizetPair approxModelIndices;
  /// model form and discretization level indices for the truth model
  SizetSizetPair truthModelIndices;

  /// Trust region lower bounds
  RealVector trLowerBounds;
  /// Trust region Upper bounds
  RealVector trUpperBounds;
};


inline SurrBasedLevelData::SurrBasedLevelData():
  trustRegionFactor(1.), trustRegionStatus(NEW_CENTER | NEW_TR_FACTOR),
  softConvCount(0), approxModelIndices(0, _NPOS), truthModelIndices(0, _NPOS)
{ responseStarTruthCorrected.first = responseCenterTruthCorrected.first = 0; }


inline SurrBasedLevelData::~SurrBasedLevelData()
{ }


inline void SurrBasedLevelData::initialize_bounds(size_t num_c_vars)
{
  trLowerBounds.sizeUninitialized(num_c_vars); // assign -DBL_MAX?
  trUpperBounds.sizeUninitialized(num_c_vars); // assign +DBL_MAX?
}


inline unsigned short SurrBasedLevelData::status()
{ return trustRegionStatus; }


inline bool SurrBasedLevelData::status(unsigned short bits)
{ return (trustRegionStatus & bits) == bits; }

  
inline void SurrBasedLevelData::set_status_bits(unsigned short bits)
{ trustRegionStatus |= bits; }


inline void SurrBasedLevelData::reset_status_bits(unsigned short bits)
{ trustRegionStatus &= ~bits; }


inline unsigned short SurrBasedLevelData::converged()
{ return (trustRegionStatus & CONVERGED); }

  
inline const Variables& SurrBasedLevelData::vars_center() const
{ return varsCenter; }


inline Variables& SurrBasedLevelData::vars_center()
{ return varsCenter; }


inline void SurrBasedLevelData::vars_center(const Variables& vars)
{
  varsCenter.active_variables(vars);
  // TODO: check for change in point? (DFSBLM manages update in TR center...)
  reset_status_bits(CENTER_STATE | CANDIDATE_STATE);
  set_status_bits(NEW_CENTER);
}


inline const RealVector& SurrBasedLevelData::c_vars_center() const
{ return varsCenter.continuous_variables(); }


inline Real SurrBasedLevelData::c_var_center(size_t i) const
{ return varsCenter.continuous_variable(i); }


inline void SurrBasedLevelData::c_vars_center(const RealVector& c_vars)
{
  varsCenter.continuous_variables(c_vars);
  reset_status_bits(CENTER_STATE | CANDIDATE_STATE);
  set_status_bits(NEW_CENTER);
}


inline void SurrBasedLevelData::c_var_center(Real c_var, size_t i)
{
  varsCenter.continuous_variable(c_var, i);
  reset_status_bits(CENTER_STATE | CANDIDATE_STATE);
  set_status_bits(NEW_CENTER);
}


inline const Variables& SurrBasedLevelData::vars_star() const
{ return varsStar; }


inline Variables& SurrBasedLevelData::vars_star()
{ return varsStar; }


inline void SurrBasedLevelData::vars_star(const Variables& vars)
{
  varsStar.active_variables(vars);
  // TODO: check for change in point? (DFSBLM manages update in TR center...)
  reset_status_bits(CENTER_STATE | CANDIDATE_STATE);
  set_status_bits(NEW_CANDIDATE);
}


inline const RealVector& SurrBasedLevelData::c_vars_star() const
{ return varsStar.continuous_variables(); }


inline Real SurrBasedLevelData::c_var_star(size_t i) const
{ return varsStar.continuous_variable(i); }


inline void SurrBasedLevelData::c_vars_star(const RealVector& c_vars)
{
  varsStar.continuous_variables(c_vars);
  reset_status_bits(CENTER_STATE | CANDIDATE_STATE);
  set_status_bits(NEW_CANDIDATE);
}


inline void SurrBasedLevelData::c_var_star(Real c_var, size_t i)
{
  varsStar.continuous_variable(c_var, i);
  reset_status_bits(CENTER_STATE | CANDIDATE_STATE);
  set_status_bits(NEW_CANDIDATE);
}


inline const ActiveSet& SurrBasedLevelData::
active_set_center(short response_type) const
{
  switch (response_type) {
  case TRUTH_RESPONSE:
    return responseCenterTruthCorrected.second.active_set(); break;
  case APPROX_RESPONSE:
    return responseCenterApproxCorrected.active_set();       break;
  }
}


inline void SurrBasedLevelData::
active_set_center(short request, short response_type, bool uncorr)
{
  ActiveSet new_set = active_set_center(response_type); // copy
  new_set.request_values(request);
  active_set_center(new_set, response_type, uncorr);
}


inline const ActiveSet& SurrBasedLevelData::
active_set_star(short response_type) const
{
  switch (response_type) {
  case TRUTH_RESPONSE:
    return responseStarTruthCorrected.second.active_set(); break;
  case APPROX_RESPONSE:
    return responseStarApproxCorrected.active_set();       break;
  }
}


inline void SurrBasedLevelData::
active_set_star(short request, short response_type, bool uncorr)
{
  ActiveSet new_set = active_set_star(response_type); // copy
  new_set.request_values(request);
  active_set_star(new_set, response_type, uncorr);
}


inline void SurrBasedLevelData::
response_star_pair(IntResponsePair& pair, short corr_response_type)
{ response_star_pair(pair.first, pair.second, corr_response_type); }


inline void SurrBasedLevelData::
response_center_pair(IntResponsePair& pair, short corr_response_type)
{ response_center_pair(pair.first, pair.second, corr_response_type); }


inline size_t SurrBasedLevelData::approx_model_form()
{ return approxModelIndices.first; }


inline size_t SurrBasedLevelData::truth_model_form()
{ return truthModelIndices.first; }


inline size_t SurrBasedLevelData::approx_model_level()
{ return approxModelIndices.second; }


inline size_t SurrBasedLevelData::truth_model_level()
{ return truthModelIndices.second; }


inline SizetSizet2DPair SurrBasedLevelData::indices()
{ return std::make_pair(approxModelIndices, truthModelIndices); }


inline void SurrBasedLevelData::reset_filter()
{ paretoFilter.clear(); }


inline void SurrBasedLevelData::initialize_filter(Real new_f, Real new_g)
{ reset_filter(); paretoFilter.insert(RealRealPair(new_f, new_g)); }


inline void SurrBasedLevelData::initialize_filter(Real new_f)
{ reset_filter(); paretoFilter.insert(RealRealPair(new_f, 0.)); }


inline size_t SurrBasedLevelData::filter_size() const
{ return paretoFilter.size(); }


inline Real SurrBasedLevelData::trust_region_factor()
{ return trustRegionFactor; }


inline void SurrBasedLevelData::trust_region_factor(Real val)
{ trustRegionFactor  = val; set_status_bits(NEW_TR_FACTOR); }


inline void SurrBasedLevelData::scale_trust_region_factor(Real val)
{ trustRegionFactor *= val; set_status_bits(NEW_TR_FACTOR); }


inline unsigned short SurrBasedLevelData::soft_convergence_count()
{ return softConvCount; }


inline void SurrBasedLevelData::reset_soft_convergence_count()
{ softConvCount = 0; }


inline void SurrBasedLevelData::increment_soft_convergence_count()
{ ++softConvCount; }


inline void SurrBasedLevelData::reset()
{
  reset_soft_convergence_count();
  reset_status_bits(CONVERGED);
  reset_filter();
}


inline const RealVector& SurrBasedLevelData::tr_lower_bounds() const
{ return trLowerBounds; }


inline Real SurrBasedLevelData::tr_lower_bound(size_t i) const
{ return trLowerBounds[i]; }


inline void SurrBasedLevelData::tr_lower_bounds(const RealVector& bounds)
{ trLowerBounds.assign(bounds); }


inline void SurrBasedLevelData::tr_lower_bound(Real bound, size_t i)
{ trLowerBounds[i] = bound; }


inline const RealVector& SurrBasedLevelData::tr_upper_bounds() const
{ return trUpperBounds; }


inline Real SurrBasedLevelData::tr_upper_bound(size_t i) const
{ return trUpperBounds[i]; }


inline void SurrBasedLevelData::tr_upper_bounds(const RealVector& bounds)
{ trUpperBounds.assign(bounds); }


inline void SurrBasedLevelData::tr_upper_bound(Real bound, size_t i)
{ trUpperBounds[i] = bound; }

} // namespace Dakota

#endif
