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

#include <utilib/SparseMatrix.h>

#include <boost/bimap.hpp>

#include "CommonTestUtils.h"
#include "TestApplications.h"


namespace colin {

class colin::unittest::Application_NonD_Constraints \
   : public CxxTest::TestSuite
{
public:
   typedef boost::bimap<size_t, std::string>  labels_t;

   // The test application
   TestApplications::singleObj_denseCon<SNLP2_problem> *app;
   TestApplications::mi_singleObj_denseCon<SMINLP2_problem> *mi_app;

   std::vector<real>  x;
   utilib::MixedIntVars mi;

   Application_NonD_Constraints()
      : x(3),
        mi(1,2,3)
   {
      mi.Binary()[0] = true;
      mi.Integer()[0] = 2;
      mi.Integer()[1] = 3;
      mi.Real()[0] = 1;
      mi.Real()[1] = 2;
      mi.Real()[2] = 4;

      x[0] = 1;
      x[1] = 2;
      x[2] = 4;

   }

   void setUp()
   {
      try{
      utilib::exception_mngr::set_mode(utilib::exception_mngr::Standard);
      app = new TestApplications::singleObj_denseCon<SNLP2_problem>(3);
      mi_app = new TestApplications::mi_singleObj_denseCon<SMINLP2_problem>(6);

      std::vector<real> c(3);

      c[0] = 0.0;
      c[1] = 20;
      c[2] = real::negative_infinity;
      app->nondeterministic_constraint_lower_bounds = c;
      mi_app->nondeterministic_constraint_lower_bounds = c;

      c[0] = real::positive_infinity;
      c[1] = 20;
      c[2] = 1.1;
      app->nondeterministic_constraint_upper_bounds = c;
      mi_app->nondeterministic_constraint_upper_bounds = c;

      app->num_linear_constraints = 0;
      mi_app->num_linear_constraints = 0;
      app->num_nonlinear_constraints = 0;
      mi_app->num_nonlinear_constraints = 0;
      }catch (std::exception& e) { TS_FAIL(e.what()); }
   }

   void tearDown()
   {
      delete app;
      delete mi_app;
   }

   void test_numc()
   {
      TS_TRACE("------------------------------------------------");
      TS_TRACE("numc - Test the number of constraints");
      TS_TRACE("------------------------------------------------");

      if (!app) 
      {
         TS_FAIL("Failed to construct application with nondeterministic "
                 "constraints."); 
         return;
      }
      TS_ASSERT_EQUALS(app->num_constraints, 3u);
      TS_ASSERT_EQUALS(app->num_nondeterministic_constraints, 3u);
      TS_ASSERT_EQUALS(app->numEqConstraints(), 1u);
      TS_ASSERT_EQUALS(app->numIneqConstraints(), 2u);
      TS_ASSERT_EQUALS(app->numNondeterministicEqConstraints(), 1u);
      TS_ASSERT_EQUALS(app->numNondeterministicIneqConstraints(), 2u);
   }

   void test_eval_ndcf()
   {
      TS_TRACE("------------------------------------------------");
      TS_TRACE("eval_ndcf - Evaluate constraints");
      TS_TRACE("------------------------------------------------");

      utilib::BasicArray<real> ans, reference(3);
      reference[0] = 21;
      reference[1] = 17;
      reference[2] = 7;

      app->EvalNDCF(x, ans);
      TS_ASSERT_EQUALS(ans.size(), 3u)
      TS_ASSERT_DELTA(ans[0], reference[0], 1e-7);
      TS_ASSERT_DELTA(ans[1], reference[1], 1e-7);
      TS_ASSERT_DELTA(ans[2], reference[2], 1e-7);

      app->EvalNDEqCF(x, ans);
      TS_ASSERT_EQUALS(ans.size(), 1u)
      TS_ASSERT_DELTA(ans[0], reference[1], 1e-7);

      app->EvalNDIneqCF(x, ans);
      TS_ASSERT_EQUALS(ans.size(), 2u)
      TS_ASSERT_DELTA(ans[0], reference[0], 1e-7);
      TS_ASSERT_DELTA(ans[1], reference[2], 1e-7);

      // Quick test: evaluating when there aren't any constraints (this
      // *should* bypass calling the underlying application)
      app->num_nondeterministic_constraints = 0;
      size_t n = app->eval_count();
      app->EvalNDCF(x, ans);
      TS_ASSERT_EQUALS(ans.size(), 0u);
      TS_ASSERT_EQUALS(app->eval_count(), n);
   }

   void test_eval_ndcvf()
   {
      TS_TRACE("------------------------------------------------");
      TS_TRACE("eval_ndcvf - Evaluate constraint violations");
      TS_TRACE("------------------------------------------------");

      utilib::BasicArray<real> ans, reference(3);
      reference[0] = 0;
      reference[1] = -3;
      reference[2] = 5.9;

      app->EvalNDCFViol(x, ans);
      TS_ASSERT_EQUALS(ans.size(), 3u)
      TS_ASSERT_DELTA(ans[0], reference[0], 1e-7);
      TS_ASSERT_DELTA(ans[1], reference[1], 1e-7);
      TS_ASSERT_DELTA(ans[2], reference[2], 1e-7);
   }

   void test_async_eval_ndcf()
   {
      TS_TRACE("--------------------------------------------------");
      TS_TRACE("async_eval_ndcf - Evaluate constraints asynchronously");
      TS_TRACE("--------------------------------------------------");

      utilib::BasicArray<real> ans, reference(3);
      reference[0] = 21;
      reference[1] = 17;
      reference[2] = 7;

      app->AsyncEvalNDCF(app->eval_mngr(), x, ans);
      app->synchronize();
      TS_ASSERT_EQUALS(ans.size(), 3u)
      TS_ASSERT_DELTA(ans[0], reference[0], 1e-7);
      TS_ASSERT_DELTA(ans[1], reference[1], 1e-7);
      TS_ASSERT_DELTA(ans[2], reference[2], 1e-7);

      app->AsyncEvalNDEqCF(app->eval_mngr(), x, ans);
      app->synchronize();
      TS_ASSERT_EQUALS(ans.size(), 1u)
      TS_ASSERT_DELTA(ans[0], reference[1], 1e-7);

      app->AsyncEvalNDIneqCF(app->eval_mngr(), x, ans);
      app->synchronize();
      TS_ASSERT_EQUALS(ans.size(), 2u)
      TS_ASSERT_DELTA(ans[0], reference[0], 1e-7);
      TS_ASSERT_DELTA(ans[1], reference[2], 1e-7);
   }

   void test_eval_ndcg()
   {
      TS_TRACE("------------------------------------------------");
      TS_TRACE("Evaluate constraint gradients");
      TS_TRACE("------------------------------------------------");

      std::vector<std::vector<real> > ans, reference(3);

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

      app->EvalNDCG(x, ans);
      TS_ASSERT_EQUALS(ans.size(), 3u);
      TS_ASSERT_EQUALS(ans[0].size(), 3u);
      TS_ASSERT_EQUALS(ans, reference);

      reference.erase(reference.begin()+1);

      app->EvalNDIneqCG(x, ans);
      TS_ASSERT_EQUALS(ans.size(), 2u);
      TS_ASSERT_EQUALS(ans[0].size(), 3u);
      TS_ASSERT_EQUALS(ans, reference);

      reference.resize(1);
      reference[0][0] = 1;
      reference[0][1] = 2;
      reference[0][2] = 3;

      app->EvalNDEqCG(x, ans);
      TS_ASSERT_EQUALS(ans.size(), 1u)
      TS_ASSERT_EQUALS(ans[0].size(), 3u);
      TS_ASSERT_EQUALS(ans, reference);

      // Quick test: evaluating when there aren't any constraints (this
      // *should* bypass calling the underlying application)
      app->num_nondeterministic_constraints = 0;
      size_t n = app->eval_count();
      app->EvalNDCG(x, ans);
      TS_ASSERT_EQUALS(ans.size(), 0u);
      TS_ASSERT_EQUALS(app->eval_count(), n);
   }


   void test_eval_mi_ndcg()
   {
      TS_TRACE("------------------------------------------------");
      TS_TRACE("Evaluate constraint gradients (mixed int problem)");
      TS_TRACE("------------------------------------------------");

      std::vector<std::vector<real> > ans, reference(3);

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

      mi_app->EvalNDCG(mi, ans);
      TS_ASSERT_EQUALS(ans.size(), 3u);
      TS_ASSERT_EQUALS(ans[0].size(), 3u);
      TS_ASSERT_EQUALS(ans, reference);

      reference.erase(reference.begin()+1);

      mi_app->EvalNDIneqCG(mi, ans);
      TS_ASSERT_EQUALS(ans.size(), 2u);
      TS_ASSERT_EQUALS(ans[0].size(), 3u);
      TS_ASSERT_EQUALS(ans, reference);

      reference.resize(1);
      reference[0][0] = 4;
      reference[0][1] = 5;
      reference[0][2] = 6;

      mi_app->EvalNDEqCG(mi, ans);
      TS_ASSERT_EQUALS(ans.size(), 1u)
      TS_ASSERT_EQUALS(ans[0].size(), 3u);
      TS_ASSERT_EQUALS(ans, reference);
   }


   void test_get_bounds_properties()
   {
      TS_TRACE("--------------------------------------------------");
      TS_TRACE("Test the method for getting constraint bounds");
      TS_TRACE("--------------------------------------------------");

      std::vector< real > lower(3), upper(3), l, u;
      lower[0] = 0.0;
      lower[1] = 20;
      lower[2] = real::negative_infinity;

      upper[0] = real::positive_infinity;
      upper[1] = 20;
      upper[2] = 1.1;

      TS_ASSERT_EQUALS(app->nondeterministic_constraint_lower_bounds, lower);
      TS_ASSERT_EQUALS(app->nondeterministic_constraint_upper_bounds, upper);

      app->get_constraint_bounds(l, u);
      TS_ASSERT_EQUALS(l, lower);
      TS_ASSERT_EQUALS(u, upper);
   }

   void test_set_bounds_properties()
   {
      TS_TRACE("--------------------------------------------------");
      TS_TRACE("Test the method for setting constraint bounds");
      TS_TRACE("--------------------------------------------------");

      std::vector< real > reference(3);

      //--- Lower ---
      reference[0] = 0.0;
      reference[1] = 20;
      reference[2] = real::negative_infinity;
      TS_ASSERT_EQUALS(app->nondeterministic_constraint_lower_bounds, reference);

      reference[0] = 5;
      app->nondeterministic_constraint_lower_bounds = reference;
      TS_ASSERT_EQUALS(app->nondeterministic_constraint_lower_bounds, reference);

      reference.push_back(0);
      TS_ASSERT_THROWS_ASSERT
         ( app->nondeterministic_constraint_lower_bounds = reference, 
           std::runtime_error &e,
           TEST_WHAT(e, "Application_NonD_Constraints::"
                     "cb_validate_vector(): vector length (4) does not "
                     "match num_nondeterministic_constraints (3)") );
      reference.resize(3);
      TS_ASSERT_EQUALS(app->nondeterministic_constraint_lower_bounds, reference);
      
      reference.resize(2);
      TS_ASSERT_THROWS_ASSERT
         ( app->nondeterministic_constraint_lower_bounds = reference, 
           std::runtime_error &e,
           TEST_WHAT(e, "Application_NonD_Constraints::"
                     "cb_validate_vector(): vector length (2) does not "
                     "match num_nondeterministic_constraints (3)") );
      reference.push_back(real::negative_infinity);
      TS_ASSERT_EQUALS(app->nondeterministic_constraint_lower_bounds, reference);

      //--- Upper ---
      reference.resize(3);
      reference[0] = real::positive_infinity;
      reference[1] = 20;
      reference[2] = 1.1;

      reference[0] = 5;
      app->nondeterministic_constraint_upper_bounds = reference;
      TS_ASSERT_EQUALS(app->nondeterministic_constraint_upper_bounds, reference);

      reference.push_back(0);
      TS_ASSERT_THROWS_ASSERT
         ( app->nondeterministic_constraint_upper_bounds = reference, 
           std::runtime_error &e,
           TEST_WHAT(e, "Application_NonD_Constraints::"
                     "cb_validate_vector(): vector length (4) does not "
                     "match num_nondeterministic_constraints (3)") );
      reference.resize(3);
      TS_ASSERT_EQUALS(app->nondeterministic_constraint_upper_bounds, reference);
      
      reference.resize(2);
      TS_ASSERT_THROWS_ASSERT
         ( app->nondeterministic_constraint_upper_bounds = reference, 
           std::runtime_error &e,
           TEST_WHAT(e, "Application_NonD_Constraints::"
                     "cb_validate_vector(): vector length (2) does not "
                     "match num_nondeterministic_constraints (3)") );
      reference.push_back(1.1);
      TS_ASSERT_EQUALS(app->nondeterministic_constraint_upper_bounds, reference);
   }

   void test_get_bound_method()
   {
      TS_TRACE("--------------------------------------------------");
      TS_TRACE("Test getting a single constraint bound");
      TS_TRACE("--------------------------------------------------");

      real lower, upper;
      app->nondeterministicConstraintBound(0, lower, upper);
      TS_ASSERT_EQUALS(lower, 0.0);
      TS_ASSERT_EQUALS(upper, real::positive_infinity);

      try {
        app->nondeterministicConstraintBound(3, lower, upper);
        TS_FAIL("Expected exception");
    }
    catch (std::runtime_error& ) {}

   }

   void test_get_equality_bounds()
   {
      TS_TRACE("--------------------------------------------------");
      TS_TRACE("Test getting ineq and eq constraint bounds");
      TS_TRACE("--------------------------------------------------");

      std::vector< real > lower, upper;

      std::vector< real > lans(2), uans(2);
      lans[0] = 0.0;
      lans[1] = real::negative_infinity;
      uans[0] = real::positive_infinity;
      uans[1] = 1.1;
      app->nondeterministicIneqConstraintBounds(lower, upper);
      TS_ASSERT_EQUALS(lower, lans);
      TS_ASSERT_EQUALS(upper, uans);

      lans.resize(1);
      lans[0] = 20;
      app->nondeterministicEqConstraintBounds(lower);
      TS_ASSERT_EQUALS(lower, lans);
   }


   void test_constraint_labels()
   {
      labels_t lbl;
      lbl.insert( labels_t::value_type(0, "con 1") );
      lbl.insert( labels_t::value_type(1, "con 2") );
      lbl.insert( labels_t::value_type(2, "con 3") );
      app->nondeterministic_constraint_labels = lbl;

      labels_t ans;
      ans = app->nondeterministic_constraint_labels;
      TS_ASSERT_EQUALS(lbl, ans);

      app->setNondeterministicConstraintLabel(1, "foo");
      lbl.left.erase(1);
      lbl.insert( labels_t::value_type(1, "foo") );
      ans = app->nondeterministic_constraint_labels;
      TS_ASSERT_EQUALS(lbl, ans);

      app->setNondeterministicConstraintLabel(1, "");
      lbl.left.erase(1);
      ans = app->nondeterministic_constraint_labels;
      TS_ASSERT_EQUALS(lbl, ans);

      TS_ASSERT_THROWS_ASSERT
         ( app->setNondeterministicConstraintLabel(3, "foo"), 
           std::runtime_error &e,
           TEST_WHAT( e, "Application_NonD_Constraints::"
                      "_setNondeterministicConstraintLabel(): index (3) past "
                      "num_nondeterministic_constraints (3)" ) );

      lbl.insert( labels_t::value_type(3, "bad con") );
      TS_ASSERT_THROWS_ASSERT
         ( app->nondeterministic_constraint_labels = lbl, std::runtime_error &e,
           TEST_WHAT( e, "Application_NonD_Constraints::"
                     "cb_validate_labels(): label id (3) is greater than "
                      "num_nondeterministic_constraints (3)" ) );
   }

   void test_resize()
   {
      labels_t lbl;
      lbl.insert( labels_t::value_type(0, "con 1") );
      lbl.insert( labels_t::value_type(1, "con 2") );
      lbl.insert( labels_t::value_type(2, "con 3") );
      app->nondeterministic_constraint_labels = lbl;

      app->num_nondeterministic_constraints = 1;

      TS_ASSERT_EQUALS(app->num_constraints, 1u);
      TS_ASSERT_EQUALS(app->num_nondeterministic_constraints, 1u);
      TS_ASSERT_EQUALS(app->numEqConstraints(), 0u);
      TS_ASSERT_EQUALS(app->numIneqConstraints(), 1u);
      TS_ASSERT_EQUALS(app->numNondeterministicEqConstraints(), 0u);
      TS_ASSERT_EQUALS(app->numNondeterministicIneqConstraints(), 1u);
      
      std::vector< real > bnd(1, 0.0);
      TS_ASSERT_EQUALS(app->nondeterministic_constraint_lower_bounds, bnd);
      bnd[0] = real::positive_infinity;
      TS_ASSERT_EQUALS(app->nondeterministic_constraint_upper_bounds, bnd);

      lbl.clear();
      lbl.insert( labels_t::value_type(0, "con 1") );
      TS_ASSERT_EQUALS(app->nondeterministic_constraint_labels, lbl);


      app->num_nondeterministic_constraints = 2;
      TS_ASSERT_EQUALS(app->num_constraints, 2u);
      TS_ASSERT_EQUALS(app->num_nondeterministic_constraints, 2u);
      TS_ASSERT_EQUALS(app->numEqConstraints(), 0u);
      TS_ASSERT_EQUALS(app->numIneqConstraints(), 2u);
      TS_ASSERT_EQUALS(app->numNondeterministicEqConstraints(), 0u);
      TS_ASSERT_EQUALS(app->numNondeterministicIneqConstraints(), 2u);
      
      bnd[0] = 0.0;
      bnd.push_back(real::negative_infinity);
      TS_ASSERT_EQUALS(app->nondeterministic_constraint_lower_bounds, bnd);
      bnd[0] = real::positive_infinity;
      bnd[1] = real::positive_infinity;
      TS_ASSERT_EQUALS(app->nondeterministic_constraint_upper_bounds, bnd);

      TS_ASSERT_EQUALS(app->nondeterministic_constraint_labels, lbl);


      app->num_nondeterministic_constraints = 0;

      TS_ASSERT_EQUALS(app->num_constraints, 0u);
      TS_ASSERT_EQUALS(app->num_nondeterministic_constraints, 0u);
      TS_ASSERT_EQUALS(app->numEqConstraints(), 0u);
      TS_ASSERT_EQUALS(app->numIneqConstraints(), 0u);
      TS_ASSERT_EQUALS(app->numNondeterministicEqConstraints(), 0u);
      TS_ASSERT_EQUALS(app->numNondeterministicIneqConstraints(), 0u);
      
      bnd.clear();
      TS_ASSERT_EQUALS(app->nondeterministic_constraint_lower_bounds, bnd);
      TS_ASSERT_EQUALS(app->nondeterministic_constraint_upper_bounds, bnd);

      lbl.clear();
      TS_ASSERT_EQUALS(app->nondeterministic_constraint_labels, lbl);
   }

   void test_cb_print()
   {
      app->setNondeterministicConstraintLabel(1, "foo");

      std::ostringstream os;
      app->colin::Application_NonD_Constraints::cb_print(os);

      std::string reference = 
"Nondeterministic constraints:  3\n"
"  Equality:               1\n"
"  Inequality:             2\n"
"Index               Label    Lower Bound    Upper Bound\n"
"    1              _none_              0       Infinity\n"
"    2                 foo             20             20\n"
"    3              _none_      -Infinity            1.1\n";
      TS_ASSERT_EQUALS(os.str(), reference);

      app->num_nondeterministic_constraints = 0;
      reference.erase(reference.find("Index"));
      reference[reference.find("1")] = '0';
      reference[reference.find("2")] = '0';
      reference[reference.find("3")] = '0';
      os.str("");
      app->colin::Application_NonD_Constraints::cb_print(os);
      TS_ASSERT_EQUALS(os.str(), reference);
   }

};

} // namespace colin
