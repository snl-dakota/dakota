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

#include <colin/ApplicationMngr.h>
#include <colin/XMLProcessor.h>
#include <boost/bimap.hpp>

#include "CommonTestUtils.h"
#include "TestApplications.h"

namespace colin {
namespace unittest { class Application_RealDomain; }

class colin::unittest::Application_RealDomain : public CxxTest::TestSuite
{
public:
   typedef boost::bimap<size_t, std::string>  labels_t;

   // The test application
   TestApplications::singleObj_denseCon<NLP0_problem> *app;

   void setUp()
   {
      utilib::exception_mngr::set_mode(utilib::exception_mngr::Standard);
      app = new TestApplications::singleObj_denseCon<NLP0_problem>(3);

      labels_t labels;
      app->real_labels = labels;

      BoundTypeArray bta(3);
      app->real_lower_bound_types = bta;
      app->real_upper_bound_types = bta;
      app->enforcing_domain_bounds = false;

      app->num_linear_constraints = 0;
      app->num_nonlinear_constraints = 0;
   }

   void tearDown()
   {
      delete app;
   }
   
   void test_num_real_vars()
   {
      TS_TRACE("--------------------------------------------------------");
      TS_TRACE(" num_real_vars - Verify that we have the appropriate");
      TS_TRACE("    number of real variables.");
      TS_TRACE("--------------------------------------------------------");
      TS_ASSERT_EQUALS( app->num_real_vars, 3);
   }

   void test_bounds_feasibility1()
   {
      TS_TRACE("--------------------------------------------------------");
      TS_TRACE(" bounds_feasibility1 - Verify that simple bounds checks work");
      TS_TRACE("--------------------------------------------------------");

      utilib::BasicArray<double> point(3);
      point << 0.0;
      TS_ASSERT_EQUALS(app->testBoundsFeasibility(point),true);

      utilib::BasicArray<double> lower(3);
      lower << -1.0;
      utilib::BasicArray<double> upper(3);
      upper << 1.0;
      app->real_lower_bounds = lower;
      app->real_upper_bounds = upper;
      TS_ASSERT_EQUALS(app->testBoundsFeasibility(point),true);

      lower[1] = 1.0;
      app->real_lower_bounds = lower;
      TS_ASSERT_EQUALS(app->testBoundsFeasibility(point),false);

      lower[1] = -1.0;
      upper[1] = -1.0;
      app->real_lower_bounds = lower;
      app->real_upper_bounds = upper;
      TS_ASSERT_EQUALS(app->testBoundsFeasibility(point),false);

      app->setRealUpperBoundType(1, colin::soft_bound);
      TS_ASSERT_EQUALS(app->testBoundsFeasibility(point),true);
    }

   void test_bounds1()
   {
      TS_TRACE("--------------------------------------------------------");
      TS_TRACE(" bounds1 - Verify that setting and gettings bounds works");
      TS_TRACE("--------------------------------------------------------");

      utilib::BasicArray<double> lower(3);
      lower << -1.0;
      utilib::BasicArray<double> upper(3);
      upper << 1.0;
      app->real_lower_bounds = lower;
      app->real_upper_bounds = upper;

      utilib::BasicArray<double> _lower = app->real_lower_bounds;
      utilib::BasicArray<double> _upper = app->real_upper_bounds;
      TS_ASSERT_EQUALS(lower,_lower);
      TS_ASSERT_EQUALS(upper,_upper);
    }

