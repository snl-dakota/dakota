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
 * \file serialCore.h
 *
 * Illustrates the core methods that need to be extended to use the PEBBL
 * branching framework in serial.
 */

#ifndef pebbl_serialCore_h
#define pebbl_serialCore_h

#include <acro_config.h>
#include <pebbl/branching.h>

namespace pebbl_CoreExample {


/**
 * Defines the core methods needed to extend the PEBBL branching class.
 */
class coreBranching : virtual public pebbl::branching 
{
public:

  /**
   * An empty constructor for a branching object.
   * A branching object must be initialized with a call to branchingInit.
   */
  coreBranching()
	{
	branchingInit();
	}
                
  /// A destructor
  virtual ~coreBranching()
	{ }

  /// Return a new subproblem
  pebbl::branchSub* blankSub();

};


/**
 * Defines the core methods needed to extend the PEBBL branchSub class.
 */
class coreBranchSub : virtual public pebbl::branchSub
{
public:

  /// Return a pointer to the global branching object
  coreBranching* global() const
	{ return globalPtr; }

  /// Return a pointer to the base class of the global branching object
  pebbl::branching* bGlobal() const
	{ return global(); }

  /**
   * Link the debugging in the subproblem to the debugging level
   * set within the global branching object.  This is not strictly necessary,
   * but it is advisable in practice.
   */
  REFER_DEBUG(global())

  /// An empty constructor for a subproblem
  coreBranchSub()
	{ }

  /// A virtual destructor for a subproblem
  virtual ~coreBranchSub()
	{ }

  /**
   * Initialize a subproblem using a branching object
   * This method is not strictly necessary, but its use here illustrates
   * a flexible mechanism for managing the initialization of subproblems.
   * The following crude fragment illustrates some common steps needed in
   * this method.
   */
  void initialize(coreBranching* master)
	{
	globalPtr = master;
	}

  /**
   * Initialize a subproblem as a child of a parent subproblem.
   * This method is not strictly necessary, but its use here illustrates
   * a flexible mechanism for managing the initialization of subproblems.
   * The following crude fragment illustrates some common steps needed in
   * this method.
   */
  void initialize(coreBranchSub* parent, int /*whichChild*/)
	{
	globalPtr = parent->globalPtr;
	branchSubAsChildOf(parent);
	}


  /// Initialize this subproblem to be the root of the branching tree
  virtual void setRootComputation()
	{ }

  /**
   * Compute the lower bound on this subproblem's value.
   * The value of controlParam is not currently defined.
   *
   * When this routine is finished the state of the subproblem should be
   * set to bounded OR dead.
   */
  void boundComputation(double* /*controlParam*/)
	{ }

  /**
   * Determine how many children will be generated.
   * This code snippet illustrates how this routine should complete this
   * calculation so-as to properly indicate the new state of the subproblem.
   * The return value is the number of children that can be generated from
   * this node.
   *
   * When this routine is finished the state of the subproblem should be
   * set to separated.
   */
  virtual int splitComputation()
	{ return -1; }

  /// Create a child subproblem of the current subproblem
  virtual branchSub* makeChild(int whichChild)
	{
	coreBranchSub *temp = new coreBranchSub;
	temp->initialize(this, whichChild);
	return temp;
	}
  
  /// Returns true if this subproblem represents a leaf node
  bool candidateSolution()
	{ return false; }

  /// Get a solution object for a subproblem
  pebbl::solution* extractSolution()
        { return NULL; }

  // JE: Don't want this any more?
//   /**
//    * Updates the incumbent solution.
//    * This method is not strictly necessary, but it is likely to be redefined
//    * in practice.
//    */
//   void foundSolution() 
//     { }


protected:

  /// A pointer to the global branching object
  coreBranching* globalPtr;

};


inline pebbl::branchSub* coreBranching::blankSub()
{
coreBranchSub *temp = new coreBranchSub();
temp->initialize(this);
return temp;
}


} // namespace pebbl_serialCore

#endif
