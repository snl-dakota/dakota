#include "DakotaOptimizer.hpp"
#include "PEBBLBranching.hpp"

using namespace std;

namespace Dakota
{
     /// Wrapper class for experimental PebbldOptimizer.
     class PebbldOptimizer : public Optimizer
     {
	  public:
               /// standard constructor
               PebbldOptimizer(ProblemDescDB& problem_db, Model& model);
	       /// Constructor
	       /** PebbldOptimizer Constructor
	       @param model DAKOTA Model object
	       */
	       PebbldOptimizer(Model &model);
	       /// Constructor
	       /** PebbldOptimizer Constructor
	       @param model DAKOTA Model object
	       @param random_seed Random Seed
	       @param max_iter Maximum number of iterations
	       @param max_eval Maximum number of Black Box Evaluations
	       */
	       PebbldOptimizer(Model &model, int random_seed, int max_iter, int max_eval);
	       /// Destructor
               ~PebbldOptimizer() {};
	       /// Calls the Pebbl Branch and Bound solver
	       void find_optimum();
	  private:
	       PebbldBranching branchAndBound;
     };

}

