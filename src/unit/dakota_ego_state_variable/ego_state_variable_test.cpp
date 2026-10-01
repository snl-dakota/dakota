/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "opt_tpl_test.hpp"

#include "DirectApplicInterface.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <memory>
#include <string>

using namespace Dakota;

namespace {

// Regression test for an EGO bug where merit-function (augmented Lagrangian)
// updates driven by surrogate ("liar") data, plus a missing convergence gate,
// corrupted the final best-point selection for constrained problems with a
// mix of design and (fixed) state variables.
//
// Problem: G3(n=2), X0 fixed (state) at sqrt(0.5), X1 the sole design
// variable, subject to X0^2 + X1^2 = 1. The true optimum is X1 = sqrt(0.5).

const double X0_FIXED = std::sqrt(0.5);

class G3StateVarDirectApplicInterface : public DirectApplicInterface
{
  public:
    G3StateVarDirectApplicInterface(const ProblemDescDB& problem_db,
                                     ParallelLibrary& parallel_lib):
      DirectApplicInterface(problem_db, parallel_lib)
      { numAnalysisServers = 1; }

    ~G3StateVarDirectApplicInterface() override { }

  protected:

    int derived_map_ac(const String& ac_name) override
    {
      if (multiProcAnalysisFlag) {
        Cerr << "Error: plugin serial direct fn does not support "
          << "multiprocessor analyses." << std::endl;
        abort_handler(-1);
      }

      int fail_code = 0;
      if (ac_name == "g3_state_var")
        fail_code = g3_state_var();
      else {
        Cerr << ac_name << " is not available as an analysis within "
          << "G3StateVarDirectApplicInterface." << std::endl;
        abort_handler(INTERFACE_ERROR);
      }

      if (fail_code) {
        std::string err_msg("Error evaluating plugin analysis_driver ");
        err_msg += ac_name;
        throw FunctionEvalFailure(err_msg);
      }

      return 0;
    }

    void wait_local_evaluations(PRPQueue& prp_queue) override
    {
      if (multiProcAnalysisFlag) {
        Cerr << "Error: plugin serial direct fn does not support "
          << "multiprocessor analyses." << std::endl;
        abort_handler(-1);
      }

      for (PRPQueueIter prp_iter = prp_queue.begin();
          prp_iter != prp_queue.end(); prp_iter++) {
        int fn_eval_id = prp_iter->eval_id();
        const Variables& vars = prp_iter->variables();
        const ActiveSet& set  = prp_iter->response().active_set();
        Response resp = prp_iter->response(); // shared rep

        double x0 = 0.0, x1 = 0.0;
        find_x0_x1(vars, x0, x1);

        const ShortArray& asv = set.request_vector();
        if (asv[0] & 1)
          resp.function_value_view(0) = objective(x0, x1);
        if (asv.size() > 1 && (asv[1] & 1))
          resp.function_value_view(1) = constraint(x0, x1);

        completionSet.insert(fn_eval_id);
      }
    }

    void test_local_evaluations(PRPQueue& prp_queue) override
      { wait_local_evaluations(prp_queue); }

    void set_communicators_checks(int max_eval_concurrency) override { }

  private:

    static double objective(double x0, double x1)
      { return -(x0 * x1) * 2.0; } // n=2 -> sqrt(n)^n == 2

    static double constraint(double x0, double x1)
      { return x0 * x0 + x1 * x1 - 1.0; }

    static void find_x0_x1(const Variables& vars, double& x0, double& x1)
    {
      StringMultiArrayConstView labels = vars.all_continuous_variable_labels();
      const RealVector& values = vars.all_continuous_variables();
      for (size_t i = 0; i < labels.size(); ++i) {
        if (labels[i] == "X0")
          x0 = values[i];
        else if (labels[i] == "X1")
          x1 = values[i];
      }
    }

    int g3_state_var()
    {
      double x0 = 0.0, x1 = 0.0;
      for (size_t i = 0; i < xCLabels.size(); ++i) {
        if (xCLabels[i] == "X0")
          x0 = xC[i];
        else if (xCLabels[i] == "X1")
          x1 = xC[i];
      }

      if (directFnASV[0] & 1)
        fnVals[0] = objective(x0, x1);
      if (directFnASV.size() > 1 && (directFnASV[1] & 1))
        fnVals[1] = constraint(x0, x1);

      return 0;
    }
};

void plugin_g3_state_var_interface(LibraryEnvironment& env)
{
  ParallelLibrary& parallel_lib = env.parallel_library();
  ProblemDescDB& problem_db = env.problem_description_db();

  std::shared_ptr<G3StateVarDirectApplicInterface> serial_iface(
    std::make_shared<G3StateVarDirectApplicInterface>(problem_db, parallel_lib));

  bool plugged_in = env.plugin_interface(
    "" /* any model type */, "direct", "g3_state_var", serial_iface);

  if (!plugged_in) {
    Cerr << "Error: no serial interface plugin performed.  Check "
      << "compatibility between parallel\n       configuration and "
      << "selected analysis_driver." << std::endl;
    abort_handler(-1);
  }
}

std::string ego_g3_state_input(int seed)
{
  return
    "environment \n"
    "  method_pointer 'EGO' \n"
    "method \n"
    "  id_method 'EGO' \n"
    "  efficient_global \n"
    "    seed = " + std::to_string(seed) + " \n"
    "    model_pointer 'TRUE_MODEL' \n"
    "model \n"
    "  id_model 'TRUE_MODEL' \n"
    "  single \n"
    "    interface_pointer 'INTERFACE' \n"
    "    variables_pointer 'VARIABLES' \n"
    "    responses_pointer 'RESPONSES' \n"
    "variables \n"
    "  id_variables 'VARIABLES' \n"
    "  continuous_design = 1 \n"
    "    descriptors 'X1' \n"
    "    initial_point 0.0 \n"
    "    lower_bounds  0.0 \n"
    "    upper_bounds  1.0 \n"
    "  continuous_state = 1 \n"
    "    descriptors 'X0' \n"
    "    initial_state " + std::to_string(X0_FIXED) + " \n"
    "    lower_bounds  0.0 \n"
    "    upper_bounds  1.0 \n"
    "interface \n"
    "  id_interface 'INTERFACE' \n"
    "  direct \n"
    "    analysis_driver = 'g3_state_var' \n"
    "responses \n"
    "  id_responses 'RESPONSES' \n"
    "  objective_functions = 1 \n"
    "  nonlinear_equality_constraints = 1 \n"
    "  no_gradients \n"
    "  no_hessians \n";
}

void run_ego_state_variable_case(int seed)
{
  std::shared_ptr<LibraryEnvironment> p_env(
    Opt_TPL_Test::create_env(ego_g3_state_input(seed)));
  LibraryEnvironment& env = *p_env;

  plugin_g3_state_var_interface(env);

  env.execute();

  const Variables& vars = env.variables_results();
  double best_x1 = vars.continuous_variable(0);

  EXPECT_NEAR(best_x1, X0_FIXED, 0.05 * X0_FIXED)
    << "seed=" << seed << ": best_x1=" << best_x1;
}

} // anonymous namespace

TEST(ego_state_variable_tests, seed_123456) { run_ego_state_variable_case(123456); }
TEST(ego_state_variable_tests, seed_111111) { run_ego_state_variable_case(111111); }
TEST(ego_state_variable_tests, seed_222222) { run_ego_state_variable_case(222222); }
TEST(ego_state_variable_tests, seed_333333) { run_ego_state_variable_case(333333); }

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
