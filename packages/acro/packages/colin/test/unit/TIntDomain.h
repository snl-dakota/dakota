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
#include <colin/real.h>
#include <colin/XMLProcessor.h>
#include <colin/ApplicationMngr.h>
#include <boost/bimap.hpp>

#include <cxxtest/TestSuite.h>

#include "CommonTestUtils.h"
#include "TestApplications.h"

namespace {

inline void fn1(const std::vector<int>& x, colin::real& ans)
{
   ans = 1.0;
   for (unsigned int i = 0; i < x.size(); i++)
   { ans *= x[i]; }
}

} // namespace (local)

namespace colin {
namespace unittest { class Application_IntDomain; }

class colin::unittest::Application_IntDomain : public CxxTest::TestSuite
{
public:
   typedef boost::bimap<size_t, std::string>  labels_t;

   TestApplications::mi_singleObj_denseCon<MINLP0_problem> *app;

   void setUp()
   {
      utilib::exception_mngr::set_mode(utilib::exception_mngr::Standard);
      app = new TestApplications::mi_singleObj_denseCon<MINLP0_problem>(0);

      app->num_int_vars = 3;
      app->num_binary_vars = 4;
      app->num_linear_constraints = 0;
      app->num_nonlinear_constraints = 0;
   }

   void tearDown()
   {
      delete app;
   }

   void test_num_int_vars()
   {
      TS_TRACE("--------------------------------------------------------");
      TS_TRACE(" num_int_vars - Verify that we have the appropriate");
      TS_TRACE("    number of integer variables.");
      TS_TRACE("--------------------------------------------------------");
      TS_ASSERT_EQUALS(app->num_int_vars, 3);
      TS_ASSERT_EQUALS(app->num_binary_vars, 4);
   }

   void test_bounds_feasibility1()
   {
      TS_TRACE("--------------------------------------------------------");
      TS_TRACE(" bounds_feasibility1 - Verify that simple bounds checks work");
      TS_TRACE("--------------------------------------------------------");

      utilib::BasicArray<int> point(3);
      point << 0;
      TS_ASSERT_EQUALS(app->testBoundsFeasibility(point), true);

      utilib::BasicArray<int> lower(3);
      lower << -1;
      utilib::BasicArray<int> upper(3);
      upper << 1;
      app->int_lower_bounds = lower;
      app->int_upper_bounds = upper;
      TS_ASSERT_EQUALS(app->testBoundsFeasibility(point), true);

      lower[1] = 1;
      app->int_lower_bounds = lower;
      TS_ASSERT_EQUALS(app->testBoundsFeasibility(point), false);

      lower[1] = -1;
      upper[1] = -1;
      app->int_lower_bounds = lower;
      app->int_upper_bounds = upper;
      TS_ASSERT_EQUALS(app->testBoundsFeasibility(point), false);

      app->setIntUpperBoundType(1, colin::soft_bound);
      TS_ASSERT_EQUALS(app->testBoundsFeasibility(point), true);
   }

   void test_bounds1()
   {
      TS_TRACE("--------------------------------------------------------");
      TS_TRACE(" bounds1 - Verify that setting and gettings bounds works");
      TS_TRACE("--------------------------------------------------------");

      utilib::BasicArray<int> lower(3);
      lower << -1;
      utilib::BasicArray<int> upper(3);
      upper << 1;
      app->int_lower_bounds = lower;
      app->int_upper_bounds = upper;

      utilib::BasicArray<int> _lower = app->int_lower_bounds;
      utilib::BasicArray<int> _upper = app->int_upper_bounds;
      TS_ASSERT_EQUALS(lower, _lower);
      TS_ASSERT_EQUALS(upper, _upper);
   }

