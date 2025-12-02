/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifdef HAVE_DAKOTA_ROL_INTERFACE

#include "DakotaROLOptimizer.hpp"
#include "DakotaROLInterface.hpp"
#include "ProblemDescDB.hpp"
#include "model_utils.hpp"

// ROL headers
#include "ROL_Bounds.hpp"
#include "ROL_Problem.hpp"
#include "ROL_Solver.hpp"

// Teuchos headers
#include "Teuchos_XMLParameterListHelpers.hpp"
#include "Teuchos_StandardCatchMacros.hpp"

#include <filesystem>

namespace Dakota {

// Enums for problem type
enum {TYPE_U=1, TYPE_B=2, TYPE_E=3, TYPE_EB=4};

// A compile unit (this source file) variable used to toggle tabular data
// output when dealing with sequential duplicates - ROL specific
namespace {
  bool orig_auto_graphics_flag = false;
}

// -----------------------------------------------------------------
// Implementation class definition (pIMPL idiom)
// -----------------------------------------------------------------

namespace rol_optimizer_impl {

class ROLOptimizerImpl {
public:
  /// Parameters for the ROL::Solver
  Teuchos::ParameterList solverParams;

  /// ROL problem type
  unsigned short problemType;

  /// Handle to ROL's solution vector
  ROL::Ptr<ROL::Vector<Real>> rolX;

  /// Handle to ROL's lower bounds vector
  ROL::Ptr<ROL::Vector<Real>> lowerBounds;

  /// Handle to ROL's upper bounds vector
  ROL::Ptr<ROL::Vector<Real>> upperBounds;

  /// ROL Problem
  ROL::Ptr<ROL::Problem<Real>> rolProblem;

  /// ROL Solver
  ROL::Ptr<ROL::Solver<Real>> rolSolver;

