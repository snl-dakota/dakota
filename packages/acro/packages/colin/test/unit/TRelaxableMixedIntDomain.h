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
#include <colin/reformulation/RelaxableMixedIntDomain.h>

#include "CommonTestUtils.h"
#include "TestApplications.h"


using utilib::BasicArray;
using utilib::TypeManager;
using utilib::Any;

#if 0
#define MAT         utilib::RMSparseMatrix<double>
#define MATVAL(r,c) matrix(r,c)
#define MATROW      matrix.get_nrows()
#define MATCOL      matrix.get_ncols()
#define SIGN(x)     x
#else
#define MAT         BasicArray<BasicArray<double> >
#define MATVAL(r,c) matrix[r][c]
#define MATROW      matrix.size()
#define MATCOL      matrix[0].size()
#define SIGN(x)     x##u
#endif

namespace colin {
namespace unittest { class RelaxableMixedIntDomain; }

class colin::unittest::RelaxableMixedIntDomain : public CxxTest::TestSuite
{
public:
   void test_construction()
   {
      TestApplications::singleObj_denseCon<NLP1_problem> base(6);

      colin::RelaxableMixedIntDomainApplication<MINLP1_problem> app;
      TS_ASSERT_EQUALS(app.num_real_vars,   0);
      TS_ASSERT_EQUALS(app.num_int_vars,    0);
      TS_ASSERT_EQUALS(app.num_binary_vars, 0);

      app.reformulate_application(base);
      TS_ASSERT_EQUALS(app.num_real_vars,   6);
      TS_ASSERT_EQUALS(app.num_int_vars,    0);
      TS_ASSERT_EQUALS(app.num_binary_vars, 0);

      TestApplications::singleObj_denseCon<NLP2_problem> alt_base1;
      TS_ASSERT_THROWS_ASSERT
         ( app.reformulate_application(alt_base1), 
           std::runtime_error &e,
           TEST_WHAT(e, "RelaxableMixedIntDomainApplication::"
                     "validate_reformulated_application(): invalid base "
                     "application type NLP2 for "
                     "RelaxableMixedIntDomainApplication<MINLP1>" ) );

      TestApplications::singleObj_denseCon<MINLP1_problem> alt_base2;
      TS_ASSERT_THROWS_ASSERT
         ( app.reformulate_application(alt_base2), 
           std::runtime_error &e,
           TEST_WHAT(e, "RelaxableMixedIntDomainApplication::"
                     "validate_reformulated_application(): invalid base "
                     "application type MINLP1 for "
                     "RelaxableMixedIntDomainApplication<MINLP1>" ) );

   }

   void test_set_discrete_domain()
   {
      TestApplications::singleObj_denseCon<NLP1_problem> base(6);
      colin::RelaxableMixedIntDomainApplication<MINLP1_problem> app(base);

      app.set_discrete_domain(1, 3);
      TS_ASSERT_EQUALS(app.num_real_vars,   2);
      TS_ASSERT_EQUALS(app.num_int_vars,    3);
      TS_ASSERT_EQUALS(app.num_binary_vars, 1);

      TS_ASSERT_THROWS_ASSERT
         ( app.set_discrete_domain(4, 3), std::runtime_error &e,
           TEST_WHAT(e, "RelaxableMixedIntDomainApplication::"
                     "set_discrete_domain: incompatible discrete domain "
                     "(4 + 3 > 6)" ) );
   }

   void test_domain_maps()
   {
      TestApplications::singleObj_denseCon<NLP1_problem> base(6);
      colin::RelaxableMixedIntDomainApplication<MINLP1_problem> app(base);
      app.set_discrete_domain(1, 3);

      std::vector<real> base_domain(6);
      utilib::MixedIntVars mi_domain(1,3,2);

      // Simple test of the forward map

      mi_domain.Binary()[0] = false;
      mi_domain.Integer()[0] = 1; 
      mi_domain.Integer()[1] = 2;
      mi_domain.Integer()[2] = 3;
      mi_domain.Real()[0] = 4;
      mi_domain.Real()[1] = 5;

      Any ans;
      TS_ASSERT( app.map_domain(mi_domain, ans, true) );
      TypeManager()->lexical_cast(ans, base_domain);
      TS_ASSERT_EQUALS(base_domain[0], 0);
      TS_ASSERT_EQUALS(base_domain[1], 1);
      TS_ASSERT_EQUALS(base_domain[2], 2);
      TS_ASSERT_EQUALS(base_domain[3], 3);
      TS_ASSERT_EQUALS(base_domain[4], 4);
      TS_ASSERT_EQUALS(base_domain[5], 5);

      // Simple test of the exact back map

      base_domain[0] = 1;
      base_domain[1] = 2;
      base_domain[2] = 4;
      base_domain[3] = 6;
      base_domain[4] = 14;
      base_domain[5] = 15;

      mi_domain.Binary()  << false;
      mi_domain.Integer() << 0;
      mi_domain.Real()    << 0.;

      TS_ASSERT( app.map_domain(base_domain, ans, false) );
      TypeManager()->lexical_cast(ans, mi_domain);
      TS_ASSERT_EQUALS(mi_domain.Binary()[0],  true );
      TS_ASSERT_EQUALS(mi_domain.Integer()[0], 2  ); 
      TS_ASSERT_EQUALS(mi_domain.Integer()[1], 4  );
      TS_ASSERT_EQUALS(mi_domain.Integer()[2], 6  );
      TS_ASSERT_EQUALS(mi_domain.Real()[0],    14 );
      TS_ASSERT_EQUALS(mi_domain.Real()[1],    15 );

      // Simple test of the approximate back map

      base_domain[0] = 1;
      base_domain[1] = 2.5;
      base_domain[2] = 4;
      base_domain[3] = 6;
      base_domain[4] = 14;
      base_domain[5] = 15.5;

      mi_domain.Binary()  << false;
      mi_domain.Integer() << 0;
      mi_domain.Real()    << 0.;

      TS_ASSERT( ! app.map_domain(base_domain, ans, false) );
      TypeManager()->lexical_cast(ans, mi_domain);
      TS_ASSERT_EQUALS(mi_domain.Binary()[0],  true );
      TS_ASSERT_EQUALS(mi_domain.Integer()[0], 2  ); 
      TS_ASSERT_EQUALS(mi_domain.Integer()[1], 4  );
      TS_ASSERT_EQUALS(mi_domain.Integer()[2], 6  );
      TS_ASSERT_EQUALS(mi_domain.Real()[0],    14 );
      TS_ASSERT_EQUALS(mi_domain.Real()[1],    15.5 );
   }

