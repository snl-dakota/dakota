/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       COLINOptimizer
//- Description: Declaration of wrapper class for COLIN solvers
//- Owner:       Patty Hough/John Siirola/Brian Adams
//- Checked by:
//- Version: $Id: COLINOptimizer.hpp 6838 2010-06-24 18:52:06Z jdsiiro $

#ifndef COLIN_OPTIMIZER_H
#define COLIN_OPTIMIZER_H

#include "DakotaOptimizer.hpp"

#include <colin/SolverMngr.h>
#include <colin/ApplicationMngr.h>

// forward declarations
class COLINApplication;

namespace Dakota {


/// Wrapper class for optimizers defined using COLIN 

/** The COLINOptimizer class wraps COLIN, a Sandia-developed C++
    optimization interface library.  A variety of COLIN optimizers are
    defined in COLIN and its associated libraries, including SCOLIB
    which contains the optimization components from the old COLINY
    (formerly SGOPT) library. COLIN contains optimizers such as
    genetic algorithms, pattern search methods, and other
    nongradient-based techniques. COLINOptimizer uses a
    COLINApplication object to perform the function evaluations.

    The user input mappings are as follows: \c max_iterations, \c
    max_function_evaluations, \c convergence_tolerance, and \c
    solution_accuracy are mapped into COLIN's \c max_iterations, \c
    max_function_evaluations_this_trial, \c function_value_tolerance,
    \c sufficient_objective_value properties.  An \c outputLevel is
    mapped to COLIN's \c output_level property and a setting of \c
    debug activates output of method initialization and sets the COLIN
    \c debug attribute to 10000 for the DEBUG output level. Refer to
    [Hart, W.E., 2006] for additional information on COLIN objects and
    controls. */

class COLINOptimizer : public Optimizer
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  COLINOptimizer(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor for on-the-fly instantiations
  COLINOptimizer(const String& method_name, Model& model, int seed,
		 int max_iter, int max_eval);
  /// alternate constructor for Iterator instantiations by name
  COLINOptimizer(const String& method_name, Model& model);
  /// destructor
  ~COLINOptimizer() {
    if (rng) delete rng;
    //colin::CacheFactory().unregister_cache("useThisCache");
    //String unique_cache_name(method_id());
    //unique_cache_name += methodName;
    //colin::CacheFactory().unregister_cache(unique_cache_name);
  }

  //
  //- Heading: Virtual member function redefinitions
  //

  /// clears internal optimizer state
  void reset();

  /// iterates the COLIN solver to determine the optimal solution
  void core_run();

  // COLIN methods cannot yet accept multiple initial points
  //bool accepts_multiple_points() const;

  /// some COLIN methods can return multiple points
  bool returns_multiple_points() const;

protected:

  //
  //- Heading: constructor convenience member functions
  //
  
  /// convenience function for setting up the particular COLIN solver
  /// and appropriate Application
  void solver_setup(unsigned short method_name);

  /// sets up the random number generator for stochastic methods
  void set_rng(int seed);

  /// sets construct-time options for specific methods based on user
  /// specifications, including calling method-specific set functions
  void set_solver_parameters();

  //
  //- Heading: runtime convenience member functions
  //

  /// Get the final set of points from the solver
  /// Look up responses and sort, first according
  /// to constraint violation, then according to
  /// function value
  void post_run(std::ostream& s);

  /// Retrieve response from Colin AppResponse, return pair indicating
  /// success for <objective, constraints>
  std::pair<bool, bool> 
  colin_cache_lookup(const colin::AppResponse& colinResponse,
		     Response& tmpResponseHolder);
  
  /// Compute constraint violation, based on nonlinear constraints in
  /// iteratedModel and provided Response data
  double constraint_violation(const Response& tmpResponseHolder);

  //
  //- Heading: Data
  //

  /// COLIN solver sub-type as enumerated in COLINOptimizer.cpp
  short solverType;

  /// handle to the COLIN solver
  colin::SolverHandle colinSolver;

  /// handle and pointer to the COLINApplication object
  std::pair<colin::ApplicationHandle, COLINApplication*> colinProblem;

  /// pointer to the COLIN evalutaion manager object
  colin::EvaluationManager_Base* colinEvalMgr;

  /// random number generator pointer
  utilib::RNG* rng;
  
  /// the \c synchronization setting: true if \c blocking, false if
  /// \c nonblocking
  bool blockingSynch;

  /// Buffer to hold problem constraint_penalty parameter
  Real constraint_penalty;

  /// Buffer to hold problem constant_penalty parameter
  bool constant_penalty;
};


inline void COLINOptimizer::reset()
{ colinSolver->reset(); }

} // namespace Dakota

#endif
