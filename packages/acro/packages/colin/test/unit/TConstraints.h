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

#include <cxxtest/TestSuite.h>

#include <colin/ApplicationMngr.h>

#include "TestApplications.h"

// TODO - Define tests that exercise the registration/collection
// operations in the Constraint class.

namespace colin {
namespace unittest { class Application_Constraints; }

class colin::unittest::Application_Constraints : public CxxTest::TestSuite
{
public:
   // The test application
   TestApplications::singleObj_denseCon<NLP1_problem> *app;

   void setUp()
   {
      utilib::exception_mngr::set_mode(utilib::exception_mngr::Standard);
      app = new TestApplications::singleObj_denseCon<NLP1_problem>(3);

      std::vector<real> c(3);

      c[0] = 0.0;
      c[1] = real::negative_infinity;
      c[2] = 0.2;
      app->nonlinear_constraint_lower_bounds = c;

      c[0] = real::positive_infinity;
      c[1] = 1.1;
      c[2] = 1.2;
      app->nonlinear_constraint_upper_bounds = c;

      c[0] = 0.0;
      c[1] = real::negative_infinity;
      c[2] = 2.2;
      app->linear_constraint_lower_bounds = c;

      c[0] = real::positive_infinity;
      c[1] = 1.1;
      c[2] = 2.2;
      app->linear_constraint_upper_bounds = c;
   }

   void tearDown()
   {
      delete app;
      ApplicationMngr().clear();
   }

   void test_numc()
   {
      TS_TRACE("------------------------------------------------");
      TS_TRACE("numc - Test the number of constraints");
      TS_TRACE("------------------------------------------------");

      TS_ASSERT_EQUALS(app->num_constraints, 6u);
      TS_ASSERT_EQUALS(app->num_linear_constraints, 3u);
      TS_ASSERT_EQUALS(app->num_nonlinear_constraints, 3u);
      TS_ASSERT_EQUALS(app->numEqConstraints(), 1u);
      TS_ASSERT_EQUALS(app->numIneqConstraints(), 5u);

   }

   void test_evalcf()
   {
      TS_TRACE("------------------------------------------------");
      TS_TRACE("evalcf - Evaluate constraints");
      TS_TRACE("------------------------------------------------");

      // NB: while these tests will pass and should be portable, the
      // ordering of linear/nonlinear constraints in the cf_info result
      // is not strictly defined (it is based on the order that
      // callbacks get registered with the Application_Constraints
      // ... set by the order that the classes are declared in the
      // Application class).

      std::vector<double> vec(3);
      vec[0] = 1;
      vec[1] = 2;
      vec[2] = 4;

      colin::real ans;
      utilib::BasicArray<colin::real> gradient;

      utilib::BasicArray<colin::real> constraints;
      utilib::BasicArray<colin::real> con_ans(6);
      con_ans[0] = 7;
      con_ans[1] = 14;
      con_ans[2] = 17;
      con_ans[3] = 21;
      con_ans[4] = 17;
      con_ans[5] = 7;

      app->EvalCF(app->eval_mngr(), vec, constraints);
      TS_ASSERT_DELTA(constraints[0], con_ans[0], 1e-7);
      TS_ASSERT_DELTA(constraints[1], con_ans[1], 1e-7);
      TS_ASSERT_DELTA(constraints[2], con_ans[2], 1e-7);
      TS_ASSERT_DELTA(constraints[3], con_ans[3], 1e-7);
      TS_ASSERT_DELTA(constraints[4], con_ans[4], 1e-7);
      TS_ASSERT_DELTA(constraints[5], con_ans[5], 1e-7);

      app->EvalEqCF(app->eval_mngr(), vec, constraints);
      con_ans.resize(1);
      con_ans[0] = 17;
      TS_ASSERT_DELTA(constraints[0], con_ans[0], 1e-7);

      app->EvalIneqCF(app->eval_mngr(), vec, constraints);
      con_ans.resize(5);
      con_ans[0] = 7;
      con_ans[1] = 14;
      con_ans[2] = 21;
      con_ans[3] = 17;
      con_ans[4] = 7;
      TS_ASSERT_DELTA(constraints[0], con_ans[0], 1e-7);
      TS_ASSERT_DELTA(constraints[1], con_ans[1], 1e-7);
      TS_ASSERT_DELTA(constraints[2], con_ans[2], 1e-7);
      TS_ASSERT_DELTA(constraints[3], con_ans[3], 1e-7);
      TS_ASSERT_DELTA(constraints[4], con_ans[4], 1e-7);
   }

