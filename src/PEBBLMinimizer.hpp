/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaMinimizer.hpp"
#include "PEBBLBranching.hpp"

namespace Dakota
{
/// Wrapper class for experimental PebbldMinimizer.

/**
 * \brief A version of TraitsBase specialized for Pebbld mptimizers
 *
 */

class PebbldTraits: public TraitsBase
{
  public:

  /// default constructor
  PebbldTraits() { }

  /// destructor
  virtual ~PebbldTraits() { }

  /// A temporary query used in the refactor
  virtual bool is_derived() { return true; }

  /// Return the flag indicating whether method supports continuous variables
  bool supports_continuous_variables() { return true; }

  /// Return the flag indicating whether method supports discrete variables
  bool supports_discrete_variables() { return true; }

  /// Return the flag indicating whether method supports nonlinear equalities
  bool supports_nonlinear_equality() { return true; }

  /// Return the flag indicating whether method supports nonlinear inequalities
  bool supports_nonlinear_inequality() { return true; }
};


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
	 PebbldMinimizer(Model &model, int random_seed,
			 size_t max_iter, size_t max_eval);
	 /// Destructor
         ~PebbldMinimizer()
         { if (branchAndBound) delete branchAndBound; }

protected:

        /// Calls the Pebbl Branch and Bound solver
        void core_run();

        /// Redefinition of default results printing.
        void print_results(std::ostream& s, short results_state = FINAL_RESULTS);

private:
        /// Object that implements the branching and bounding logic.
        PebbldBranching* branchAndBound;

        /// Minimizer used to solve the subproblem on each branch.
        Iterator subProbMinimizer;
};

}     // namespace
