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

#include "TestApplications.h"

namespace colin {
namespace unittest { class TestApplications_tests; }

/// This tests the testing infrastructure
class colin::unittest::TestApplications_tests : public CxxTest::TestSuite
{
   std::vector<double> x;
   utilib::MixedIntVars mi;
   utilib::seed_t seed;

public:
   TestApplications_tests()
      : x(3),
        mi(1,2,3)
   {
      x[0] = 1;
      x[1] = 2;
      x[2] = 4;

      mi.Binary()[0] = true;
      mi.Integer()[0] = 2;
      mi.Integer()[1] = 3;
      mi.Real()[0] = 1;
      mi.Real()[1] = 2;
      mi.Real()[2] = 4;
   }

   ///////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////

   void test_application_configuration()
   {
      // basic construction for single objective continuous application
      {
         TestApplications::singleObj_denseCon<UNLP0_problem> app;
         
         TS_ASSERT_EQUALS(app.domain_size, 3);
         TS_ASSERT_EQUALS(app.sense, minimization);
         TS_ASSERT( ! app.has_property("num_constraints") );
      }
      {
         TestApplications::singleObj_denseCon<LP_problem> app;

         TS_ASSERT_EQUALS(app.domain_size, 3);
         TS_ASSERT_EQUALS(app.sense, minimization);
         TS_ASSERT( app.has_property("num_constraints") );
         TS_ASSERT_EQUALS( app.num_constraints, 3 );
      }
      {
         TestApplications::singleObj_denseCon<NLP2_problem> app;
         
         TS_ASSERT_EQUALS(app.domain_size, 3);
         TS_ASSERT_EQUALS(app.sense, minimization);
         TS_ASSERT( app.has_property("num_constraints") );
         TS_ASSERT_EQUALS( app.num_constraints, 6 );
      }
      // basic construction for multiobjective continuous application
      {
         TestApplications::multiObj_denseCon<MO_UNLP0_problem> app;

         TS_ASSERT_EQUALS(app.domain_size, 3);
         std::vector<optimizationSense> sense = app.sense;
         TS_ASSERT_EQUALS(sense.size(), 2u);
         TS_ASSERT_EQUALS(sense[0], minimization);
         TS_ASSERT_EQUALS(sense[1], maximization);
         TS_ASSERT( ! app.has_property("num_constraints") );
      }
      {
         TestApplications::multiObj_denseCon<MO_LP_problem> app;

         TS_ASSERT_EQUALS(app.domain_size, 3);
         std::vector<optimizationSense> sense = app.sense;
         TS_ASSERT_EQUALS(sense.size(), 2u);
         TS_ASSERT_EQUALS(sense[0], minimization);
         TS_ASSERT_EQUALS(sense[1], maximization);
         TS_ASSERT( app.has_property("num_constraints") );
         TS_ASSERT_EQUALS( app.num_constraints, 3 );
      }
      {
         TestApplications::multiObj_denseCon<MO_NLP2_problem> app;

         TS_ASSERT_EQUALS(app.domain_size, 3);
         std::vector<optimizationSense> sense = app.sense;
         TS_ASSERT_EQUALS(sense.size(), 2u);
         TS_ASSERT_EQUALS(sense[0], minimization);
         TS_ASSERT_EQUALS(sense[1], maximization);
         TS_ASSERT( app.has_property("num_constraints") );
         TS_ASSERT_EQUALS( app.num_constraints, 6 );
      }
      // basic construction for single objective mixed-int application
      {
         TestApplications::mi_singleObj_denseCon<UMINLP0_problem> app;
         
         TS_ASSERT_EQUALS(app.domain_size, 6);
         TS_ASSERT_EQUALS(app.num_real_vars, 3);
         TS_ASSERT_EQUALS(app.num_int_vars, 2);
         TS_ASSERT_EQUALS(app.num_binary_vars, 1);
         TS_ASSERT_EQUALS(app.sense, minimization);
         TS_ASSERT( ! app.has_property("num_constraints") );
      }
      {
         TestApplications::mi_singleObj_denseCon<MILP_problem> app;

         TS_ASSERT_EQUALS(app.domain_size, 6);
         TS_ASSERT_EQUALS(app.num_real_vars, 3);
         TS_ASSERT_EQUALS(app.num_int_vars, 2);
         TS_ASSERT_EQUALS(app.num_binary_vars, 1);
         TS_ASSERT_EQUALS(app.sense, minimization);
         TS_ASSERT( app.has_property("num_constraints") );
         TS_ASSERT_EQUALS( app.num_constraints, 3 );
      }
      {
         TestApplications::mi_singleObj_denseCon<MINLP2_problem> app;
         
         TS_ASSERT_EQUALS(app.domain_size, 6);
         TS_ASSERT_EQUALS(app.num_real_vars, 3);
         TS_ASSERT_EQUALS(app.num_int_vars, 2);
         TS_ASSERT_EQUALS(app.num_binary_vars, 1);
         TS_ASSERT_EQUALS(app.sense, minimization);
         TS_ASSERT( app.has_property("num_constraints") );
         TS_ASSERT_EQUALS( app.num_constraints, 6 );
      }
      // basic construction for multiobjective mixed-int application
      {
         TestApplications::mi_multiObj_denseCon<MO_UMINLP0_problem> app;

         TS_ASSERT_EQUALS(app.domain_size, 6);
         TS_ASSERT_EQUALS(app.num_real_vars, 3);
         TS_ASSERT_EQUALS(app.num_int_vars, 2);
         TS_ASSERT_EQUALS(app.num_binary_vars, 1);
         std::vector<optimizationSense> sense = app.sense;
         TS_ASSERT_EQUALS(sense.size(), 2u);
         TS_ASSERT_EQUALS(sense[0], minimization);
         TS_ASSERT_EQUALS(sense[1], maximization);
         TS_ASSERT( ! app.has_property("num_constraints") );
      }
      {
         TestApplications::mi_multiObj_denseCon<MO_MILP_problem> app;

         TS_ASSERT_EQUALS(app.domain_size, 6);
         TS_ASSERT_EQUALS(app.num_real_vars, 3);
         TS_ASSERT_EQUALS(app.num_int_vars, 2);
         TS_ASSERT_EQUALS(app.num_binary_vars, 1);
         std::vector<optimizationSense> sense = app.sense;
         TS_ASSERT_EQUALS(sense.size(), 2u);
         TS_ASSERT_EQUALS(sense[0], minimization);
         TS_ASSERT_EQUALS(sense[1], maximization);
         TS_ASSERT( app.has_property("num_constraints") );
         TS_ASSERT_EQUALS( app.num_constraints, 3 );
      }
      {
         TestApplications::mi_multiObj_denseCon<MO_MINLP2_problem> app;

         TS_ASSERT_EQUALS(app.domain_size, 6);
         TS_ASSERT_EQUALS(app.num_real_vars, 3);
         TS_ASSERT_EQUALS(app.num_int_vars, 2);
         TS_ASSERT_EQUALS(app.num_binary_vars, 1);
         std::vector<optimizationSense> sense = app.sense;
         TS_ASSERT_EQUALS(sense.size(), 2u);
         TS_ASSERT_EQUALS(sense[0], minimization);
         TS_ASSERT_EQUALS(sense[1], maximization);
         TS_ASSERT( app.has_property("num_constraints") );
         TS_ASSERT_EQUALS( app.num_constraints, 6 );
      }
   }

