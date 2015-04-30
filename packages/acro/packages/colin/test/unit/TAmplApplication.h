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

#include <colin/AmplApplication.h>
#include <cxxtest/TestSuite.h>

namespace colin {
namespace unittest { class AmplApplication; }

class colin::unittest::AmplApplication : public CxxTest::TestSuite
{
public:

   void test_ulp_1()
   {
      std::string fname = "../problems/ulp_1";
      colin::AmplApplication app;
      app.set_nl_file(fname);
      std::cout << "Read in " << fname << std::endl;
      app.print_summary(std::cout);

      TS_ASSERT_EQUALS(app.num_objectives,  1);
      TS_ASSERT_EQUALS(app.num_constraints, 0);
      TS_ASSERT_EQUALS(app.num_real_vars,   1);
      TS_ASSERT_EQUALS(app.num_int_vars,    0);
      TS_ASSERT_EQUALS(app.num_binary_vars, 0);

      TS_ASSERT_EQUALS(app.realLabel(0), "x[1]");

      TS_ASSERT_EQUALS(app.real_lower_bounds, 1);
      TS_ASSERT_EQUALS(app.real_upper_bounds, 2);

      double ans = 0.;
      app.EvalMF(1.5, ans);
      TS_ASSERT_DELTA(ans, 1.5, 1e-8);
   }


   void test_umip_1()
   {
      std::string fname = "../problems/umip_1.nl";
      colin::AmplApplication app;
      app.set_nl_file(fname);
      std::cout << "Read in " << fname << std::endl;
      app.print_summary(std::cout);

      TS_ASSERT_EQUALS(app.num_objectives,  1);
      TS_ASSERT_EQUALS(app.num_constraints, 0);
      TS_ASSERT_EQUALS(app.num_real_vars,   1);
      TS_ASSERT_EQUALS(app.num_int_vars,    1);
      TS_ASSERT_EQUALS(app.num_binary_vars, 0);

      TS_ASSERT_EQUALS(app.realLabel(0), "x[1]");
      TS_ASSERT_EQUALS(app.intLabel(0), "y[1]");

      TS_ASSERT_EQUALS(app.real_lower_bounds, 1);
      TS_ASSERT_EQUALS(app.real_upper_bounds, 2);
      TS_ASSERT_EQUALS(app.int_lower_bounds, 1);
      TS_ASSERT_EQUALS(app.int_upper_bounds, 10);

      utilib::MixedIntVars mi(0,1,1);
      mi.Real() << 1.6;
      mi.Integer() << 5;
      double ans = 0.;
      app.EvalMF(mi, ans);
      TS_ASSERT_DELTA(ans, 6.6, 1e-8);
   }

