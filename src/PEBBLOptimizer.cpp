#include "PEBBLOptimizer.hpp"

using namespace std;

namespace Dakota 
{
     PebbldMinimizer::PebbldMinimizer(ProblemDescDB& problem_db, Model& model): Minimizer(problem_db, model)
{
  // While this copy will be replaced in best update, initialize here
  // since relied on in Minimizer::initialize_run when a sub-iterator
  bestVariablesArray.push_back(
    iteratedModel.current_variables().copy());

  // Instantiate the approximate sub-problem minimizer
  const String& subprob_method_ptr
    = probDescDB.get_string("method.sub_method_pointer");
  const String& subprob_method_name
    = probDescDB.get_string("method.sub_method_name");

  if (!subprob_method_ptr.empty()) {
    // Approach 1: method spec support for subProbMinimizer
    const String& model_ptr = probDescDB.get_string("method.model_pointer");
    size_t method_index = probDescDB.get_db_method_node(); // for restoration
    probDescDB.set_db_method_node(subprob_method_ptr); // method only
    // sub-problem minimizer will use shallow copy of iteratedModel
    // (from problem_db.get_model())
    subProbMinimizer = probDescDB.get_iterator();//(iteratedModel);
    // suppress DB ctor default and don't output summary info
    subProbMinimizer.summary_output(false);
    // verify method's modelPointer is empty or consistent
    const String& am_model_ptr = probDescDB.get_string("method.model_pointer");
    if (!am_model_ptr.empty() && am_model_ptr != model_ptr)
      Cerr << "Warning: BandB method_pointer specification includes an\n"
	   << "         inconsistent model_pointer that will be ignored."
	   << std::endl;
    probDescDB.set_db_method_node(method_index); // restore method only
  }
  else if (!subprob_method_name.empty())
    // Approach 2: instantiate on-the-fly w/o method spec support
    subProbMinimizer
      = probDescDB.get_iterator(subprob_method_name, iteratedModel);

  branchAndBound = new PebbldBranching(model, subProbMinimizer);
}

  PebbldMinimizer::PebbldMinimizer(Model &model) : Minimizer(BRANCH_AND_BOUND, model) 
{//branchAndBound(model)
};
     PebbldMinimizer::PebbldMinimizer(Model &model, int random_seed, int max_iter, int max_eval) 
       : Minimizer(BRANCH_AND_BOUND, model)
{//branchAndBound(model,random_seed, max_iter, max_eval)
};

void PebbldMinimizer::bound_subproblem()
     {
	  InitializeTiming();
	  branchAndBound->search();
	  
	  pebbl::solution* newSolution = branchAndBound->getSolution();
	  
	  // newSolution->array[0] is the size of the data
	  // Retrieve the results...
	  std::cout << "Final Optimization Results:" << std::endl;
	  // .. and Print Them.
	  newSolution->printContents(std::cout);
	  
     }
}
