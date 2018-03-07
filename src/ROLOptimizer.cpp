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
// CLEAN-UP: Are these comments still needed?
// BMA TODO: Above will break with newer Teuchos; instead need 
//#include "Teuchos_XMLParameterListCoreHelpers.hpp"
#include "Teuchos_StandardCatchMacros.hpp"

// Semi-standard headers
#include <boost/iostreams/filtering_stream.hpp>

//
// - ROLOptimizer implementation
//

namespace Dakota {

// -----------------------------------------------------------------
/** Standard constructor for ROLOptimizer.  Sets up ROL solver based
    on information from the problem database. */

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
}

// -----------------------------------------------------------------
/** Alternate constructor for Iterator instantiations by name.  Sets
    up ROL solver based on information passed as arguments. */

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
}

// -----------------------------------------------------------------
/** core_run redefines the Optimizer virtual function to perform
    the optimization using ROL and catalogue the results. */

void ROLOptimizer::core_run()
{
  // ostream that will prefix lines with ROL identifier
  boost::iostreams::filtering_ostream rol_cout;
  rol_cout.push(PrefixingLineFilter("ROL: "));
  // CLEAN-UP: Are these comments still needed?
  // Tried to disable buffering so ROL output gets inlined with
  // Dakota's; doesn't work perfectly. Instead, modified ROL to flush()
  //std::streamsize buffer_size = 0;
  //rol_cout.push(Cout, buffer_size);
  rol_cout.push(Cout);

  // Instantiate and call simplified interface solver object
  ROL::OptimizationSolver<Real> opt_solver( optProblem, optSolverParams );
  opt_solver.solve(rol_cout);
  rol_cout.flush();

  // TODO: print termination criteria (based on Step or AlgorithmState?)

  // CLEAN-UP: If memory serves me correctly, Russell implementd a
  // function at the DakotaOptimizer level that sets all the final
  // values.  It was based on APPS, but we should revisit to figure
  // out how this compares and if it makes sense to merge them.

  // QUESTION: Do we need the copy_data?  Where is rolX coming from
  // and/or what does is get used for?
  // Copy ROL solution to Dakota bestVariablesArray
  Variables& best_vars = bestVariablesArray.front();
  RealVector& cont_vars = best_vars.continuous_variables_view();
  copy_data(*rolX, cont_vars);

  // ROL does not currently provide access to the final solution, so
  // attempt a model database lookup directly into best.
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

    // QUESTION: Is this evaluating at the right parameter values?  If
    // I read other code correctly, rolX was copied into cont_vars.
    // But rolX hasn't been touched since the problem was set up.
    // Evaluate model for responses at best parameters and set Dakota
    // bestResponseArray
    iteratedModel.continuous_variables(cont_vars);
    iteratedModel.evaluate();
    const RealVector& best_fns =
      iteratedModel.current_response().function_values();
    best_resp.function_values(best_fns);
  }
}

// -----------------------------------------------------------------
/** Helper function to populate ROL data with user-provided problem
    dimensions and initial values. */