   void test_f()
   {
      TestApplications::singleObj_denseCon<NLP1_problem> base(6);
      colin::RelaxableMixedIntDomainApplication<MINLP1_problem> app(base);

      std::vector<real> r_domain(6);
      for( size_t i = 0; i < 6; ++i )
         r_domain[i] = i+1;
      TS_ASSERT_EQUALS(base.EvalF(r_domain).get(f_info), 720.);
      TS_ASSERT_EQUALS(app.EvalF(r_domain).get(f_info), 720.);

      app.set_discrete_domain(1, 3);

      utilib::MixedIntVars mi_domain(1,3,2);
      mi_domain.Binary()[0] = false;
      mi_domain.Integer()[0] = 1; 
      mi_domain.Integer()[1] = 2;
      mi_domain.Integer()[2] = 3;
      mi_domain.Real()[0] = 4;
      mi_domain.Real()[1] = 5;
      TS_ASSERT_EQUALS(app.EvalF(mi_domain).get(f_info), 0.);

      mi_domain.Binary()[0] = true;
      TS_ASSERT_EQUALS(app.EvalF(mi_domain).get(f_info), 120.);

      mi_domain.Binary()[0] = true;
      mi_domain.Real()[0] = 1;
      mi_domain.Real()[1] = 1;
      mi_domain.Integer()[0] = 1; 
      mi_domain.Integer()[1] = 1;
      mi_domain.Integer()[2] = 1;
      TS_ASSERT_EQUALS(app.EvalF(mi_domain).get(f_info), 1.);

      utilib::MixedIntVars bad_mi_domain(2,3,4);
      TS_ASSERT_THROWS_ASSERT
         ( app.EvalF(bad_mi_domain), std::runtime_error &e,
           TEST_WHAT(e, "RelaxableMixedIntDomainApplication::map_domain(): "
                     "invalid domain: provided (binary, int, real) = " 
                     "(2, 3, 4); expected (1, 3, 2)") );
   }

   void test_mf()
   {
      TestApplications::multiObj_denseCon<MO_NLP1_problem> base(6);
      colin::RelaxableMixedIntDomainApplication<MO_MINLP1_problem> app(base);

      std::vector<real> r_domain(6);
      for( size_t i = 0; i < 6; ++i )
         r_domain[i] = i+1;

      std::vector<double> ans;
      base.EvalMF(r_domain, ans);
      TS_ASSERT_EQUALS(ans[0], 720.);
      TS_ASSERT_EQUALS(ans[1], -91.);
      
      ans.clear();
      app.EvalMF(r_domain, ans);
      TS_ASSERT_EQUALS(ans[0], 720.);
      TS_ASSERT_EQUALS(ans[1], -91.);

      app.set_discrete_domain(1, 3);

      utilib::MixedIntVars mi_domain(1,3,2);
      mi_domain.Binary()[0] = false;
      mi_domain.Integer()[0] = 1; 
      mi_domain.Integer()[1] = 2;
      mi_domain.Integer()[2] = 3;
      mi_domain.Real()[0] = 4;
      mi_domain.Real()[1] = 5;

      ans.clear();
      app.EvalMF(mi_domain, ans);
      TS_ASSERT_EQUALS(ans[0], 0.);
      TS_ASSERT_EQUALS(ans[1], -55.);

      mi_domain.Binary()[0] = true;

      ans.clear();
      app.EvalMF(mi_domain, ans);
      TS_ASSERT_EQUALS(ans[0], 120.);
      TS_ASSERT_EQUALS(ans[1], -56.);

      mi_domain.Binary()[0] = true;
      mi_domain.Real()[0] = 1;
      mi_domain.Real()[1] = 1;
      mi_domain.Integer()[0] = 1; 
      mi_domain.Integer()[1] = 1;
      mi_domain.Integer()[2] = 1;

      ans.clear();
      app.EvalMF(mi_domain, ans);
      TS_ASSERT_EQUALS(ans[0], 1.);
      TS_ASSERT_EQUALS(ans[1], -6.);

      utilib::MixedIntVars bad_mi_domain(2,3,4);
      TS_ASSERT_THROWS_ASSERT
         ( app.set_domain(bad_mi_domain), std::runtime_error &e,
           TEST_WHAT(e, "RelaxableMixedIntDomainApplication::map_domain(): "
                     "invalid domain: provided (binary, int, real) = " 
                     "(2, 3, 4); expected (1, 3, 2)") );
   }

