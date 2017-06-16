
#include "APPSOptimizer.hpp"

#include "opt_tpl_test.hpp"

#include <string>
#include <map>

#include <Teuchos_UnitTestHarness.hpp> 


using namespace Dakota;

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_apps,cyl_head_1)
{
  /// Default Dakota input string:
  static const char cyl_head_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 500"
    "   asynch_pattern_search"
    "     threshold_delta = 1.e-10"
    "     synchronization blocking"
    " variables,"
    "   continuous_design = 2"
    "     initial_point    1.51         0.01"
    "     upper_bounds     2.164        4.0"
    "     lower_bounds     1.5          0.0"
    "     descriptors      'intake_dia' 'flatness'"
    " interface,"
    "   direct"
    "     analysis_driver = 'cyl_head'"
    " responses,"
    "   num_objective_functions = 1"
    "   nonlinear_inequality_constraints = 3"
    "   no_gradients"
    "   no_hessians";

  Dakota::LibraryEnvironment * p_env = Opt_TPL_Test::create_env(cyl_head_input);
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

  target = 2.1224215765;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.7659069377;
  max_tol = 1.e-2;
  rel_err = fabs((vars.continuous_variable(1) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = -2.4614299775;
  max_tol = 1.e-3;
  rel_err = fabs((resp.function_value(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_apps,cyl_head_2)
{
  /// Default Dakota input string:
  static const char cyl_head_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 150"
    "   asynch_pattern_search"
    "     threshold_delta = 1.e-10"
    "     synchronization blocking"
    " variables,"
    "   continuous_design = 2"
    "     initial_point    1.51         0.01"
    "     upper_bounds     2.164        4.0"
    "     lower_bounds     1.5          0.0"
    "     descriptors      'intake_dia' 'flatness'"
    " interface,"
    "   direct"
    "     analysis_driver = 'cyl_head'"
    " responses,"
    "   num_objective_functions = 1"
    "   nonlinear_inequality_constraints = 3"
    "   no_gradients"
    "   no_hessians";

  Dakota::LibraryEnvironment * p_env = Opt_TPL_Test::create_env(cyl_head_input);
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

  target = 2.1224215765;
  max_tol = 1.e-4;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.7659069377;
  max_tol = 1.e-1;
  rel_err = fabs((vars.continuous_variable(1) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = -2.4614299775;
  max_tol = 1.e-2;
  rel_err = fabs((resp.function_value(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_apps,cyl_head_3)
{
  /// Default Dakota input string:
  static const char cyl_head_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 50"
    "   asynch_pattern_search"
    "     threshold_delta = 1.e-10"
    "     synchronization blocking"
    " variables,"
    "   continuous_design = 2"
    "     initial_point    1.51         0.01"
    "     upper_bounds     2.164        4.0"
    "     lower_bounds     1.5          0.0"
    "     descriptors      'intake_dia' 'flatness'"
    " interface,"
    "   direct"
    "     analysis_driver = 'cyl_head'"
    " responses,"
    "   num_objective_functions = 1"
    "   nonlinear_inequality_constraints = 3"
    "   no_gradients"
    "   no_hessians";

  Dakota::LibraryEnvironment * p_env = Opt_TPL_Test::create_env(cyl_head_input);
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

  target = 2.1224215765;
  max_tol = 1.e0;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.7659069377;
  max_tol = 1.e0;
  rel_err = fabs((vars.continuous_variable(1) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = -2.4614299775;
  max_tol = 1.e-1;
  rel_err = fabs((resp.function_value(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_apps,text_book_1)
{
  /// Default Dakota input string:
  static const char text_book_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 1000"
    "   asynch_pattern_search"
    "     threshold_delta = 1.e-10"
    "     synchronization blocking"
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
  max_tol = 1.e-2;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e-2;
  rel_err = fabs((vars.continuous_variable(1) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e-2;
  rel_err = fabs((vars.continuous_variable(2) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 0.0;
  max_tol = 1.e-8;
  rel_err = fabs((resp.function_value(0) - target)/1.0);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_apps,text_book_2)
{
  /// Default Dakota input string:
  static const char text_book_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 300"
    "   asynch_pattern_search"
    "     threshold_delta = 1.e-10"
    "     synchronization blocking"
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

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_apps,text_book_3)
{
  /// Default Dakota input string:
  static const char text_book_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 50"
    "   asynch_pattern_search"
    "     threshold_delta = 1.e-10"
    "     synchronization blocking"
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
  max_tol = 1.e-1;
  rel_err = fabs((vars.continuous_variable(2) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 0.0;
  max_tol = 1.e-2;
  rel_err = fabs((resp.function_value(0) - target)/1.0);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_apps,text_book_lin_constr_1)
{
  /// Default Dakota input string:
  static const char text_book_lin_constr_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 1000"
    "   asynch_pattern_search"
    "     threshold_delta = 1.e-10"
    "     synchronization blocking"
    " variables,"
    "   continuous_design = 3"
    "     initial_point  -1.0    1.5   2.0"
    "     upper_bounds  10.0   10.0  10.0"
    "     lower_bounds     -10.0  -10.0 -10.0"
    "     descriptors 'x1'  'x2'  'x3'"
    "     linear_equality_constraint_matrix = 2. 1. 1."
    "     linear_equality_targets = 4.0"
    " interface,"
    "   direct"
    "     analysis_driver = 'text_book'"
    " responses,"
    "   num_objective_functions = 1"
    "   no_gradients"
    "   no_hessians";

  Dakota::LibraryEnvironment * p_env = Opt_TPL_Test::create_env(text_book_lin_constr_input);
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
  max_tol = 1.e-2;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e-2;
  rel_err = fabs((vars.continuous_variable(1) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e-2;
  rel_err = fabs((vars.continuous_variable(2) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 0.0;
  max_tol = 1.e-9;
  rel_err = fabs((resp.function_value(0) - target)/1.0);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_apps,text_book_lin_constr_2)
{
  /// Default Dakota input string:
  static const char text_book_lin_constr_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 100"
    "   asynch_pattern_search"
    "     threshold_delta = 1.e-10"
    "     synchronization blocking"
    " variables,"
    "   continuous_design = 3"
    "     initial_point  -1.0    1.5   2.0"
    "     upper_bounds  10.0   10.0  10.0"
    "     lower_bounds     -10.0  -10.0 -10.0"
    "     descriptors 'x1'  'x2'  'x3'"
    "     linear_equality_constraint_matrix = 2. 1. 1."
    "     linear_equality_targets = 4.0"
    " interface,"
    "   direct"
    "     analysis_driver = 'text_book'"
    " responses,"
    "   num_objective_functions = 1"
    "   no_gradients"
    "   no_hessians";

  Dakota::LibraryEnvironment * p_env = Opt_TPL_Test::create_env(text_book_lin_constr_input);
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

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_apps,text_book_lin_constr_3)
{
  /// Default Dakota input string:
  static const char text_book_lin_constr_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 20"
    "   asynch_pattern_search"
    "     threshold_delta = 1.e-10"
    "     synchronization blocking"
    " variables,"
    "   continuous_design = 3"
    "     initial_point  -1.0    1.5   2.0"
    "     upper_bounds  10.0   10.0  10.0"
    "     lower_bounds     -10.0  -10.0 -10.0"
    "     descriptors 'x1'  'x2'  'x3'"
    "     linear_equality_constraint_matrix = 2. 1. 1."
    "     linear_equality_targets = 4.0"
    " interface,"
    "   direct"
    "     analysis_driver = 'text_book'"
    " responses,"
    "   num_objective_functions = 1"
    "   no_gradients"
    "   no_hessians";

  Dakota::LibraryEnvironment * p_env = Opt_TPL_Test::create_env(text_book_lin_constr_input);
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
  max_tol = 1.e-3;
  rel_err = fabs((resp.function_value(0) - target)/1.0);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_apps,herbie_1)
{
  /// Default Dakota input string:
  static const char herbie_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 200"
    "   asynch_pattern_search"
    "     threshold_delta = 1.e-10"
    "     synchronization blocking"
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
  max_tol = 1.e-8;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = -1.04082591;
  max_tol = 1.e-8;
  rel_err = fabs((vars.continuous_variable(1) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = -1.1268717458;
  max_tol = 1.e-10;
  rel_err = fabs((resp.function_value(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_apps,herbie_2)
{
  /// Default Dakota input string:
  static const char herbie_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 70"
    "   asynch_pattern_search"
    "     threshold_delta = 1.e-10"
    "     synchronization blocking"
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
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = -1.04082591;
  max_tol = 1.e-3;
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

TEUCHOS_UNIT_TEST(opt_apps,herbie_3)
{
  /// Default Dakota input string:
  static const char herbie_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 25"
    "   asynch_pattern_search"
    "     threshold_delta = 1.e-10"
    "     synchronization blocking"
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
  max_tol = 1.e0;
  rel_err = fabs((vars.continuous_variable(1) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = -1.1268717458;
  max_tol = 1.e-1;
  rel_err = fabs((resp.function_value(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_apps,smooth_herbie_1)
{
  /// Default Dakota input string:
  static const char smooth_herbie_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 200"
    "   asynch_pattern_search"
    "     threshold_delta = 1.e-10"
    "     synchronization blocking"
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
  max_tol = 1.e-1;
  rel_err = fabs((resp.function_value(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_apps,smooth_herbie_2)
{
  /// Default Dakota input string:
  static const char smooth_herbie_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 70"
    "   asynch_pattern_search"
    "     threshold_delta = 1.e-10"
    "     synchronization blocking"
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
  max_tol = 1.e-1;
  rel_err = fabs((resp.function_value(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_apps,smooth_herbie_3)
{
  /// Default Dakota input string:
  static const char smooth_herbie_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 30"
    "   asynch_pattern_search"
    "     threshold_delta = 1.e-10"
    "     synchronization blocking"
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
  max_tol = 1.e-1;
  rel_err = fabs((resp.function_value(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_apps,text_book_disc_range_1)
{
  /// Default Dakota input string:
  static const char text_book_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 300"
    "   asynch_pattern_search"
    "     threshold_delta = 1.e-10"
    "     synchronization blocking"
    " variables,"
    "   discrete_design_range = 3"
    "     initial_point      -5     5     -5"
    "     lower_bounds       -10     -10     -10"
    "     upper_bounds       10     10     10"
    "     descriptors      'y1'   'y2'   'y3'"
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
  max_tol = 1.e-10;
  rel_err = fabs((((double)vars.discrete_int_variable(0)) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e-10;
  rel_err = fabs((((double)vars.discrete_int_variable(1)) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e-10;
  rel_err = fabs((((double)vars.discrete_int_variable(2)) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 0.0;
  max_tol = 1.e-10;
  rel_err = fabs((resp.function_value(0) - target)/1.0);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_apps,text_book_disc_range_2)
{
  /// Default Dakota input string:
  static const char text_book_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 60"
    "   asynch_pattern_search"
    "     threshold_delta = 1.e-10"
    "     synchronization blocking"
    " variables,"
    "   discrete_design_range = 3"
    "     initial_point      -5     5     -5"
    "     lower_bounds       -10     -10     -10"
    "     upper_bounds       10     10     10"
    "     descriptors      'y1'   'y2'   'y3'"
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
  max_tol = 1.e-10;
  rel_err = fabs((((double)vars.discrete_int_variable(0)) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e1;
  rel_err = fabs((((double)vars.discrete_int_variable(1)) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e1;
  rel_err = fabs((((double)vars.discrete_int_variable(2)) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 0.0;
  max_tol = 1.e1;
  rel_err = fabs((resp.function_value(0) - target)/1.0);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_apps,text_book_disc_range_3)
{
  /// Default Dakota input string:
  static const char text_book_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 30"
    "   asynch_pattern_search"
    "     threshold_delta = 1.e-10"
    "     synchronization blocking"
    " variables,"
    "   discrete_design_range = 3"
    "     initial_point      -5     5     -5"
    "     lower_bounds       -10     -10     -10"
    "     upper_bounds       10     10     10"
    "     descriptors      'y1'   'y2'   'y3'"
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
  rel_err = fabs((((double)vars.discrete_int_variable(0)) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e1;
  rel_err = fabs((((double)vars.discrete_int_variable(1)) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e1;
  rel_err = fabs((((double)vars.discrete_int_variable(2)) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 0.0;
  max_tol = 1.e3;
  rel_err = fabs((resp.function_value(0) - target)/1.0);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_apps,text_book_disc_set_1)
{
  /// Default Dakota input string:
  static const char text_book_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 300"
    "   asynch_pattern_search"
    "     threshold_delta = 1.e-10"
    "     synchronization blocking"
    " variables,"
    "   discrete_design_set"
    "   integer = 3"
    "   initial_point     -2   -4   2"
    "   num_set_values =  5   5   5"
    "   set_values = -4 -2 0 2 4 -4 -2 0 2 4 -4 -2 1 2 4"
    "   descriptors      'y1'   'y2'   'y3'"
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

  target = 0.0;
  max_tol = 1.e-10;
  rel_err = fabs((((double)vars.discrete_int_variable(0)) - target)/1.0);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 0.0;
  max_tol = 1.e-10;
  rel_err = fabs((((double)vars.discrete_int_variable(1)) - target)/1.0);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e-10;
  rel_err = fabs((((double)vars.discrete_int_variable(2)) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 2.0;
  max_tol = 1.e-10;
  rel_err = fabs((resp.function_value(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_apps,text_book_disc_set_2)
{
  /// Default Dakota input string:
  static const char text_book_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 15"
    "   asynch_pattern_search"
    "     threshold_delta = 1.e-10"
    "     synchronization blocking"
    " variables,"
    "   discrete_design_set"
    "   integer = 3"
    "   initial_point     -2   -4   2"
    "   num_set_values =  5   5   5"
    "   set_values = -4 -2 0 2 4 -4 -2 0 2 4 -4 -2 1 2 4"
    "   descriptors      'y1'   'y2'   'y3'"
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

  target = 0.0;
  max_tol = 1.e-10;
  rel_err = fabs((((double)vars.discrete_int_variable(0)) - target)/1.0);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 0.0;
  max_tol = 1.e-10;
  rel_err = fabs((((double)vars.discrete_int_variable(1)) - target)/1.0);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e1;
  rel_err = fabs((((double)vars.discrete_int_variable(2)) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 2.0;
  max_tol = 1.e0;
  rel_err = fabs((resp.function_value(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);
}


//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_apps,text_book_disc_set_3)
{
  /// Default Dakota input string:
  static const char text_book_input[] = 
    " method,"
    "   output silent"
    "   max_function_evaluations 10"
    "   asynch_pattern_search"
    "     threshold_delta = 1.e-10"
    "     synchronization blocking"
    " variables,"
    "   discrete_design_set"
    "   integer = 3"
    "   initial_point     -2   -4   2"
    "   num_set_values =  5   5   5"
    "   set_values = -4 -2 0 2 4 -4 -2 0 2 4 -4 -2 1 2 4"
    "   descriptors      'y1'   'y2'   'y3'"
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

  target = 0.0;
  max_tol = 1.e-10;
  rel_err = fabs((((double)vars.discrete_int_variable(0)) - target)/1.0);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 0.0;
  max_tol = 1.e1;
  rel_err = fabs((((double)vars.discrete_int_variable(1)) - target)/1.0);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e1;
  rel_err = fabs((((double)vars.discrete_int_variable(2)) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 2.0;
  max_tol = 1.e2;
  rel_err = fabs((resp.function_value(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);
}