   void test_singleObj_denseCon_calc_f()
   {
      TestApplications::singleObj_denseCon<NLP2_problem> app;

      utilib::Any domain;
      app.map_domain(x, domain, true);
 
      double ans;
      utilib::TypeManager()->lexical_cast(app.calc_f(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, 8.);
   }

   void test_singleObj_denseCon_calc_g()
   {
      TestApplications::singleObj_denseCon<NLP2_problem> app;

      utilib::Any domain;
      app.map_domain(x, domain, true);
 
      // validate the objective gradient
      std::vector<double> ans, reference(3);
      reference[0] = 8;
      reference[1] = 4;
      reference[2] = 2;

      utilib::TypeManager()->lexical_cast(app.calc_g(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);
   }

   void test_singleObj_denseCon_calc_h()
   {
      TestApplications::singleObj_denseCon<NLP2_problem> app;

      utilib::Any domain;
      app.map_domain(x, domain, true);
 
      // validate the objective hessian
      std::vector<std::vector<double> > ans, reference(3);
      for(size_t i=0; i<3; reference[i++].resize(3));
      reference[0][0] = 0;
      reference[0][1] = 4;
      reference[0][2] = 2;
      reference[1][0] = 4;
      reference[1][1] = 0;
      reference[1][2] = 1;
      reference[2][0] = 2;
      reference[2][1] = 1;
      reference[2][2] = 0;

      utilib::TypeManager()->lexical_cast(app.calc_h(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);
   }


   void test_singleObj_denseCon_calc_lcf()
   {
      TestApplications::singleObj_denseCon<NLP2_problem> app;

      utilib::Any domain;
      app.map_domain(x, domain, true);

      // validate the linear constraint computations
      std::vector<double> ans, reference(3);
      reference[0] = 7;
      reference[1] = 14;
      reference[2] = 17;

      utilib::TypeManager()->lexical_cast(app.calc_lcf(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);

      app.num_linear_constraints = 4;
      reference.push_back(0);

      utilib::TypeManager()->lexical_cast(app.calc_lcf(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);

      // validate empty constraints...
      app.num_linear_constraints = 0;
      reference.clear();

      utilib::TypeManager()->lexical_cast(app.calc_lcf(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);
   }

   void test_singleObj_denseCon_calc_lcg()
   {
      TestApplications::singleObj_denseCon<NLP2_problem> app;

      utilib::Any domain;
      app.map_domain(x, domain, true);

      // validate the linear constraint gradient (A-matrix) computations
      std::vector<std::vector<double> > ans, reference(3);
      for(size_t i=0; i<3; reference[i++].resize(3));
      reference[0][0] = 1;
      reference[0][1] = 1;
      reference[0][2] = 1;
      reference[1][0] = 2;
      reference[1][1] = 2;
      reference[1][2] = 2;
      reference[2][0] = 1;
      reference[2][1] = 2;
      reference[2][2] = 3;

      utilib::TypeManager()->lexical_cast(app.calc_lcg(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);
   }

   void test_singleObj_denseCon_calc_nlcf()
   {
      TestApplications::singleObj_denseCon<NLP2_problem> app;
 
      utilib::Any domain;
      app.map_domain(x, domain, true);

      // validate the nonlinear constraint computations
      std::vector<double> ans;
      std::vector<double> reference(3);
      reference[0] = 21;
      reference[1] = 17;
      reference[2] = 7;

      utilib::TypeManager()->lexical_cast(app.calc_nlcf(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);

      app.num_nonlinear_constraints = 4;
      reference.push_back(0);

      utilib::TypeManager()->lexical_cast(app.calc_nlcf(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);

      // validate empty constraints...
      app.num_nonlinear_constraints = 0;
      reference.clear();

      utilib::TypeManager()->lexical_cast(app.calc_nlcf(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);
   }

   void test_singleObj_denseCon_calc_nlcg()
   {
      TestApplications::singleObj_denseCon<NLP2_problem> app;

      utilib::Any domain;
      app.map_domain(x, domain, true);

      // validate the nonlinear constraint gradient (Jacobian) computations
      std::vector<std::vector<double> > ans, reference(3);
      for(size_t i=0; i<3; reference[i++].resize(3));
      reference[0][0] = 2;
      reference[0][1] = 4;
      reference[0][2] = 8;
      reference[1][0] = 1;
      reference[1][1] = 2;
      reference[1][2] = 3;
      reference[2][0] = 1;
      reference[2][1] = 1;
      reference[2][2] = 1;

      utilib::TypeManager()->lexical_cast(app.calc_nlcg(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);
   }

   ///////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////

   void test_multiObj_denseCon_calc_mf()
   {
      TestApplications::multiObj_denseCon<MO_NLP2_problem> app;

      utilib::Any domain;
      app.map_domain(x, domain, true);
 
      // validate the objective gradient
      std::vector<double> ans, reference(2);
      reference[0] = 8;
      reference[1] = -21;

      utilib::TypeManager()->lexical_cast(app.calc_mf(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);
   }

   void test_multiObj_denseCon_calc_g()
   {
      TestApplications::multiObj_denseCon<MO_NLP2_problem> app;

      utilib::Any domain;
      app.map_domain(x, domain, true);
 
      // validate the objective gradient
      std::vector<std::vector<double> > ans, reference(2);
      for(size_t i=0; i<2; reference[i++].resize(3));
      reference[0][0] = 8;
      reference[0][1] = 4;
      reference[0][2] = 2;
      reference[1][0] = -2;
      reference[1][1] = -4;
      reference[1][2] = -8;

      utilib::TypeManager()->lexical_cast(app.calc_g(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);
   }

   void test_multiObj_denseCon_calc_h()
   {
      TestApplications::multiObj_denseCon<MO_NLP2_problem> app;

      utilib::Any domain;
      app.map_domain(x, domain, true);
 
      // validate the objective hessian
      std::vector<std::vector<std::vector<double> > > ans, reference(2);
      for(size_t i=0; i<2; ++i)
      {
         reference[i].resize(3);
         for(size_t j=0; j<3; reference[i][j++].resize(3));
      }
      reference[0][0][0] = 0;
      reference[0][0][1] = 4;
      reference[0][0][2] = 2;
      reference[0][1][0] = 4;
      reference[0][1][1] = 0;
      reference[0][1][2] = 1;
      reference[0][2][0] = 2;
      reference[0][2][1] = 1;
      reference[0][2][2] = 0;
      reference[1][0][0] = -2;
      reference[1][0][1] = 0;
      reference[1][0][2] = 0;
      reference[1][1][0] = 0;
      reference[1][1][1] = -2;
      reference[1][1][2] = 0;
      reference[1][2][0] = 0;
      reference[1][2][1] = 0;
      reference[1][2][2] = -2;

      utilib::TypeManager()->lexical_cast(app.calc_h(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);
   }


   void test_multiObj_denseCon_calc_lcf()
   {
      TestApplications::multiObj_denseCon<MO_NLP2_problem> app;

      utilib::Any domain;
      app.map_domain(x, domain, true);

      // validate the linear constraint computations
      std::vector<double> ans, reference(3);
      reference[0] = 7;
      reference[1] = 14;
      reference[2] = 17;

      utilib::TypeManager()->lexical_cast(app.calc_lcf(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);

      app.num_linear_constraints = 4;
      reference.push_back(0);

      utilib::TypeManager()->lexical_cast(app.calc_lcf(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);

      // validate empty constraints...
      app.num_linear_constraints = 0;
      reference.clear();

      utilib::TypeManager()->lexical_cast(app.calc_lcf(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);
   }

   void test_multiObj_denseCon_calc_lcg()
   {
      TestApplications::multiObj_denseCon<MO_NLP2_problem> app;

      utilib::Any domain;
      app.map_domain(x, domain, true);

      // validate the linear constraint gradient (A-matrix) computations
      std::vector<std::vector<double> > ans, reference(3);
      for(size_t i=0; i<3; reference[i++].resize(3));
      reference[0][0] = 1;
      reference[0][1] = 1;
      reference[0][2] = 1;
      reference[1][0] = 2;
      reference[1][1] = 2;
      reference[1][2] = 2;
      reference[2][0] = 1;
      reference[2][1] = 2;
      reference[2][2] = 3;

      utilib::TypeManager()->lexical_cast(app.calc_lcg(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);
   }

   void test_multiObj_denseCon_calc_nlcf()
   {
      TestApplications::multiObj_denseCon<MO_NLP2_problem> app;
 
      utilib::Any domain;
      app.map_domain(x, domain, true);

      // validate the nonlinear constraint computations
      std::vector<double> ans;
      std::vector<double> reference(3);
      reference[0] = 21;
      reference[1] = 17;
      reference[2] = 7;

      utilib::TypeManager()->lexical_cast(app.calc_nlcf(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);

      app.num_nonlinear_constraints = 4;
      reference.push_back(0);

      utilib::TypeManager()->lexical_cast(app.calc_nlcf(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);

      // validate empty constraints...
      app.num_nonlinear_constraints = 0;
      reference.clear();

      utilib::TypeManager()->lexical_cast(app.calc_nlcf(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);
   }

   void test_multiObj_denseCon_calc_nlcg()
   {
      TestApplications::multiObj_denseCon<MO_NLP2_problem> app;

      utilib::Any domain;
      app.map_domain(x, domain, true);

      // validate the nonlinear constraint gradient (Jacobian) computations
      std::vector<std::vector<double> > ans, reference(3);
      for(size_t i=0; i<3; reference[i++].resize(3));
      reference[0][0] = 2;
      reference[0][1] = 4;
      reference[0][2] = 8;
      reference[1][0] = 1;
      reference[1][1] = 2;
      reference[1][2] = 3;
      reference[2][0] = 1;
      reference[2][1] = 1;
      reference[2][2] = 1;

      utilib::TypeManager()->lexical_cast(app.calc_nlcg(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);
   }

   ///////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////

   void test_mi_singleObj_denseCon_calc_f()
   {
      TestApplications::mi_singleObj_denseCon<MINLP2_problem> app;

      utilib::Any domain;
      app.map_domain(mi, domain, true);
 
      double ans;
      utilib::TypeManager()->lexical_cast(app.calc_f(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, 48.);
   }

   void test_mi_singleObj_denseCon_calc_g()
   {
      TestApplications::mi_singleObj_denseCon<MINLP2_problem> app;

      utilib::Any domain;
      app.map_domain(mi, domain, true);
 
      // validate the objective gradient
      std::vector<double> ans, reference(3);
      reference[0] = 48;
      reference[1] = 24;
      reference[2] = 12;

      utilib::TypeManager()->lexical_cast(app.calc_g(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);
   }

   void test_mi_singleObj_denseCon_calc_h()
   {
      TestApplications::mi_singleObj_denseCon<MINLP2_problem> app;

      utilib::Any domain;
      app.map_domain(mi, domain, true);
 
      // validate the objective hessian
      std::vector<std::vector<double> > ans, reference(3);
      for(size_t i=0; i<3; reference[i++].resize(3));
      reference[0][0] = 0;
      reference[0][1] = 24;
      reference[0][2] = 12;
      reference[1][0] = 24;
      reference[1][1] = 0;
      reference[1][2] = 6;
      reference[2][0] = 12;
      reference[2][1] = 6;
      reference[2][2] = 0;

      utilib::TypeManager()->lexical_cast(app.calc_h(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);
   }


   void test_mi_singleObj_denseCon_calc_lcf()
   {
      TestApplications::mi_singleObj_denseCon<MINLP2_problem> app;

      utilib::Any domain;
      app.map_domain(mi, domain, true);

      // validate the linear constraint computations
      std::vector<double> ans, reference(3);
      reference[0] = 13;
      reference[1] = 26;
      reference[2] = 52;

      utilib::TypeManager()->lexical_cast(app.calc_lcf(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);

      app.num_linear_constraints = 4;
      reference.push_back(0);

      utilib::TypeManager()->lexical_cast(app.calc_lcf(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);

      // validate empty constraints...
      app.num_linear_constraints = 0;
      reference.clear();

      utilib::TypeManager()->lexical_cast(app.calc_lcf(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);
   }

   void test_mi_singleObj_denseCon_calc_lcg()
   {
      TestApplications::mi_singleObj_denseCon<MINLP2_problem> app;

      utilib::Any domain;
      app.map_domain(mi, domain, true);

      // validate the linear constraint gradient (A-matrix) computations
      std::vector<std::vector<double> > ans, reference(3);
      for(size_t i=0; i<3; reference[i++].resize(3));
      reference[0][0] = 1;
      reference[0][1] = 1;
      reference[0][2] = 1;
      reference[1][0] = 2;
      reference[1][1] = 2;
      reference[1][2] = 2;
      reference[2][0] = 4;
      reference[2][1] = 5;
      reference[2][2] = 6;

      utilib::TypeManager()->lexical_cast(app.calc_lcg(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);
   }

   void test_mi_singleObj_denseCon_calc_nlcf()
   {
      TestApplications::mi_singleObj_denseCon<MINLP2_problem> app;
 
      utilib::Any domain;
      app.map_domain(mi, domain, true);

      // validate the nonlinear constraint computations
      std::vector<double> ans;
      std::vector<double> reference(3);
      reference[0] = 35;
      reference[1] = 52;
      reference[2] = 13;

      utilib::TypeManager()->lexical_cast(app.calc_nlcf(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);

      app.num_nonlinear_constraints = 4;
      reference.push_back(0);

      utilib::TypeManager()->lexical_cast(app.calc_nlcf(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);

      // validate empty constraints...
      app.num_nonlinear_constraints = 0;
      reference.clear();

      utilib::TypeManager()->lexical_cast(app.calc_nlcf(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);
   }

   void test_mi_singleObj_denseCon_calc_nlcg()
   {
      TestApplications::mi_singleObj_denseCon<MINLP2_problem> app;

      utilib::Any domain;
      app.map_domain(mi, domain, true);

      // validate the nonlinear constraint gradient (Jacobian) computations
      std::vector<std::vector<double> > ans, reference(3);
      for(size_t i=0; i<3; reference[i++].resize(3));
      reference[0][0] = 2;
      reference[0][1] = 4;
      reference[0][2] = 8;
      reference[1][0] = 4;
      reference[1][1] = 5;
      reference[1][2] = 6;
      reference[2][0] = 1;
      reference[2][1] = 1;
      reference[2][2] = 1;

      utilib::TypeManager()->lexical_cast(app.calc_nlcg(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);
   }

   ///////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////

   void test_mi_multiObj_denseCon_calc_mf()
   {
      TestApplications::mi_multiObj_denseCon<MO_MINLP2_problem> app;

      utilib::Any domain;
      app.map_domain(mi, domain, true);
 
      // validate the objective gradient
      std::vector<double> ans, reference(2);
      reference[0] = 48;
      reference[1] = -35;

      utilib::TypeManager()->lexical_cast(app.calc_mf(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);
   }

   void test_mi_multiObj_denseCon_calc_g()
   {
      TestApplications::mi_multiObj_denseCon<MO_MINLP2_problem> app;

      utilib::Any domain;
      app.map_domain(mi, domain, true);
 
      // validate the objective gradient
      std::vector<std::vector<double> > ans, reference(2);
      for(size_t i=0; i<2; reference[i++].resize(3));
      reference[0][0] = 48;
      reference[0][1] = 24;
      reference[0][2] = 12;
      reference[1][0] = -2;
      reference[1][1] = -4;
      reference[1][2] = -8;

      utilib::TypeManager()->lexical_cast(app.calc_g(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);
   }

   void test_mi_multiObj_denseCon_calc_h()
   {
      TestApplications::mi_multiObj_denseCon<MO_MINLP2_problem> app;

      utilib::Any domain;
      app.map_domain(mi, domain, true);
 
      // validate the objective hessian
      std::vector<std::vector<std::vector<double> > > ans, reference(2);
      for(size_t i=0; i<2; ++i)
      {
         reference[i].resize(3);
         for(size_t j=0; j<3; reference[i][j++].resize(3));
      }
      reference[0][0][0] = 0;
      reference[0][0][1] = 24;
      reference[0][0][2] = 12;
      reference[0][1][0] = 24;
      reference[0][1][1] = 0;
      reference[0][1][2] = 6;
      reference[0][2][0] = 12;
      reference[0][2][1] = 6;
      reference[0][2][2] = 0;
      reference[1][0][0] = -2;
      reference[1][0][1] = 0;
      reference[1][0][2] = 0;
      reference[1][1][0] = 0;
      reference[1][1][1] = -2;
      reference[1][1][2] = 0;
      reference[1][2][0] = 0;
      reference[1][2][1] = 0;
      reference[1][2][2] = -2;

      utilib::TypeManager()->lexical_cast(app.calc_h(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);
   }


   void test_mi_multiObj_denseCon_calc_lcf()
   {
      TestApplications::mi_multiObj_denseCon<MO_MINLP2_problem> app;

      utilib::Any domain;
      app.map_domain(mi, domain, true);

      // validate the linear constraint computations
      std::vector<double> ans, reference(3);
      reference[0] = 13;
      reference[1] = 26;
      reference[2] = 52;

      utilib::TypeManager()->lexical_cast(app.calc_lcf(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);

      app.num_linear_constraints = 4;
      reference.push_back(0);

      utilib::TypeManager()->lexical_cast(app.calc_lcf(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);

      // validate empty constraints...
      app.num_linear_constraints = 0;
      reference.clear();

      utilib::TypeManager()->lexical_cast(app.calc_lcf(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);
   }

   void test_mi_multiObj_denseCon_calc_lcg()
   {
      TestApplications::mi_multiObj_denseCon<MO_MINLP2_problem> app;

      utilib::Any domain;
      app.map_domain(mi, domain, true);

      // validate the linear constraint gradient (A-matrix) computations
      std::vector<std::vector<double> > ans, reference(3);
      for(size_t i=0; i<3; reference[i++].resize(3));
      reference[0][0] = 1;
      reference[0][1] = 1;
      reference[0][2] = 1;
      reference[1][0] = 2;
      reference[1][1] = 2;
      reference[1][2] = 2;
      reference[2][0] = 4;
      reference[2][1] = 5;
      reference[2][2] = 6;

      utilib::TypeManager()->lexical_cast(app.calc_lcg(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);
   }

   void test_mi_multiObj_denseCon_calc_nlcf()
   {
      TestApplications::mi_multiObj_denseCon<MO_MINLP2_problem> app;
 
      utilib::Any domain;
      app.map_domain(mi, domain, true);

      // validate the nonlinear constraint computations
      std::vector<double> ans;
      std::vector<double> reference(3);
      reference[0] = 35;
      reference[1] = 52;
      reference[2] = 13;

      utilib::TypeManager()->lexical_cast(app.calc_nlcf(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);

      app.num_nonlinear_constraints = 4;
      reference.push_back(0);

      utilib::TypeManager()->lexical_cast(app.calc_nlcf(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);

      // validate empty constraints...
      app.num_nonlinear_constraints = 0;
      reference.clear();

      utilib::TypeManager()->lexical_cast(app.calc_nlcf(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);
   }

   void test_mi_multiObj_denseCon_calc_nlcg()
   {
      TestApplications::mi_multiObj_denseCon<MO_MINLP2_problem> app;

      utilib::Any domain;
      app.map_domain(mi, domain, true);

      // validate the nonlinear constraint gradient (Jacobian) computations
      std::vector<std::vector<double> > ans, reference(3);
      for(size_t i=0; i<3; reference[i++].resize(3));
      reference[0][0] = 2;
      reference[0][1] = 4;
      reference[0][2] = 8;
      reference[1][0] = 4;
      reference[1][1] = 5;
      reference[1][2] = 6;
      reference[2][0] = 1;
      reference[2][1] = 1;
      reference[2][2] = 1;

      utilib::TypeManager()->lexical_cast(app.calc_nlcg(domain, seed), ans);
      TS_ASSERT_EQUALS(ans, reference);
   }
};

} // namespace colin
