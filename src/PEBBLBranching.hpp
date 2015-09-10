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

// TODO references

class lockwoodSub;

namespace Dakota
{
/// Main Branching class for the experimental Pebbl-based Optimizer
class PebbldBranching : virtual public pebbl::branching
{
     friend class PebbldBranchSub;
     protected:
          std::vector<double> constrVals;
          std::vector<double> x;
          std::vector<double> xl;
          std::vector<double> xu;
          std::vector<std::string> varNames;
	  int n_binary;
	  int n_int;
	  int n_cont;
	  int m_constrs;
	  Model parentModel;
          Iterator NLPsolver;
	  int random_seed;
	  int max_iter;
	  int max_eval;
     public:
	  PebbldBranching();
	  /// Constructor
	  /** PebbldOptimizer Constructor
	  @param model DAKOTA Model object
	  */
  PebbldBranching(Model& _model, Iterator& sub_prob_solver);
  PebbldBranching(Model& _model, Iterator& sub_prob_solver, int _random_seed, int _max_iter, int _max_eval);
	  /// Destructor
	  ~PebbldBranching();
	  /// Method that returns an empty Sub-Branch
          pebbl::branchSub* blankSub();
	  void setModel(Model& _model);
};

/// Sub Branch
/// class for the experimental Pebbl-based Optimizer
  class PebbldBranchSub : virtual public pebbl::branchSub {
     friend class PebbldBranching;
     
     protected:
	  PebbldBranching* globalPtr;

	  double objFn;
          std::vector<double> constrVals;
          std::vector<double> x;
          std::vector<double> new_x;
          std::vector<double> xl;
          std::vector<double> xu;
          std::vector<std::string> varNames;
	  int n_binary;
	  int n_int;
	  int n_cont;
	  int n;
	  int m_constrs;
	  int random_seed;
	  int max_iter;
	  int max_eval;
	  
	  Model subModel;
          Iterator NLPsolver;
	  
	  // Computation parameters?.
	  double eps;
	  double precision;
	  
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
	  /** Method that sets up the main Branching object.
	  */
	  void setGlobalInfo(PebbldBranching* global_);
	  /** Method that is called when declaring the current node as a root node.
	  */
	  void setRootComputation();
	  /** Method that does the Bounding Operation.
	  In other words, it calls the optimization algorithm on the relaxed domain.
	  */
	  void boundComputation(double* controlParam);
	  /** Method called after the bounding operation, returns true if 
	  the bounding resulted in a possible solution to the original non-relaxed problem.
	  */
	  bool candidateSolution();
	  /** Method that returns a PEBBL-based solution. 
	  */
          pebbl::solution* extractSolution();
	  /** Method that determines how many branches are created and how.
	  Returns the number of branches created from this object.
	  */
	  int splitComputation();
	  /** Method that returns a new PebbldBranchSub object based on 
	  Objective Function improvements and the number of branches.
	  */
          pebbl::branchSub* makeChild(int whichChild);
	  /** Method that creates a new PebbldBranching object.
	  */
	  void pebbldSubAsChildOf(PebbldBranchSub* parent, int splitVar, int whichChild);
};

}
