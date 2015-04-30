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
// loadObject.cpp
//
// Generic objects for keeping track of workload in PEBBL.
// This version has been stripped of any parallel code objects.
//
// Jonathan Eckstein
//

#include <acro_config.h>
#include <utilib/mpiUtil.h>
#include <pebbl/loadObject.h>
#include <pebbl/branching.h>

using namespace std;

namespace pebbl {



void loadObject::reset()
{
  resetSums();

  pCount         = 0;
  createdSPs     = 0;
  boundedSPs     = 0;
  flags          = 0;

  if (bGlobal)
    {
      incumbentValue = bGlobal->incumbentValue;
      if (bGlobal->enumerating)
	fathomValue = bGlobal->fathomValue();
      else
	fathomValue = incumbentValue;
      aggregateBound = bGlobal->sense*MAXDOUBLE;
      if (bGlobal->enumerating)
	{
	  repositorySize = bGlobal->repositorySize();
	  worstInRepos   = bGlobal->worstReposValue();
	}
      else
	{
	  repositorySize = 0;
	  worstInRepos   = 0;
	}
    }

#ifdef MEMUTIL_PRESENT
  memUsed = 0;
#endif
}


void loadObject::init(branching* bGlobal_, bool useSPCounts_)
{
  bGlobal = bGlobal_;
  if (bGlobal_)
    powerSum.resize(bGlobal_->loadMeasureDegree);
  useSPCounts = useSPCounts_;
  reset();
}


void loadObject::update()
{
  incumbentValue = bGlobal->incumbentValue;
  if (bGlobal->enumerating)
    {
      fathomValue    = bGlobal->fathomValue();
      repositorySize = bGlobal->repositorySize();
      worstInRepos   = bGlobal->worstReposValue();
    }
  else
    fathomValue = incumbentValue;
  if (useSPCounts)
    {
      boundedSPs = bGlobal->subCount[beingBounded];
      createdSPs = bGlobal->subCount[boundable];
    }
}


void loadObject::setBound(double b)
{
  flags &= ~boundUnknownBit;
  aggregateBound = b;
}


void loadObject::adjustBound(double b)
{
  if (!boundUnknown() && (bGlobal->sense*(b - aggregateBound) < 0))
    aggregateBound = b;
}


void loadObject::operator+=(const coreSPInfo& sp)
{ 
  addLoad(sp.boundEstimate(),sp.loadXFactor()); 
}


void loadObject::operator-=(const coreSPInfo& sp)
{ 
  subtractLoad(sp.boundEstimate(),sp.loadXFactor()); 
}


double loadObject::loadMeasure() const
{
  if (pCount == 0)
    return 0;
#ifdef ACRO_VALIDATING
  if (bGlobal == 0)
    EXCEPTION_MNGR(runtime_error,"loadObject::loadMeasure() called "
                                 "with null bGlobal pointer");
#endif
  double bound  = bGlobal->fathomValue();
  int    sense  = bGlobal->sense;
  int    degree = bGlobal->loadMeasureDegree;
  if (bound == sense*MAXDOUBLE)
    return pCount;
  // Added code to deal with an incumbent value of zero
  if (bound == 0)
    {
      double sign = (degree & 1 ? -1.0 : +1.0);
      return sense*sign*powerSum[degree-1];
    }
  double multiplier = 1;
  int p;
  for(p=0; p<degree; p++)
    multiplier *= sense*bound;
  double sum = multiplier*pCount;
  const double* ptr = powerSum.data();
  for(p=0; p<degree; )
    {
      multiplier *= (p - degree);
      multiplier /= (bound*(++p));
      sum += multiplier*(*(ptr++));
    }
  return sum;
}


void loadObject::resetSums()
{
  for(unsigned int p=0; p<powerSum.size(); p++)
    powerSum[p] = 0;
}


void loadObject::addLoad(double bound,int multiplicity)
{
  pCount += multiplicity;
  double product = multiplicity;
  for(int p=0; p<bGlobal->loadMeasureDegree; p++)
    {
      product *= bound;
      powerSum[p] += product;
    }
  updateAggBound(bound);
}


void loadObject::operator+=(const loadObject& other)
{
  if (other.pCount == 0)
    {
      flags |= (other.flags & ~boundUnknownBit);
    }
  else if (pCount == 0)
    {
      flags = (flags & ~boundUnknownBit) | other.flags;
    }
  else 
    {
      flags |= other.flags;
    }
  
  if (other.pCount > 0)
    updateAggBound(other.aggregateBound);

  pCount += other.pCount;
    
  for(int p=0; p<bGlobal->loadMeasureDegree; p++)
    powerSum[p] += other.powerSum[p];

  boundedSPs += other.boundedSPs;
  createdSPs += other.createdSPs;
  
  if (incumbentValue != other.incumbentValue)
    {
      DEBUGPRX(200,bGlobal,"Incumbent mismatch: " << incumbentValue
	       << " versus " << other.incumbentValue << '\n');
      setMismatch();
      if ((bGlobal->sense)*(other.incumbentValue - incumbentValue) < 0)
	incumbentValue = other.incumbentValue;
    }

  if (bGlobal->enumerating)
    {
      repositorySize += other.repositorySize;
      if ((bGlobal->sense)*(other.worstInRepos - worstInRepos) > 0)
	worstInRepos = other.worstInRepos;
      if (fathomValue != other.fathomValue)
	{
	  setFathomMismatch();
	  if ((bGlobal->sense)*(other.fathomValue - fathomValue) < 0)
	    fathomValue = other.fathomValue;
	}
    }

#ifdef MEMUTIL_PRESENT
  if (other.memUsed > memUsed) 
     memUsed = other.memUsed;
#endif

}


void loadObject::updateAggBound(double bound)
{
  if (!boundUnknown())
    if ((pCount == 0) ||
	((bGlobal->sense)*(bound - aggregateBound) < 0))
      aggregateBound = bound;
}


void loadObject::subtractLoad(double bound,int multiplicity)
{
  pCount -= multiplicity;
  if (pCount == 0)
    {
      aggregateBound = bGlobal->sense*MAXDOUBLE;
      resetSums();
      return;
    }
  else if (pCount < 0)
    EXCEPTION_MNGR(runtime_error, 
		   "Load object contains a negative number of subproblems");
  double product = multiplicity;
  for(int p=0; p<bGlobal->loadMeasureDegree; p++)
    {
      product *= bound;
      powerSum[p] -= product;
    }
}


void loadObject::dump(std::ostream& stream,const char* name)
{
  stream << objectTypeString() << ' ' << name << " : " << *this << endl;
  stream << "aggregateBound = " << aggregateBound
         << ", incumbentValue = " << incumbentValue << endl;
  stream << "fathomValue = " << fathomValue << endl;
  stream << "Subproblems: processed=" << boundedSPs
	 << ", created=" << createdSPs << endl;
  if (bGlobal->enumerating)
    stream << "repositorySize = " << repositorySize << endl
	   << "worstInRepos = " << worstInRepos << endl;
  stream << "Flags=" << (int) flags << ": " 
	 << (busy() ? "busy " : "")
	 << (boundUnknown() ? "boundUnknown " : "")
	 << (mismatch() ? "mismatch " : "") 
	 << (cpBusy() ? "cpBusy " : "" ) 
         << (fathomMismatch() ? "fathomMismatch " : "")
         << (reposMergePending() ? "reposMergePending " : "")
	 << endl;
#ifdef MEMUTIL_PRESENT
  stream << "memUsed = "  << memUsed << endl;
#endif
}



} // namespace pebbl

