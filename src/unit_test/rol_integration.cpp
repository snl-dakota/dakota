
#include "ROL_OptimizationSolver.hpp"

#include "ROL_RandomVector.hpp"
#include "ROL_StdObjective.hpp"
#include "ROL_StdConstraint.hpp"
#include "ROL_Bounds.hpp"

#include "Teuchos_UnitTestHarness.hpp"
#include "Teuchos_oblackholestream.hpp"
#include "Teuchos_GlobalMPISession.hpp"

/* OBJECTIVE FUNCTION */

template<class Real> 
class ObjectiveQL : public ROL::StdObjective<Real> {
private:
  std::vector<Real> coeff;

public:

  ObjectiveQL() : coeff({-21.98,-1.26,61.39,5.3,101.3}) { 
  }

  Real value(const std::vector<Real> &x, Real &tol) {
    Real result = 0;
    for( int i=0; i<5; ++i ) {
      result +=x[i]*(0.5*x[i]+coeff[i]);
    }  
    return result;
  }

  void gradient( std::vector<Real> &g, const std::vector<Real> &x, Real &tol ) {
    for( int i=0; i<5; ++i ) {
      g[i] = x[i]+coeff[i];
    }  
  }

  void hessVec( std::vector<Real> &hv, const std::vector<Real> &v, const std::vector<Real> &x, Real &tol ) {
    hv = v;
  }

}; // class ObjectiveQL

/* INEQUALITY CONSTRAINT */

template<class Real> 
class InequalityQL : public ROL::StdConstraint<Real> {
private:
  std::vector<Real> coeff;
  Real offset;

public:
  InequalityQL() : coeff({-7.56,0.0,0.0,0.0,0.5}), offset(39.1) {}

  void value( std::vector<Real> &c, const std::vector<Real> &x, Real &tol) {
    c[0] = offset;
    for( int i=0; i<5; ++i ) {
      c[0] += coeff[i]*x[i];
    }
  }

  void applyJacobian(  std::vector<Real> &jv, const std::vector<Real> &v ,const std::vector<Real> &x, Real &tol ) {
    jv[0] = 0;
    for( int i=0; i<5; ++i ) {
      jv[0] += coeff[i]*v[i];
    }
  }

  void applyAdjointJacobian( std::vector<Real> &ajv, const std::vector<Real> &v ,const std::vector<Real> &x, Real &tol ) {
    for( int i=0; i<5; ++i ) {
      ajv[i] = v[0]*coeff[i];
    }
  }

