/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       ROLOptimizer
//- Description: Wrapper class for ROL
//- Owner:       Moe Khalil
//- Checked by:
//- Version: $Id$

// Dakota headers
#include "ROLOptimizer.hpp"
#include "ProblemDescDB.hpp"

// ROL headers
#include "ROL_StdVector.hpp"
#include "ROL_Objective.hpp"
#include "ROL_Bounds.hpp"
#include "ROL_Constraint.hpp"
#include "ROL_Algorithm.hpp"
#include "ROL_OptimizationSolver.hpp"

// Teuchos headers
#include "Teuchos_XMLParameterListHelpers.hpp"
#include "Teuchos_StandardCatchMacros.hpp"

// Semi-standard headers
#include <boost/iostreams/filtering_stream.hpp>

//
// - ROLOptimizer implementation
//

// An compile unit (this source file) variable used to toggle tabular data
// output when dealing with sequential duplicates - ROL specific
namespace {
  bool orig_auto_graphics_flag = false;
}

namespace Dakota {

// -----------------------------------------------------------------
/** Implementation of ROLOptimizer class. */


// Standard constructor for ROLOptimizer.  Sets up ROL solver based on
// information from the problem database.
ROLOptimizer::ROLOptimizer(ProblemDescDB& problem_db, Model& model):
  Optimizer(problem_db, model, std::shared_ptr<TraitsBase>(new ROLTraits())),
  optSolverParams("Dakota::ROL"), problemType(TYPE_B)
{
  // Populate ROL data with user-provided problem dimensions and
  // initial values, and set ROL solver parameters.  These calls are
  // order-dependent in that the solver settings depend on
  // problemType, which is set with the problem data.

  set_problem();
  set_rol_parameters();

} // default constructor


// Alternate constructor for Iterator instantiations by name.  Sets up
// ROL solver based on information passed as arguments.
ROLOptimizer::ROLOptimizer(const String& method_string, Model& model):
  Optimizer(method_string_to_enum(method_string), model,
	    std::shared_ptr<TraitsBase>(new ROLTraits())),
  optSolverParams("Dakota::ROL"), problemType(TYPE_B)
{
  // Populate ROL data with user-provided problem dimensions and
  // initial values, and set ROL solver parameters.  These calls are
  // order-dependent in that the solver settings depend on
  // problemType, which is set with the problem data.

  set_problem();
  set_rol_parameters();

} // alternate constructor


// core_run redefines the Optimizer virtual function to perform the
// optimization using ROL and catalogue the results.
void ROLOptimizer::core_run()
{
  // ostream that will prefix lines with ROL identifier to distinguish
  // ROL output from Dakota output.
  boost::iostreams::filtering_ostream rol_cout;
  rol_cout.push(PrefixingLineFilter("ROL: "));
  rol_cout.push(Cout);

  // Should we attempt to remove this call from the constructor to avoid redundancy? - RWH
  set_problem();

  // Instantiate and call simplified interface solver object
  ROL::OptimizationSolver<Real> opt_solver( optProblem, optSolverParams );
  opt_solver.solve(rol_cout);
  rol_cout.flush();
  opt_solver.reset(true);

  // TODO: print termination criteria (based on Step or AlgorithmState?)
  // TODO: If memory serves me correctly, Russell implementd a
  // function at the DakotaOptimizer level that sets all the final
  // values.  It was based on APPS, but we should revisit to figure
  // out how this compares and if it makes sense to merge them.

  // Copy ROL solution to Dakota bestVariablesArray
  Variables& best_vars = bestVariablesArray.front();
  RealVector& cont_vars = best_vars.continuous_variables_view();
  copy_data(*rolX, cont_vars);

  // ROL does not currently provide access to the final solution, so
  // attempt a model database lookup directly into best.
  if (!localObjectiveRecast)
  {
    Response& best_resp = bestResponseArray.front();
    ActiveSet search_set(best_resp.active_set());
    search_set.request_values(AS_FUNC);
    best_resp.active_set(search_set);
    bool db_found = iteratedModel.db_lookup(best_vars, search_set, best_resp);

    // Fall back on re-evaluation if not found.
    if (db_found)
      Cout << "INFO: ROL retrieved best response from cache." << std::endl;
    else {
      Cout << "INFO: ROL re-evaluating model to retrieve best response."
        << std::endl;

      // Evaluate model for responses at best parameters and set Dakota
      // bestResponseArray.
      iteratedModel.continuous_variables(cont_vars);
      iteratedModel.evaluate();
      const RealVector& best_fns =
        iteratedModel.current_response().function_values();
      best_resp.function_values(best_fns);
    }
  }

} // core_run


// Helper function to populate ROL data with user-provided problem
// dimensions and initial values.
void ROLOptimizer::set_problem()
{
  size_t j;

  // Needed in redefining upper and lower bounds for variables and
  // inequality constraints as required by ROL for good performance.
  Real rol_inf = ROL::ROL_INF<Real>();
  Real rol_ninf = ROL::ROL_NINF<Real>();

  // TODO: Seems like we should be able to use traits to determine
  // when we need to provide the sum rather than making the user do
  // it.  Also, same comment about class data as below.  What do we
  // want/need to do about these member variables anyway?

  size_t num_eq_const = numLinearEqConstraints + numNonlinearEqConstraints;
  size_t num_ineq_const = numLinearIneqConstraints + 
                          numNonlinearIneqConstraints;

  // Set the ROL problem type.  Default is TYPE_B (bound constrained),
  // so overwrite for other constraint scenarios.
  if ( (num_ineq_const > 0) ||
       ((num_eq_const > 0) && (boundConstraintFlag)) )
    problemType = TYPE_EB;
  else {
    if (!boundConstraintFlag)
      problemType = (num_eq_const > 0) ? TYPE_E : TYPE_U;
  }

  // Instantiate null defaults for ROL problem data.  They will be
  // dimensioned and populated as appropriate later.
  Teuchos::RCP<DakotaROLObjective> obj = Teuchos::null;
  Teuchos::RCP<ROL::Vector<Real> > x = Teuchos::null;
  Teuchos::RCP<ROL::BoundConstraint<Real> > bnd = Teuchos::null;
  Teuchos::RCP<DakotaROLEqConstraints> eq_const = Teuchos::null;
  Teuchos::RCP<ROL::Vector<Real> > emul = Teuchos::null;
  Teuchos::RCP<DakotaROLIneqConstraints> ineq_const = Teuchos::null;
  Teuchos::RCP<ROL::Vector<Real> > imul = Teuchos::null;
  Teuchos::RCP<ROL::BoundConstraint<Real> > ineq_bnd = Teuchos::null;

  // Dimension ROL variable vector and set initial values.
  rolX.reset(new std::vector<Real>(numContinuousVars, 0.0));
  get_initial_values(iteratedModel, *rolX);
  x.reset( new ROL::StdVector<Real>(rolX) );

  // For TYPE_B and TYPE_EB problems, dimension bounds variable
  // vectors and set initial values.  Map any Dakota infinite bounds
  // to ROL_INF for best performance.
  if ( (problemType == TYPE_B) || (problemType == TYPE_EB) ) {

    Teuchos::RCP<std::vector<Real> >
      l_rcp(new std::vector<Real>(numContinuousVars, 0.0));
    Teuchos::RCP<std::vector<Real> >
      u_rcp(new std::vector<Real>(numContinuousVars, 0.0));
    get_bounds(iteratedModel, *l_rcp, *u_rcp);

    // Set bounds greater (less) than ROL_INF (ROL_NINF) to ROL_INF
    // (ROL_NINF)
    for (size_t i = 0; i < numContinuousVars; i++) {
      if ((*l_rcp)[i] < rol_ninf)
        (*l_rcp)[i] = rol_ninf;
      if ((*u_rcp)[i] > rol_inf)
        (*u_rcp)[i] = rol_inf;
    }

    // Set bounds values in ROL::BoundConstraint object.
    lowerBounds = Teuchos::rcp( new ROL::StdVector<Real>( l_rcp ) );
    upperBounds = Teuchos::rcp( new ROL::StdVector<Real>( u_rcp ) );
    bnd.reset( new ROL::Bounds<Real>(lowerBounds, upperBounds) );
  }

  // Create objective function object and give it access to Dakota
  // model.  If there is a Dakota/user-provided Hessian, need to
  // instantiate the "Hess" version to enable support for it.
  if (iteratedModel.hessian_type() == "none")
    obj.reset(new DakotaROLObjective(iteratedModel));
  else
    obj.reset(new DakotaROLObjectiveHess(iteratedModel));

  // If there are equality constraints, create the object and provide
  // the Dakota model.
  if (num_eq_const > 0){
    // create appropriate equality constraint object and give it
    // access to Dakota model
    if (iteratedModel.hessian_type() == "none")
      eq_const.reset(new DakotaROLEqConstraints(iteratedModel));
    else
      eq_const.reset(new DakotaROLEqConstraintsHess(iteratedModel));

    // Initialize Lagrange multipliers for equality constraints.
    Teuchos::RCP<std::vector<Real> > emul_rcp = Teuchos::rcp( new std::vector<Real>(num_eq_const,0.0) );
    emul.reset(new ROL::StdVector<Real>(emul_rcp) );
  }

  // If there are inequality constraints, create the object and provide
  // the Dakota model. (Order: [linear_ineq, nonlinear_ineq])
  if (num_ineq_const > 0){
    // create appropriate inequality constraint object and give it
    // access to Dakota model
    if (iteratedModel.hessian_type() == "none")
      ineq_const.reset(new DakotaROLIneqConstraints(iteratedModel));
    else
      ineq_const.reset(new DakotaROLIneqConstraintsHess(iteratedModel));

    // Initial Lagrange multipliers for inequality constraints.
    Teuchos::RCP<std::vector<Real> > imul_rcp = 
      Teuchos::rcp( new std::vector<Real>(num_ineq_const,0.0) );
    imul.reset(new ROL::StdVector<Real>(imul_rcp) );
  
    // Create ROL inequality constraint bound vectors.
    Teuchos::RCP<std::vector<Real> >
      ineq_l_rcp(new std::vector<Real>(num_ineq_const, 0.0));
    Teuchos::RCP<std::vector<Real> >
      ineq_u_rcp(new std::vector<Real>(num_ineq_const, 0.0));

    // TODO: Seems like we should be able to pull this into the
    // adapters and use traits to determine what to populate the
    // bounds vectors with...similar to get_bounds for the bound
    // constraints above.

    // Get the inequality bounds from Dakota and transfer them into
    // the ROL vectors. 
    copy_data_partial(iteratedModel.linear_ineq_constraint_lower_bounds(),
		      *ineq_l_rcp, 0);
    copy_data_partial(iteratedModel.linear_ineq_constraint_upper_bounds(),
		      *ineq_u_rcp, 0);
    copy_data_partial(iteratedModel.nonlinear_ineq_constraint_lower_bounds(),
		      *ineq_l_rcp, numLinearIneqConstraints);
    copy_data_partial(iteratedModel.nonlinear_ineq_constraint_upper_bounds(),
		      *ineq_u_rcp, numLinearIneqConstraints);

    // Set bounds greater (less) than ROL_INF (ROL_NINF) to ROL_INF
    // (ROL_NINF)
    for (size_t i = 0; i < num_ineq_const; i++) {
      if ((*ineq_l_rcp)[i] < rol_ninf)
        (*ineq_l_rcp)[i] = rol_ninf;
      if ((*ineq_u_rcp)[i] > rol_inf)
        (*ineq_u_rcp)[i] = rol_inf;
    }

    // Set the ROL::BoundConstraint object.
    Teuchos::RCP<ROL::Vector<Real> >
      ineq_lower_bounds( new ROL::StdVector<Real>( ineq_l_rcp ) );
    Teuchos::RCP<ROL::Vector<Real> >
      ineq_upper_bounds( new ROL::StdVector<Real>( ineq_u_rcp ) );
    ineq_bnd.reset( new ROL::Bounds<Real>(ineq_lower_bounds,ineq_upper_bounds) );
  }

  // Instantiate ROL problem and populate it with relevant data.
  optProblem = ROL::OptimizationProblem<Real> (obj, x, bnd, eq_const, emul,
					       ineq_const, imul, ineq_bnd);

} // set_problem

void ROLOptimizer::initialize_run()
{
  Optimizer::initialize_run();

  // Needed to make ROL output to tabular data consistent with other Opt TPLs
  // NOTE: This cannot be set in the consructr (or helper functions associated
  //       with the contructor) because some variables get assigned by base 
  //       class constructors and are not available until all have completed.
  orig_auto_graphics_flag = iteratedModel.auto_graphics();
}

// Helper function to reset ROL data and solver parameters.  This can
// be used to ensure that ROL is re-entrant since ROL itself does not
// provide such assurance.
void ROLOptimizer::reset_solver_options(const Teuchos::ParameterList & params)
{
  // Reset ROL solver settings.
  optSolverParams.setParameters(params);
} // reset_solver_options


// Helper function to set ROL solver parameters.  This function uses
// ProblemDescDB and therefore should be called at construct time.
void ROLOptimizer::set_rol_parameters()
{
  // PRECEDENCE 1: hard-wired default settings

  // If the user has specified "no_hessians", tell ROL to use its own
  // Hessian approximation.
  if (iteratedModel.hessian_type() == "none") {
    optSolverParams.sublist("General").sublist("Secant").
      set("Type", "Limited-Memory BFGS");
    optSolverParams.sublist("General").sublist("Secant").
      set("Use as Hessian", true);
  }

  // Set the solver based on the type of problem.
  if (problemType == TYPE_U){
    optSolverParams.sublist("Step").set("Type","Trust Region");
    optSolverParams.sublist("Step").sublist("Trust Region").set("Subproblem Solver", "Truncated CG");
  }
  else if (problemType == TYPE_B){
    optSolverParams.sublist("Step").set("Type","Trust Region");
    optSolverParams.sublist("Step").sublist("Trust Region").set("Subproblem Solver", "Truncated CG");
  }
  else if (problemType == TYPE_E){
    optSolverParams.sublist("Step").set("Type","Composite Step");
  }
  else if (problemType == TYPE_EB){
    optSolverParams.sublist("Step").set("Type","Augmented Lagrangian");
    optSolverParams.sublist("Step").sublist("Trust Region").set("Subproblem Solver", "Truncated CG");
    // The default choice of Kelley-Sachs was performing lots of fn
    // evals for smoothing, so ROL developers recommend Coleman-Li.
    optSolverParams.sublist("Step").sublist("Trust Region").set("Subproblem Model", "Coleman-Li");

    // Turns off adaptively choosing initial penalty parameters
    // New ROL capabaility that results in slower convergence overall
    optSolverParams.sublist("Step").sublist("Augmented Lagrangian").
        set("Use Default Initial Penalty Parameter",false);

    // Turns off automatic constraint and objective scaling
    // New ROL capabaility that results in slower convergence overall
    optSolverParams.sublist("Step").sublist("Augmented Lagrangian").
        set("Use Default Problem Scaling",false);

    // QUESTION: Is there a reason this is only for problem TYPE_EB?
    // Set the verbosity level.
    if (outputLevel >= VERBOSE_OUTPUT)
      optSolverParams.sublist("Step").sublist("Augmented Lagrangian").
        set("Print Intermediate Optimization History",true);
  }

  // PRECEDENCE 2: Dakota input file settings

  // Set the verbosity level.
  optSolverParams.sublist("General").
    set("Print Verbosity", outputLevel < VERBOSE_OUTPUT ? 0 : 1);

  // Set the stopping criteria.
  optSolverParams.sublist("Status Test").
    set("Gradient Tolerance", probDescDB.get_real("method.gradient_tolerance"));
  optSolverParams.sublist("Status Test").
    set("Constraint Tolerance",
	probDescDB.get_real("method.constraint_tolerance")
	);
  optSolverParams.sublist("Status Test").
    set("Step Tolerance", probDescDB.get_real("method.variable_tolerance"));
  optSolverParams.sublist("Status Test").set("Iteration Limit", maxIterations);

  // BMA: We aren't yet using ROL's Trust Region Step, but Patty
  // called out these settings that we'll want to map

  // TODO: how to map Dakota vector to ROL scalar?
  // const RealVector& tr_init_size =
  //   probDescDB.get_rv("method.trust_region.initial_size");
  // if (!tr_init_size.empty())
  //   optSolverParams.sublist("Step").sublist("Trust Region").
  //      set("Initial Radius", tr_init_size[0]);

  // optSolverParams.sublist("Step").sublist("Trust Region").
  //   set("Radius Shrinking Threshold",
  // 	probDescDB.get_real("method.trust_region.contract_threshold"));
  // optSolverParams.sublist("Step").sublist("Trust Region").
  //   set("Radius Growing Threshold",
  // 	probDescDB.get_real("method.trust_region.expand_threshold"));
  // optSolverParams.sublist("Step").sublist("Trust Region").
  //   set("Radius Shrinking Rate (Negative rho)",
  // 	probDescDB.get_real("method.trust_region.contraction_factor"));
  // optSolverParams.sublist("Step").sublist("Trust Region").
  //   set("Radius Shrinking Rate (Positive rho)",
  // 	probDescDB.get_real("method.trust_region.contraction_factor"));
  // optSolverParams.sublist("Step").sublist("Trust Region").
  //   set("Radius Growing Rate",
  // 	probDescDB.get_real("method.trust_region.expansion_factor"));

  // PRECEDENCE 3: power-user advanced options

  // Check for ROL XML input file.
  String adv_opts_file = probDescDB.get_string("method.advanced_options_file");
  if (!adv_opts_file.empty()) {
    if (boost::filesystem::exists(adv_opts_file)) {
      if (outputLevel >= NORMAL_OUTPUT)
	       Cout << "Any ROL options in file '" << adv_opts_file
	         << "' will override Dakota options." << std::endl;
    }
    else {
      Cerr << "\nError: ROL options_file '" << adv_opts_file
	   << "' specified, but file not found.\n";
      abort_handler(METHOD_ERROR);
    }

    // Set ROL solver parameters based on the XML input.  Overrides
    // anything previously set.
    bool success;
    try {
      Teuchos::Ptr<Teuchos::ParameterList> osp_ptr(&optSolverParams);
      Teuchos::updateParametersFromXmlFile(adv_opts_file, osp_ptr);
      if (outputLevel >= VERBOSE_OUTPUT) {
      	Cout << "ROL OptimizationSolver parameters:\n";
      	   optSolverParams.print(Cout, 2, true, true);
      }
    }
    TEUCHOS_STANDARD_CATCH_STATEMENTS(outputLevel >= VERBOSE_OUTPUT, Cerr,
				      success);
  }

} // set_rol_parameters


// --------------------------------------------------------------
//    These classes could go into a new file for evaluators along
//    the lines of APPS and COLIN if desirable.
// --------------------------------------------------------------


namespace {
  // Helper function to manage response evaluations, both objectives
  // and nonlinear constraints.  The response and all derivatives are
  // evaluated on first call and need only be retrieved from the
  // database upon subsequent calls.  This allows for time-saving
  // speculative derivative computation, which is not natively
  // supported by ROL.

