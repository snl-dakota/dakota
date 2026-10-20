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

json selection_study(const char* top_method = "method_b")
{
  return {
    {"environment", {{"top_method_pointer", top_method}}},
    {"method", {
      {{"sampling", {{"id_method", "method_a"},
                      {"model_pointer", "model_a"}}}},
      {{"sampling", {{"id_method", "method_b"},
                      {"model_pointer", "model_b"}}}}
    }},
    {"model", {
      {{"single", {{"id_model", "model_a"},
                    {"variables_pointer", "variables_a"},
                    {"interface_pointer", "interface_a"},
                    {"responses_pointer", "responses_a"}}}},
      {{"single", {{"id_model", "model_b"},
                    {"variables_pointer", "variables_b"},
                    {"interface_pointer", "interface_b"},
                    {"responses_pointer", "responses_b"}}}}
    }},
    {"variables", {
      {{"id_variables", "variables_a"}},
      {{"id_variables", "variables_b"}}
    }},
    {"interface", {
      {{"id_interface", "interface_a"}},
      {{"id_interface", "interface_b"}}
    }},
    {"responses", {
      {{"id_responses", "responses_a"}},
      {{"id_responses", "responses_b"}}
    }}
  };
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

TEST(problem_desc_db_dump_tests, ir_selection_follows_ids_and_model_pointers)
{
  ProblemDescDB db(1, 0);
  db.enable_json_input(selection_study());
  db.resolve_top_method();

  EXPECT_EQ(db.method_id(), "method_b");
  EXPECT_EQ(db.model_id(), "model_b");
  EXPECT_EQ(db.interface_id(), "interface_b");
  EXPECT_EQ(db.get_active_method_index(), 1);
  EXPECT_EQ(db.get_active_model_index(), 1);
  EXPECT_EQ(db.get_active_variables_index(), 1);
  EXPECT_EQ(db.get_active_interface_index(), 1);
  EXPECT_EQ(db.get_active_responses_index(), 1);
}

TEST(problem_desc_db_dump_tests, ir_selection_preserves_index_lock_semantics)
{
  ProblemDescDB db(1, 0);
  db.enable_json_input(selection_study());
  db.resolve_top_method();

  const size_t selected = db.get_db_method_node();
  ASSERT_EQ(selected, 1);
  db.set_db_method_node(2);
  EXPECT_EQ(db.get_db_method_node(), _NPOS);
  db.set_db_method_node(selected);
  EXPECT_EQ(db.get_db_method_node(), selected);
  EXPECT_EQ(db.method_id(), "method_b");
}

TEST(problem_desc_db_dump_tests, ir_selection_rejects_unknown_ids)
{
  ProblemDescDB db(1, 0);
  db.enable_json_input(selection_study());
  EXPECT_ANY_THROW(db.set_db_method_node("missing_method"));
}

TEST(problem_desc_db_dump_tests, ir_selection_uses_first_empty_id)
{
  json study = selection_study();
  study["environment"].erase("top_method_pointer");
  study["method"][0]["sampling"].erase("id_method");

  ProblemDescDB db(1, 0);
  db.enable_json_input(study);
  db.set_db_method_node("");

  EXPECT_EQ(db.get_db_method_node(), 0);
  EXPECT_TRUE(db.method_id().empty());
}

TEST(problem_desc_db_dump_tests, replacing_ir_study_replaces_active_blocks)
{
  ProblemDescDB db(1, 0);
  db.enable_json_input(selection_study());
  db.resolve_top_method();
  ASSERT_EQ(db.method_id(), "method_b");

  db.enable_json_input(selection_study("method_a"));
  db.resolve_top_method();
  EXPECT_EQ(db.method_id(), "method_a");
  EXPECT_EQ(db.model_id(), "model_a");
}

TEST(problem_desc_db_dump_tests, ir_backed_queries_respect_block_locks)
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
      (void)db.get<const String>("method.model_pointer");
    });
}

} // namespace
} // namespace Dakota

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
