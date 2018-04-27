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

TEUCHOS_UNIT_TEST(opt_rol,nln_ineq_const_1)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   rol"
    "     gradient_tolerance 1.0e-8"
    "     constraint_tolerance 1.0e-8"
    "     variable_tolerance 1.0e-8"
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
    "   nonlinear_inequality_upper_bounds = 3*0.0"
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

  // convergence tests: "Optimal" solution used for comparison
  // is obtained using Teuchos unit tests in
  // opt_tpl_test_exact_soln.cpp
  double rel_err;
  double target;
  double max_tol;

  target = 2.122418832160676;
  max_tol = 1.e-12;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.768556833059435;
  max_tol = 1.e-7;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env1.response_results();

  target = -2.461031295405810;
  max_tol = 1.e-8;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}


TEUCHOS_UNIT_TEST(opt_rol,nln_ineq_const_2)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   rol"
    "     gradient_tolerance 1.0e-8"
    "     constraint_tolerance 1.0e-8"
    "     variable_tolerance 1.0e-8"
    "     max_iterations 20"
    " variables,"
    "   continuous_design = 2"
    "     initial_point  30.   40."
    " interface,"
    "   direct"
    "     analysis_driver = 'barnes'"
    " responses,"
    "   num_objective_functions = 1"
    "   nonlinear_inequality_constraints = 3"
    "   nonlinear_inequality_lower_bounds = 0.0 0.0 0.0"
    "   nonlinear_inequality_upper_bounds = 1.0 1.0 1.0"
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

  // convergence tests: "Optimal" solution used for comparison
  // is obtained using Teuchos unit tests in
  // opt_tpl_test_exact_soln.cpp
  double rel_err;
  double target;
  double max_tol;

  target = 4.9526287645e+01;
  max_tol = 1.e-9;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.9622825343e+01;
  max_tol = 1.e-10;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env1.response_results();

  target = -3.1636810417e+01;
  max_tol = 1.e-9;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}


TEUCHOS_UNIT_TEST(opt_rol,nln_ineq_const_3)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   rol"
    "     gradient_tolerance 1.0e-8"
    "     constraint_tolerance 1.0e-8"
    "     variable_tolerance 1.0e-8"
    "     max_iterations 20"
    " variables,"
    "   continuous_design = 2"
    "     initial_point  2*2.0"
    "    upper_bounds    2*4.0"
    "    lower_bounds     2*1.0"
    "    descriptors     'w' 't'"
    "        continuous_state = 4"
    "          initial_state   40000.  29.E+6  500.  1000."
    "    descriptors       'R'    'E'    'X'    'Y'"
    " interface,"
    "   direct"
    "     analysis_driver = 'cantilever'"
    " responses,"
    "   num_objective_functions = 1"
    "   nonlinear_inequality_constraints = 2"
    "   nonlinear_inequality_upper_bounds = 0.0 0.0"
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

  // convergence tests: "Optimal" solution used for comparison
  // is obtained using Teuchos unit tests in
  // opt_tpl_test_exact_soln.cpp
  double rel_err;
  double target;
  double max_tol;

  target = 2.352034071879461;
  max_tol = 1.e-9;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 3.326278485790921;
  max_tol = 1.e-8;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env1.response_results();

  target = 7.823520331139867;
  max_tol = 1.e-8;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------
/// 3D textbook problem with two active nonlinear inequality
/// constraints:
/// Since the non-linear constraints are in terms of the first two
/// variables, and due to the objective function being
/// decompose-able, the third through 6th variables are expected
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

TEUCHOS_UNIT_TEST(opt_rol,nln_ineq_const_4)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   rol"
    "     gradient_tolerance 1.0e-8"
    "     constraint_tolerance 1.0e-8"
    "     variable_tolerance 1.0e-8"
    "     max_iterations 20"
    " variables,"
    "   continuous_design = 6"
    "     initial_point  6*0.0"
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

  target = 1.0;
  // BMA: Needs review
  max_tol = 1.0e-3;
  rel_err = fabs((vars.continuous_variable(3) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  // BMA: Needs review
  max_tol = 1.0e-3;
  rel_err = fabs((vars.continuous_variable(4) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 1.0;
  // BMA: Needs review
  max_tol = 1.0e-3;
  rel_err = fabs((vars.continuous_variable(5) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env1.response_results();

  target = 3.485000875338049e-02;
  max_tol = 1.e-7;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}

//----------------------------------------------------------------
/// 3D textbook problem with 6 active nonlinear inequality
/// constraints:
/// Since every 2 non-linear constraints are in terms of two
/// variables, and due to the objective function being
/// decompose-able, the odd-numbered variables are expected
/// to have a best value of 7.048612140100710e-01 while
/// the even-numbered variables are expected to have a best value
/// of 5.936586620315040e-01 (see above for derviation)

TEUCHOS_UNIT_TEST(opt_rol,nln_ineq_const_5)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " method,"
    "   rol"
    "     gradient_tolerance 1.0e-8"
    "     constraint_tolerance 1.0e-8"
    "     variable_tolerance 1.0e-8"
    "     max_iterations 20"
    " variables,"
    "   continuous_design = 6"
    "     initial_point  6*0.0"
    " interface,"
    "   direct"
    "     analysis_driver = 'scalable_text_book'"
    " responses,"
    "   num_objective_functions = 1"
    "   nonlinear_inequality_constraints = 6"
    "   nonlinear_inequality_upper_bounds = 0.2 0.0 0.2 0.0 0.2 0.0"
    "   nonlinear_inequality_lower_bounds = 0.0 -0.2 0.0 -0.2 0.0 -0.2"
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
  max_tol = 1.e-9;
  rel_err = fabs((vars.continuous_variable(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 5.936586620315040e-01;
  max_tol = 1.e-8;
  rel_err = fabs((vars.continuous_variable(1) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 7.048612140100710e-01;
  max_tol = 1.e-9;
  rel_err = fabs((vars.continuous_variable(2) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 5.936586620315040e-01;
  max_tol = 1.e-8;
  rel_err = fabs((vars.continuous_variable(3) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 7.048612140100710e-01;
  max_tol = 1.e-9;
  rel_err = fabs((vars.continuous_variable(4) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  target = 5.936586620315040e-01;
  max_tol = 1.e-8;
  rel_err = fabs((vars.continuous_variable(5) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);

  // retrieve the final response values
  const Response& resp  = env1.response_results();

  target = 3*3.485000875338049e-02;
  max_tol = 1.e-8;
  rel_err = fabs((resp.function_value(0) - target)/target );
  TEST_COMPARE(rel_err,<, max_tol);
}
}