   void test_bounds2()
   {
      TS_TRACE("--------------------------------------------------------");
      TS_TRACE(" bounds2 - Verify finite bounds");
      TS_TRACE("--------------------------------------------------------");

      TS_ASSERT_EQUALS( app->finite_bound_constraints(),false);
      TS_ASSERT_EQUALS( app->enforcing_domain_bounds, false);
      TS_ASSERT_EQUALS( app->hasRealLowerBound(0), false);
      TS_ASSERT_EQUALS( app->hasRealUpperBound(0), false);

      utilib::BasicArray<double> lower(3);
      lower << -1.0;
      utilib::BasicArray<double> upper(3);
      upper << 1.0;

      app->real_lower_bounds = lower;
      app->real_upper_bounds = upper;
      TS_ASSERT_EQUALS( app->finite_bound_constraints(),true);
      TS_ASSERT_EQUALS( app->enforcing_domain_bounds,true);
      TS_ASSERT_EQUALS( app->hasRealLowerBound(0), true);
      TS_ASSERT_EQUALS( app->hasRealUpperBound(0), true);
      TS_ASSERT_EQUALS( app->hasPeriodicRealBound(0), false);

      app->setRealLowerBoundType(0, soft_bound);
      TS_ASSERT_EQUALS( app->finite_bound_constraints(),true);
      TS_ASSERT_EQUALS( app->enforcing_domain_bounds,true);
      TS_ASSERT_EQUALS( app->hasRealLowerBound(0), true);
      TS_ASSERT_EQUALS( app->hasRealUpperBound(0), true);
      TS_ASSERT_EQUALS( app->hasPeriodicRealBound(0), false);

      app->setRealLowerBoundType(0, no_bound);
      TS_ASSERT_EQUALS( app->finite_bound_constraints(),false);
      TS_ASSERT_EQUALS( app->enforcing_domain_bounds,true);
      TS_ASSERT_EQUALS( app->hasRealLowerBound(0), false);
      TS_ASSERT_EQUALS( app->hasRealUpperBound(0), true);
      TS_ASSERT_EQUALS( app->hasPeriodicRealBound(0), false);

      TS_ASSERT_THROWS_ASSERT
         ( app->setRealLowerBoundType(0, hard_bound), std::logic_error e,
           TEST_WHAT(e, "Application_RealDomain::cb_validate_bound_types(): "
                     "unsetting no_bound on an infinite bound (index=0).") );

      app->setRealLowerBoundType(1, soft_bound);
      TS_ASSERT_EQUALS( app->finite_bound_constraints(),false);
      TS_ASSERT_EQUALS( app->enforcing_domain_bounds,true);
      TS_ASSERT_EQUALS( app->hasRealLowerBound(1), true);
      TS_ASSERT_EQUALS( app->hasRealUpperBound(1), true);
      TS_ASSERT_EQUALS( app->hasPeriodicRealBound(1), false);

      app->setRealLowerBoundType(1, no_bound);
      TS_ASSERT_EQUALS( app->finite_bound_constraints(),false);
      TS_ASSERT_EQUALS( app->enforcing_domain_bounds,true);
      TS_ASSERT_EQUALS( app->hasRealLowerBound(0), false);
      TS_ASSERT_EQUALS( app->hasRealUpperBound(0), true);
      TS_ASSERT_EQUALS( app->hasPeriodicRealBound(0), false);
      TS_ASSERT_EQUALS( app->hasRealLowerBound(1), false);
      TS_ASSERT_EQUALS( app->hasRealUpperBound(1), true);
      TS_ASSERT_EQUALS( app->hasPeriodicRealBound(1), false);

      lower = app->real_lower_bounds;
      lower[0] = -1;
      app->real_lower_bounds = lower;
      TS_ASSERT_EQUALS( app->finite_bound_constraints(),false);
      TS_ASSERT_EQUALS( app->enforcing_domain_bounds,true);
      TS_ASSERT_EQUALS( app->hasRealLowerBound(0), true);
      TS_ASSERT_EQUALS( app->hasRealUpperBound(0), true);
      TS_ASSERT_EQUALS( app->hasPeriodicRealBound(0), false);

      lower[1] = -1;
      app->real_lower_bounds = lower;
      TS_ASSERT_EQUALS( app->finite_bound_constraints(),true);
      TS_ASSERT_EQUALS( app->enforcing_domain_bounds,true);
      TS_ASSERT_EQUALS( app->hasRealLowerBound(0), true);
      TS_ASSERT_EQUALS( app->hasRealUpperBound(0), true);
      TS_ASSERT_EQUALS( app->hasPeriodicRealBound(0), false);

      app->setRealUpperBoundType(0, no_bound);
      TS_ASSERT_EQUALS( app->finite_bound_constraints(),false);
      TS_ASSERT_EQUALS( app->enforcing_domain_bounds,true);
      TS_ASSERT_EQUALS( app->hasRealLowerBound(0), true);
      TS_ASSERT_EQUALS( app->hasRealUpperBound(0), false);
      TS_ASSERT_EQUALS( app->hasPeriodicRealBound(0), false);

      TS_ASSERT_THROWS_ASSERT
         ( app->setPeriodicRealBound(0), std::logic_error e,
           TEST_WHAT(e, "Application_RealDomain::cb_validate_bound_types(): "
                     "unsetting no_bound on an infinite bound (index=0).") );

      app->real_upper_bounds = upper;
      app->setPeriodicRealBound(0);
      TS_ASSERT_EQUALS( app->finite_bound_constraints(),true);
      TS_ASSERT_EQUALS( app->enforcing_domain_bounds,true);
      TS_ASSERT_EQUALS( app->hasRealLowerBound(0), true);
      TS_ASSERT_EQUALS( app->hasRealUpperBound(0), true);
      TS_ASSERT_EQUALS( app->hasPeriodicRealBound(0), true);

      app->enforcing_domain_bounds = false;
      TS_ASSERT_EQUALS( app->finite_bound_constraints(),false);
      TS_ASSERT_EQUALS( app->enforcing_domain_bounds,false);
      TS_ASSERT_EQUALS( app->hasRealLowerBound(0), false);
      TS_ASSERT_EQUALS( app->hasRealUpperBound(0), false);
      TS_ASSERT_EQUALS( app->hasPeriodicRealBound(0), false);

      app->enforcing_domain_bounds = true;

      // changing the upper bound should not clear the periodic flag
      upper[0] = 100;
      app->real_upper_bounds = upper;
      TS_ASSERT_EQUALS( app->finite_bound_constraints(),true);
      TS_ASSERT_EQUALS( app->enforcing_domain_bounds,true);
      TS_ASSERT_EQUALS( app->hasRealLowerBound(0), true);
      TS_ASSERT_EQUALS( app->hasRealUpperBound(0), true);
      TS_ASSERT_EQUALS( app->hasPeriodicRealBound(0), true);

      // clearing the upper bound *should* clear the periodic flag
      upper[0] = MAXDOUBLE;
      app->real_upper_bounds = upper;
      TS_ASSERT_EQUALS( app->finite_bound_constraints(),false);
      TS_ASSERT_EQUALS( app->enforcing_domain_bounds,true);
      TS_ASSERT_EQUALS( app->hasRealLowerBound(0), true);
      TS_ASSERT_EQUALS( app->hasRealUpperBound(0), false);
      TS_ASSERT_EQUALS( app->hasPeriodicRealBound(0), false);
    }

