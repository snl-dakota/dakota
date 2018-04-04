/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef OPT_TPL_ROL_TEST_INTERFACE_HPP
#define OPT_TPL_ROL_TEST_INTERFACE_HPP

#include "DirectApplicInterface.hpp"
#include "ROL_ParaboloidCircle.hpp"
#include "ROL_SimpleEqConstrained.hpp"
//#include "ROL_HS29.hpp"
//#include "ROL_HS32.hpp"

// BMA TODO: remove this
using namespace Dakota;

/// A Dakota testing Interface to ROL optimization test problems (currently zoo)
class RolTestInterface: public Dakota::DirectApplicInterface
{
public:

  /// constructor, accepting name of ROL problem to construct
  RolTestInterface(const std::string& problem_name,
		     const Dakota::ProblemDescDB& problem_db);

  ~RolTestInterface() {  /* empty dtor */  }

  /// execute an analysis code portion of a direct evaluation invocation
  int derived_map_ac(const Dakota::String& ac_name) override;

  /// number of optimization variables
  int numVars = 0;
  /// number of inequality constraints
  int numIneqCons = 0;
  /// number of equality constraints
  int numEqCons = 0;

  /// ROL's suggested initial point
  Teuchos::RCP<std::vector<Real>> xInitial{new std::vector<Real>()};
  /// ROL's known optimal solution
  Teuchos::RCP<std::vector<Real>> xOptimal{new std::vector<Real>()};
  /// optimal function values (empty if require evaluation)
  Teuchos::RCP<std::vector<Real>> optimalFns;

protected:

  /// evaluate any ROL objectives and constraints, populating fnVals, fnGrads
  void evaluate_rol_fns();

  /// convenience function to load ROL simple equality constrained test
  void init_simple_eq_cons();
  /// convenience function to load ROL paraboloid circle test
  void init_paraboloid_circle();

  /// convenience function to evaluate ROL functions at best known x
  /// when the test problem doesn't provide best fns/constraints.
  void evaluate_optimal_solution();

  /// handle to the ROL objective evaluator
  Teuchos::RCP<ROL::Objective<Real>> rolObj;
  /// handle to the ROL inequality evaluator
  Teuchos::RCP<ROL::Constraint<Real>> rolIneqCon;
  /// handle to the ROL equality evaluator
  Teuchos::RCP<ROL::Constraint<Real>> rolEqCon;

};


RolTestInterface::
RolTestInterface(const std::string& problem_name,
		   const Dakota::ProblemDescDB& problem_db):
  Dakota::DirectApplicInterface(problem_db)
{
  numAnalysisServers = 1;

  if (problem_name == "simple_eq_cons")
    init_simple_eq_cons();
  else if (problem_name == "paraboloid_circle")
    init_paraboloid_circle();
  else {
    Cerr << "\nError: Unknonw ROL test problem '" << problem_name << "'\n";
    Dakota::abort_handler(INTERFACE_ERROR);
  }
}


int RolTestInterface::derived_map_ac(const Dakota::String& ac_name)
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: plugin serial direct fn does not support multiprocessor "
	 << "analyses." << std::endl;
    Dakota::abort_handler(Dakota::INTERFACE_ERROR);
  }
  if (ac_name != "rol_testers") {
    Cerr << ac_name << " is not available as an analysis within "
	 << "RolTestInterface." << std::endl;
    Dakota::abort_handler(Dakota::INTERFACE_ERROR);
  }

  try {
    evaluate_rol_fns();
  }
  catch (...) {
    // Failure capturing
    std::string err_msg("Error evaluating plugin analysis_driver ");
    err_msg += ac_name;
    throw Dakota::FunctionEvalFailure(err_msg);
  }

  return 0;
}



void RolTestInterface::init_simple_eq_cons()
{
  numVars = 5;
  numEqCons = 3;

  ROL::StdVector<Real> x0(xInitial);
  ROL::StdVector<Real> sol(xOptimal);

  ROL::ZOO::getSimpleEqConstrained
    < Real, ROL::StdVector<Real>, ROL::StdVector<Real>, ROL::StdVector<Real>,
      ROL::StdVector<Real> > PC;
  rolObj = PC.getObjective();
  rolEqCon = PC.getEqualityConstraint();

  evaluate_optimal_solution();
}


void RolTestInterface::init_paraboloid_circle()
{
  numVars = 2;
  numEqCons = 1;

  ROL::StdVector<Real> x0(xInitial);
  ROL::StdVector<Real> sol(xOptimal);

  ROL::ZOO::getParaboloidCircle
    < Real, ROL::StdVector<Real>, ROL::StdVector<Real>, ROL::StdVector<Real>,
      ROL::StdVector<Real> > PC;
  rolObj = PC.getObjective();
  rolEqCon = PC.getEqualityConstraint();

  evaluate_optimal_solution();
}


