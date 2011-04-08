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


} // namespace Dakota