   void test_bounds2()
   {
      TS_TRACE("--------------------------------------------------------");
      TS_TRACE(" bounds2 - Verify finite bounds");
      TS_TRACE("--------------------------------------------------------");

      TS_ASSERT_EQUALS(app->finite_bound_constraints(), false);
      TS_ASSERT_EQUALS(app->enforcing_domain_bounds, false);
      TS_ASSERT_EQUALS(app->hasIntLowerBound(0), false);
      TS_ASSERT_EQUALS(app->hasIntUpperBound(0), false);

      utilib::BasicArray<int> lower(3);
      lower << -1;
      utilib::BasicArray<int> upper(3);
      upper << 1;

      app->int_lower_bounds = lower;
      app->int_upper_bounds = upper;
      TS_ASSERT_EQUALS(app->finite_bound_constraints(), true);
      TS_ASSERT_EQUALS(app->enforcing_domain_bounds, true);
      TS_ASSERT_EQUALS(app->hasIntLowerBound(0), true);
      TS_ASSERT_EQUALS(app->hasIntUpperBound(0), true);
      TS_ASSERT_EQUALS(app->hasPeriodicIntBound(0), false);

      app->setIntLowerBoundType(0, soft_bound);
      TS_ASSERT_EQUALS(app->finite_bound_constraints(), true);
      TS_ASSERT_EQUALS(app->enforcing_domain_bounds, true);
      TS_ASSERT_EQUALS(app->hasIntLowerBound(0), true);
      TS_ASSERT_EQUALS(app->hasIntUpperBound(0), true);
      TS_ASSERT_EQUALS(app->hasPeriodicIntBound(0), false);

      app->setIntLowerBoundType(0, no_bound);
      TS_ASSERT_EQUALS(app->finite_bound_constraints(), false);
      TS_ASSERT_EQUALS(app->enforcing_domain_bounds, true);
      TS_ASSERT_EQUALS(app->hasIntLowerBound(0), false);
      TS_ASSERT_EQUALS(app->hasIntUpperBound(0), true);
      TS_ASSERT_EQUALS(app->hasPeriodicIntBound(0), false);

      TS_ASSERT_THROWS_ASSERT
         ( app->setIntLowerBoundType(0, hard_bound), std::logic_error e,
           TEST_WHAT(e, "Application_IntDomain::cb_validate_bound_types(): "
                     "unsetting no_bound on an infinite bound (index=0).") );

      app->setIntLowerBoundType(1, soft_bound);
      TS_ASSERT_EQUALS( app->finite_bound_constraints(),false);
      TS_ASSERT_EQUALS( app->enforcing_domain_bounds,true);
      TS_ASSERT_EQUALS( app->hasIntLowerBound(1), true);
      TS_ASSERT_EQUALS( app->hasIntUpperBound(1), true);
      TS_ASSERT_EQUALS( app->hasPeriodicIntBound(1), false);

      app->setIntLowerBoundType(1, no_bound);
      TS_ASSERT_EQUALS( app->finite_bound_constraints(),false);
      TS_ASSERT_EQUALS( app->enforcing_domain_bounds,true);
      TS_ASSERT_EQUALS( app->hasIntLowerBound(0), false);
      TS_ASSERT_EQUALS( app->hasIntUpperBound(0), true);
      TS_ASSERT_EQUALS( app->hasPeriodicIntBound(0), false);
      TS_ASSERT_EQUALS( app->hasIntLowerBound(1), false);
      TS_ASSERT_EQUALS( app->hasIntUpperBound(1), true);
      TS_ASSERT_EQUALS( app->hasPeriodicIntBound(1), false);

      lower = app->int_lower_bounds;
      lower[0] = -1;
      app->int_lower_bounds = lower;
      TS_ASSERT_EQUALS( app->finite_bound_constraints(),false);
      TS_ASSERT_EQUALS( app->enforcing_domain_bounds,true);
      TS_ASSERT_EQUALS( app->hasIntLowerBound(0), true);
      TS_ASSERT_EQUALS( app->hasIntUpperBound(0), true);
      TS_ASSERT_EQUALS( app->hasPeriodicIntBound(0), false);

      lower[1] = -1;
      app->int_lower_bounds = lower;
      TS_ASSERT_EQUALS( app->finite_bound_constraints(),true);
      TS_ASSERT_EQUALS( app->enforcing_domain_bounds,true);
      TS_ASSERT_EQUALS( app->hasIntLowerBound(0), true);
      TS_ASSERT_EQUALS( app->hasIntUpperBound(0), true);
      TS_ASSERT_EQUALS( app->hasPeriodicIntBound(0), false);

      app->setIntUpperBoundType(0, no_bound);
      TS_ASSERT_EQUALS(app->finite_bound_constraints(), false);
      TS_ASSERT_EQUALS(app->enforcing_domain_bounds, true);
      TS_ASSERT_EQUALS(app->hasIntLowerBound(0), true);
      TS_ASSERT_EQUALS(app->hasIntUpperBound(0), false);
      TS_ASSERT_EQUALS(app->hasPeriodicIntBound(0), false);

      TS_ASSERT_THROWS_ASSERT
         ( app->setPeriodicIntBound(0), std::logic_error e,
           TEST_WHAT(e, "Application_IntDomain::cb_validate_bound_types(): "
                     "unsetting no_bound on an infinite bound (index=0).") );

      app->int_upper_bounds = upper;
      app->setPeriodicIntBound(0);
      TS_ASSERT_EQUALS(app->finite_bound_constraints(), true);
      TS_ASSERT_EQUALS(app->enforcing_domain_bounds, true);
      TS_ASSERT_EQUALS(app->hasIntLowerBound(0), true);
      TS_ASSERT_EQUALS(app->hasIntUpperBound(0), true);
      TS_ASSERT_EQUALS(app->hasPeriodicIntBound(0), true);

      app->enforcing_domain_bounds = false;
      TS_ASSERT_EQUALS(app->finite_bound_constraints(), false);
      TS_ASSERT_EQUALS(app->enforcing_domain_bounds, false);
      TS_ASSERT_EQUALS(app->hasIntLowerBound(0), false);
      TS_ASSERT_EQUALS(app->hasIntUpperBound(0), false);
      TS_ASSERT_EQUALS(app->hasPeriodicIntBound(0), false);

      app->enforcing_domain_bounds = true;

      // changing the upper bound should not clear the periodic flag
      upper[0] = 100;
      app->int_upper_bounds = upper;
      TS_ASSERT_EQUALS(app->finite_bound_constraints(), true);
      TS_ASSERT_EQUALS(app->enforcing_domain_bounds, true);
      TS_ASSERT_EQUALS(app->hasIntLowerBound(0), true);
      TS_ASSERT_EQUALS(app->hasIntUpperBound(0), true);
      TS_ASSERT_EQUALS(app->hasPeriodicIntBound(0), true);

      // clearing the upper bound *should* clear the periodic flag
      upper[0] = MAXINT;
      app->int_upper_bounds = upper;
      TS_ASSERT_EQUALS(app->finite_bound_constraints(), false);
      TS_ASSERT_EQUALS(app->enforcing_domain_bounds, true);
      TS_ASSERT_EQUALS(app->hasIntLowerBound(0), true);
      TS_ASSERT_EQUALS(app->hasIntUpperBound(0), false);
      TS_ASSERT_EQUALS(app->hasPeriodicIntBound(0), false);
   }