  void applyAdjointHessian( std::vector<Real> &ahuv, const std::vector<Real> &u,
                       const std::vector<Real> &v ,const std::vector<Real> &x, Real &tol ) {
    ahuv.assign(5,0.0);
  }

}; // class InequalityQL

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(rol, basic1)
{
  using Teuchos::RCP; using Teuchos::rcp;

  typedef double RealT;
  int iprint     = 1;
  RCP<std::ostream> outStream;
  Teuchos::oblackholestream bhs; // outputs nothing
  if (iprint > 0)
    outStream = rcp(&std::cout, false);
  else
    outStream = rcp(&bhs, false);


  int errorFlag   = 0;

  try {
 
    Teuchos::ParameterList parlist;
    parlist.sublist("Step").set("Type","Line Search");
    

    RCP<std::vector<RealT> > l_rcp = rcp( new std::vector<RealT>(5,-100.0) );
    RCP<std::vector<RealT> > u_rcp = rcp( new std::vector<RealT>(5, 100.0) );

    RCP<ROL::Vector<RealT> > lower = rcp( new ROL::StdVector<RealT>( l_rcp ) );
    RCP<ROL::Vector<RealT> > upper = rcp( new ROL::StdVector<RealT>( u_rcp ) ); 

    RCP<std::vector<RealT> > x_rcp  = rcp( new std::vector<RealT>(5,1.0) );
    RCP<std::vector<RealT> > li_rcp = rcp( new std::vector<RealT>(1,0.0) );
    RCP<std::vector<RealT> > ll_rcp = rcp( new std::vector<RealT>(1,0.0) );
    RCP<std::vector<RealT> > lu_rcp = rcp( new std::vector<RealT>(1,ROL::ROL_INF<RealT>()) );

    RCP<ROL::Vector<RealT> > x  = rcp( new ROL::StdVector<RealT>(x_rcp) );
    RCP<ROL::Vector<RealT> > li = rcp( new ROL::StdVector<RealT>(li_rcp) );
    RCP<ROL::Vector<RealT> > ll = rcp( new ROL::StdVector<RealT>(ll_rcp) );
    RCP<ROL::Vector<RealT> > lu = rcp( new ROL::StdVector<RealT>(lu_rcp) );

    RCP<ROL::Objective<RealT> >             obj  = rcp( new ObjectiveQL<RealT>() );
    RCP<ROL::BoundConstraint<RealT> >       bnd  = rcp( new ROL::Bounds<RealT>(lower,upper) );
    RCP<ROL::Constraint<RealT> >            ineq = rcp( new InequalityQL<RealT>() );
    RCP<ROL::BoundConstraint<RealT> >       ibnd = rcp( new ROL::Bounds<RealT>(ll,lu) );

    ROL::OptimizationProblem<RealT> problem( obj, x, bnd, ineq, li, ibnd);    

    /* checkAdjointJacobianConsistency fails for the OptimizationProblem if we don't do this first... why? */
    RCP<ROL::Vector<RealT> > u = x->clone(); 
    RandomizeVector(*u);
    ineq->checkAdjointConsistencyJacobian(*li,*x,*u,true,*outStream);
    /*******************************************************************************************************/

    problem.check(*outStream);

    ROL::OptimizationSolver<RealT> solver( problem, parlist );

    solver.solve(*outStream); 

    *outStream << "x_opt = [";
    for(int i=0;i<4;++i) {
      *outStream << (*x_rcp)[i] << ", " ;
    } 
    *outStream << (*x_rcp)[4] << "]" << std::endl;
  }
  catch (std::logic_error err) {
    *outStream << err.what() << "\n";
    errorFlag = -1000;
  }; // end try

  if (errorFlag != 0)
    std::cout << "End Result: TEST FAILED\n";
  else
    std::cout << "End Result: TEST PASSED\n";
}


//
//
//using namespace Dakota;
//
////----------------------------------------------------------------
//
//TEUCHOS_UNIT_TEST(opt_apps,cyl_head_1)
//{
//  /// Default Dakota input string:
//  static const char cyl_head_input[] = 
//    " method,"
//    "   output silent"
//    "   max_function_evaluations 500"
//    "   asynch_pattern_search"
//    "     threshold_delta = 1.e-10"
//    "     synchronization blocking"
//    " variables,"
//    "   continuous_design = 2"
//    "     initial_point    1.51         0.01"
//    "     upper_bounds     2.164        4.0"
//    "     lower_bounds     1.5          0.0"
//    "     descriptors      'intake_dia' 'flatness'"
//    " interface,"
//    "   direct"
//    "     analysis_driver = 'cyl_head'"
//    " responses,"
//    "   num_objective_functions = 1"
//    "   nonlinear_inequality_constraints = 3"
//    "   no_gradients"
//    "   no_hessians";
//
//  Dakota::LibraryEnvironment * p_env = Opt_TPL_Test::create_env(cyl_head_input);
//  Dakota::LibraryEnvironment & env = *p_env;
//
//  if (env.parallel_library().mpirun_flag())
//    TEST_ASSERT( false ); // This test only works for serial builds
//
//  // Execute the environment
//  env.execute();
//
//  // retrieve the final parameter values
//  const Variables& vars = env.variables_results();
//
//  // convergence tests: "Optimal" solution used for comparison
//  // is obtained using Teuchos unit tests in
//  // opt_tpl_test_exact_soln.cpp
//  double rel_err;
//  double target;
//  double max_tol;
//
//  target = 2.1224215765;
//  max_tol = 1.e-3;
//  rel_err = fabs((vars.continuous_variable(0) - target)/target);
//  TEST_COMPARE(rel_err,<, max_tol);
//
//  target = 1.7659069377;
//  max_tol = 1.e-2;
//  rel_err = fabs((vars.continuous_variable(1) - target)/target);
//  TEST_COMPARE(rel_err,<, max_tol);
//
//  // retrieve the final response values
//  const Response& resp  = env.response_results();
//
//  target = -2.4614299775;
//  max_tol = 1.e-3;
//  rel_err = fabs((resp.function_value(0) - target)/target);
//  TEST_COMPARE(rel_err,<, max_tol);
//}
