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

#include <colin/reformulation/Downcast.h>

#include <cxxtest/TestSuite.h>

#include "TestApplications.h"
#include "CommonTestUtils.h"

namespace colin {
namespace unittest { class DowncastApplication; }

class colin::unittest::DowncastApplication : public CxxTest::TestSuite
{
public:
   void test_construct()
   {
      TestApplications::singleObj_denseCon<NLP1_problem> base;
      base.configure(3);

      colin::DowncastApplication<NLP0_problem> app;

      TS_ASSERT_EQUALS(app.domain_size, 0);
      TS_ASSERT_EQUALS(app.num_constraints, 0);
      
      app.reformulate_application(base);

      TS_ASSERT_EQUALS(app.domain_size, 3);
      TS_ASSERT_EQUALS(app.num_constraints, 6);      

      std::vector<double> x(3);
      x[0] = 1;
      x[1] = 2;
      x[2] = 4;

      colin::real ans;
      app.EvalF(x, ans);
      TS_ASSERT_EQUALS(ans, 8.0);

      TestApplications::singleObj_denseCon<NLP1_problem> new_base;
      new_base.configure(4);

      app.reformulate_application(new_base);

      TS_ASSERT_EQUALS(app.domain_size, 4);
      TS_ASSERT_EQUALS(app.num_constraints, 6);      

      x.push_back(2);
      app.EvalF(x, ans);
      TS_ASSERT_EQUALS(ans, 16.0);
   }

   void test_invalid_construct()
   {
      TestApplications::singleObj_denseCon<NLP1_problem> base;

      TS_ASSERT_THROWS_ASSERT
         ( colin::DowncastApplication<MINLP0_problem> app1(base), 
           std::runtime_error &e,
           TEST_WHAT(e, "DowncastApplication::"
                     "validate_reformulated_application(): The downcast "
                     "problem type, MINLP0, is not a subset of the "
                     "original problem type, NLP1" ) );

      TS_ASSERT_THROWS_ASSERT
         ( colin::DowncastApplication<NLP2_problem> app2(base), 
           std::runtime_error &e,
           TEST_WHAT(e, "DowncastApplication::"
                     "validate_reformulated_application(): The downcast "
                     "problem type, NLP2, is not a subset of the "
                     "original problem type, NLP1" ) );

      TS_ASSERT_THROWS_ASSERT
         ( colin::DowncastApplication<NLP1_problem> app3(base), 
           std::runtime_error &e,
           TEST_WHAT(e, "DowncastApplication::"
                     "validate_reformulated_application(): The downcast "
                     "problem type, NLP1, is not a subset of the "
                     "original problem type, NLP1" ) );
   }
};

} // namespace colin
