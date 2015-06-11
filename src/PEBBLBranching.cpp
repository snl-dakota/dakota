#include "PEBBLBranching.hpp"
#include "COLINOptimizer.hpp"

using namespace std;

namespace Dakota {
// MAIN PROBLEM CLASS
PebbldBranching::PebbldBranching()
{
     branchingInit(minimization);
}

// Default initialization
PebbldBranching::PebbldBranching(Model& _model) : parentModel(_model)
{
     branchingInit(minimization);
     random_seed = 12345;
     max_iter = 100;
     max_eval = 100;
     StringMultiArrayConstView varLabels = parentModel.all_continuous_variable_labels();
     varNames.resize(varLabels.size());
     for(int i=0;i<varNames.size();i++)
     {
	  varNames[i] = varLabels[i];
     }
     
}

PebbldBranching::PebbldBranching(Model& _model, int _random_seed, int _max_iter, int _max_eval) : parentModel(_model)
{
     branchingInit(minimization);
     // Initialization for inner optimization algorithm
     random_seed = _random_seed;
     max_iter = _max_iter/10;
     max_eval = _max_eval/10;
}

PebbldBranching::~PebbldBranching() {};

pebbl::branchSub* PebbldBranching::blankSub()
{
     // Creates a new empty Sub Branch
     PebbldBranchSub* newSub = new PebbldBranchSub(parentModel);
     newSub->setGlobalInfo(this);
     return newSub;
}

void PebbldBranching::setModel(Model& _model) 
{
  parentModel = _model; // Not sure if this is gonna work yet.
}

// BRANCH CLASS
PebbldBranchSub::PebbldBranchSub() {};

PebbldBranchSub::PebbldBranchSub(Model& _model) : subModel(_model) {};

PebbldBranchSub::~PebbldBranchSub() {};

PebbldBranching* PebbldBranchSub::global() const { return globalPtr; };
pebbl::branching* PebbldBranchSub::bGlobal() const { return global(); };

void PebbldBranchSub::setGlobalInfo(PebbldBranching* _global)
{
     globalPtr = _global;
};

void PebbldBranchSub::setRootComputation()
{
     // Sets the default values for a starting/root
     // Branch.
     eps = 0.01;
     precision = 0.0001;
     
     n_binary = globalPtr->n_binary;
     n_int = globalPtr->n_int;
     n_cont = globalPtr->n_cont;
     m_constrs = globalPtr->m_constrs;
     
     constrVals = vector<double>(globalPtr->constrVals);
     x = vector<double>(globalPtr->x);
     xl = vector<double>(globalPtr->xl);
     xu = vector<double>(globalPtr->xu);
     varNames = vector<string>(globalPtr->varNames);
     
     n = n_int + n_cont + n_binary;
}

void PebbldBranchSub::boundComputation(double* controlParam)
{
     // Calculate the bound -- Solve Problem Relaxation.
     // The Discrete Domain is relaxed into a Continuous Domain.
         
     
     string method_name = "coliny_direct";
     COLINOptimizer optimizer(method_name, subModel, random_seed, max_iter, max_eval);
     
     // Call Optimizer
     optimizer.find_optimum();
     
     Variables _variables = optimizer.variables_results();
     
     //We care about te variables ue
     Response _response = optimizer.response_results();
     
     // Considering that the problem is relaxed, all variables should be continuous.
     RealVector _finalVars = _variables.continuous_variables();
     RealVector _functions = _response.function_values();     
     
     // Load variable results into new_x
     new_x.resize(_finalVars.length());
     
     for(int i=0;i<_finalVars.length();i++)
     {
	  new_x[i] = _finalVars[i];
     }
     
     // _functions[0] is the ObjFn
     objFn = _functions[0];
     
     // When Bounding is over, call
     setState(bounded);
     // If state is not set to bounded, boundComputation will be called again
}

// In this case, a Candidate Solution is one in which the Discrete
// variables are actually Discrete after Relaxation
bool PebbldBranchSub::candidateSolution()
{
     for(int i=0;i<n_binary+n_int;i++)
     {
	  if(fmod(new_x[i],1)!=0.0)
	  {
	       return false;
	  }
     }
     return true;
};

// Simple binary splitting, that's why the method returns always 2.
int PebbldBranchSub::splitComputation()
{
     // Assuming that in the relaxed problem, the Binary/Integer
     // elements of the domain are first.
     for(int i=0;i<n_binary+n_int;i++) 
     {
	  if(fmod(new_x[i],1)!=0.0)
	  {
	       splitVar = i;
	       break;
	  }
     }
     setState(separated);
     return 2;
};

// Given a child index, return the corresponding branch.
pebbl::branchSub* PebbldBranchSub::makeChild(int whichChild) 
{
     // if whichChild is 0, it's lower bound; else, it's upper bound
     PebbldBranchSub *temp = new PebbldBranchSub(subModel);
     temp->pebbldSubAsChildOf(this, splitVar, whichChild);
     return temp;
};

void PebbldBranchSub::pebbldSubAsChildOf(PebbldBranchSub* parent, int _splitVar, int whichChild)
{
     globalPtr = parent->global();
     
     double newValue = 0.0;
     
     newValue = parent->x[_splitVar];
     
     constrVals = vector<double>(parent->constrVals);
     x = vector<double>(parent->new_x);
     xl = vector<double>(parent->xl);
     xu = vector<double>(parent->xu);
     varNames = parent->varNames;
     n_binary = parent->n_binary;
     n_int = parent->n_int;
     n_cont = parent->n_cont;
     n = parent->n;
     m_constrs = parent->m_constrs;
     bound = parent->bound;
     
     if(whichChild==0)
     {
	  double newValue2 = floor(newValue);
	  xu[_splitVar] = newValue2;
	  if(x[_splitVar]>newValue2) 
	  {
	       x[_splitVar]=newValue2;
	  }
     }
     else 
     {
	  double newValue2 = ceil(newValue);
	  xl[_splitVar] = newValue2;
	  if(x[_splitVar]<newValue2) 
	  {
	       x[_splitVar]=newValue2;
	  }
     }
     
     // Call the Model Constraints Bounds
     // Modify the Model Constraints Bounds
     // continuous_lower_bounds
     // continuous_upper_bounds
     // Call the Model Variables
     // Modify the Model Variables
     // continuous_variables
     
     RealVector cont_vars = subModel.continuous_variables();
     RealVector lower_bounds = subModel.continuous_lower_bounds();
     RealVector upper_bounds = subModel.continuous_upper_bounds();
     
     lower_bounds[_splitVar] = xl[_splitVar];
     upper_bounds[_splitVar] = xu[_splitVar];
     cont_vars = x[_splitVar];
     
     subModel.continuous_lower_bounds(lower_bounds);
     subModel.continuous_upper_bounds(upper_bounds);
     subModel.continuous_variables(cont_vars);
     
}

// arraySolution<double>(double objFnVal, std::vector<double> varValues, branching* branch-deriv-obj)
pebbl::solution* PebbldBranchSub::extractSolution()
{
     // To Consider: Creating a Custom Solution Object.
     string varDesc = "Domain";
     pebbl::BasicArray<pebbl::CharString> variableNames;
     variableNames.resize(varNames.size());
     
     for(int i = 0;i<varNames.size();i++)
     {
          pebbl::CharString tempChar(varNames[i].c_str());
	  variableNames[i] = tempChar;
     }
     
     return new pebbl::arraySolution<double> (objFn, new_x, globalPtr);
};

}
