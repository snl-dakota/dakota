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

#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

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

  MPIManager mpi_mgr;
  ProgramOptions prog_opts(mpi_mgr.world_rank());
  prog_opts.write_restart_file("di_construction_demo.rst");
  OutputManager output_mgr(prog_opts, mpi_mgr.world_rank(),
                           mpi_mgr.mpirun_flag());
  output_mgr.startup_message("Running Dakota DI construction demo.");
  ParallelLibrary parallel_lib(mpi_mgr, prog_opts, output_mgr);
  output_mgr.push_output_tag("", prog_opts, false, true);

  std::cout << "Constructing DI study components...\n";
  Variables variables(variables_store);
  Response response(responses_store, variables);
  auto interface = std::make_shared<ForkApplicInterface>(
    interface_store, parallel_lib);
  auto model = std::make_shared<SimulationModel>(
    model_store, variables, interface, response, parallel_lib);
  NonDLHSSampling sampling(method_store, parallel_lib, model);

  std::cout << "Running sampling study...\n";
  sampling.run();

  const auto& responses = sampling.all_responses();
  std::cout << "Completed DI study.\n";
  std::cout << "Samples evaluated: " << responses.size() << '\n';
  if (!responses.empty()) {
    const auto& first_response = responses.begin()->second;
    if (first_response.num_functions() > 0)
      std::cout << "First response value: "
                << first_response.function_value(0) << '\n';
  }

  output_mgr.pop_output_tag();

  return 0;
}
