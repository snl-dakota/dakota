#include "PEBBLBranching.hpp"

namespace Dakota {
// MAIN PROBLEM CLASS
PebbldBranching::PebbldBranching()
{
  // Reset has to be called here.  Otherwise, something doesn't get
  // initialized appropriately in PEBBL and results in a crash.
  branchingInit(minimization);
  pebbl::branching::reset();
}

PebbldBranching::~PebbldBranching()
{}

// Creates a new empty Sub Branch, as specified in PEBBLE
// documentation
pebbl::branchSub* PebbldBranching::blankSub()
{
  PebbldBranchSub* newDakotaSub = new PebbldBranchSub();
  newDakotaSub->setGlobalInfo(this);
  return newDakotaSub;
}

// BRANCH CLASS
PebbldBranchSub::PebbldBranchSub()
{}

PebbldBranchSub::~PebbldBranchSub()
{}

// Enable access to the sub-problem global data
PebbldBranching* PebbldBranchSub::global() const
{
  return globalPtr;
}

pebbl::branching* PebbldBranchSub::bGlobal() const
{
  return global();
}

// Set pointer to the sub-problem global data
void PebbldBranchSub::setGlobalInfo(PebbldBranching* _global)
{
  globalPtr = _global;
}

// Set up the problem at the root node
void PebbldBranchSub::setRootComputation()
{
  // Use this model and solver
  subModel = globalPtr->parentModel;
  subNLPSolver = globalPtr->nlpSolver;

  // The only model-related data that differs across sub-problems are
  // the initial variable values and the bounds.  This needs to be a
  // deep copy.  While it may seem unnecessary at the root node, there
  // is code shared with the sub-problems that requires it to be here
  // for consistency, i.e., so there are not seg faults due to these
  // not existing.
  cont_vars.resize(subModel.continuous_variables().length());
  lower_bounds.resize(subModel.continuous_lower_bounds().length());
  upper_bounds.resize(subModel.continuous_upper_bounds().length());
  for (int i=0; i<subModel.continuous_variables().length(); i++)
    cont_vars[i] = subModel.continuous_variables()[i];
  for (int i=0; i<subModel.continuous_lower_bounds().length(); i++)
    lower_bounds[i] = subModel.continuous_lower_bounds()[i];
  for (int i=0; i<subModel.continuous_upper_bounds().length(); i++)
    upper_bounds[i] = subModel.continuous_upper_bounds()[i];
}

void PebbldBranchSub::boundComputation(double* controlParam)
{
  // Calculate the bound -- Solve Problem Relaxation.
  // The Discrete Domain is relaxed into a Continuous Domain.

  // Reset the model variable values and bounds for the sub-problem.
  subModel.continuous_variables(cont_vars);
  subModel.continuous_lower_bounds(lower_bounds);
  subModel.continuous_upper_bounds(upper_bounds);

  // Run the solver.
  subNLPSolver.run();

  Variables _variables = subNLPSolver.variables_results();
  Response _response = subNLPSolver.response_results();
     
  // Considering that the problem is relaxed, all variables should be continuous.
  RealVector _finalVars = _variables.continuous_variables();
  RealVector _functions = _response.function_values();     
   
  // Load variable results into new_x
  candidate_x.resize(_finalVars.length());
  for(int i=0;i<_finalVars.length();i++)
    candidate_x[i] = _finalVars[i];

  // _functions[0] is the ObjFn
  candidate_objFn = _functions[0];
  bound = candidate_objFn;
   
  // When Bounding is over, call
  // If state is not set to bounded, boundComputation will be called again
  setState(bounded);
}

// In this case, a Candidate Solution is one in which the Discrete
// variables are actually Discrete after Relaxation
bool PebbldBranchSub::candidateSolution()
{
  const SharedVariablesData& svd = subModel.current_variables().shared_data();
  const BitArray int_relaxed = svd.all_relaxed_discrete_int();
  int num_int_vars = int_relaxed.size();
  int num_cont_vars = subModel.cv() - num_int_vars;

  for(int i=num_cont_vars;i<num_cont_vars+num_int_vars;i++)
  {
    if(fmod(candidate_x[i],1)!=0.0)
      return false;
   }
  return true;
}

pebbl::solution* PebbldBranchSub::extractSolution()
{
  // To Consider: Creating a Custom Solution Object.
  return new pebbl::arraySolution<double> (candidate_objFn, candidate_x, globalPtr);
}

// Simple binary splitting, that's why the method returns always 2.
int PebbldBranchSub::splitComputation()
{
  const SharedVariablesData& svd = subModel.current_variables().shared_data();
  const BitArray int_relaxed = svd.all_relaxed_discrete_int();
  int num_int_vars = int_relaxed.size();
  int num_cont_vars = subModel.cv() - num_int_vars;

  // Assuming that in the relaxed problem, the Binary/Integer
  // elements of the domain are first.
  for(int i=num_cont_vars;i<num_cont_vars+num_int_vars;i++) 
  {
    if(fmod(candidate_x[i],1)!=0.0)
    {
      splitVar = i;
      break;
    }
  }

  if (splitVar>=num_cont_vars)
  {
    setState(separated);
    return 2;
  }
  else
  {
    setState(dead);
    return 0;
  }
}

// Given a child index, return the corresponding branch.
pebbl::branchSub* PebbldBranchSub::makeChild(int whichChild) 
{
  // if whichChild is 0, it's lower bound; else, it's upper bound
  PebbldBranchSub *temp = new PebbldBranchSub();
  // Child won't have access to the current variable values and
  // bounds, so pass them through.
  temp->pebbldSubAsChildOf(this, splitVar, whichChild, candidate_x, lower_bounds, upper_bounds);
  return temp;
}

// Set up all the data for the subproblem.     
void PebbldBranchSub::pebbldSubAsChildOf(PebbldBranchSub* parent, int _splitVar, int whichChild, std::vector<double> _candidate_x, RealVector _lower_bounds, RealVector _upper_bounds)
{
  // Use this model and solver. Note that the model is the same as
  // that used at the parent node and for all other sub-problems.
  // Since the only things that change are initial variable values and
  // bounds, re-use the model and just reset that data before running
  // the solver.  There is some chance we will have to re-consider
  // this in the parallel version...not sure if we'll have
  // accesses/updates to the model over the course of the optimization
  // stomping on each other.
  globalPtr = parent->global();
  subModel = parent->global()->parentModel;
  subNLPSolver = parent->global()->nlpSolver;

  // The only model-related data that differs across sub-problems are
  // the initial variable values and the bounds.  This needs to be a
  // deep copy.
  cont_vars.resize(subModel.continuous_variables().length());
  lower_bounds.resize(subModel.continuous_lower_bounds().length());
  upper_bounds.resize(subModel.continuous_upper_bounds().length());
  for (int i=0; i<subModel.continuous_variables().length(); i++)
    cont_vars[i] = _candidate_x[i];
  for (int i=0; i<subModel.continuous_lower_bounds().length(); i++)
    lower_bounds[i] = _lower_bounds[i];
  for (int i=0; i<subModel.continuous_upper_bounds().length(); i++)
    upper_bounds[i] = _upper_bounds[i];

  // Reset the bounds for this sub-problem.  Also move the initial
  // variable value to feasible if necessary.
  if(whichChild==0)
  {
    upper_bounds[_splitVar] = floor(cont_vars[_splitVar]);
    if(cont_vars[_splitVar]>upper_bounds[_splitVar]) 
      cont_vars[_splitVar] = upper_bounds[_splitVar];
  }
  else 
  {
    lower_bounds[_splitVar] = ceil(cont_vars[_splitVar]);
    if(cont_vars[_splitVar]<lower_bounds[_splitVar])
      cont_vars[_splitVar] = lower_bounds[_splitVar];
  }
}

}