  /// Constructor
  ROLOptimizerImpl()
    : solverParams("Dakota::ROL"),
      problemType(TYPE_B),
      rolX(ROL::nullPtr),
      lowerBounds(ROL::nullPtr),
      upperBounds(ROL::nullPtr),
      rolProblem(ROL::nullPtr),
      rolSolver(ROL::nullPtr)
  {}
};

} // namespace rol_optimizer_impl


// -----------------------------------------------------------------
// ROLOptimizer implementation
// -----------------------------------------------------------------


// Standard constructor for ROLOptimizer. Sets up ROL solver based on
// information from the problem database.
ROLOptimizer::ROLOptimizer(ProblemDescDB& problem_db,
                           ParallelLibrary& parallel_lib,
                           std::shared_ptr<Model> model)
  : Optimizer(problem_db, parallel_lib, model,
              std::shared_ptr<TraitsBase>(new ROLTraits())),
    pimpl_(std::make_unique<rol_optimizer_impl::ROLOptimizerImpl>())
{
  // Populate ROL data with user-provided problem dimensions and
  // initial values, and set ROL solver parameters. These calls are
  // order-dependent in that the solver settings depend on
  // problemType, which is set with the problem data.

  set_problem();
  set_rol_parameters();
}


// Alternate constructor for Iterator instantiations by name. Sets up
// ROL solver based on information passed as arguments.
ROLOptimizer::ROLOptimizer(const String& method_string,
                           std::shared_ptr<Model> model)
  : Optimizer(method_string_to_enum(method_string), model,
              std::shared_ptr<TraitsBase>(new ROLTraits())),
    pimpl_(std::make_unique<rol_optimizer_impl::ROLOptimizerImpl>())
{
  // Populate ROL data with user-provided problem dimensions and
  // initial values, and set ROL solver parameters. These calls are
  // order-dependent in that the solver settings depend on
  // problemType, which is set with the problem data.

  set_problem();
  set_rol_parameters();
}


// Destructor
ROLOptimizer::~ROLOptimizer() = default;


// Initialize_run redefines the Optimizer virtual function to perform
// initialization specific to ROL.
void ROLOptimizer::initialize_run()
{
  Optimizer::initialize_run();

  // Needed to make ROL output to tabular data consistent with other Opt TPLs
  // NOTE: This cannot be set in the constructor (or helper functions associated
  //       with the constructor) because some variables get assigned by base
  //       class constructors and are not available until all have completed.
  orig_auto_graphics_flag = iteratedModel->auto_graphics();
}


// core_run redefines the Optimizer virtual function to perform the
// optimization using ROL and catalogue the results.
void ROLOptimizer::core_run()
{
  using namespace rol_interface;

  // Rebuild the problem in case it needs to be updated
  set_problem();

  // Create output stream filter to prefix ROL output with "ROL: "
  OutputStreamFilter rolOutputStream(Cout);

  // Solve the optimization problem
  pimpl_->rolSolver->solve(rolOutputStream.stream());

  // Copy ROL solution to Dakota bestVariablesArray
  Variables& best_vars = bestVariablesArray.front();
  RealVector& cont_vars = best_vars.continuous_variables_view();
  const auto& x_dakota = as_dakota_vector(*pimpl_->rolX);
  copy_data(x_dakota, cont_vars);

  // ROL does not currently provide access to the final solution, so
  // attempt a model database lookup directly into best.
  if (!localObjectiveRecast)
  {
    Response& best_resp = bestResponseArray.front();
    ActiveSet search_set(best_resp.active_set());
    search_set.request_values(1); // Function values only
    best_resp.active_set(search_set);
    bool db_found = iteratedModel->db_lookup(best_vars, search_set, best_resp);

    // Fall back on re-evaluation if not found.
    if (db_found)
      Cout << "INFO: ROL retrieved best response from cache." << std::endl;
    else {
      Cout << "INFO: ROL re-evaluating model to retrieve best response."
           << std::endl;

      // Evaluate model for responses at best parameters and set Dakota
      // bestResponseArray.
      ModelUtils::continuous_variables(*iteratedModel, cont_vars);
      iteratedModel->evaluate();
      const RealVector& best_fns =
        iteratedModel->current_response().function_values();
      best_resp.function_values(best_fns);
    }
  }
}


// Helper function to populate ROL data with user-provided problem
// dimensions and initial values.
void ROLOptimizer::set_problem()
{
  using namespace rol_interface;

  size_t num_cv = numContinuousVars;
  size_t num_eq_const = numLinearEqConstraints + numNonlinearEqConstraints;
  size_t num_ineq_const = numLinearIneqConstraints + numNonlinearIneqConstraints;

  // Set the ROL problem type. Default is TYPE_B (bound constrained),
  // so overwrite for other constraint scenarios.
  if ((num_ineq_const > 0) ||
      ((num_eq_const > 0) && (boundConstraintFlag)))
    pimpl_->problemType = TYPE_EB;
  else {
    if (!boundConstraintFlag)
      pimpl_->problemType = (num_eq_const > 0) ? TYPE_E : TYPE_U;
  }

  // Create initial guess vector
  pimpl_->rolX = make_vector(num_cv, true);
  auto& x_dakota = as_dakota_vector(*pimpl_->rolX);
  get_initial_values(*iteratedModel, x_dakota);

  // Create objective and ROL Problem with initial guess
  auto obj = Objective::createFromModel(*iteratedModel);
  pimpl_->rolProblem = ROL::makePtr<ROL::Problem<Real>>(obj, pimpl_->rolX);

  // Set variable bounds if needed
  if ((pimpl_->problemType == TYPE_B) || (pimpl_->problemType == TYPE_EB)) {
    Real rol_inf = ROL::ROL_INF<Real>();
    Real rol_ninf = ROL::ROL_NINF<Real>();

    // Create and populate bounds vectors
    pimpl_->lowerBounds = make_vector(num_cv, false);
    pimpl_->upperBounds = make_vector(num_cv, false);

    auto& l_dakota = as_dakota_vector(*pimpl_->lowerBounds);
    auto& u_dakota = as_dakota_vector(*pimpl_->upperBounds);

    get_bounds(*iteratedModel, l_dakota, u_dakota);

    // Set bounds greater (less) than ROL_INF (ROL_NINF) to ROL_INF (ROL_NINF)
    for (size_t i = 0; i < num_cv; i++) {
      if (l_dakota[i] < rol_ninf)
        l_dakota[i] = rol_ninf;
      if (u_dakota[i] > rol_inf)
        u_dakota[i] = rol_inf;
    }

    // Set bounds in ROL problem
    auto bnd = ROL::makePtr<ROL::Bounds<Real>>(pimpl_->lowerBounds,
                                               pimpl_->upperBounds);
    pimpl_->rolProblem->addBoundConstraint(bnd);
  }

  // Add constraints using the new interface
  auto constraints = Constraint::createSetFromModel(*iteratedModel);

  // Add linear equality constraints
  if (constraints.linearEquality != ROL::nullPtr) {
    size_t num_lin_eq = ModelUtils::num_linear_eq_constraints(*iteratedModel);
    auto emul = make_vector(num_lin_eq, true);
    pimpl_->rolProblem->addLinearConstraint("Linear Equality",
                                            constraints.linearEquality,
                                            emul);
  }

  // Add linear inequality constraints with bounds
  if (constraints.linearInequality != ROL::nullPtr) {
    size_t num_lin_ineq = ModelUtils::num_linear_ineq_constraints(*iteratedModel);
    auto imul = make_vector(num_lin_ineq, true);

    // Get constraint bounds
    auto li_lower = make_vector(num_lin_ineq, false);
    auto li_upper = make_vector(num_lin_ineq, false);
    auto& li_l_dakota = as_dakota_vector(*li_lower);
    auto& li_u_dakota = as_dakota_vector(*li_upper);

    copy_data(ModelUtils::linear_ineq_constraint_lower_bounds(*iteratedModel),
              li_l_dakota);
    copy_data(ModelUtils::linear_ineq_constraint_upper_bounds(*iteratedModel),
              li_u_dakota);

    // Clip to ROL bounds
    Real rol_inf = ROL::ROL_INF<Real>();
    Real rol_ninf = ROL::ROL_NINF<Real>();
    for (size_t i = 0; i < num_lin_ineq; i++) {
      if (li_l_dakota[i] < rol_ninf) li_l_dakota[i] = rol_ninf;
      if (li_u_dakota[i] > rol_inf)  li_u_dakota[i] = rol_inf;
    }

    auto li_bnd = ROL::makePtr<ROL::Bounds<Real>>(li_lower, li_upper);
    ROL::Ptr<ROL::Vector<Real>> li_res = ROL::nullPtr;
    pimpl_->rolProblem->addLinearConstraint("Linear Inequality",
                                            constraints.linearInequality,
                                            imul,
                                            li_bnd,
                                            li_res);
  }

  // Add nonlinear equality constraints
  if (constraints.nonlinearEquality != ROL::nullPtr) {
    size_t num_nln_eq = ModelUtils::num_nonlinear_eq_constraints(*iteratedModel);
    auto emul = make_vector(num_nln_eq, true);
    pimpl_->rolProblem->addConstraint("Nonlinear Equality",
                                      constraints.nonlinearEquality,
                                      emul);
  }

  // Add nonlinear inequality constraints with bounds
  if (constraints.nonlinearInequality != ROL::nullPtr) {
    size_t num_nln_ineq = ModelUtils::num_nonlinear_ineq_constraints(*iteratedModel);
    auto imul = make_vector(num_nln_ineq, true);

    // Get constraint bounds
    auto nli_lower = make_vector(num_nln_ineq, false);
    auto nli_upper = make_vector(num_nln_ineq, false);
    auto& nli_l_dakota = as_dakota_vector(*nli_lower);
    auto& nli_u_dakota = as_dakota_vector(*nli_upper);

    copy_data(ModelUtils::nonlinear_ineq_constraint_lower_bounds(*iteratedModel),
              nli_l_dakota);
    copy_data(ModelUtils::nonlinear_ineq_constraint_upper_bounds(*iteratedModel),
              nli_u_dakota);

    // Clip to ROL bounds
    Real rol_inf = ROL::ROL_INF<Real>();
    Real rol_ninf = ROL::ROL_NINF<Real>();
    for (size_t i = 0; i < num_nln_ineq; i++) {
      if (nli_l_dakota[i] < rol_ninf) nli_l_dakota[i] = rol_ninf;
      if (nli_u_dakota[i] > rol_inf)  nli_u_dakota[i] = rol_inf;
    }

    auto nli_bnd = ROL::makePtr<ROL::Bounds<Real>>(nli_lower, nli_upper);
    ROL::Ptr<ROL::Vector<Real>> nli_res = ROL::nullPtr;
    pimpl_->rolProblem->addConstraint("Nonlinear Inequality",
                                      constraints.nonlinearInequality,
                                      imul,
                                      nli_bnd,
                                      nli_res);
  }

  // Finalize the problem
  pimpl_->rolProblem->finalize(false, true, Cout);

  // Create the solver
  pimpl_->rolSolver = ROL::makePtr<ROL::Solver<Real>>(pimpl_->rolProblem,
                                                       pimpl_->solverParams);
}


// Helper function to reset ROL data and solver parameters. This can
// be used to ensure that ROL is re-entrant since ROL itself does not
// provide such assurance.
void ROLOptimizer::reset_solver_options(const Teuchos::ParameterList& params)
{
  // Reset ROL solver settings.
  pimpl_->solverParams.setParameters(params);
}


// Helper function to set ROL solver parameters. This function uses
// ProblemDescDB and therefore should be called at construct time.
void ROLOptimizer::set_rol_parameters()
{
  // PRECEDENCE 1: hard-wired default settings per ROL developers'
  // suggestions

  // If the user has specified "no_hessians", tell ROL to use its own
  // Hessian approximation.
  if (iteratedModel->hessian_type() == "none") {
    pimpl_->solverParams.sublist("General").sublist("Secant").
      set("Type", "Limited-Memory BFGS");
    pimpl_->solverParams.sublist("General").sublist("Secant").
      set("Use as Hessian", true);
  }

  // Set the solver based on the type of problem.
  if (pimpl_->problemType == TYPE_U) {
    pimpl_->solverParams.sublist("Step").set("Type", "Trust Region");
    pimpl_->solverParams.sublist("Step").sublist("Trust Region").
      set("Subproblem Solver", "Truncated CG");
  }
  else if (pimpl_->problemType == TYPE_B) {
    pimpl_->solverParams.sublist("Step").set("Type", "Trust Region");
    pimpl_->solverParams.sublist("Step").sublist("Trust Region").
      set("Subproblem Solver", "Truncated CG");
  }
  else if (pimpl_->problemType == TYPE_E) {
    pimpl_->solverParams.sublist("Step").set("Type", "Composite Step");
  }
  else if (pimpl_->problemType == TYPE_EB) {
    pimpl_->solverParams.sublist("Step").set("Type", "Augmented Lagrangian");
    pimpl_->solverParams.sublist("Step").sublist("Trust Region").
      set("Subproblem Solver", "Truncated CG");
    // The default choice of Kelley-Sachs was performing lots of fn
    // evals for smoothing, so ROL developers recommend Coleman-Li.
    pimpl_->solverParams.sublist("Step").sublist("Trust Region").
      set("Subproblem Model", "Coleman-Li");

    // Turns off adaptively choosing initial penalty parameters
    // New ROL capability that results in slower convergence overall
    pimpl_->solverParams.sublist("Step").sublist("Augmented Lagrangian").
      set("Use Default Initial Penalty Parameter", false);

    // Turns off automatic constraint and objective scaling
    // New ROL capability that results in slower convergence overall
    pimpl_->solverParams.sublist("Step").sublist("Augmented Lagrangian").
      set("Use Default Problem Scaling", false);

    // Set the verbosity level.
    if (outputLevel >= VERBOSE_OUTPUT)
      pimpl_->solverParams.sublist("Step").sublist("Augmented Lagrangian").
        set("Print Intermediate Optimization History", true);
  }

  // PRECEDENCE 2: Dakota input file settings

  // Set the verbosity level.
  pimpl_->solverParams.sublist("General").
    set("Print Verbosity", outputLevel < VERBOSE_OUTPUT ? 0 : 1);

  // Set the stopping criteria.
  pimpl_->solverParams.sublist("Status Test").
    set("Gradient Tolerance", probDescDB.get_real("method.gradient_tolerance"));
  pimpl_->solverParams.sublist("Status Test").
    set("Constraint Tolerance",
        probDescDB.get_real("method.constraint_tolerance"));
  pimpl_->solverParams.sublist("Status Test").
    set("Step Tolerance", probDescDB.get_real("method.variable_tolerance"));
  // ROL enforces an int; cast is Ok since SZ_MAX default removed at Minimizer
  pimpl_->solverParams.sublist("Status Test").
    set("Iteration Limit", (int)maxIterations);

  // PRECEDENCE 3: power-user advanced options

  // Check for ROL XML input file.
  String adv_opts_file = probDescDB.get_string("method.advanced_options_file");
  if (!adv_opts_file.empty()) {
    if (std::filesystem::exists(adv_opts_file)) {
      if (outputLevel >= NORMAL_OUTPUT)
        Cout << "Any ROL options in file '" << adv_opts_file
             << "' will override Dakota options." << std::endl;
    }
    else {
      Cerr << "\nError: ROL options_file '" << adv_opts_file
           << "' specified, but file not found.\n";
      abort_handler(METHOD_ERROR);
    }

    // Set ROL solver parameters based on the XML input. Overrides
    // anything previously set.
    bool success;
    try {
      Teuchos::Ptr<Teuchos::ParameterList> osp_ptr(&pimpl_->solverParams);
      Teuchos::updateParametersFromXmlFile(adv_opts_file, osp_ptr);
      if (outputLevel >= VERBOSE_OUTPUT) {
        Cout << "ROL Solver parameters:\n";
        pimpl_->solverParams.print(Cout, 2, true, true);
      }
    }
    TEUCHOS_STANDARD_CATCH_STATEMENTS(outputLevel >= VERBOSE_OUTPUT, Cerr,
                                      success);
  }
}

} // namespace Dakota

#endif // HAVE_DAKOTA_ROL_INTERFACE