void RolTestInterface::evaluate_rol_fns()
{
  // We're omitting ActiveSet information for now, since ROL always
  // wants all data and it's cheap to populate.
  for (auto as_elt : directFnASV)
    if (as_elt & 4) {
      Cerr << "\nError: ROL test drivers do not support Hessians\n";
      abort_handler(INTERFACE_ERROR);
    }

  // continuous variables
  Teuchos::RCP<std::vector<Real>> xc_rcp(new std::vector<Real>(numVars, 0.0));
  copy_data(xC, *xc_rcp);
  ROL::StdVector<Real> rol_xc(xc_rcp);

  // we don't use ROL's tolerance information here; TODO: reasonable default?
  Real tol = 0.0;

  // objective value
  fnVals[0] = rolObj->value(rol_xc, tol);
  // objective gradient
  Teuchos::RCP<std::vector<Real>>
    obj_grad_rcp(new std::vector<Real>(numVars, 0.0));
  ROL::StdVector<Real> rol_obj_grad(obj_grad_rcp);
  rolObj->gradient(rol_obj_grad, rol_xc, tol);
  RealVector fn_grad(Teuchos::getCol(Teuchos::View, fnGrads, 0));
  copy_data(*obj_grad_rcp, fn_grad);

  if (numIneqCons > 0) {

    // inequality constraint values
    Teuchos::RCP<std::vector<Real>>
      cons_value_rcp(new std::vector<Real>(numIneqCons, 0.0));
    ROL::StdVector<Real> rol_cons_value(cons_value_rcp);
    rolIneqCon->value(rol_cons_value, rol_xc, tol);
    // TODO: copy_data_partial
    for (int j=0; j<numIneqCons; ++j)
      fnVals[1 + j] = (*cons_value_rcp)[j];

    // apply the Jacobian to the numVars identity matrix to get gradient
    for (int i=0; i<numVars; ++i) {

      Teuchos::RCP<std::vector<Real>>
	eye_col_rcp(new std::vector<Real>(numVars, 0.0));
      (*eye_col_rcp)[i] = 1.0;
      ROL::StdVector<Real> rol_eye_col(eye_col_rcp);

      Teuchos::RCP<std::vector<Real>>
	cons_grad_rcp(new std::vector<Real>(numIneqCons, 0.0));
      ROL::StdVector<Real> rol_cons_grad(cons_grad_rcp);

      rolIneqCon->applyJacobian(rol_cons_grad, rol_eye_col, rol_xc, tol);

      // TODO: copy_data for row of matrix
      for (int j=0; j<numIneqCons; ++j)
	fnGrads(i, 1 + j) = (*cons_grad_rcp)[j];
    }

  }

  if (numEqCons > 0) {

    // equality constraint values
    Teuchos::RCP<std::vector<Real>>
      cons_value_rcp(new std::vector<Real>(numEqCons, 0.0));
    ROL::StdVector<Real> rol_cons_value(cons_value_rcp);
    rolEqCon->value(rol_cons_value, rol_xc, tol);
    // TODO: copy_data_partial
    for (int j=0; j<numEqCons; ++j)
      fnVals[1 + numIneqCons + j] = (*cons_value_rcp)[j];

    // apply the Jacobian to the numVars identity matrix to get gradient
    for (int i=0; i<numVars; ++i) {

      Teuchos::RCP<std::vector<Real>>
	eye_col_rcp(new std::vector<Real>(numVars, 0.0));
      (*eye_col_rcp)[i] = 1.0;
      ROL::StdVector<Real> rol_eye_col(eye_col_rcp);

      Teuchos::RCP<std::vector<Real>>
	cons_grad_rcp(new std::vector<Real>(numEqCons, 0.0));
      ROL::StdVector<Real> rol_cons_grad(cons_grad_rcp);

      rolEqCon->applyJacobian(rol_cons_grad, rol_eye_col, rol_xc, tol);

      // TODO: copy_data for row of matrix
      for (int j=0; j<numEqCons; ++j)
	fnGrads(i, 1 + numIneqCons + j) = (*cons_grad_rcp)[j];
    }

  }

}


void RolTestInterface::evaluate_optimal_solution()
{
  copy_data(*xOptimal, xC);
  // don't get sized until runtime
  fnVals.size(1 + numIneqCons + numEqCons);
  fnGrads.shape(numVars, 1 + numIneqCons + numEqCons);

  evaluate_rol_fns();

  optimalFns.reset(new std::vector<Real>(numFns, 0.0));
  copy_data(fnVals, *optimalFns);
}

#endif  // OPT_TPL_ROL_TEST_INTERFACE