   void test_async_evalcf()
   {
      TS_TRACE("--------------------------------------------------");
      TS_TRACE("async_evalcf - Evaluate constraints asynchronously");
      TS_TRACE("--------------------------------------------------");

      std::vector<double> vec(3);
      vec[0] = 1;
      vec[1] = 2;
      vec[2] = 4;

      colin::real ans;
      utilib::BasicArray<colin::real> gradient;

      utilib::BasicArray<colin::real> constraints;
      utilib::BasicArray<colin::real> con_ans(6);
      con_ans[0] = 7;
      con_ans[1] = 14;
      con_ans[2] = 17;
      con_ans[3] = 21;
      con_ans[4] = 17;
      con_ans[5] = 7;

      app->AsyncEvalCF(app->eval_mngr(), vec, constraints);
      app->synchronize();
      TS_ASSERT_DELTA(constraints[0], con_ans[0], 1e-7);
      TS_ASSERT_DELTA(constraints[1], con_ans[1], 1e-7);
      TS_ASSERT_DELTA(constraints[2], con_ans[2], 1e-7);
      TS_ASSERT_DELTA(constraints[3], con_ans[3], 1e-7);
      TS_ASSERT_DELTA(constraints[4], con_ans[4], 1e-7);
      TS_ASSERT_DELTA(constraints[5], con_ans[5], 1e-7);

      app->AsyncEvalEqCF(app->eval_mngr(), vec, constraints);
      app->synchronize();
      con_ans.resize(1);
      con_ans[0] = 17;
      TS_ASSERT_DELTA(constraints[0], con_ans[0], 1e-7);

      app->AsyncEvalIneqCF(app->eval_mngr(), vec, constraints);
      app->synchronize();
      con_ans.resize(5);
      con_ans[0] = 7;
      con_ans[1] = 14;
      con_ans[2] = 21;
      con_ans[3] = 17;
      con_ans[4] = 7;
      TS_ASSERT_DELTA(constraints[0], con_ans[0], 1e-7);
      TS_ASSERT_DELTA(constraints[1], con_ans[1], 1e-7);
      TS_ASSERT_DELTA(constraints[2], con_ans[2], 1e-7);
      TS_ASSERT_DELTA(constraints[3], con_ans[3], 1e-7);
      TS_ASSERT_DELTA(constraints[4], con_ans[4], 1e-7);
   }

   void test_cfviol()
   {
      TS_TRACE("--------------------------------------------------");
      TS_TRACE("cfviol - Evaluate constraint violations ");
      TS_TRACE("--------------------------------------------------");

      std::vector<double> vec(3);
      vec[0] = 1;
      vec[1] = 2;
      vec[2] = 4;

      colin::real ans;

      utilib::BasicArray<colin::real> constraints;
      utilib::BasicArray<colin::real> con_ans(6);
      con_ans[0] = 0;
      con_ans[1] = 12.9;
      con_ans[2] = 14.8;
      con_ans[3] = 0;
      con_ans[4] = 15.9;
      con_ans[5] = 5.8;

      app->EvalCFViol(app->eval_mngr(), vec, constraints);
      TS_ASSERT_DELTA(constraints[0], con_ans[0], 1e-7);
      TS_ASSERT_DELTA(constraints[1], con_ans[1], 1e-7);
      TS_ASSERT_DELTA(constraints[2], con_ans[2], 1e-7);
      TS_ASSERT_DELTA(constraints[3], con_ans[3], 1e-7);
      TS_ASSERT_DELTA(constraints[4], con_ans[4], 1e-7);
      TS_ASSERT_DELTA(constraints[5], con_ans[5], 1e-7);
   }

