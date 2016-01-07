#include <pebbl/branching.h>
#include "DakotaOptimizer.hpp"

namespace Dakota
{
class PebbldBranchSub;

/// Main Branching class for the PEBBL-based Minimizer
class PebbldBranching : virtual public pebbl::branching
{
  friend class PebbldBranchSub;

protected:
  /// Original model, before branching
  Model parentModel;
  /// Solver to be used at root node
  Iterator nlpSolver;
  /// Initial variable values for root node
  RealVector cont_vars;
  /// Lower bounds for root node
  RealVector lower_bounds;
  /// Upper bounds for root node
  RealVector upper_bounds;

public:
  /// Default Constructor
  PebbldBranching();
  /// Destructor
  ~PebbldBranching();

  /// Method that returns an empty Sub-Branch
  pebbl::branchSub* blankSub();
  void setModel(Model& model) {parentModel = model;};
  void setIterator(Iterator& iterator) {nlpSolver = iterator;};
};

/// Sub Branch class for the PEBBL-based Minimizer
class PebbldBranchSub : virtual public pebbl::branchSub
{
  friend class PebbldBranching;
     
protected:
  /// Pointer referring to all info passed to subproblem
  PebbldBranching* globalPtr;
  /// Model used for sub-problem
  Model subModel;
  /// Solver used for sub-problems
  Iterator subNLPSolver;
  /// Candidate solution after solving sub-problem (also the bound)
  std::vector<double> candidate_x;
  /// Objective value at the candidate solution
  double candidate_objFn;
  /// Variable to branch on 
  int splitVar;
  /// Initial variable values for sub-problem
  RealVector cont_vars;
  /// Lower bounds for sub-problem
  RealVector lower_bounds;
  /// Upper bounds for sub-problem
  RealVector upper_bounds;

public:
  /// Constructor
  PebbldBranchSub();
  /// Destructor
  ~PebbldBranchSub();

  /// Returns a reference to the corresponding main Branching object.
  PebbldBranching* global() const;
  /// Returns a reference to the corresponding main Branching object.
  pebbl::branching* bGlobal() const;
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
  void pebbldSubAsChildOf(PebbldBranchSub* parent, int splitVar, int whichChild, std::vector<double> _candidate_x, RealVector _lower_bounds, RealVector _upper_bounds);
};

}
