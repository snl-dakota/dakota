/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale
    Applications Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota
    directory.
    _______________________________________________________________________ */

//- Class:       APPSOptimizer
//- Description: Wrapper class for APPSPACK
//- Owner:       Patty Hough
//- Checked by:
//- Version: $Id

#ifndef APPS_OPTIMIZER_H
#define APPS_OPTIMIZER_H

#include "DakotaOptimizer.hpp"
#include "APPSEvalMgr.hpp"

#include "HOPSPACK_ParameterList.hpp"
#include "HOPSPACK_LinConstr.hpp"
#include "HOPSPACK_Hopspack.hpp"
#include "HOPSPACK_float.hpp"


namespace Dakota {

/// Wrapper class for HOPSPACK 

/** The APPSOptimizer class provides a wrapper for HOPSPACK, a
    Sandia-developed C++ library for generalized pattern search.
    HOPSPACK defaults to a coordinate pattern search but also allows
    for augmented search patterns.  It can solve problems with bounds,
    linear constraints, and general nonlinear constraints.
    APPSOptimizer uses an APPSEvalMgr object to manage the function
    evaluations.

    The user input mappings are as follows: \c output \c
    max_function_evaluations, \c constraint_tol \c initial_delta, \c
    contraction_factor, \c threshold_delta, \c solution_target, \c
    synchronization, \c merit_function, \c constraint_penalty, and \c
    smoothing_factor are mapped into HOPS's \c "Display", "Maximum
    Evaluations", "Active Tolerance"/"Nonlinear Active Tolerance",
    "Initial Step", "Contraction Factor", "Step Tolerance", "Objective
    Target", "Synchronous Evaluations", "Penalty Function", "Penalty
    Parameter", and "Penalty Smoothing Value" data attributes.  Refer
    to the HOPS web site (https://software.sandia.gov/trac/hopspack)
    for additional information on HOPS objects and controls. */

class APPSOptimizer : public Optimizer
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// constructor
  APPSOptimizer(ProblemDescDB& problem_db, Model& model);

  /// alternate constructor for on-the-fly instantiation without ProblemDescDB
  APPSOptimizer(Model& model);

  /// alternate constructor for even more rudimentary on-the-fly instantiation
  APPSOptimizer() { }

  /// destructor
  ~APPSOptimizer() {
    if (evalMgr) delete evalMgr;
  }

  //
  //- Heading: Virtual function redefinitions
  //

  /// compute the optimal solution
  void core_run();

protected:

  //
  //- Heading: Member functions
  //

  /// sets options for specific methods based on user specifications
  void set_apps_parameters();

  /// initializes problem variables and constraints
  void initialize_variables_and_constraints();

  //
  //- Heading: Private data
  //

  /// Total across all types of variables
  int numTotalVars;

  /// Pointer to APPS parameter list
  HOPSPACK::ParameterList  params;

  /// Pointer to APPS problem parameter sublist
  HOPSPACK::ParameterList* problemParams;

  /// Pointer to APPS linear constraint parameter sublist
  HOPSPACK::ParameterList* linearParams;

  /// Pointer to APPS mediator parameter sublist
  HOPSPACK::ParameterList* mediatorParams;

  /// Pointer to APPS citizen/algorithm parameter sublist
  HOPSPACK::ParameterList* citizenParams;

  /// Pointer to the APPS evaluation manager object
  APPSEvalMgr* evalMgr;

//PDH: Don't think we would need these data members anymore.

  /// map from Dakota constraint number to APPS constraint number
  std::vector<int> constraintMapIndices;

  /// multipliers for constraint transformations
  std::vector<double> constraintMapMultipliers;

  /// offsets for constraint transformations
  std::vector<double> constraintMapOffsets;
};

// --------------------------------------------------

struct APPSOptimizerAdapter {

  typedef HOPSPACK::Hopspack OptT;
  typedef HOPSPACK::Vector VecT;
  typedef HOPSPACK::Matrix MatT;

  static double noValue();

  // Allows Dakota to use a single call that gets redirected to a unique Optimizer
  static double getBestObj(const OptT &);

  static void copy_data(const RealMatrix& source, HOPSPACK::Matrix& target);

}; // namespace APPSOptimizerAdapter


// --------------------------------------------------

inline double APPSOptimizerAdapter::noValue()
{ return HOPSPACK::dne(); }


// --------------------------------------------------

inline void APPSOptimizerAdapter::copy_data(const RealMatrix& source, HOPSPACK::Matrix& target)
{
  HOPSPACK::Vector tmp_vector;
  for (int i=0; i<source.numRows(); ++i) {
    copy_row_vector(source, i, tmp_vector);
    target.addRow(tmp_vector);
  }
}

// --------------------------------------------------

inline double APPSOptimizerAdapter::getBestObj(const OptT & optimizer)
{
  return optimizer.getBestF();
}

// --------------------------------------------------

} // namespace Dakota
#endif
