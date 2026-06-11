#include "DakotaVariables.hpp"
#include "DakotaResponse.hpp"
#include "DOTOptimizer.hpp"
#include "ConcurrentMetaIterator.hpp"
#include "ForkApplicInterface.hpp"
#include "InstructionMaterializer.hpp"
#include "LibraryRuntimeSupport.hpp"
#include "MPIManager.hpp"
#include "NestedModel.hpp"
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

IRStore make_nested_model_store(const IRStore& base_model_store)
{
  IRStore nested_model_store = base_model_store;
  nested_model_store.set_value("type", String("nested"));
  return nested_model_store;
}

IRStore make_concurrent_multistart_store(InstructionMaterializer& materializer)
{
  const json method_json = {
    {"multi_start", {
      {"sub_method", {{"method_name", {{"method_name", "sampling"}}}}},
      {"starting_points", {0.1, 0.2, 0.3, 0.4}}
    }}
  };

  return materializer.materialize_block(method_json, irgen::BlockType::Method);
}

IRStore make_dot_method_store(InstructionMaterializer& materializer)
{
  const json method_json = {
    {"dot_bfgs", {
      {"max_iterations", 10},
      {"convergence_tolerance", 1.e-4},
      {"constraint_tolerance", 0.0}
    }}
  };

  return materializer.materialize_block(method_json, irgen::BlockType::Method);
}