  void update_model(Model & model, const std::vector<Real> & x)
  {
    static std::vector<Real> prev_x = x;
    static bool first_step = true;

    // throttle consequetive duplicate output
    bool is_seq_dup = !first_step && (prev_x == x);
    if( is_seq_dup )
      model.auto_graphics(false);
    else
      prev_x = x;

    // CLEAN-UP: Use an adapter.
    // Set the model variables to the current values.
    size_t num_cv = model.cv();
    for(size_t i=0; i<num_cv; ++i)
      model.continuous_variable(x[i], i);

    // Evaluate response, gradient, and Hessian; the last only if
    // provided by Dakota/user.
    ActiveSet eval_set(model.current_response().active_set());
    if (model.hessian_type() == "none")
      eval_set.request_values(AS_FUNC+AS_GRAD);
    else
      eval_set.request_values(AS_FUNC+AS_GRAD+AS_HESS);
    model.evaluate(eval_set);

    // Restore tabular data output state
    if( is_seq_dup )
      model.auto_graphics(orig_auto_graphics_flag);

    first_step = false;

  } // update_model

} // namespace anonymous


// -----------------------------------------------------------------
/** Implementation of the DakotaROLObjective class. */


// Constructor.
DakotaROLObjective::DakotaROLObjective(Model & model) :
  dakotaModel(model)
{ }


// CLEAN-UP: Can we hide the model details in a data adapter for the
// objective and gradient evaluations?  Because of the way ROL appears
// to handle constraints, may not be a big deal for this wrapper, but
// it will likely simplify things in other wrappers.


// Compute objective value and return to ROL.
Real
DakotaROLObjective::value(const std::vector<Real> &x, Real &tol)
{
  update_model(dakotaModel, x);
  return dakotaModel.current_response().function_value(0);

} // objective value


// Compute objective gradient and return to ROL.
void
DakotaROLObjective::gradient( std::vector<Real> &g, const std::vector<Real> &x, Real &tol )
{
  update_model(dakotaModel, x);
  copy_column_vector(dakotaModel.current_response().function_gradients(), 0, g);

} // objective gradient


// -----------------------------------------------------------------
/** Implementation of the DakotaROLObjective class. */


// Constructor.
DakotaROLObjectiveHess::DakotaROLObjectiveHess(Model & model) :
  DakotaROLObjective(model)
{ }


// Compute objective Hessian-vector product and return to ROL.
void
DakotaROLObjectiveHess::hessVec(std::vector<Real> &hv,
				const std::vector<Real> &v,
				const std::vector<Real> &x,
				Real &tol)
{
  // Make sure the Hessian has been evaluated and get it from Dakota
  // Response.
  update_model(dakotaModel, x);
  const RealSymMatrix& hess_f = dakotaModel.current_response().function_hessian(0);

  // Multiply the objective Hessian matrix by the vector.
  apply_matrix_partial(hess_f, v, hv);

} // objective hessVec


// Compute product of objective Hessian and a vector.  Not implemented
// since no ROL methods currently exposed use it.
void
DakotaROLObjectiveHess::invHessVec(std::vector<Real> &h,
				   const std::vector<Real> &v,
				   const std::vector<Real> &x,
				   Real &tol)
{
  Cerr << "\nError: DakotaROLObjectiveHess::invHessVec is not currently supported."
       << "  logic error.\n";
  abort_handler(METHOD_ERROR);

} // objective invHessVec


// -----------------------------------------------------------------
/** Implementation of the DakotaROLIneqConstraints class. */


// Constructor.
DakotaROLIneqConstraints::DakotaROLIneqConstraints(Model & model) :
  dakotaModel(model)
{
  haveNlnConst = model.num_nonlinear_ineq_constraints() > 0;

} // ineqConstraints constructor


// Compute inequality constraint values and return to ROL.
void
DakotaROLIneqConstraints::value(std::vector<Real> &c, const std::vector<Real> &x, Real &tol)
{
  // CLEAN-UP: Should look into whether or not it makes sense to use
  // traits to determine how to pack these and just return one vector
  // of constraint values?

  // Evaluate nonlinear constraints.
  update_model(dakotaModel, x);

  // Matrix-vector multiply to get linear constraint values.
  apply_linear_constraints( dakotaModel, CONSTRAINT_EQUALITY_TYPE::INEQUALITY, x, c );

  get_nonlinear_ineq_constraints( dakotaModel, c );

} // ineqConstraints value


// Multiply a vector by the inequality constraint Jacobian and return
// to ROL.
void 
DakotaROLIneqConstraints::applyJacobian(std::vector<Real> &jv,
    const std::vector<Real> &v, const std::vector<Real> &x, Real &tol)
{
  // Matrix-vector multiply to apply linear constraint Jacobian.
  const RealMatrix & lin_ineq_coeffs = dakotaModel.linear_ineq_constraint_coeffs();
  apply_matrix_partial(lin_ineq_coeffs, v, jv);

  // Apply nonlinear constraint Jacobian.
  if( haveNlnConst ) {
    update_model(dakotaModel, x);
    apply_nonlinear_constraints(dakotaModel, CONSTRAINT_EQUALITY_TYPE::INEQUALITY, v, jv);
  }

} // ineqConstraints applyJacobian


// Multiply a vector by the transpose of the inequality constraint
// Jacobian and return to ROL.
void
DakotaROLIneqConstraints::applyAdjointJacobian(std::vector<Real> &ajv,
    const std::vector<Real> &v, const std::vector<Real> &x, Real &tol)
{
  // Must init since are merging the effect of linear and nonlinear
  // might get skipped.
  ajv.assign(ajv.size(), 0.0);

  // Matrix-vector multiply to apply transpose of linear constraint
  // Jacobian.
  const RealMatrix & lin_ineq_coeffs = dakotaModel.linear_ineq_constraint_coeffs();
  apply_matrix_transpose_partial(lin_ineq_coeffs, v, ajv);

  // Apply transpose of nonlinear constraint Jacobian.
  if (haveNlnConst) {
    update_model(dakotaModel, x);
    apply_nonlinear_constraints(dakotaModel, CONSTRAINT_EQUALITY_TYPE::INEQUALITY, v, ajv, true);
  }

} // ineqConstraints applyAdjointJacobian


// -----------------------------------------------------------------
/** Implementation of the DakotaROLIneqConstraintsHess class. */


// Constructor.
DakotaROLIneqConstraintsHess::DakotaROLIneqConstraintsHess(Model & model) :
  DakotaROLIneqConstraints(model)
{ }


// Multiply a vector by the transpose of the inequality constraint
// Hessian and return to ROL.
void
DakotaROLIneqConstraintsHess::applyAdjointHessian( std::vector<Real> & ahuv,
                                                   const std::vector<Real> & u, 
                                                   const std::vector<Real> & v,
                                                   const std::vector<Real> & x,
                                                   Real &tol )
{
  // Must init since linear info is 0.0 and we are appending 
  // the effect of nonlinear
  ahuv.assign(ahuv.size(), 0.0);

  // apply nonlinear constraint Hessian (might be empty)
  if (haveNlnConst) {
    // make sure that model is current
    update_model(dakotaModel, x);

    RealSymMatrix hu(dakotaModel.current_response().function_hessian(1));
    hu *= u[0];

    for( size_t i=1; i<dakotaModel.num_nonlinear_ineq_constraints(); ++i )
    {
      RealSymMatrix temp_hu(dakotaModel.current_response().function_hessian(1+i));
      temp_hu *= u[i];
      hu += temp_hu;
    }

    apply_matrix_partial(hu, v, ahuv);
  }

} // ineqConstraintsHess applyAdjointHessian


// -----------------------------------------------------------------
/** Implementation of the DakotaROLEqConstraints class. */


// Constructor.
DakotaROLEqConstraints::DakotaROLEqConstraints(Model & model) :
  dakotaModel(model)
{
  haveNlnConst = model.num_nonlinear_eq_constraints() > 0;

} // eqContraints constructor


// Compute equality constraint values and return to ROL.
void
DakotaROLEqConstraints::value(std::vector<Real> &c, const std::vector<Real> &x, Real &tol)
{
  // Evaluate nonlinear constraints.
  update_model(dakotaModel, x);

  // Matrix-vector multiply to get linear constraint values.
  apply_linear_constraints( dakotaModel, CONSTRAINT_EQUALITY_TYPE::EQUALITY, x, c );

  // QUESTION: What does this do?  Also, why the -1.0?
  get_nonlinear_eq_constraints( dakotaModel, c, -1.0 );

} // eqConstraints value


// Multiply a vector by the equality constraint Jacobian and return
// to ROL.
void
DakotaROLEqConstraints::applyJacobian(std::vector<Real> &jv,
    const std::vector<Real> &v, const std::vector<Real> &x, Real &tol)
{
  // Matrix-vector multiply to apply linear constraint Jacobian.
  const RealMatrix & lin_eq_coeffs = dakotaModel.linear_eq_constraint_coeffs();
  apply_matrix_partial(lin_eq_coeffs, v, jv);

  // Apply nonlinear constraint Jacobian.
  if( haveNlnConst ) {
    update_model(dakotaModel, x);
    apply_nonlinear_constraints(dakotaModel, CONSTRAINT_EQUALITY_TYPE::EQUALITY, v, jv);
  }

} // eqConstraints applyJacobian


// Multiply a vector by the transpose of the equality constraint
// Jacobian and return to ROL.
void
DakotaROLEqConstraints::applyAdjointJacobian(std::vector<Real> &ajv,
    const std::vector<Real> &v, const std::vector<Real> &x, Real &tol)
{
  // Must init since are merging the effect of linear and nonlinear
  // and linear might get skipped
  ajv.assign(ajv.size(), 0.0);

  // Matrix-vector multiply to apply transpose of linear constraint
  // Jacobian.
  const RealMatrix & lin_eq_coeffs = dakotaModel.linear_eq_constraint_coeffs();
  apply_matrix_transpose_partial(lin_eq_coeffs, v, ajv);

  // Apply transpose of nonlinear constraint Jacobian.
  if (haveNlnConst) {
    update_model(dakotaModel, x);
    apply_nonlinear_constraints(dakotaModel, CONSTRAINT_EQUALITY_TYPE::EQUALITY, v, ajv, true);
  }

} // eqConstraints applyAdjointJacobian



// -----------------------------------------------------------------
/** Implementation of the DakotaROLEqConstraintsHess class. */


// Constructor.
DakotaROLEqConstraintsHess::DakotaROLEqConstraintsHess(Model & model) :
  DakotaROLEqConstraints(model)
{ }


// Multiply a vector by the transpose of the inequality constraint
// Hessian and return to ROL.
void
DakotaROLEqConstraintsHess::applyAdjointHessian( std::vector<Real> & ahuv,
                                                   const std::vector<Real> & u, 
                                                   const std::vector<Real> & v,
                                                   const std::vector<Real> & x,
                                                   Real &tol )
{
  // Must init since linear info is 0.0 and we are appending 
  // the effect of nonlinear
  ahuv.assign(ahuv.size(), 0.0);

  // apply nonlinear constraint Hessian (might be empty)
  if (haveNlnConst) {

    size_t num_nln_ineq_constraints = dakotaModel.num_nonlinear_ineq_constraints();
    // make sure that model is current
    update_model(dakotaModel, x);

    RealSymMatrix hu(dakotaModel.current_response().function_hessian(1+num_nln_ineq_constraints));
    hu *= u[0];

    for( size_t i=1; i<dakotaModel.num_nonlinear_eq_constraints(); ++i )
    {
      RealSymMatrix temp_hu(dakotaModel.current_response().function_hessian(1+num_nln_ineq_constraints+i));
      temp_hu *= u[i];
      hu += temp_hu;
    }

    apply_matrix_partial(hu, v, ahuv);
  }

} // eqConstraintsHess applyAdjointHessian


} // namespace Dakota
