/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "opt_tpl_test.hpp"
#include <string>
#include <map>
#include <Teuchos_UnitTestHarness.hpp>

using namespace Dakota;

//----------------------------------------------------------------
/// Unconstrained 3D textbook problem with known solution of
/// {1,1,1}

TEUCHOS_UNIT_TEST(opt_npsol,text_book_base)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   npsol_sqp"
    "     convergence_tolerance = 3.e-8"
    " variables,"
    "   continuous_design = 3"
    "     initial_point  0.0    0.0   0.0"
    "     descriptors 'x_1'  'x_2'  'x_3'"
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

  // convergence tests:
  double rel_err;
  double target;
  double max_tol;

  target = 1.0;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(2) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 0.0;
  max_tol = 1.e-11;
  rel_err = fabs((resp.function_value(0) - target)/1.0);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------
/// 3D textbook problem with active bound constraints:
/// The 2nd variable has a best value at the corresponding upper
/// bound constraint of 0.5 (in comparison to a best value of 1
/// for the unconstrained problem; other two variables have
/// expected best value of 1 with a best objective 0f 0.5^4

TEUCHOS_UNIT_TEST(opt_npsol,text_book_bound_const)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   npsol_sqp"
    "     convergence_tolerance = 3.e-8"
    " variables,"
    "   continuous_design = 3"
    "     initial_point  0.0    0.0   0.0"
    "     upper_bounds  2.0   0.5  2.0"
    "     lower_bounds     0.0  0.0 0.0"
    "     descriptors 'x_1'  'x_2'  'x_3'"
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

  // convergence tests:
  double rel_err;
  double target;
  double max_tol;

  target = 1.0;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 0.5;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(2) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 0.0625;
  max_tol = 1.e-10;
  rel_err = fabs((resp.function_value(0) - target)/target);
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------
/// 3D textbook problem with two active linear equality
/// constraints:
/// The 2nd constraint forces the 3rd variable to have a value of
/// 3; as a result, the first constraint is equivalent to having
/// the sum of the first two variables to be 1; due to the
/// symmetry of the sub-problem (in terms of the first two
/// variables), both variables are expected to have the same best
/// value and therefore each should have a best value of 0.5 with
/// a corresponding best objective of 16.125

TEUCHOS_UNIT_TEST(opt_npsol,text_book_lin_eq_const)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   npsol_sqp"
    "     convergence_tolerance = 3.e-4"
    " variables,"
    "   continuous_design = 3"
    "     initial_point  1.5    -0.5   3.5"
    "     descriptors 'x_1'  'x_2'  'x_3'"
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

  // convergence tests:
  double rel_err;
  double target;
  double max_tol;

  target = 0.5;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 0.5;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 3.0;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(2) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 16.125;
  max_tol = 1.e-8;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------
/// 3D textbook problem with one active nonlinear equality
/// constraint:
/// Since the non-linear constraint is in terms of the first two
/// variables (x_1^2 - x_2/2 = 0), and due to the objective
/// function being decompose-able, the third variable is expected
/// to have a best value of 1. As for the other two variables, the
/// constraint can be rewritten as x_2 = 2x_1^2. Substituting that
/// into the objective function and minimizing, we get a root
/// finding problem for a 7th-degree polynomial as the solution to
/// x_1. The only real root (obtained using matlab) is
/// 0.762750812626551. As a result, the best value for x_2 is
/// 1.163577604324929. The corresponding best objective is
/// 3.884217188745469e-03

TEUCHOS_UNIT_TEST(opt_npsol,text_book_nln_eq_const)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   npsol_sqp"
    "     convergence_tolerance = 1.e-8"
    " variables,"
    "   continuous_design = 3"
    "     initial_point  1.76    0.16   2.0"
    "     descriptors 'x_1'  'x_2'  'x_3'"
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

  // convergence tests:
  double rel_err;
  double target;
  double max_tol;

  target = 0.762750812626551;
  // BMA: Needs review
  max_tol = 1.0e-3;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.163577604324929;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  // BMA: Needs review
  max_tol = 1.0e-3;
  rel_err = fabs((vars.continuous_variable(2) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 3.884217188745469e-03;
  max_tol = 1.e-10;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------
/// 3D textbook problem with two active equality constraints, one
/// linear and one nonlinear:
/// The linear equality forces the first variable to have the
/// value of 1. The non-linear constraint is in terms of the first
/// two variables (x_1^2 - x_2/2 = 1), and thus x_2 must be 0.
/// Also, due to the objective function being decompose-able, the
/// third variable is expected to have a best value of 1 and
/// therefore best objective is 1

TEUCHOS_UNIT_TEST(opt_npsol,text_book_nln_lin_eq_const)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   npsol_sqp"
    "     convergence_tolerance = 1.e-8"
    " variables,"
    "   continuous_design = 3"
    "     initial_point  0.0    0.5   0.0"
    "     descriptors 'x_1'  'x_2'  'x_3'"
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

  // convergence tests:
  double rel_err;
  double target;
  double max_tol;

  target = 1.0;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 0.0;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(1) - target)/1.0 );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  // BMA: Needs review
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(2) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 1.0;
  max_tol = 1.e-12;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------
