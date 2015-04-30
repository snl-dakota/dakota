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
 * \file parallelLipshitzian.h
 *
 * A simple nonlinear branch-and-bound method using a Lipshitz constant to
 * compute a lower bound.
 */

#ifndef pebbl_parallelLipshitzian_h
#define pebbl_parallelLipshitzian_h

#include <acro_config.h>
#include <pebbl/serialLipshitzian.h>
#ifdef ACRO_HAVE_MPI
#include <utilib/CharString.h>
#include <pebbl/parBranching.h>

namespace pebbl {

#if !defined(TFLOPS)
using pebbl::parallelBranching;
#endif

/**
 * Defines the branching class for a parallel Lipshitzian optimizer
 */
template <class FunctionT>
class parallelLipshitzian : public parallelBranching, public serialLipshitzian<FunctionT>
{
public:

  #if !defined(SOLARIS)
  using serialLipshitzian<FunctionT>::Lipshitz_constant;
  using serialLipshitzian<FunctionT>::func;
  #endif

  /// An empty constructor for a branching object
  parallelLipshitzian()
	: buffer_size(0)
	{ }
                
  /// A destructor
  virtual ~parallelLipshitzian()
	{ }

  /// Note: should this include the VB flag?
  void reset(bool VBflag=true)
    {
      serialLipshitzian<FunctionT>::reset(VBflag);
      size_type numVars = func->lower.size();
      registerFirstSolution(new arraySolution<double>(numVars,this));
      parallelBranching::reset(false);
     }

  /// Return a new subproblem
  pebbl::parallelBranchSub* blankParallelSub();

  /// Pack the branching information into a buffer
  void pack(utilib::PackBuffer& outBuffer)
	{
	outBuffer << Lipshitz_constant;
	unsigned int len = func->lower.size();
	outBuffer << len;
	for (unsigned int i=0; i<len; i++)
	  outBuffer << func->lower[i] << func->upper[i];
	double flag = -999.0;
	outBuffer << flag;
	}

  /// Unpack the branching information from a buffer
  void unpack(utilib::UnPackBuffer& inBuffer)
	{
	inBuffer >> Lipshitz_constant;
	unsigned int len;
	inBuffer >> len;
	func->lower.resize(len);
	func->upper.resize(len);
	for (unsigned int i=0; i<len; i++)
	  inBuffer >> func->lower[i] >> func->upper[i];
	double flag;
	inBuffer >> flag;
	if (flag != -999.0)
	   EXCEPTION_MNGR(std::runtime_error,"parallelLipshitzian - problem unpacking");
	}

  /// Compute the size of the buffer needed for a subproblem
  int spPackSize()
	{
	if (buffer_size == 0) {
	   PackBuffer buffer;
	   pebbl::parallelBranchSub* tmp = blankParallelSub();
	   tmp->pack(buffer);
	   buffer_size = 1000*buffer.size();
	   delete tmp;
	   //ucout << "parallelLipshitzian::spPackSize - size initialized to " << buffer_size << endl;
	   }
	return buffer_size;
	}

  ///
  bool setup(int& argc,char**& argv, FunctionT& func_) 
    { 
      func = &func_;
      return parallelBranching::setup(argc,argv);
    }

  ///
  bool setup(int& argc,char**& argv)
    { return parallelBranching::setup(argc,argv); }

  ///
  void printSolution(const char* header = "",
		     const char* footer = "",
		     std::ostream& outStream = ucout)
    {
      parallelBranching::printSolution(header,footer,outStream);
    }



protected:

  /// Buffer size
  int buffer_size;
};


/**
 * Defines the core methods needed to extend the PEBBL branchSub class.
 */
template <class FunctionT>
class parallelLipshitzianNode : public parallelBranchSub, public serialLipshitzianNode<FunctionT>
{
  #if !defined(SOLARIS)
  using serialLipshitzianNode<FunctionT>::branchVariable;
  using serialLipshitzianNode<FunctionT>::lower;
  using serialLipshitzianNode<FunctionT>::upper;
  using serialLipshitzianNode<FunctionT>::branch_status;
  using serialLipshitzianNode<FunctionT>::midpoint;
  using serialLipshitzianNode<FunctionT>::corner_dist;
  using serialLipshitzianNode<FunctionT>::midpoint_value;
  #endif

public:

  /// Return a pointer to the global branching object
  parallelLipshitzian<FunctionT>* global() const
	{ return globalPtr; }

  /// Return a pointer to the base class of the global branching object
  pebbl::parallelBranching* pGlobal() const
	{ return global(); }

  /// An empty constructor for a subproblem
  parallelLipshitzianNode()
	{ }

  /// A virtual destructor for a subproblem
  virtual ~parallelLipshitzianNode()
	{ }

  /**
   * Initialize a subproblem using a branching object
   */
  void initialize(parallelLipshitzian<FunctionT>* master)
	{
	globalPtr = master;
	serialLipshitzianNode<FunctionT>::initialize( master );
	}

  /**
   * Initialize a subproblem as a child of a parent subproblem.
   */
  void initialize(parallelLipshitzianNode<FunctionT>* parent,int whichChild)
	{
	globalPtr = parent->globalPtr;
	serialLipshitzianNode<FunctionT>::initialize(parent, whichChild);
	}

  /// Pack the information in this subproblem into a buffer
  void pack(utilib::PackBuffer& outBuffer)
	{
	unsigned int len = midpoint.size();
	outBuffer << len;
	for (unsigned int i=0; i<len; i++)
          outBuffer << midpoint[i] << lower[i] << upper[i];

	outBuffer << midpoint_value << corner_dist << branchVariable;
	if (branch_status == serialLipshitzian<FunctionT>::no_branch)
	   outBuffer << "no_branch";
	if (branch_status == serialLipshitzian<FunctionT>::branch_down)
   	   outBuffer << "branch_down";
	if (branch_status == serialLipshitzian<FunctionT>::branch_up)
	   outBuffer << "branch_up";

	double flag = -999.0;
	outBuffer << flag;
	}

  /// Unpack the information for this subproblem from a buffer
  void unpack(utilib::UnPackBuffer& inBuffer)
	{
	unsigned int len;
	inBuffer >> len;
	for (unsigned int i=0; i<len; i++)
          inBuffer >> midpoint[i] >> lower[i] >> upper[i];

	inBuffer >> midpoint_value >> corner_dist >> branchVariable;
	//
	// Need to be be careful packing and unpacking enum types
	//
	utilib::CharString branch_code;
	inBuffer >> branch_code;
	if (branch_code == "branch_down")
	   branch_status = serialLipshitzian<FunctionT>::branch_down;
	if (branch_code == "branch_up")
	   branch_status = serialLipshitzian<FunctionT>::branch_up;
	if (branch_code == "no_branch")
	   branch_status = serialLipshitzian<FunctionT>::no_branch;

	double flag;
	inBuffer >> flag;
	if (flag != -999.0)
	   EXCEPTION_MNGR(std::runtime_error,"parallelLipshitzianNode - problem unpacking");
	}

  /// Create a child subproblem of the current subproblem
  virtual parallelBranchSub* makeParallelChild(int whichChild)
	{
	parallelLipshitzianNode<FunctionT> *temp = new parallelLipshitzianNode<FunctionT>;
	temp->initialize(this, whichChild);
	return temp;
	}
  
protected:

  /// A pointer to the global branching object
  parallelLipshitzian<FunctionT>* globalPtr;

};


template <class FunctionT>
pebbl::parallelBranchSub* parallelLipshitzian<FunctionT>::blankParallelSub()
{
parallelLipshitzianNode<FunctionT> *temp = new parallelLipshitzianNode<FunctionT>();
temp->initialize(this);
return temp;
}


} // namespace pebbl

#endif

#endif
