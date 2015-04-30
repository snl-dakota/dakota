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

#include <colin/reformulation/ConstraintPenalty.h>

#include <cxxtest/TestSuite.h>

#include "TestApplications.h"
#include "CommonTestUtils.h"

namespace colin {
namespace unittest { class ConstraintPenaltyApplication; }

class colin::unittest::ConstraintPenaltyApplication : public CxxTest::TestSuite
{
public:
   void test_construct()
   {
      TestApplications::singleObj_denseCon<NLP0_problem> base;
      base.configure(3);

      colin::ConstraintPenaltyApplication<UNLP0_problem> app1;
      TS_ASSERT_EQUALS(app1.domain_size, 0);
      TS_ASSERT_EQUALS(app1.constraint_penalty, 1000);
      
      app1.reformulate_application(base);
      TS_ASSERT_EQUALS(app1.domain_size, 3);

      colin::ConstraintPenaltyApplication<UNLP0_problem> app2(base);
      TS_ASSERT_EQUALS(app2.domain_size, 3);
   }


   void test_invalid_construct()
   {
      TestApplications::singleObj_denseCon<NLP1_problem> base;

      TS_ASSERT_THROWS_ASSERT
         ( colin::ConstraintPenaltyApplication<UNLP0_problem> app1(base), 
           std::runtime_error &e,
           TEST_WHAT(e, "ConstraintPenaltyApplication::"
                     "validate_reformulated_application(): invalid base "
                     "application type NLP1 for "
                     "ConstraintPenaltyApplication<UNLP0>" ) );

      TS_ASSERT_THROWS_ASSERT
         ( colin::ConstraintPenaltyApplication<NLP1_problem> app2(base), 
           std::runtime_error &e,
           TEST_WHAT(e, "ConstraintPenaltyApplication::"
                     "validate_reformulated_application(): invalid base "
                     "application type NLP1 for "
                     "ConstraintPenaltyApplication<NLP1>" ) );

      TestApplications::singleObj_denseCon<UNLP1_problem> u_base;

      TS_ASSERT_THROWS_ASSERT
         ( colin::ConstraintPenaltyApplication<UNLP1_problem> app3(u_base), 
           std::runtime_error &e,
           TEST_WHAT(e, "ConstraintPenaltyApplication::"
                     "validate_reformulated_application(): invalid base "
                     "application type UNLP1 for "
                     "ConstraintPenaltyApplication<UNLP1>" ) );
   }