   void test_grad()
   {
      TestApplications::singleObj_denseCon<NLP1_problem> base(6);
      colin::RelaxableMixedIntDomainApplication<MINLP1_problem> app(base);

      std::vector<real> r_domain(6);
      for( size_t i = 0; i < 6; ++i )
         r_domain[i] = i+1;

      std::vector<double> gradient(6), ans;
      gradient[0] = 720;
      gradient[1] = 360;
      gradient[2] = 240;
      gradient[3] = 180;
      gradient[4] = 144;
      gradient[5] = 120;
      base.EvalG(r_domain, ans);
      TS_ASSERT_EQUALS(ans, gradient);
      ans.clear();
      app.EvalG(r_domain, ans);
      TS_ASSERT_EQUALS(ans, gradient);

      app.set_discrete_domain(1, 3);
      gradient.resize(2);
      gradient[0] = 0;
      gradient[1] = 0;

      utilib::MixedIntVars mi_domain(1,3,2);
      mi_domain.Binary()[0] = false;
      mi_domain.Integer()[0] = 1; 
      mi_domain.Integer()[1] = 2;
      mi_domain.Integer()[2] = 3;
      mi_domain.Real()[0] = 4;
      mi_domain.Real()[1] = 5;
      app.EvalG(mi_domain, ans);
      TS_ASSERT_EQUALS(gradient, ans);

      mi_domain.Binary()[0] = true;
      gradient[0] = 30;
      gradient[1] = 24;

      app.EvalG(mi_domain, ans);
      TS_ASSERT_EQUALS(gradient, ans);

      mi_domain.Binary()[0] = true;
      mi_domain.Real()[0] = 1;
      mi_domain.Real()[1] = 1;
      mi_domain.Integer()[0] = 1; 
      mi_domain.Integer()[1] = 1;
      mi_domain.Integer()[2] = 1;

      gradient[0] = 1;
      gradient[1] = 1;

      app.EvalG(mi_domain, ans);
      TS_ASSERT_EQUALS(gradient, ans);
   }

   void test_mo_grad()
   {
      TestApplications::multiObj_denseCon<MO_NLP1_problem> base(6);
      colin::RelaxableMixedIntDomainApplication<MO_MINLP1_problem> app(base);

      std::vector<real> r_domain(6);
      for( size_t i = 0; i < 6; ++i )
         r_domain[i] = i+1;

      std::vector<std::vector<double> > gradient(2), ans;
      gradient[0].resize(6);
      gradient[1].resize(6);
      gradient[0][0] = 720;
      gradient[0][1] = 360;
      gradient[0][2] = 240;
      gradient[0][3] = 180;
      gradient[0][4] = 144;
      gradient[0][5] = 120;
      gradient[1][0] = -2;
      gradient[1][1] = -4;
      gradient[1][2] = -6;
      gradient[1][3] = -8;
      gradient[1][4] = -10;
      gradient[1][5] = -12;
      base.EvalG(r_domain, ans);
      TS_ASSERT_EQUALS(gradient, ans);
      ans.clear();
      app.EvalG(r_domain, ans);
      TS_ASSERT_EQUALS(gradient, ans);

      app.set_discrete_domain(1, 3);
      gradient[0].resize(2);
      gradient[1].resize(2);
      gradient[0][0] = 0;
      gradient[0][1] = 0;
      gradient[1][0] = -8;
      gradient[1][1] = -10;

      utilib::MixedIntVars mi_domain(1,3,2);
      mi_domain.Binary()[0] = false;
      mi_domain.Integer()[0] = 1; 
      mi_domain.Integer()[1] = 2;
      mi_domain.Integer()[2] = 3;
      mi_domain.Real()[0] = 4;
      mi_domain.Real()[1] = 5;
      app.EvalG(mi_domain, ans);
      TS_ASSERT_EQUALS(gradient, ans);

      mi_domain.Binary()[0] = true;
      gradient[0][0] = 30;
      gradient[0][1] = 24;

      app.EvalG(mi_domain, ans);
      TS_ASSERT_EQUALS(gradient, ans);

      mi_domain.Binary()[0] = true;
      mi_domain.Real()[0] = 1;
      mi_domain.Real()[1] = 1;
      mi_domain.Integer()[0] = 1; 
      mi_domain.Integer()[1] = 1;
      mi_domain.Integer()[2] = 1;

      gradient[0][0] = 1;
      gradient[0][1] = 1;
      gradient[1][0] = -2;
      gradient[1][1] = -2;

      app.EvalG(mi_domain, ans);
      TS_ASSERT_EQUALS(gradient, ans);
   }

