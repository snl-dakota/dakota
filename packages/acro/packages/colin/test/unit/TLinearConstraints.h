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

class colin::unittest::Application_LinearConstraints       \
   : public CxxTest::TestSuite
{
public:
   typedef boost::bimap<size_t, std::string>  labels_t;

   // The test application
   TestApplications::singleObj_denseCon<LP_problem> *app;
   TestApplications::mi_singleObj_denseCon<MILP_problem> *mi_app;

   std::vector<real>  x;
   utilib::MixedIntVars mi;

   Application_LinearConstraints()
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
      utilib::exception_mngr::set_mode(utilib::exception_mngr::Standard);
      app = new TestApplications::singleObj_denseCon<LP_problem>(3);
      mi_app = new TestApplications::mi_singleObj_denseCon<MILP_problem>(6);

      std::vector<real> c(3);

      c[0] = 0.0;
      c[1] = 15;
      c[2] = real::negative_infinity;
      app->linear_constraint_lower_bounds = c;
      mi_app->linear_constraint_lower_bounds = c;

      c[0] = real::positive_infinity;
      c[1] = 15;
      c[2] = 1.1;
      app->linear_constraint_upper_bounds = c;
      mi_app->linear_constraint_upper_bounds = c;
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
         TS_FAIL("Failed to construct application with linear constraints."); 
         return;
      }
      TS_ASSERT_EQUALS(app->num_constraints, 3u);
      TS_ASSERT_EQUALS(app->num_linear_constraints, 3u);
      TS_ASSERT_EQUALS(app->numEqConstraints(), 1u);
      TS_ASSERT_EQUALS(app->numIneqConstraints(), 2u);
      TS_ASSERT_EQUALS(app->numLinearEqConstraints(), 1u);
      TS_ASSERT_EQUALS(app->numLinearIneqConstraints(), 2u);
   }

   void test_eval_lcf()
   {
      TS_TRACE("------------------------------------------------");
      TS_TRACE("evalcf - Evaluate constraints");
      TS_TRACE("------------------------------------------------");

      utilib::BasicArray<real> ans, reference(3);
      reference[0] = 7;
      reference[1] = 14;
      reference[2] = 17;

      app->EvalLCF(x, ans);
      TS_ASSERT_EQUALS(ans.size(), 3u)
      TS_ASSERT_DELTA(ans[0], reference[0], 1e-7);
      TS_ASSERT_DELTA(ans[1], reference[1], 1e-7);
      TS_ASSERT_DELTA(ans[2], reference[2], 1e-7);

      app->EvalLEqCF(x, ans);
      TS_ASSERT_EQUALS(ans.size(), 1u)
      TS_ASSERT_DELTA(ans[0], reference[1], 1e-7);

      app->EvalLIneqCF(x, ans);
      TS_ASSERT_EQUALS(ans.size(), 2u)
      TS_ASSERT_DELTA(ans[0], reference[0], 1e-7);
      TS_ASSERT_DELTA(ans[1], reference[2], 1e-7);

      // Quick test: evaluating when there aren't any constraints (this
      // *should* bypass calling the underlying application)
      app->num_linear_constraints = 0;
      size_t n = app->eval_count();
      app->EvalLCF(x, ans);
      TS_ASSERT_EQUALS(ans.size(), 0u);
      TS_ASSERT_EQUALS(app->eval_count(), n);
   }

   void test_eval_lcvf()
   {
      TS_TRACE("------------------------------------------------");
      TS_TRACE("evalcvf - Evaluate constraint violations");
      TS_TRACE("------------------------------------------------");

      utilib::BasicArray<real> ans, reference(3);
      reference[0] = 0;
      reference[1] = -1;
      reference[2] = 15.9;

      app->EvalLCFViol(x, ans);
      TS_ASSERT_EQUALS(ans.size(), 3u)
      TS_ASSERT_DELTA(ans[0], reference[0], 1e-7);
      TS_ASSERT_DELTA(ans[1], reference[1], 1e-7);
      TS_ASSERT_DELTA(ans[2], reference[2], 1e-7);
   }

   void test_async_eval_lcf()
   {
      TS_TRACE("--------------------------------------------------");
      TS_TRACE("async_evalcf - Evaluate constraints asynchronously");
      TS_TRACE("--------------------------------------------------");

      utilib::BasicArray<real> ans, reference(3);
      reference[0] = 7;
      reference[1] = 14;
      reference[2] = 17;

      app->AsyncEvalLCF(app->eval_mngr(), x, ans);
      app->synchronize();
      TS_ASSERT_EQUALS(ans.size(), 3u)
      TS_ASSERT_DELTA(ans[0], reference[0], 1e-7);
      TS_ASSERT_DELTA(ans[1], reference[1], 1e-7);
      TS_ASSERT_DELTA(ans[2], reference[2], 1e-7);

      app->AsyncEvalLEqCF(app->eval_mngr(), x, ans);
      app->synchronize();
      TS_ASSERT_EQUALS(ans.size(), 1u)
      TS_ASSERT_DELTA(ans[0], reference[1], 1e-7);

      app->AsyncEvalLIneqCF(app->eval_mngr(), x, ans);
      app->synchronize();
      TS_ASSERT_EQUALS(ans.size(), 2u)
      TS_ASSERT_DELTA(ans[0], reference[0], 1e-7);
      TS_ASSERT_DELTA(ans[1], reference[2], 1e-7);
   }

   void test_eval_lcg()
   {
      TS_TRACE("------------------------------------------------");
      TS_TRACE("Evaluate constraint gradients");
      TS_TRACE("------------------------------------------------");

      std::vector<std::vector<real> > ans, reference(3);

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

      app->EvalLCG(x, ans);
      TS_ASSERT_EQUALS(ans.size(), 3u);
      TS_ASSERT_EQUALS(ans[0].size(), 3u);
      TS_ASSERT_EQUALS(ans, reference);

      reference.erase(reference.begin()+1);

      app->EvalLIneqCG(x, ans);
      TS_ASSERT_EQUALS(ans.size(), 2u);
      TS_ASSERT_EQUALS(ans[0].size(), 3u);
      TS_ASSERT_EQUALS(ans, reference);

      reference.resize(1);
      reference[0][0] = 2;
      reference[0][1] = 2;
      reference[0][2] = 2;

      app->EvalLEqCG(x, ans);
      TS_ASSERT_EQUALS(ans.size(), 1u)
      TS_ASSERT_EQUALS(ans[0].size(), 3u);
      TS_ASSERT_EQUALS(ans, reference);
   }


   void test_eval_mi_lcg()
   {
      TS_TRACE("------------------------------------------------");
      TS_TRACE("Evaluate constraint gradients (mixed int problem)");
      TS_TRACE("------------------------------------------------");

      std::vector<std::vector<real> > ans, reference(3);

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

      mi_app->EvalLCG(mi, ans);
      TS_ASSERT_EQUALS(ans.size(), 3u);
      TS_ASSERT_EQUALS(ans[0].size(), 3u);
      TS_ASSERT_EQUALS(ans, reference);

      reference.erase(reference.begin()+1);

      mi_app->EvalLIneqCG(mi, ans);
      TS_ASSERT_EQUALS(ans.size(), 2u);
      TS_ASSERT_EQUALS(ans[0].size(), 3u);
      TS_ASSERT_EQUALS(ans, reference);

      reference.resize(1);
      reference[0][0] = 2;
      reference[0][1] = 2;
      reference[0][2] = 2;

      mi_app->EvalLEqCG(mi, ans);
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
      lower[1] = 15;
      lower[2] = real::negative_infinity;

      upper[0] = real::positive_infinity;
      upper[1] = 15;
      upper[2] = 1.1;

      TS_ASSERT_EQUALS(app->linear_constraint_lower_bounds, lower);
      TS_ASSERT_EQUALS(app->linear_constraint_upper_bounds, upper);

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
      reference[1] = 15;
      reference[2] = real::negative_infinity;
      TS_ASSERT_EQUALS(app->linear_constraint_lower_bounds, reference);

      reference[0] = 5;
      app->linear_constraint_lower_bounds = reference;
      TS_ASSERT_EQUALS(app->linear_constraint_lower_bounds, reference);

      reference.push_back(0);
      TS_ASSERT_THROWS_ASSERT
         ( app->linear_constraint_lower_bounds = reference, 
           std::runtime_error &e,
           TEST_WHAT(e, "Application_LinearConstraints::"
                     "cb_validate_vector(): vector length (4) does not "
                     "match num_linear_constraints (3)") );
      reference.resize(3);
      TS_ASSERT_EQUALS(app->linear_constraint_lower_bounds, reference);
      
      reference.resize(2);
      TS_ASSERT_THROWS_ASSERT
         ( app->linear_constraint_lower_bounds = reference, 
           std::runtime_error &e,
           TEST_WHAT(e, "Application_LinearConstraints::"
                     "cb_validate_vector(): vector length (2) does not "
                     "match num_linear_constraints (3)") );
      reference.push_back(real::negative_infinity);
      TS_ASSERT_EQUALS(app->linear_constraint_lower_bounds, reference);

      //--- Upper ---
      reference.resize(3);
      reference[0] = real::positive_infinity;
      reference[1] = 15;
      reference[2] = 1.1;

      reference[0] = 5;
      app->linear_constraint_upper_bounds = reference;
      TS_ASSERT_EQUALS(app->linear_constraint_upper_bounds, reference);

      reference.push_back(0);
      TS_ASSERT_THROWS_ASSERT
         ( app->linear_constraint_upper_bounds = reference, 
           std::runtime_error &e,
           TEST_WHAT(e, "Application_LinearConstraints::"
                     "cb_validate_vector(): vector length (4) does not "
                     "match num_linear_constraints (3)") );
      reference.resize(3);
      TS_ASSERT_EQUALS(app->linear_constraint_upper_bounds, reference);
      
      reference.resize(2);
      TS_ASSERT_THROWS_ASSERT
         ( app->linear_constraint_upper_bounds = reference, 
           std::runtime_error &e,
           TEST_WHAT(e, "Application_LinearConstraints::"
                     "cb_validate_vector(): vector length (2) does not "
                     "match num_linear_constraints (3)") );
      reference.push_back(1.1);
      TS_ASSERT_EQUALS(app->linear_constraint_upper_bounds, reference);
   }

   void test_get_bound_method()
   {
      TS_TRACE("--------------------------------------------------");
      TS_TRACE("Test getting a single constraint bound");
      TS_TRACE("--------------------------------------------------");

      real lower, upper;
      app->linearConstraintBound(0, lower, upper);
      TS_ASSERT_EQUALS(lower, 0.0);
      TS_ASSERT_EQUALS(upper, real::positive_infinity);

      try {
        app->linearConstraintBound(3, lower, upper);
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
      app->linearIneqConstraintBounds(lower, upper);
      TS_ASSERT_EQUALS(lower, lans);
      TS_ASSERT_EQUALS(upper, uans);

      lans.resize(1);
      lans[0] = 15;
      app->linearEqConstraintBounds(lower);
      TS_ASSERT_EQUALS(lower, lans);
   }

   void test_assign_linear_constraint_matrix()
   {
      TS_TRACE("--------------------------------------------------");
      TS_TRACE("Test getting/setting linear constraint matrix");
      TS_TRACE("--------------------------------------------------");

      utilib::BasicArray< utilib::BasicArray<double>  > mat
         = app->linear_constraint_matrix;
      TS_ASSERT( mat.size() == 0 ); 

      utilib::BasicArray< utilib::BasicArray<double> > Mat(2);
      TS_ASSERT_THROWS_ASSERT
         ( app->linear_constraint_matrix = Mat, std::runtime_error &e,
           TEST_WHAT(e, "Application_LinearConstraintGradients::"
                     "cb_validate_matrix(): number of rows (2) "
                     "does not match num_linear_constraints (3)") );

      Mat.resize(3);
      for (int i=0; i<3; i++) {
         Mat[i].resize(3);
         Mat[i] << 0.0;
         Mat[i][i] = i;
      }
      app->linear_constraint_matrix = Mat;

      mat = app->linear_constraint_matrix;
      TS_ASSERT_EQUALS(mat.size(), 3u);
      TS_ASSERT_EQUALS(mat[0].size(), 3u);
      TS_ASSERT_EQUALS(mat, Mat);

      Mat[2].resize(4);
      // Initialize the new value because BasicArray::resize() doesn't!
      Mat[2][3] = 0; 
      TS_ASSERT_THROWS_ASSERT
         ( app->linear_constraint_matrix = Mat, std::runtime_error &e,
           TEST_WHAT(e, "Application_LinearConstraintGradients::"
                     "cb_validate_matrix(): number of columns (4) "
                     "does not match domain_size (3)") );
   }


   void test_evaluate_linear_constraint_matrix()
   {
      TS_TRACE("--------------------------------------------------");
      TS_TRACE("Test computing with a local linear constraint matrix");
      TS_TRACE("--------------------------------------------------");

      // use the *same* matrix
      TS_ASSERT_EQUALS(app->eval_count(), 0u);
      app->linear_constraint_matrix = app->EvalLCG(x).get(lcg_info);
      TS_ASSERT_EQUALS(app->eval_count(), 1u);

      std::vector<real> ans, reference(3);
      reference[0] = 7;
      reference[1] = 14;
      reference[2] = 17;

      app->EvalLCF(x, ans);
      TS_ASSERT_EQUALS(app->eval_count(), 1u);
      TS_ASSERT_EQUALS(ans.size(), 3u)
      TS_ASSERT_DELTA(ans[0], reference[0], 1e-7);
      TS_ASSERT_DELTA(ans[1], reference[1], 1e-7);
      TS_ASSERT_DELTA(ans[2], reference[2], 1e-7);

      app->EvalLEqCF(x, ans);
      TS_ASSERT_EQUALS(app->eval_count(), 1u);
      TS_ASSERT_EQUALS(ans.size(), 1u)
      TS_ASSERT_DELTA(ans[0], reference[1], 1e-7);

      app->EvalLIneqCF(x, ans);
      TS_ASSERT_EQUALS(app->eval_count(), 1u);
      TS_ASSERT_EQUALS(ans.size(), 2u)
      TS_ASSERT_DELTA(ans[0], reference[0], 1e-7);
      TS_ASSERT_DELTA(ans[1], reference[2], 1e-7);

      // Evaluating the LCG of an LP should return the matrix...
      std::vector<std::vector<real> > Mat
         = app->linear_constraint_matrix;
      std::vector<std::vector<real> > Mat_ans;
      app->EvalLCG(x, Mat_ans);
      TS_ASSERT_EQUALS(app->eval_count(), 1u);
      TS_ASSERT_EQUALS(Mat, Mat_ans);

      //
      // Reset the matrix to the identity matrix...
      //

      for(size_t i = 0; i<3; ++i)
         for(size_t j = 0; j<3; ++j)
            Mat[i][j] = ( i == j ? 1 : 0 );
      app->linear_constraint_matrix = Mat;
      reference = x;

      app->EvalLCF(x, ans);
      TS_ASSERT_EQUALS(app->eval_count(), 1u);
      TS_ASSERT_EQUALS(ans.size(), 3u)
      TS_ASSERT_DELTA(ans[0], reference[0], 1e-7);
      TS_ASSERT_DELTA(ans[1], reference[1], 1e-7);
      TS_ASSERT_DELTA(ans[2], reference[2], 1e-7);

      app->EvalLEqCF(x, ans);
      TS_ASSERT_EQUALS(app->eval_count(), 1u);
      TS_ASSERT_EQUALS(ans.size(), 1u)
      TS_ASSERT_DELTA(ans[0], reference[1], 1e-7);

      app->EvalLIneqCF(x, ans);
      TS_ASSERT_EQUALS(app->eval_count(), 1u);
      TS_ASSERT_EQUALS(ans.size(), 2u)
      TS_ASSERT_DELTA(ans[0], reference[0], 1e-7);
      TS_ASSERT_DELTA(ans[1], reference[2], 1e-7);

      // Evaluating the LCG should return the matrix...
      app->EvalLCG(x, Mat_ans);
      TS_ASSERT_EQUALS(app->eval_count(), 1u);
      TS_ASSERT_EQUALS(Mat, Mat_ans);

      //
      // Calculations on an empty constraint set should return empty
      // vectors without calling perform_evaluation()
      //

      app->num_linear_constraints = 0;
      app->EvalLCF(x, ans);
      TS_ASSERT_EQUALS(app->eval_count(), 1u);
      TS_ASSERT_EQUALS(ans.size(), 0u)

      app->EvalLEqCF(x, ans);
      TS_ASSERT_EQUALS(app->eval_count(), 1u);
      TS_ASSERT_EQUALS(ans.size(), 0u)

      app->EvalLIneqCF(x, ans);
      TS_ASSERT_EQUALS(app->eval_count(), 1u);
      TS_ASSERT_EQUALS(ans.size(), 0u)

      // Evaluating the LCG should return the matrix...
      app->EvalLCG(x, Mat_ans);
      TS_ASSERT_EQUALS(app->eval_count(), 1u);
      TS_ASSERT_EQUALS(Mat_ans.size(), 0u);
   }


   void test_mixed_integer_assign_linear_constraint_matrix()
   {
      TS_TRACE("--------------------------------------------------");
      TS_TRACE("Test getting/setting linear constraint matrix");
      TS_TRACE("--------------------------------------------------");

      utilib::BasicArray< utilib::BasicArray<double>  > mat
         = mi_app->linear_constraint_matrix;
      TS_ASSERT( mat.size() == 0 ); 

      utilib::BasicArray< utilib::BasicArray<double> > Mat(2);
      TS_ASSERT_THROWS_ASSERT
         ( mi_app->linear_constraint_matrix = Mat, std::runtime_error &e,
           TEST_WHAT(e, "Application_LinearConstraintGradients::"
                     "cb_validate_matrix(): number of rows (2) "
                     "does not match num_linear_constraints (3)") );

      Mat.resize(3);
      for (int i=0; i<3; i++) {
         Mat[i].resize(6);
         Mat[i] << 0.0;
         Mat[i][i] = i;
      }
      mi_app->linear_constraint_matrix = Mat;

      mat = mi_app->linear_constraint_matrix;
      TS_ASSERT_EQUALS(mat.size(), 3u);
      TS_ASSERT_EQUALS(mat[0].size(), 6u);
      TS_ASSERT_EQUALS(mat, Mat);

      for (int i=0; i<3; Mat[i++].resize(5));
      TS_ASSERT_THROWS_ASSERT
         ( mi_app->linear_constraint_matrix = Mat, std::runtime_error &e,
           TEST_WHAT(e, "Application_LinearConstraintGradients::"
                     "cb_validate_matrix(): number of columns (5) "
                     "does not match domain_size (6)") );
   }


   void test_evaluate_mixed_integer_linear_constraint_matrix()
   {
      TS_TRACE("--------------------------------------------------");
      TS_TRACE("Test computing with a local linear constraint matrix");
      TS_TRACE("--------------------------------------------------");

      std::vector<real> ans, reference(3);
      reference[0] = 13;
      reference[1] = 26;
      reference[2] = 52;

      // check the base evaluation
      TS_ASSERT_EQUALS(mi_app->eval_count(), 0u);
      TS_ASSERT_EQUALS(mi_app->response_count(), 0u);
      mi_app->EvalLCF(mi, ans);
      TS_ASSERT_EQUALS(mi_app->eval_count(), 1u);
      TS_ASSERT_EQUALS(mi_app->response_count(), 1u);
      TS_ASSERT_EQUALS(ans.size(), 3u)
      TS_ASSERT_DELTA(ans[0], reference[0], 1e-7);
      TS_ASSERT_DELTA(ans[1], reference[1], 1e-7);
      TS_ASSERT_DELTA(ans[2], reference[2], 1e-7);

      // use the *same* matrix
      TS_ASSERT_EQUALS(mi_app->eval_count(), 1u);
      TS_ASSERT_EQUALS(mi_app->response_count(), 1u);
      std::vector<std::vector<real> > Mat;
      mi_app->EvalLCG(mi, Mat);
      TS_ASSERT_THROWS_ASSERT
         ( mi_app->linear_constraint_matrix = Mat, std::runtime_error &e,
           TEST_WHAT(e, "Application_LinearConstraintGradients::"
                     "cb_validate_matrix(): number of columns (3) "
                     "does not match domain_size (6)") );
      for(size_t i=0; i<3; Mat[i++].resize(6));
      for(size_t i=0; i<6; i++)
      {
         Mat[0][i] = 1;
         Mat[1][i] = 2;
         Mat[2][i] = i+1;
      }
      mi_app->linear_constraint_matrix = Mat;
      TS_ASSERT_EQUALS(mi_app->eval_count(), 2u);
      TS_ASSERT_EQUALS(mi_app->response_count(), 2u);

      mi_app->EvalLCF(mi, ans);
      TS_ASSERT_EQUALS(mi_app->eval_count(), 2u);
      TS_ASSERT_EQUALS(mi_app->response_count(), 3u);
      TS_ASSERT_EQUALS(ans.size(), 3u)
      TS_ASSERT_DELTA(ans[0], reference[0], 1e-7);
      TS_ASSERT_DELTA(ans[1], reference[1], 1e-7);
      TS_ASSERT_DELTA(ans[2], reference[2], 1e-7);

      mi_app->EvalLEqCF(mi, ans);
      TS_ASSERT_EQUALS(mi_app->eval_count(), 2u);
      TS_ASSERT_EQUALS(mi_app->response_count(), 4u);
      TS_ASSERT_EQUALS(ans.size(), 1u)
      TS_ASSERT_DELTA(ans[0], reference[1], 1e-7);

      mi_app->EvalLIneqCF(mi, ans);
      TS_ASSERT_EQUALS(mi_app->eval_count(), 2u);
      TS_ASSERT_EQUALS(mi_app->response_count(), 5u);
      TS_ASSERT_EQUALS(ans.size(), 2u)
      TS_ASSERT_DELTA(ans[0], reference[0], 1e-7);
      TS_ASSERT_DELTA(ans[1], reference[2], 1e-7);

      // Evaluating the LCG of an MILP should return the real half of
      // the matrix...
      for(size_t i=0; i<3; ++i)
         Mat[i].erase(Mat[i].begin(), Mat[i].begin() + 3);
      std::vector<std::vector<real> > Mat_ans;
      mi_app->EvalLCG(mi, Mat_ans);
      TS_ASSERT_EQUALS(mi_app->eval_count(), 2u);
      TS_ASSERT_EQUALS(mi_app->response_count(), 6u);
      TS_ASSERT_EQUALS(Mat, Mat_ans);

      //
      // Reset the real-part of the matrix to the identity matrix...
      //

      for(size_t i = 0; i<3; ++i)
      {
         Mat[i].resize(6);
         for(size_t j = 0; j<6; ++j)
            Mat[i][j] = ( i+3 == j ? 1 : 0 );
      }
      mi_app->linear_constraint_matrix = Mat;
      utilib::TypeManager()->lexical_cast(mi.Real(), reference);

      mi_app->EvalLCF(mi, ans);
      TS_ASSERT_EQUALS(mi_app->eval_count(), 2u);
      TS_ASSERT_EQUALS(mi_app->response_count(), 7u);
      TS_ASSERT_EQUALS(ans.size(), 3u)
      TS_ASSERT_DELTA(ans[0], reference[0], 1e-7);
      TS_ASSERT_DELTA(ans[1], reference[1], 1e-7);
      TS_ASSERT_DELTA(ans[2], reference[2], 1e-7);

      mi_app->EvalLEqCF(mi, ans);
      TS_ASSERT_EQUALS(mi_app->eval_count(), 2u);
      TS_ASSERT_EQUALS(mi_app->response_count(), 8u);
      TS_ASSERT_EQUALS(ans.size(), 1u)
      TS_ASSERT_DELTA(ans[0], reference[1], 1e-7);

      mi_app->EvalLIneqCF(mi, ans);
      TS_ASSERT_EQUALS(mi_app->eval_count(), 2u);
      TS_ASSERT_EQUALS(mi_app->response_count(), 9u);
      TS_ASSERT_EQUALS(ans.size(), 2u)
      TS_ASSERT_DELTA(ans[0], reference[0], 1e-7);
      TS_ASSERT_DELTA(ans[1], reference[2], 1e-7);

      // Evaluating the LCG should return the matrix...
      mi_app->EvalLCG(mi, Mat_ans);
      TS_ASSERT_EQUALS(mi_app->eval_count(), 2u);
      TS_ASSERT_EQUALS(mi_app->response_count(), 10u);
      for(size_t i=0; i<3; ++i)
         Mat[i].erase(Mat[i].begin(), Mat[i].begin() + 3);
      TS_ASSERT_EQUALS(Mat, Mat_ans);

      //
      // Calculations on an empty constraint set should return empty
      // vectors without calling perform_evaluation()
      //

      mi_app->num_linear_constraints = 0;
      mi_app->EvalLCF(mi, ans);
      TS_ASSERT_EQUALS(mi_app->eval_count(), 2u);
      TS_ASSERT_EQUALS(mi_app->response_count(), 11u);
      TS_ASSERT_EQUALS(ans.size(), 0u)

      mi_app->EvalLEqCF(mi, ans);
      TS_ASSERT_EQUALS(mi_app->eval_count(), 2u);
      TS_ASSERT_EQUALS(mi_app->response_count(), 12u);
      TS_ASSERT_EQUALS(ans.size(), 0u)

      mi_app->EvalLIneqCF(mi, ans);
      TS_ASSERT_EQUALS(mi_app->eval_count(), 2u);
      TS_ASSERT_EQUALS(mi_app->response_count(), 13u);
      TS_ASSERT_EQUALS(ans.size(), 0u)

      // Evaluating the LCG should return the matrix...
      mi_app->EvalLCG(mi, Mat_ans);
      TS_ASSERT_EQUALS(mi_app->eval_count(), 2u);
      TS_ASSERT_EQUALS(mi_app->response_count(), 14u);
      TS_ASSERT_EQUALS(Mat_ans.size(), 0u);
   }


   void test_constraint_labels()
   {
      TS_ASSERT_EQUALS(app->linearConstraintLabel(0), "lc[1]");
      TS_ASSERT_EQUALS(app->linearConstraintLabel(1), "");
      TS_ASSERT_EQUALS(app->linearConstraintLabel(2), "lc[3]");
      TS_ASSERT_THROWS_ASSERT
         ( app->linearConstraintLabel(3), std::runtime_error &e, 
           TEST_WHAT(e, "Application_LinearConstraints"
                     "::linearConstraintLabel(): specified index (3) "
                     "out of range (max = 2)") );

      labels_t lbl;
      lbl.insert( labels_t::value_type(0, "con 1") );
      lbl.insert( labels_t::value_type(1, "con 2") );
      lbl.insert( labels_t::value_type(2, "con 3") );
      app->linear_constraint_labels = lbl;

      labels_t ans;
      ans = app->linear_constraint_labels;
      TS_ASSERT_EQUALS(lbl, ans);

      app->setLinearConstraintLabel(1, "foo");
      lbl.left.erase(1);
      lbl.insert( labels_t::value_type(1, "foo") );
      ans = app->linear_constraint_labels;
      TS_ASSERT_EQUALS(lbl, ans);

      app->setLinearConstraintLabel(1, "");
      lbl.left.erase(1);
      ans = app->linear_constraint_labels;
      TS_ASSERT_EQUALS(lbl, ans);

      TS_ASSERT_THROWS_ASSERT
         ( app->setLinearConstraintLabel(3, "foo"), std::runtime_error &e,
           TEST_WHAT( e, "Application_LinearConstraints::"
                      "_setLinearConstraintLabel(): index (3) past "
                      "num_linear_constraints (3)" ) );

      lbl.insert( labels_t::value_type(3, "bad con") );
      TS_ASSERT_THROWS_ASSERT
         ( app->linear_constraint_labels = lbl, std::runtime_error &e,
           TEST_WHAT( e, "Application_LinearConstraints::"
                     "cb_validate_labels(): label id (3) is greater than "
                      "num_linear_constraints (3)" ) );
   }

   void test_resize()
   {
      labels_t lbl;
      lbl.insert( labels_t::value_type(0, "con 1") );
      lbl.insert( labels_t::value_type(1, "con 2") );
      lbl.insert( labels_t::value_type(2, "con 3") );
      app->linear_constraint_labels = lbl;

      app->num_linear_constraints = 1;

      TS_ASSERT_EQUALS(app->num_constraints, 1u);
      TS_ASSERT_EQUALS(app->num_linear_constraints, 1u);
      TS_ASSERT_EQUALS(app->numEqConstraints(), 0u);
      TS_ASSERT_EQUALS(app->numIneqConstraints(), 1u);
      TS_ASSERT_EQUALS(app->numLinearEqConstraints(), 0u);
      TS_ASSERT_EQUALS(app->numLinearIneqConstraints(), 1u);
      
      std::vector< real > bnd(1, 0.0);
      TS_ASSERT_EQUALS(app->linear_constraint_lower_bounds, bnd);
      bnd[0] = real::positive_infinity;
      TS_ASSERT_EQUALS(app->linear_constraint_upper_bounds, bnd);

      lbl.clear();
      lbl.insert( labels_t::value_type(0, "con 1") );
      TS_ASSERT_EQUALS(app->linear_constraint_labels, lbl);


      app->num_linear_constraints = 2;
      TS_ASSERT_EQUALS(app->num_constraints, 2u);
      TS_ASSERT_EQUALS(app->num_linear_constraints, 2u);
      TS_ASSERT_EQUALS(app->numEqConstraints(), 0u);
      TS_ASSERT_EQUALS(app->numIneqConstraints(), 2u);
      TS_ASSERT_EQUALS(app->numLinearEqConstraints(), 0u);
      TS_ASSERT_EQUALS(app->numLinearIneqConstraints(), 2u);
      
      bnd[0] = 0.0;
      bnd.push_back(real::negative_infinity);
      TS_ASSERT_EQUALS(app->linear_constraint_lower_bounds, bnd);
      bnd[0] = real::positive_infinity;
      bnd[1] = real::positive_infinity;
      TS_ASSERT_EQUALS(app->linear_constraint_upper_bounds, bnd);

      TS_ASSERT_EQUALS(app->linear_constraint_labels, lbl);


      app->num_linear_constraints = 0;

      TS_ASSERT_EQUALS(app->num_constraints, 0u);
      TS_ASSERT_EQUALS(app->num_linear_constraints, 0u);
      TS_ASSERT_EQUALS(app->numEqConstraints(), 0u);
      TS_ASSERT_EQUALS(app->numIneqConstraints(), 0u);
      TS_ASSERT_EQUALS(app->numLinearEqConstraints(), 0u);
      TS_ASSERT_EQUALS(app->numLinearIneqConstraints(), 0u);
      
      bnd.clear();
      TS_ASSERT_EQUALS(app->linear_constraint_lower_bounds, bnd);
      TS_ASSERT_EQUALS(app->linear_constraint_upper_bounds, bnd);

      lbl.clear();
      TS_ASSERT_EQUALS(app->linear_constraint_labels, lbl);
   }

   void test_cb_print()
   {
      std::ostringstream os;
      app->colin::Application_LinearConstraints::cb_print(os);

      std::string reference = 
"Linear constraints:       3\n"
"  Equality:               1\n"
"  Inequality:             2\n"
"Index               Label    Lower Bound    Upper Bound\n"
"    1               lc[1]              0       Infinity\n"
"    2              _none_             15             15\n"
"    3               lc[3]      -Infinity            1.1\n";
      TS_ASSERT_EQUALS(os.str(), reference);

      app->num_linear_constraints = 0;
      reference.erase(reference.find("Index"));
      reference[reference.find("1")] = '0';
      reference[reference.find("2")] = '0';
      reference[reference.find("3")] = '0';
      os.str("");
      app->colin::Application_LinearConstraints::cb_print(os);
      TS_ASSERT_EQUALS(os.str(), reference);
   }

};
} // namespace colin
