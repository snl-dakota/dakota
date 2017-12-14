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

enum {AS_FUNC=1, AS_GRAD=2, AS_HESS=4};
 
using namespace ROL;

// TODO: data transfers cleanup once decide on std vs Teuchos
/** Convenience function to set the variables from ROL into a Dakota Model */
void set_continuous_vars(Teuchos::RCP<const std::vector<Real>> x, Model& model)
{
  size_t num_cv = model.cv();
  for(size_t i=0; i<num_cv; ++i)
    model.continuous_variable((*x)[i], i);
}


template<class Real> 
class DakotaToROLIneqConstraints : public ROL::Constraint<Real> {

private:

  Teuchos::RCP<const std::vector<Real>> getVector( const Vector<Real>& x ) {
    using Teuchos::dyn_cast;
    return dyn_cast<const StdVector<Real>>(x).getVector();
  }

  Teuchos::RCP<std::vector<Real>> getVector( Vector<Real>& x ) {
    using Teuchos::dyn_cast;
    return dyn_cast<StdVector<Real>>(x).getVector();
  }

  // extract model parameters relevant for inequality constraint
  // evaluation
  void extract_model_params() {

    numContinuousVars = iteratedModel.cv();
    num_nln_ineq = iteratedModel.num_nonlinear_ineq_constraints();
    num_lin_ineq = iteratedModel.num_linear_ineq_constraints();

    // BMA: I don't think we need copies at all, could just access
    // model, but for now, assign instead of looping... until discuss with MK
    lin_ineq_coeffs.reshape((int)num_lin_ineq,(int)numContinuousVars);
    lin_ineq_coeffs.assign(iteratedModel.linear_ineq_constraint_coeffs());
  }

  /// Number of continuous variables
  int numContinuousVars;

  /// Shallow copy of the model on which ROL will iterate.
  Model iteratedModel;

  /// Deep copy of linear inequaity coefficients
  RealMatrix lin_ineq_coeffs;

  /// Number of nonlinear inequalities
  size_t num_nln_ineq;

  /// Number of linear inequalities
  size_t num_lin_ineq;

public:

  DakotaToROLIneqConstraints(Model& dakota_model): iteratedModel(dakota_model)
  {
    extract_model_params();
  }

  void value(Vector<Real> &c, const Vector<Real> &x, Real &tol){

    using Teuchos::RCP;

    // Pointer to constraint vector
    RCP<std::vector<Real>> cp = getVector(c);

    // Pointer to optimization vector     
    RCP<const std::vector<Real>> xp = getVector(x);

    for(size_t i=0;i<num_lin_ineq;++i) {
      (*cp)[i] = 0.0;
      for (size_t j=0; j<numContinuousVars; j++)
        (*cp)[i] += lin_ineq_coeffs(i,j) * (*xp)[j];
    }

    if (num_nln_ineq > 0) {

      set_continuous_vars(xp, iteratedModel);

      iteratedModel.evaluate();

      const RealVector& dakota_fns =
	iteratedModel.current_response().function_values();

      for(size_t i=0;i<num_nln_ineq;++i)
        (*cp)[i+num_lin_ineq] = dakota_fns[i+1];

    }
  }