void ROLOptimizer::set_problem()
{
  size_t j;

  // CLEAN-UP: Seems like we should be able to use traits to determine
  // when we need to provide the sum rather than making the user do
  // it.  Also, same comment about class data as below.

  size_t num_eq_const = numLinearEqConstraints + numNonlinearEqConstraints;
  size_t num_ineq_const = numLinearIneqConstraints + numNonlinearIneqConstraints;
  // Set the ROL problem type.  Default is TYPE_B (bound constrained),
  // so overwrite for other constraint scenarios.
  if ( (num_ineq_const > 0) ||
       ((num_eq_const > 0) && (boundConstraintFlag)) )
    problemType = TYPE_EB;
  else {
    if (!boundConstraintFlag)
      problemType = (num_eq_const > 0) ? TYPE_E : TYPE_U;
  }

  // QUESTION: Why do it this way?  Is it recommended practice for
  // ROL?  Some other best practice?
  // Instantiate null defaults for ROL problem data.  They will be
  // dimensioned and populated later.
  Teuchos::RCP<DakotaROLObjective> obj = Teuchos::null;
  Teuchos::RCP<ROL::Vector<Real> > x = Teuchos::null;
  Teuchos::RCP<ROL::BoundConstraint<Real> > bnd = Teuchos::null;
  Teuchos::RCP<DakotaROLEqConstraints> eq_const = Teuchos::null;
  Teuchos::RCP<ROL::Vector<Real> > emul = Teuchos::null;
  Teuchos::RCP<DakotaROLIneqConstraints> ineq_const = Teuchos::null;
  Teuchos::RCP<ROL::Vector<Real> > imul = Teuchos::null;
  Teuchos::RCP<ROL::BoundConstraint<Real> > ineq_bnd = Teuchos::null;

  // CLEAN-UP: Should figure out which of this class data (e.g.,
  // numContinuousVars) we could/should elminate and which we should
  // keep and formally make part of the TPL API.

  // QUESTION: Why do we need both rolX and x?
  // QUESTION: Should all bound constraint related stuff be inside the
  // conditional below?
  // Dimension ROL variable vector and set initial values.
  rolX.reset(new std::vector<Real>(numContinuousVars, 0.0));
  Teuchos::RCP<std::vector<Real> >
    l_rcp(new std::vector<Real>(numContinuousVars, 0.0));
  Teuchos::RCP<std::vector<Real> >
    u_rcp(new std::vector<Real>(numContinuousVars, 0.0));

  get_initial_values(iteratedModel, *rolX);
  x.reset( new ROL::StdVector<Real>(rolX) );

  // For TYPE_B and TYPE_EB problems, set values of the bounds.  Map
  // any Dakota infinite bounds to ROL_INF for best performance.
  if ( (problemType == TYPE_B) || (problemType == TYPE_EB) ) {
    Real rol_inf = ROL::ROL_INF<Real>();
    Real rol_ninf = ROL::ROL_NINF<Real>();

    get_bounds(iteratedModel, *l_rcp, *u_rcp);

    // QUESTION: Is there a reason not to just use isfinite() and set
    // to rol_(n)inf based on that?  And maybe save the extra data
    // running around with the assignment above?
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

  // QUESTION: Is there a distinction/order between linear and
  // nonlinear as there appears to be with the inequality constraints?
  // Does ROL assume the target is 0.0?
  // If there are equality constraints, create the object and provide
  // the Dakota model.
  if (num_eq_const > 0){
    eq_const.reset(new DakotaROLEqConstraints(iteratedModel));

    // QUESTION: What are the multipliers for?
    // equality multipliers
    Teuchos::RCP<std::vector<Real> > emul_rcp = Teuchos::rcp( new std::vector<Real>(num_eq_const,0.0) );
    emul.reset(new ROL::StdVector<Real>(emul_rcp) );
  }

  // If there are inequality constraints, create the object and provide
  // the Dakota model. (Order: [linear_ineq, nonlinear_ineq])
  if (num_ineq_const > 0){
    ineq_const.reset(new DakotaROLIneqConstraints(iteratedModel));

    // QUESTION: What are the multipliers for?
    // inequality multipliers
    Teuchos::RCP<std::vector<Real> > imul_rcp = 
      Teuchos::rcp( new std::vector<Real>(num_ineq_const,0.0) );
    imul.reset(new ROL::StdVector<Real>(imul_rcp) );
  
    // create ROL inequality constraint bound vectors
    Teuchos::RCP<std::vector<Real> >
      ineq_l_rcp(new std::vector<Real>(num_ineq_const, 0.0));
    Teuchos::RCP<std::vector<Real> >
      ineq_u_rcp(new std::vector<Real>(num_ineq_const, 0.0));

    // CLEAN-UP: Seems like we should be able to pull this into the
    // adapters and use traits to determine what to populate the
    // bounds vectors with...similar to get_bounds for the bound
    // constraints above.

    // Get the inequality bounds from Dakota and transfer them into a
    // ROL::BoundConstraint object.
    copy_data_partial(iteratedModel.linear_ineq_constraint_lower_bounds(),
		      *ineq_l_rcp, 0);
    copy_data_partial(iteratedModel.linear_ineq_constraint_upper_bounds(),
		      *ineq_u_rcp, 0);
    copy_data_partial(iteratedModel.nonlinear_ineq_constraint_lower_bounds(),
		      *ineq_l_rcp, numLinearIneqConstraints);
    copy_data_partial(iteratedModel.nonlinear_ineq_constraint_upper_bounds(),
		      *ineq_u_rcp, numLinearIneqConstraints);
    Teuchos::RCP<ROL::Vector<Real> >
      ineq_lower_bounds( new ROL::StdVector<Real>( ineq_l_rcp ) );
    Teuchos::RCP<ROL::Vector<Real> >
      ineq_upper_bounds( new ROL::StdVector<Real>( ineq_u_rcp ) );
    ineq_bnd.reset( new ROL::Bounds<Real>(ineq_lower_bounds,ineq_upper_bounds) );
  }

  // Instantiate ROL problem and populate it with relevant data.
  optProblem = ROL::OptimizationProblem<Real> (obj, x, bnd, eq_const, emul,
					       ineq_const, imul, ineq_bnd);

  // CLEAN-UP: Do we need these comments anymore?
  // checking, may be enabled in tests or debug mode

  // Teuchos::RCP<std::ostream> outStream_checking;
  // outStream_checking = Teuchos::rcp(&std::cout, false);
  // optProblem.check(*outStream_checking);
}

// QUESTION: Shouldn't this be called from somewhere?
// -----------------------------------------------------------------
/** Helper function to reset ROL data and solver parameters.  This
    can be used to ensure that ROL is re-entrant since ROL itself
    does not provide such assurance. */

void ROLOptimizer::reset_problem( const RealVector & init_vals,
                                  const RealVector & lower_bnds,
                                  const RealVector & upper_bnds,
                                  const Teuchos::ParameterList & params)
{
  // Reset ROL problem data.
  copy_data(init_vals, *rolX);
  copy_data(lower_bnds, *(lowerBounds->getVector()));
  copy_data(upper_bnds, *(upperBounds->getVector()));
  optProblem.reset();

  // Reset ROL solver settings.
  optSolverParams.setParameters(params);
}

// -----------------------------------------------------------------
/** Helper function to set ROL solver parameters.  This function uses
    ProblemDescDB and therefore must should be called at construct
    time. */

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

    // QUESTION: Is there a reason this is only for problem TYPE_EB?
    // Set the verbosity level.
    if (outputLevel >= VERBOSE_OUTPUT)
      optSolverParams.sublist("Step").sublist("Augmented Lagrangian").
        set("Print Intermediate Optimization History","true");
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
    set("Step Tolerance", probDescDB.get_real("method.threshold_delta"));
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

  // Check for and ROL XML input file.
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

    // Update ROL solver parameters based on the XML input.
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
}