   void test_minlp_1()
   {
      std::string fname = "../problems/minlp_1";
      colin::AmplApplication app;
      utilib::Any init_pt = app.set_nl_file(fname);
      std::cout << "Read in " << fname << std::endl;
      app.print_summary(std::cout);

      TS_ASSERT_EQUALS(app.num_objectives,  1);
      TS_ASSERT_EQUALS(app.num_real_vars,   2);
      TS_ASSERT_EQUALS(app.num_int_vars,    2);
      TS_ASSERT_EQUALS(app.num_binary_vars, 1);
      TS_ASSERT_EQUALS(app.num_constraints, 3);
      TS_ASSERT_EQUALS(app.num_nonlinear_constraints, 1);
      TS_ASSERT_EQUALS(app.num_linear_constraints, 2);
      TS_ASSERT_EQUALS(app.num_nonlinear_constraints, 1);
      TS_ASSERT_EQUALS(app.numLinearEqConstraints(), 1);

      TS_ASSERT_EQUALS(app.realLabel(0), "x1");
      TS_ASSERT_EQUALS(app.realLabel(1), "x2");
      TS_ASSERT_EQUALS(app.intLabel(0), "y1");
      TS_ASSERT_EQUALS(app.intLabel(1), "y3");
      TS_ASSERT_EQUALS(app.binaryLabel(0), "y2");

      TS_ASSERT_EQUALS(app.nonlinearConstraintLabel(0), "c1");
      TS_ASSERT_EQUALS(app.linearConstraintLabel(0), "c2");
      TS_ASSERT_EQUALS(app.linearConstraintLabel(1), "c3");

      std::vector<real> x(2);
      x[0] = real::negative_infinity;
      x[1] = -10;
      TS_ASSERT_EQUALS(app.real_lower_bounds, x);
      x[0] = real::positive_infinity;
      x[1] = 10;
      TS_ASSERT_EQUALS(app.real_upper_bounds, x);
      x[0] = -20;
      x[1] = 0;
      TS_ASSERT_EQUALS(app.int_lower_bounds, x);
      x[0] = 20;
      x[1] = 2;
      TS_ASSERT_EQUALS(app.int_upper_bounds, x);

      utilib::MixedIntVars mi(1,2,2);
      mi.Real()[0] = 0;
      mi.Real()[1] = 1;
      mi.Integer()[0] = 5;
      mi.Integer()[1] = 4;
      mi.Binary()[0] = false;
      TS_ASSERT_EQUALS(init_pt, utilib::Any(mi));

      double ans = 0.;
      std::vector<double> v(2), v_ans;
      std::vector<std::vector<double> > m(2), m_ans;
      m[0].resize(2);
      m[0][1] = 1;
      m[1].resize(2);
      m[1][0] = -1;

      mi.Real() << 1.;
      mi.Integer() << 1;
      mi.Binary()[0] = true;
      app.EvalMF(mi, ans);
      TS_ASSERT_DELTA(ans, 0. + 0.5*0.5 + 2.*2. + 100 + 50.*1., 1e-8);
      app.EvalNLCF(mi, ans);
      TS_ASSERT_DELTA(ans, 1. + 1. + 1., 1e-8);
      app.EvalLEqCF(mi, ans);
      TS_ASSERT_DELTA(ans, 1. + 10., 1e-8);
      app.EvalLIneqCF(mi, ans);
      TS_ASSERT_DELTA(ans, -1. + 5.*1., 1e-8);
      v[0] = 2.*(1.-1);
      v[1] = 2.*(1.-1.5);
      app.EvalG(mi, v_ans);
      TS_ASSERT_DELTA(v_ans, v, 1e-8);
      v[0] = 2.*(1.);
      v[1] = 1.;
      app.EvalNLCG(mi, v_ans);
      TS_ASSERT_DELTA(v_ans, v, 1e-8);
      app.EvalLCG(mi, m_ans);
      TS_ASSERT_DELTA(m_ans, m, 1e-8);

      mi.Real()[0] = 2;
      mi.Real()[1] = 3;
      mi.Integer()[0] = 4;
      mi.Integer()[1] = 5;
      mi.Binary()[0] = true;
      app.EvalMF(mi, ans);
      TS_ASSERT_DELTA(ans, 1. + 1.5*1.5 + 5.*5. + 100 + 50.*5., 1e-8);
      app.EvalNLCF(mi, ans);
      TS_ASSERT_DELTA(ans, 2.*2. + 4.*4. + 3., 1e-8);
      app.EvalLEqCF(mi, ans);
      TS_ASSERT_DELTA(ans, 3. + 10., 1e-8);
      app.EvalLIneqCF(mi, ans);
      TS_ASSERT_DELTA(ans, -2. + 5.*5., 1e-8);
      v[0] = 2.*(2.-1);
      v[1] = 2.*(3.-1.5);
      app.EvalG(mi, v_ans);
      TS_ASSERT_DELTA(v_ans, v, 1e-8);
      v[0] = 2.*(2.);
      v[1] = 1.;
      app.EvalNLCG(mi, v_ans);
      TS_ASSERT_DELTA(v_ans, v, 1e-8);
      app.EvalLCG(mi, m_ans);
      TS_ASSERT_DELTA(m_ans, m, 1e-8);
   }

