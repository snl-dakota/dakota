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

// extern PRPCache data_pairs;

namespace Dakota {
  extern PRPCache data_pairs;


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

TEUCHOS_UNIT_TEST(opt_rol,easy_nln_ineq_const_1)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-8"
    "     constraint_tolerance 1.0e-8"
    "     threshold_delta 1.0e-8"
    "     max_iterations 20"
    " variables,"
    "   continuous_design = 3"
    "     initial_point  0.3    0.6   0.5"
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

  std::shared_ptr<Dakota::LibraryEnvironment> p_env1(Opt_TPL_Test::create_env(text_book_input));
  Dakota::LibraryEnvironment & env1 = *p_env1;

  if (env1.parallel_library().mpirun_flag())
    TEST_ASSERT( false ); // This test only works for serial builds

  // Execute the environment

  data_pairs.clear();
  env1.execute();

  // retrieve the final parameter values
  const Variables& vars = env1.variables_results();

  // convergence tests:
  double rel_err;
  double target;
  double max_tol;

  target = 0.8140754878147402;
  max_tol = 1.e-9;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.125437799721614;
  max_tol = 1.e-9;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  // BMA: Needs review
  max_tol = 1.0e-3;
  rel_err = fabs((vars.continuous_variable(2) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env1.response_results();

  target = 1.442520331911729e-03;
  max_tol = 1.e-8;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}

TEUCHOS_UNIT_TEST(opt_rol,easy_nln_ineq_const_2)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-4"
    "     constraint_tolerance 1.0e-8"
    "     threshold_delta 1.0e-8"
    "     max_iterations 20"
    " variables,"
    "   continuous_design = 3"
    "     initial_point  0.3    0.6   0.5"
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

  std::shared_ptr<Dakota::LibraryEnvironment> p_env2(Opt_TPL_Test::create_env(text_book_input));
  Dakota::LibraryEnvironment & env2 = *p_env2;

  if (env2.parallel_library().mpirun_flag())
    TEST_ASSERT( false ); // This test only works for serial builds

  // Execute the environment

  data_pairs.clear();
  env2.execute();

  // retrieve the final parameter values
  const Variables& vars = env2.variables_results();

  // convergence tests:
  double rel_err;
  double target;
  double max_tol;

  target = 0.8140754878147402;
  max_tol = 1.e-7;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.125437799721614;
  max_tol = 1.e-7;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  // BMA: Needs review
  max_tol = 1.0e-2;
  rel_err = fabs((vars.continuous_variable(2) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env2.response_results();

  target = 1.442520331911729e-03;
  max_tol = 1.e-6;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}

TEUCHOS_UNIT_TEST(opt_rol,easy_nln_ineq_const_3)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-8"
    "     constraint_tolerance 1.0e-4"
    "     threshold_delta 1.0e-8"
    "     max_iterations 20"
    " variables,"
    "   continuous_design = 3"
    "     initial_point  0.3    0.6   0.5"
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

  std::shared_ptr<Dakota::LibraryEnvironment> p_env3(Opt_TPL_Test::create_env(text_book_input));
  Dakota::LibraryEnvironment & env3 = *p_env3;

  if (env3.parallel_library().mpirun_flag())
    TEST_ASSERT( false ); // This test only works for serial builds

  // Execute the environment
  data_pairs.clear();
  env3.execute();

  // retrieve the final parameter values
  const Variables& vars = env3.variables_results();

  // convergence tests:
  double rel_err;
  double target;
  double max_tol;

  target = 0.8140754878147402;
  max_tol = 1.e-7;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.125437799721614;
  max_tol = 1.e-7;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  // BMA: Needs review
  max_tol = 1.0e-3;
  rel_err = fabs((vars.continuous_variable(2) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env3.response_results();

  target = 1.442520331911729e-03;
  max_tol = 1.e-6;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}

TEUCHOS_UNIT_TEST(opt_rol,easy_nln_ineq_const_4)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-8"
    "     constraint_tolerance 1.0e-8"
    "     threshold_delta 1.0e-4"
    "     max_iterations 20"
    " variables,"
    "   continuous_design = 3"
    "     initial_point  0.3    0.6   0.5"
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

  std::shared_ptr<Dakota::LibraryEnvironment> p_env4(Opt_TPL_Test::create_env(text_book_input));
  Dakota::LibraryEnvironment & env4 = *p_env4;

  if (env4.parallel_library().mpirun_flag())
    TEST_ASSERT( false ); // This test only works for serial builds

  // Execute the environment
  data_pairs.clear();
  env4.execute();

  // retrieve the final parameter values
  const Variables& vars = env4.variables_results();

  // convergence tests:
  double rel_err;
  double target;
  double max_tol;

  target = 0.8140754878147402;
  max_tol = 1.e-7;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.125437799721614;
  max_tol = 1.e-6;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  // BMA: Needs review
  max_tol = 1.0e-2;
  rel_err = fabs((vars.continuous_variable(2) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env4.response_results();

  target = 1.442520331911729e-03;
  max_tol = 1.e-6;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}

TEUCHOS_UNIT_TEST(opt_rol,easy_nln_ineq_const_5)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-8"
    "     constraint_tolerance 1.0e-8"
    "     threshold_delta 1.0e-8"
    "     max_iterations 5"
    " variables,"
    "   continuous_design = 3"
    "     initial_point  0.3    0.6   0.5"
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

  std::shared_ptr<Dakota::LibraryEnvironment> p_env5(Opt_TPL_Test::create_env(text_book_input));
  Dakota::LibraryEnvironment & env5 = *p_env5;

  if (env5.parallel_library().mpirun_flag())
    TEST_ASSERT( false ); // This test only works for serial builds

  // Execute the environment
  data_pairs.clear();
  env5.execute();

  // retrieve the final parameter values
  const Variables& vars = env5.variables_results();

  // convergence tests:
  double rel_err;
  double target;
  double max_tol;

  target = 0.8140754878147402;
  max_tol = 1.e-7;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.125437799721614;
  max_tol = 1.e-6;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  // BMA: Needs review
  max_tol = 1.0e-2;
  rel_err = fabs((vars.continuous_variable(2) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env5.response_results();

  target = 1.442520331911729e-03;
  max_tol = 1.e-6;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}


//----------------------------------------------------------------
/// 3D textbook problem with two active nonlinear inequality
/// constraints:
/// Since the non-linear constraints are in terms of the first two
/// variables, and due to the objective function being
/// decompose-able, the third through 5th variables are expected
/// to have a best value of 1. As for the other two variables, the
/// constraints can be rewritten as 2x_1^2 < x_2 < 2x_1^2 + 0.4
/// and 2x_2^2 -0.4 < x_1 < 2x_2^2. Upon close examination, the
/// objective will be minimized at the x_2 = 2x_1^2 + 0.4 and 
/// x_1 = 2x_2^2 interfaces of the nonlinear (2-sided) inequality
/// constraints. The two constraints can be combined to get a root
/// finding problem involving a 4th-degree polynomial as the
/// solution to x_2 (satisfying the two constraints). There are
/// two real roots for x_2, being -3.180943549534391e-01 and 
/// 5.936586620315043e-01. The later results in a smaller objective
/// function value. As a result, the best value for x_2 is
/// 5.936586620315043e-01 with c orresponding x_1 of
/// 7.048612140100710e-01 and a best objective of
/// 3.485000875338049e-02

TEUCHOS_UNIT_TEST(opt_rol,medium_nln_ineq_const_1)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-8"
    "     constraint_tolerance 1.0e-8"
    "     threshold_delta 1.0e-8"
    "     max_iterations 20"
    " variables,"
    "   continuous_design = 5"
    "     initial_point  5*0.0"
    "     descriptors 'x_1'  'x_2'  'x_3'  'x_4'  'x_5'"
    " interface,"
    "   direct"
    "     analysis_driver = 'text_book'"
    " responses,"
    "   num_objective_functions = 1"
    "   nonlinear_inequality_constraints = 2"
    "   nonlinear_inequality_upper_bounds = 0.2 0.0"
    "   nonlinear_inequality_lower_bounds = 0.0 -0.2"
    "   analytic_gradients"
    "   no_hessians";

  std::shared_ptr<Dakota::LibraryEnvironment> p_env1(Opt_TPL_Test::create_env(text_book_input));
  Dakota::LibraryEnvironment & env1 = *p_env1;

  if (env1.parallel_library().mpirun_flag())
    TEST_ASSERT( false ); // This test only works for serial builds

  // Execute the environment
  data_pairs.clear();
  env1.execute();

  // retrieve the final parameter values
  const Variables& vars = env1.variables_results();

  // convergence tests:
  double rel_err;
  double target;
  double max_tol;

  target = 7.048612140100710e-01;
  max_tol = 1.e-8;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 5.936586620315040e-01;
  max_tol = 1.e-8;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  // BMA: Needs review
  max_tol = 1.0e-3;
  rel_err = fabs((vars.continuous_variable(2) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env1.response_results();

  target = 3.485000875338049e-02;
  max_tol = 1.e-7;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}

TEUCHOS_UNIT_TEST(opt_rol,medium_nln_ineq_const_2)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-4"
    "     constraint_tolerance 1.0e-8"
    "     threshold_delta 1.0e-8"
    "     max_iterations 20"
    " variables,"
    "   continuous_design = 5"
    "     initial_point  5*0.0"
    "     descriptors 'x_1'  'x_2'  'x_3'  'x_4'  'x_5'"
    " interface,"
    "   direct"
    "     analysis_driver = 'text_book'"
    " responses,"
    "   num_objective_functions = 1"
    "   nonlinear_inequality_constraints = 2"
    "   nonlinear_inequality_upper_bounds = 0.2 0.0"
    "   nonlinear_inequality_lower_bounds = 0.0 -0.2"
    "   analytic_gradients"
    "   no_hessians";

  std::shared_ptr<Dakota::LibraryEnvironment> p_env1(Opt_TPL_Test::create_env(text_book_input));
  Dakota::LibraryEnvironment & env1 = *p_env1;

  if (env1.parallel_library().mpirun_flag())
    TEST_ASSERT( false ); // This test only works for serial builds

  // Execute the environment
  data_pairs.clear();
  env1.execute();

  // retrieve the final parameter values
  const Variables& vars = env1.variables_results();

  // convergence tests:
  double rel_err;
  double target;
  double max_tol;

  target = 7.048612140100710e-01;
  max_tol = 1.e-8;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 5.936586620315040e-01;
  max_tol = 1.e-7;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  // BMA: Needs review
  max_tol = 1.0e-2;
  rel_err = fabs((vars.continuous_variable(2) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env1.response_results();

  target = 3.485000875338049e-02;
  max_tol = 1.e-7;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}

TEUCHOS_UNIT_TEST(opt_rol,medium_nln_ineq_const_3)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-8"
    "     constraint_tolerance 1.0e-4"
    "     threshold_delta 1.0e-8"
    "     max_iterations 20"
    " variables,"
    "   continuous_design = 5"
    "     initial_point  5*0.0"
    "     descriptors 'x_1'  'x_2'  'x_3'  'x_4'  'x_5'"
    " interface,"
    "   direct"
    "     analysis_driver = 'text_book'"
    " responses,"
    "   num_objective_functions = 1"
    "   nonlinear_inequality_constraints = 2"
    "   nonlinear_inequality_upper_bounds = 0.2 0.0"
    "   nonlinear_inequality_lower_bounds = 0.0 -0.2"
    "   analytic_gradients"
    "   no_hessians";

  std::shared_ptr<Dakota::LibraryEnvironment> p_env1(Opt_TPL_Test::create_env(text_book_input));
  Dakota::LibraryEnvironment & env1 = *p_env1;

  if (env1.parallel_library().mpirun_flag())
    TEST_ASSERT( false ); // This test only works for serial builds

  // Execute the environment
  data_pairs.clear();
  env1.execute();

  // retrieve the final parameter values
  const Variables& vars = env1.variables_results();

  // convergence tests:
  double rel_err;
  double target;
  double max_tol;

  target = 7.048612140100710e-01;
  max_tol = 1.e-4;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 5.936586620315040e-01;
  max_tol = 1.e-4;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  // BMA: Needs review
  max_tol = 1.0e-2;
  rel_err = fabs((vars.continuous_variable(2) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env1.response_results();

  target = 3.485000875338049e-02;
  max_tol = 1.e-3;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}

TEUCHOS_UNIT_TEST(opt_rol,medium_nln_ineq_const_4)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-8"
    "     constraint_tolerance 1.0e-8"
    "     threshold_delta 1.0e-4"
    "     max_iterations 20"
    " variables,"
    "   continuous_design = 5"
    "     initial_point  5*0.0"
    "     descriptors 'x_1'  'x_2'  'x_3'  'x_4'  'x_5'"
    " interface,"
    "   direct"
    "     analysis_driver = 'text_book'"
    " responses,"
    "   num_objective_functions = 1"
    "   nonlinear_inequality_constraints = 2"
    "   nonlinear_inequality_upper_bounds = 0.2 0.0"
    "   nonlinear_inequality_lower_bounds = 0.0 -0.2"
    "   analytic_gradients"
    "   no_hessians";

  std::shared_ptr<Dakota::LibraryEnvironment> p_env1(Opt_TPL_Test::create_env(text_book_input));
  Dakota::LibraryEnvironment & env1 = *p_env1;

  if (env1.parallel_library().mpirun_flag())
    TEST_ASSERT( false ); // This test only works for serial builds

  // Execute the environment
  data_pairs.clear();
  env1.execute();

  // retrieve the final parameter values
  const Variables& vars = env1.variables_results();

  // convergence tests:
  double rel_err;
  double target;
  double max_tol;

  target = 7.048612140100710e-01;
  max_tol = 1.e-6;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 5.936586620315040e-01;
  max_tol = 1.e-6;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  // BMA: Needs review
  max_tol = 1.0e-3;
  rel_err = fabs((vars.continuous_variable(2) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env1.response_results();

  target = 3.485000875338049e-02;
  max_tol = 1.e-5;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}

TEUCHOS_UNIT_TEST(opt_rol,medium_nln_ineq_const_5)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-8"
    "     constraint_tolerance 1.0e-8"
    "     threshold_delta 1.0e-8"
    "     max_iterations 5"
    " variables,"
    "   continuous_design = 5"
    "     initial_point  5*0.0"
    "     descriptors 'x_1'  'x_2'  'x_3'  'x_4'  'x_5'"
    " interface,"
    "   direct"
    "     analysis_driver = 'text_book'"
    " responses,"
    "   num_objective_functions = 1"
    "   nonlinear_inequality_constraints = 2"
    "   nonlinear_inequality_upper_bounds = 0.2 0.0"
    "   nonlinear_inequality_lower_bounds = 0.0 -0.2"
    "   analytic_gradients"
    "   no_hessians";

  std::shared_ptr<Dakota::LibraryEnvironment> p_env1(Opt_TPL_Test::create_env(text_book_input));
  Dakota::LibraryEnvironment & env1 = *p_env1;

  if (env1.parallel_library().mpirun_flag())
    TEST_ASSERT( false ); // This test only works for serial builds

  // Execute the environment
  data_pairs.clear();
  env1.execute();

  // retrieve the final parameter values
  const Variables& vars = env1.variables_results();

  // convergence tests:
  double rel_err;
  double target;
  double max_tol;

  target = 7.048612140100710e-01;
  max_tol = 1.e-2;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 5.936586620315040e-01;
  max_tol = 1.e-2;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  // BMA: Needs review
  max_tol = 1.0e-2;
  rel_err = fabs((vars.continuous_variable(2) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env1.response_results();

  target = 3.485000875338049e-02;
  max_tol = 1.e-1;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}


//----------------------------------------------------------------
/// 3D cylinder head problem with three active nonlinear
/// inequality constraints:
/// "Optimal" solution used for comparison is obtained using
/// Teuchos unit tests in opt_tpl_test_exact_soln.cpp

TEUCHOS_UNIT_TEST(opt_rol,hard_nln_ineq_const_1)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-8"
    "     constraint_tolerance 1.0e-8"
    "     threshold_delta 1.0e-8"
    "     max_iterations 20"
    " variables,"
    "   continuous_design = 2"
    "     initial_point  1.8 1.0"
    " interface,"
    "   direct"
    "     analysis_driver = 'cyl_head'"
    " responses,"
    "   num_objective_functions = 1"
    "   nonlinear_inequality_constraints = 3"
    "   nonlinear_inequality_upper_bounds = 0.0 0.0 0.0"
    "   nonlinear_inequality_lower_bounds = -1.0 -1.0 -0.1 "
    "   numerical_gradients"
    "      method_source dakota"
    "      interval_type central"
    "      fd_step_size = 1.e-6"
    "   no_hessians";

  std::shared_ptr<Dakota::LibraryEnvironment> p_env1(Opt_TPL_Test::create_env(text_book_input));
  Dakota::LibraryEnvironment & env1 = *p_env1;

  if (env1.parallel_library().mpirun_flag())
    TEST_ASSERT( false ); // This test only works for serial builds

  // Execute the environment
  data_pairs.clear();
  env1.execute();

  // retrieve the final parameter values
  const Variables& vars = env1.variables_results();

  // convergence tests: "Optimal" solution used for comparison
  // is obtained using Teuchos unit tests in
  // opt_tpl_test_exact_soln.cpp
  double rel_err;
  double target;
  double max_tol;

  target = 2.1224215765;
  max_tol = 1.e-5;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.7659069377;
  max_tol = 1.e-2;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env1.response_results();

  target = -2.4614299775;
  max_tol = 1.e-3;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}

TEUCHOS_UNIT_TEST(opt_rol,hard_nln_ineq_const_2)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-4"
    "     constraint_tolerance 1.0e-8"
    "     threshold_delta 1.0e-8"
    "     max_iterations 20"
    " variables,"
    "   continuous_design = 2"
    "     initial_point  1.8 1.0"
    " interface,"
    "   direct"
    "     analysis_driver = 'cyl_head'"
    " responses,"
    "   num_objective_functions = 1"
    "   nonlinear_inequality_constraints = 3"
    "   nonlinear_inequality_upper_bounds = 0.0 0.0 0.0"
    "   nonlinear_inequality_lower_bounds = -1.0 -1.0 -0.1 "
    "   numerical_gradients"
    "      method_source dakota"
    "      interval_type central"
    "      fd_step_size = 1.e-6"
    "   no_hessians";

  std::shared_ptr<Dakota::LibraryEnvironment> p_env1(Opt_TPL_Test::create_env(text_book_input));
  Dakota::LibraryEnvironment & env1 = *p_env1;

  if (env1.parallel_library().mpirun_flag())
    TEST_ASSERT( false ); // This test only works for serial builds

  // Execute the environment
  data_pairs.clear();
  env1.execute();

  // retrieve the final parameter values
  const Variables& vars = env1.variables_results();

  // convergence tests: "Optimal" solution used for comparison
  // is obtained using Teuchos unit tests in
  // opt_tpl_test_exact_soln.cpp
  double rel_err;
  double target;
  double max_tol;

  target = 2.1224215765;
  max_tol = 1.e-5;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.7659069377;
  max_tol = 1.e-2;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env1.response_results();

  target = -2.4614299775;
  max_tol = 1.e-3;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}

TEUCHOS_UNIT_TEST(opt_rol,hard_nln_ineq_const_3)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-8"
    "     constraint_tolerance 1.0e-4"
    "     threshold_delta 1.0e-8"
    "     max_iterations 20"
    " variables,"
    "   continuous_design = 2"
    "     initial_point  1.8 1.0"
    " interface,"
    "   direct"
    "     analysis_driver = 'cyl_head'"
    " responses,"
    "   num_objective_functions = 1"
    "   nonlinear_inequality_constraints = 3"
    "   nonlinear_inequality_upper_bounds = 0.0 0.0 0.0"
    "   nonlinear_inequality_lower_bounds = -1.0 -1.0 -0.1 "
    "   numerical_gradients"
    "      method_source dakota"
    "      interval_type central"
    "      fd_step_size = 1.e-6"
    "   no_hessians";

  std::shared_ptr<Dakota::LibraryEnvironment> p_env1(Opt_TPL_Test::create_env(text_book_input));
  Dakota::LibraryEnvironment & env1 = *p_env1;

  if (env1.parallel_library().mpirun_flag())
    TEST_ASSERT( false ); // This test only works for serial builds

  // Execute the environment
  data_pairs.clear();
  env1.execute();

  // retrieve the final parameter values
  const Variables& vars = env1.variables_results();

  // convergence tests: "Optimal" solution used for comparison
  // is obtained using Teuchos unit tests in
  // opt_tpl_test_exact_soln.cpp
  double rel_err;
  double target;
  double max_tol;

  target = 2.1224215765;
  max_tol = 1.e-5;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.7659069377;
  max_tol = 1.e-2;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env1.response_results();

  target = -2.4614299775;
  max_tol = 1.e-3;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}

TEUCHOS_UNIT_TEST(opt_rol,hard_nln_ineq_const_4)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-8"
    "     constraint_tolerance 1.0e-8"
    "     threshold_delta 1.0e-4"
    "     max_iterations 20"
    " variables,"
    "   continuous_design = 2"
    "     initial_point  1.8 1.0"
    " interface,"
    "   direct"
    "     analysis_driver = 'cyl_head'"
    " responses,"
    "   num_objective_functions = 1"
    "   nonlinear_inequality_constraints = 3"
    "   nonlinear_inequality_upper_bounds = 0.0 0.0 0.0"
    "   nonlinear_inequality_lower_bounds = -1.0 -1.0 -0.1 "
    "   numerical_gradients"
    "      method_source dakota"
    "      interval_type central"
    "      fd_step_size = 1.e-6"
    "   no_hessians";

  std::shared_ptr<Dakota::LibraryEnvironment> p_env1(Opt_TPL_Test::create_env(text_book_input));
  Dakota::LibraryEnvironment & env1 = *p_env1;

  if (env1.parallel_library().mpirun_flag())
    TEST_ASSERT( false ); // This test only works for serial builds

  // Execute the environment
  data_pairs.clear();
  env1.execute();

  // retrieve the final parameter values
  const Variables& vars = env1.variables_results();

  // convergence tests: "Optimal" solution used for comparison
  // is obtained using Teuchos unit tests in
  // opt_tpl_test_exact_soln.cpp
  double rel_err;
  double target;
  double max_tol;

  target = 2.1224215765;
  max_tol = 1.e-5;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.7659069377;
  max_tol = 1.e-2;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env1.response_results();

  target = -2.4614299775;
  max_tol = 1.e-3;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}

TEUCHOS_UNIT_TEST(opt_rol,hard_nln_ineq_const_5)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   rol_ls"
    "     gradient_tolerance 1.0e-8"
    "     constraint_tolerance 1.0e-8"
    "     threshold_delta 1.0e-8"
    "     max_iterations 5"
    " variables,"
    "   continuous_design = 2"
    "     initial_point  1.8 1.0"
    " interface,"
    "   direct"
    "     analysis_driver = 'cyl_head'"
    " responses,"
    "   num_objective_functions = 1"
    "   nonlinear_inequality_constraints = 3"
    "   nonlinear_inequality_upper_bounds = 0.0 0.0 0.0"
    "   nonlinear_inequality_lower_bounds = -1.0 -1.0 -0.1 "
    "   numerical_gradients"
    "      method_source dakota"
    "      interval_type central"
    "      fd_step_size = 1.e-6"
    "   no_hessians";

  std::shared_ptr<Dakota::LibraryEnvironment> p_env1(Opt_TPL_Test::create_env(text_book_input));
  Dakota::LibraryEnvironment & env1 = *p_env1;

  if (env1.parallel_library().mpirun_flag())
    TEST_ASSERT( false ); // This test only works for serial builds

  // Execute the environment
  data_pairs.clear();
  env1.execute();

  // retrieve the final parameter values
  const Variables& vars = env1.variables_results();

  // convergence tests: "Optimal" solution used for comparison
  // is obtained using Teuchos unit tests in
  // opt_tpl_test_exact_soln.cpp
  double rel_err;
  double target;
  double max_tol;

  target = 2.1224215765;
  max_tol = 1.e-5;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.7659069377;
  max_tol = 1.e-2;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env1.response_results();

  target = -2.4614299775;
  max_tol = 1.e-3;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}
}