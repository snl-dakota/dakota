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

#include <colin/reformulation/WeightedSum.h>

#include <cxxtest/TestSuite.h>

#include "TestApplications.h"
#include "CommonTestUtils.h"

namespace colin {
namespace unittest { class WeightedSumApplication; }

class colin::unittest::WeightedSumApplication : public CxxTest::TestSuite
{
public:
   void test_construct()
   {
      TestApplications::multiObj_denseCon<MO_NLP0_problem> base;
      base.configure(3);

      colin::WeightedSumApplication<NLP0_problem> app1;
      TS_ASSERT_EQUALS(app1.domain_size, 0);
      
      app1.reformulate_application(base);
      TS_ASSERT_EQUALS(app1.domain_size, 3);

      colin::WeightedSumApplication<NLP0_problem> app2(base);
      TS_ASSERT_EQUALS(app2.domain_size, 3);
   }


   void test_invalid_construct()
   {
      TestApplications::multiObj_denseCon<MO_NLP1_problem> base;

      TS_ASSERT_THROWS_ASSERT
         ( colin::WeightedSumApplication<NLP0_problem> app1(base), 
           std::runtime_error &e,
           TEST_WHAT(e, "WeightedSumApplication::"
                     "validate_reformulated_application(): invalid base "
                     "application type MO_NLP1 for "
                     "WeightedSumApplication<NLP0>" ) );

      TestApplications::singleObj_denseCon<NLP1_problem> u_base;

      TS_ASSERT_THROWS_ASSERT
         ( colin::WeightedSumApplication<NLP1_problem> app2(u_base), 
           std::runtime_error &e,
           TEST_WHAT(e, "WeightedSumApplication::"
                     "validate_reformulated_application(): invalid base "
                     "application type NLP1 for "
                     "WeightedSumApplication<NLP1>" ) );

      TS_ASSERT_THROWS_ASSERT
         ( colin::WeightedSumApplication<MO_NLP1_problem> app3(base), 
           std::logic_error &e,
           TEST_WHAT(e, "WeightedSumApplication - "
                     "cannot be created as a multi-objective application.") );

      TS_ASSERT_THROWS_ASSERT
         ( colin::WeightedSumApplication<NLP2_problem> app4(base), 
           std::logic_error &e,
           TEST_WHAT(e, "WeightedSumApplication - "
                     "cannot be created with Hessian information.") );
   }

   void test_domain_update()
   {
      TestApplications::multiObj_denseCon<MO_NLP0_problem> base;
      colin::WeightedSumApplication<NLP0_problem> app(base);

      TS_ASSERT_EQUALS(app.domain_size, 3);

      base.configure(4);

      TS_ASSERT_EQUALS(app.domain_size, 4);
      TS_ASSERT_EQUALS(app.realLabel(3), "x_3");
      TS_ASSERT_EQUALS(app.realLabel(2), "x_2");
      TS_ASSERT_EQUALS(app.realLowerBound(2), real::negative_infinity);
      TS_ASSERT_EQUALS(app.realLowerBound(3), -4);
      TS_ASSERT_EQUALS(app.realUpperBound(2), real::positive_infinity);
      TS_ASSERT_EQUALS(app.realUpperBound(3), real::positive_infinity);
      TS_ASSERT_EQUALS(app.realLowerBoundType(2), no_bound);
      TS_ASSERT_EQUALS(app.realLowerBoundType(3), hard_bound);
      TS_ASSERT_EQUALS(app.realUpperBoundType(2), no_bound);
      TS_ASSERT_EQUALS(app.realUpperBoundType(3), no_bound);
   }


   void test_weights()
   {
      TestApplications::multiObj_denseCon<MO_NLP0_problem> base;
      colin::WeightedSumApplication<NLP0_problem> app(base);

      std::vector<double> ans(2,1);
      std::vector<double> w = app.weights;
      TS_ASSERT_EQUALS(w.size(), 2u);
      TS_ASSERT_EQUALS(w, ans);

      ans[0] = 2;
      ans[1] = 3;
      app.weights = ans;
      w.clear();
      w = app.weights;
      TS_ASSERT_EQUALS(w.size(), 2u);
      TS_ASSERT_EQUALS(w, ans);

      ans.push_back(4);
      TS_ASSERT_THROWS_ASSERT
         ( app.weights = ans, std::runtime_error e,
           TEST_WHAT(e, "WeightedSumApplication::cb_validate_weights(): "
                     "new weight vector size (3) does not match the "
                     "number of objectives in the wrapped problem (2)" ) );
   }


   void test_change_nObj()
   {
      TestApplications::multiObj_denseCon<MO_NLP0_problem> base;
      colin::WeightedSumApplication<NLP0_problem> app(base);

      std::vector<double> ans(2);
      ans[0] = 2;
      ans[1] = 3;
      app.weights = ans;

      base.num_objectives = 3;
      ans.push_back(1);
      TS_ASSERT_EQUALS(app.weights, ans);

      base.num_objectives = 1;
      ans.resize(1);
      TS_ASSERT_EQUALS(app.weights, ans);
   }


   void test_f()
   {
      TestApplications::multiObj_denseCon<MO_NLP0_problem> base;
      colin::WeightedSumApplication<NLP0_problem> app(base);

      std::vector<double> domain(3);
      domain[0] = 1;
      domain[1] = 2;
      domain[2] = 3;

      double ans;
      app.EvalF(domain, ans);
      TS_ASSERT_EQUALS(ans, 6.+14.); // 1*(1*2*3) + 1*(1+4+9)

      std::vector<double> weights = app.weights;
      weights[0] = 2;
      weights[1] = 3;
      app.weights = weights;
      app.EvalF(domain, ans);
      TS_ASSERT_EQUALS(ans, 12.+42.); // 2*(1*2*3) + 3*(1+4+9)
   }


   void test_g()
   {
      TestApplications::multiObj_denseCon<MO_NLP1_problem> base;
      colin::WeightedSumApplication<NLP1_problem> app(base);

      std::vector<double> domain(3);
      domain[0] = 1;
      domain[1] = 2;
      domain[2] = 3;

      std::vector<double> g(3);
      g[0] = 6.+2.; // 1*(2*3) - 1*-1*2*1
      g[1] = 3.+4.; // 1*(1*3) - 1*-1*2*2
      g[2] = 2.+6.; // 1*(1*2) - 1*-1*2*3
      std::vector<double> ans;
      app.EvalG(domain, ans);
      TS_ASSERT_EQUALS(ans, g);

      std::vector<double> weights = app.weights;
      weights[0] = 2;
      weights[1] = 3;
      app.weights = weights;
      g[0] = 12.+6. ; // 2*(2*3) - 3*-1*2*1
      g[1] = 6. +12.; // 2*(1*3) - 3*-1*2*2
      g[2] = 4. +18.; // 2*(1*2) - 3*-1*2*3
      app.EvalG(domain, ans);
      TS_ASSERT_EQUALS(ans, g);
   }
};

} // namespace colin
