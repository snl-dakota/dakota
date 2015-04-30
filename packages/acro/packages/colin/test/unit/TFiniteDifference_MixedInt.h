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

#include <colin/reformulation/FiniteDifference.h>

#include <cxxtest/TestSuite.h>

#include "TestApplications.h"
#include "CommonTestUtils.h"

namespace colin {
namespace unittest { class FiniteDifferenceApplication_MixedIntDomain; }

class colin::unittest::FiniteDifferenceApplication_MixedIntDomain       \
  : public CxxTest::TestSuite
{
   std::vector<double>  x;
   double  delta;

public:
   FiniteDifferenceApplication_MixedIntDomain()
      : x(3)
   {
      x[0] = 1;
      x[1] = 2;
      x[2] = 4;

      delta = 1e-5;
   }

public:
   void test_construct()
   {
      TestApplications::singleObj_denseCon<MINLP0_problem> base;
      base.configure(3);

      colin::FiniteDifferenceApplication<MINLP1_problem> app1;
      TS_ASSERT_EQUALS(app1.domain_size, 0);
      
      app1.reformulate_application(base);
      TS_ASSERT_EQUALS(app1.domain_size, 3);

      colin::FiniteDifferenceApplication<MINLP1_problem> app2(base);
      TS_ASSERT_EQUALS(app2.domain_size, 3);
   }

   void test_invalid_construct()
   {
      TestApplications::singleObj_denseCon<MINLP1_problem> base;

      TS_ASSERT_THROWS_ASSERT
         ( colin::FiniteDifferenceApplication<MINLP1_problem> app1(base),
           std::runtime_error &e,
           TEST_WHAT(e, "FiniteDifferenceApplication_Core::"
                     "reformulate_application(): invalid base "
                     "application type MINLP1 for "
                     "FiniteDifferenceApplication<MINLP1>" ) );

      TS_ASSERT_THROWS_ASSERT
         ( colin::FiniteDifferenceApplication<MINLP0_problem> app3, 
           std::logic_error &e,
           TEST_WHAT(e, "FiniteDifferenceApplication - Attempting to create "
                     "finite difference for an application without gratient "
                     "information?!?") );
      TS_ASSERT_THROWS_ASSERT
         ( colin::FiniteDifferenceApplication<MINLP2_problem> app4, 
           std::logic_error &e,
           TEST_WHAT(e, "FiniteDifferenceApplication - "
                     "Does not support calculating Hessian information") );
   }

   void test_f()
   {
      TestApplications::singleObj_denseCon<MINLP0_problem> base;
      colin::FiniteDifferenceApplication<MINLP1_problem> app(base);

      double ans;
      app.EvalF(x, ans);
      TS_ASSERT_EQUALS(ans, 8.);
   }

   void test_g()
   {
      TestApplications::singleObj_denseCon<MINLP0_problem> base;
      colin::FiniteDifferenceApplication<MINLP1_problem> app(base);

      std::vector<double> g(3);
      g[0] = 8;
      g[1] = 4;
      g[2] = 2;
      std::vector<double> vec;
      app.EvalG(x, vec);
      TS_ASSERT_DELTA(vec, g, delta);
      TS_ASSERT_EQUALS(base.eval_count(), 4);

      app.difference_mode = FiniteDifferenceApplication_Core::backward;
      vec.clear();
      app.EvalG(x, vec);
      TS_ASSERT_DELTA(vec, g, delta);
      TS_ASSERT_EQUALS(base.eval_count(), 8);

      app.difference_mode = FiniteDifferenceApplication_Core::central;
      vec.clear();
      app.EvalG(x, vec);
      TS_ASSERT_DELTA(vec, g, delta);
      TS_ASSERT_EQUALS(base.eval_count(), 14);
   }

   void test_mo_g()
   {
      TestApplications::multiObj_denseCon<MO_MINLP0_problem> base;
      colin::FiniteDifferenceApplication<MO_MINLP1_problem> app(base);

      std::vector<std::vector<double> > g(2);
      for(size_t i=0; i<g.size(); g[i++].resize(3));
      g[0][0] = 8;
      g[0][1] = 4;
      g[0][2] = 2;
      g[1][0] = -2;
      g[1][1] = -4;
      g[1][2] = -8;
      std::vector<std::vector<double> > mat;
      app.EvalG(x, mat);
      TS_ASSERT_DELTA(mat, g, delta);
      TS_ASSERT_EQUALS(base.eval_count(), 4);

      app.difference_mode = FiniteDifferenceApplication_Core::backward;
      mat.clear();
      app.EvalG(x, mat);
      TS_ASSERT_DELTA(mat, g, delta);
      TS_ASSERT_EQUALS(base.eval_count(), 8);

      app.difference_mode = FiniteDifferenceApplication_Core::central;
      mat.clear();
      app.EvalG(x, mat);
      TS_ASSERT_DELTA(mat, g, delta);
      TS_ASSERT_EQUALS(base.eval_count(), 14);
   }

   void test_cg()
   {
      TestApplications::singleObj_denseCon<SMINLP0_problem> base;
      colin::FiniteDifferenceApplication<SMINLP1_problem> app(base);

      std::vector<std::vector<double> > cg(9);
      for(size_t i=0; i<cg.size(); cg[i++].resize(3));
      cg[0][0] = 1;
      cg[0][1] = 1;
      cg[0][2] = 1;
      cg[1][0] = 2;
      cg[1][1] = 2;
      cg[1][2] = 2;
      cg[2][0] = 1;
      cg[2][1] = 2;
      cg[2][2] = 3;
      cg[3][0] = 2;
      cg[3][1] = 4;
      cg[3][2] = 8;
      cg[4][0] = 1;
      cg[4][1] = 2;
      cg[4][2] = 3;
      cg[5][0] = 1;
      cg[5][1] = 1;
      cg[5][2] = 1;
      cg[6][0] = 2;
      cg[6][1] = 4;
      cg[6][2] = 8;
      cg[7][0] = 1;
      cg[7][1] = 2;
      cg[7][2] = 3;
      cg[8][0] = 1;
      cg[8][1] = 1;
      cg[8][2] = 1;
      std::vector<std::vector<double> > mat;
      app.EvalCG(x, mat);
      TS_ASSERT_DELTA(mat, cg, delta);
      TS_ASSERT_EQUALS(base.eval_count(), 4);

      app.difference_mode = FiniteDifferenceApplication_Core::backward;
      mat.clear();
      app.EvalCG(x, mat);
      TS_ASSERT_DELTA(mat, cg, delta);
      TS_ASSERT_EQUALS(base.eval_count(), 8);

      app.difference_mode = FiniteDifferenceApplication_Core::central;
      mat.clear();
      app.EvalCG(x, mat);
      TS_ASSERT_DELTA(mat, cg, delta);
      TS_ASSERT_EQUALS(base.eval_count(), 14);
   }
};
} // namespace colin
