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

#include <boost/iostreams/filtering_stream.hpp>

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
  // These calls are order-dependent in that the parameter settings depend on problemType
  set_problem();
  set_rol_parameters();
}

/// Alternate constructor for Iterator instantiations by name.

ROLOptimizer::
ROLOptimizer(const String& method_string, Model& model):
  Optimizer(method_string_to_enum(method_string), model, std::shared_ptr<TraitsBase>(new ROLTraits())),
  optSolverParams("Dakota::ROL")
{
  // These calls are order-dependent in that the parameter settings depend on problemType
  set_problem();
  set_rol_parameters();
}


/** This function uses ProblemDescDB and therefore must only be called
    at construct time. */
void ROLOptimizer::set_rol_parameters()
{
  // PRECEDENCE 1: hard-wired default settings

  // HESSIAN TODO: if Dakota Hessian available, don't use "Secant"
  // need to check on if anything else needs to be set.

  optSolverParams.sublist("General").sublist("Secant").
    set("Type", "Limited-Memory BFGS");
  optSolverParams.sublist("General").sublist("Secant").
    set("Use as Hessian", true);

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
    // The default choice of Kelley-Sachs was performing lots of fn evals for smoothing
    optSolverParams.sublist("Step").sublist("Trust Region").set("Subproblem Model", "Coleman-Li");

    if (outputLevel >= VERBOSE_OUTPUT)
      optSolverParams.sublist("Step").sublist("Augmented Lagrangian").
        set("Print Intermediate Optimization History","true");
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

  //PDH: Seems like we should be able to use traits to determine when we
  //need to provide the sum rather than making the user do it.  Also, same
  //comment about class data as below.

  size_t num_eq_const = numLinearEqConstraints + numNonlinearEqConstraints;
  size_t num_ineq_const = numLinearIneqConstraints + numNonlinearIneqConstraints;

  // Obtain ROL problem type
  // Defaults to Type-U, otherwise overwrite
  problemType = TYPE_U;
  if (num_ineq_const > 0)
    problemType = TYPE_EB;
  else{
    if (num_eq_const > 0){
      if (boundConstraintFlag)
        problemType = TYPE_EB;
      else
        problemType = TYPE_E;
    }
    else{
      if (boundConstraintFlag)
        problemType = TYPE_B;
    }
  }

  // HESSIAN TODO: instantiate obj based on whether or not Hessian is
  // provided

  // null defaults for various elements of ROL's simplified interface
  // will be overridden as required
  Teuchos::RCP<DakotaROLObjective> obj = Teuchos::null;
  Teuchos::RCP<ROL::Vector<Real> > x = Teuchos::null;
  Teuchos::RCP<ROL::BoundConstraint<Real> > bnd = Teuchos::null;
  Teuchos::RCP<DakotaROLEqConstraints> eq_const = Teuchos::null;
  Teuchos::RCP<ROL::Vector<Real> > emul = Teuchos::null;
  Teuchos::RCP<DakotaROLIneqConstraints> ineq_const = Teuchos::null;
  Teuchos::RCP<ROL::Vector<Real> > imul = Teuchos::null;
  Teuchos::RCP<ROL::BoundConstraint<Real> > ineq_bnd = Teuchos::null;

//PDH: Should figure out which of this class data (e.g., numContinuousVars)
//we could/should elminate and which we should keep and formally make part
//of the TPL API.

// create ROL variable vector
  rolX.reset(new std::vector<Real>(numContinuousVars, 0.0));
  Teuchos::RCP<std::vector<Real> >
    l_rcp(new std::vector<Real>(numContinuousVars, 0.0));
  Teuchos::RCP<std::vector<Real> >
    u_rcp(new std::vector<Real>(numContinuousVars, 0.0));

  get_initial_values(iteratedModel, *rolX);
  x.reset( new ROL::StdVector<Real>(rolX) );

  // create ROL bound vector; for Type-EB problems, map any Dakota
  // infinite bounds to ROL_INF for best performance
  if (boundConstraintFlag || (problemType == TYPE_EB)){
    Real rol_inf = ROL::ROL_INF<Real>();
    Real rol_ninf = ROL::ROL_NINF<Real>();

    get_bounds(iteratedModel, *l_rcp, *u_rcp);

    // map bounds greater than (in absolute sense) ROL_INF,
    // including any Dakota infinite bounds, to ROL_INF
    for (size_t i = 0; i < numContinuousVars; i++) {
      if ((*l_rcp)[i] < rol_ninf)
        (*l_rcp)[i] = rol_ninf;
      if ((*u_rcp)[i] > rol_inf)
        (*u_rcp)[i] = rol_inf;
    }

    lowerBounds = Teuchos::rcp( new ROL::StdVector<Real>( l_rcp ) );
    upperBounds = Teuchos::rcp( new ROL::StdVector<Real>( u_rcp ) );

    // create ROL::BoundConstraint object to house variable bounds information
    bnd.reset( new ROL::Bounds<Real>(lowerBounds, upperBounds) );
  }

  // HESSIAN TODO: reset obj based on whether or not Hessian is provided

  // create objective function object and give it access to Dakota model 
  obj.reset(new DakotaROLObjective(iteratedModel));

  // Equality constraints
  if (num_eq_const > 0){
    // create equality constraint object and give it access to Dakota model 
    eq_const.reset(new DakotaROLEqConstraints(iteratedModel));

//PDH: What are the multipliers for?

    // equality multipliers
    Teuchos::RCP<std::vector<Real> > emul_rcp = Teuchos::rcp( new std::vector<Real>(num_eq_const,0.0) );
    emul.reset(new ROL::StdVector<Real>(emul_rcp) );
  }

  // Inequality constraints: [linear_ineq, nonlinear_ineq]
  if (num_ineq_const > 0){
    // create inequality constraint object and give it access to Dakota model 
    ineq_const.reset(new DakotaROLIneqConstraints(iteratedModel));

    // inequality multipliers
    Teuchos::RCP<std::vector<Real> > imul_rcp = Teuchos::rcp( new std::vector<Real>(num_ineq_const,0.0) );
    imul.reset(new ROL::StdVector<Real>(imul_rcp) );
  

    // create ROL inequality constraint bound vectors
    Teuchos::RCP<std::vector<Real> >
      ineq_l_rcp(new std::vector<Real>(num_ineq_const, 0.0));
    Teuchos::RCP<std::vector<Real> >
      ineq_u_rcp(new std::vector<Real>(num_ineq_const, 0.0));

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
  optProblem = ROL::OptimizationProblem<Real> (obj, x, bnd, eq_const, emul, ineq_const, imul, ineq_bnd);

  // checking, may be enabled in tests or debug mode

  // Teuchos::RCP<std::ostream> outStream_checking;
  // outStream_checking = Teuchos::rcp(&std::cout, false);
  // optProblem.check(*outStream_checking);
}

void ROLOptimizer::reset_problem( const RealVector & init_vals,
                                  const RealVector & lower_bnds,
                                  const RealVector & upper_bnds,
                                  const Teuchos::ParameterList & params)
{
  copy_data(init_vals, *rolX);
  copy_data(lower_bnds, *(lowerBounds->getVector()));
  copy_data(upper_bnds, *(upperBounds->getVector()));
  optProblem.reset();

  // These changes get used in core_run
  optSolverParams.setParameters(params);
}

/** core_run redefines the Optimizer virtual function to perform
    the optimization using ROL. It first sets up the simplified ROL
    problem data, then executes solve() on the simplified ROL
    solver interface and finally catalogues the results. */
void ROLOptimizer::core_run()
{
  // ostream that will prefix lines with ROL identifier
  boost::iostreams::filtering_ostream rol_cout;
  rol_cout.push(PrefixingLineFilter("ROL: "));
  // Tried to disable buffering so ROL output gets inlined with
  // Dakota's; doesn't work perfectly. Instead, modified ROL to flush()
  //std::streamsize buffer_size = 0;
  //rol_cout.push(Cout, buffer_size);
  rol_cout.push(Cout);

  // Setup and call simplified interface solver object
  ROL::OptimizationSolver<Real> opt_solver( optProblem, optSolverParams );
  opt_solver.solve(rol_cout);

  rol_cout.flush();

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

    // HESSIAN TODO: added AS_HESS; need to track if this creates isses

    ActiveSet eval_set(model.current_response().active_set());
    eval_set.request_values(AS_FUNC+AS_GRAD+AS_HESS);
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

  // HESSIAN TODO: Fill out the details of these

DakotaROLObjectiveHess::DakotaROLObjectiveHess(Model & model) :
  DakotaROLObjective(model)
{ }

void
DakotaROLObjectiveHess::hessVec(std::vector<Real> &hv,
				const std::vector<Real> &v,
				const std::vector<Real> &x,
				Real &tol)
{/*
  // make sure the Hessian has been evaluated and is available
  update_model(dakotaModel, x);

  // get the objective Hessian matrix from Dakota Response
  hess_f = local_response.function_hessian(0);

  // multiply the objective Hessian matrix by the vector
  apply_matrix_partial(hess_f, v, hv);
 */}

void
DakotaROLObjectiveHess::invHessVec(std::vector<Real> &h,
				   const std::vector<Real> &v,
				   const std::vector<Real> &x,
				   Real &tol)
{/*
  // make sure the Hessian has been evaluated and is available
  update_model(dakotaModel, x);

  // get the objective Hessian matrix from Dakota Response
  hess_f = local_response.function_hessian(0);

  // stole this from DakotaLeastSq.cpp; think we want to do something
  // like this; return product in h

  // We are using a formulation where the standard error of the
  // parameter vector is calculated as the square root of the diagonal
  // elements of sigma_sq_hat*inverse(J'J), where J is the matrix
  // of derivatives of the model with respect to the parameters,
  // and J' is the transpose of J.  Insteaad of calculating J'J and
  // explicitly taking the inverse, we are using a QR decomposition,
  // where J=QR, and inv(J'J)= inv((QR)'QR)=inv(R'Q'QR)=inv(R'R)=
  // inv(R)*inv(R').
  // J must be in column order for the Fortran call
  Teuchos::LAPACK<int, Real> la;
  int info;
  int M = numLeastSqTerms;
  int N = numContinuousVars;
  int LDA = numLeastSqTerms;
  double *tau, *work;
  double* Jmatrix = new double[numLeastSqTerms*numContinuousVars];
  
  // With scaling, the iter_resp will potentially contain
  // d(scaled_resp) / d(scaled_params).
  //
  // When parameters are scaled, have to apply the variable
  // transformation Jacobian to get to
  // d(scaled_resp) / d(native_params) =
  //   d(scaled_resp) / d(scaled_params) * d(scaled_params) / d(native_params)

  // envelope to hold the either unscaled or iterator response
  Response ultimate_resp = scaleFlag ? iter_resp.copy() : iter_resp; 
  if (scaleFlag) {
    ScalingModel* scale_model_rep =
      static_cast<ScalingModel*>(scalingModel.model_rep());
    bool unscale_resp = false;
    scale_model_rep->response_modify_s2n(native_vars, iter_resp,
					 ultimate_resp, 0, numLeastSqTerms,
					 unscale_resp);
  }
  const RealMatrix& fn_grads = ultimate_resp.function_gradients_view();

  // BMA: TODO we don't need to transpose this matrix...
  for (int i=0; i<numLeastSqTerms; i++)
    for (int j=0; j<numContinuousVars; j++)
      Jmatrix[(j*numLeastSqTerms)+i]=fn_grads(j,i);

  // This is the QR decomposition, the results are returned in J
  work = new double[N + std::min(M,N)];
  tau = work + N;

  la.GEQRF(M,N,Jmatrix,LDA,tau,work,N,&info);
  bool error_flag = info;
  delete[] work;

  // if you add these three lines right after DGEQRF, then the upper triangular
  // part of Jmatrix will then contain Rinverse

  char uplo = 'U'; // upper triangular
  char unitdiag = 'N'; // non-unit trangular
  la.TRTRI(uplo, unitdiag, N, Jmatrix, LDA, &info); 
  error_flag &= info;

  if (error_flag) {
    Cout << "\nWarning: LAPACK error computing confidence intervals.\n\n";
    return;
  }

  RealVector standard_error(numContinuousVars);
  RealVector diag(numContinuousVars, true);
  for (int i=0; i<numContinuousVars; i++) {
    for (int j=i; j<numContinuousVars; j++)
      diag(i) += Jmatrix[j*numLeastSqTerms+i]*Jmatrix[j*numLeastSqTerms+i];
    standard_error[i] = std::sqrt(diag(i)*sigma_sq_hat);
  }
  delete[] Jmatrix;

 */}

} // namespace Dakota