   void test_bounds3()
   {
      TS_TRACE("--------------------------------------------------------");
      TS_TRACE(" bounds3 - Verify bound types");
      TS_TRACE("--------------------------------------------------------");

      BoundTypeArray lower_bt = app->real_lower_bound_types;
      BoundTypeArray upper_bt = app->real_upper_bound_types;
      TS_ASSERT_EQUALS( lower_bt.size(), 3u);
      TS_ASSERT_EQUALS( upper_bt.size(), 3u);
      TS_ASSERT_EQUALS( upper_bt[0], colin::no_bound);
      TS_ASSERT_EQUALS( app->realLowerBoundType(0), colin::no_bound);
      TS_ASSERT_EQUALS( app->realUpperBoundType(0), colin::no_bound);

      utilib::BasicArray<double> lower(3);
      lower << -1.0;
      utilib::BasicArray<double> upper(3);
      upper << 1.0;
      app->real_lower_bounds = lower;
      app->real_upper_bounds = upper;
      lower_bt = app->real_lower_bound_types;
      upper_bt = app->real_upper_bound_types;
      TS_ASSERT_EQUALS( upper_bt[0], colin::hard_bound);
      TS_ASSERT_EQUALS( app->realLowerBoundType(0), colin::hard_bound);
      TS_ASSERT_EQUALS( app->realUpperBoundType(0), colin::hard_bound);

      lower[1] = -MAXDOUBLE;
      app->real_lower_bounds = lower;
      lower_bt = app->real_lower_bound_types;
      upper_bt = app->real_upper_bound_types;
      TS_ASSERT_EQUALS( lower_bt[0], colin::hard_bound);
      TS_ASSERT_EQUALS( lower_bt[1], colin::no_bound);
      TS_ASSERT_EQUALS( app->realLowerBoundType(1), colin::no_bound);
      TS_ASSERT_EQUALS( app->realUpperBoundType(0), colin::hard_bound);
   }

