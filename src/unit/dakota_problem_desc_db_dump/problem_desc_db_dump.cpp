/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "ProblemDescDB.hpp"
#include "ProblemDescDBDump.hpp"
#include "IRState.hpp"
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>

namespace Dakota {
namespace {

using json = nlohmann::json;

TEST(problem_desc_db_dump_tests, legacy_problem_desc_db_dump_uses_indexed_legacy_keys)
{
  ProblemDescDB db(1, 0);

  DataEnvironment env;
  env.data_rep()->topMethodPointer = "method_b";
  db.insert_node(env);

  DataMethod method_a;
  method_a.data_rep()->idMethod = "method_a";
  method_a.data_rep()->modelPointer = "model_a";
  db.insert_node(method_a);

  DataMethod method_b;
  method_b.data_rep()->idMethod = "method_b";
  method_b.data_rep()->modelPointer = "model_b";
  db.insert_node(method_b);

  DataModel model_a;
  model_a.data_rep()->idModel = "model_a";
  model_a.data_rep()->variablesPointer = "vars_a";
  db.insert_node(model_a);

  DataModel model_b;
  model_b.data_rep()->idModel = "model_b";
  model_b.data_rep()->variablesPointer = "vars_b";
  db.insert_node(model_b);

  DataVariables vars_a;
  vars_a.data_rep()->idVariables = "vars_a";
  vars_a.data_rep()->numContinuousDesVars = 2;
  db.insert_node(vars_a);

  DataVariables vars_b;
  vars_b.data_rep()->idVariables = "vars_b";
  vars_b.data_rep()->numContinuousDesVars = 4;
  db.insert_node(vars_b);

  DataInterface iface;
  iface.data_rep()->idInterface = "iface";
  iface.data_rep()->analysisDrivers = {"driver_a"};
  db.insert_node(iface);

  DataResponses responses;
  responses.data_rep()->idResponses = "resp";
  responses.data_rep()->responseLabels = {"f"};
  db.insert_node(responses);

  const auto out_path =
    std::filesystem::temp_directory_path() / "dakota_problem_desc_db_dump_legacy.json";
  db.write_json_dump(out_path.string());

  std::ifstream in(out_path);
  ASSERT_TRUE(in.good());
  json dumped = json::parse(in);

  ASSERT_EQ(dumped["_meta"]["implementation"], "legacy_data_class");
  ASSERT_EQ(dumped["_meta"]["omitted_keys"], json::array({"method.dl_solver.dlLib"}));

  const json& values = dumped["values"];
  EXPECT_EQ(values["environment.top_method_pointer"], "method_b");
  EXPECT_EQ(values["method[0].id"], "method_a");
  EXPECT_EQ(values["method[1].id"], "method_b");
  EXPECT_EQ(values["method[0].model_pointer"], "model_a");
  EXPECT_EQ(values["model[1].id"], "model_b");
  EXPECT_EQ(values["variables[0].id"], "vars_a");
  EXPECT_EQ(values["variables[1].continuous_design"], 4);
  EXPECT_EQ(values["interface[0].id"], "iface");
  EXPECT_EQ(values["responses[0].labels"], json::array({"f"}));
}

TEST(problem_desc_db_dump_tests, ir_state_dump_preserves_native_ir_keys)
{
  IRState state;
  state.environment.set_value("top_method_pointer", String("method_b"));

  state.method.resize(2);
  state.method[0].set_value("id", String("method_a"));
  state.method[0].set_value("model_pointer", String("model_a"));
  state.method[1].set_value("id", String("method_b"));
  state.method[1].set_value("model_pointer", String("model_b"));

  state.model.resize(1);
  state.model[0].set_value("id", String("model_a"));
  state.model[0].set_value("variables_pointer", String("vars_a"));

  state.variables.resize(1);
  state.variables[0].set_value("id", String("vars_a"));
  state.variables[0].set_value("continuous_design", size_t(2));

  const json dumped = dump_ir_state_json(state);
  ASSERT_EQ(dumped["_meta"]["implementation"], "ir_state");

  const json& values = dumped["values"];
  EXPECT_EQ(values["environment.top_method_pointer"], "method_b");
  EXPECT_EQ(values["method[0].id"], "method_a");
  EXPECT_EQ(values["method[1].model_pointer"], "model_b");
  EXPECT_EQ(values["model[0].variables_pointer"], "vars_a");
  EXPECT_EQ(values["variables[0].continuous_design"], 2);
  EXPECT_TRUE(values.contains("method[0].id"));
  EXPECT_FALSE(values.contains("method[0].id_method"));
}

TEST(problem_desc_db_dump_tests, problem_desc_db_dump_prefers_ir_state_when_present)
{
  ProblemDescDB db(1, 0);
  const auto input_path =
    std::filesystem::temp_directory_path() / "dakota_problem_desc_db_dump_ir_input.json";
  {
    std::ofstream input(input_path);
    ASSERT_TRUE(input.good());
    input << "{\n"
             "  \"environment\": {}\n"
             "}\n";
  }

  db.enable_json_input(input_path.string());

  const auto out_path =
    std::filesystem::temp_directory_path() / "dakota_problem_desc_db_dump_ir_preferred.json";
  db.write_json_dump(out_path.string());

  std::ifstream in(out_path);
  ASSERT_TRUE(in.good());
  json dumped = json::parse(in);

  ASSERT_EQ(dumped["_meta"]["implementation"], "ir_state");
  EXPECT_EQ(dumped["_meta"]["omitted_keys"], json::array());
  EXPECT_TRUE(dumped["values"].is_object());
}

TEST(problem_desc_db_dump_tests, ir_backed_queries_still_respect_legacy_block_locks)
{
  ProblemDescDB db(1, 0);
  const auto input_path =
    std::filesystem::temp_directory_path() / "dakota_problem_desc_db_locking_input.json";
  {
    std::ofstream input(input_path);
    ASSERT_TRUE(input.good());
    input << "{\n"
             "  \"method\": [\n"
             "    {\n"
             "      \"bayes_calibration\": {\n"
             "        \"model_pointer\": \"HIERARCH\",\n"
             "        \"sub_method\": {\n"
             "          \"queso\": {\n"
             "            \"chain_samples\": 10\n"
             "          }\n"
             "        }\n"
             "      }\n"
             "    }\n"
             "  ],\n"
             "  \"model\": [\n"
             "    {\n"
             "      \"surrogate\": {\n"
             "        \"id_model\": \"HIERARCH\"\n"
             "      }\n"
             "    }\n"
             "  ]\n"
             "}\n";
  }

  db.enable_json_input(input_path.string());

  EXPECT_ANY_THROW(
    {
      (void)db.get_string("method.model_pointer");
    });
}

} // namespace
} // namespace Dakota

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