  void applyJacobian(Vector<Real> &jv,
        const Vector<Real> &v, const Vector<Real> &x, Real &tol){

    using Teuchos::RCP;

    // Pointer to optimization vector     
    RCP<const std::vector<Real>> xp = getVector(x);

    // Pointer to jv vector
    RCP<std::vector<Real>> jvp = getVector(jv);

    // apply linear constraint Jacobian
    for(size_t i=0;i<num_lin_ineq;++i) {
      (*jvp)[i] = 0.0;
      for (size_t j=0; j<numContinuousVars; j++)
        (*jvp)[i] += lin_ineq_coeffs(i,j) * (*xp)[j];
    }

    // apply nonlinear constraint Jacobian
    if (num_nln_ineq > 0) {

      set_continuous_vars(getVector(x), iteratedModel);

      // For now, ROL will always evaluate all objectives, constraints,
      // and their gradients with every hard update, so hard-wire the ASV
      // = 1 & 2 = 3
      ActiveSet eval_set(iteratedModel.current_response().active_set());
      eval_set.request_values(AS_GRAD);
      iteratedModel.evaluate(eval_set);

      const RealMatrix& dakota_grads
        = iteratedModel.current_response().function_gradients();

      for(size_t i=0;i<num_nln_ineq;++i){
        (*jvp)[i+num_lin_ineq] = 0.0;
        for (size_t j=0; j<numContinuousVars; j++)
          (*jvp)[i+num_lin_ineq] += dakota_grads(j,i+1) * (*xp)[j];
      }
        
    }

  }

  // provide access to Dakota model
  void pass_model(Model& model) {

    iteratedModel = model;

    extract_model_params();
  }

}; // class DakotaToROLIneqConstraints

template<class Real> 
class DakotaToROLEqConstraints : public ROL::Constraint<Real> {

private:

  Teuchos::RCP<const std::vector<Real>> getVector( const Vector<Real>& x ) {
    using Teuchos::dyn_cast;
    return dyn_cast<const StdVector<Real>>(x).getVector();
  }

  Teuchos::RCP<std::vector<Real>> getVector( Vector<Real>& x ) {
    using Teuchos::dyn_cast;
    return dyn_cast<StdVector<Real>>(x).getVector();
  }

  // extract model parameters relevant for equality constraint
  // evaluation
  void extract_model_params() {

    numContinuousVars = iteratedModel.cv();
    num_nln_eq = iteratedModel.num_nonlinear_eq_constraints();
    num_nln_ineq = iteratedModel.num_nonlinear_ineq_constraints();
    num_lin_eq = iteratedModel.num_linear_eq_constraints();

    // BMA: I don't think we need copies at all, could just access
    // model, but for now, assign instead of looping... until discuss with MK
    lin_eq_coeffs.reshape((int)num_lin_eq,(int)numContinuousVars);
    lin_eq_coeffs.assign(iteratedModel.linear_eq_constraint_coeffs());
    lin_eq_targets.resize((int)num_lin_eq);
    lin_eq_targets.assign(iteratedModel.linear_eq_constraint_targets());
    nln_eq_targets.resize((int)num_nln_eq);
    nln_eq_targets.assign(iteratedModel.nonlinear_eq_constraint_targets());
  }

  /// Number of continuous variables
  int numContinuousVars;

  /// Shallow copy of the model on which ROL will iterate.
  Model iteratedModel;

  /// Deep copy of linear equaity coefficients
  RealMatrix lin_eq_coeffs;

  /// Deep copy of linear equaity targets
  RealVector lin_eq_targets;

  /// Deep copy of nonlinear equaity targets
  RealVector nln_eq_targets;

  /// Number of nonlinear equalities
  size_t num_nln_eq;

  /// Number of nonlinear inequalities
  size_t num_nln_ineq;

  /// Number of linear equalities
  size_t num_lin_eq;

public:

  DakotaToROLEqConstraints(Model& dakota_model): iteratedModel(dakota_model)
  {
    extract_model_params();
  }

  // BMA TODO: don't we now have data adapters that convert linear to
  // nonlinear constraints and manage the indexing?

  void value(Vector<Real> &c, const Vector<Real> &x, Real &tol){

    using Teuchos::RCP;

    // Pointer to constraint vector
    RCP<std::vector<Real>> cp = getVector(c);

    // Pointer to optimization vector     
    RCP<const std::vector<Real>> xp = getVector(x);

    for(size_t i=0;i<num_lin_eq;++i) {
      (*cp)[i] = -lin_eq_targets(i);
      for (size_t j=0; j<numContinuousVars; j++)
        (*cp)[i] += lin_eq_coeffs(i,j) * (*xp)[j];
    }

    if (num_nln_eq > 0) {

      set_continuous_vars(xp, iteratedModel);

      iteratedModel.evaluate();

      const RealVector& dakota_fns
        = iteratedModel.current_response().function_values();

      for(size_t i=0;i<num_nln_eq;++i)
        (*cp)[i+num_lin_eq] = -nln_eq_targets(i)+dakota_fns[i+1+num_nln_ineq];
    }

  }