   void test_nl_jacobian()
   {
      TestApplications::singleObj_denseCon<NLP1_problem> base(6);
      colin::RelaxableMixedIntDomainApplication<MINLP1_problem> app(base);

      std::vector<real> r_domain(6);
      for( size_t i = 0; i < 6; ++i )
         r_domain[i] = i;

      MAT matrix;
      app.EvalNLCG(r_domain, matrix);
      TS_ASSERT_EQUALS(MATROW, SIGN(3));
      TS_ASSERT_EQUALS(MATCOL, SIGN(6));
      if ( ! CxxTest::tracker().testFailed() )
      {
         TS_ASSERT_EQUALS(MATVAL(0,0), 0);
         TS_ASSERT_EQUALS(MATVAL(0,1), 2);
         TS_ASSERT_EQUALS(MATVAL(0,2), 4);
         TS_ASSERT_EQUALS(MATVAL(0,3), 6);
         TS_ASSERT_EQUALS(MATVAL(0,4), 8);
         TS_ASSERT_EQUALS(MATVAL(0,5), 10);
         TS_ASSERT_EQUALS(MATVAL(1,0), 1);
         TS_ASSERT_EQUALS(MATVAL(1,1), 2);
         TS_ASSERT_EQUALS(MATVAL(1,2), 3);
         TS_ASSERT_EQUALS(MATVAL(1,3), 4);
         TS_ASSERT_EQUALS(MATVAL(1,4), 5);
         TS_ASSERT_EQUALS(MATVAL(1,5), 6);
         TS_ASSERT_EQUALS(MATVAL(2,0), 1);
         TS_ASSERT_EQUALS(MATVAL(2,1), 1);
         TS_ASSERT_EQUALS(MATVAL(2,2), 1);
         TS_ASSERT_EQUALS(MATVAL(2,3), 1);
         TS_ASSERT_EQUALS(MATVAL(2,4), 1);
         TS_ASSERT_EQUALS(MATVAL(2,5), 1);
      }

      app.set_discrete_domain(1, 3);

      utilib::MixedIntVars mi_domain(1,3,2);
      mi_domain.Binary()[0] = true;
      mi_domain.Integer()[0] = 2; 
      mi_domain.Integer()[1] = 3;
      mi_domain.Integer()[2] = 4;
      mi_domain.Real()[0] = 0;
      mi_domain.Real()[1] = 1;

      app.EvalNLCG(mi_domain, matrix);
      TS_ASSERT_EQUALS(MATROW, SIGN(3));
      TS_ASSERT_EQUALS(MATCOL, SIGN(2));
      if ( ! CxxTest::tracker().testFailed() )
      {
         TS_ASSERT_EQUALS(MATVAL(0,0), 0);
         TS_ASSERT_EQUALS(MATVAL(0,1), 2);
         TS_ASSERT_EQUALS(MATVAL(1,0), 5);
         TS_ASSERT_EQUALS(MATVAL(1,1), 6);
         TS_ASSERT_EQUALS(MATVAL(2,0), 1);
         TS_ASSERT_EQUALS(MATVAL(2,1), 1);
      }

      // All constraints are inequalities
      app.EvalNLIneqCG(mi_domain, matrix);
      TS_ASSERT_EQUALS(MATROW, SIGN(3));
      TS_ASSERT_EQUALS(MATCOL, SIGN(2));
      if ( ! CxxTest::tracker().testFailed() )
      {
         TS_ASSERT_EQUALS(MATVAL(0,0), 0);
         TS_ASSERT_EQUALS(MATVAL(0,1), 2);
         TS_ASSERT_EQUALS(MATVAL(1,0), 5);
         TS_ASSERT_EQUALS(MATVAL(1,1), 6);
         TS_ASSERT_EQUALS(MATVAL(2,0), 1);
         TS_ASSERT_EQUALS(MATVAL(2,1), 1);
      }

      utilib::RMSparseMatrix<double> empty_matrix;
      app.EvalNLEqCG(mi_domain, empty_matrix);
      TS_ASSERT_EQUALS(empty_matrix.get_nrows(), 0);
      TS_ASSERT_EQUALS(empty_matrix.get_ncols(), 2);

      /// Make an equality constraint
      std::vector<double> v;
      v = base.nonlinear_constraint_lower_bounds;
      v[2] = 5;
      base.nonlinear_constraint_lower_bounds = v;
      v = base.nonlinear_constraint_upper_bounds;
      v[2] = 5;
      base.nonlinear_constraint_upper_bounds = v;

      app.EvalNLIneqCG(mi_domain, matrix);
      TS_ASSERT_EQUALS(MATROW, SIGN(2));
      TS_ASSERT_EQUALS(MATCOL, SIGN(2));
      if ( ! CxxTest::tracker().testFailed() )
      {
         TS_ASSERT_EQUALS(MATVAL(0,0), 0);
         TS_ASSERT_EQUALS(MATVAL(0,1), 2);
         TS_ASSERT_EQUALS(MATVAL(1,0), 5);
         TS_ASSERT_EQUALS(MATVAL(1,1), 6);
      }

       app.EvalNLEqCG(mi_domain, matrix);
      TS_ASSERT_EQUALS(MATROW, SIGN(1));
      TS_ASSERT_EQUALS(MATCOL, SIGN(2));
      if ( ! CxxTest::tracker().testFailed() )
      {
         TS_ASSERT_EQUALS(MATVAL(0,0), 1);
         TS_ASSERT_EQUALS(MATVAL(0,1), 1);
      }    
   }


