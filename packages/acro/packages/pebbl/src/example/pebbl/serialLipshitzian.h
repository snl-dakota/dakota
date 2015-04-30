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
 * \file serialLipshitzian.h
 *
 * A simple nonlinear branch-and-bound method using a Lipshitz constant to
 * compute a lower bound.
 *
 * TODO: allow for random selection of branching and/or branching based on
 * 		pseudo-cost estimates of the value of a given branch.
 * TODO: a more clever incubment policy
 * TODO: apply a pseudo-cost type idea to provide a bias for the selection
 *		of branching variables.
 */

#ifndef pebbl_serialLipshitzian_h
#define pebbl_serialLipshitzian_h

#include <acro_config.h>
#include <utilib/Ereal.h>
#include <utilib/BasicArray.h>
#include <pebbl/branching.h>

namespace pebbl {

using utilib::Ereal;
using utilib::BasicArray;



/**
 * Abstract Problem Type
 *
 * This illustrates the class structure of the problem class
 * used by the serialLipshitzian optimizer.
 */
class serialLipshitzianProblemExample
{
public:

  /// Virtual destructor
  virtual ~serialLipshitzianProblemExample() {}

  /// An operator that takes a point and returns the value of it
  virtual Ereal<double> operator()(BasicArray<double>& x) = 0;

  /// Lower bounds on the search domain
  BasicArray<double> lower;

  /// Upper bounds on the search domain
  BasicArray<double> upper;

};


/**
 * Defines the branching class for a serial Lipshitzian optimizer
 */
template <class ProblemT>
class serialLipshitzian : virtual public pebbl::branching 
{
public:

  /// An empty constructor
  serialLipshitzian()
    : func(0)
    {
      version_info += ", Lipshitzian example 0.1";
      branchingInit();
      Lipshitz_constant=0.0;
      ParameterSet::create_categorized_parameter("lipshitzConstant",
		  Lipshitz_constant,"<double>","0.0",
		  "An upper bound on the maximum L2 norm of the derivative",
		  "Lipschitzian Global Optimizer",
		  utilib::ParameterPositive<double>());
	}
                
  /// A destructor
  virtual ~serialLipshitzian()
	{ }

  ///
  bool setup(int& argc, char**& argv, ProblemT& func_)
	{
	func = &func_;
	return pebbl::branching::setup(argc,argv);
	}

  ///
  bool setup(int& argc, char**& argv)
	{return pebbl::branching::setup(argc,argv);}

  ///
  void reset(bool resetVB = true)
	{
	if (Lipshitz_constant <= 0.0)
	   EXCEPTION_MNGR(std::runtime_error,"Bad Lipshitz_constant: " <<
			Lipshitz_constant);
        pebbl::branching::reset(resetVB);
	}

  /// Return a new subproblem
  pebbl::branchSub* blankSub();

  ///
  //void printAllStatistics(std::ostream& stream = std::cout)
	//{
	//pebbl::branching::printAllStatistics();
	//}

  /// The Lipshitz constant for this application
  double Lipshitz_constant;

  /// The function object for this application
  ProblemT* func;

  ///
  enum branch_type {branch_up, branch_down, no_branch};

};


/**
 * Defines the nodes used for a serial Lipshitzian optimizer
 */
template <class ProblemT>
class serialLipshitzianNode : virtual public pebbl::branchSub
{
public:

  /// The type of the branching class
  typedef serialLipshitzian<ProblemT> branching_t;

  /// An empty constructor for a subproblem
  serialLipshitzianNode()
	: midpoint_value(Ereal<double>::negative_infinity),
	  corner_dist(0.0),
	  branch_status(branching_t::no_branch),
	  branchVariable(-2)
	{ }

  /// A virtual destructor for a subproblem
  virtual ~serialLipshitzianNode()
	{ }

  /// Return a pointer to the global branching object
  branching_t* global() const
	{ return globalPtr; }

  /// Return a pointer to the base class of the global branching object
  pebbl::branching* bGlobal() const
	{ return global(); }

  /**
   * Link the debugging in the subproblem to the debugging level
   * set within the global branching object.
   */
  REFER_DEBUG(global())

  /**
   * Initialize a subproblem using a branching object
   */
  void initialize(branching_t* master)
	{
	globalPtr = master;
	lower.resize(master->func->lower.size());
	upper.resize(master->func->lower.size());
	for (size_type i=0; i<lower.size(); i++) {
	  lower[i] = master->func->lower[i];
	  upper[i] = master->func->upper[i];
	  }
	midpoint.resize(lower.size());
	}

