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

#include "TestApplications.h"


namespace colin {
namespace unittest { class Application_Gradient; }

class colin::unittest::Application_Gradient : public CxxTest::TestSuite
{
public:

   // The test application
   TestApplications::singleObj_denseCon<NLP1_problem> *app;

   void setUp()
   {
      utilib::exception_mngr::set_mode(utilib::exception_mngr::Standard);
      app = new TestApplications::singleObj_denseCon<NLP1_problem>(3);
   }

   void tearDown()
   {
      delete app;
   }

   void test_evalg()
   {
      TS_TRACE("------------------------------------------------");
      TS_TRACE("evalg - Evaluate a gradient");
      TS_TRACE("------------------------------------------------");

      std::vector<double> vec(3);
      vec[0] = 1;
      vec[1] = 2;
      vec[2] = 4;

      colin::real ans;
      utilib::BasicArray<colin::real> gradient;

      utilib::BasicArray<colin::real> grad_ans(3);
      grad_ans[0] = 8.0;
      grad_ans[1] = 4.0;
      grad_ans[2] = 2.0;
      app->EvalG(app->eval_mngr(), vec, gradient);
      TS_ASSERT_EQUALS(gradient, grad_ans);
   }

   void test_async_evalg()
   {
      TS_TRACE("------------------------------------------------");
      TS_TRACE("async_evalg - Evaluate a gradient asynchronously");
      TS_TRACE("------------------------------------------------");

      std::vector<double> vec(3);
      vec[0] = 1;
      vec[1] = 2;
      vec[2] = 4;

      colin::real ans;
      utilib::BasicArray<colin::real> gradient;

      utilib::BasicArray<colin::real> grad_ans(3);
      grad_ans[0] = 8.0;
      grad_ans[1] = 4.0;
      grad_ans[2] = 2.0;
      app->AsyncEvalG(app->eval_mngr(), vec, gradient);
      app->synchronize();
      TS_ASSERT_EQUALS(gradient, grad_ans);
   }

};

} // namespace colin
