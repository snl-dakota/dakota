/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "JSONProblemDescDB.hpp"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace Dakota;


TEST(json_results_parser_tests, test_metadata_double)
{
  GTEST_SKIP() << "Skipping test while re-doing JSONProblemDescDB";

  JSONProblemDescDB jpdb("sample_input.json");

  try {
    // environment
    auto env_output_file  = jpdb.get_string("environment.results_output.results_output_file");
    auto env_hdf5_mod_sel = jpdb.get_bool  ("environment.results_output.hdf5.model_selection.all");
    auto env_hdf5_int_sel = jpdb.get_bool  ("environment.results_output.hdf5.interface_selection.all");
    auto env_output_prec  = jpdb.get_int   ("environment.output_precision");

    EXPECT_TRUE(env_hdf5_mod_sel);
    EXPECT_TRUE(env_hdf5_int_sel);
    EXPECT_EQ(env_output_prec, 6);
    EXPECT_EQ(env_output_file, "dakota_results");

    // method
    auto met_sample_type = jpdb.get_bool("method.sampling.sample_type.lhs");
    auto met_seed        = jpdb.get_int ("method.sampling.seed");
    auto met_samples     = jpdb.get_int ("method.sampling.samples");
    EXPECT_TRUE(met_sample_type);
    EXPECT_EQ(met_seed, 1337);
    EXPECT_EQ(met_samples, 100);

    // variables
    auto var_active_all        = jpdb.get_bool("variables.active.all");
    auto var_uniform_count     = jpdb.get_int ("variables.uniform_uncertain.count");
    auto var_uniform_descs     = jpdb.get_sa  ("variables.uniform_uncertain.descriptors");
    auto var_uniform_lb        = jpdb.get_rv  ("variables.uniform_uncertain.lower_bounds");
    auto var_uniform_ub        = jpdb.get_rv  ("variables.uniform_uncertain.upper_bounds");
    auto var_continuous_count  = jpdb.get_int ("variables.continuous_design.count");
    auto var_continuous_descs  = jpdb.get_sa  ("variables.continuous_design.descriptors");
    auto var_continuous_lb     = jpdb.get_rv  ("variables.continuous_design.lower_bounds");
    auto var_continuous_ub     = jpdb.get_rv  ("variables.continuous_design.upper_bounds");

    EXPECT_TRUE(var_active_all);
    EXPECT_EQ(var_uniform_count, 2);
    EXPECT_EQ(var_uniform_descs[0], "x2"); EXPECT_EQ(var_uniform_descs[1], "x3");
    EXPECT_EQ(var_uniform_lb[0],    -1.0); EXPECT_EQ(var_uniform_lb[1],    -1.0);
    EXPECT_EQ(var_uniform_ub[0],     1.0); EXPECT_EQ(var_uniform_ub[1],     1.0);
    EXPECT_EQ(var_continuous_count, 1);
    EXPECT_EQ(var_continuous_descs[0], "x1");
    EXPECT_EQ(var_continuous_lb[0], 0.0);
    EXPECT_EQ(var_continuous_ub[0], 1.0);

    // reponses
    auto res_count     = jpdb.get_int ("responses.response_type.response_functions.count");
    auto res_desc      = jpdb.get_sa  ("responses.descriptors");
    auto res_grad_type = jpdb.get_bool("responses.gradient_type.no_gradients");
    auto res_hess_type = jpdb.get_bool("responses.hessian_type.no_hessians");

    EXPECT_EQ(res_count, 1);
    EXPECT_EQ(res_desc[0], "f");
    EXPECT_TRUE(res_grad_type);
    EXPECT_TRUE(res_hess_type);

    // interface ??

  } catch (const json::exception& e) {
    std::cerr << "JSON error: " << e.what() << std::endl;
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