  void applyJacobian(Vector<Real> &jv,
        const Vector<Real> &v, const Vector<Real> &x, Real &tol){

    using Teuchos::RCP;

    // Pointer to optimization vector     
    RCP<const std::vector<Real>> xp = getVector(x);

    // Pointer to jv vector
    RCP<std::vector<Real>> jvp = getVector(jv);

    // apply linear constraint Jacobian
    for(size_t i=0;i<num_lin_eq;++i) {
      (*jvp)[i] = 0.0;
      for (size_t j=0; j<numContinuousVars; j++)
        (*jvp)[i] += lin_eq_coeffs(i,j) * (*xp)[j];
    }

    // apply nonlinear constraint Jacobian
    if (num_nln_eq > 0) {

      set_continuous_vars(getVector(x), iteratedModel);

      // For now, ROL will always evaluate all objectives, constraints,
      // and their gradients with every hard update, so hard-wire the ASV
      // = 1 & 2 = 3
      ActiveSet eval_set(iteratedModel.current_response().active_set());
      eval_set.request_values(AS_GRAD);
      iteratedModel.evaluate(eval_set);

      const RealMatrix& dakota_grads
        = iteratedModel.current_response().function_gradients();

      for(size_t i=0;i<num_nln_eq;++i){
        (*jvp)[i+num_lin_eq] = 0.0;
        for (size_t j=0; j<numContinuousVars; j++)
          (*jvp)[i+num_lin_eq] += dakota_grads(j,i+1+num_nln_ineq) * (*xp)[j];
      }
        
    }

  }

  // provide access to Dakota model
  void pass_model(Model& model) {

    iteratedModel = model;

    extract_model_params();
  }

}; // class DakotaToROLEqConstraints


template<class Real> 
class DakotaToROLObjective : public ROL::Objective<Real> {

private:

  Teuchos::RCP<const std::vector<Real>> getVector( const Vector<Real>& x ) {
    using Teuchos::dyn_cast;
    return dyn_cast<const StdVector<Real>>(x).getVector();
  }

  Teuchos::RCP<std::vector<Real>> getVector( Vector<Real>& x ) {
    using Teuchos::dyn_cast;
    return dyn_cast<StdVector<Real>>(x).getVector();
  }

  // extract model parameters relevant for objective
  // function evaluation
  void extract_model_params() {
    numContinuousVars = iteratedModel.cv();
  }

  /// Number of continuous variables
  int numContinuousVars;

  /// Shallow copy of the model on which ROL will iterate.
  Model iteratedModel;

public:

  DakotaToROLObjective(Model& dakota_model): iteratedModel(dakota_model)
  {
    extract_model_params();
  }

  Real value(const Vector<Real> &x, Real &tol) {

    set_continuous_vars(getVector(x), iteratedModel);

    iteratedModel.evaluate();

    Real fn_val = iteratedModel.current_response().function_value(0);

    return fn_val;
  }

  void gradient( Vector<Real> &g, const Vector<Real> &x, Real &tol ) {

    set_continuous_vars(getVector(x), iteratedModel);

    // For now, ROL will always evaluate all objectives, constraints,
    // and their gradients with every hard update, so hard-wire the ASV
    // = 1 & 2 = 3
    ActiveSet eval_set(iteratedModel.current_response().active_set());
    eval_set.request_values(AS_GRAD);
    iteratedModel.evaluate(eval_set);

    const RealMatrix& dakota_grads
      = iteratedModel.current_response().function_gradients();
   
    Teuchos::RCP<std::vector<Real>> gp = getVector(g);
    for (int i=0; i<numContinuousVars; ++i)
      (*gp)[i] = dakota_grads(i, 0);

  }

