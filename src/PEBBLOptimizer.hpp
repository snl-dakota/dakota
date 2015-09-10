#include "DakotaMinimizer.hpp"
#include "PEBBLBranching.hpp"

using namespace std;

namespace Dakota
{
     /// Wrapper class for experimental PebbldMinimizer.
     class PebbldMinimizer : public Minimizer
     {
	  public:
               /// standard constructor
               PebbldMinimizer(ProblemDescDB& problem_db, Model& model);
	       /// Constructor
	       /** PebbldMinimizer Constructor
	       @param model DAKOTA Model object
	       */
	       PebbldMinimizer(Model &model);
	       /// Constructor
	       /** PebbldMinimizer Constructor
	       @param model DAKOTA Model object
	       @param random_seed Random Seed
	       @param max_iter Maximum number of iterations
	       @param max_eval Maximum number of Black Box Evaluations
	       */
	       PebbldMinimizer(Model &model, int random_seed, int max_iter, int max_eval);
	       /// Destructor
               ~PebbldMinimizer()
       {
	 if (branchAndBound) delete branchAndBound;
       }
	       /// Calls the Pebbl Branch and Bound solver
	       void bound_subproblem();
	  private:
	       PebbldBranching* branchAndBound;
       void core_run();
       /// the minimizer used to solve the subproblem on each branch.
       Iterator subProbMinimizer;
     };

inline void PebbldMinimizer::core_run()
{ bound_subproblem(); }

}