   void test_bounds4()
   {
      TS_TRACE("--------------------------------------------------------");
      TS_TRACE(" bounds4 - Verify bound types (some more)");
      TS_TRACE("--------------------------------------------------------");

      BoundTypeArray lower_bt = app->real_lower_bound_types;
      BoundTypeArray upper_bt = app->real_upper_bound_types;
      TS_ASSERT_EQUALS( lower_bt.size(), 3u);
      TS_ASSERT_EQUALS( upper_bt.size(), 3u);
      TS_ASSERT_EQUALS( upper_bt[0], colin::no_bound);
      TS_ASSERT_EQUALS( app->realLowerBoundType(0), colin::no_bound);
      TS_ASSERT_EQUALS( app->realUpperBoundType(0), colin::no_bound);

      utilib::BasicArray<double> lower(3);
      lower << -1.0;
      utilib::BasicArray<double> upper(3);
      upper << 1.0;
      app->real_lower_bounds = lower;
      app->real_upper_bounds = upper;
      lower_bt = app->real_lower_bound_types;
      upper_bt = app->real_upper_bound_types;
      TS_ASSERT_EQUALS( upper_bt[0], colin::hard_bound);
      TS_ASSERT_EQUALS( app->realLowerBoundType(0), colin::hard_bound);
      TS_ASSERT_EQUALS( app->realUpperBoundType(0), colin::hard_bound);

      lower_bt[1] = colin::soft_bound;
      upper_bt[0] = colin::periodic_bound;
      app->real_lower_bound_types = lower_bt;
      app->real_upper_bound_types = upper_bt;
      TS_ASSERT_EQUALS( app->realLowerBoundType(1), colin::soft_bound);
      TS_ASSERT_EQUALS( app->realUpperBoundType(0), colin::periodic_bound);
   }

   void test_bounds5()
   {
      TS_TRACE("--------------------------------------------------------");
      TS_TRACE(" bounds5 - Verify error checking for bounds methods");
      TS_TRACE("--------------------------------------------------------");

      try {
        app->realLowerBoundType(4);
        TS_FAIL("Expected runtime error");
        }
      catch (std::runtime_error&) {}

      try {
        app->realUpperBoundType(4);
        TS_FAIL("Expected runtime error");
        }
      catch (std::runtime_error&) {}

      try {
        app->hasRealLowerBound(4);
        TS_FAIL("Expected runtime error");
        }
      catch (std::runtime_error&) {}

      try {
        app->hasRealUpperBound(4);
        TS_FAIL("Expected runtime error");
        }
      catch (std::runtime_error&) {}

      try {
        app->setRealLowerBoundType(4, colin::no_bound);
        TS_FAIL("Expected runtime error");
        }
      catch (std::runtime_error&) {}

      try {
        app->setRealUpperBoundType(4, colin::no_bound);
        TS_FAIL("Expected runtime error");
        }
      catch (std::runtime_error&) {}

      try {
        app->hasPeriodicRealBound(4);
        TS_FAIL("Expected runtime error");
        }
      catch (std::runtime_error&) {}

      try {
        app->setPeriodicRealBound(4);
        TS_FAIL("Expected runtime error");
        }
      catch (std::runtime_error&) {}

      utilib::BasicArray<double> lower(3);
      lower << -1.0;
      utilib::BasicArray<double> upper(2);
      upper << -1.0;
      try {
         app->real_lower_bounds = lower;
         app->real_upper_bounds = upper;
         TS_FAIL("Expected runtime error");
      }
      catch (std::runtime_error&) {}
      lower.resize(2);
      upper.resize(3);
      try {
         app->real_lower_bounds = lower;
         app->real_upper_bounds = upper;
         TS_FAIL("Expected runtime error");
      }
      catch (std::runtime_error&) {}

      BoundTypeArray lower_bt(3);
      BoundTypeArray upper_bt(2);
      try {
         app->real_lower_bound_types = lower_bt;
         app->real_upper_bound_types = upper_bt;
         TS_FAIL("Expected runtime error");
      }
      catch (std::runtime_error&) {}
      lower_bt.resize(2);
      upper_bt.resize(3);
      try {
         app->real_lower_bound_types = lower_bt;
         app->real_upper_bound_types = upper_bt;
         TS_FAIL("Expected runtime error");
      }
      catch (std::runtime_error&) {}
    }