   void test_bounds3()
   {
      TS_TRACE("--------------------------------------------------------");
      TS_TRACE(" bounds3 - Verify bound types");
      TS_TRACE("--------------------------------------------------------");

      BoundTypeArray lower_bt = app->int_lower_bound_types;
      BoundTypeArray upper_bt = app->int_upper_bound_types;
      TS_ASSERT_EQUALS(lower_bt.size(), 3u);
      TS_ASSERT_EQUALS(upper_bt.size(), 3u);
      TS_ASSERT_EQUALS(upper_bt[0], colin::no_bound);
      TS_ASSERT_EQUALS(app->intLowerBoundType(0), colin::no_bound);
      TS_ASSERT_EQUALS(app->intUpperBoundType(0), colin::no_bound);

      utilib::BasicArray<int> lower(3);
      lower << -1;
      utilib::BasicArray<int> upper(3);
      upper << 1;
      app->int_lower_bounds = lower;
      app->int_upper_bounds = upper;
      lower_bt = app->int_lower_bound_types;
      upper_bt = app->int_upper_bound_types;
      TS_ASSERT_EQUALS(upper_bt[0], colin::hard_bound);
      TS_ASSERT_EQUALS(app->intLowerBoundType(0), colin::hard_bound);
      TS_ASSERT_EQUALS(app->intUpperBoundType(0), colin::hard_bound);

      lower[1] = INT_MIN;
      app->int_lower_bounds = lower;
      lower_bt = app->int_lower_bound_types;
      upper_bt = app->int_upper_bound_types;
      TS_ASSERT_EQUALS(lower_bt[0], colin::hard_bound);
      TS_ASSERT_EQUALS(lower_bt[1], colin::no_bound);
      TS_ASSERT_EQUALS(app->intLowerBoundType(1), colin::no_bound);
      TS_ASSERT_EQUALS(app->intUpperBoundType(0), colin::hard_bound);
   }

