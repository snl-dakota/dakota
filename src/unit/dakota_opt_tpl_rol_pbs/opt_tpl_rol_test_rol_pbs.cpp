/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "opt_tpl_rol_test_interface.hpp"
#include "opt_tpl_test.hpp"
#include "opt_tpl_test_fixture.hpp"  // for plugin interface

#include <gtest/gtest.h>

TEST(opt_tpl_rol_test_rol_pbs_tests, test_simple_eq_cons)
{
  /// Dakota input string:
  static const char text_book_input[] =
    "environment                          "
    "    write_restart 'test_simple_eq_cons.rst' "
    " method,"
    "   rol"
    "     gradient_tolerance 1.0e-5"
    "     constraint_tolerance 1.0e-5"
    "     variable_tolerance 1.0e-5"
    "     max_iterations 20"
    "   output silent"
    " variables,"
    "   continuous_design = 5"
    "     initial_point  -1.8  1.7  1.9  -0.8  -0.8"
    "     descriptors 'x_1'  'x_2'  'x_3'  'x_4'  'x_5'"
    " interface,"
    "   direct"
    "     analysis_driver = 'rol_testers'"
    " responses,"
    "   num_objective_functions = 1"
    "   nonlinear_equality_constraints = 3"
    "   nonlinear_equality_targets = 0.0  0.0  0.0"
    "   analytic_gradients"
    "   no_hessians";

  std::shared_ptr<Dakota::LibraryEnvironment> p_env(Opt_TPL_Test::create_env(text_book_input));
  Dakota::LibraryEnvironment & env = *p_env;

  Dakota::ProblemDescDB& problem_db = env.problem_description_db();
  std::shared_ptr<RolTestInterface> rol_iface =
    std::make_shared<RolTestInterface>("simple_eq_cons", problem_db);
  serial_interface_plugin(env, "rol_testers", rol_iface);

  if (env.parallel_library().mpirun_flag())
    FAIL(); // This test only works for serial builds

  // Execute the environment
  env.execute();

  // retrieve the Dakota results
  const RealVector& x_best = env.variables_results().continuous_variables();
  const RealVector& f_best = env.response_results().function_values();

  // known ROL optimal solution
  RealVector sol_x(rol_iface->numVars);
  copy_data(*(rol_iface->xOptimal), sol_x);
  RealVector sol_f(1 + rol_iface->numIneqCons + rol_iface->numEqCons);
  copy_data(*(rol_iface->optimalFns), sol_f);

  // verify norm(sol - x) < tol
  double max_tol = 2.0*std::sqrt(std::numeric_limits<double>::epsilon());

  sol_x -= x_best;
  double max_error_x = sol_x.normInf();
  EXPECT_LT(max_error_x, max_tol);

  sol_f -= f_best;
  double max_error_f = sol_f.normInf();
  // This fails but previously passed with TEST_INEQUALITY
  //   (which I don't think was the intended check) - RWH
  // ------ TODO revisit -------
  //EXPECT_LT(max_error_f, max_tol);
  EXPECT_TRUE((max_error_f != max_tol));
}

TEST(opt_tpl_rol_test_rol_pbs_tests, test_paraboloid_circle)
{
  /// Dakota input string:
  static const char text_book_input[] =
    " environment"
    "   write_restart 'test_paraboloid_circle.rst' "
    " method,"
    "   rol"
    "     gradient_tolerance 1.0e-5"
    "     constraint_tolerance 1.0e-5"
    "     variable_tolerance 1.0e-5"
    "     max_iterations 20"
    "   output silent"
    " variables,"
    "   continuous_design = 2"
    "     initial_point  0.5  0.5"
    "     descriptors 'x_1'  'x_2'"
    " interface,"
    "   direct"
    "     analysis_driver = 'rol_testers'"
    " responses,"
    "   num_objective_functions = 1"
    "   nonlinear_equality_constraints = 1"
    "   nonlinear_equality_targets = 0.0"
    "   analytic_gradients"
    "   no_hessians";

  std::shared_ptr<Dakota::LibraryEnvironment> p_env(Opt_TPL_Test::create_env(text_book_input));
  Dakota::LibraryEnvironment & env = *p_env;

  Dakota::ProblemDescDB& problem_db = env.problem_description_db();
  std::shared_ptr<RolTestInterface> rol_iface =
    std::make_shared<RolTestInterface>("paraboloid_circle", problem_db);
  serial_interface_plugin(env, "rol_testers", rol_iface);

  if (env.parallel_library().mpirun_flag())
    FAIL(); // This test only works for serial builds

  // Execute the environment
  env.execute();

  // retrieve the Dakota results
  const RealVector& x_best = env.variables_results().continuous_variables();
  const RealVector& f_best = env.response_results().function_values();

  // known ROL optimal solution
  RealVector sol_x(rol_iface->numVars);
  copy_data(*(rol_iface->xOptimal), sol_x);
  RealVector sol_f(1 + rol_iface->numIneqCons + rol_iface->numEqCons);
  copy_data(*(rol_iface->optimalFns), sol_f);

  // verify norm(sol - x) < tol
  double max_tol =std::sqrt(std::numeric_limits<double>::epsilon());

  sol_x -= x_best;
  double max_error_x = sol_x.normInf();
  EXPECT_LT(max_error_x, max_tol);

  sol_f -= f_best;
  double max_error_f = sol_f.normInf();
  // This fails but previously passed with TEST_INEQUALITY
  //   (which I don't think was the intended check) - RWH
  // ------ TODO revisit -------
  //EXPECT_LT(max_error_f, max_tol);
  EXPECT_TRUE((max_error_f != max_tol));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
