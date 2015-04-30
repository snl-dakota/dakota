/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */
//
// pebblBase.cpp
//
// A base class for PEBBL.  This class contains only static members (mainly 
// parameter definitions) and enum declarations.  The branching and branchSub
// classes are both derived from this class, so they effectively share these 
// declarations.
//
// Jonathan Eckstein
//

#include <acro_config.h>
#include <pebbl/pebblBase.h>

using namespace std;

namespace pebbl {

double pebblBase:: integerTolerance = 1e-5;

pebblBase::pebblBase()
{ } 

const char* pebblBase::stateStringArray[pebblBase::numStates] =
{
  "Created",
  "Started Bounding",
  "Bounded",
  "Started Splitting",
  "Split",
  "Dead"
  };

ostream* pebblBase::vout = 0;

ostream* pebblBase::hlog = 0;

// This is just a handy function used in multiple places
// (branching-type classes and branchsub-type classes)

bool pebblBase::isInteger(double val)
{
  double fracPart = val - floor(val);
  if (min(fracPart,1.0-fracPart) > integerTolerance)
    return(false);
  return(true);
}

// Again a handy function, particularly useful for incumbent routines which
// have to determine non-zero variables and will want to do this directly
// from the LP solution without looking at the list of fractional variables

bool pebblBase::isZero(double val)
{
  if (!isInteger(val)) return(false);
  if (fabs(val) <= integerTolerance) return(true);
  return(false);
}

} // namespace pebbl
