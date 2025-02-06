/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#include "opt_tpl_test_fixture.hpp"

#include <string>
#include <map>

#include <gtest/gtest.h>

using namespace Dakota;

//----------------------------------------------------------------

TEST(opt_tpl_api_tests, test_hopspack)
{
  Dakota::LibraryEnvironment * p_env = Opt_TPL_Test_Fixture::create_default_env(Dakota::ASYNCH_PATTERN_SEARCH);
  Dakota::LibraryEnvironment & env = *p_env;

  if (env.parallel_library().mpirun_flag())
    FAIL(); // This test only works for serial builds

  // Try to get the concrte Optimizer
  //Dakota::ProblemDescDB& problem_db = env.problem_description_db();
  //IteratorList& iter_list = problem_db.iterator_list();
  //Dakota::Iterator & dak_iter = *iter_list.begin();
  //Cout << "The iterator is a : " << dak_iter.method_string() << endl;
  //dak_iter.print_results(Cout);
  //Dakota::Optimizer * optimizer = dynamic_cast<Dakota::Optimizer*>(dak_iter.iterator_rep());

  // Execute the environment
  env.execute();

  const Real max_tol = 1.e-10;

  // retrieve the final parameter values
  const Variables& vars = env.variables_results();
  EXPECT_LT(std::fabs(1. - vars.continuous_variable(0) / -1.50), max_tol/100. );
  EXPECT_LT(std::fabs(1. - vars.continuous_variable(1) /  0.75), max_tol/100. );

  // retrieve the final response values
  const Response& resp  = env.response_results();
  //resp.function_values().print(Cout);
  //resp.write(Cout);
  EXPECT_LT(std::fabs(1. - (resp.function_value(0)+1.0) / 1.0), max_tol/100. );
}

//----------------------------------------------------------------

TEST(opt_tpl_api_tests, test_optpp)
{
  Dakota::LibraryEnvironment * p_env = Opt_TPL_Test_Fixture::create_default_env(Dakota::OPTPP_PDS);
  Dakota::LibraryEnvironment & env = *p_env;

  if (env.parallel_library().mpirun_flag())
    FAIL(); // This test only works for serial builds

  // Execute the environment
  env.execute();

  // retrieve the final parameter values
  const Variables& vars = env.variables_results();
  //vars.continuous_variables().print(Cout);
  EXPECT_LT(std::fabs(1. - vars.continuous_variable(0) / -1.50), 0.6/100. /* not so good tol */ );
  EXPECT_LT(std::fabs(1. - vars.continuous_variable(1) /  0.75), 2.0/100. /* ditto */ );

  // retrieve the final response values
  const Response& resp  = env.response_results();
  //resp.function_values().print(Cout);
  //resp.write(Cout);
  EXPECT_LT(std::fabs(1. - (resp.function_value(0)+1.0) / 1.0), 0.02/100. /* not so good tol */ );

  // Make sure to cleanup the object we own
  delete p_env;
}

// Commented out by MK: Test fasiled with message:
// "Invalid iterator: nlpql_sqp not available."
// //----------------------------------------------------------------

// TEST(opt_tpl_api_tests, test_nlpqlp)
// {
//   Dakota::LibraryEnvironment * p_env = Opt_TPL_Test_Fixture::create_default_env(Dakota::NLPQL_SQP);
//   Dakota::LibraryEnvironment & env = *p_env;

//   if (env.parallel_library().mpirun_flag())
//     FAIL(); // This test only works for serial builds

//   // Execute the environment
//   env.execute();

//   const Real max_tol = 1.e-10;

//   // retrieve the final parameter values
//   const Variables& vars = env.variables_results();
//   //vars.continuous_variables().print(Cout);
//   EXPECT_LT(std::fabs(1. - vars.continuous_variable(0) / -1.50), max_tol/100. );
//   EXPECT_LT(std::fabs(1. - vars.continuous_variable(1) /  0.75), max_tol/100. );

//   // retrieve the final response values
//   const Response& resp  = env.response_results();
//   //resp.function_values().print(Cout);
//   //resp.write(Cout);
//   EXPECT_LT(std::fabs(1. - (resp.function_value(0)+1.0) / 1.0), max_tol/100. );

//   // Make sure to cleanup the object we own
//   delete p_env;
// }

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
