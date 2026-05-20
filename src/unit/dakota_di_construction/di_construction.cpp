#include "DakotaVariables.hpp"
#include "DakotaResponse.hpp"
#include "ForkApplicInterface.hpp"
#include "InstructionMaterializer.hpp"
#include "NonDLHSSampling.hpp"
#include "ParallelLibrary.hpp"
#include "SimulationModel.hpp"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

namespace Dakota {
namespace {

TEST(di_construction_tests, can_construct_pilot_components_from_irstores)
{
  InstructionMaterializer materializer;

  const json method_json = {
    {"sampling", {
      {"sample_type", {{"lhs", true}}},
      {"samples", 10},
      {"seed", 1234}
    }}
  };

  const json variables_json = {
    {"active", {{"all", true}}},
    {"uniform_uncertain", {
      {"count", 2},
      {"descriptors", {"x1", "x2"}},
      {"lower_bounds", {0.0, 0.0}},
      {"upper_bounds", {1.0, 1.0}}
    }}
  };

  const json responses_json = {
    {"response_type", {{"response_functions", {{"count", 1}}}}},
    {"descriptors", {"f"}},
    {"gradient_type", {{"no_gradients", true}}},
    {"hessian_type", {{"no_hessians", true}}}
  };

  const json interface_json = {
    {"analysis_drivers", {
      {"drivers", {"text_book"}},
      {"interface_type", {{"fork", json::object()}}}
    }}
  };

  const json model_json = json::object();

  const IRStore method_store =
    materializer.materialize_block(method_json, irgen::BlockType::Method);
  const IRStore variables_store =
    materializer.materialize_block(variables_json, irgen::BlockType::Variables);
  const IRStore responses_store =
    materializer.materialize_block(responses_json, irgen::BlockType::Responses);
  const IRStore interface_store =
    materializer.materialize_block(interface_json, irgen::BlockType::Interface);
  const IRStore model_store =
    materializer.materialize_block(model_json, irgen::BlockType::Model);

  ASSERT_TRUE(method_store.contains("samples"));
  EXPECT_EQ(method_store.get<int>("samples"), 10);
  ASSERT_TRUE(method_store.contains("sample_type"));
  EXPECT_EQ(method_store.get<unsigned short>("sample_type"), SUBMETHOD_LHS);

  ASSERT_TRUE(variables_store.contains("total"));
  EXPECT_EQ(variables_store.get<size_t>("total"), 2U);

  ASSERT_TRUE(responses_store.contains("num_response_functions"));
  EXPECT_EQ(responses_store.get<size_t>("num_response_functions"), 1U);

  ASSERT_TRUE(interface_store.contains("application.analysis_drivers"));
  ASSERT_FALSE(interface_store.get<StringArray>("application.analysis_drivers").empty());
  EXPECT_EQ(interface_store.get<StringArray>("application.analysis_drivers")[0], "text_book");

  ParallelLibrary parallel_lib;

  std::cerr << "[di] constructing Variables\n";
  Variables variables(variables_store);
  EXPECT_EQ(variables.tv(), 2);

  std::cerr << "[di] constructing Response\n";
  Response response(responses_store, variables);
  EXPECT_EQ(response.num_functions(), 1);
  EXPECT_EQ(response.shared_data().num_functions(), 1);
  EXPECT_EQ(response.shared_data().num_scalar_responses(), 1);
  EXPECT_EQ(response.shared_data().num_scalar_primary(), 1);
  EXPECT_EQ(response.shared_data().function_labels().size(), 1);
  if (!response.shared_data().function_labels().empty())
    EXPECT_EQ(response.shared_data().function_labels()[0], "f");

  std::cerr << "[di] constructing ForkApplicInterface\n";
  auto interface = std::make_shared<ForkApplicInterface>(
    interface_store, response, parallel_lib);
  ASSERT_NE(interface, nullptr);

  std::cerr << "[di] constructing SimulationModel\n";
  auto model = std::make_shared<SimulationModel>(
    model_store, variables_store, variables, interface, response, parallel_lib);
  EXPECT_EQ(model->current_response().num_functions(), 1);
  EXPECT_EQ(model->current_variables().tv(), 2);
  EXPECT_EQ(model->current_response().shared_data().num_functions(), 1);

  std::cerr << "[di] constructing NonDLHSSampling\n";
  NonDLHSSampling sampling(method_store, parallel_lib, model);
  EXPECT_EQ(sampling.sampling_scheme(), SUBMETHOD_LHS);
  std::cerr << "[di] completed test\n";
}

} // namespace
} // namespace Dakota

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
