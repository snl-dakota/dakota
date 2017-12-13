
#include "APPSOptimizer.hpp"

#include "opt_tpl_test.hpp"

#include <string>
#include <map>

#include <Teuchos_UnitTestHarness.hpp> 


using namespace Dakota;

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_rol,text_book_base)
{
  /// Default Dakota input string:
  /// This demonstrates setting some Dakota input options that won't
  /// meet tolerance, but overriding with XML that will
  static const char text_book_input[] = 
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-5"
    "     constraint_tolerance 1.0e-5"
    "     threshold_delta 1.0e-5"
    "     max_iterations 20"
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

  std::shared_ptr<Dakota::LibraryEnvironment> p_env(Opt_TPL_Test::create_env(text_book_input));
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
  max_tol = 1.e-2;
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

TEUCHOS_UNIT_TEST(opt_rol,text_book_options_file)
{
  /// Default Dakota input string:
  /// This demonstrates setting some Dakota input options that won't
  /// meet tolerance, but overriding with XML that will
  static const char text_book_input[] = 
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-5"
    "     constraint_tolerance 1.0e-5"
    "     threshold_delta 1.0e-5"
    "     max_iterations 20"
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

  std::shared_ptr<Dakota::LibraryEnvironment> p_env(Opt_TPL_Test::create_env(text_book_input));
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

TEUCHOS_UNIT_TEST(opt_rol,text_book_bound_const)
{
  /// Default Dakota input string:
  /// This demonstrates setting some Dakota input options that won't
  /// meet tolerance, but overriding with XML that will
  static const char text_book_input[] = 
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-5"
    "     constraint_tolerance 1.0e-5"
    "     threshold_delta 1.0e-5"
    "     max_iterations 20"
    " variables,"
    "   continuous_design = 3"
    "     initial_point  0.2    0.2   0.2"
    "     upper_bounds  2.0   0.5  2.0"
    "     lower_bounds     0.0  0.0 0.0"
    "     descriptors 'x1'  'x2'  'x3'"
    " interface,"
    "   direct"
    "     analysis_driver = 'text_book'"
    " responses,"
    "   num_objective_functions = 1"
    "   no_gradients"
    "   no_hessians";

  std::shared_ptr<Dakota::LibraryEnvironment> p_env(Opt_TPL_Test::create_env(text_book_input));
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

  target = 0.5;
  max_tol = 1.e-10;
  rel_err = fabs((vars.continuous_variable(1) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e-1;
  rel_err = fabs((vars.continuous_variable(2) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 0.0625;
  max_tol = 1.e-7;
  rel_err = fabs((resp.function_value(0) - target)/1.0);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_rol,text_book_lin_eq_const)
{
  /// Default Dakota input string:
  /// This demonstrates setting some Dakota input options that won't
  /// meet tolerance, but overriding with XML that will
  static const char text_book_input[] = 
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-5"
    "     constraint_tolerance 1.0e-5"
    "     threshold_delta 1.0e-5"
    "     max_iterations 20"
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

  std::shared_ptr<Dakota::LibraryEnvironment> p_env(Opt_TPL_Test::create_env(text_book_input));
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

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_rol,text_book_nln_eq_const)
{
  /// Default Dakota input string:
  /// This demonstrates setting some Dakota input options that won't
  /// meet tolerance, but overriding with XML that will
  static const char text_book_input[] = 
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-5"
    "     constraint_tolerance 1.0e-5"
    "     threshold_delta 1.0e-5"
    "     max_iterations 20"
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
    "   nonlinear_equality_constraints = 1"
    "   nonlinear_equality_targets = 0.0"
    "   no_gradients"
    "   no_hessians";

  std::shared_ptr<Dakota::LibraryEnvironment> p_env(Opt_TPL_Test::create_env(text_book_input));
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

  target = 7.6275076292e-01;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.1635774876e+00;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(1) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 9.9757169578e-01;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(2) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 3.8842178353e-03;
  max_tol = 1.e-3;
  rel_err = fabs((resp.function_value(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_rol,text_book_nln_lin_eq_const)
{
  /// Default Dakota input string:
  /// This demonstrates setting some Dakota input options that won't
  /// meet tolerance, but overriding with XML that will
  static const char text_book_input[] = 
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-4"
    "     constraint_tolerance 1.0e-4"
    "     threshold_delta 1.0e-4"
    "     max_iterations 20"
    " variables,"
    "   continuous_design = 3"
    "     initial_point  0.0    0.0   0.0"
    "     upper_bounds  2.0   2.0  2.0"
    "     lower_bounds     -2.0  -2.0 -2.0"
    "     descriptors 'x1'  'x2'  'x3'"
    "     linear_equality_constraint_matrix = 1. 0. 0."
    "     linear_equality_targets = 1.0"
    " interface,"
    "   direct"
    "     analysis_driver = 'text_book'"
    " responses,"
    "   num_objective_functions = 1"
    "   nonlinear_equality_constraints = 1"
    "   nonlinear_equality_targets = 1.0"
    "   no_gradients"
    "   no_hessians";

  std::shared_ptr<Dakota::LibraryEnvironment> p_env(Opt_TPL_Test::create_env(text_book_input));
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

  target = 0.0;
  max_tol = 1.e-3;
  rel_err = fabs(vars.continuous_variable(1) - target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e-2;
  rel_err = fabs((vars.continuous_variable(2) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 1.0;
  max_tol = 1.e-3;
  rel_err = fabs((resp.function_value(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_rol,text_book_lin_ineq_const)
{
  /// Default Dakota input string:
  /// This demonstrates setting some Dakota input options that won't
  /// meet tolerance, but overriding with XML that will
  static const char text_book_input[] = 
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-5"
    "     constraint_tolerance 1.0e-5"
    "     threshold_delta 1.0e-5"
    "     max_iterations 20"
    " variables,"
    "   continuous_design = 3"
    "     initial_point  -2.3    2.3   0.23"
    "     upper_bounds  10.0   10.0  10.0"
    "     lower_bounds     -10.0  -10.0 -10.0"
    "     descriptors 'x1'  'x2'  'x3'"
    "     linear_inequality_constraint_matrix = 1. 1. 1."
    "                                         0. 0. 1."
    "     linear_inequality_upper_bounds = 4.0 4.0"
    "     linear_inequality_lower_bounds = 3.0 3.0"
    " interface,"
    "   direct"
    "     analysis_driver = 'text_book'"
    " responses,"
    "   num_objective_functions = 1"
    "   no_gradients"
    "   no_hessians";

  std::shared_ptr<Dakota::LibraryEnvironment> p_env(Opt_TPL_Test::create_env(text_book_input));
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

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_rol,text_book_nln_ineq_const)
{
  /// Default Dakota input string:
  /// This demonstrates setting some Dakota input options that won't
  /// meet tolerance, but overriding with XML that will
  static const char text_book_input[] = 
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-5"
    "     constraint_tolerance 1.0e-5"
    "     threshold_delta 1.0e-5"
    "     max_iterations 20"
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
    "   nonlinear_inequality_constraints = 1"
    "   nonlinear_inequality_upper_bounds = 0.1"
    "   nonlinear_inequality_lower_bounds = -0.1"
    "   no_gradients"
    "   no_hessians";

  std::shared_ptr<Dakota::LibraryEnvironment> p_env(Opt_TPL_Test::create_env(text_book_input));
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

  target = 8.1407406910e-01;
  max_tol = 1.e-11;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.1254320576e+00;
  max_tol = 1.e-10;
  rel_err = fabs((vars.continuous_variable(1) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e-2;
  rel_err = fabs((vars.continuous_variable(2) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 1.4425203319e-03;
  max_tol = 1.e-5;
  rel_err = fabs((resp.function_value(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_rol,text_book_nln_lin_ineq_const)
{
  /// Default Dakota input string:
  /// This demonstrates setting some Dakota input options that won't
  /// meet tolerance, but overriding with XML that will
  static const char text_book_input[] = 
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-5"
    "     constraint_tolerance 1.0e-5"
    "     threshold_delta 1.0e-5"
    "     max_iterations 20"
    " variables,"
    "   continuous_design = 3"
    "     initial_point  0.0    0.0   0.0"
    "     upper_bounds  10.0   10.0  10.0"
    "     lower_bounds     -10.0  -10.0 -10.0"
    "     descriptors 'x1'  'x2'  'x3'"
    "     linear_inequality_constraint_matrix = 0. 0. 1."
    "     linear_inequality_upper_bounds = 3.0"
    "     linear_inequality_lower_bounds = 2.0"
    " interface,"
    "   direct"
    "     analysis_driver = 'text_book'"
    " responses,"
    "   num_objective_functions = 1"
    "   nonlinear_inequality_constraints = 1"
    "   nonlinear_inequality_upper_bounds = 0.1"
    "   nonlinear_inequality_lower_bounds = -0.1"
    "   no_gradients"
    "   no_hessians";

  std::shared_ptr<Dakota::LibraryEnvironment> p_env(Opt_TPL_Test::create_env(text_book_input));
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

  target = 8.1407548275e-01;
  max_tol = 1.e-11;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.1254377841e+00;
  max_tol = 1.e-11;
  rel_err = fabs(vars.continuous_variable(1) - target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 2.0;
  max_tol = 1.e-5;
  rel_err = fabs((vars.continuous_variable(2) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 1.0014425203387e+00;
  max_tol = 1.e-4;
  rel_err = fabs((resp.function_value(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_rol,text_book_nln_lin_eq_ineq_const)
{
  /// Default Dakota input string:
  /// This demonstrates setting some Dakota input options that won't
  /// meet tolerance, but overriding with XML that will
  static const char text_book_input[] = 
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-5"
    "     constraint_tolerance 1.0e-5"
    "     threshold_delta 1.0e-5"
    "     max_iterations 20"
    " variables,"
    "   continuous_design = 3"
    "     initial_point  0.0    0.0   0.0"
    "     upper_bounds  10.0   10.0  10.0"
    "     lower_bounds     -10.0  -10.0 -10.0"
    "     descriptors 'x1'  'x2'  'x3'"
    "     linear_equality_constraint_matrix = 1. 1. 1."
    "     linear_equality_targets = 4.0"
    "     linear_inequality_constraint_matrix = 0. 0. 1."
    "     linear_inequality_upper_bounds = 3.0"
    "     linear_inequality_lower_bounds = 2.0"
    " interface,"
    "   direct"
    "     analysis_driver = 'text_book'"
    " responses,"
    "   num_objective_functions = 1"
    "   nonlinear_inequality_constraints = 1"
    "   nonlinear_inequality_upper_bounds = 0.1"
    "   nonlinear_inequality_lower_bounds = -0.1"
    "   nonlinear_equality_constraints = 1"
    "   nonlinear_equality_targets = 0.0"
    "   no_gradients"
    "   no_hessians";

  std::shared_ptr<Dakota::LibraryEnvironment> p_env(Opt_TPL_Test::create_env(text_book_input));
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

  target = 6.1404233466e-01;
  max_tol = 1.e-5;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 5.5409525784e-01;
  max_tol = 1.e-5;
  rel_err = fabs(vars.continuous_variable(1) - target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 2.8318621616e+00;
  max_tol = 1.e-5;
  rel_err = fabs((vars.continuous_variable(2) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 1.1322573720e+01;
  max_tol = 1.e-4;
  rel_err = fabs((resp.function_value(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);
}