   void test_domain_update()
   {
      TestApplications::singleObj_denseCon<NLP0_problem> base;
      colin::ConstraintPenaltyApplication<UNLP0_problem> app(base);

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

   void test_objective()
   {
      TestApplications::singleObj_denseCon<NLP0_problem> base;
      colin::ConstraintPenaltyApplication<UNLP0_problem> app(base);

      std::vector<double> x(3);
      x[0] = 1;
      x[1] = 2;
      x[2] = 4;

      colin::real scalar;
      std::vector<double> vector;
      std::vector<double> vec_ans;

      // validate the base test application:
      base.EvalF(x, scalar);
      TS_ASSERT_EQUALS(scalar, 8.0);
      vec_ans.resize(6);
      vec_ans[0] = 0;
      vec_ans[1] = 13;
      vec_ans[2] = -3;
      vec_ans[3] = 0;
      vec_ans[4] = 15.9;
      vec_ans[5] = -8;
      base.EvalCFViol(x, vector);
      TS_ASSERT_EQUALS(vector, vec_ans);

      // test the constraint penalty
      double f = 8. + 1000*(13.*13. + 3.*3. + 15.9*15.9 + 8.*8.); // 494818
      app.EvalF(x, scalar);
      TS_ASSERT_EQUALS(scalar, f);

      //
      // Try another application (and a non-default constraint penalty)
      //
      TestApplications::singleObj_denseCon<NLP0_problem> new_base;
      new_base.configure(4);

      app.reformulate_application(new_base);
      TS_ASSERT_EQUALS(app.domain_size, 4);
      x.push_back(2);

      app.constraint_penalty = 2;

      // validate the base test application:
      new_base.EvalF(x, scalar);
      TS_ASSERT_EQUALS(scalar, 16.0);
      vec_ans.resize(6);
      vec_ans[0] = 0;
      vec_ans[1] = 17;
      vec_ans[2] = 0;
      vec_ans[3] = 0;
      vec_ans[4] = 23.9;
      vec_ans[5] = -6;
      new_base.EvalCFViol(x, vector);
      TS_ASSERT_EQUALS(vector, vec_ans);

      // test the constraint penalty
      f = 16. + 2*(17.*17. + 23.9*23.9 + 6.*6.); // 912.21
      app.EvalF(x, scalar);
      TS_ASSERT_DELTA(scalar, f, 1e-10);
   }

   void test_maximization_objective()
   {
      TestApplications::singleObj_denseCon<NLP0_problem> base;
      colin::ConstraintPenaltyApplication<UNLP0_problem> app(base);

      base.sense = maximization;

      std::vector<double> x(3);
      x[0] = 1;
      x[1] = 2;
      x[2] = 4;

      colin::real scalar;
      std::vector<double> vector;
      std::vector<double> vec_ans;

      // test the constraint penalty
      double f = 8. - 1000*(13.*13. + 3.*3. + 15.9*15.9 + 8.*8.); // -494802
      app.EvalF(x, scalar);
      TS_ASSERT_EQUALS(scalar, f);
   }

   void test_gradient()
   {
      TestApplications::singleObj_denseCon<NLP1_problem> base;
      colin::ConstraintPenaltyApplication<UNLP1_problem> app(base);

      std::vector<double> x(3);
      x[0] = 1;
      x[1] = 2;
      x[2] = 4;

      utilib::BasicArray<utilib::BasicArray<double> > mat;
      std::vector<double> vec;

      // validate the base application gradients
      std::vector<double> grad(3);
      grad[0] = 8;
      grad[1] = 4;
      grad[2] = 2;
      base.EvalG(x, vec);
      TS_ASSERT_EQUALS(vec, grad);

      base.EvalCG(app.eval_mngr(), x, mat);
      TS_ASSERT_EQUALS(mat.size(), 6u);
      TS_ASSERT_EQUALS(mat[0].size(), 3u);
      if ( ! CxxTest::tracker().testFailed() )
      {
         TS_ASSERT_EQUALS(mat[0][0], 1.);
         TS_ASSERT_EQUALS(mat[0][1], 1.);
         TS_ASSERT_EQUALS(mat[0][2], 1.);
         TS_ASSERT_EQUALS(mat[1][0], 2.);
         TS_ASSERT_EQUALS(mat[1][1], 2.);
         TS_ASSERT_EQUALS(mat[1][2], 2.);
         TS_ASSERT_EQUALS(mat[2][0], 1.);
         TS_ASSERT_EQUALS(mat[2][1], 2.);
         TS_ASSERT_EQUALS(mat[2][2], 3.);
         TS_ASSERT_EQUALS(mat[3][0], 2.);
         TS_ASSERT_EQUALS(mat[3][1], 4.);
         TS_ASSERT_EQUALS(mat[3][2], 8.);
         TS_ASSERT_EQUALS(mat[4][0], 1.);
         TS_ASSERT_EQUALS(mat[4][1], 2.);
         TS_ASSERT_EQUALS(mat[4][2], 3.);
         TS_ASSERT_EQUALS(mat[5][0], 1.);
         TS_ASSERT_EQUALS(mat[5][1], 1.);
         TS_ASSERT_EQUALS(mat[5][2], 1.);
      }

      std::vector<double> cv(6);
      cv[0] = 0;
      cv[1] = 13;
      cv[2] = -3;
      cv[3] = 0;
      cv[4] = 15.9;
      cv[5] = -8;
      base.EvalCFViol(x, vec);
      TS_ASSERT_EQUALS(vec, cv);

      std::vector<double> penalty_grad(3);
      // g + p*2*cf*cg ...
      // 8 + 2*0*1 + 2*13*2 + 2*-3*1 + 2*0*2 + 2*15.9*1 + 2*-8*1
      // 4 + 2*0*1 + 2*13*2 + 2*-3*2 + 2*0*4 + 2*15.9*2 + 2*-8*1
      // 2 + 2*0*1 + 2*13*2 + 2*-3*3 + 2*0*8 + 2*15.9*3 + 2*-8*1
      penalty_grad[0] = 8. + 1000*((52. -  6.) + (31.8 - 16.));
      penalty_grad[1] = 4. + 1000*((52. - 12.) + (63.6 - 16.));
      penalty_grad[2] = 2. + 1000*((52. - 18.) + (95.4 - 16.));

      app.EvalG(x, vec);
      // Compare w/ delta because of round-off error...
      TS_ASSERT_DELTA(vec, penalty_grad, 1e-10);

      /// change the penalty
      app.constraint_penalty = 2;

      penalty_grad[0] = 8. + 2.*((52. -  6.) + (31.8 - 16.));
      penalty_grad[1] = 4. + 2.*((52. - 12.) + (63.6 - 16.));
      penalty_grad[2] = 2. + 2.*((52. - 18.) + (95.4 - 16.));

      app.EvalG(x, vec);
      // Compare w/ delta because of round-off error...
      TS_ASSERT_DELTA(vec, penalty_grad, 1e-10);
   }

   void test_maximization_gradient()
   {
      TestApplications::singleObj_denseCon<NLP1_problem> base;
      colin::ConstraintPenaltyApplication<UNLP1_problem> app(base);

      base.sense = maximization;

      std::vector<double> x(3);
      x[0] = 1;
      x[1] = 2;
      x[2] = 4;

      std::vector<double> vec;

      std::vector<double> penalty_grad(3);
      // g + p*2*cf*cg ...
      // 8 + 2*0*1 + 2*13*2 + 2*-3*1 + 2*0*2 + 2*15.9*1 + 2*-8*1
      // 4 + 2*0*1 + 2*13*2 + 2*-3*2 + 2*0*4 + 2*15.9*2 + 2*-8*1
      // 2 + 2*0*1 + 2*13*2 + 2*-3*3 + 2*0*8 + 2*15.9*3 + 2*-8*1
      penalty_grad[0] = 8. - 1000*((52. -  6.) + (31.8 - 16.));
      penalty_grad[1] = 4. - 1000*((52. - 12.) + (63.6 - 16.));
      penalty_grad[2] = 2. - 1000*((52. - 18.) + (95.4 - 16.));

      app.EvalG(x, vec);
      // Compare w/ delta because of round-off error...
      TS_ASSERT_DELTA(vec, penalty_grad, 1e-10);

      /// change the penalty
      app.constraint_penalty = 2;

      penalty_grad[0] = 8. - 2.*((52. -  6.) + (31.8 - 16.));
      penalty_grad[1] = 4. - 2.*((52. - 12.) + (63.6 - 16.));
      penalty_grad[2] = 2. - 2.*((52. - 18.) + (95.4 - 16.));

      app.EvalG(x, vec);
      // Compare w/ delta because of round-off error...
      TS_ASSERT_DELTA(vec, penalty_grad, 1e-10);
   }

};

} // namespace colin