   void test_l_jacobian()
   {
      TestApplications::singleObj_denseCon<NLP1_problem> base(6);
      colin::RelaxableMixedIntDomainApplication<MINLP1_problem> app(base);

      std::vector<real> r_domain(6);
      for( size_t i = 0; i < 6; ++i )
         r_domain[i] = i;

      MAT matrix;
      app.EvalLCG(r_domain, matrix);
      TS_ASSERT_EQUALS(MATROW, SIGN(3));
      TS_ASSERT_EQUALS(MATCOL, SIGN(6));
      if ( ! CxxTest::tracker().testFailed() )
      {
         TS_ASSERT_EQUALS(MATVAL(0,0), 1);
         TS_ASSERT_EQUALS(MATVAL(0,1), 1);
         TS_ASSERT_EQUALS(MATVAL(0,2), 1);
         TS_ASSERT_EQUALS(MATVAL(0,3), 1);
         TS_ASSERT_EQUALS(MATVAL(0,4), 1);
         TS_ASSERT_EQUALS(MATVAL(0,5), 1);
         TS_ASSERT_EQUALS(MATVAL(1,0), 2);
         TS_ASSERT_EQUALS(MATVAL(1,1), 2);
         TS_ASSERT_EQUALS(MATVAL(1,2), 2);
         TS_ASSERT_EQUALS(MATVAL(1,3), 2);
         TS_ASSERT_EQUALS(MATVAL(1,4), 2);
         TS_ASSERT_EQUALS(MATVAL(1,5), 2);
         TS_ASSERT_EQUALS(MATVAL(2,0), 1);
         TS_ASSERT_EQUALS(MATVAL(2,1), 2);
         TS_ASSERT_EQUALS(MATVAL(2,2), 3);
         TS_ASSERT_EQUALS(MATVAL(2,3), 4);
         TS_ASSERT_EQUALS(MATVAL(2,4), 5);
         TS_ASSERT_EQUALS(MATVAL(2,5), 6);
      }

      app.set_discrete_domain(1, 3);

      utilib::MixedIntVars mi_domain(1,3,2);
      mi_domain.Binary()[0] = true;
      mi_domain.Integer()[0] = 2; 
      mi_domain.Integer()[1] = 3;
      mi_domain.Integer()[2] = 4;
      mi_domain.Real()[0] = 0;
      mi_domain.Real()[1] = 1;

      app.EvalLCG(mi_domain, matrix);
      TS_ASSERT_EQUALS(MATROW, SIGN(3));
      TS_ASSERT_EQUALS(MATCOL, SIGN(2));
      if ( ! CxxTest::tracker().testFailed() )
      {
         TS_ASSERT_EQUALS(MATVAL(0,0), 1);
         TS_ASSERT_EQUALS(MATVAL(0,1), 1);
         TS_ASSERT_EQUALS(MATVAL(1,0), 2);
         TS_ASSERT_EQUALS(MATVAL(1,1), 2);
         TS_ASSERT_EQUALS(MATVAL(2,0), 5);
         TS_ASSERT_EQUALS(MATVAL(2,1), 6);
      }

      // All constraints are inequalities
      app.EvalLIneqCG(mi_domain, matrix);
      TS_ASSERT_EQUALS(MATROW, SIGN(3));
      TS_ASSERT_EQUALS(MATCOL, SIGN(2));
      if ( ! CxxTest::tracker().testFailed() )
      {
         TS_ASSERT_EQUALS(MATVAL(0,0), 1);
         TS_ASSERT_EQUALS(MATVAL(0,1), 1);
         TS_ASSERT_EQUALS(MATVAL(1,0), 2);
         TS_ASSERT_EQUALS(MATVAL(1,1), 2);
         TS_ASSERT_EQUALS(MATVAL(2,0), 5);
         TS_ASSERT_EQUALS(MATVAL(2,1), 6);
      }

      utilib::RMSparseMatrix<double> empty_matrix;
      app.EvalLEqCG(mi_domain, empty_matrix);
      TS_ASSERT_EQUALS(empty_matrix.get_nrows(), 0);
      TS_ASSERT_EQUALS(empty_matrix.get_ncols(), 2);

      /// Make an equality constraint
      std::vector<double> v;
      v = base.linear_constraint_lower_bounds;
      v[0] = 5;
      base.linear_constraint_lower_bounds = v;
      v = base.linear_constraint_upper_bounds;
      v[0] = 5;
      base.linear_constraint_upper_bounds = v;

      app.EvalLIneqCG(mi_domain, matrix);
      TS_ASSERT_EQUALS(MATROW, SIGN(2));
      TS_ASSERT_EQUALS(MATCOL, SIGN(2));
      if ( ! CxxTest::tracker().testFailed() )
      {
         TS_ASSERT_EQUALS(MATVAL(0,0), 2);
         TS_ASSERT_EQUALS(MATVAL(0,1), 2);
         TS_ASSERT_EQUALS(MATVAL(1,0), 5);
         TS_ASSERT_EQUALS(MATVAL(1,1), 6);
      }

      app.EvalLEqCG(mi_domain, matrix);
      TS_ASSERT_EQUALS(MATROW, SIGN(1));
      TS_ASSERT_EQUALS(MATCOL, SIGN(2));
      if ( ! CxxTest::tracker().testFailed() )
      {
         TS_ASSERT_EQUALS(MATVAL(0,0), 1);
         TS_ASSERT_EQUALS(MATVAL(0,1), 1);
      }    
   }


