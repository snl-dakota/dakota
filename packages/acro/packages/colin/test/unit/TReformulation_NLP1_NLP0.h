/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */

#include <acro_config.h>
#include <colin/reformulation/Downcast.h>
#include <colin/ConfigurableApplication.h>
#include <colin/reformulation/ConstraintPenalty.h>
#include <colin/OptApplications.h>
#include <colin/Problem.h>
#include <colin/real.h>
#include <utilib/pvector.h>
#include <utilib/Any.h>
#include <cxxtest/TestSuite.h>

#include "TestApplications.h"

namespace colin {
namespace unittest { class Test_Reformulation_NLP0_NLP1; }

class colin::unittest::Test_Reformulation_NLP0_NLP1 : public CxxTest::TestSuite
{
public:

#if 0
   void Xtest_traits()
    {
    utilib::Ereal<double> foo;
    foo = 1.0;
    CxxTest::ValueTraits< utilib::Ereal<double> > bar;
    CxxTest::ValueTraits< utilib::Ereal<double> > tmp = CxxTest::traits(foo);
    std::cerr << tmp.asString() << std::endl;
    }
#endif


   void test_test1b()
   {
      TS_TRACE("------------------------------------------------");
      TS_TRACE("Test1b - Setup NLP1 application and downcast it ");
      TS_TRACE("         before giving it to a NLP0 problem.");
      TS_TRACE("------------------------------------------------");
      colin::Problem<colin::NLP0_problem> prob;
      TS_TRACE("Problem setup");

      std::string foo = "example1";
      TestApplications::singleObj_denseCon<NLP1_problem> app;
      app.num_linear_constraints = 0;
      TS_TRACE("Application setup");

      DowncastApplication<NLP0_problem> dapp(app);
      TS_TRACE("Application reformulated");

      prob.set_application(dapp);
      TS_TRACE("Application assigned to problem");

      std::vector<double> vec(3);
      vec[0] = 1;
      vec[1] = 2;
      vec[2] = 4;

      colin::real ans;
      utilib::BasicArray<colin::real> gradient;
      utilib::BasicArray<colin::real> constraints;

      prob->EvalF(prob->eval_mngr(), vec, ans);
      TS_ASSERT_EQUALS(ans, 8.0);

      colin::Problem<colin::NLP1_problem> prob2;
      prob2.set_application(app);

      prob2->EvalF(prob2->eval_mngr(), vec, ans);
      TS_ASSERT_EQUALS(ans, 8.0);

      utilib::BasicArray<colin::real> grad_ans(3);
      grad_ans[0] = 8.0;
      grad_ans[1] = 4.0;
      grad_ans[2] = 2.0;
      prob2->EvalG(prob2->eval_mngr(), vec, gradient);
      TS_ASSERT_EQUALS(gradient, grad_ans);

      utilib::BasicArray<colin::real> con_ans(3);
      con_ans[0] = 21.0;
      con_ans[1] = 17.0;
      con_ans[2] = 7.0;
      prob2->EvalCF(prob2->eval_mngr(), vec, constraints);
      TS_ASSERT_EQUALS(constraints, con_ans);

      //ApplicationMngr().clear();
      TS_TRACE("Done.");
   }


