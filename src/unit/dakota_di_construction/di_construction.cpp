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
#include "Study.hpp"
#include "StudyServices.hpp"
#include "RunOptions.hpp"
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
    runOptions(std::make_shared<RunOptions>(programOptions.user_modes())),
    parallelLibrary(std::make_shared<ParallelLibrary>(
      mpiManager, programOptions, *outputManager)),
    services(std::make_shared<StudyServices>(
      parallelLibrary, outputManager, runOptions))
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
  std::shared_ptr<RunOptions> runOptions;
  std::shared_ptr<ParallelLibrary> parallelLibrary;
  std::shared_ptr<StudyServices> services;
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

TEST(di_construction_tests, can_construct_pilot_components_from_irstores_with_default_services)
{
  InstructionMaterializer materializer;
  IRStore method_store, variables_store, responses_store, interface_store, model_store;
  materialize_pilot_blocks(materializer, method_store, variables_store,
                           responses_store, interface_store, model_store);

  auto services = std::make_shared<StudyServices>();
  Variables variables(variables_store);
  Response response(responses_store, variables);
  std::shared_ptr<Interface> interface = std::make_shared<ForkApplicInterface>(
    interface_store, services);
  auto model = std::make_shared<SimulationModel>(
    model_store, variables, interface, response, services);
  NonDLHSSampling sampling(method_store, model, services);

  EXPECT_EQ(variables.tv(), 2);
  EXPECT_EQ(response.num_functions(), 1);
  EXPECT_EQ(model->current_response().num_functions(), 1);
  EXPECT_EQ(sampling.sampling_scheme(), SUBMETHOD_LHS);
}

TEST(di_construction_tests, default_study_constructs_coherent_services)
{
  Study study;

  ASSERT_TRUE(study.services());
  ASSERT_TRUE(study.parallel_library());
  ASSERT_TRUE(study.output_manager());
  ASSERT_TRUE(study.run_options());
  EXPECT_EQ(study.services()->parallel_library_ptr(),
            study.parallel_library().get());
  EXPECT_EQ(study.services()->output_manager_ptr(),
            study.output_manager().get());
  EXPECT_EQ(study.services()->run_options_ptr(),
            study.run_options().get());
}

TEST(di_construction_tests, study_config_output_precision_reaches_output_manager)
{
  const int saved_precision = Dakota::write_precision;

  StudyConfig config;
  config.output.precision = 12;
  Study study(config);

  EXPECT_EQ(study.output_manager()->write_precision(), 12);
  Dakota::write_precision = saved_precision;
}

TEST(di_construction_tests, study_config_run_defaults_to_all_phases)
{
  Study study;
  const RunOptions& run_options = *study.run_options();

  EXPECT_TRUE(run_options.preRun);
  EXPECT_TRUE(run_options.run);
  EXPECT_TRUE(run_options.postRun);
  EXPECT_FALSE(run_options.requestedUserModes);
}

TEST(di_construction_tests, study_config_run_preserves_explicit_phase_selection)
{
  StudyConfig config;
  config.run.run = true;
  config.run.preRunOutput = "pre.out";
  Study study(config);
  const RunOptions& run_options = *study.run_options();

  EXPECT_FALSE(run_options.preRun);
  EXPECT_TRUE(run_options.run);
  EXPECT_FALSE(run_options.postRun);
  EXPECT_TRUE(run_options.requestedUserModes);
  EXPECT_EQ(run_options.preRunOutput, "pre.out");
}

TEST(di_construction_tests, study_factory_constructs_fork_interface)
{
  InstructionMaterializer materializer;
  IRStore method_store, variables_store, responses_store, interface_store, model_store;
  materialize_pilot_blocks(materializer, method_store, variables_store,
                           responses_store, interface_store, model_store);

  Study study;
  auto interface = study.interface(interface_store);

  ASSERT_TRUE(interface);
  EXPECT_TRUE(std::dynamic_pointer_cast<ForkApplicInterface>(interface));
  EXPECT_EQ(interface->parallel_library_ptr(), study.parallel_library().get());
  EXPECT_EQ(interface->output_manager_ptr(), study.output_manager().get());
}

