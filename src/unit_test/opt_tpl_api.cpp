
#include "APPSOptimizer.hpp"

#include "opt_tpl_test_fixture.hpp"

#include <string>
#include <map>

#include <Teuchos_UnitTestHarness.hpp> 


using namespace Dakota;

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_api, hopspack)
{
  Dakota::LibraryEnvironment * p_env = Opt_TPL_Test_Fixture::create_default_env(Dakota::ASYNCH_PATTERN_SEARCH);
  Dakota::LibraryEnvironment & env = *p_env;

  if (env.parallel_library().mpirun_flag())
    TEST_ASSERT( false ); // This test only works for serial builds

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
  TEST_FLOATING_EQUALITY( vars.continuous_variable(0), -1.50, max_tol );
  TEST_FLOATING_EQUALITY( vars.continuous_variable(1),  0.75, max_tol );

  // retrieve the final response values
  const Response& resp  = env.response_results();
  //resp.function_values().print(Cout);
  //resp.write(Cout);
  TEST_FLOATING_EQUALITY( resp.function_value(0)+1.0, 1.0, max_tol );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_api, optpp)
{
  Dakota::LibraryEnvironment * p_env = Opt_TPL_Test_Fixture::create_default_env(Dakota::OPTPP_PDS);
  Dakota::LibraryEnvironment & env = *p_env;

  if (env.parallel_library().mpirun_flag())
    TEST_ASSERT( false ); // This test only works for serial builds

  // Execute the environment
  env.execute();

  // retrieve the final parameter values
  const Variables& vars = env.variables_results();
  //vars.continuous_variables().print(Cout);
  TEST_FLOATING_EQUALITY( vars.continuous_variable(0), -1.50, 4.e-3 /* not so good tol */ );
  TEST_FLOATING_EQUALITY( vars.continuous_variable(1),  0.75, 2.e-2 /* ditto */ );

  // retrieve the final response values
  const Response& resp  = env.response_results();
  //resp.function_values().print(Cout);
  //resp.write(Cout);
  TEST_FLOATING_EQUALITY( resp.function_value(0)+1.0, 1.0, 2.e-4 /* not so good tol */ );

  // Make sure to cleanup the object we own
  delete p_env;
}

// Commented out by MK: Test fasiled with message:
// "Invalid iterator: nlpql_sqp not available."
// //----------------------------------------------------------------

// TEUCHOS_UNIT_TEST(opt_api, nlpqlp)
// {
//   Dakota::LibraryEnvironment * p_env = Opt_TPL_Test_Fixture::create_default_env(Dakota::NLPQL_SQP);
//   Dakota::LibraryEnvironment & env = *p_env;

//   if (env.parallel_library().mpirun_flag())
//     TEST_ASSERT( false ); // This test only works for serial builds

//   // Execute the environment
//   env.execute();

//   const Real max_tol = 1.e-10;

//   // retrieve the final parameter values
//   const Variables& vars = env.variables_results();
//   //vars.continuous_variables().print(Cout);
//   TEST_FLOATING_EQUALITY( vars.continuous_variable(0), -1.50, max_tol );
//   TEST_FLOATING_EQUALITY( vars.continuous_variable(1),  0.75, max_tol );

//   // retrieve the final response values
//   const Response& resp  = env.response_results();
//   //resp.function_values().print(Cout);
//   //resp.write(Cout);
//   TEST_FLOATING_EQUALITY( resp.function_value(0)+1.0, 1.0, max_tol );

//   // Make sure to cleanup the object we own
//   delete p_env;
// }