/// 3D textbook problem with two active linear inequality
/// constraints:
/// The 2nd constraint essentially acts as a bounds constraint for
/// the 3rd variable, x_3, to be between 3 and 4. The other
/// constraint is 0 < x_1 + x_2 < 1. Due to having a
/// symmetric and decompose-able objective being sum_i (x_i - 1)^4,
/// the best x_3 is 3 and x_1 = x_2 = 0.5 with a corresponding best
/// objective of 16.125

TEUCHOS_UNIT_TEST(opt_npsol,text_book_1_lin_ineq_const)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   npsol_sqp"
    "     convergence_tolerance = 3.e-2"
    " variables,"
    "   continuous_design = 3"
    "     initial_point  1.5    -0.5   3.5"
    "     descriptors 'x_1'  'x_2'  'x_3'"
    "     linear_inequality_constraint_matrix = 1. 1. 0."
    "                                         0. 0. 1."
    "     linear_inequality_upper_bounds = 1.0 4.0"
    "     linear_inequality_lower_bounds = 0.0 3.0"
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

  // convergence tests:
  double rel_err;
  double target;
  double max_tol;

  target = 0.5;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 0.5;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 3.0;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(2) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 16.125;
  max_tol = 1.e-12;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------
/// 3D textbook problem with two active linear inequality
/// constraints:
/// The 2nd constraint essentially acts as a bounds constraint for
/// the 3rd variable, x_3, to be between 3 and 4. The other
/// constraint (the sum of all variables to be between 3 and 4) is
/// thus equivalent to 0 < x_1 + x_2 < 1. Due to having a
/// symmetric and decompose-able objective being sum_i (x_i - 1)^4,
/// the best x_3 is 3 and x_1 = x_2 = 0.5 with a corresponding best
/// objective of 16.125

TEUCHOS_UNIT_TEST(opt_npsol,text_book_2_lin_ineq_const)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   npsol_sqp"
    "     convergence_tolerance = 1.e-0"
    " variables,"
    "   continuous_design = 3"
    "     initial_point  1.5    0.1   3.5"
    "     descriptors 'x_1'  'x_2'  'x_3'"
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

  // convergence tests:
  double rel_err;
  double target;
  double max_tol;

  target = 0.5;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 0.5;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 3.0;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(2) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 16.125;
  max_tol = 1.e-10;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------
/// 3D textbook problem with one active nonlinear inequality
/// constraint:
/// Since the non-linear constraint is in terms of the first two
/// variables, being -0.1 < x_1^2 - x_2/2 < 0.1, and due to the
/// objective function being decompose-able, the third variable is
/// expected to have a best value of 1. As for the other two
/// variables, the constraint can be rewritten as
/// 2x_1^2 - 0.2 < x_2 < 2x_1^2 + 0.2. Upon close examination, the
/// objective will be minimized at the 2x_1^2 - 0.2 = x_2
/// interface of the nonlinear (2-sided) inequality constraint.
/// Substituting that into the objective function and minimizing,
/// we get a root finding problem for a 7th-degree polynomial as
/// the solution to x_1. The only real root (obtained using matlab)
/// is 0.8140754878147402. As a result, the best value for x_2 is
/// 1.125437799721614. The corresponding best objective is
/// 1.442520331911729e-03

