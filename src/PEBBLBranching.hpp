#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdlib.h> 
#include <cmath>
#include <iomanip>
#include <pebbl/branching.h>
#include <vector>
#include "DakotaOptimizer.hpp"

#ifdef ACRO_HAVE_MPI
#include <utilib/PackBuf.h>
#endif

namespace Dakota
{
/// Main Branching class for the experimental Pebbl-based Optimizer
class PebbldBranching : virtual public pebbl::branching
{
friend class PebbldBranchSub;

protected:
	/// Number of binary variables
	int n_binary;
	/// Number of discrete variables
	int n_int;
	/// Number of continuous variables
	int n_cont;
	/// Number of constraints
	int m_constrs;
	/// Vector of x values
        std::vector<double> x;
	/// Vector of lower bounds
        std::vector<double> xl;
	/// Vector of upper bounds
        std::vector<double> xu;
	/// Vector of constraint values
        std::vector<double> constrVals;
	/// Original model, before branching
	Model parentModel;
	/// Nonlinear optimizer to solve subproblems
        Iterator NLPsolver;

public:
	/// Default Constructor
	PebbldBranching();
	/// Constructor
	/** PebbldOptimizer Constructor
	@param model DAKOTA Model object
	*/
	PebbldBranching(Model& _model, Iterator& sub_prob_solver);
	/// Destructor
	~PebbldBranching();
	/// Method that returns an empty Sub-Branch
        pebbl::branchSub* blankSub();
	void setModel(Model& _model);
};

/// Sub Branch
/// class for the experimental Pebbl-based Optimizer
class PebbldBranchSub : virtual public pebbl::branchSub
{
friend class PebbldBranching;
     
protected:
	/// Pointer referring to all info passed to subproblem
	PebbldBranching* globalPtr;

	/// Number of binary variables
        int n_binary;
	/// Number of discrete variables
        int n_int;
	/// Number of continuous variables
        int n_cont;
	/// Total number of variables
        int n;
	/// Number of constraints
        int m_constrs;

	/// Vector of variable values
        std::vector<double> x;
	/// Vector of variable values modified for subproblem
        std::vector<double> new_x;
	/// Vector of lower bounds
        std::vector<double> xl;
	/// Vector of upper bounds
        std::vector<double> xu;

	/// Objective function values
	double objFn;
	/// Vector of constraint values
        std::vector<double> constrVals;

	/// Model on which subproblem is solved
	Model subModel;
	/// Nonlinear optimization method to solve subproblem
        Iterator NLPsolver;
	  
	// Computation parameters?.
	double eps;
	double precision;
	/// Variable to branch on 
	int splitVar;

public:
	/// Returns a reference to the corresponding main Branching object.
	PebbldBranching* global() const;
	/// Returns a reference to the corresponding main Branching object.
        pebbl::branching* bGlobal() const;
	PebbldBranchSub();
	/// Constructor
	/** PebbldBranchSub Constructor
	    @param model DAKOTA Model object
	  */
	PebbldBranchSub(Model& _model);
	/// Destructor
	~PebbldBranchSub();

	/// Method that sets up the main Branching object.
	void setGlobalInfo(PebbldBranching* global_);
	/// Method that is called when declaring the current node as a root node.
	void setRootComputation();
	/// Method that does the Bounding Operation.
	/// In other words, it calls the optimization algorithm on the
	/// relaxed domain
	void boundComputation(double* controlParam);
	/// Method called after the bounding operation, returns true if 
	/// the bounding resulted in a possible solution to the original
	/// non-relaxed problem.
	bool candidateSolution();
	/// Method that returns a PEBBL-based solution. 
	pebbl::solution* extractSolution();
	/// Method that determines how many branches are created and how.
	/// Returns the number of branches created from this object.
	int splitComputation();
	/// Method that returns a new PebbldBranchSub object based on 
	/// Objective Function improvements and the number of branches.
        pebbl::branchSub* makeChild(int whichChild);
	/// Method that creates a new PebbldBranching object.
	void pebbldSubAsChildOf(PebbldBranchSub* parent, int splitVar, int whichChild);
};

}     // namespace
