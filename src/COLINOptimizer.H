/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       COLINOptimizer
//- Description: Declaration of wrapper class for COLIN solvers
//- Owner:       Patty Hough/John Siirola/Brian Adams
//- Checked by:
//- Version: $Id: COLINOptimizer.H 6838 2010-06-24 18:52:06Z jdsiiro $

#ifndef COLIN_OPTIMIZER_H
#define COLIN_OPTIMIZER_H

#include "DakotaOptimizer.H"

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

    BMA TODO: Update the following
    The user input mappings are as follows: \c max_iterations, \c
    max_function_evaluations, \c convergence_tolerance, \c
    solution_accuracy and \c max_cpu_time are mapped into COLIN's \c
    max_iters, \c max_neval, \c ftol, \c accuracy, and \c max_time
    data attributes.  An \c output setting of \c verbose is passed to
    COLIN's set_output() function and a setting of \c debug activates
    output of method initialization and sets the COLIN \c debug
    attribute to 10000. Refer to [Hart, W.E., 2006] for additional
    information on COLIN objects and controls. */

class COLINOptimizer : public Optimizer
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// constructor
  COLINOptimizer(Model& model);
  /// alternate constructor for on-the-fly instantiations
  COLINOptimizer(Model& model, int seed);
  /// alternate constructor for Iterator instantiations by name
  COLINOptimizer(NoDBBaseConstructor, Model& model);
  /// destructor
  ~COLINOptimizer() {
    if (rng) delete rng;
  }

  //
  //- Heading: Virtual member function redefinitions
  //

  /// iterates the COLIN solver to determine the optimal solution
  void find_optimum();

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
  void solver_setup(Model &model);

  /// sets up the random number generator for stochastic methods
  void set_rng(int seed);

  /// sets construct-time options for specific methods based on user
  /// specifications, including calling method-specific set functions
  void set_solver_parameters();

  //
  //- Heading: runtime convenience member functions
  //

  /// Get the set of best points from the solver
  void get_final_points();

  /// resize bestVariablesArray
  void resize_final_points(size_t newsize);

  /// resize bestResponseArray
  void resize_final_responses(size_t newsize);

  //
  //- Heading: Data
  //

  /// COLIN solver sub-type as enumerated in COLINOptimizer.C
  short solverType;

  /// handle to the COLIN solver
  colin::SolverHandle colinSolver;

  /// handle and pointer to the COLINApplication object
  std::pair<colin::ApplicationHandle, COLINApplication*> colinProblem;

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

} // namespace Dakota

#endif