   void test_test1c()
   {
      TS_TRACE("------------------------------------------------");
      TS_TRACE("Test1c - Setup NLP1 application and reformulate ");
      TS_TRACE("         it to a NLP0 problem.");
      TS_TRACE("------------------------------------------------");
      colin::Problem<colin::NLP0_problem> prob;
      TS_TRACE("Problem setup");

      std::string foo = "example1";
      TestApplications::singleObj_denseCon<NLP1_problem> app;
      app.num_nonlinear_constraints = 0;
      app.num_linear_constraints = 0;
      TS_TRACE("Application setup");

      colin::DowncastApplication<colin::NLP0_problem> reformulated_app(app);
      TS_TRACE("Application reformulated");

      prob.set_application(reformulated_app);
      TS_TRACE("Application assigned to problem");

      std::vector<double> vec(3);
      vec[0] = 1;
      vec[1] = 2;
      vec[2] = 4;

      colin::real ans;
      prob->EvalF(prob->eval_mngr(), vec, ans);
      TS_ASSERT_EQUALS(ans, 8.0);

      colin::Problem<colin::NLP1_problem> prob2;
      prob2.set_application(app);

      prob2->EvalF(prob2->eval_mngr(), vec, ans);
      TS_ASSERT_EQUALS(ans, 8.0);

      utilib::BasicArray<colin::real> grad_ans(3);
      grad_ans[0] = 8.0;
      grad_ans[1] = 4.0;
      grad_ans[2] = 2.0;
      utilib::BasicArray<colin::real> gradient;
      prob2->EvalG(prob2->eval_mngr(), vec, gradient);
      TS_ASSERT_EQUALS(gradient, grad_ans);

      utilib::BasicArray<colin::real> con_ans(0);
      utilib::BasicArray<colin::real> constraints;
      prob2->EvalCF(prob2->eval_mngr(), vec, constraints);
      TS_ASSERT_EQUALS(constraints, con_ans);

      //ApplicationMngr().clear();
      TS_TRACE("Done.");
   }


   void test_test1d()
   {
      TS_TRACE("------------------------------------------------");
      TS_TRACE("Test1d - Setup NLP1 application and reformulate ");
      TS_TRACE("         it to eliminate constraints.");
      TS_TRACE("------------------------------------------------");

      std::string foo = "example1";

      TestApplications::singleObj_denseCon<NLP1_problem> app;
      app.num_linear_constraints = 0;
      std::vector<real> clower(3);
      std::vector<real> cupper(3);
      clower[0] = 0.0;
      clower[1] = real::negative_infinity;
      clower[2] = 0.2;
      cupper[0] = real::positive_infinity;
      cupper[1] = 1.1;
      cupper[2] = 1.2;
      app.nonlinear_constraint_lower_bounds = clower;
      app.nonlinear_constraint_upper_bounds = cupper;
      TS_TRACE("Application setup");

      std::vector<double> x(3);
      x[0] = 1;
      x[1] = 2;
      x[2] = 4;

      colin::real scalar;
      utilib::BasicArray<colin::real> vec;
      utilib::BasicArray<utilib::BasicArray<double> > mat;

      utilib::BasicArray<colin::real> con(3);
      con[0] = 21;
      con[1] = 17;
      con[2] = 7;

      utilib::BasicArray<colin::real> cviol(3);
      cviol[0] = 0;
      cviol[1] = 15.9;
      cviol[2] = 5.8;

      utilib::BasicArray<colin::real> grad(3);
      grad[0] = 8;
      grad[1] = 4;
      grad[2] = 2;

      utilib::BasicArray<colin::real> penalty_grad(3);
      penalty_grad[0] = 51.4; // 8 + 2*0*2 + 2*15.9*1 + 2*5.8*1
      penalty_grad[1] = 79.2; // 4 + 2*0*4 + 2*15.9*2 + 2*5.8*1
      penalty_grad[2] = 109;  // 2 + 2*0*8 + 2*15.9*3 + 2*5.8*1

      try
      {
         //std::cout << std::endl;
         //std::cout << "SUMMARY" << std::endl;
         //prob2->print_summary(cout);
         //std::cout << std::endl;

         app.EvalF(app.eval_mngr(), x, scalar);
         TS_ASSERT_EQUALS(scalar, 8.0)

         app.EvalG(app.eval_mngr(), x, vec);
         TS_ASSERT_EQUALS(vec, grad)

         app.EvalNLCF(app.eval_mngr(), x, vec);
         TS_ASSERT_EQUALS(vec, con)

         app.EvalCFViol(app.eval_mngr(), x, vec);
         TS_ASSERT_EQUALS(vec, cviol)

         app.EvalCG(app.eval_mngr(), x, mat);
         TS_ASSERT_EQUALS(mat[0][0], 2.);
         TS_ASSERT_EQUALS(mat[0][1], 4.);
         TS_ASSERT_EQUALS(mat[0][2], 8.);
         TS_ASSERT_EQUALS(mat[1][0], 1.);
         TS_ASSERT_EQUALS(mat[1][1], 2.);
         TS_ASSERT_EQUALS(mat[1][2], 3.);
         TS_ASSERT_EQUALS(mat[2][0], 1.);
         TS_ASSERT_EQUALS(mat[2][1], 1.);
         TS_ASSERT_EQUALS(mat[2][2], 1.);

         colin::ConstraintPenaltyApplication<colin::UNLP1_problem>
            penalty_app(app);
         TS_TRACE("Application reformulated");

         //std::cout << std::endl;
         //std::cout << "SUMMARY" << std::endl;
         //prob->print_summary(cout);
         //std::cout << std::endl;

         penalty_app.EvalF(penalty_app.eval_mngr(), x, scalar);
         TS_ASSERT_EQUALS(scalar, 294.45)

         penalty_app.EvalG(penalty_app.eval_mngr(), x, vec);
         // Compare w/ delta because of round-off error...
         TS_ASSERT_DELTA(vec[0], penalty_grad[0], 1e-10);
         TS_ASSERT_DELTA(vec[1], penalty_grad[1], 1e-10);
         TS_ASSERT_DELTA(vec[2], penalty_grad[2], 1e-10);
      }
      catch (utilib::bad_lexical_cast& err)
      {
         std::cerr << "Exception caught:" << err.what() << std::endl;
         throw;
      }

      //ApplicationMngr().clear();
      TS_TRACE("Done.");
   }


