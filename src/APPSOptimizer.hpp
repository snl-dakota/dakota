/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale
    Applications Copyright (c) 2010, Sandia National Laboratories.
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

namespace Dakota {

/// Wrapper class for APPSPACK 

/** The APPSOptimizer class provides a wrapper for APPSPACK, a
    Sandia-developed C++ library for generalized pattern search.
    APPSPACK defaults to a coordinate pattern search but also allows
    for augmented search patterns.  It can solve problems with bounds,
    linear constraints, and general nonlinear constraints.
    APPSOptimizer uses an APPSEvalMgr object to manage the function
    evaluations.

    The user input mappings are as follows: \c output \c
    max_function_evaluations, \c constraint_tol \c initial_delta, \c
    contraction_factor, \c threshold_delta, \c solution_target, \c
    synchronization, \c merit_function, \c constraint_penalty, and \c
    smoothing_factor are mapped into APPS's \c "Debug", "Maximum
    Evaluations", "Bounds Tolerance"/"Machine Epsilon"/"Constraint
    Tolerance", "Initial Step", "Contraction Factor", "Step
    Tolerance", "Function Tolerance", "Synchronous", "Method",
    "Initial Penalty Value", and "Initial Smoothing Value" data
    attributes.  Refer to the APPS web site
    (http://software.sandia.gov/appspack) for additional information
    on APPS objects and controls. */

class APPSOptimizer : public Optimizer
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// constructor
  APPSOptimizer(Model& model);

  /// alternate constructor for on-the-fly instantiations
  APPSOptimizer(NoDBBaseConstructor, Model& model);

  /// destructor
  ~APPSOptimizer() {
    if (evalMgr) delete evalMgr;
  }

  /// Performs the iterations to determine the optimal solution.
  void find_optimum();

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

  /// map from Dakota constraint number to APPS constraint number
  std::vector<int> constraintMapIndices;

  /// multipliers for constraint transformations
  std::vector<double> constraintMapMultipliers;

  /// offsets for constraint transformations
  std::vector<double> constraintMapOffsets;
};

} // namespace Dakota

#endif