   /*
   void xtest_fixed1()
   {
      TS_TRACE("--------------------------------------------------------");
      TS_TRACE(" fixed1 - Verify that map of fixed variables can be");
      TS_TRACE("    accessed.");
      TS_TRACE("--------------------------------------------------------");

      const std::map<size_t, double>& fixed = app->get_fixed_real_vars();
      TS_ASSERT_EQUALS( fixed.size(), 0u);
      app->fix_real_var(1,0.5);
      TS_ASSERT_EQUALS( fixed.size(), 1u);
      app->unfix_real_var(1);
      TS_ASSERT_EQUALS( fixed.size(), 0u);

      try {
        app->fix_real_var(3,0.5);
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

      TS_ASSERT_EQUALS( app->real_labels.as<labels_t>().size(), 0u);

      app->setRealLabel(0, "x0");
      TS_ASSERT_EQUALS( app->real_labels.as<labels_t>().size(), 1u);

      app->setRealLabel(0, "");
      TS_ASSERT_EQUALS( app->real_labels.as<labels_t>().size(), 0u);
    }

   void test_var_labels2()
   {
      TS_TRACE("--------------------------------------------------------");
      TS_TRACE(" var_labels2 - Verify that variable labels can be ");
      TS_TRACE("    setup with a bimap.");
      TS_TRACE("--------------------------------------------------------");

      labels_t _labels;
      _labels.left.insert(std::make_pair(0, "x0"));
      _labels.left.insert(std::make_pair(2, "x2"));
      app->real_labels = _labels;

      labels_t labels = app->real_labels;
      TS_ASSERT_EQUALS( labels.size(), 2u);

      app->real_labels = labels_t();
      labels = app->real_labels;
      TS_ASSERT_EQUALS( labels.size(), 0u);

      _labels.left.insert(std::make_pair(3, "x3"));
      try {
        app->real_labels = _labels;
        TS_FAIL("Expected runtime_error exception");
      }
      catch (std::runtime_error& err) { }

      try {
        app->setRealLabel(3,"x3");
        TS_FAIL("Expected runtime_error exception");
      }
      catch (std::runtime_error& err) { }
    }

    void test_print_summary1()
    {
    std::ofstream of("print_summary1.out");

      utilib::BasicArray<double> lower(3);
      lower << -1.0;
      utilib::BasicArray<double> upper(3);
      upper << 1.0;
      app->real_lower_bounds = lower;
      app->real_upper_bounds = upper;

      app->setRealLabel(0,"x0");
      //app->fix_real_var(1,0.5);

    app->print_summary(of);
    of.close();

    TS_ASSERT_SAME_FILES("print_summary1.out", "print_summary1.txt");
    }

    void test_parse_xml1()
    {
    TiXmlDocument doc;
    doc.LoadFile("example1a.xml");
    XMLProcessor().process(doc.RootElement());

    std::string name = ApplicationMngr().get_newest_application();
    std::ofstream of("print_summary2.out");
    ApplicationMngr().get_application(name)->print_summary(of);
    of.close();
    ApplicationMngr().unregister_application(name);
    TS_ASSERT_SAME_FILES("print_summary2.out", "print_summary2.txt");
    }

    void test_parse_xml2()
    {
    TiXmlDocument doc;
    doc.LoadFile("example1b.xml");
    try {
        XMLProcessor().process(doc.RootElement());
        TS_FAIL("Expected runtime_error");
    }
    catch (std::runtime_error&) {}
    }

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
      colin::ConfigurableApplication<colin::NLP0_problem>* app
         = colin::new_application(foo, &fn1);
      app->num_real_vars = 3;
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
