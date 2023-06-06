/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
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
#include "DakotaTraitsBase.hpp"


namespace Dakota {

/// HOPSPACK-specific traits class.

/** AppsTraits specializes some traits accessors by over-riding the default 
accessors in TraitsBase. */
class AppsTraits: public TraitsBase
{
  public:

  /// default constructor
  AppsTraits();

  typedef HOPSPACK::Hopspack OptT;
  typedef HOPSPACK::Vector VecT;
  typedef HOPSPACK::Matrix MatT;

  static double noValue();

  // Allows Dakota to use a single call that gets redirected to a unique Optimizer
  static double getBestObj(const OptT &);

  static void copy_matrix_data(const RealMatrix& source, HOPSPACK::Matrix& target);

  /// destructor
  virtual ~AppsTraits();

  /// A temporary query used in the refactor
  virtual bool is_derived() { return true; }

  /// Return the flag indicating whether method supports continuous variables
  bool supports_continuous_variables() { return true; }

  /// Return the flag indicating whether method supports discrete variables
  bool supports_discrete_variables() { return true; }

  /// Return the flag indicating whether method supports linear equalities
  bool supports_linear_equality() { return true; }

  /// Return the flag indicating whether method supports linear inequalities
  bool supports_linear_inequality() { return true; }

  /// Return the flag indicating whether method supports nonlinear equalities
  bool supports_nonlinear_equality() { return true; }

  /// Return the format used for nonlinear equality constraints
  NONLINEAR_EQUALITY_FORMAT nonlinear_equality_format()
    { return NONLINEAR_EQUALITY_FORMAT::TRUE_EQUALITY; }

  /// Return the flag indicating whether method supports nonlinear inequalities
  bool supports_nonlinear_inequality() { return true; }

  /// Return the format used for nonlinear inequality constraints
  NONLINEAR_INEQUALITY_FORMAT nonlinear_inequality_format()
    { return NONLINEAR_INEQUALITY_FORMAT::ONE_SIDED_LOWER; }

};

inline AppsTraits::~AppsTraits()
{ }


inline double AppsTraits::noValue()
{ return HOPSPACK::dne(); }


inline void AppsTraits::copy_matrix_data(const RealMatrix& source, HOPSPACK::Matrix& target)
{
  HOPSPACK::Vector tmp_vector;
  for (int i=0; i<source.numRows(); ++i) {
    copy_row_vector(source, i, tmp_vector);
    target.addRow(tmp_vector);
  }
}


inline double AppsTraits::getBestObj(const OptT & optimizer)
{
  return optimizer.getBestF();
}


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
    contraction_factor, \c variable_tolerance, \c solution_target, \c
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
  APPSOptimizer():Optimizer(std::shared_ptr<TraitsBase>(new AppsTraits())) { }

  /// destructor
  ~APPSOptimizer() {
    if (evalMgr) delete evalMgr;
  }

  //
  //- Heading: Virtual function redefinitions
  //

  // Allows us to initialize constraints in a different order than the default behavior
  //    Could be replaced with better use of traits - RWH
  void initialize_run();

  /// compute the optimal solution
  void core_run();

protected:

  //
  //- Heading: Member functions
  //

  /// sets options for specific methods based on user specifications
  void set_apps_parameters();

  /// sets traits for specific TPL
  void set_apps_traits();

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
};

} // namespace Dakota
#endif
