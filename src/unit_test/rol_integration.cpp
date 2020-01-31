
#include "opt_tpl_test.hpp"
#include "ROLOptimizer.hpp"

#include "ROL_OptimizationSolver.hpp"
#include "ROL_OptimizationProblem.hpp"
#include "ROL_RandomVector.hpp"
#include "ROL_StdObjective.hpp"
#include "ROL_StdConstraint.hpp"
#include "ROL_Bounds.hpp"

#include "Teuchos_UnitTestHarness.hpp"
#include "Teuchos_oblackholestream.hpp"
#include "Teuchos_GlobalMPISession.hpp"

#include <memory>

using namespace Dakota;

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


//----------------------------------------------------------------
/// This is based on the corresponding test, text_book_nln_ineq_const,
/// in opt_tpl_rol_test_textbook.cpp but does not run the problem
/// and contains TWO nonlinear_inequality_constraints instead of one.
/// Instead of running the simulation, this test takes "0" iterations
/// to ensure objects are properly configured and then uses ROL's 
/// checkConstraint utility to compare numerical to analytic 
/// "apply hessian" values at various finite-difference step sizes.
/// The results are written to the file rol_diagnostics.txt.  
///
/// TODO:
///        I hope to be able to access these values
///        and use the differences to assess correctness and 
///        pass/fail criteria.

TEUCHOS_UNIT_TEST(rol, text_book_nln_ineq_const)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   rol"
    "     gradient_tolerance 1.0e-4"
    "     constraint_tolerance 1.0e-4"
    "     variable_tolerance 1.0e-4"
    "     max_iterations 0"
    " variables,"
    "   continuous_design = 3"
    "     initial_point  0.3    0.6   0.5"
    "     descriptors 'x_1'  'x_2'  'x_3'"
    " interface,"
    "   direct"
    "     analysis_driver = 'text_book'"
    " responses,"
    "   num_objective_functions = 1"
    "   nonlinear_inequality_constraints = 2"
    "   nonlinear_inequality_upper_bounds = 0.1 0.2"
    "   nonlinear_inequality_lower_bounds = -0.1 -0.05"
    "   analytic_gradients"
    "   analytic_hessians";

  Teuchos::oblackholestream bhs; // outputs nothing
  std::ofstream rol_diags("rol_diagnostics.txt");
  Teuchos::RCP<std::ostream> outStream =
    Teuchos::rcp(&rol_diags, false);
    //Teuchos::rcp(&Cout, false);
    //Teuchos::rcp(&bhs, false);

  std::shared_ptr<Dakota::LibraryEnvironment> p_env(Opt_TPL_Test::create_env(text_book_input));
  Dakota::LibraryEnvironment & env = *p_env;

  if (env.parallel_library().mpirun_flag())
    TEST_ASSERT( false ); // This test only works for serial builds

  // Execute the environment
  env.execute();

  // Now get the ROL Optimizer and test various reset functionality
  Dakota::ProblemDescDB& problem_db = env.problem_description_db();
  IteratorList& iter_list = problem_db.iterator_list();
  Dakota::Iterator & dak_iter = *iter_list.begin();
  //Cout << "The iterator is a : " << dak_iter.method_string() << endl;
  dak_iter.print_results(Cout);
  Dakota::ROLOptimizer * rol_optimizer = dynamic_cast<Dakota::ROLOptimizer*>(dak_iter.iterator_rep());

  ROL::OptimizationProblem<Real> & rol_problem = rol_optimizer->get_rol_problem();

  Teuchos::RCP<ROL::Vector<Real> > x, u, v, c, l;
  Teuchos::RCP<ROL::Vector<Real> > sol = rol_problem.getSolutionVector();
  Teuchos::RCP<ROL::Vector<Real> > mul = rol_problem.getMultiplierVector();

  x = sol->clone(); RandomizeVector(*x);
  u = sol->clone(); RandomizeVector(*u);
  v = sol->clone(); RandomizeVector(*v);

  c = mul->dual().clone(); RandomizeVector(*c);
  l = mul->clone();        RandomizeVector(*l);

  rol_problem.checkConstraint(*x, *u, *v, *c, *l, *outStream);

  rol_diags.close();
}