   void test_text_book()
   {
      std::string fname = "../problems/text_book";
      colin::AmplApplication app;
      app.set_nl_file(fname);
      std::cout << "Read in " << fname << std::endl;
      app.print_summary(std::cout);

      TS_ASSERT_EQUALS(app.num_objectives,  1);
      TS_ASSERT_EQUALS(app.num_constraints, 2);
      TS_ASSERT_EQUALS(app.num_real_vars,   2);
      TS_ASSERT_EQUALS(app.num_int_vars,    0);
      TS_ASSERT_EQUALS(app.num_binary_vars, 0);

      TS_ASSERT_EQUALS(app.realLabel(0), "x1");
      TS_ASSERT_EQUALS(app.realLabel(1), "x2");

      std::vector<double> x(2);
      x[0] = 0.5;
      x[1] = -2.9;
      TS_ASSERT_EQUALS(app.real_lower_bounds, x);

      x[0] = 5.8;
      x[1] = 2.9;
      TS_ASSERT_EQUALS(app.real_upper_bounds, x);

      x[0] = x[1] = 0.5;
      std::vector<double> ans;
      std::vector<double> ref(1);
      ref[0] = 0.125;
      app.EvalMF(x, ans);
      TS_ASSERT_DELTA(ans, ref, 1e-8);

      ref.resize(2);
      ref[0] = 0;
      ref[1] = 0;
      app.EvalCF(x, ans);
      TS_ASSERT_DELTA(ans, ref, 1e-8);

      x[0] = 1;
      x[1] = 2;

      ref.resize(1);
      ref[0] = 1;
      app.EvalMF(x, ans);
      TS_ASSERT_DELTA(ans, ref, 1e-8);

      ref.resize(2);
      ref[0] = 1. - 1.;
      ref[1] = 4. - 1./2.;
      app.EvalCF(x, ans);
      TS_ASSERT_DELTA(ans, ref, 1e-8);
   }

   void test_rosenbrock()
   {
      std::string fname = "../problems/rosenbrock";
      colin::AmplApplication app;
      app.set_nl_file(fname);
      std::cout << "Read in " << fname << std::endl;
      app.print_summary(std::cout);

      TS_ASSERT_EQUALS(app.num_objectives,  1);
      TS_ASSERT_EQUALS(app.num_constraints, 0);
      TS_ASSERT_EQUALS(app.num_real_vars,   2);
      TS_ASSERT_EQUALS(app.num_int_vars,    0);
      TS_ASSERT_EQUALS(app.num_binary_vars, 0);

      TS_ASSERT_EQUALS(app.realLabel(0), "x[1]");
      TS_ASSERT_EQUALS(app.realLabel(1), "x[2]");

      std::vector<real> x(2);
      x[0] = x[1] = real::negative_infinity;
      TS_ASSERT_EQUALS(app.real_lower_bounds, x);
      x[0] = x[1] = real::positive_infinity;
      TS_ASSERT_EQUALS(app.real_upper_bounds, x);

      double ans;
      double ref;

      x[0] = x[1] = 1.;
      ref = 0;
      app.EvalMF(x, ans);
      TS_ASSERT_DELTA(ans, ref, 1e-8);

      x[0] = 2;
      x[1] = 3;
      ref = 100+1;
      app.EvalMF(x, ans);
      TS_ASSERT_DELTA(ans, ref, 1e-8);
   }

   void test_reset_nl_file()
   {
      std::string fname = "../problems/text_book";
      colin::AmplApplication app;
      app.set_nl_file(fname);

      double ans;
      double ref;
      std::vector<real> x(2);
      x[0] = x[1] = 0.5;
      ref = 0.125;
      app.EvalMF(x, ans);
      TS_ASSERT_DELTA(ans, ref, 1e-8);

      // redirect to another NL file
      fname = "../problems/rosenbrock";
      app.set_nl_file(fname);

      x[0] = 2;
      x[1] = 3;
      ref = 100+1;
      app.EvalMF(x, ans);
      TS_ASSERT_DELTA(ans, ref, 1e-8);
   }
};

} // namespace colin
