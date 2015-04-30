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
 * \file parallelCore.h
 *
 * Illustrates the core methods that need to be extended to use the PEBBL
 * branching framework in parallel.
 *
 * \todo The parallelCore example needs to be reworked!
 */

#include <acro_config.h>
#if 0

#ifndef pebbl_parallelCore_h
#define pebbl_parallelCore_h

#ifdef ACRO_HAVE_MPE

#include <pebbl/parBranching.h>
#include <pebbl/serialCore.h>

namespace pebbl_CoreExample {

/**
 * Defines the core methods needed to extend the PEBBL parBranching class.
 */
class coreParallelBranching : public pebbl::parallelBranching, public coreBranching
{
public:

  /// An empty constructor for a branching object
  coreParallelBranching()
	{ }
                
  /// A destructor
  virtual ~coreParallelBranching()
	{ }

  /// Return a new subproblem
  pebbl::parallelBranchSub* blankParallelSub();

  /// Pack the branching information into a buffer
  void pack(utilib::PackBuffer& outBuffer)
	{ }

  /// Unpack the branching information from a buffer
  void unpack(utilib::UnPackBuffer& inBuffer)
	{ }

  /// Compute the size of the buffer needed for a subproblem
  int spPackSize()
	{ return 0; }

};


/**
 * Defines the core methods needed to extend the PEBBL branchSub class.
 */
class coreParallelBranchSub : public pebbl::parallelBranchSub, public coreBranchSub
{
public:

  /// Return a pointer to the global branching object
  coreParallelBranching* global() const
	{ return globalPtr; }

  /// Return a pointer to the base class of the global branching object
  pebbl::parallelBranching* pGlobal() const
	{ return global(); }

  /// An empty constructor for a subproblem
  coreParallelBranchSub()
	{ }

  /// A virtual destructor for a subproblem
  virtual ~coreParallelBranchSub()
	{ }

  /**
   * Initialize a subproblem using a branching object
   * This method is not strictly necessary, but its use here illustrates
   * a flexible mechanism for managing the initialization of subproblems.
   * The following cude fragment illustrates some common steps needed in
   * this method.
   */
  void initialize(coreParallelBranching* master)
	{
	globalPtr = master;
	initialize( (coreBranching*)master );
	}

  /**
   * Initialize a subproblem as a child of a parent subproblem.
   * This method is not strictly necessary, but its use here illustrates
   * a flexible mechanism for managing the initialization of subproblems.
   * The following cude fragment illustrates some common steps needed in
   * this method.
   */
  void initialize(coreParallelBranchSub* parent,int whichChild)
	{
	globalPtr = parent->globalPtr;
	initialize( (coreBranchSub*)parent, whichChild);
	}


  /// Pack the information in this subproblem into a buffer
  void pack(utilib::PackBuffer& outBuffer)
	{ }

  /// Unpack the information for this subproblem from a buffer
  void unpack(utilib::UnPackBuffer& inBuffer)
	{ }

  /// Create a child subproblem of the current subproblem
  virtual parallelBranchSub* makeParallelChild(int whichChild)
	{
	coreParallelBranchSub *temp = new coreParallelBranchSub;
	temp->initialize(this, whichChild);
	return temp;
	}
  
protected:

  /// A pointer to the global branching object
  coreParallelBranching* globalPtr;

};


inline pebbl::parallelBranchSub* coreParallelBranching::blankParallelSub()
{
coreParallelBranchSub *temp = new coreParallelBranchSub();
temp->initialize(this);
return temp;
}


} // namespace pebbl_parallelCore

#endif

#endif

#endif
