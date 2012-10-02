/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Description:  This file contains code related to data utilities that should
//-               be compiled, rather than inlined in data_util.h.
//-
//- Owner:        Mike Eldred
//- Version: $Id: data_util.C 7024 2010-10-16 01:24:42Z mseldre $

#include "data_util.h"


namespace Dakota {

// ------------
// == operators
// ------------


bool operator==(const ShortArray& dsa1, const ShortArray& dsa2)
{
  // Check for equality in array lengths
  size_t len = dsa1.size();
  if ( dsa2.size() != len )
    return false;

  // Check each value
  size_t i;
  for (i=0; i<len; ++i)
    if ( dsa2[i] != dsa1[i] )
      return false;

  return true;
}


bool operator==(const StringArray& dsa1, const StringArray& dsa2)
{
  // Check for equality in array lengths
  size_t len = dsa1.size();
  if ( dsa2.size() != len )
    return false;

  // Check each string
  size_t i;
  for (i=0; i<len; ++i)
    if ( dsa2[i] != dsa1[i] )
      return false;

  return true;
}


bool operator==(const SizetArray& sa, SizetMultiArrayConstView smav)
{
  // Check for equality in array lengths
  size_t len = sa.size();
  if ( smav.size() != len )
    return false;

  // Check each size_t
  size_t i;
  for (i=0; i<len; ++i)
    if ( smav[i] != sa[i] )
      return false;

  return true;
}


// ---------------------------------
// miscellaneous numerical utilities
// ---------------------------------

Real rel_change_L2(const RealVector& curr_rv, const RealVector& prev_rv)
{
  size_t i, rv_len = prev_rv.length();
  Real norm = 0.;

  // check previous vector for zeros
  bool zero_prev = false, zero_curr = false;
  for (i=0; i<rv_len; ++i)
    if (std::abs(prev_rv[i]) < Pecos::SMALL_NUMBER)
      { zero_prev = true; break; }
  // check current vector for zeros
  if (zero_prev)
    for (i=0; i<rv_len; ++i)
      if (std::abs(curr_rv[i]) < Pecos::SMALL_NUMBER)
	{ zero_curr = true; break; }

  // Compute norm of relative change one of three ways
  if (!zero_prev) { // change relative to previous
    for (i=0; i<rv_len; ++i)
      norm += std::pow(curr_rv[i] / prev_rv[i] - 1., 2.);
    return std::sqrt(norm);
  }
  else if (!zero_curr) { // change relative to current
    for (i=0; i<rv_len; ++i)
      norm += std::pow(prev_rv[i] / curr_rv[i] - 1., 2.);
    return std::sqrt(norm);
  }
  else { // absolute change scaled by norm of previous
    Real scaling = 0.;
    for (i=0; i<rv_len; ++i) {
      norm    += std::pow(curr_rv[i] - prev_rv[i], 2.);
      scaling += std::pow(prev_rv[i], 2.);
    }
    return (scaling > Pecos::SMALL_NUMBER) ?
      std::sqrt(norm / scaling) : std::sqrt(norm);
  }
}


Real rel_change_L2(const RealVector& curr_rv1, const RealVector& prev_rv1,
		   const IntVector&  curr_iv,  const IntVector&  prev_iv,
		   const RealVector& curr_rv2, const RealVector& prev_rv2)
{
  size_t i, rv1_len = prev_rv1.length(), iv_len = prev_iv.length(),
    rv2_len = prev_rv2.length();
  Real norm = 0.;

  // check previous vectors for zeros
  bool zero_prev = false, zero_curr = false;
  for (i=0; i<rv1_len; ++i)
    if (std::abs(prev_rv1[i]) < Pecos::SMALL_NUMBER)
      { zero_prev = true; break; }
  if (!zero_prev)
    for (i=0; i<iv_len; ++i)
      if (std::abs(prev_iv[i]))
	{ zero_prev = true; break; }
  if (!zero_prev)
    for (i=0; i<rv2_len; ++i)
      if (std::abs(prev_rv2[i]) < Pecos::SMALL_NUMBER)
	{ zero_prev = true; break; }
  // check current vectors for zeros
  if (zero_prev) {
    for (i=0; i<rv1_len; ++i)
      if (std::abs(curr_rv1[i]) < Pecos::SMALL_NUMBER)
	{ zero_curr = true; break; }
    if (!zero_prev)
      for (i=0; i<iv_len; ++i)
	if (std::abs(curr_iv[i]))
	  { zero_curr = true; break; }
    if (!zero_prev)
      for (i=0; i<rv2_len; ++i)
	if (std::abs(curr_rv2[i]) < Pecos::SMALL_NUMBER)
	  { zero_curr = true; break; }
  }

  // Compute norm of relative change one of three ways
  if (!zero_prev) { // change relative to previous
    for (i=0; i<rv1_len; ++i)
      norm += std::pow(curr_rv1[i] / prev_rv1[i] - 1., 2.);
    for (i=0; i<iv_len; ++i)
      norm += std::pow(curr_iv[i]  / prev_iv[i]  - 1., 2.);
    for (i=0; i<rv2_len; ++i)
      norm += std::pow(curr_rv2[i] / prev_rv2[i] - 1., 2.);
    return std::sqrt(norm);
  }
  else if (!zero_curr) { // change relative to current
    for (i=0; i<rv1_len; ++i)
      norm += std::pow(prev_rv1[i] / curr_rv1[i] - 1., 2.);
    for (i=0; i<iv_len; ++i)
      norm += std::pow(prev_iv[i]  / curr_iv[i]  - 1., 2.);
    for (i=0; i<rv2_len; ++i)
      norm += std::pow(prev_rv2[i] / curr_rv2[i] - 1., 2.);
    return std::sqrt(norm);
  }
  else { // absolute change scaled by norm of previous
    Real scaling = 0.;
    for (i=0; i<rv1_len; ++i) {
      norm    += std::pow(curr_rv1[i] - prev_rv1[i], 2.);
      scaling += prev_rv1[i] * prev_rv1[i];
    }
    for (i=0; i<iv_len; ++i) {
      norm    += std::pow(curr_iv[i] - prev_iv[i], 2.);
      scaling += prev_iv[i] * prev_iv[i];
    }
    for (i=0; i<rv2_len; ++i) {
      norm    += std::pow(curr_rv2[i] - prev_rv2[i], 2.);
      scaling += prev_rv2[i] * prev_rv2[i];
    }
    return (scaling > Pecos::SMALL_NUMBER) ?
      std::sqrt(norm / scaling) : std::sqrt(norm);
  }
}

} // namespace Dakota
