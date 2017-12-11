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
 
using namespace ROL;

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

    const RealMatrix& lin_ineq_coeffs_temp
        = iteratedModel.linear_ineq_constraint_coeffs();

    lin_ineq_coeffs.reshape((int)num_lin_ineq,(int)numContinuousVars);
    for(size_t i=0;i<num_lin_ineq;++i) {
      for (size_t j=0; j<numContinuousVars; j++)
        lin_ineq_coeffs(i,j) = lin_ineq_coeffs_temp(i,j);
    }
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

  DakotaToROLIneqConstraints() {}

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

    if (num_nln_ineq){

      RealVector act_cont_vars(numContinuousVars, false);

      for(size_t i=0; i<numContinuousVars; i++){
        act_cont_vars[i] = (*xp)[i];
      }
      
      iteratedModel.continuous_variables(act_cont_vars);

      iteratedModel.evaluate();

      const RealVector& dakota_fns
          = iteratedModel.current_response().function_values();

      for(size_t i=0;i<num_nln_ineq;++i)
        (*cp)[i+num_lin_ineq] = dakota_fns[i+1];

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

    const RealMatrix& lin_eq_coeffs_temp
        = iteratedModel.linear_eq_constraint_coeffs();

    lin_eq_coeffs.reshape((int)num_lin_eq,(int)numContinuousVars);
    for(size_t i=0;i<num_lin_eq;++i) {
      for (size_t j=0; j<numContinuousVars; j++)
        lin_eq_coeffs(i,j) = lin_eq_coeffs_temp(i,j);
    }

    const RealVector& lin_eq_targets_temp
        = iteratedModel.linear_eq_constraint_targets();

    lin_eq_targets.resize((int)num_lin_eq);
    for(size_t i=0;i<num_lin_eq;++i)
        lin_eq_targets(i) = lin_eq_targets_temp(i);

    const RealVector& nln_eq_targets_temp
        = iteratedModel.nonlinear_eq_constraint_targets();

    nln_eq_targets.resize((int)num_nln_eq);
    for(size_t i=0;i<num_nln_eq;++i)
        nln_eq_targets(i) = nln_eq_targets_temp(i);
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

  DakotaToROLEqConstraints() {}

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

    RealVector act_cont_vars(numContinuousVars, false);

    for(size_t i=0; i<numContinuousVars; i++){
      act_cont_vars[i] = (*xp)[i];
    }
    
    iteratedModel.continuous_variables(act_cont_vars);

    iteratedModel.evaluate();

    const RealVector& dakota_fns
        = iteratedModel.current_response().function_values();

    for(size_t i=0;i<num_nln_eq;++i)
      (*cp)[i+num_lin_eq] = -nln_eq_targets(i)+dakota_fns[i+1+num_nln_ineq];

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

  DakotaToROLObjective() {}

  Real value(const Vector<Real> &x, Real &tol) {

    using Teuchos::RCP;   

    // Pointer to opt vector 
    RCP<const std::vector<Real>> xp = getVector(x); 

    RealVector act_cont_vars(numContinuousVars, false);

    for(size_t i=0; i<numContinuousVars; i++){
      act_cont_vars[i] = (*xp)[i];
    }
    
    iteratedModel.continuous_variables(act_cont_vars);

    iteratedModel.evaluate();

    Real fn_val = iteratedModel.current_response().function_value(0);

    return fn_val;
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

  // Extract Dakota variable and bound vectors
  const RealVector& initial_points = iteratedModel.continuous_variables();
  const RealVector& c_l_bnds = iteratedModel.continuous_lower_bounds();
  const RealVector& c_u_bnds = iteratedModel.continuous_upper_bounds();

  // create ROL variable and bound vectors
  x_rcp.reset(new std::vector<RealT>(numContinuousVars, 0.0));
  Teuchos::RCP<std::vector<RealT> >
    l_rcp(new std::vector<RealT>(numContinuousVars, 0.0));
  Teuchos::RCP<std::vector<RealT> >
    u_rcp(new std::vector<RealT>(numContinuousVars, 0.0));

  // BMA: left this loop for data transfers consolidation
  for(j=0; j<numContinuousVars; j++){
    x_rcp->operator[](j) = initial_points[j];
    l_rcp->operator[](j) = c_l_bnds[j];
    u_rcp->operator[](j) = c_u_bnds[j];
  }

  Teuchos::RCP<ROL::Vector<RealT> > x( new ROL::StdVector<RealT>(x_rcp) );
  Teuchos::RCP<ROL::Vector<RealT> > lower( new ROL::StdVector<RealT>( l_rcp ) );
  Teuchos::RCP<ROL::Vector<RealT> > upper( new ROL::StdVector<RealT>( u_rcp ) );

  // create ROL::BoundConstraint object to house variable bounds information
  Teuchos::RCP<ROL::BoundConstraint<RealT> > bnd( new ROL::Bounds<RealT>(lower,upper) );

  // create objective function object and give it access to Dakota model 
  Teuchos::RCP<DakotaToROLObjective<RealT> > obj(new DakotaToROLObjective<RealT>());
  obj->pass_model(iteratedModel);

  size_t numEqConstraints = numLinearEqConstraints + numNonlinearEqConstraints;
  size_t numIneqConstraints = numLinearIneqConstraints + numNonlinearIneqConstraints;

  // No constraints
  if ((numEqConstraints == 0) && (numIneqConstraints == 0)){
    // Call simplified interface problem generator
    problem = ROL::OptimizationProblem<RealT> ( obj, x, bnd);  

  }
  // No inequality constraints
  else if ((numEqConstraints > 0) && (numIneqConstraints == 0)){
    // create equality constraint object and give it access to Dakota model 
    Teuchos::RCP<DakotaToROLEqConstraints<RealT> > eqConst(new DakotaToROLEqConstraints<RealT>());
    eqConst->pass_model(iteratedModel);

    // equality multipliers
    Teuchos::RCP<std::vector<RealT> > emul_rcp = Teuchos::rcp( new std::vector<RealT>(numEqConstraints,0.0) );
    Teuchos::RCP<ROL::Vector<RealT> > emul = Teuchos::rcp( new ROL::StdVector<RealT>(emul_rcp) );
  
    // Call simplified interface problem generator
    problem = ROL::OptimizationProblem<RealT> ( obj, x, bnd, eqConst, emul); 
  }
  // No equality constraints
  else if ((numEqConstraints == 0) && (numIneqConstraints > 0)){
    // create inequality constraint object and give it access to Dakota model 
    Teuchos::RCP<DakotaToROLIneqConstraints<RealT> > ineqConst(new DakotaToROLIneqConstraints<RealT>());
    ineqConst->pass_model(iteratedModel);

    // inequality multipliers
    Teuchos::RCP<std::vector<RealT> > imul_rcp = Teuchos::rcp( new std::vector<RealT>(numIneqConstraints,0.0) );
    Teuchos::RCP<ROL::Vector<RealT> > imul = Teuchos::rcp( new ROL::StdVector<RealT>(imul_rcp) );
  

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
    Teuchos::RCP<ROL::BoundConstraint<RealT> > ineq_bnd( new ROL::Bounds<RealT>(ineq_lower_bounds,ineq_upper_bounds) );


    // Call simplified interface problem generator
    problem = ROL::OptimizationProblem<RealT> ( obj, x, bnd, ineqConst, imul,ineq_bnd); 
  }
  // Equality and inequality constraints
  else {
    // create equality constraint object and give it access to Dakota model 
    Teuchos::RCP<DakotaToROLEqConstraints<RealT> > eqConst(new DakotaToROLEqConstraints<RealT>());
    eqConst->pass_model(iteratedModel);

    // equality multipliers
    Teuchos::RCP<std::vector<RealT> > emul_rcp = Teuchos::rcp( new std::vector<RealT>(numEqConstraints,0.0) );
    Teuchos::RCP<ROL::Vector<RealT> > emul = Teuchos::rcp( new ROL::StdVector<RealT>(emul_rcp) );
  
    // create inequality constraint object and give it access to Dakota model 
    Teuchos::RCP<DakotaToROLIneqConstraints<RealT> > ineqConst(new DakotaToROLIneqConstraints<RealT>());
    ineqConst->pass_model(iteratedModel);

    // inequality multipliers
    Teuchos::RCP<std::vector<RealT> > imul_rcp = Teuchos::rcp( new std::vector<RealT>(numIneqConstraints,0.0) );
    Teuchos::RCP<ROL::Vector<RealT> > imul = Teuchos::rcp( new ROL::StdVector<RealT>(imul_rcp) );
  

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
    Teuchos::RCP<ROL::BoundConstraint<RealT> > ineq_bnd( new ROL::Bounds<RealT>(ineq_lower_bounds,ineq_upper_bounds) );


    // Call simplified interface problem generator
    problem = ROL::OptimizationProblem<RealT> ( obj, x, bnd, eqConst, emul, ineqConst, imul,ineq_bnd); 
  }


  // checking, may be enabled in tests or debug mode

  // Teuchos::RCP<std::ostream> outStream_checking;
  // outStream_checking = Teuchos::rcp(&std::cout, false);
  // problem.check(*outStream_checking);
}


/** core_run redefines the Optimizer virtual function to perform
    the optimization using ROL. It first sets up the simplified ROL
    problem data, then executes solve() on the simplified ROL
    solver interface and finally catalogues the results. */

void ROLOptimizer::core_run()
{
  typedef double RealT;
  size_t j;

  // Simplified interface solver object
  ROL::OptimizationSolver<RealT> solver( problem, optSolverParams );

  // Print iterates to screen, need to control using Dakota output keyword
  Teuchos::RCP<std::ostream> outStream;
  outStream = Teuchos::rcp(&std::cout, false);

  // Call simplified interface solver
  solver.solve(*outStream); 

  // copy ROL solution to Dakota bestVariablesArray
  RealVector contVars(numContinuousVars);
  for(j=0; j<numContinuousVars; j++){
    contVars[j] = (*x_rcp)[j];
  }
  bestVariablesArray.front().continuous_variables(contVars);

  // Evaluate model for responses at best parameters
  RealVector best_fns(iteratedModel.num_functions());
  iteratedModel.continuous_variables(contVars);
  iteratedModel.evaluate();
  // push best responses through Dakota bestResponseArray
  best_fns = iteratedModel.current_response().function_values();
  bestResponseArray.front().function_values(best_fns);
}

} // namespace Dakota
