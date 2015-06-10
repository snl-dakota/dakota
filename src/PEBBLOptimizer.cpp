#include "PEBBLOptimizer.hpp"

using namespace std;

namespace Dakota 
{
     PebbldOptimizer::PebbldOptimizer(ProblemDescDB& problem_db, Model& model): branchAndBound(model) {};
     PebbldOptimizer::PebbldOptimizer(Model &model) : branchAndBound(model) {};
     PebbldOptimizer::PebbldOptimizer(Model &model, int random_seed, int max_iter, int max_eval) 
	  : branchAndBound(model,random_seed, max_iter, max_eval) {};
void PebbldOptimizer::find_optimum()
     {
	  InitializeTiming();
	  branchAndBound.search();
	  
	  pebbl::solution newSolution = branchAndBound.getSolution();
	  
	  // newSolution->array[0] is the size of the data
	  // Retrieve the results...
	  cout << "Final Optimization Results:";
	  // .. and Print Them.
	  newSolution.printContents(cout);
	  
     }
}