// --------------------------------------------------------------

// --------------------------------------------------------------
//    These classes could go into a new file for evaluators along
//    the lines of APPS and COLIN if desirable.
// --------------------------------------------------------------

/// A helper function for consolidating model callbacks
namespace {

  void update_model(Model & model, const std::vector<Real> & x)
  {
    // Could replace with an adapter call - RWH
//PDH: Agreed.

    size_t num_cv = model.cv();
    for(size_t i=0; i<num_cv; ++i)
      model.continuous_variable(x[i], i);

    // HESSIAN TODO: added AS_HESS; need to track if this creates issues

    ActiveSet eval_set(model.current_response().active_set());
    if (model.hessian_type() == "none")
      eval_set.request_values(AS_FUNC+AS_GRAD);
    else
      eval_set.request_values(AS_FUNC+AS_GRAD+AS_HESS);
    model.evaluate(eval_set);

    // now we can use the response currently in the model for any
    // obj/cons/grad/hess
  }

} // namespace anonymous

// --------------------------------------------------------------
//               DakotaROLObjective
// --------------------------------------------------------------

DakotaROLObjective::DakotaROLObjective(Model & model) :
  dakotaModel(model)
{ }

//PDH: Can we hide the model details in a data adapter for the objective
//and gradient evaluations?  Because of the way ROL appears to handle
//constraints, may not be a big deal for this wrapper, but it will likely
//simplify things in other wrappers.

Real
DakotaROLObjective::value(const std::vector<Real> &x, Real &tol)
{
  update_model(dakotaModel, x);
  return dakotaModel.current_response().function_value(0);
}

void
DakotaROLObjective::gradient( std::vector<Real> &g, const std::vector<Real> &x, Real &tol )
{
  update_model(dakotaModel, x);
  copy_column_vector(dakotaModel.current_response().function_gradients(), 0, g);
}

// --------------------------------------------------------------
//             DakotaROLIneqConstraints
// --------------------------------------------------------------

DakotaROLIneqConstraints::DakotaROLIneqConstraints(Model & model) :
  dakotaModel(model)
{
  haveNlnConst = model.num_nonlinear_ineq_constraints() > 0;
}

void
DakotaROLIneqConstraints::value(std::vector<Real> &c, const std::vector<Real> &x, Real &tol)
{
//PDH: Should look into whether or not it makes sense to use traits to
//determine how to pack these and just return one vectore of constraint
//values.

  update_model(dakotaModel, x);
  apply_linear_constraints( dakotaModel, CONSTRAINT_EQUALITY_TYPE::INEQUALITY, x, c );
  get_nonlinear_ineq_constraints( dakotaModel, c );
}

