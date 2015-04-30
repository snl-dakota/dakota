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
// pbPack.cpp
//
// Parallel branching class code for PEBBL -- contains code for packing and 
// unpacking generic problem and subproblem (including token) information).
//
// Jonathan Eckstein
//


#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <pebbl/parBranching.h>
#include <pebbl/packPointer.h>

using namespace std;

namespace pebbl {


//  Pack and unpack generic parallelBranching Information.
//  Typically followed by a call to "pack" or "unpack" to pack/unpack
//  application-specific stuff.

void parallelBranching::packGeneric(PackBuffer& outBuffer)
{
  DEBUGPR(120, ucout << "entered packGeneric, outBuffer = " 
	  << &outBuffer << "\n");
  outBuffer << (int) sense;
  outBuffer << incumbentValue;
  outBuffer << problemName;
  // CAP: changed to get this to compile.  
  // If we don't have a local relTol, absTol now, is there
  // any need to pack this?
  outBuffer << relTolerance;
  outBuffer << absTolerance;
  DEBUGPR(120, ucout << "exiting packGeneric, outBuffer curr = "
	  << outBuffer.curr() << endl);
}


void parallelBranching::unpackGeneric(UnPackBuffer& inBuffer)
{
  unpackAs(inBuffer,sense,optimType,int);
  inBuffer >> incumbentValue;
  inBuffer >> problemName;
  // CAP: see comment in pack routine above
  inBuffer >> relTolerance;
  inBuffer >> absTolerance;
}


//  The same stuff, but for subproblems.

void parallelBranchSub::packGeneric(PackBuffer& outBuffer)
{
  DEBUGPRX(150,bGlobal(),"Packing " << this << '\n');
  outBuffer << bound;
  outBuffer << integralityMeasure;
  outBuffer << id;
  outBuffer << (int) state;
  outBuffer << depth;
  outBuffer << totalChildren;
  outBuffer << childrenLeft;
  if (valLogOutput() && !pGlobal()->writingCheckpoint)
    valLogPackPrint();
}


// Cindy contributed this -- for packing the generic parts of subproblems
// that we are packing but not completely generating.

void parallelBranchSub::packChildGeneric(PackBuffer& outBuffer)
{
  DEBUGPRX(150,bGlobal(),"Packing Child of " << this << '\n');
  if (--childrenLeft < 0)
    EXCEPTION_MNGR(runtime_error,"Trying to pack too many children of a subproblem");
  outBuffer << bound;                      // This is the parent's bound
  outBuffer << integralityMeasure;
  branchSubId newId;
  newId.branchSubIdFromBranching(bGlobal());
  outBuffer << newId;
  // The state.  Currently use branchSub default in milpNode constructors
  outBuffer << (int) boundable; 
  // This is where creation is recorded
  bGlobal()->subCount[boundable]++;
  outBuffer << depth + 1;
  outBuffer << 0;                          // totalChildren
  outBuffer << 0;                          // childrenLeft
  if (valLogOutput())
    valLogPackChildPrint();
}


void parallelBranchSub::unpackGeneric(UnPackBuffer& inBuffer)
{
  inBuffer >> bound;
  inBuffer >> integralityMeasure;
  inBuffer >> id;
  unpackAs(inBuffer,state,subState,int);
  inBuffer >> depth;
  inBuffer >> totalChildren;
  inBuffer >> childrenLeft;
  DEBUGPRX(150,bGlobal(),"Unpacked " << this << '\n');
}


void spToken::pack(PackBuffer& outBuffer)
{
  outBuffer << bound;
  outBuffer << integralityMeasure;
  outBuffer << (int) state;
  outBuffer << id;
  outBuffer << depth;
  outBuffer << childrenRepresented;
  outBuffer << spProcessor;
  outBuffer << whichChild;
  packPointer(outBuffer,memAddress);
};



void spToken::unpack(UnPackBuffer& inBuffer)
{
  inBuffer >> bound;
  inBuffer >> integralityMeasure;
  unpackAs(inBuffer,state,subState,int);
  inBuffer >> id;
  inBuffer >> depth;
  inBuffer >> childrenRepresented;
  inBuffer >> spProcessor;
  inBuffer >> whichChild;
  memAddress = (parallelBranchSub*) unpackPointer(inBuffer);
};

} // namespace pebbl


utilib::PackBuffer& operator<<(utilib::PackBuffer& buf, 
			       const pebbl::branchSubId& id)
{
  return buf << id.serial << id.creatingProcessor;
}


utilib::UnPackBuffer& operator>>(utilib::UnPackBuffer& buf, 
				 pebbl::branchSubId& id)
{
  return buf >> id.serial >> id.creatingProcessor;
}


utilib::PackBuffer& operator<<(utilib::PackBuffer& buf, 
			       const pebbl::solutionIdentifier& solId)
{
  solId.pack(buf);
  return buf;
}


utilib::UnPackBuffer& operator>>(utilib::UnPackBuffer& buf, 
				 pebbl::solutionIdentifier& solId)
{
  solId.unpack(buf);
  return buf;
}




#endif