  /**
   * Initialize a subproblem as a child of a parent subproblem.
   * This method is not strictly necessary, but its use here illustrates
   * a flexible mechanism for managing the initialization of subproblems.
   */
  void initialize(serialLipshitzianNode<ProblemT>* parent,int whichChild)
	{
	globalPtr = parent->globalPtr;
	branchSubAsChildOf(parent);
        //
        // If we are asked for the 2nd child, switch the branch
	// status to be the "opposite" of the last branch (in the parent!)
	//
	if (whichChild > 0) {
	   if (parent->branch_status == branching_t::branch_down)
	      parent->branch_status = branching_t::branch_up;
	   else if (parent->branch_status == branching_t::branch_up)
	      parent->branch_status= branching_t::branch_down;
	   else 
	      EXCEPTION_MNGR(std::runtime_error, "Trying to create a child without a proper branching status: whichChild = " << whichChild);
	   }
	//
	// Initialize this node's info
	//
	lower = parent->lower;
	upper = parent->upper;
	midpoint = parent->midpoint;
	//
	// If the branch_status is no_branch, then this is the first
	// branch for a subproblem and the user has not defined a default
	// branching policy.  We are simply making this down_branch for now.
	//
	//
	// Compute the branch
	//
	if (parent->branch_status == branching_t::branch_down)
	   upper[parent->branchVariable] = midpoint[parent->branchVariable];
	else
	   lower[parent->branchVariable] = midpoint[parent->branchVariable];
	}

  /// Initialize this subproblem to be the root of the branching tree
  virtual void setRootComputation()
	{
	branchVariable = -1;
	}

  /**
   * Compute the lower bound on this subproblem's value.
   * The value of controlParam is not currently defined.
   *
   * When this routine is finished the state of the subproblem should be
   * set to bounded OR dead.
   */
  void boundComputation(double* /*controlParam*/)
	{
	double max_range = 0.0;
	for (unsigned int i=0; i<lower.size(); i++) {
	  midpoint[i] = (upper[i] + lower[i])/2.0;
	  if ((upper[i] - lower[i]) > max_range)
	     max_range = upper[i] - lower[i];
	  }
	//
	// Compute the distance to the corner using a numerically stable 
	// computation
	//
	corner_dist=0.0;
	for (unsigned int i=0; i<lower.size(); i++) {
	  double tmp = (upper[i] - lower[i])/(max_range);
	  corner_dist += tmp*tmp;
	  }
	corner_dist = max_range*std::sqrt(corner_dist);
	midpoint_value = (*(global()->func))(midpoint);
	bound = midpoint_value - global()->Lipshitz_constant * corner_dist;
	DEBUGPR(300,
                ucout << "boundComputation - midpoint_val = " 
                      << midpoint_value << std::endl;
                ucout << "boundComputation - midpoint = " 
                      << midpoint << std::endl;
                ucout << "boundComputation - lower = " << lower << std::endl;
                ucout << "boundComputation - upper = " << upper << std::endl;
                ucout << "boundComputation - dist     = " 
                      << corner_dist << std::endl;
                ucout << "boundComputation - L        = " 
                      << global()->Lipshitz_constant << std::endl;
                ucout << "boundComputation - bound    = " << bound << std::endl
                );
	setState(bounded);
	}

  /**
   * Determine how many children will be generated and how they will be
   * generated (e.g. the branching variable).
   * The return value is the number of children that can be generated from
   * this node.
   */
  virtual int splitComputation()
	{
	branchVariable = 0;
	double range = upper[0] - lower[0];
	for (unsigned int i=1; i<lower.size(); i++) {
	  double tmp = upper[i] - lower[i];
	  if (tmp > range) {
	     range = tmp;
	     branchVariable = i;
	     }
	  }
	DEBUGPR(300,
                ucout << "boundComputation - midpoint_val = " 
                      << midpoint_value << std::endl;
                ucout << "boundComputation - midpoint = " 
                      << midpoint << std::endl;
                ucout << "splitComputation - lower = " << lower << std::endl;
                ucout << "splitComputation - upper = " << upper << std::endl;
                ucout << "splitComputation - branchVar = " 
                      << branchVariable << std::endl;
                );
	branch_status = branching_t::branch_down;
	setState(separated);
	return 2;
	}

  /// Create a child subproblem of the current subproblem
  virtual branchSub* makeChild(int whichChild)
	{
	serialLipshitzianNode<ProblemT> *temp 
				= new serialLipshitzianNode<ProblemT>;
	temp->initialize(this, whichChild);
	return temp;
	}
  
  /// Returns true if this subproblem represents a feasible solution
  bool candidateSolution()
	{ return true; }

  /**
   * Gets a solution from a subproblem
   */
  virtual solution* extractSolution()
    {
      if (!globalPtr->canFathom(midpoint_value))
	{
	  DEBUGPR(300, ucout << "extractSolution - value = " 
		  << midpoint_value << std::endl);
	  return new arraySolution<double>(midpoint_value,midpoint,globalPtr);
	}
      return NULL;
    }

protected:

  /// A pointer to the global branching object
  branching_t* globalPtr;

  /// The midpoint of the node
  BasicArray<double> midpoint;

  /// Lower bounds on the search domain
  BasicArray<double> lower;

  /// Upper bounds on the search domain
  BasicArray<double> upper;

  /// Value of the midpoint of the current node
  Ereal<double> midpoint_value;

  /// Distance from the midpoint to a corner of the box
  double corner_dist;

  /// Flag that indicates which branch to take next to create a subproblem
  typename branching_t::branch_type branch_status;

  /**
   * The variable that will be branched next.
   * If set to -1, then this is the root computation.
   */
  int branchVariable;
};


template <class ProblemT>
pebbl::branchSub* serialLipshitzian<ProblemT>::blankSub()
{
serialLipshitzianNode<ProblemT> *temp = new serialLipshitzianNode<ProblemT>();
temp->initialize(this);
return temp;
}


} // namespace pebbl_serialLipshitzian

#endif
