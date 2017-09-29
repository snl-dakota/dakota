
#include "APPSOptimizer.hpp"

#include "opt_tpl_test.hpp"

#include <string>
#include <map>

#include <Teuchos_UnitTestHarness.hpp> 


using namespace Dakota;

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_apps,text_book)
{
  /// Default Dakota input string:
  static const char text_book_input[] = 
    " method,"
    "   rol_ls"
    " variables,"
    "   continuous_design = 3"
    "     initial_point  -2.3    2.3   0.23"
    "     upper_bounds  10.0   10.0  10.0"
    "     lower_bounds     -10.0  -10.0 -10.0"
    "     descriptors 'x1'  'x2'  'x3'"
    " interface,"
    "   direct"
    "     analysis_driver = 'text_book'"
    " responses,"
    "   num_objective_functions = 1"
    "   no_gradients"
    "   no_hessians";

  Dakota::LibraryEnvironment * p_env = Opt_TPL_Test::create_env(text_book_input);
  Dakota::LibraryEnvironment & env = *p_env;

  if (env.parallel_library().mpirun_flag())
    TEST_ASSERT( false ); // This test only works for serial builds

  // Execute the environment
  env.execute();

  // retrieve the final parameter values
  const Variables& vars = env.variables_results();

  // convergence tests: "Optimal" solution used for comparison
  // is obtained using Teuchos unit tests in
  // opt_tpl_test_exact_soln.cpp
  double rel_err;
  double target;
  double max_tol;

  target = 1.0;
  max_tol = 1.e-1;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e-1;
  rel_err = fabs((vars.continuous_variable(1) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e-1;
  rel_err = fabs((vars.continuous_variable(2) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 0.0;
  max_tol = 1.e-7;
  rel_err = fabs((resp.function_value(0) - target)/1.0);
  TEST_COMPARE(rel_err,<, max_tol);
}