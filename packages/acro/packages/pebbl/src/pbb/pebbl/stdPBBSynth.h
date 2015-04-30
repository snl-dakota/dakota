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

/**
 * \file stdPBBSynth.h
 * \author Jonathan Eckstein
 *
 * Defines a standard synthesis of a concrete serial branching class
 * with the abstract class parallelBranching.
 */


// Just defining macros, so no need for the pragma stuff for gcc

// To declare the beginning of the definition of the parallel
// master class.

#include <acro_config.h>
#define declareParMaster(MASTERCLASS,BASEMASTERCLASS,SPCLASS,BASESPCLASS) \
class SPCLASS; \
  \
class MASTERCLASS : \
public parallelBranching, \
public BASEMASTERCLASS, \
virtual public branching \


// The standard merger of an instantiated serial class with the general
// parallel one (UGLY)...

#define mergeParMaster(MASTERCLASS,BASEMASTERCLASS,SPCLASS,BASESPCLASS) \
 \
  virtual void pack(PackBuffer& outBuffer); \
  virtual void unpack(UnPackBuffer& inBuffer); \
  virtual int  spPackSize(); \
 \
  parallelBranchSub* blankParallelSub(); \
 \
  void printSolution(const char* header = "", \
		     const char* footer = "", \
		     ostream& outStream = ucout) \
    { \
      parallelBranching::printSolution(header,footer,outStream); \
    }

#define mergeMasterConstruct(MASTERCLASS,BASEMASTERCLASS,SPCLASS,BASESPCLASS) \
  MASTERCLASS() : \
  BASEMASTERCLASS(), \
  BRANCHING_CONSTRUCTOR,

#define mergeMasterDestruct(MASTERCLASS,BASEMASTERCLASS,SPCLASS,BASESPCLASS) \
  ~MASTERCLASS() { }


//  Stuff to come at the end of the merged master classs definition
//  (currently nothing)

#define endMergeParMaster(MASTERCLASS,BASEMASTERCLASS,SPCLASS,BASESPCLASS) \
void dummyMethod()


// Now to declare the class for the subproblems

#define declareParSub(MASTERCLASS,BASEMASTERCLASS,SPCLASS,BASESPCLASS) \
class SPCLASS : \
public parallelBranchSub, \
public BASESPCLASS, \
virtual public branchSub


// To do the basic merger of the subproblems.

#define mergeParSub(MASTERCLASS,BASEMASTERCLASS,SPCLASS,BASESPCLASS) \
  MASTERCLASS* global()  \
    { \
      return (MASTERCLASS*) BASESPCLASS::global(); \
    }; \
 \
  void pack(PackBuffer& outBuffer); \
  void unpack(UnPackBuffer& inBuffer); \
 \
  SPCLASS(MASTERCLASS* master_) : \
  BASESPCLASS(master_) \
    { }; \
 \
  SPCLASS(SPCLASS* parent,int whichChild) : \
  branchSub(parent), \
  BASESPCLASS(parent,whichChild) \
    { }; \
 \
  ~SPCLASS() { }; \
 \
  parallelBranchSub* makeParallelChild(int whichChild) \
    { \
      return new SPCLASS(this,whichChild); \
    }; \
 \


//  To end the merger definition for subproblems

#define endMergeParSub(MASTERCLASS,BASEMASTERCLASS,SPCLASS,BASESPCLASS) }; \
 \
inline parallelBranchSub* MASTERCLASS::blankParallelSub() \
{  \
  return new SPCLASS(this);