void 
DakotaROLIneqConstraints::applyJacobian(std::vector<Real> &jv,
    const std::vector<Real> &v, const std::vector<Real> &x, Real &tol)
{
  // apply linear constraint Jacobian
  const RealMatrix & lin_ineq_coeffs = dakotaModel.linear_ineq_constraint_coeffs();
  apply_matrix_partial(lin_ineq_coeffs, v, jv);

  // apply nonlinear constraint Jacobian
  if( haveNlnConst ) {
    update_model(dakotaModel, x);
    apply_nonlinear_constraints(dakotaModel, CONSTRAINT_EQUALITY_TYPE::INEQUALITY, v, jv);
  }
}

void
DakotaROLIneqConstraints::applyAdjointJacobian(std::vector<Real> &ajv,
    const std::vector<Real> &v, const std::vector<Real> &x, Real &tol)
{
  // Must init since are merging the effect of linear and nonlinear
  // and linear might get skipped
  ajv.assign(ajv.size(), 0.0);

  // apply linear constraint Jacobian (might be empty)
  const RealMatrix & lin_ineq_coeffs = dakotaModel.linear_ineq_constraint_coeffs();
  apply_matrix_transpose_partial(lin_ineq_coeffs, v, ajv);

  // apply nonlinear constraint Jacobian (might be empty)
  if (haveNlnConst) {
    // makes sure that model is current
    update_model(dakotaModel, x);
    apply_nonlinear_constraints(dakotaModel, CONSTRAINT_EQUALITY_TYPE::INEQUALITY, v, ajv, true);
  }
}

// --------------------------------------------------------------
//               DakotaROLEqConstraints
// --------------------------------------------------------------

DakotaROLEqConstraints::DakotaROLEqConstraints(Model & model) :
  dakotaModel(model)
{
  haveNlnConst = model.num_nonlinear_eq_constraints() > 0;
}

void
DakotaROLEqConstraints::value(std::vector<Real> &c, const std::vector<Real> &x, Real &tol)
{
  update_model(dakotaModel, x);
//PDH: Same comment as for the inequality constraints.  Also, why the -1.0
//when getting the nonlinear constraint values?

  apply_linear_constraints( dakotaModel, CONSTRAINT_EQUALITY_TYPE::EQUALITY, x, c );
  get_nonlinear_eq_constraints( dakotaModel, c, -1.0 );
}

void
DakotaROLEqConstraints::applyJacobian(std::vector<Real> &jv,
    const std::vector<Real> &v, const std::vector<Real> &x, Real &tol)
{
  // apply linear constraint Jacobian
  const RealMatrix & lin_eq_coeffs = dakotaModel.linear_eq_constraint_coeffs();
  apply_matrix_partial(lin_eq_coeffs, v, jv);

  // apply nonlinear constraint Jacobian
  if( haveNlnConst ) {
    update_model(dakotaModel, x);
    apply_nonlinear_constraints(dakotaModel, CONSTRAINT_EQUALITY_TYPE::EQUALITY, v, jv);
  }
}

void
DakotaROLEqConstraints::applyAdjointJacobian(std::vector<Real> &ajv,
    const std::vector<Real> &v, const std::vector<Real> &x, Real &tol)
{
  // Must init since are merging the effect of linear and nonlinear
  // and linear might get skipped
  ajv.assign(ajv.size(), 0.0);

  // apply linear constraint Jacobian (might be empty)
  const RealMatrix & lin_eq_coeffs = dakotaModel.linear_eq_constraint_coeffs();
  apply_matrix_transpose_partial(lin_eq_coeffs, v, ajv);

  // apply nonlinear constraint Jacobian (might be empty)
  if (haveNlnConst) {
    // makes sure that model is current
    update_model(dakotaModel, x);
    apply_nonlinear_constraints(dakotaModel, CONSTRAINT_EQUALITY_TYPE::EQUALITY, v, ajv, true);
  }
}

DakotaROLObjectiveHess::DakotaROLObjectiveHess(Model & model) :
  DakotaROLObjective(model)
{ }

void
DakotaROLObjectiveHess::hessVec(std::vector<Real> &hv,
				const std::vector<Real> &v,
				const std::vector<Real> &x,
				Real &tol)
{
  // make sure the Hessian has been evaluated and is available
  update_model(dakotaModel, x);

  // get the objective Hessian matrix from Dakota Response
  const RealSymMatrix& hess_f = dakotaModel.current_response().function_hessian(0);

  // multiply the objective Hessian matrix by the vector
  apply_matrix_partial(hess_f, v, hv);
}

void
DakotaROLObjectiveHess::invHessVec(std::vector<Real> &h,
				   const std::vector<Real> &v,
				   const std::vector<Real> &x,
				   Real &tol)
{
  Cerr << "\nError: DakotaROLObjectiveHess::invHessVec is not currently supported."
       << "  logic error.\n";
  abort_handler(METHOD_ERROR);
}

} // namespace Dakota
