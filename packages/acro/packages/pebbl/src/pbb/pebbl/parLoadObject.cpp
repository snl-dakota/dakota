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
// parLoadObject.cpp
//
// Load object tracker for parallel branching search.
//
// Jonathan Eckstein
//


#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <pebbl/parBranching.h>

using namespace std;

namespace pebbl {


void parLoadObject::reset()
{
  loadObject::reset();
  messages.reset();
  hubTrack.reset();
  lastHubTrack.reset();
  if (bGlobal)
    serverBound = bGlobal->sense*MAXDOUBLE;
  else
    serverBound = MAXDOUBLE;
};


void parLoadObject::init(parallelBranching* pGlobal_, 
			 bool useSPCounts_, 
			 bool useMessageCounts_)
{
  loadObject::init(pGlobal_,useSPCounts_);
  
  pGlobal = pGlobal_;
  useMessageCounts = useMessageCounts_;

  if (pGlobal_)
    aggregateBound = (pGlobal->sense)*MAXDOUBLE;
  else
    aggregateBound = MAXDOUBLE;

  serverBound = aggregateBound;  // Initialization is identical to agg bound
  
  setIncumbentUnknown();
  messages.reset();   
  hubTrack.reset();
  lastHubTrack.reset();
}


void parLoadObject::update()
{
  loadObject::update();
  incumbentSource = pGlobal->incumbentSource;
  if (incumbentSource == MPI_ANY_SOURCE)
    {
      if (pGlobal->parameter_initialized("startIncumbent"))
	incumbentValue = pGlobal->startIncumbent;
      else
	incumbentValue = pGlobal->sense*MAXDOUBLE;
    }
  if (pGlobal->enumerating)
    fathomValue = pGlobal->fathomValue();
  else
    fathomValue = incumbentValue;
  if (useMessageCounts)
    messages = pGlobal->messages;
  setCPBusy(!(pGlobal->suspending()));
  if (pGlobal->iAmWorker())
    serverBound = pGlobal->serverPool.updatedLoad().aggregateBound;
};


void parLoadObject::setIncumbentUnknown()
{
  incumbentSource = MPI_ANY_SOURCE;
  if (pGlobal)
    {
      if (pGlobal->parameter_initialized("startIncumbent"))
	incumbentValue = pGlobal->startIncumbent;
      else
	incumbentValue = pGlobal->sense*MAXDOUBLE;
    }
  else
    incumbentValue = MAXDOUBLE;

  fathomValue = incumbentValue;
}


void parLoadObject::operator+=(const parLoadObject& other)
{
  loadObject::operator+=(other);

  if (incumbentSource != other.incumbentSource)
    {
      DEBUGPRX(200,bGlobal,"Incumbent source mismatch: [" << incumbentSource
	       << "] versus [" << other.incumbentSource << "]\n");
      setMismatch();
    }
  
  messages     += other.messages;
  hubTrack     += other.hubTrack;
  lastHubTrack += other.lastHubTrack;

  if (bGlobal->sense*(other.serverBound - serverBound) < 0)
    serverBound = other.serverBound;
};


bool parLoadObject::senseClusterBusy()
{
  loadObject::senseBusy();
  if (!(pGlobal->suspending()))
    setCPBusy();
  if (mismatch())
    setBothBusy();
  else if (!messages.localScatter.inBalance() ||
	   !messages.hubDispatch.inBalance())
    setBothBusy();
  // No longer use needReposMerge to set busy flag; just set 
  // a special bit checked by the quiescence poll
  if (pGlobal->needReposMerge)
    setReposMergePending();
  DEBUGPRX(100,bGlobal,"senseClusterBusy: flags set to " 
	   << (int) flags << endl);
  return busy();
}


bool parLoadObject::senseBusy()
{
  senseClusterBusy();
  if (!messages.nonLocalScatter.inBalance())
    setBothBusy();
  // No longer check messages.general message balance here.
  // That is now done by the quiescence poll operation
  return busy();
}


bool parLoadObject::readyToPoll()
{
  if (!pGlobal->suspending() && count() > 0)
    return false;
  if (!messages.hubDispatch.inBalance())
    return false;
  if (!messages.localScatter.inBalance())
    return false;
  return messages.nonLocalScatter.inBalance();
}


bool parLoadObject::seemsReallyDone()
{
  if (!readyToPoll())
    return false;
  if (reposMergePending())
    return false;
  return messages.general.inBalance();
}


bool parLoadObject::reportablyDifferent(const parLoadObject& other)
{
  if (count() != other.count())
    return true;
  DEBUGPRX(1000,pGlobal,"reportablyDifferent: Counts match.\n");
  // // Don't test flags now -- *should* be handled by quiescence test
  // if (flags != other.flags)
  //   return true;
  // DEBUGPRX(1000,pGlobal,"reportablyDifferent: flags match.\n");
  if ((incumbentValue  != other.incumbentValue)   ||
      (incumbentSource != other.incumbentSource))
    return true;
  if (fathomValue != other.fathomValue)
    return true;
  DEBUGPRX(1000,pGlobal,"reportablyDifferent: incumbents match.\n");
  if (!(messages.localScatter == other.messages.localScatter))
    return true;
  if (!(messages.hubDispatch == other.messages.hubDispatch))
    return true;
  // No longer check change in general message counts as a reason 
  // for a reportable difference.  Balance of these messages is checked
  // by an active poll operation.  Just check localScatter, hubDispatch, 
  // and nonLocalScatter messages.
  return !(messages.nonLocalScatter == other.messages.nonLocalScatter);
}


void parLoadObject::packEmpty(PackBuffer& buf, parallelBranching* pGlobal_)
{
  parLoadObject junkLoad(pGlobal_);
#ifdef ACRO_VALIDATING
  //
  // WEH - initialize this data to avoid memory warnings in purify.
  //	Note:  this _could_ hide some bugs.  
  //
  junkLoad.aggregateBound = -999.0;
  junkLoad.incumbentValue = -999.0;
  junkLoad.fathomValue    = -999.0;
  junkLoad.pCount = -999;
  junkLoad.boundedSPs = -999;
  junkLoad.createdSPs = -999;
  junkLoad.incumbentSource = -999;
  junkLoad.flags = '\000';
  junkLoad.repositorySize = -999;
  junkLoad.worstInRepos = -999;
#ifdef MEMUTIL_PRESENT
  junkLoad.memUsed = -999.0;
#endif
  junkLoad.serverBound = -999.0;
#endif
  buf << junkLoad;
}


int parLoadObject::packSize(parallelBranching* pGlobal_)
{
  PackBuffer    junkBuffer(128);
  parLoadObject::packEmpty(junkBuffer,pGlobal_);
  junkBuffer << (double) 0.0;
  return junkBuffer.size();
};


void parLoadObject::write(PackBuffer& buff) const
{
  for(int i=pGlobal->loadMeasureDegree - 1; i>=0; i--)
    buff << powerSum[i];
  buff << aggregateBound;
  buff << incumbentValue;
  if (pGlobal->enumerating)
    buff << fathomValue;
  buff << serverBound;
  buff << hubTrack;
  buff << lastHubTrack;
  buff << pCount;
  buff << boundedSPs;
  buff << createdSPs;
  buff << repositorySize;
  buff << worstInRepos;
  buff << incumbentSource;
  buff << messages;
  buff << flags;
#ifdef MEMUTIL_PRESENT
  buff << memUsed;
#endif
}


void parLoadObject::read(UnPackBuffer& buff)
{
  for(int i=pGlobal->loadMeasureDegree - 1; i>=0; i--)
    buff >> powerSum[i];
  buff >> aggregateBound;
  buff >> incumbentValue;
  if (pGlobal->enumerating)
    buff >> fathomValue;
  else
    fathomValue = incumbentValue;
  buff >> serverBound;
  buff >> hubTrack;
  buff >> lastHubTrack;
  buff >> pCount;
  buff >> boundedSPs;
  buff >> createdSPs;
  buff >> repositorySize;
  buff >> worstInRepos;
  buff >> incumbentSource;
  buff >> messages;
  buff >> flags;
#ifdef MEMUTIL_PRESENT
  buff >> memUsed;
#endif
}

void parLoadObject::operator+=(const spToken& sp)
{ 
  addLoad(sp.boundEstimate(),sp.loadXFactor()); 
}


void parLoadObject::operator-=(const spToken& sp)
{ 
  subtractLoad(sp.boundEstimate(),sp.loadXFactor()); 
}


void parLoadObject::operator+=(const parallelBranchSub &sp)
{
  addLoad(sp.boundEstimate(),sp.loadXFactor());
}

void parLoadObject::operator-=(const parallelBranchSub &sp)
{
  subtractLoad(sp.boundEstimate(),sp.loadXFactor());
}


void parLoadObject::dump(std::ostream& stream, const char* name)
{
  stream << "----------------------\n";
  loadObject::dump(stream,name);
  stream << "serverBound = " << serverBound << '\n';
  stream << "incumbentSource = " << incumbentSource << '\n';
  stream << "localScatter: " << messages.localScatter << '\n';
  stream << "nonLocalScatter: " << messages.nonLocalScatter << '\n';
  stream << "hubDispatch: " << messages.hubDispatch << '\n';
  stream << "general: " << messages.general << '\n';
  stream << "----------------------\n";
}


} // namespace pebbl

#endif
