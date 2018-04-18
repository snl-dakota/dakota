/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef OPT_TPL_ROL_HS_HPP
#define OPT_TPL_ROL_HS_HPP


#include "DirectApplicInterface.hpp"

//#include "ROL_HS29.hpp"
//#include "ROL_HS32.hpp"

#include "ROL_SimpleEqConstrained.hpp"

class RolTesterInterface: public DirectApplicInterface
{
  /// execute an analysis code portion of a direct evaluation invocation
  int derived_map_ac(const Dakota::String& ac_name)
  {
    if (multiProcAnalysisFlag) {
      Cerr << "Error: plugin serial direct fn does not support multiprocessor "
	   << "analyses." << std::endl;
      Dakota::abort_handler(-1);
    }


    //ROL::ZOO::Objective_SimpleEqConstrained<> obj;
    //ROL::ZOO::EqualityConstraint_SimpleEqConstrained<> eq_cons;
    Teuchos::RCP<ROL::Objective<Real>> obj;
    Teuchos::RCP<ROL::Constraint<Real>> eq_cons;

    Teuchos::RCP<std::vector<Real>> x0_rcp(new std::vector<Real>(5, 0.0));
    Teuchos::RCP<std::vector<Real>> sol_rcp(new std::vector<Real>(5, 0.0));

    ROL::StdVector<Real> x0(x0_rcp);
    ROL::StdVector<Real> sol(sol_rcp);

    ROL::ZOO::getSimpleEqConstrained
      < Real, ROL::StdVector<Real>, ROL::StdVector<Real>, ROL::StdVector<Real>,
	ROL::StdVector<Real> > (obj, eq_cons, x0, sol);
    
    int fail_code = 0;

    // simple_eq_constrained has 5 vars, 1 obj, 3 eq
    // paraboloid_circle has 2 vars, 1 obj, 1 eq

    if (ac_name == "simple_eq_constrained") {
      Dakota::RealVector fn_grad; 
      Dakota::RealSymMatrix fn_hess;
      if (directFnASV[0] & 2)
	fn_grad = Teuchos::getCol(Teuchos::View, fnGrads, 0);
      if (directFnASV[0] & 4)
	fn_hess = Dakota::RealSymMatrix(Teuchos::View, fnHessians[0],
					fnHessians[0].numRows());
      //      fail_code = simple_quad(xC, directFnASV[0], fnVals[0], fn_grad, fn_hess);

      // get objective
      Teuchos::RCP<std::vector<Real>> x_curr_rcp(new std::vector<Real>(5, 0.0));
      copy_data(xC, *x_curr_rcp);
      ROL::StdVector<Real> x_curr(x_curr_rcp);
      Real tol = 0.0;
      if (directFnASV[0] & 2)
	fnVals[0] = obj->value(x_curr, tol);
      if (directFnASV[0] & 2) {
	Teuchos::RCP<std::vector<Real>> 
	  obj_grad_rcp(new std::vector<Real>(5, 0.0));
	ROL::StdVector<Real> obj_grad(obj_grad_rcp);
	obj->gradient(obj_grad, x_curr, tol);
	fn_grad = Teuchos::getCol(Teuchos::View, fnGrads, 0);
	copy_data(*obj_grad_rcp, fn_grad);
      }

      // TODO: ASV
      if (true) {

	Teuchos::RCP<std::vector<Real>> 
	  cons_value_rcp(new std::vector<Real>(3, 0.0));
	ROL::StdVector<Real> cons_value(cons_value_rcp);
	eq_cons->value(cons_value, x_curr, tol);

	for (int i=0; i<5; ++i) {

	  // apply the Jacobian to the 5x5 identity matrix to get gradient
	  Teuchos::RCP<std::vector<Real>> 
	    eye_col_rcp(new std::vector<Real>(5, 0.0));
	  (*eye_col_rcp)[i] = 1.0;
	  ROL::StdVector<Real> eye_col(eye_col_rcp);

	  Teuchos::RCP<std::vector<Real>> 
	    cons_grad_rcp(new std::vector<Real>(3, 0.0));
	  ROL::StdVector<Real> cons_grad(cons_grad_rcp);
	  eq_cons->applyJacobian(cons_grad, eye_col, x_curr, tol);

	  // have to set this into a row of the gradient...
	  for (int j=0; j<3; ++i)
	    fnGrads(i, 1+j) = (*cons_grad_rcp)[j];
	}
      }

    }
    else {
      Cerr << ac_name << " is not available as an analysis within "
	   << "OptTestDirectApplicInterface." << std::endl;
      Dakota::abort_handler(Dakota::INTERFACE_ERROR);
    }

    // Failure capturing
    if (fail_code) {
      std::string err_msg("Error evaluating plugin analysis_driver ");
      err_msg += ac_name;
      throw Dakota::FunctionEvalFailure(err_msg);
    }

    return 0;
  }

private:
	// ROL::ZOO::Objective_HS32<double> hs32_obj;
	// ROL::ZOO::EqualityConstraint_HS32<double> hs32_eq;
	// ROL::ZOO::InequalityConstraint_HS32<double> hs32_ineq;


};


#endif  // OPT_TPL_ROL_HS_HPP