   void test_bounds4()
   {
      TS_TRACE("--------------------------------------------------------");
      TS_TRACE(" bounds4 - Verify bound types (some more)");
      TS_TRACE("--------------------------------------------------------");

      BoundTypeArray lower_bt = app->int_lower_bound_types;
      BoundTypeArray upper_bt = app->int_upper_bound_types;
      TS_ASSERT_EQUALS(lower_bt.size(), 3u);
      TS_ASSERT_EQUALS(upper_bt.size(), 3u);
      TS_ASSERT_EQUALS(upper_bt[0], colin::no_bound);
      TS_ASSERT_EQUALS(app->intLowerBoundType(0), colin::no_bound);
      TS_ASSERT_EQUALS(app->intUpperBoundType(0), colin::no_bound);

      utilib::BasicArray<int> lower(3);
      lower << -1;
      utilib::BasicArray<int> upper(3);
      upper << 1;
      app->int_lower_bounds = lower;
      app->int_upper_bounds = upper;
      lower_bt = app->int_lower_bound_types;
      upper_bt = app->int_upper_bound_types;
      TS_ASSERT_EQUALS(upper_bt[0], colin::hard_bound);
      TS_ASSERT_EQUALS(app->intLowerBoundType(0), colin::hard_bound);
      TS_ASSERT_EQUALS(app->intUpperBoundType(0), colin::hard_bound);

      lower_bt[1] = colin::soft_bound;
      upper_bt[0] = colin::periodic_bound;
      app->int_lower_bound_types = lower_bt;
      app->int_upper_bound_types = upper_bt;
      TS_ASSERT_EQUALS(app->intLowerBoundType(1), colin::soft_bound);
      TS_ASSERT_EQUALS(app->intUpperBoundType(0), colin::periodic_bound);
   }

   void test_bounds5()
   {
      TS_TRACE("--------------------------------------------------------");
      TS_TRACE(" bounds5 - Verify error checking for bounds methods");
      TS_TRACE("--------------------------------------------------------");

      try
      {
         app->intLowerBoundType(4);
         TS_FAIL("Expected runtime error");
      }
      catch (std::runtime_error&) {}

      try
      {
         app->intUpperBoundType(4);
         TS_FAIL("Expected runtime error");
      }
      catch (std::runtime_error&) {}

      try
      {
         app->hasIntLowerBound(4);
         TS_FAIL("Expected runtime error");
      }
      catch (std::runtime_error&) {}

      try
      {
         app->hasIntUpperBound(4);
         TS_FAIL("Expected runtime error");
      }
      catch (std::runtime_error&) {}

      try
      {
         app->setIntLowerBoundType(4, colin::no_bound);
         TS_FAIL("Expected runtime error");
      }
      catch (std::runtime_error&) {}

      try
      {
         app->setIntUpperBoundType(4, colin::no_bound);
         TS_FAIL("Expected runtime error");
      }
      catch (std::runtime_error&) {}

      try
      {
         app->hasPeriodicIntBound(4);
         TS_FAIL("Expected runtime error");
      }
      catch (std::runtime_error&) {}

      try
      {
         app->setPeriodicIntBound(4);
         TS_FAIL("Expected runtime error");
      }
      catch (std::runtime_error&) {}

      utilib::BasicArray<int> lower(3);
      lower << -1;
      utilib::BasicArray<int> upper(2);
      upper << -1;
      try
      {
         app->int_lower_bounds = lower;
         app->int_upper_bounds = upper;
         TS_FAIL("Expected runtime error");
      }
      catch (std::runtime_error&) {}
      lower.resize(2);
      upper.resize(3);
      try
      {
         app->int_lower_bounds = lower;
         app->int_upper_bounds = upper;
         TS_FAIL("Expected runtime error");
      }
      catch (std::runtime_error&) {}

      BoundTypeArray lower_bt(3);
      BoundTypeArray upper_bt(2);
      try
      {
         app->int_lower_bound_types = lower_bt;
         app->int_upper_bound_types = upper_bt;
         TS_FAIL("Expected runtime error");
      }
      catch (std::runtime_error&) {}
      lower_bt.resize(2);
      upper_bt.resize(3);
      try
      {
         app->int_lower_bound_types = lower_bt;
         app->int_upper_bound_types = upper_bt;
         TS_FAIL("Expected runtime error");
      }
      catch (std::runtime_error&) {}}