   void test_async_cfviol()
   {
      TS_TRACE("--------------------------------------------------");
      TS_TRACE("async_cfviol - Evaluate constraint violations");
      TS_TRACE("    asynchronously");
      TS_TRACE("--------------------------------------------------");

      std::vector<double> vec(3);
      vec[0] = 1;
      vec[1] = 2;
      vec[2] = 4;

      colin::real ans;

      utilib::BasicArray<colin::real> constraints;
      utilib::BasicArray<colin::real> con_ans(6);
      con_ans[0] = 0;
      con_ans[1] = 12.9;
      con_ans[2] = 14.8;
      con_ans[3] = 0;
      con_ans[4] = 15.9;
      con_ans[5] = 5.8;

      app->AsyncEvalCFViol(app->eval_mngr(), vec, constraints);
      app->synchronize();
      TS_ASSERT_DELTA(constraints[0], con_ans[0], 1e-7);
      TS_ASSERT_DELTA(constraints[1], con_ans[1], 1e-7);
      TS_ASSERT_DELTA(constraints[2], con_ans[2], 1e-7);
      TS_ASSERT_DELTA(constraints[3], con_ans[3], 1e-7);
      TS_ASSERT_DELTA(constraints[4], con_ans[4], 1e-7);
      TS_ASSERT_DELTA(constraints[5], con_ans[5], 1e-7);
   }

   void test_epsilon()
   {
      TS_TRACE("--------------------------------------------------");
      TS_TRACE("epsilon - Test that the constraint epsilon works");
      TS_TRACE("    as expected");
      TS_TRACE("--------------------------------------------------");

      std::vector<double> vec(3);
      vec[0] = 1;
      vec[1] = 2;
      vec[2] = 4;

      TS_ASSERT(app->equality_epsilon < 0.001);

      // initially, there should be 1 equality constraint (linear #3)
      std::vector<double> ans;
      app->EvalEqCF(app->eval_mngr(), vec, ans);
      TS_ASSERT_EQUALS(ans.size(), 1u);
      TS_ASSERT_EQUALS(ans[0], 17);
      app->EvalIneqCF(app->eval_mngr(), vec, ans);
      TS_ASSERT_EQUALS(ans.size(), 5u);

      app->equality_epsilon = 2.0;
      TS_ASSERT_EQUALS(app->equality_epsilon, 2.0);
      
      // with a broadened epsilon, nonlinear #3 should also be an "equality"
      app->EvalEqCF(app->eval_mngr(), vec, ans);
      TS_ASSERT_EQUALS(ans.size(), 2u);
      TS_ASSERT_EQUALS(ans[0], 17);
      TS_ASSERT_EQUALS(ans[1], 7);
      app->EvalIneqCF(app->eval_mngr(), vec, ans);
      TS_ASSERT_EQUALS(ans.size(), 4u);

   }

   void test_get_bounds1()
   {
      TS_TRACE("--------------------------------------------------");
      TS_TRACE("get_bounds1 - Test the method for getting ");
      TS_TRACE("    constraint bounds");
      TS_TRACE("--------------------------------------------------");

      std::vector< colin::real > lower, upper;
      app->get_constraint_bounds(lower, upper);

      std::vector< colin::real > lans(6), uans(6);
      lans[0] = 0.0;
      lans[1] = real::negative_infinity;
      lans[2] = 2.2;
      lans[3] = 0.0;
      lans[4] = real::negative_infinity;
      lans[5] = 0.2;

      uans[0] = real::positive_infinity;
      uans[1] = 1.1;
      uans[2] = 2.2;
      uans[3] = real::positive_infinity;
      uans[4] = 1.1;
      uans[5] = 1.2;
      TS_ASSERT_EQUALS(lower, lans);
      TS_ASSERT_EQUALS(upper, uans);
   }

   void test_get_bound2()
   {
      TS_TRACE("--------------------------------------------------");
      TS_TRACE("get_bounds2 - Test the method for getting ");
      TS_TRACE("    constraint bounds - no constraints");
      TS_TRACE("--------------------------------------------------");

      /*
      std::string foo = "dummy";
      colin::ConfigurableApplication<colin::NLP1_problem> *local_app 
         = colin::new_application(foo, &fn4);
      prob.set_application(local_app);
      local_app->num_real_vars = 3;
      */
      app->num_linear_constraints = 0;
      app->num_nonlinear_constraints = 0;

      std::vector< colin::real > lower, upper;
      app->get_constraint_bounds(lower, upper);

      std::vector< colin::real > lans, uans;
      TS_ASSERT_EQUALS(lower, lans);
      TS_ASSERT_EQUALS(upper, uans);

      //delete local_app;
   }

};

} // namespace colin
