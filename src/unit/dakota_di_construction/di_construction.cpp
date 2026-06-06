#include "DakotaVariables.hpp"
#include "DakotaResponse.hpp"
#include "ForkApplicInterface.hpp"
#include "InstructionMaterializer.hpp"
#include "MPIManager.hpp"
#include "NonDLHSSampling.hpp"
#include "OutputManager.hpp"
#include "ParallelLibrary.hpp"
#include "ProgramOptions.hpp"
#include "SimulationModel.hpp"
#include "WorkdirHelper.hpp"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <memory>
#include <stdexcept>

using json = nlohmann::json;

namespace Dakota {
namespace {

struct ExplicitRuntime {
  ExplicitRuntime():
    mpiManager(),
    programOptions(mpiManager.world_rank()),
    outputManager(std::make_shared<OutputManager>(
      programOptions, mpiManager.world_rank(), mpiManager.mpirun_flag())),
    parallelLibrary(std::make_shared<ParallelLibrary>(
      mpiManager, programOptions, *outputManager))
  {
    WorkdirHelper::initialize();
    outputManager->push_output_tag("", programOptions, false, true);
  }

  ~ExplicitRuntime()
  {
    outputManager->pop_output_tag();
  }

  MPIManager mpiManager;
  ProgramOptions programOptions;
  std::shared_ptr<OutputManager> outputManager;
  std::shared_ptr<ParallelLibrary> parallelLibrary;
};

void materialize_pilot_blocks(InstructionMaterializer& materializer,
                              IRStore& method_store,
                              IRStore& variables_store,
                              IRStore& responses_store,
                              IRStore& interface_store,
                              IRStore& model_store)
{
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

  method_store = materializer.materialize_block(method_json, irgen::BlockType::Method);
  variables_store = materializer.materialize_block(variables_json, irgen::BlockType::Variables);
  responses_store = materializer.materialize_block(responses_json, irgen::BlockType::Responses);
  interface_store = materializer.materialize_block(interface_json, irgen::BlockType::Interface);
  model_store = materializer.materialize_block(model_json, irgen::BlockType::Model);
}

TEST(di_construction_tests, can_construct_pilot_components_from_irstores_without_explicit_services)
{
  InstructionMaterializer materializer;
  IRStore method_store, variables_store, responses_store, interface_store, model_store;
  materialize_pilot_blocks(materializer, method_store, variables_store,
                           responses_store, interface_store, model_store);

  Variables variables(variables_store);
  Response response(responses_store, variables);
  std::shared_ptr<Interface> interface = std::make_shared<ForkApplicInterface>(interface_store);
  auto model = std::make_shared<SimulationModel>(
    model_store, variables, interface, response);
  NonDLHSSampling sampling(method_store, model);

  EXPECT_EQ(variables.tv(), 2);
  EXPECT_EQ(response.num_functions(), 1);
  EXPECT_EQ(model->current_response().num_functions(), 1);
  EXPECT_EQ(sampling.sampling_scheme(), SUBMETHOD_LHS);
}

TEST(di_construction_tests, can_construct_pilot_components_from_irstores_with_explicit_services)
{
  InstructionMaterializer materializer;
  IRStore method_store, variables_store, responses_store, interface_store, model_store;
  materialize_pilot_blocks(materializer, method_store, variables_store,
                           responses_store, interface_store, model_store);

  ExplicitRuntime runtime;

  Variables variables(variables_store);
  Response response(responses_store, variables);
  std::shared_ptr<Interface> interface = std::make_shared<ForkApplicInterface>(
    interface_store, runtime.parallelLibrary, runtime.outputManager);
  auto model = std::make_shared<SimulationModel>(
    model_store, variables, interface, response,
    runtime.parallelLibrary, runtime.outputManager);
  NonDLHSSampling sampling(
    method_store, model, runtime.parallelLibrary, runtime.outputManager);

  EXPECT_EQ(interface->parallel_library_ptr(), runtime.parallelLibrary.get());
  EXPECT_EQ(model->parallel_library_ptr(), runtime.parallelLibrary.get());
  EXPECT_EQ(model->output_manager_ptr(), runtime.outputManager.get());
  EXPECT_EQ(sampling.parallel_library_ptr(), runtime.parallelLibrary.get());
}

TEST(di_construction_tests, throws_on_inconsistent_parent_child_runtime_services)
{
  InstructionMaterializer materializer;
  IRStore method_store, variables_store, responses_store, interface_store, model_store;
  materialize_pilot_blocks(materializer, method_store, variables_store,
                           responses_store, interface_store, model_store);

  ExplicitRuntime runtime_a;
  ExplicitRuntime runtime_b;

  Variables variables(variables_store);
  Response response(responses_store, variables);
  auto interface = std::make_shared<ForkApplicInterface>(
    interface_store, runtime_a.parallelLibrary, runtime_a.outputManager);

  EXPECT_THROW(
    std::make_shared<SimulationModel>(
      model_store, variables, interface, response,
      runtime_b.parallelLibrary, runtime_b.outputManager),
    std::runtime_error);
}

} // namespace
} // namespace Dakota

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
