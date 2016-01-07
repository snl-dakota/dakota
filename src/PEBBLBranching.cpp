#include "PEBBLBranching.hpp"

namespace Dakota {
// MAIN PROBLEM CLASS
PebbldBranching::PebbldBranching()
{
  branchingInit(minimization);
  pebbl::branching::reset();
}

PebbldBranching::~PebbldBranching()
{}

// Creates a new empty Sub Branch
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

PebbldBranching* PebbldBranchSub::global() const
{
  return globalPtr;
}

pebbl::branching* PebbldBranchSub::bGlobal() const
{
  return global();
}


void PebbldBranchSub::setGlobalInfo(PebbldBranching* _global)
{
  globalPtr = _global;
}

void PebbldBranchSub::setRootComputation()
{
  childNum = 0;
  subModel = globalPtr->parentModel;
  subNLPSolver = globalPtr->nlpSolver;
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

  subModel.continuous_variables(cont_vars);
  subModel.continuous_lower_bounds(lower_bounds);
  subModel.continuous_upper_bounds(upper_bounds);

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

  for (int i=0; i<subModel.continuous_variables().length(); i++)
    cont_vars[i] = subModel.continuous_variables()[i];
  for (int i=0; i<subModel.continuous_lower_bounds().length(); i++)
    lower_bounds[i] = subModel.continuous_lower_bounds()[i];
  for (int i=0; i<subModel.continuous_upper_bounds().length(); i++)
    upper_bounds[i] = subModel.continuous_upper_bounds()[i];
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
  temp->pebbldSubAsChildOf(this, splitVar, whichChild, candidate_x, lower_bounds, upper_bounds);
  return temp;
}

// Set up all the data for the subproblem.     
void PebbldBranchSub::pebbldSubAsChildOf(PebbldBranchSub* parent, int _splitVar, int whichChild, std::vector<double> _candidate_x, RealVector _lower_bounds, RealVector _upper_bounds)
{
  globalPtr = parent->global();
  childNum = whichChild;
  subModel = parent->global()->parentModel;
  subNLPSolver = parent->global()->nlpSolver;

  cont_vars.resize(subModel.continuous_variables().length());
  lower_bounds.resize(subModel.continuous_lower_bounds().length());
  upper_bounds.resize(subModel.continuous_upper_bounds().length());
  for (int i=0; i<subModel.continuous_variables().length(); i++)
    cont_vars[i] = _candidate_x[i];
  for (int i=0; i<subModel.continuous_lower_bounds().length(); i++)
    lower_bounds[i] = _lower_bounds[i];
  for (int i=0; i<subModel.continuous_upper_bounds().length(); i++)
    upper_bounds[i] = _upper_bounds[i];

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
