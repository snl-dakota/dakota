
#include "APPSOptimizer.hpp"

#include "opt_tpl_test.hpp"

#include <string>
#include <map>

#include <Teuchos_UnitTestHarness.hpp> 


using namespace Dakota;

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_ncsu,text_book_1)
{
  /// Default Dakota input string:
  static const char text_book_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 500"
    "   ncsu_direct"
    "     min_boxsize_limit 1.e-20"
    "     volume_boxsize_limit 1.e-20"
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
  max_tol = 1.e-4;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e-4;
  rel_err = fabs((vars.continuous_variable(1) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e-4;
  rel_err = fabs((vars.continuous_variable(2) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 0.0;
  max_tol = 1.e-10;
  rel_err = fabs((resp.function_value(0) - target)/1.0);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_ncsu,text_book_2)
{
  /// Default Dakota input string:
  static const char text_book_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 100"
    "   ncsu_direct"
    "     min_boxsize_limit 1.e-20"
    "     volume_boxsize_limit 1.e-20"
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
  max_tol = 1.e0;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e0;
  rel_err = fabs((vars.continuous_variable(1) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e0;
  rel_err = fabs((vars.continuous_variable(2) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 0.0;
  max_tol = 1.e6;
  rel_err = fabs((resp.function_value(0) - target)/1.0);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_ncsu,text_book_3)
{
  /// Default Dakota input string:
  static const char text_book_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 20"
    "   ncsu_direct"
    "     min_boxsize_limit 1.e-20"
    "     volume_boxsize_limit 1.e-20"
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
  max_tol = 1.e1;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e1;
  rel_err = fabs((vars.continuous_variable(1) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e1;
  rel_err = fabs((vars.continuous_variable(2) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 0.0;
  max_tol = 1.e2;
  rel_err = fabs((resp.function_value(0) - target)/1.0);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_ncsu,herbie_1)
{
  /// Default Dakota input string:
  static const char herbie_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 300"
    "   ncsu_direct"
    " variables,"
    "   continuous_design = 2"
    "     initial_point  -1.9 -1.9"
    "     upper_bounds 2.0  2.0"
    "     lower_bounds -2.0 -2.0"
    "     descriptors 'x1'  'x2'"
    " interface,"
    "   direct"
    "     analysis_driver = 'herbie'"
    " responses,"
    "   num_objective_functions = 1"
    "   no_gradients"
    "   no_hessians";

  Dakota::LibraryEnvironment * p_env = Opt_TPL_Test::create_env(herbie_input);
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

  target = -1.04082591;
  max_tol = 1.e-2;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = -1.04082591;
  max_tol = 1.e-2;
  rel_err = fabs((vars.continuous_variable(1) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = -1.1268717458;
  max_tol = 1.e-6;
  rel_err = fabs((resp.function_value(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_ncsu,herbie_2)
{
  /// Default Dakota input string:
  static const char herbie_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 100"
    "   ncsu_direct"
    " variables,"
    "   continuous_design = 2"
    "     initial_point  -1.9 -1.9"
    "     upper_bounds 2.0  2.0"
    "     lower_bounds -2.0 -2.0"
    "     descriptors 'x1'  'x2'"
    " interface,"
    "   direct"
    "     analysis_driver = 'herbie'"
    " responses,"
    "   num_objective_functions = 1"
    "   no_gradients"
    "   no_hessians";

  Dakota::LibraryEnvironment * p_env = Opt_TPL_Test::create_env(herbie_input);
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

  target = -1.04082591;
  max_tol = 1.e-1;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = -1.04082591;
  max_tol = 1.e-1;
  rel_err = fabs((vars.continuous_variable(1) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = -1.1268717458;
  max_tol = 1.e-3;
  rel_err = fabs((resp.function_value(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_ncsu,herbie_3)
{
  /// Default Dakota input string:
  static const char herbie_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 30"
    "   ncsu_direct"
    " variables,"
    "   continuous_design = 2"
    "     initial_point  -1.9 -1.9"
    "     upper_bounds 2.0  2.0"
    "     lower_bounds -2.0 -2.0"
    "     descriptors 'x1'  'x2'"
    " interface,"
    "   direct"
    "     analysis_driver = 'herbie'"
    " responses,"
    "   num_objective_functions = 1"
    "   no_gradients"
    "   no_hessians";

  Dakota::LibraryEnvironment * p_env = Opt_TPL_Test::create_env(herbie_input);
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

  target = -1.04082591;
  max_tol = 1.e1;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = -1.04082591;
  max_tol = 1.e1;
  rel_err = fabs((vars.continuous_variable(1) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = -1.1268717458;
  max_tol = 1.e0;
  rel_err = fabs((resp.function_value(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_ncsu,smooth_herbie_1)
{
  /// Default Dakota input string:
  static const char smooth_herbie_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 500"
    "   ncsu_direct"
    " variables,"
    "   continuous_design = 2"
    "     initial_point  -1.9 -1.9"
    "     upper_bounds 2.0  2.0"
    "     lower_bounds -2.0 -2.0"
    "     descriptors 'x1'  'x2'"
    " interface,"
    "   direct"
    "     analysis_driver = 'smooth_herbie'"
    " responses,"
    "   num_objective_functions = 1"
    "   no_gradients"
    "   no_hessians";

  Dakota::LibraryEnvironment * p_env = Opt_TPL_Test::create_env(smooth_herbie_input);
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

  target = -1.04082591;
  max_tol = 1.e-2;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = -1.04082591;
  max_tol = 1.e-2;
  rel_err = fabs((vars.continuous_variable(1) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = -1.1268717458;
  max_tol = 1.e-6;
  rel_err = fabs((resp.function_value(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_ncsu,smooth_herbie_2)
{
  /// Default Dakota input string:
  static const char smooth_herbie_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 100"
    "   ncsu_direct"
    " variables,"
    "   continuous_design = 2"
    "     initial_point  -1.9 -1.9"
    "     upper_bounds 2.0  2.0"
    "     lower_bounds -2.0 -2.0"
    "     descriptors 'x1'  'x2'"
    " interface,"
    "   direct"
    "     analysis_driver = 'smooth_herbie'"
    " responses,"
    "   num_objective_functions = 1"
    "   no_gradients"
    "   no_hessians";

  Dakota::LibraryEnvironment * p_env = Opt_TPL_Test::create_env(smooth_herbie_input);
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

  target = -1.04082591;
  max_tol = 1.e-1;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = -1.04082591;
  max_tol = 1.e-1;
  rel_err = fabs((vars.continuous_variable(1) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = -1.1268717458;
  max_tol = 1.e-3;
  rel_err = fabs((resp.function_value(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_ncsu,smooth_herbie_3)
{
  /// Default Dakota input string:
  static const char smooth_herbie_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 20"
    "   ncsu_direct"
    " variables,"
    "   continuous_design = 2"
    "     initial_point  -1.9 -1.9"
    "     upper_bounds 2.0  2.0"
    "     lower_bounds -2.0 -2.0"
    "     descriptors 'x1'  'x2'"
    " interface,"
    "   direct"
    "     analysis_driver = 'smooth_herbie'"
    " responses,"
    "   num_objective_functions = 1"
    "   no_gradients"
    "   no_hessians";

  Dakota::LibraryEnvironment * p_env = Opt_TPL_Test::create_env(smooth_herbie_input);
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

  target = -1.04082591;
  max_tol = 1.e1;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = -1.04082591;
  max_tol = 1.e1;
  rel_err = fabs((vars.continuous_variable(1) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = -1.1268717458;
  max_tol = 1.e1;
  rel_err = fabs((resp.function_value(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);
}