   /*
   void x_test_fixed1()
   {
      TS_TRACE("--------------------------------------------------------");
      TS_TRACE(" fixed1 - Verify that map of fixed variables can be");
      TS_TRACE("    accessed.");
      TS_TRACE("--------------------------------------------------------");

      const std::map<size_t, int>& ifixed = app->get_fixed_int_vars();
      TS_ASSERT_EQUALS(ifixed.size(), 0u);
      app->fix_int_var(1, -1);
      TS_ASSERT_EQUALS(ifixed.size(), 1u);
      app->unfix_int_var(1);
      TS_ASSERT_EQUALS(ifixed.size(), 0u);

      const std::map<size_t, bool>& bfixed = app->get_fixed_binary_vars();
      TS_ASSERT_EQUALS(bfixed.size(), 0u);
      app->fix_binary_var(2, false);
      TS_ASSERT_EQUALS(bfixed.size(), 1u);
      app->unfix_binary_var(2);
      TS_ASSERT_EQUALS(bfixed.size(), 0u);

      try
      {
         app->fix_int_var(3, -1);
         TS_FAIL("expected runtime errro");
      }
      catch (std::runtime_error&) {}

      try
      {
         app->unfix_int_var(1);
         TS_FAIL("expected runtime errro");
      }
      catch (std::runtime_error&) {}

      try
      {
         app->fix_binary_var(4, false);
         TS_FAIL("expected runtime errro");
      }
      catch (std::runtime_error&) {}

      try
      {
         app->unfix_binary_var(2);
         TS_FAIL("expected runtime errro");
      }
      catch (std::runtime_error&) {}

      }
   */

   void test_var_labels1()
   {
      TS_TRACE("--------------------------------------------------------");
      TS_TRACE(" var_labels1 - Verify that variable labels can be ");
      TS_TRACE("    accessed and modified.");
      TS_TRACE("--------------------------------------------------------");

      TS_ASSERT_EQUALS(app->int_labels.as<labels_t>().size(), 0u);
      TS_ASSERT_EQUALS(app->binary_labels.as<labels_t>().size(), 0u);

      app->setIntLabel(0, "x0");
      TS_ASSERT_EQUALS(app->int_labels.as<labels_t>().size(), 1u);

      app->setIntLabel(0, "");
      TS_ASSERT_EQUALS(app->int_labels.as<labels_t>().size(), 0u);
   
      app->setBinaryLabel(0, "x0");
      TS_ASSERT_EQUALS(app->binary_labels.as<labels_t>().size(), 1u);

      app->setBinaryLabel(0, "");
      TS_ASSERT_EQUALS(app->binary_labels.as<labels_t>().size(), 0u);
   }

   void test_int_labels()
   {
      TS_TRACE("--------------------------------------------------------");
      TS_TRACE(" int_labels - Verify that variable labels can be ");
      TS_TRACE("    setup with a bimap.");
      TS_TRACE("--------------------------------------------------------");

      labels_t _labels;
      _labels.left.insert(std::make_pair(0, "x0"));
      _labels.left.insert(std::make_pair(2, "x2"));
      app->int_labels = _labels;

      labels_t labels = app->int_labels;
      TS_ASSERT_EQUALS(labels.size(), 2u);

      app->int_labels = labels_t();
      labels = app->int_labels;
      TS_ASSERT_EQUALS(labels.size(), 0u);

      _labels.left.insert(std::make_pair(3, "x3"));
      try
      {
         app->int_labels = _labels;
         TS_FAIL("Expected runtime_error exception");
      }
      catch (std::runtime_error& err) { }

      try
      {
         app->setIntLabel(3, "x3");
         TS_FAIL("Expected runtime_error exception");
      }
      catch (std::runtime_error& err) { }

    }

   void test_binary_labels()
   {
      TS_TRACE("--------------------------------------------------------");
      TS_TRACE(" binary_labels - Verify that variable labels can be ");
      TS_TRACE("    setup with a bimap.");
      TS_TRACE("--------------------------------------------------------");

      labels_t _labels;
      _labels.left.insert(std::make_pair(0, "x0"));
      _labels.left.insert(std::make_pair(2, "x2"));
      app->binary_labels = _labels;

      labels_t labels = app->binary_labels;
      TS_ASSERT_EQUALS(labels.size(), 2u);

      app->binary_labels = labels_t();
      labels = app->binary_labels;
      TS_ASSERT_EQUALS(labels.size(), 0u);

      _labels.left.insert(std::make_pair(4, "x4"));
      try
      {
         app->binary_labels = _labels;
         TS_FAIL("Expected runtime_error exception");
      }
      catch (std::runtime_error& err) { }

      try
      {
         app->setBinaryLabel(4, "x4");
         TS_FAIL("Expected runtime_error exception");
      }
      catch (std::runtime_error& err) { }

    }