void materialize_dot_blocks(InstructionMaterializer& materializer,
                            IRStore& method_store,
                            IRStore& variables_store,
                            IRStore& responses_store,
                            IRStore& interface_store,
                            IRStore& model_store)
{
  method_store = make_dot_method_store(materializer);

  const json variables_json = {
    {"continuous_design", {
      {"count", 2},
      {"descriptors", {"x1", "x2"}},
      {"initial_point", {0.9, 1.1}},
      {"lower_bounds", {0.5, 0.5}},
      {"upper_bounds", {5.8, 2.9}}
    }}
  };

  const json responses_json = {
    {"response_type", {{"objective_functions", {{"count", 1}}}}},
    {"descriptors", {"f"}},
    {"gradient_type", {{"analytic_gradients", true}}},
    {"hessian_type", {{"no_hessians", true}}}
  };

  const json interface_json = {
    {"analysis_drivers", {
      {"drivers", {"text_book"}},
      {"interface_type", {{"fork", json::object()}}}
    }}
  };

  const json model_json = json::object();

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

TEST(di_construction_tests, resolve_runtime_accepts_matching_multiple_dependencies)
{
  ExplicitRuntime runtime;

  auto resolved = detail::resolve_runtime(
    std::shared_ptr<ParallelLibrary>(), std::shared_ptr<OutputManager>(),
    {detail::RuntimeDependency("dep_a", runtime.parallelLibrary.get(),
                               runtime.outputManager.get()),
     detail::RuntimeDependency("dep_b", runtime.parallelLibrary.get(),
                               runtime.outputManager.get())},
    "Owner");

  EXPECT_EQ(resolved.parallelLibrary, runtime.parallelLibrary.get());
  EXPECT_EQ(resolved.outputManager, runtime.outputManager.get());
}

TEST(di_construction_tests, resolve_runtime_keeps_output_manager_independent_from_dependency_parallel_library)
{
  ExplicitRuntime runtime;

  auto resolved = detail::resolve_runtime(
    std::shared_ptr<ParallelLibrary>(), std::shared_ptr<OutputManager>(),
    {detail::RuntimeDependency("dep", runtime.parallelLibrary.get(), nullptr)},
    "Owner");

  EXPECT_EQ(resolved.parallelLibrary, runtime.parallelLibrary.get());
  EXPECT_EQ(resolved.outputManager, nullptr);
}

TEST(di_construction_tests, resolve_runtime_throws_on_conflicting_multiple_dependencies)
{
  ExplicitRuntime runtime_a;
  ExplicitRuntime runtime_b;

  EXPECT_THROW(
    detail::resolve_runtime(
      std::shared_ptr<ParallelLibrary>(), std::shared_ptr<OutputManager>(),
      {detail::RuntimeDependency("dep_a", runtime_a.parallelLibrary.get(),
                                 runtime_a.outputManager.get()),
       detail::RuntimeDependency("dep_b", runtime_b.parallelLibrary.get(),
                                 runtime_b.outputManager.get())},
      "Owner"),
    std::runtime_error);
}

TEST(di_construction_tests, can_construct_concurrent_meta_iterator_from_irstore)
{
  InstructionMaterializer materializer;
  IRStore sampling_method_store, variables_store, responses_store, interface_store, model_store;
  materialize_pilot_blocks(materializer, sampling_method_store, variables_store,
                           responses_store, interface_store, model_store);
  IRStore concurrent_method_store = make_concurrent_multistart_store(materializer);

  ExplicitRuntime runtime;

  Variables variables(variables_store);
  Response response(responses_store, variables);
  auto interface = std::make_shared<ForkApplicInterface>(
    interface_store, runtime.parallelLibrary, runtime.outputManager);
  auto simulation_model = std::make_shared<SimulationModel>(
    model_store, variables, interface, response,
    runtime.parallelLibrary, runtime.outputManager);
  auto sub_iterator = std::make_shared<NonDLHSSampling>(
    sampling_method_store, simulation_model, runtime.parallelLibrary,
    runtime.outputManager);

  ConcurrentMetaIterator concurrent_iterator(
    concurrent_method_store, sub_iterator,
    runtime.parallelLibrary, runtime.outputManager);

  EXPECT_EQ(concurrent_iterator.parallel_library_ptr(),
            runtime.parallelLibrary.get());
  EXPECT_EQ(concurrent_iterator.iterated_model().get(), simulation_model.get());
}


#ifdef HAVE_DOT
TEST(di_construction_tests, can_construct_dot_optimizer_from_irstore)
{
  InstructionMaterializer materializer;
  IRStore method_store, variables_store, responses_store, interface_store, model_store;
  materialize_dot_blocks(materializer, method_store, variables_store,
                         responses_store, interface_store, model_store);

  ExplicitRuntime runtime;

  Variables variables(variables_store);
  Response response(responses_store, variables);
  auto interface = std::make_shared<ForkApplicInterface>(
    interface_store, runtime.parallelLibrary, runtime.outputManager);
  auto simulation_model = std::make_shared<SimulationModel>(
    model_store, variables, interface, response,
    runtime.parallelLibrary, runtime.outputManager);

  DOTOptimizer optimizer(method_store, simulation_model,
                         runtime.parallelLibrary, runtime.outputManager);

  EXPECT_EQ(optimizer.parallel_library_ptr(), runtime.parallelLibrary.get());
  EXPECT_EQ(optimizer.output_manager_ptr(), runtime.outputManager.get());
  EXPECT_EQ(optimizer.iterated_model().get(), simulation_model.get());
}
#endif

TEST(di_construction_tests, can_construct_nested_model_from_irstore_without_optional_interface)
{
  InstructionMaterializer materializer;
  IRStore method_store, variables_store, responses_store, interface_store, model_store;
  materialize_pilot_blocks(materializer, method_store, variables_store,
                           responses_store, interface_store, model_store);

  ExplicitRuntime runtime;

  Variables variables(variables_store);
  Response response(responses_store, variables);
  auto interface = std::make_shared<ForkApplicInterface>(
    interface_store, runtime.parallelLibrary, runtime.outputManager);
  auto simulation_model = std::make_shared<SimulationModel>(
    model_store, variables, interface, response,
    runtime.parallelLibrary, runtime.outputManager);
  auto sub_iterator = std::make_shared<NonDLHSSampling>(
    method_store, simulation_model, runtime.parallelLibrary,
    runtime.outputManager);

  NestedModel nested_model(
    make_nested_model_store(model_store), sub_iterator, nullptr,
    variables, response, runtime.parallelLibrary, runtime.outputManager);
  Model& nested_as_model = nested_model;

  EXPECT_EQ(nested_model.parallel_library_ptr(), runtime.parallelLibrary.get());
  EXPECT_EQ(nested_model.output_manager_ptr(), runtime.outputManager.get());
  EXPECT_EQ(nested_as_model.subordinate_iterator().get(), sub_iterator.get());
  EXPECT_EQ(nested_as_model.subordinate_model().get(), simulation_model.get());
}

TEST(di_construction_tests, can_construct_nested_model_from_irstore_with_optional_interface)
{
  InstructionMaterializer materializer;
  IRStore method_store, variables_store, responses_store, interface_store, model_store;
  materialize_pilot_blocks(materializer, method_store, variables_store,
                           responses_store, interface_store, model_store);

  ExplicitRuntime runtime;

  Variables variables(variables_store);
  Response response(responses_store, variables);
  auto simulation_interface = std::make_shared<ForkApplicInterface>(
    interface_store, runtime.parallelLibrary, runtime.outputManager);
  auto optional_interface = std::make_shared<ForkApplicInterface>(
    interface_store, runtime.parallelLibrary, runtime.outputManager);
  auto simulation_model = std::make_shared<SimulationModel>(
    model_store, variables, simulation_interface, response,
    runtime.parallelLibrary, runtime.outputManager);
  auto sub_iterator = std::make_shared<NonDLHSSampling>(
    method_store, simulation_model, runtime.parallelLibrary,
    runtime.outputManager);

  NestedModel nested_model(
    make_nested_model_store(model_store), sub_iterator, optional_interface,
    variables, response, runtime.parallelLibrary, runtime.outputManager);
  Model& nested_as_model = nested_model;

  EXPECT_EQ(nested_model.parallel_library_ptr(), runtime.parallelLibrary.get());
  EXPECT_EQ(nested_as_model.subordinate_model().get(), simulation_model.get());
}

TEST(di_construction_tests, nested_model_throws_when_subiterator_has_no_model)
{
  InstructionMaterializer materializer;
  IRStore variables_store, responses_store, model_store;
  IRStore method_store, interface_store;
  materialize_pilot_blocks(materializer, method_store, variables_store,
                           responses_store, interface_store, model_store);

  Variables variables(variables_store);
  Response response(responses_store, variables);
  auto sub_iterator = std::make_shared<Iterator>();

  EXPECT_THROW(
    NestedModel(make_nested_model_store(model_store), sub_iterator, nullptr,
                variables, response),
    std::runtime_error);
}

TEST(di_construction_tests, nested_model_throws_on_inconsistent_runtime_services)
{
  InstructionMaterializer materializer;
  IRStore method_store, variables_store, responses_store, interface_store, model_store;
  materialize_pilot_blocks(materializer, method_store, variables_store,
                           responses_store, interface_store, model_store);

  ExplicitRuntime runtime_a;
  ExplicitRuntime runtime_b;

  Variables variables(variables_store);
  Response response(responses_store, variables);
  auto simulation_interface = std::make_shared<ForkApplicInterface>(
    interface_store, runtime_a.parallelLibrary, runtime_a.outputManager);
  auto optional_interface = std::make_shared<ForkApplicInterface>(
    interface_store, runtime_b.parallelLibrary, runtime_b.outputManager);
  auto simulation_model = std::make_shared<SimulationModel>(
    model_store, variables, simulation_interface, response,
    runtime_a.parallelLibrary, runtime_a.outputManager);
  auto sub_iterator = std::make_shared<NonDLHSSampling>(
    method_store, simulation_model, runtime_a.parallelLibrary,
    runtime_a.outputManager);

  EXPECT_THROW(
    NestedModel(make_nested_model_store(model_store), sub_iterator,
                optional_interface, variables, response),
    std::runtime_error);
}

} // namespace
} // namespace Dakota

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
