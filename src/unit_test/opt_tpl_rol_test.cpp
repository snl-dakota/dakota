
#include "APPSOptimizer.hpp"

#include "opt_tpl_test.hpp"

#include <string>
#include <map>

#include <Teuchos_UnitTestHarness.hpp> 


using namespace Dakota;

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_rol,text_book_obj)
{
  /// Default Dakota input string:
  /// This demonstrates setting some Dakota input options that won't
  /// meet tolerance, but overriding with XML that will
  static const char text_book_input[] = 
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-3"
    "     constraint_tolerance 1.0e-3"
    "     threshold_delta 1.0e-3"
    "     max_iterations 2"
    "     options_file 'opt_tpl_test_files/opt_rol-text_book.xml'"
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
  max_tol = 1.e-10;
  rel_err = fabs((resp.function_value(0) - target)/1.0);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_rol,text_book_eq_const)
{
  /// Default Dakota input string:
  /// This demonstrates setting some Dakota input options that won't
  /// meet tolerance, but overriding with XML that will
  static const char text_book_input[] = 
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-3"
    "     constraint_tolerance 1.0e-3"
    "     threshold_delta 1.0e-3"
    "     max_iterations 2"
    "     options_file 'opt_tpl_test_files/opt_rol-text_book.xml'"
    " variables,"
    "   continuous_design = 3"
    "     initial_point  -2.3    2.3   0.23"
    "     upper_bounds  10.0   10.0  10.0"
    "     lower_bounds     -10.0  -10.0 -10.0"
    "     descriptors 'x1'  'x2'  'x3'"
    "     linear_equality_constraint_matrix = 1. 1. 1."
    "                                         0. 0. 1."
    "     linear_equality_targets = 4.0 3.0"
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

  target = 0.5;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 0.5;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(1) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 3.0;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(2) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 1.6124999999e+01;
  max_tol = 1.e-3;
  rel_err = fabs((resp.function_value(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);
}
