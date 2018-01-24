/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       ROLOptimizer
//- Description: Implementation of the ROLOptimizer class, Only able to
//               call line search optimization for now (i.e. ROLOptimizer
//               is single-method TPL for this iterations)
//- Owner:       Moe Khalil
//- Checked by:
//- Version: $Id$

// BMA TODO: Traits should indicate that ROL requires gradients

#include "Teuchos_XMLParameterListHelpers.hpp"
// BMA TODO: Above will break with newer Teuchos; instead need 
//#include "Teuchos_XMLParameterListCoreHelpers.hpp"
#include "Teuchos_StandardCatchMacros.hpp"

#include "ROL_StdVector.hpp"
#include "ROL_Algorithm.hpp"
#include "ROL_Objective.hpp"
#include "ROL_Constraint.hpp"
#include "ROL_Bounds.hpp"

#include "ROL_OptimizationSolver.hpp"

#include "ROLOptimizer.hpp"
#include "ProblemDescDB.hpp"


using std::endl;

//
// - ROLOptimizer implementation
//

namespace Dakota {

/// Standard constructor.

ROLOptimizer::ROLOptimizer(ProblemDescDB& problem_db, Model& model):
  Optimizer(problem_db, model, std::shared_ptr<TraitsBase>(new ROLTraits())),
  optSolverParams("Dakota::ROL")
{
  set_problem();

  set_rol_parameters();
}

/// Alternate constructor for Iterator instantiations by name.

ROLOptimizer::
ROLOptimizer(const String& method_string, Model& model):
  Optimizer(method_string_to_enum(method_string), model, std::shared_ptr<TraitsBase>(new ROLTraits())),
  optSolverParams("Dakota::ROL")
{
  set_problem();

  set_rol_parameters();
}


/** This function uses ProblemDescDB and therefore must only be called
    at construct time. */
void ROLOptimizer::set_rol_parameters()
{
  // PRECEDENCE 1: hard-wired default settings

  if (problemType == TYPE_U)
  {
    optSolverParams.sublist("Step").set("Type","Trust Region");
    optSolverParams.sublist("Step").sublist("Trust Region").set("Subproblem Solver", "Truncated CG");
  }
  if (problemType == TYPE_B)
  {
    optSolverParams.sublist("Step").set("Type","Trust Region");
    optSolverParams.sublist("Step").sublist("Trust Region").set("Subproblem Solver", "Truncated CG");
  }
  if (problemType == TYPE_E)
  {
    optSolverParams.sublist("Step").set("Type","Composite Step");
  }
  if (problemType == TYPE_EB)
  {
    optSolverParams.sublist("Step").set("Type","Augmented Lagrangian");
    optSolverParams.sublist("Step").sublist("Trust Region").set("Subproblem Solver", "Truncated CG");
  }

  // PRECEDENCE 2: Dakota input file settings

  optSolverParams.sublist("General").
    set("Print Verbosity", outputLevel < VERBOSE_OUTPUT ? 0 : 1);
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

// need to move functionality from core_run below here.
void ROLOptimizer::set_problem()
{
  size_t j;

  // null defaults for various elements of ROL's simplified interface
  // will be overridden as required
  Teuchos::RCP<DakotaROLObjective> obj = Teuchos::null;
  Teuchos::RCP<ROL::Vector<Real> > x = Teuchos::null;
  Teuchos::RCP<ROL::BoundConstraint<Real> > bnd = Teuchos::null;
  Teuchos::RCP<DakotaROLEqConstraints> eqConst = Teuchos::null;
  Teuchos::RCP<ROL::Vector<Real> > emul = Teuchos::null;
  Teuchos::RCP<DakotaROLIneqConstraints> ineqConst = Teuchos::null;
  Teuchos::RCP<ROL::Vector<Real> > imul = Teuchos::null;
  Teuchos::RCP<ROL::BoundConstraint<Real> > ineq_bnd = Teuchos::null;

//PDH: Should figure out which of this class data (e.g., numContinuousVars)
//we could/should elminate and which we should keep and formall make part
//of the TPL API.

  // create ROL variable and bound vectors
  rolX.reset(new std::vector<Real>(numContinuousVars, 0.0));
  Teuchos::RCP<std::vector<Real> >
    l_rcp(new std::vector<Real>(numContinuousVars, 0.0));
  Teuchos::RCP<std::vector<Real> >
    u_rcp(new std::vector<Real>(numContinuousVars, 0.0));

  get_initial_values(iteratedModel, *rolX);
  x.reset( new ROL::StdVector<Real>(rolX) );

  if (boundConstraintFlag){
    get_bounds(iteratedModel, *l_rcp, *u_rcp);
    Teuchos::RCP<ROL::Vector<Real> > lower( new ROL::StdVector<Real>( l_rcp ) );
    Teuchos::RCP<ROL::Vector<Real> > upper( new ROL::StdVector<Real>( u_rcp ) );

    // create ROL::BoundConstraint object to house variable bounds information
    bnd.reset( new ROL::Bounds<Real>(lower,upper) );
  }

  // create objective function object and give it access to Dakota model 
  obj.reset(new DakotaROLObjective(iteratedModel));

//PDH: Seems like we should be able to use traits to determine when we
//need to provide the sum rather than making the user do it.  Also, same
//comment about class data as above.

  size_t numEqConstraints = numLinearEqConstraints + numNonlinearEqConstraints;
  size_t numIneqConstraints = numLinearIneqConstraints + numNonlinearIneqConstraints;

  // Equality constraints
  if (numEqConstraints > 0){
    // create equality constraint object and give it access to Dakota model 
    eqConst.reset(new DakotaROLEqConstraints(iteratedModel));

//PDH: What are the multipliers for?

    // equality multipliers
    Teuchos::RCP<std::vector<Real> > emul_rcp = Teuchos::rcp( new std::vector<Real>(numEqConstraints,0.0) );
    emul.reset(new ROL::StdVector<Real>(emul_rcp) );
  }

  // Inequality constraints: [linear_ineq, nonlinear_ineq]
  if (numIneqConstraints > 0){
    // create inequality constraint object and give it access to Dakota model 
    ineqConst.reset(new DakotaROLIneqConstraints(iteratedModel));

    // inequality multipliers
    Teuchos::RCP<std::vector<Real> > imul_rcp = Teuchos::rcp( new std::vector<Real>(numIneqConstraints,0.0) );
    imul.reset(new ROL::StdVector<Real>(imul_rcp) );
  

    // create ROL inequality constraint bound vectors
    Teuchos::RCP<std::vector<Real> >
      ineq_l_rcp(new std::vector<Real>(numIneqConstraints, 0.0));
    Teuchos::RCP<std::vector<Real> >
      ineq_u_rcp(new std::vector<Real>(numIneqConstraints, 0.0));

//PDH: Seems like we should be able to pull this into the adapters and
//use traits to determine what to populate the bounds vectors with.

    // copy all to partial (should be no-op if no data to copy)
    copy_data_partial(iteratedModel.linear_ineq_constraint_lower_bounds(),
		      *ineq_l_rcp, 0);
    copy_data_partial(iteratedModel.linear_ineq_constraint_upper_bounds(),
		      *ineq_u_rcp, 0);
    copy_data_partial(iteratedModel.nonlinear_ineq_constraint_lower_bounds(),
		      *ineq_l_rcp, numLinearIneqConstraints);
    copy_data_partial(iteratedModel.nonlinear_ineq_constraint_upper_bounds(),
		      *ineq_u_rcp, numLinearIneqConstraints);

    Teuchos::RCP<ROL::Vector<Real> > ineq_lower_bounds( new ROL::StdVector<Real>( ineq_l_rcp ) );
    Teuchos::RCP<ROL::Vector<Real> > ineq_upper_bounds( new ROL::StdVector<Real>( ineq_u_rcp ) );

    // create ROL::BoundConstraint object to house variable bounds information
    ineq_bnd.reset( new ROL::Bounds<Real>(ineq_lower_bounds,ineq_upper_bounds) );
  }

  // Call simplified interface problem generator
  optProblem = ROL::OptimizationProblem<Real> (obj, x, bnd, eqConst, emul, ineqConst, imul, ineq_bnd);

  // Obtain ROL problem type
  // Defaults to Type-U, otherwise overwrite
  problemType = TYPE_U;
  if (numIneqConstraints > 0)
    problemType = TYPE_EB;
  else
  {
    if (numEqConstraints > 0)
    {
      if (boundConstraintFlag)
        problemType = TYPE_EB;
      else
        problemType = TYPE_E;
    }
    else
    {
      if (boundConstraintFlag)
        problemType = TYPE_B;
    }
  }

  // checking, may be enabled in tests or debug mode

  // Teuchos::RCP<std::ostream> outStream_checking;
  // outStream_checking = Teuchos::rcp(&std::cout, false);
  // optProblem.check(*outStream_checking);
}


/** core_run redefines the Optimizer virtual function to perform
    the optimization using ROL. It first sets up the simplified ROL
    problem data, then executes solve() on the simplified ROL
    solver interface and finally catalogues the results. */
void ROLOptimizer::core_run()
{
  // Setup and call simplified interface solver object
  ROL::OptimizationSolver<Real> opt_solver( optProblem, optSolverParams );
  opt_solver.solve(Cout);

  // TODO: print termination criteria (based on Step or AlgorithmState?)

//PDH: If memory serves me correctly, Russell implementd a function at the
//DakotaOptimizer level that sets all the final values.  It was based on
//APPS, but we should revisit to figure out how this compares and if it
//makes sense to merge them.

  // copy ROL solution to Dakota bestVariablesArray
  Variables& best_vars = bestVariablesArray.front();
  RealVector& cont_vars = best_vars.continuous_variables_view();
  copy_data(*rolX, cont_vars);

  // Attempt DB lookup directly into best, fallback on re-evaluation if needed
  Response& best_resp = bestResponseArray.front();
  ActiveSet search_set(best_resp.active_set());
  search_set.request_values(AS_FUNC);
  best_resp.active_set(search_set);
  bool db_found = iteratedModel.db_lookup(best_vars, search_set, best_resp);
  if (db_found)
    Cout << "INFO: ROL retrieved best response from cache." << std::endl;
  else {
    Cout << "INFO: ROL re-evaluating model to retrieve best response."
	 << std::endl;
    // Evaluate model for responses at best parameters
    iteratedModel.continuous_variables(cont_vars);
    iteratedModel.evaluate();
    // push best responses through Dakota bestResponseArray
    const RealVector& best_fns =
      iteratedModel.current_response().function_values();
    best_resp.function_values(best_fns);
  }
}

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

    ActiveSet eval_set(model.current_response().active_set());
    eval_set.request_values(AS_FUNC+AS_GRAD);
    model.evaluate(eval_set);

    // now we can use the response currently in the model for any
    // obj/cons/grad/hess
  }

} // namespace anonymous

