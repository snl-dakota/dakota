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
#include "StudyRuntime.hpp"
#include "WorkdirHelper.hpp"

#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace {

struct ExplicitRuntime {
  ExplicitRuntime():
    mpiManager(),
    programOptions(mpiManager.world_rank()),
    outputManager(std::make_shared<Dakota::OutputManager>(
      programOptions, mpiManager.world_rank(), mpiManager.mpirun_flag())),
    parallelLibrary(std::make_shared<Dakota::ParallelLibrary>(
      mpiManager, programOptions, *outputManager)),
    studyRuntime(std::make_shared<Dakota::StudyRuntime>(
      *parallelLibrary, outputManager.get()))
  {
    // Explicitly mirror the Environment-owned setup that library-mode callers
    // currently need for driver PATH handling and restart/output activation.
    Dakota::WorkdirHelper::initialize();
    outputManager->push_output_tag("", programOptions, false, true);
  }

  ~ExplicitRuntime()
  {
    outputManager->pop_output_tag();
  }

  Dakota::MPIManager mpiManager;
  Dakota::ProgramOptions programOptions;
  std::shared_ptr<Dakota::OutputManager> outputManager;
  std::shared_ptr<Dakota::ParallelLibrary> parallelLibrary;
  std::shared_ptr<Dakota::StudyRuntime> studyRuntime;
};

} // namespace

int main()
{
  using namespace Dakota;

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
      {"interface_type",
        {
          {
            "fork", {
              {"parameters_file", "params.in"},
              {"results_file", "results.out"},
              {"file_save", true}
            }
          }
        }
      },
      {"deactivate", {"restart_file", true}},

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

  // This explicit service setup demonstrates the current library-mode knobs:
  // ParallelLibrary coordinates evaluation execution, and OutputManager owns
  // restart/output state needed by the pilot path.
  ExplicitRuntime runtime;

  std::cout << "Constructing DI study components...\n";
  Variables variables(variables_store);
  Response response(responses_store, variables);
  auto interface = std::make_shared<ForkApplicInterface>(
    interface_store, runtime.parallelLibrary, runtime.outputManager);
  auto model = std::make_shared<SimulationModel>(
    model_store, variables, interface, response,
    runtime.parallelLibrary, runtime.outputManager);
  NonDLHSSampling sampling(
    method_store, model, runtime.parallelLibrary, runtime.outputManager);

  std::cout << "Running sampling study...\n";
  runtime.studyRuntime->execute_iterator(sampling);

  const auto& responses = sampling.all_responses();
  std::cout << "Completed DI study.\n";
  std::cout << "Samples evaluated: " << responses.size() << '\n';
  if (!responses.empty()) {
    const auto& first_response = responses.begin()->second;
    if (first_response.num_functions() > 0)
      std::cout << "First response value: "
                << first_response.function_value(0) << '\n';
  }

  return 0;
}