   void test_jacobian()
   {
      TestApplications::singleObj_denseCon<NLP1_problem> base(6);
      colin::RelaxableMixedIntDomainApplication<MINLP1_problem> app(base);

      std::vector<real> r_domain(6);
      for( size_t i = 0; i < 6; ++i )
         r_domain[i] = i;

      MAT matrix;
      app.EvalCG(r_domain, matrix);
      if ( ! CxxTest::tracker().testFailed() )
      {
         TS_ASSERT_EQUALS(MATROW, SIGN(6));
         TS_ASSERT_EQUALS(MATCOL, SIGN(6));
         TS_ASSERT_EQUALS(MATVAL(0,0), 1);
         TS_ASSERT_EQUALS(MATVAL(0,1), 1);
         TS_ASSERT_EQUALS(MATVAL(0,2), 1);
         TS_ASSERT_EQUALS(MATVAL(0,3), 1);
         TS_ASSERT_EQUALS(MATVAL(0,4), 1);
         TS_ASSERT_EQUALS(MATVAL(0,5), 1);
         TS_ASSERT_EQUALS(MATVAL(1,0), 2);
         TS_ASSERT_EQUALS(MATVAL(1,1), 2);
         TS_ASSERT_EQUALS(MATVAL(1,2), 2);
         TS_ASSERT_EQUALS(MATVAL(1,3), 2);
         TS_ASSERT_EQUALS(MATVAL(1,4), 2);
         TS_ASSERT_EQUALS(MATVAL(1,5), 2);
         TS_ASSERT_EQUALS(MATVAL(2,0), 1);
         TS_ASSERT_EQUALS(MATVAL(2,1), 2);
         TS_ASSERT_EQUALS(MATVAL(2,2), 3);
         TS_ASSERT_EQUALS(MATVAL(2,3), 4);
         TS_ASSERT_EQUALS(MATVAL(2,4), 5);
         TS_ASSERT_EQUALS(MATVAL(2,5), 6);
         TS_ASSERT_EQUALS(MATVAL(3,0), 0);
         TS_ASSERT_EQUALS(MATVAL(3,1), 2);
         TS_ASSERT_EQUALS(MATVAL(3,2), 4);
         TS_ASSERT_EQUALS(MATVAL(3,3), 6);
         TS_ASSERT_EQUALS(MATVAL(3,4), 8);
         TS_ASSERT_EQUALS(MATVAL(3,5), 10);
         TS_ASSERT_EQUALS(MATVAL(4,0), 1);
         TS_ASSERT_EQUALS(MATVAL(4,1), 2);
         TS_ASSERT_EQUALS(MATVAL(4,2), 3);
         TS_ASSERT_EQUALS(MATVAL(4,3), 4);
         TS_ASSERT_EQUALS(MATVAL(4,4), 5);
         TS_ASSERT_EQUALS(MATVAL(4,5), 6);
         TS_ASSERT_EQUALS(MATVAL(5,0), 1);
         TS_ASSERT_EQUALS(MATVAL(5,1), 1);
         TS_ASSERT_EQUALS(MATVAL(5,2), 1);
         TS_ASSERT_EQUALS(MATVAL(5,3), 1);
         TS_ASSERT_EQUALS(MATVAL(5,4), 1);
         TS_ASSERT_EQUALS(MATVAL(5,5), 1);
      }

      app.set_discrete_domain(1, 3);

      utilib::MixedIntVars mi_domain(1,3,2);
      mi_domain.Binary()[0] = true;
      mi_domain.Integer()[0] = 2; 
      mi_domain.Integer()[1] = 3;
      mi_domain.Integer()[2] = 4;
      mi_domain.Real()[0] = 0;
      mi_domain.Real()[1] = 1;

      app.EvalCG(mi_domain, matrix);
      TS_ASSERT_EQUALS(MATROW, SIGN(6));
      TS_ASSERT_EQUALS(MATCOL, SIGN(2));
      if ( ! CxxTest::tracker().testFailed() )
      {
         TS_ASSERT_EQUALS(MATVAL(0,0), 1);
         TS_ASSERT_EQUALS(MATVAL(0,1), 1);
         TS_ASSERT_EQUALS(MATVAL(1,0), 2);
         TS_ASSERT_EQUALS(MATVAL(1,1), 2);
         TS_ASSERT_EQUALS(MATVAL(2,0), 5);
         TS_ASSERT_EQUALS(MATVAL(2,1), 6);
         TS_ASSERT_EQUALS(MATVAL(3,0), 0);
         TS_ASSERT_EQUALS(MATVAL(3,1), 2);
         TS_ASSERT_EQUALS(MATVAL(4,0), 5);
         TS_ASSERT_EQUALS(MATVAL(4,1), 6);
         TS_ASSERT_EQUALS(MATVAL(5,0), 1);
         TS_ASSERT_EQUALS(MATVAL(5,1), 1);
      }
   }