   void test_test1e()
   {
      TS_TRACE("------------------------------------------------");
      TS_TRACE("Test1d - Setup NLP1 application and reformulate ");
      TS_TRACE("         it to eliminate constraints using .");
      TS_TRACE("         lexical casts");
      TS_TRACE("------------------------------------------------");

      std::string foo = "example1";
      TestApplications::singleObj_denseCon<NLP1_problem> app;
      app.num_linear_constraints = 0;
      std::vector<real> clower(3);
      std::vector<real> cupper(3);
      clower[0] = 0.0;
      clower[1] = real::negative_infinity;
      clower[2] = 0.2;
      cupper[0] = real::positive_infinity;
      cupper[1] = 1.1;
      cupper[2] = 1.2;
      app.nonlinear_constraint_lower_bounds = clower;
      app.nonlinear_constraint_upper_bounds = cupper;
      TS_TRACE("Application setup");

      std::vector<double> vec(3);
      vec[0] = 1;
      vec[1] = 2;
      vec[2] = 4;
      colin::real ans;

      utilib::BasicArray<colin::real> grad_ans(3);
      grad_ans[0] = 8.0;
      grad_ans[1] = 4.0;
      grad_ans[2] = 2.0;

      utilib::BasicArray<colin::real> con_ans(3);
      con_ans[0] = 21;
      con_ans[1] = 17;
      con_ans[2] = 7;

      try
      {
         //utilib::exception_mngr::set_mode(utilib::exception_mngr::Exit);
         colin::Problem<colin::NLP1_problem> prob2;
         prob2.set_application(app);

         //std::cout << std::endl;
         //std::cout << "SUMMARY" << std::endl;
         //prob2->print_summary(cout);
         //std::cout << std::endl;
         prob2->EvalF(prob2->eval_mngr(), vec, ans);
         TS_ASSERT_EQUALS(ans, 8.0)

         utilib::BasicArray<colin::real> gradient;
         prob2->EvalG(prob2->eval_mngr(), vec, gradient);
         TS_ASSERT_EQUALS(gradient, grad_ans)

         utilib::BasicArray<colin::real> constraints;
         prob2->EvalNLCF(prob2->eval_mngr(), vec, constraints);
         TS_ASSERT_EQUALS(constraints, con_ans)

         Problem<UNLP0_problem> prob;
         prob = prob2;
      TS_TRACE("Application assigned to problem");

         //std::cout << std::endl;
         //std::cout << "SUMMARY" << std::endl;
         //prob->print_summary(cout);
         //std::cout << std::endl;

         prob->EvalF(prob->eval_mngr(), vec, ans);
         TS_ASSERT_EQUALS(ans, 294.45)
      }
      catch (utilib::bad_lexical_cast& err)
      {
         std::cerr << "Exception caught:" << err.what()
         << std::endl;
         throw;
      }

      //ApplicationMngr().clear();
      TS_TRACE("Done");
   }


#if 0
   void Xtest_test1f()
   {
      std::cout << std::endl;
      std::cout << "Test1f - full test of NLP application" << std::endl;

      std::string foo = "example1";
      colin::ConfigurableApplication<colin::NLP1_problem>* app
      = colin::new_application(foo, &fn4);
      app->set_num_real_vars(3);

      utilib::pvector<double> lower(3);
      utilib::pvector<double> upper(3);
      app->set_real_bounds(lower, upper);
      app->set_num_nonlinear_constraints(3);
      app->set_num_linear_constraints(3);

      TS_TRACE("Application setup");

      std::vector<double> vec(3);
      vec[0] = 1;
      vec[1] = 2;
      vec[2] = 4;

      colin::real ans;
      app->EvalF(app->eval_mngr(), vec, ans);
      std::cout << "ANS: f_info = " << ans << std::endl;

      colin::AppResponse response;
      colin::AppRequest request = app->set_domain(vec);
      ans = 0.0;
      app->Request_F(request, ans);
      app->eval_mngr().perform_evaluation(request);
      std::cout << "ANS: f_info = " << ans << std::endl;

      vector<real> v_ans;

      v_ans.clear();
      request = app->set_domain(vec);
      app->Request_CF(request, v_ans);
      response = app->eval_mngr().perform_evaluation(request);
      cout << "cf_info = " << v_ans << endl;
      cout << "response = " << response;
      cout << "  (cf) = " << response.get(cf_info) << endl;
      cout << "  (nlcf) = " << response.get(nlcf_info) << endl;

      v_ans.clear();
      app->EvalNLCF(app->eval_mngr(), vec, v_ans);
      std::cout << "nlcf_info =\t" << v_ans << std::endl;

      v_ans.clear();
      app->EvalNLEqCF(app->eval_mngr(), vec, v_ans);
      std::cout << "nleqcf_info =\t" << v_ans << std::endl;

      v_ans.clear();
      app->EvalNLIneqCF(app->eval_mngr(), vec, v_ans);
      std::cout << "nlineqcf_info =\t" << v_ans << std::endl;

      v_ans.clear();
      app->EvalLCF(app->eval_mngr(), vec, v_ans);
      std::cout << "lcf_info =\t" << v_ans << std::endl;

      v_ans.clear();
      app->EvalLEqCF(app->eval_mngr(), vec, v_ans);
      std::cout << "leqcf_info =\t" << v_ans << std::endl;

      v_ans.clear();
      app->EvalLIneqCF(app->eval_mngr(), vec, v_ans);
      std::cout << "ineqlcf_info =\t" << v_ans << std::endl;

      v_ans.clear();
      app->EvalCF(app->eval_mngr(), vec, v_ans);
      std::cout << "cf_info =\t" << v_ans << std::endl;

      v_ans.clear();
      app->EvalEqCF(app->eval_mngr(), vec, v_ans);
      std::cout << "eqcf_info =\t" << v_ans << std::endl;

      v_ans.clear();
      app->EvalIneqCF(app->eval_mngr(), vec, v_ans);
      std::cout << "ineqcf_info =\t" << v_ans << std::endl;

      v_ans.clear();
      app->EvalCFViol(app->eval_mngr(), vec, v_ans);
      std::cout << "cvf_info =\t" << v_ans << std::endl;

      std::vector<real> clower(3);
      std::vector<real> cupper(3);
      clower[0] = 0.0;
      clower[1] = real::negative_infinity;
      clower[2] = 10;

      cupper[0] = real::positive_infinity;
      cupper[1] = 1;
      cupper[2] = 10;
      app->set_nonlinear_constraint_bounds(clower, cupper);
      app->set_linear_constraint_bounds(clower, cupper);
      TS_TRACE("Set constraint bounds");

      v_ans.clear();
      app->EvalCF(app->eval_mngr(), vec, v_ans);
      cout << "cf_info = \t" << v_ans << endl;

      v_ans.clear();
      app->EvalNLCF(app->eval_mngr(), vec, v_ans);
      std::cout << "nlcf_info =\t" << v_ans << std::endl;

      v_ans.clear();
      app->EvalNLEqCF(app->eval_mngr(), vec, v_ans);
      std::cout << "nleqcf_info =\t" << v_ans << std::endl;

      v_ans.clear();
      app->EvalNLIneqCF(app->eval_mngr(), vec, v_ans);
      std::cout << "nlineqcf_info =\t" << v_ans << std::endl;

      v_ans.clear();
      app->EvalLCF(app->eval_mngr(), vec, v_ans);
      std::cout << "lcf_info =\t" << v_ans << std::endl;

      v_ans.clear();
      app->EvalLEqCF(app->eval_mngr(), vec, v_ans);
      std::cout << "leqcf_info =\t" << v_ans << std::endl;

      v_ans.clear();
      app->EvalLIneqCF(app->eval_mngr(), vec, v_ans);
      std::cout << "ineqlcf_info =\t" << v_ans << std::endl;

      v_ans.clear();
      app->EvalCF(app->eval_mngr(), vec, v_ans);
      std::cout << "cf_info =\t" << v_ans << std::endl;

      v_ans.clear();
      app->EvalEqCF(app->eval_mngr(), vec, v_ans);
      std::cout << "eqcf_info =\t" << v_ans << std::endl;

      v_ans.clear();
      app->EvalIneqCF(app->eval_mngr(), vec, v_ans);
      std::cout << "ineqcf_info =\t" << v_ans << std::endl;

      v_ans.clear();
      app->EvalCFViol(app->eval_mngr(), vec, v_ans);
      std::cout << "cvf_info =\t" << v_ans << std::endl;


      TS_TRACE("Set linear constraint matrix");
      utilib::RMSparseMatrix<double> mat;
      int*    itmp = new int[3];
      double* dtmp = new double[3];
      *itmp = 0;
      *dtmp = 1.0;
      mat.adjoinRow(1, itmp, dtmp);
      *itmp = 1;
      *dtmp = 2.0;
      mat.adjoinRow(1, itmp, dtmp);
      itmp[0] = 0;
      dtmp[0] = -1.0;
      itmp[1] = 2;
      dtmp[1] = 1.0;
      mat.adjoinRow(2, itmp, dtmp);
      app->set_linear_constraint_matrix(mat);
      //  1 . .
      //  . 2 .
      // -1 . 1

      v_ans.clear();
      app->EvalLCF(app->eval_mngr(), vec, v_ans);
      std::cout << "lcf_info =\t" << v_ans << std::endl;

      v_ans.clear();
      app->EvalLEqCF(app->eval_mngr(), vec, v_ans);
      std::cout << "leqcf_info =\t" << v_ans << std::endl;

      v_ans.clear();
      app->EvalLIneqCF(app->eval_mngr(), vec, v_ans);
      std::cout << "ineqlcf_info =\t" << v_ans << std::endl;

      v_ans.clear();
      app->EvalCF(app->eval_mngr(), vec, v_ans);
      cout << "cf_info = \t" << v_ans << endl;


      TS_TRACE("Deleting Application");
      ApplicationMngr().unregister_application(foo);
      delete app;

      delete [] itmp;
      delete [] dtmp;

      ApplicationMngr().clear();
      TS_TRACE("Done");
   }
#endif

};

} // namespace colin