TEUCHOS_UNIT_TEST(opt_npsol,text_book_nln_ineq_const)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   npsol_sqp"
    "     convergence_tolerance = 1.e-8"
    "     function_precision = 1.e-12"
    " variables,"
    "   continuous_design = 3"
    "     initial_point  -0.2    0.6   0.0"
    "     descriptors 'x_1'  'x_2'  'x_3'"
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

  // convergence tests:
  double rel_err;
  double target;
  double max_tol;

  target = 0.8140754878147402;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.125437799721614;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  // BMA: Needs review
  max_tol = 1.0e-3;
  rel_err = fabs((vars.continuous_variable(2) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 1.442520331911729e-03;
  max_tol = 1.e-9;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------
/// 3D textbook problem with two active inequality constraints,
/// one linear and one nonlinear:
/// The linear inequality constraint essentially acts as a bounds
/// constraint for the 3rd variable, x_3, to be between 2 and 3.
/// Since the non-linear constraint is in terms of the first two
/// variables, being -0.1 < x_1^2 - x_2/2 < 0.1, and due to the
/// objective function being decompose-able, the third variable is
/// expected to have a best value of 2 while satisfying the linear
/// inequality. The nonlinear inequality can be rewritten as
/// 2x_1^2 - 0.2 < x_2 < 2x_1^2 + 0.2. Substituting that into the
/// objective function and minimizing, we get a root finding
/// problem for a 7th-degree polynomial as the solution to x_1.
/// The only real root (obtained using matlab) is
/// 0.8140754878147402. As a result, the best value for x_2 is
/// 1.125437799721614. The corresponding best objective is
/// 1.001442520331912

TEUCHOS_UNIT_TEST(opt_npsol,text_book_nln_lin_ineq_const)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   npsol_sqp"
    "     convergence_tolerance = 3.e-3"
    " variables,"
    "   continuous_design = 3"
    "     initial_point  -0.2    0.6   0.0"
    "     descriptors 'x_1'  'x_2'  'x_3'"
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

  // convergence tests:
  double rel_err;
  double target;
  double max_tol;

  target = 0.8140754878147402;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.125437799721614;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 2.0;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(2) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 1.001442520331912;
  max_tol = 1.e-11;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------
/// 3D textbook problem with two active inequality constraints,
/// one linear and one nonlinear, and two active equality
/// constraints, one linear and one nonlinear:
/// Constraint (1): x_1 + x_2 + x_3 = 4
/// Constraint (2): 2 < x_2 < 3
/// Constraint (3): -0.1 < x_1^2 - x_2/2 < 0.1
/// Constraint (4): x_2^2 - x_1/2 = 0    ==>    x_1 = 2 x_2^2
/// (1) & (2): 1 < x_1 + x_2 < 2 (5)
/// (4) & (5): 1 < 2 x_2^2 + x_2 < 2 (6)
/// (4) & (3): -0.1 < 4 x_2^4 - x_2/2 < 0.1 (7)
/// (6) & root finding: (-1.280776406404415 < x_2 < -1) OR 
///                 (0.5 < x_2 < 0.7807764064044151) (8)
/// (7) & root finding: (-0.189650762381091 < x_2 < 0.218102125965436) OR
///                 (0.395256237746866 < x_2 < 0.554094667611217) (9)
/// (8) & (9): 0.5 < x_2 < 0.554094667611217 (10)
/// (1) & (4): x_3 = 4 - x_2 - x_2^2/2 (11)
///
/// (10) above is the equivalent bound constraint for x_2. Using
/// identities (4) and (11), we can rewrite the objective function
/// in terms of x_2, resulting in an 8-th order poynomial in x_2.
/// MATLAB root finding provides only one root for the derivative,
/// being a global minimum at 0.648817781347564. Since x_2 is bound
/// by (10) which does not include the global minimum of
/// 0.648817781347564, we evaluate the objective at the two bounds
/// in (10), with the miminum found at x_2 = 0.554094667611217 with
/// a corresponding x_1 = 0.614041801350369 and
/// x_3 = 2.831863531038414, along with a best objective of
/// 11.322607724274398, with all constraints satisfied (and active).

TEUCHOS_UNIT_TEST(opt_npsol,text_book_nln_lin_eq_ineq_const)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   npsol_sqp"
    "     convergence_tolerance = 1.e-2"
    " variables,"
    "   continuous_design = 3"
    "     initial_point  1.6    1.5   1.8"
    "     descriptors 'x_1'  'x_2'  'x_3'"
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

  // convergence tests:
  double rel_err;
  double target;
  double max_tol;

  target = 0.614041801350369;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 5.5409525784e-01;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 2.831863531038414;
  max_tol = 1.e-3;
  rel_err = fabs((vars.continuous_variable(2) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env.response_results();

  target = 11.322607724274398;
  max_tol = 1.e-10;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}