   void test_reconfigure()
   {
      TestApplications::singleObj_denseCon<NLP1_problem> base(6);
      colin::RelaxableMixedIntDomainApplication<MINLP1_problem> app(base);

      // spot checking some values...
      TS_ASSERT_EQUALS(app.num_real_vars,   6);
      TS_ASSERT_EQUALS(app.num_int_vars,    0);
      TS_ASSERT_EQUALS(app.num_binary_vars, 0);

      TS_ASSERT_EQUALS(app.realLabel(3), "x_3");
      TS_ASSERT_EQUALS(app.realLowerBound(3), -4);
      TS_ASSERT_EQUALS(app.realUpperBound(3), real::positive_infinity);
      TS_ASSERT_EQUALS(app.realLowerBoundType(3), hard_bound);
      TS_ASSERT_EQUALS(app.realUpperBoundType(3), no_bound);

      // make the domain larger
      base.num_real_vars = 7;
      TS_ASSERT_EQUALS( base.realUpperBound(6), 
                        real::positive_infinity );
      TS_ASSERT_EQUALS( base.realLowerBound(6), 
                        real::negative_infinity );

      // spot checking some values...
      TS_ASSERT_EQUALS(app.num_real_vars,   7);
      TS_ASSERT_EQUALS(app.num_int_vars,    0);
      TS_ASSERT_EQUALS(app.num_binary_vars, 0);

      TS_ASSERT_EQUALS(app.realLabel(3), "x_3");
      TS_ASSERT_EQUALS(app.realLabel(6), "");
      TS_ASSERT_EQUALS(app.realLowerBound(3), -4);
      TS_ASSERT_EQUALS(app.realLowerBound(6), real::negative_infinity);
      TS_ASSERT_EQUALS(app.realUpperBound(3), real::positive_infinity);
      TS_ASSERT_EQUALS(app.realUpperBound(6), real::positive_infinity);
      TS_ASSERT_EQUALS(app.realLowerBoundType(3), hard_bound);
      TS_ASSERT_EQUALS(app.realLowerBoundType(6), no_bound);
      TS_ASSERT_EQUALS(app.realUpperBoundType(3), no_bound);
      TS_ASSERT_EQUALS(app.realUpperBoundType(6), no_bound);


      // check configure callbacks...
      base.configure(7);
      TS_ASSERT_EQUALS( base.realUpperBound(6),  7 );
      TS_ASSERT_EQUALS( base.realLowerBound(6), -7 );
      TS_ASSERT_EQUALS(app.realUpperBound(6),  7);
      TS_ASSERT_EQUALS(app.realLowerBound(6), -7);
      TS_ASSERT_EQUALS(app.realLowerBoundType(6), hard_bound);
      TS_ASSERT_EQUALS(app.realUpperBoundType(6), hard_bound);

      base.setRealLowerBoundType(6, no_bound);

      // spot checking some values...
      TS_ASSERT_EQUALS(app.num_real_vars,   7);
      TS_ASSERT_EQUALS(app.num_int_vars,    0);
      TS_ASSERT_EQUALS(app.num_binary_vars, 0);

      TS_ASSERT_EQUALS(app.realLabel(3), "x_3");
      TS_ASSERT_EQUALS(app.realLabel(6), "x_6");
      TS_ASSERT_EQUALS(app.realLowerBound(3), -4);
      TS_ASSERT_EQUALS(app.realLowerBound(5), -6);
      TS_ASSERT_EQUALS(app.realLowerBound(6), real::negative_infinity);
      TS_ASSERT_EQUALS(app.realUpperBound(3), real::positive_infinity);
      TS_ASSERT_EQUALS(app.realUpperBound(5), 6);
      TS_ASSERT_EQUALS(app.realUpperBound(6), 7);
      TS_ASSERT_EQUALS(app.realLowerBoundType(3), hard_bound);
      TS_ASSERT_EQUALS(app.realLowerBoundType(5), hard_bound);
      TS_ASSERT_EQUALS(app.realLowerBoundType(6), no_bound);
      TS_ASSERT_EQUALS(app.realUpperBoundType(3), no_bound);
      TS_ASSERT_EQUALS(app.realUpperBoundType(5), hard_bound);
      TS_ASSERT_EQUALS(app.realUpperBoundType(6), hard_bound);

      app.set_discrete_domain(2, 3);

      // check the values...
      TS_ASSERT_EQUALS(app.num_binary_vars, 2);
      TS_ASSERT_EQUALS(app.num_int_vars,    3);
      TS_ASSERT_EQUALS(app.num_real_vars,   2);

      TS_ASSERT_EQUALS(app.binaryLabel(0), "x_0");
      TS_ASSERT_EQUALS(app.intLabel(0),    "x_2");
      TS_ASSERT_EQUALS(app.realLabel(0),   "x_5");

      TS_ASSERT_EQUALS(app.intLowerBound(0),  INT_MIN);
      TS_ASSERT_EQUALS(app.intLowerBound(1),  -4);
      TS_ASSERT_EQUALS(app.intLowerBound(2),  -5);
      TS_ASSERT_EQUALS(app.realLowerBound(0), -6);
      TS_ASSERT_EQUALS(app.realLowerBound(1), real::negative_infinity);

      TS_ASSERT_EQUALS(app.intUpperBound(0),  INT_MAX);
      TS_ASSERT_EQUALS(app.intUpperBound(1),  INT_MAX);
      TS_ASSERT_EQUALS(app.intUpperBound(2),  5);
      TS_ASSERT_EQUALS(app.realUpperBound(0), 6);
      TS_ASSERT_EQUALS(app.realUpperBound(1), 7);

      TS_ASSERT_EQUALS(app.intLowerBoundType(0), no_bound);
      TS_ASSERT_EQUALS(app.intLowerBoundType(1), hard_bound);
      TS_ASSERT_EQUALS(app.intLowerBoundType(2), hard_bound);
      TS_ASSERT_EQUALS(app.realLowerBoundType(0), hard_bound);
      TS_ASSERT_EQUALS(app.realLowerBoundType(1), no_bound);

      TS_ASSERT_EQUALS(app.intUpperBoundType(0),  no_bound);
      TS_ASSERT_EQUALS(app.intUpperBoundType(1),  no_bound);
      TS_ASSERT_EQUALS(app.intUpperBoundType(2),  hard_bound);
      TS_ASSERT_EQUALS(app.realUpperBoundType(0), hard_bound);
      TS_ASSERT_EQUALS(app.realUpperBoundType(1), hard_bound);

      // make the domain smaller
      base.num_real_vars = 6;

      // check the values...
      TS_ASSERT_EQUALS(app.num_binary_vars, 2);
      TS_ASSERT_EQUALS(app.num_int_vars,    3);
      TS_ASSERT_EQUALS(app.num_real_vars,   1);

      TS_ASSERT_EQUALS(app.binaryLabel(0), "x_0");
      TS_ASSERT_EQUALS(app.intLabel(0),    "x_2");
      TS_ASSERT_EQUALS(app.realLabel(0),   "x_5");

      TS_ASSERT_EQUALS(app.intLowerBound(0),  INT_MIN);
      TS_ASSERT_EQUALS(app.intLowerBound(1),  -4);
      TS_ASSERT_EQUALS(app.intLowerBound(2),  -5);
      TS_ASSERT_EQUALS(app.realLowerBound(0), -6);

      TS_ASSERT_EQUALS(app.intUpperBound(0),  INT_MAX);
      TS_ASSERT_EQUALS(app.intUpperBound(1),  INT_MAX);
      TS_ASSERT_EQUALS(app.intUpperBound(2),  5);
      TS_ASSERT_EQUALS(app.realUpperBound(0), 6);

      TS_ASSERT_EQUALS(app.intLowerBoundType(0), no_bound);
      TS_ASSERT_EQUALS(app.intLowerBoundType(1), hard_bound);
      TS_ASSERT_EQUALS(app.intLowerBoundType(2), hard_bound);
      TS_ASSERT_EQUALS(app.realLowerBoundType(0), hard_bound);

      TS_ASSERT_EQUALS(app.intUpperBoundType(0),  no_bound);
      TS_ASSERT_EQUALS(app.intUpperBoundType(1),  no_bound);
      TS_ASSERT_EQUALS(app.intUpperBoundType(2),  hard_bound);
      TS_ASSERT_EQUALS(app.realUpperBoundType(0), hard_bound);

      // make the domain smaller
      base.num_real_vars = 5;

      // check the values...
      TS_ASSERT_EQUALS(app.num_binary_vars, 2);
      TS_ASSERT_EQUALS(app.num_int_vars,    3);
      TS_ASSERT_EQUALS(app.num_real_vars,   0);

      // make the domain smaller
      base.num_real_vars = 3;

      // check the values...
      TS_ASSERT_EQUALS(app.num_binary_vars, 2);
      TS_ASSERT_EQUALS(app.num_int_vars,    1);
      TS_ASSERT_EQUALS(app.num_real_vars,   0);

      // make the domain smaller
      base.num_real_vars = 2;

      // check the values...
      TS_ASSERT_EQUALS(app.num_binary_vars, 2);
      TS_ASSERT_EQUALS(app.num_int_vars,    0);
      TS_ASSERT_EQUALS(app.num_real_vars,   0);

      // reconfigure split
      app.set_discrete_domain(1, 0);
      TS_ASSERT_EQUALS(app.num_binary_vars, 1);
      TS_ASSERT_EQUALS(app.num_int_vars,    0);
      TS_ASSERT_EQUALS(app.num_real_vars,   1);
   }
};

} // namespace colin
