#include "DakotaVariables.hpp"
#include "DakotaResponse.hpp"
#include "InstructionMaterializer.hpp"
#include "NonDLHSSampling.hpp"
#include "SimulationModel.hpp"
#include "Study.hpp"
#include "StudyConfig.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <sys/stat.h>
#include <nlohmann/json.hpp>

#ifdef DAKOTA_HAVE_MPI
#include <mpi.h>
#endif

using json = nlohmann::json;

#ifdef DAKOTA_HAVE_MPI
namespace {

const char driver_path[] = "./di_mpi_communicator_driver.sh";

void write_demo_driver()
{
  std::ofstream driver(driver_path);
  if (!driver)
    throw std::runtime_error("Could not create DI MPI communicator demo driver.");

  driver << "#!/bin/sh\n"
         << "results_file=\"$2\"\n"
         << "if [ -z \"$results_file\" ]; then results_file=results.out; fi\n"
         << "printf '1.0 f\\n' > \"$results_file\"\n";
  driver.close();

  if (chmod(driver_path, 0755) != 0)
    throw std::runtime_error("Could not mark DI MPI communicator demo driver executable.");
}

} // namespace
#endif

int main(int argc, char** argv)
{
#ifndef DAKOTA_HAVE_MPI
  std::cerr << "di_mpi_communicator_demo requires an MPI-enabled Dakota build.\n";
  return 1;
#else
  MPI_Init(&argc, &argv);

  int world_rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if (world_rank == 0)
    write_demo_driver();
  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Comm dakota_comm = MPI_COMM_NULL;
  MPI_Comm_dup(MPI_COMM_WORLD, &dakota_comm);

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
        {"drivers", {driver_path}},
        {"interface_type", {{"fork", {
          {"parameters_file", "params.in"},
          {"results_file", "results.out"},
          {"file_save", true},
          {"file_tag", true}
        }}}},
        {"deactivate", {{"restart_file", true}}}
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

    StudyConfig config;
    config.output.precision = 12;
    config.output.outputFile = "di_mpi_communicator_demo.out";
    config.output.errorFile = "di_mpi_communicator_demo.err";
    config.output.writeRestart = "di_mpi_communicator_demo.rst";

    Study study(dakota_comm, config);

    if (world_rank == 0)
      std::cout << "Constructing DI study on caller-supplied MPI_Comm...\n";

    Variables variables(variables_store);
    Response response(responses_store, variables);
    auto interface = study.interface(interface_store);
    auto model = study.model().simulation(
      model_store, variables, interface, response);
    auto sampling = study.method().sampling(method_store, model);

    if (world_rank == 0)
      std::cout << "Running sampling study through Study(MPI_Comm, config)...\n";
    study.run(sampling);

    if (world_rank == 0) {
      const auto& responses = sampling->all_responses();
      std::cout << "Completed DI MPI communicator study.\n";
      std::cout << "Samples evaluated: " << responses.size() << '\n';
    }
  }

  MPI_Comm_free(&dakota_comm);
  MPI_Finalize();
  return 0;
#endif
}