  // provide access to Dakota model
  void pass_model(Model& model) {

    iteratedModel = model;

    extract_model_params();
  }

}; // class DakotaToROLObjective

/// Standard constructor.

ROLOptimizer::ROLOptimizer(ProblemDescDB& problem_db, Model& model):
  Optimizer(problem_db, model, std::shared_ptr<TraitsBase>(new ROLTraits())),
  optSolverParams("Dakota::ROL")
{
  set_rol_parameters();

  set_problem();
}

/// Alternate constructor for Iterator instantiations by name.

ROLOptimizer::
ROLOptimizer(const String& method_string, Model& model):
  Optimizer(method_string_to_enum(method_string), model, std::shared_ptr<TraitsBase>(new ROLTraits())),
  optSolverParams("Dakota::ROL")
{
  set_rol_parameters();

  set_problem();
}


/** This function uses ProblemDescDB and therefore must only be called
    at construct time. */
void ROLOptimizer::set_rol_parameters()
{
  // PRECEDENCE 1: hard-wired default settings

  // (we only support line search for now)
  optSolverParams.sublist("Step").set("Type","Line Search");


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
void ROLOptimizer::set_problem() {
  typedef double RealT;
  size_t j;

  // null defaults for various elements of ROL's simplified interface
  // will be overridden as required
  Teuchos::RCP<DakotaToROLObjective<RealT> > obj = Teuchos::null;
  Teuchos::RCP<ROL::Vector<RealT> > x = Teuchos::null;
  Teuchos::RCP<ROL::BoundConstraint<RealT> > bnd = Teuchos::null;
  Teuchos::RCP<DakotaToROLEqConstraints<RealT> > eqConst = Teuchos::null;
  Teuchos::RCP<ROL::Vector<RealT> > emul = Teuchos::null;
  Teuchos::RCP<DakotaToROLIneqConstraints<RealT> > ineqConst = Teuchos::null;
  Teuchos::RCP<ROL::Vector<RealT> > imul = Teuchos::null;
  Teuchos::RCP<ROL::BoundConstraint<RealT> > ineq_bnd = Teuchos::null;

  // Extract Dakota variable and bound vectors
  const RealVector& initial_points = iteratedModel.continuous_variables();
  const RealVector& c_l_bnds = iteratedModel.continuous_lower_bounds();
  const RealVector& c_u_bnds = iteratedModel.continuous_upper_bounds();

  // create ROL variable and bound vectors
  rolX.reset(new std::vector<RealT>(numContinuousVars, 0.0));
  Teuchos::RCP<std::vector<RealT> >
    l_rcp(new std::vector<RealT>(numContinuousVars, 0.0));
  Teuchos::RCP<std::vector<RealT> >
    u_rcp(new std::vector<RealT>(numContinuousVars, 0.0));

  // BMA: left this loop for data transfers consolidation
  for(j=0; j<numContinuousVars; j++){
    rolX->operator[](j) = initial_points[j];
    l_rcp->operator[](j) = c_l_bnds[j];
    u_rcp->operator[](j) = c_u_bnds[j];
  }

  x.reset( new ROL::StdVector<RealT>(rolX) );
  Teuchos::RCP<ROL::Vector<RealT> > lower( new ROL::StdVector<RealT>( l_rcp ) );
  Teuchos::RCP<ROL::Vector<RealT> > upper( new ROL::StdVector<RealT>( u_rcp ) );

  // create ROL::BoundConstraint object to house variable bounds information
  bnd.reset( new ROL::Bounds<RealT>(lower,upper) );

  // create objective function object and give it access to Dakota model 
  obj.reset(new DakotaToROLObjective<RealT>(iteratedModel));

  size_t numEqConstraints = numLinearEqConstraints + numNonlinearEqConstraints;
  size_t numIneqConstraints = numLinearIneqConstraints + numNonlinearIneqConstraints;

  // Equality constraints
  if (numEqConstraints > 0){
    // create equality constraint object and give it access to Dakota model 
    eqConst.reset(new DakotaToROLEqConstraints<RealT>(iteratedModel));

    // equality multipliers
    Teuchos::RCP<std::vector<RealT> > emul_rcp = Teuchos::rcp( new std::vector<RealT>(numEqConstraints,0.0) );
    emul.reset(new ROL::StdVector<RealT>(emul_rcp) );
  }

  // Inequality constraints
  if (numIneqConstraints > 0){
    // create inequality constraint object and give it access to Dakota model 
    ineqConst.reset(new DakotaToROLIneqConstraints<RealT>(iteratedModel));

    // inequality multipliers
    Teuchos::RCP<std::vector<RealT> > imul_rcp = Teuchos::rcp( new std::vector<RealT>(numIneqConstraints,0.0) );
    imul.reset(new ROL::StdVector<RealT>(imul_rcp) );
  

    // create ROL inequality constraint bound vectors
    Teuchos::RCP<std::vector<RealT> >
      ineq_l_rcp(new std::vector<RealT>(numIneqConstraints, 0.0));
    Teuchos::RCP<std::vector<RealT> >
      ineq_u_rcp(new std::vector<RealT>(numIneqConstraints, 0.0));

    if (numLinearIneqConstraints){
      const RealVector& lin_eq_lwr_bnds = iteratedModel.linear_ineq_constraint_lower_bounds();
      const RealVector& lin_eq_upr_bnds = iteratedModel.linear_ineq_constraint_upper_bounds();

      for(j=0; j<numLinearIneqConstraints; j++){
        ineq_l_rcp->operator[](j) = lin_eq_lwr_bnds[j];
        ineq_u_rcp->operator[](j) = lin_eq_upr_bnds[j];
      }
    }
    if (numNonlinearIneqConstraints){
      const RealVector& nln_eq_lwr_bnds = iteratedModel.nonlinear_ineq_constraint_lower_bounds();
      const RealVector& nln_eq_upr_bnds = iteratedModel.nonlinear_ineq_constraint_upper_bounds();

      for(j=0; j<numNonlinearIneqConstraints; j++){
        ineq_l_rcp->operator[](j+numLinearIneqConstraints) = nln_eq_lwr_bnds[j];
        ineq_u_rcp->operator[](j+numLinearIneqConstraints) = nln_eq_upr_bnds[j];
      }
    }

    Teuchos::RCP<ROL::Vector<RealT> > ineq_lower_bounds( new ROL::StdVector<RealT>( ineq_l_rcp ) );
    Teuchos::RCP<ROL::Vector<RealT> > ineq_upper_bounds( new ROL::StdVector<RealT>( ineq_u_rcp ) );

    // create ROL::BoundConstraint object to house variable bounds information
    ineq_bnd.reset( new ROL::Bounds<RealT>(ineq_lower_bounds,ineq_upper_bounds) );
  }

  // Call simplified interface problem generator
  optProblem = ROL::OptimizationProblem<RealT> (obj, x, bnd, eqConst, emul, ineqConst, imul, ineq_bnd);

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

  // copy ROL solution to Dakota bestVariablesArray
  Variables& best_vars = bestVariablesArray.front();
  RealVector& cont_vars = best_vars.continuous_variables_view();
  // TODO: data transfers consolidation: copy_data(rolX, cont_vars)
  for(int j=0; j<numContinuousVars; j++)
    cont_vars[j] = (*rolX)[j];

  // Attempt DB lookup directly into best, fallback on re-evaluation if needed
  Response& best_resp = bestResponseArray.front();
  ActiveSet search_set(best_resp.active_set());
  enum {AS_FUNC=1, AS_GRAD=2, AS_HESS=4};
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


} // namespace Dakota