TEST(di_construction_tests, study_factories_construct_components_with_shared_services)
{
  InstructionMaterializer materializer;
  IRStore method_store, variables_store, responses_store, interface_store, model_store;
  materialize_pilot_blocks(materializer, method_store, variables_store,
                           responses_store, interface_store, model_store);

  Study study;

  Variables variables(variables_store);
  Response response(responses_store, variables);
  auto interface = study.interface(interface_store);
  auto model = study.model().simulation(model_store, variables, interface, response);
  auto sampling = study.method().sampling(method_store, model);

  ASSERT_TRUE(model);
  ASSERT_TRUE(sampling);
  EXPECT_EQ(model->parallel_library_ptr(), study.parallel_library().get());
  EXPECT_EQ(model->output_manager_ptr(), study.output_manager().get());
  EXPECT_EQ(model->run_options_ptr(), study.run_options().get());
  EXPECT_EQ(sampling->parallel_library_ptr(), study.parallel_library().get());
  EXPECT_EQ(sampling->output_manager_ptr(), study.output_manager().get());
  EXPECT_EQ(sampling->run_options_ptr(), study.run_options().get());
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
    interface_store, runtime.services);
  auto model = std::make_shared<SimulationModel>(
    model_store, variables, interface, response, runtime.services);
  NonDLHSSampling sampling(method_store, model, runtime.services);

  EXPECT_EQ(interface->parallel_library_ptr(), runtime.parallelLibrary.get());
  EXPECT_EQ(model->parallel_library_ptr(), runtime.parallelLibrary.get());
  EXPECT_EQ(model->output_manager_ptr(), runtime.outputManager.get());
  EXPECT_EQ(sampling.parallel_library_ptr(), runtime.parallelLibrary.get());
  EXPECT_EQ(sampling.run_options_ptr(), runtime.runOptions.get());
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
    interface_store, runtime_a.services);

  EXPECT_THROW(
    std::make_shared<SimulationModel>(
      model_store, variables, interface, response, runtime_b.services),
    std::runtime_error);
}

TEST(di_construction_tests, validate_services_accepts_matching_multiple_dependencies)
{
  ExplicitRuntime runtime;

  EXPECT_NO_THROW(detail::validate_services(
    "Owner", runtime.services,
    {detail::RuntimeDependency("dep_a", runtime.services.get()),
     detail::RuntimeDependency("dep_b", runtime.services.get())}));
}

TEST(di_construction_tests, validate_services_ignores_null_dependencies)
{
  ExplicitRuntime runtime;

  EXPECT_NO_THROW(detail::validate_services(
    "Owner", runtime.services,
    {detail::RuntimeDependency("dep", static_cast<const StudyServices*>(nullptr))}));
}

TEST(di_construction_tests, validate_services_throws_on_missing_owner_services)
{
  EXPECT_THROW(
    detail::validate_services("Owner", std::shared_ptr<StudyServices>()),
    std::runtime_error);
}

TEST(di_construction_tests, validate_services_throws_on_conflicting_multiple_dependencies)
{
  ExplicitRuntime runtime_a;
  ExplicitRuntime runtime_b;

  EXPECT_THROW(
    detail::validate_services(
      "Owner", runtime_a.services,
      {detail::RuntimeDependency("dep_a", runtime_a.services.get()),
       detail::RuntimeDependency("dep_b", runtime_b.services.get())}),
    std::runtime_error);
}

TEST(di_construction_tests, study_services_can_construct_owned_default_bundle)
{
  auto services = std::make_shared<StudyServices>();

  ASSERT_TRUE(services->parallel_library_ptr());
  ASSERT_TRUE(services->output_manager_ptr());
  ASSERT_TRUE(services->run_options_ptr());
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
    interface_store, runtime.services);
  auto simulation_model = std::make_shared<SimulationModel>(
    model_store, variables, interface, response, runtime.services);
  auto sub_iterator = std::make_shared<NonDLHSSampling>(
    sampling_method_store, simulation_model, runtime.services);

  ConcurrentMetaIterator concurrent_iterator(
    concurrent_method_store, sub_iterator, runtime.services);

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
    interface_store, runtime.services);
  auto simulation_model = std::make_shared<SimulationModel>(
    model_store, variables, interface, response, runtime.services);

  DOTOptimizer optimizer(method_store, simulation_model, runtime.services);

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
    interface_store, runtime.services);
  auto simulation_model = std::make_shared<SimulationModel>(
    model_store, variables, interface, response, runtime.services);
  auto sub_iterator = std::make_shared<NonDLHSSampling>(
    method_store, simulation_model, runtime.services);

  NestedModel nested_model(
    make_nested_model_store(model_store), sub_iterator, nullptr,
    variables, response, runtime.services);
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
    interface_store, runtime.services);
  auto optional_interface = std::make_shared<ForkApplicInterface>(
    interface_store, runtime.services);
  auto simulation_model = std::make_shared<SimulationModel>(
    model_store, variables, simulation_interface, response, runtime.services);
  auto sub_iterator = std::make_shared<NonDLHSSampling>(
    method_store, simulation_model, runtime.services);

  NestedModel nested_model(
    make_nested_model_store(model_store), sub_iterator, optional_interface,
    variables, response, runtime.services);
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
                variables, response, std::make_shared<StudyServices>()),
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
    interface_store, runtime_a.services);
  auto optional_interface = std::make_shared<ForkApplicInterface>(
    interface_store, runtime_b.services);
  auto simulation_model = std::make_shared<SimulationModel>(
    model_store, variables, simulation_interface, response, runtime_a.services);
  auto sub_iterator = std::make_shared<NonDLHSSampling>(
    method_store, simulation_model, runtime_a.services);

  EXPECT_THROW(
    NestedModel(make_nested_model_store(model_store), sub_iterator,
                optional_interface, variables, response, runtime_a.services),
    std::runtime_error);
}

} // namespace
} // namespace Dakota

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
