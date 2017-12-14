
#include "APPSOptimizer.hpp"

#include "opt_tpl_test.hpp"

#include <string>
#include <map>

#include <Teuchos_UnitTestHarness.hpp> 


using namespace Dakota;

//----------------------------------------------------------------
/// Unconstrained 3D textbook problem with known solution of
/// {1,1,1}

TEUCHOS_UNIT_TEST(opt_rol,text_book_base)
{
  /// Dakota input string:
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
    "     descriptors 'x1'  'x2'  'x3'"
    " interface,"
    "   direct"
    "     analysis_driver = 'text_book'"
    " responses,"
    "   num_objective_functions = 1"
    "   analytic_gradients"
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
  max_tol = 1.e-6;
  rel_err = fabs((resp.function_value(0) - target)/1.0);
  TEST_COMPARE(rel_err,<, max_tol);
}


//----------------------------------------------------------------
/// Unconstrained 3D textbook problem with known solution of
/// {1,1,1}
/// This demonstrates setting some Dakota input options that won't
/// meet tolerance, but overriding with XML that will; compare
/// tolerances for this test and the looser ones for the above
/// test

TEUCHOS_UNIT_TEST(opt_rol,text_book_options_file)
{
  /// Dakota input string:

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
    "     descriptors 'x1'  'x2'  'x3'"
    " interface,"
    "   direct"
    "     analysis_driver = 'text_book'"
    " responses,"
    "   num_objective_functions = 1"
    "   analytic_gradients"
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
/// 3D textbook problem with active bound constraints; the 2nd
/// variable has a best value at the corresponding upper bound
/// constraint of 0.5 (in comparison to a best value of 1 for
/// the unconstrained problem; other two variables have expected
/// best value of 1 with a best objective 0f 0.5^4

TEUCHOS_UNIT_TEST(opt_rol,text_book_bound_const)
{
  /// Dakota input string:
  static const char text_book_input[] = 
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-6"
    "     constraint_tolerance 1.0e-6"
    "     threshold_delta 1.0e-6"
    "     max_iterations 100"
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
    "   analytic_gradients"
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

  target = 0.5;
  max_tol = 1.e-10;
  rel_err = fabs((vars.continuous_variable(1) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e-2;
  rel_err = fabs((vars.continuous_variable(2) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 0.0625;
  max_tol = 1.e-8;
  rel_err = fabs((resp.function_value(0) - target)/1.0);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------
/// 3D textbook problem with two active linear equality
/// constraints; the 2nd constraint forces the 3rd variable to
/// have a value of 3; as a result, the first constraint is
/// equivalent to having the sum of the first two variables to
/// be 1; due to the symmetry of the sub-problem (in terms of
/// the first two variables), both variables are expected to
/// have the same best value and therefore each should have a
/// best value of 0.5 with a corresponding best objective of
/// 16.125

TEUCHOS_UNIT_TEST(opt_rol,text_book_lin_eq_const)
{
  /// Dakota input string:
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
    "     descriptors 'x1'  'x2'  'x3'"
    "     linear_equality_constraint_matrix = 1. 1. 1."
    "                                         0. 0. 1."
    "     linear_equality_targets = 4.0 3.0"
    " interface,"
    "   direct"
    "     analysis_driver = 'text_book'"
    " responses,"
    "   num_objective_functions = 1"
    "   analytic_gradients"
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
  max_tol = 1.e-10;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 0.5;
  max_tol = 1.e-10;
  rel_err = fabs((vars.continuous_variable(1) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 3.0;
  max_tol = 1.e-11;
  rel_err = fabs((vars.continuous_variable(2) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 16.125;
  max_tol = 1.e-10;
  rel_err = fabs((resp.function_value(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------
/// 3D textbook problem with one active nonlinear equality
/// constraint; since the non-linear constraint is in terms of the
/// first two variables (x_1^2 - x_2/2 = 0), and due to the
/// objective function being decompose-able, the third variable
/// is expected to have a best value of 1. As for the other two
/// variables, the constraint can be rewritten as x_2 = 2x_1^2.
/// Substituting that into the objective function and minimizing,
/// we get a root finding problem for a 7th-degree polynomial as
/// the solution to x_1. The only real root (obtained using matlab)
/// is 0.762750812626551. As a result, the best value for x_2 is
/// 1.163577604324929. The corresponding best objective is
/// 3.884217188745469e-03

TEUCHOS_UNIT_TEST(opt_rol,text_book_nln_eq_const)
{
  /// Dakota input string:
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
    "     descriptors 'x1'  'x2'  'x3'"
    " interface,"
    "   direct"
    "     analysis_driver = 'text_book'"
    " responses,"
    "   num_objective_functions = 1"
    "   nonlinear_equality_constraints = 1"
    "   nonlinear_equality_targets = 0.0"
    "   analytic_gradients"
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

  target = 0.762750812626551;
  max_tol = 1.e-6;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.163577604324929;
  max_tol = 1.e-5;
  rel_err = fabs((vars.continuous_variable(1) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e-2;
  rel_err = fabs((vars.continuous_variable(2) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 3.884217188745469e-03;
  max_tol = 1.e-4;
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
    "   analytic_gradients"
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
  max_tol = 1.e-3;
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
    "   analytic_gradients"
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
  max_tol = 1.e-5;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 0.5;
  max_tol = 1.e-5;
  rel_err = fabs((vars.continuous_variable(1) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 3.0;
  max_tol = 1.e-5;
  rel_err = fabs((vars.continuous_variable(2) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 1.6125e+01;
  max_tol = 1.e-5;
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
    "     descriptors 'x1'  'x2'  'x3'"
    " interface,"
    "   direct"
    "     analysis_driver = 'text_book'"
    " responses,"
    "   num_objective_functions = 1"
    "   nonlinear_inequality_constraints = 1"
    "   nonlinear_inequality_upper_bounds = 0.1"
    "   nonlinear_inequality_lower_bounds = -0.1"
    "   analytic_gradients"
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
  max_tol = 1.e-5;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.1254320576e+00;
  max_tol = 1.e-5;
  rel_err = fabs((vars.continuous_variable(1) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e-2;
  rel_err = fabs((vars.continuous_variable(2) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 1.4425203319e-03;
  max_tol = 1.e-4;
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
    "   analytic_gradients"
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
  max_tol = 1.e-8;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.1254377841e+00;
  max_tol = 1.e-7;
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
    "   analytic_gradients"
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
  max_tol = 1.e-12;
  rel_err = fabs((vars.continuous_variable(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 5.5409525784e-01;
  max_tol = 1.e-11;
  rel_err = fabs(vars.continuous_variable(1) - target);
  TEST_COMPARE(rel_err,<, max_tol);

  target = 2.8318621616e+00;
  max_tol = 1.e-11;
  rel_err = fabs((vars.continuous_variable(2) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 1.1322573720e+01;
  max_tol = 1.e-10;
  rel_err = fabs((resp.function_value(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);
}