   void test_print_summary1()
   {
      std::ofstream of("print_summary3.out");

      utilib::BasicArray<int> lower(3);
      lower << -1;
      utilib::BasicArray<int> upper(3);
      upper << 1;
      app->int_lower_bounds = lower;
      app->int_upper_bounds = upper;

      app->setIntLabel(0, "x0");
      //app->fix_int_var(1, 0);
      app->setBinaryLabel(2, "b2");
      //app->fix_binary_var(2, 0);

      app->print_summary(of);
      of.close();

      TS_ASSERT_SAME_FILES("print_summary3.out", "print_summary3.txt");
   }

   void test_relaxed1()
   {
      TS_TRACE("--------------------------------------------------------");
      TS_TRACE(" relaxed1 - Verify that the relaxed application is empty");
      TS_TRACE("--------------------------------------------------------");
      TS_ASSERT_EQUALS(app->relaxed_application().empty(),true)
   }

   void test_parse_xml1()
   {
      TS_TRACE("--------------------------------------------------------");
      TS_TRACE(" parse_xml1 - Verify that a ColinInput XML block can");
      TS_TRACE("    be parsed.");
      TS_TRACE("--------------------------------------------------------");

      TiXmlDocument doc;
      doc.LoadFile("example2a.xml");
      XMLProcessor().process(doc.RootElement());

      std::string name = ApplicationMngr().get_newest_application();
      std::ofstream of("print_summary4.out");
      ApplicationMngr().get_application(name)->print_summary(of);
      of.close();
      ApplicationMngr().unregister_application(name);
      TS_ASSERT_SAME_FILES("print_summary4.out", "print_summary4.txt");
   }

   void test_parse_xml2()
   {
      TiXmlDocument doc;
      doc.LoadFile("example2b.xml");
      try
      {
         XMLProcessor().process(doc.RootElement());
         TS_FAIL("Expected runtime_error");
      }
      catch (std::runtime_error&) {}}

   void test_parse_xml3()
   {
      TiXmlDocument doc;
      doc.LoadFile("example2c.xml");
      try
      {
         XMLProcessor().process(doc.RootElement());
         TS_FAIL("Expected runtime_error");
      }
      catch (std::runtime_error&) {}}

};


#if 0
class Test_NLP0Problem_Misc : public CxxTest::TestSuite
{
public:

   void Xtest_test1a()
   {
      TS_TRACE("------------------------------------------------");
      TS_TRACE("Test1a - simple application setup of NLP0Problem");
      TS_TRACE("------------------------------------------------");
      colin::Problem<colin::NLP0_problem> prob;
      TS_TRACE("Problem set up");

      std::string foo = "example1";
      colin::ConfigurableApplication<colin::NLP0_problem>* app = colin::new_application(foo, &fn1);
      app->num_int_vars = 3;
      utilib::pvector<double> lower(3);
      utilib::pvector<double> upper(3);
      app->real_lower_bounds = lower;
      app->real_upper_bounds = upper;
      TS_TRACE("Application set up");

      prob.set_application(app);
      TS_TRACE("Application assigned to problem");

      std::vector<double> vec(3);
      vec[0] = 1;
      vec[1] = 2;
      vec[2] = 4;

      colin::real ans;
      prob->EvalF(prob->eval_mngr(), vec, ans);
      TS_ASSERT_EQUALS(ans, 8.0);
      colin::AppResponse response;
      colin::AppRequest request = prob->set_domain(vec);
      ans = 0.0;
      prob->Request_F(request, ans);
      response = prob->eval_mngr().perform_evaluation(request);
      TS_ASSERT_EQUALS(ans, 8.0);
      TS_ASSERT_EQUALS(prob->eval_mngr(), 1);

      TS_TRACE("Deleting application");
      ApplicationMngr().unregister_application(foo);
      delete app;
      TS_TRACE("Done.");

      ApplicationMngr().clear();
   }

};
#endif

} // namespace colin
