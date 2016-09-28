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

#define TRUTH_RESPONSE 2
#define APPROX_RESPONSE 3

namespace Dakota {


class SurrBasedLevelData
{
public:

  /// default constructor
  SurrBasedLevelData();
  /// constructor
  SurrBasedLevelData(const Response& initial_resp, size_t approx_form,
		     size_t truth_form, size_t approx_level = 0,
		     size_t truth_level = 0);
  /// destructor
  ~SurrBasedLevelData();

  Response response_star(short response_type,
                         bool return_corrected = false) const;
  Response response_center(short response_type,
                           bool return_corrected = false) const;

  Response& response_star(short response_type, bool return_corrected = false);
  Response& response_center(short response_type, bool return_corrected = false);

  void response_star(const Response& resp, short response_type,
                     bool return_corrected = false);
  void response_center(const Response& resp, short response_type,
                       bool return_corrected = false);

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


inline SurrBasedLevelData::~SurrBasedLevelData()
{ }


inline size_t SurrBasedLevelData::approx_model_form()
{ return approxModelForm; }


inline size_t SurrBasedLevelData::truth_model_form()
{ return truthModelForm; }


inline size_t SurrBasedLevelData::approx_model_level()
{ return approxModelLevel; }


inline size_t SurrBasedLevelData::truth_model_level()
{ return truthModelLevel; }


inline const ActiveSet& SurrBasedLevelData::active_set() const
{ return responseTruthCenterUncorrected.active_set(); }


inline Real SurrBasedLevelData::trust_region_factor()
{ return trustRegionFactor; }


inline void SurrBasedLevelData::trust_region_factor(Real val)
{ trustRegionFactor = val; }


inline void SurrBasedLevelData::scale_trust_region_factor(Real val)
{ trustRegionFactor *= val; }


inline bool SurrBasedLevelData::new_center()
{ return newCenterFlag; }


inline void SurrBasedLevelData::new_center(bool val)
{ newCenterFlag = val; }


inline const Variables& SurrBasedLevelData::vars_center() const
{ return varsCenter; }


inline Variables& SurrBasedLevelData::vars_center()
{ return varsCenter; }


inline void SurrBasedLevelData::vars_center(const Variables& val)
{
  varsCenter = val.copy();
  newCenterFlag = true; // TODO: check for change in point? (DFSBLM manages update in TR center...)
}

inline const Variables& SurrBasedLevelData::vars_star() const
{ return varsStar; }


inline Variables& SurrBasedLevelData::vars_star()
{ return varsStar; }


inline void SurrBasedLevelData::
vars_star(const Variables& val)
{ varsStar = val.copy(); }


inline Response SurrBasedLevelData::
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


inline Response SurrBasedLevelData::
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


inline Response& SurrBasedLevelData::
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


inline Response& SurrBasedLevelData::
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


inline void SurrBasedLevelData::
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


inline void SurrBasedLevelData::
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


inline void SurrBasedLevelData::tr_lower_bnds(const RealVector& bounds)
{ trLowerBounds = bounds; }


inline void SurrBasedLevelData::tr_upper_bnds(const RealVector& bounds)
{ trUpperBounds = bounds; }


inline const RealVector& SurrBasedLevelData::tr_lower_bnds() const
{ return trLowerBounds; }


inline const RealVector& SurrBasedLevelData::tr_upper_bnds() const
{ return trUpperBounds; }

} // namespace Dakota

#endif
