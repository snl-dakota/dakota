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

#include <colin/reformulation/UnconMultiobj.h>

#include <cxxtest/TestSuite.h>

#include "TestApplications.h"
#include "CommonTestUtils.h"

namespace colin {
namespace unittest { class UnconMultiobjApplication; }

class colin::unittest::UnconMultiobjApplication : public CxxTest::TestSuite
{
public:
   void test_construct()
   {
      TestApplications::multiObj_denseCon<MO_NLP0_problem> base;
      base.configure(3);

      colin::UnconMultiobjApplication<MO_UNLP0_problem> app1;
      TS_ASSERT_EQUALS(app1.domain_size, 0);
      
      app1.reformulate_application(base);
      TS_ASSERT_EQUALS(app1.domain_size, 3);

      colin::UnconMultiobjApplication<MO_UNLP0_problem> app2(base);
      TS_ASSERT_EQUALS(app2.domain_size, 3);
   }


   void test_invalid_construct()
   {
      TestApplications::multiObj_denseCon<MO_NLP1_problem> base;

      TS_ASSERT_THROWS_ASSERT
         ( colin::UnconMultiobjApplication<MO_UNLP0_problem> app1(base), 
           std::runtime_error &e,
           TEST_WHAT(e, "UnconMultiobjApplication::"
                     "validate_reformulated_application(): invalid base "
                     "application type MO_NLP1 for "
                     "UnconMultiobjApplication<MO_UNLP0>" ) );

      TestApplications::multiObj_denseCon<MO_UNLP1_problem> u_base;

      TS_ASSERT_THROWS_ASSERT
         ( colin::UnconMultiobjApplication<MO_UNLP1_problem> app2(u_base), 
           std::runtime_error &e,
           TEST_WHAT(e, "UnconMultiobjApplication::"
                     "validate_reformulated_application(): invalid base "
                     "application type MO_UNLP1 for "
                     "UnconMultiobjApplication<MO_UNLP1>" ) );

      //TS_ASSERT_THROWS_ASSERT
      //   ( colin::UnconMultiobjApplication<UNLP1_problem> app3, 
      //     std::logic_error &e,
      //     TEST_WHAT(e, "UnconMultiobjApplication - cannot be "
      //               "instantiated as a single-objective application.") );

      TS_ASSERT_THROWS_ASSERT
         ( colin::UnconMultiobjApplication<MO_UNLP2_problem> app4, 
           std::logic_error &e,
           TEST_WHAT(e, "UnconMultiobjApplication - cannot be "
                     "instantiated with Hessian information.") );
   }

