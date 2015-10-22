#include "PEBBLBranching.hpp"
#include "COLINOptimizer.hpp"

namespace Dakota {
// MAIN PROBLEM CLASS
PebbldBranching::PebbldBranching()
{
     branchingInit(minimization);
}

// Default initialization
PebbldBranching::PebbldBranching(Model& _model, Iterator& sub_prob_solver) : parentModel(_model), NLPsolver(sub_prob_solver)
{
     branchingInit(minimization);

     /// Get all the data from the model
     RealVector cont_vars = parentModel.continuous_variables();
     RealVector lower_bounds = parentModel.continuous_lower_bounds();
     RealVector upper_bounds = parentModel.continuous_upper_bounds();
     const IntVector& init_pt_int = parentModel.discrete_int_variables();
     const IntVector& lower_bnds_int = parentModel.discrete_int_lower_bounds();
     const IntVector& upper_bnds_int = parentModel.discrete_int_upper_bounds();

     const RealVector& current_vars = parentModel.all_continuous_variables();

     short model_view = parentModel.current_variables().view().second;
     const SharedVariablesData& svd = parentModel.current_variables().shared_data();
     const BitArray int_relaxed = svd.all_relaxed_discrete_int();
     const BitArray real_relaxed = svd.all_relaxed_discrete_real();

     /// Use the data obtained from the model to set the B-and-B data
     n_int = int_relaxed.size();
     n_cont = parentModel.cv() - n_int;

     x.resize(n_int+n_cont);
     xl.resize(n_int+n_cont);
     xu.resize(n_int+n_cont);
     for(int i=0;i<n_cont+n_int;i++)
     {
	  x[i] = cont_vars[i];
	  xl[i] = lower_bounds[i];
	  xu[i] = upper_bounds[i];
     }

     reset();
}

PebbldBranching::~PebbldBranching() {};

pebbl::branchSub* PebbldBranching::blankSub()
{
     // Creates a new empty Sub Branch
     PebbldBranchSub* newSub = new PebbldBranchSub(parentModel);

     n_binary = 0;

     newSub->setGlobalInfo(this);
     return newSub;
}

void PebbldBranching::setModel(Model& _model) 
{
  parentModel = _model;
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

     constrVals = std::vector<double>(globalPtr->constrVals);
     x = std::vector<double>(globalPtr->x);
     xl = std::vector<double>(globalPtr->xl);
     xu = std::vector<double>(globalPtr->xu);

     NLPsolver = globalPtr->NLPsolver;
     
     n = n_int + n_cont + n_binary;
}

void PebbldBranchSub::boundComputation(double* controlParam)
{
     // Calculate the bound -- Solve Problem Relaxation.
     // The Discrete Domain is relaxed into a Continuous Domain.

     NLPsolver.run();
     
     Variables _variables = NLPsolver.variables_results();
     
     Response _response = NLPsolver.response_results();
     
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
     for(int i=n_cont;i<n_cont+n_binary+n_int;i++)
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
  int i;

     // Assuming that in the relaxed problem, the Binary/Integer
     // elements of the domain are first.
     for(i=n_cont;i<n_cont+n_binary+n_int;i++) 
     {
	  if(fmod(new_x[i],1)!=0.0)
	  {
	       splitVar = i;
	       break;
	  }
     }

     setState(separated);
     if (splitVar>=n_cont)
       return 2;
     else
       return 0;
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

     // Set up all the data for the subproblem.     
     double newValue = 0.0;
     
     newValue = parent->x[_splitVar];
     
     constrVals = std::vector<double>(parent->constrVals);
     x = std::vector<double>(parent->new_x);
     xl = std::vector<double>(parent->xl);
     xu = std::vector<double>(parent->xu);
     n_binary = parent->n_binary;
     n_int = parent->n_int;
     n_cont = parent->n_cont;
     n = parent->n;
     m_constrs = parent->m_constrs;
     bound = parent->bound;

     NLPsolver = globalPtr->NLPsolver;
     
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
     cont_vars[_splitVar] = x[_splitVar];
     
     subModel.continuous_lower_bounds(lower_bounds);
     subModel.continuous_upper_bounds(upper_bounds);
     subModel.continuous_variables(cont_vars);
}

pebbl::solution* PebbldBranchSub::extractSolution()
{
     // To Consider: Creating a Custom Solution Object.
     std::string varDesc = "Domain";
     pebbl::BasicArray<pebbl::CharString> variableNames;
     
     return new pebbl::arraySolution<double> (objFn, new_x, globalPtr);
};

}