// --------------------------------------------------------------
//    These classes could go into a new file for evaluators along
//    the lines of APPS and COLIN if desirable.
// --------------------------------------------------------------

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
  apply_matrix(lin_ineq_coeffs, v, jv);

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
  // BMA --> RWH: Not sure how you'd prefer to handler this...
  // Must init since are merging the effect of linear and nonlinear
  // and linear might get skipped
  ajv.assign(ajv.size(), 0.0);

  // apply linear constraint Jacobian (might be empty)
  const RealMatrix & lin_ineq_coeffs = dakotaModel.linear_ineq_constraint_coeffs();
  apply_matrix_transpose(lin_ineq_coeffs, v, ajv);

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
  apply_matrix(lin_eq_coeffs, v, jv);

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
  // BMA --> RWH: Not sure how you'd prefer to handler this...
  // Must init since are merging the effect of linear and nonlinear
  // and linear might get skipped
  ajv.assign(ajv.size(), 0.0);

  // apply linear constraint Jacobian (might be empty)
  const RealMatrix & lin_eq_coeffs = dakotaModel.linear_eq_constraint_coeffs();
  apply_matrix_transpose(lin_eq_coeffs, v, ajv);

  // apply nonlinear constraint Jacobian (might be empty)
  if (haveNlnConst) {
    // makes sure that model is current
    update_model(dakotaModel, x);
    apply_nonlinear_constraints(dakotaModel, CONSTRAINT_EQUALITY_TYPE::EQUALITY, v, ajv, true);
  }
}


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

} // namespace Dakota