   void test_domain_update()
   {
      TestApplications::multiObj_denseCon<MO_NLP0_problem> base;
      colin::UnconMultiobjApplication<MO_UNLP0_problem> app(base);

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


   void test_change_nObj()
   {
      TestApplications::multiObj_denseCon<MO_NLP0_problem> base;
      colin::UnconMultiobjApplication<MO_UNLP0_problem> app(base);

      std::vector<optimizationSense> sense(3, colin::minimization);
      sense[1] = colin::maximization;

      TS_ASSERT_EQUALS(base.num_objectives, 2);
      TS_ASSERT_EQUALS(app.num_objectives, 3);
      TS_ASSERT_EQUALS(app.sense, sense);

      base.num_objectives = 3;
      sense.push_back(colin::minimization);
      TS_ASSERT_EQUALS(base.num_objectives, 3);
      TS_ASSERT_EQUALS(app.num_objectives, 4);
      TS_ASSERT_EQUALS(app.sense, sense);

      base.num_objectives = 1;
      sense.resize(2);
      sense[1] = colin::minimization;
      TS_ASSERT_EQUALS(base.num_objectives, 1);
      TS_ASSERT_EQUALS(app.num_objectives, 2);
      TS_ASSERT_EQUALS(app.sense, sense);

      base.num_linear_constraints = 0;
      TS_ASSERT_EQUALS(base.num_objectives, 1);
      TS_ASSERT_EQUALS(app.num_objectives, 2);
      TS_ASSERT_EQUALS(app.sense, sense);

      base.num_nonlinear_constraints = 0;
      sense.resize(1);
      TS_ASSERT_EQUALS(base.num_objectives, 1);
      TS_ASSERT_EQUALS(app.num_objectives, 1);
      TS_ASSERT_EQUALS(app.sense, sense);
   }


   void test_nonD()
   {
      TestApplications::multiObj_denseCon<SMO_NLP0_problem> base;
      std::vector<bool> nond = base.nond_objective;
      nond[4] = true;
      base.nond_objective = nond;

      colin::UnconMultiobjApplication<SMO_UNLP0_problem> app(base);
      nond.resize(3);

      base.num_nondeterministic_constraints = 0;

      TS_ASSERT_EQUALS(base.num_objectives, 2);
      TS_ASSERT_EQUALS(app.num_objectives, 3);
      TS_ASSERT_EQUALS(app.nond_objective, nond);

      base.num_nondeterministic_constraints = 1;
      nond[2] = true;
      TS_ASSERT_EQUALS(app.nond_objective, nond);

      base.num_nondeterministic_constraints = 0;
      nond[2] = false;
      TS_ASSERT_EQUALS(app.nond_objective, nond);

      base.num_objectives = 3;
      TS_ASSERT_EQUALS(base.num_objectives, 3);
      TS_ASSERT_EQUALS(app.num_objectives, 4);

      base.num_objectives = 1;
      TS_ASSERT_EQUALS(base.num_objectives, 1);
      TS_ASSERT_EQUALS(app.num_objectives, 2);

      base.num_linear_constraints = 0;
      TS_ASSERT_EQUALS(base.num_objectives, 1);
      TS_ASSERT_EQUALS(app.num_objectives, 2);

      base.num_nonlinear_constraints = 0;
      TS_ASSERT_EQUALS(base.num_objectives, 1);
      TS_ASSERT_EQUALS(app.num_objectives, 1);

      base.num_nondeterministic_constraints = 1;
      nond.resize(2);
      nond[1] = true;
      TS_ASSERT_EQUALS(base.num_objectives, 1);
      TS_ASSERT_EQUALS(app.num_objectives, 2);
      TS_ASSERT_EQUALS(app.nond_objective, nond);

      base.num_nondeterministic_constraints = 0;
      nond.resize(1);
      TS_ASSERT_EQUALS(base.num_objectives, 1);
      TS_ASSERT_EQUALS(app.num_objectives, 1);
      TS_ASSERT_EQUALS(app.nond_objective, nond);
   }


   void test_mf()
   {
      TestApplications::multiObj_denseCon<MO_NLP0_problem> base;
      colin::UnconMultiobjApplication<MO_UNLP0_problem> app(base);

      std::vector<double> domain(3);
      domain[0] = 1;
      domain[1] = 2;
      domain[2] = 3;

      std::vector<double> mf(3);
      mf[0] =   6.; // (1*2*3)
      mf[1] = -14.; // -1*(1+4+9)
      mf[2] = 404.41; 
      // Lin:     [6] + (12-1)^2 + (14-20)^2 + 
      // NonLin: [14] + (14-1.1)^2 + (6-15)^2

      std::vector<double> ans;
      app.EvalMF(domain, ans);
      TS_ASSERT_DELTA(ans[0], mf[0], 1e-10); 
      TS_ASSERT_DELTA(ans[1], mf[1], 1e-10); 
      TS_ASSERT_DELTA(ans[2], mf[2], 1e-10); 
   }


   void test_g()
   {
      TestApplications::multiObj_denseCon<MO_NLP1_problem> base;
      colin::UnconMultiobjApplication<MO_UNLP1_problem> app(base);

      std::vector<double> domain(3);
      domain[0] = 1;
      domain[1] = 2;
      domain[2] = 3;

      std::vector<std::vector<double> > g(3);
      for(size_t i=0; i<g.size(); g[i++].resize(3));
      g[0][0] =  6.; // (2*3)
      g[0][1] =  3.; // (1*3)
      g[0][2] =  2.; // (1*2)
      g[1][0] = -2.; // -1*2*1
      g[1][1] = -4.; // -1*2*2
      g[1][2] = -6.; // -1*2*3
      g[2][0] = 39.8; // [] + 2*11*2 + 2*-6*1 + [] + 2*12.9*1 + 2*-9*1
      g[2][1] = 53.6; // [] + 2*11*2 + 2*-6*2 + [] + 2*12.9+2 + 2*-9
      g[2][2] = 67.4; // [] + 2*11*2 + 2*-6*3 + [] + 2*12.9*3 + 2*-9
      std::vector<std::vector<double> > ans;
      app.EvalG(domain, ans);
      TS_ASSERT_EQUALS(ans, g);
   }


   void test_mf_noConstraints()
   {
      TestApplications::multiObj_denseCon<MO_NLP0_problem> base;
      colin::UnconMultiobjApplication<MO_UNLP0_problem> app(base);
      base.num_nonlinear_constraints = 0;
      base.num_linear_constraints = 0;

      std::vector<double> domain(3);
      domain[0] = 1;
      domain[1] = 2;
      domain[2] = 3;

      std::vector<double> mf(2);
      mf[0] =   6.; // (1*2*3)
      mf[1] = -14.; // -1*(1+4+9)

      std::vector<double> ans;
      app.EvalMF(domain, ans);
      TS_ASSERT_DELTA(ans[0], mf[0], 1e-10); 
      TS_ASSERT_DELTA(ans[1], mf[1], 1e-10); 
   }


   void test_g_noConstraints()
   {
      TestApplications::multiObj_denseCon<MO_NLP1_problem> base;
      colin::UnconMultiobjApplication<MO_UNLP1_problem> app(base);
      base.num_nonlinear_constraints = 0;
      base.num_linear_constraints = 0;

      std::vector<double> domain(3);
      domain[0] = 1;
      domain[1] = 2;
      domain[2] = 3;

      std::vector<std::vector<double> > g(2);
      for(size_t i=0; i<g.size(); g[i++].resize(3));
      g[0][0] =  6.; // (2*3)
      g[0][1] =  3.; // (1*3)
      g[0][2] =  2.; // (1*2)
      g[1][0] = -2.; // -1*2*1
      g[1][1] = -4.; // -1*2*2
      g[1][2] = -6.; // -1*2*3
      std::vector<std::vector<double> > ans;
      app.EvalG(domain, ans);
      TS_ASSERT_EQUALS(ans, g);
   }
};

} // namespace colin
