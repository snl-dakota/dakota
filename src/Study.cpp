/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "Study.hpp"

#include "ConcurrentMetaIterator.hpp"
#include "DOTOptimizer.hpp"
#include "DakotaInterface.hpp"
#include "DakotaInterfaceEnums.hpp"
#include "DakotaIterator.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "DakotaVariables.hpp"
#ifndef _WIN32
#include "ForkApplicInterface.hpp"
#else
#include "SpawnApplicInterface.hpp"
#endif
#include "IRState.hpp"
#include "MPIManager.hpp"
#include "NestedModel.hpp"
#include "InstructionMaterializer.hpp"
#include "NonDLHSSampling.hpp"
#include "OutputManager.hpp"
#include "ParallelLibrary.hpp"
#include "ProgramOptions.hpp"
#include "RunOptions.hpp"
#include "SimulationModel.hpp"
#include "StudyRuntime.hpp"
#include "StudyServices.hpp"
#include "WorkdirHelper.hpp"
#include "dakota_input_reader.hpp"
#include "dakota_global_defs.hpp"

#include <stdexcept>
#include <nlohmann/json.hpp>
#include <utility>

namespace Dakota {
namespace {

void apply_output_config_to_program_options(
  ProgramOptions& program_options, const StudyOutputConfig& output_config)
{
  if (!output_config.outputFile.empty())
    program_options.output_file(output_config.outputFile);
  if (!output_config.errorFile.empty())
    program_options.error_file(output_config.errorFile);
  if (!output_config.readRestart.empty())
    program_options.read_restart_file(output_config.readRestart);
  if (output_config.stopRestart > 0)
    program_options.stop_restart_evals(output_config.stopRestart);
  if (!output_config.writeRestart.empty())
    program_options.write_restart_file(output_config.writeRestart);
}

RunOptions make_run_options(const StudyRunConfig& run_config)
{
  RunOptions run_options;
  run_options.preRun = run_config.preRun;
  run_options.run = run_config.run;
  run_options.postRun = run_config.postRun;

  if (!run_options.preRun && !run_options.run && !run_options.postRun) {
    run_options.preRun = true;
    run_options.run = true;
    run_options.postRun = true;
    run_options.requestedUserModes = false;
  }
  else
    run_options.requestedUserModes = true;

  run_options.preRunInput = run_config.preRunInput;
  run_options.preRunOutput = run_config.preRunOutput;
  run_options.runInput = run_config.runInput;
  run_options.runOutput = run_config.runOutput;
  run_options.postRunInput = run_config.postRunInput;
  run_options.postRunOutput = run_config.postRunOutput;
  run_options.preRunOutputFormat = run_config.preRunOutputFormat;
  run_options.postRunInputFormat = run_config.postRunInputFormat;

  if (run_options.preRun && !run_options.run && run_options.postRun)
    throw std::runtime_error(
      "StudyConfig run phase 'run' is required when both pre_run and "
      "post_run are enabled.");

  return run_options;
}

void apply_run_options_to_program_options(ProgramOptions& program_options,
                                          const RunOptions& run_options)
{
  program_options.pre_run(run_options.preRun);
  program_options.run(run_options.run);
  program_options.post_run(run_options.postRun);
  program_options.pre_run_input(run_options.preRunInput);
  program_options.pre_run_output(run_options.preRunOutput);
  program_options.run_input(run_options.runInput);
  program_options.run_output(run_options.runOutput);
  program_options.post_run_input(run_options.postRunInput);
  program_options.post_run_output(run_options.postRunOutput);
}

} // namespace

namespace {

using json = nlohmann::json;

IRStore materialize_validated_block(const json& validated_block_json,
                                    irgen::BlockType block_type)
{
  return InstructionMaterializer().materialize_block(validated_block_json, block_type);
}

IRStore validate_and_materialize_variables(const json& variables_json)
{
  return materialize_validated_block(
    dakota::validate_variables_block_json_to_json(variables_json),
    irgen::BlockType::Variables);
}

IRStore validate_and_materialize_responses(const json& responses_json)
{
  return materialize_validated_block(
    dakota::validate_responses_block_json_to_json(responses_json),
    irgen::BlockType::Responses);
}

IRStore validate_and_materialize_interface(const json& interface_json)
{
  return materialize_validated_block(
    dakota::validate_interface_block_json_to_json(interface_json),
    irgen::BlockType::Interface);
}

IRStore validate_and_materialize_selected_method(const json& method_json,
                                                 const char* selector)
{
  json wrapped = json::object();
  wrapped[selector] = method_json;
  return materialize_validated_block(
    dakota::validate_method_block_json_to_json(wrapped),
    irgen::BlockType::Method);
}

IRStore validate_and_materialize_selected_model(const json& model_json,
                                                const char* selector)
{
  json wrapped = json::object();
  wrapped[selector] = model_json;
  return materialize_validated_block(
    dakota::validate_model_block_json_to_json(wrapped),
    irgen::BlockType::Model);
}

} // namespace

Study::Study(const StudyConfig& config):
  Study(std::make_shared<MPIManager>(), config)
{ }


#ifdef DAKOTA_HAVE_MPI
Study::Study(MPI_Comm dakota_mpi_comm, const StudyConfig& config):
  Study(std::make_shared<MPIManager>(dakota_mpi_comm), config)
{ }
#endif


Study::Study(std::shared_ptr<MPIManager> mpi_manager,
             const StudyConfig& config):
  mpiManager(std::move(mpi_manager)),
  programOptions(std::make_shared<ProgramOptions>(mpiManager->world_rank()))
{
  WorkdirHelper::initialize();

  apply_output_config_to_program_options(*programOptions, config.output);
  runOptions = std::make_shared<RunOptions>(make_run_options(config.run));
  apply_run_options_to_program_options(*programOptions, *runOptions);

  outputManager = std::make_shared<OutputManager>(
    *programOptions, mpiManager->world_rank(), mpiManager->mpirun_flag());
  outputManager->apply(config.output);

  parallelLibrary = std::make_shared<ParallelLibrary>(
    *mpiManager, *programOptions, *outputManager);
  studyServices = std::make_shared<StudyServices>(
    parallelLibrary, outputManager, runOptions);
  studyRuntime = std::make_shared<StudyRuntime>(
    *parallelLibrary, outputManager.get());

  parallelLibrary->push_output_tag(*parallelLibrary->w_parallel_level_iterator());
}

Study::~Study()
{
  if (parallelLibrary)
    parallelLibrary->pop_output_tag(*parallelLibrary->w_parallel_level_iterator());
}

std::shared_ptr<StudyServices> Study::services() const
{ return studyServices; }

std::shared_ptr<ParallelLibrary> Study::parallel_library() const
{ return parallelLibrary; }

std::shared_ptr<OutputManager> Study::output_manager() const
{ return outputManager; }

std::shared_ptr<RunOptions> Study::run_options() const
{ return runOptions; }

Variables Study::variables(const nlohmann::json& variables_json) const
{
  return Variables(validate_and_materialize_variables(variables_json));
}

Response Study::responses(const nlohmann::json& responses_json,
                          const Variables& variables) const
{
  return Response(validate_and_materialize_responses(responses_json), variables);
}

std::shared_ptr<Interface> Study::interface(const IRStore& interface_store) const
{
  const unsigned short interface_type =
    interface_store.get<unsigned short>("type");

  if (interface_type == FORK_INTERFACE) {
#ifndef _WIN32
    return std::make_shared<ForkApplicInterface>(
      interface_store, studyServices);
#else
    return std::make_shared<SpawnApplicInterface>(
      interface_store, studyServices);
#endif
  }

  throw std::runtime_error(
    "Study::interface currently supports only fork interfaces.");
}

std::shared_ptr<Interface> Study::interface(const nlohmann::json& interface_json) const
{
  return interface(validate_and_materialize_interface(interface_json));
}

Study::MethodFactory Study::method() const
{ return MethodFactory(*this); }

Study::ModelFactory Study::model() const
{ return ModelFactory(*this); }

void Study::run(Iterator& iterator) const
{ studyRuntime->execute_iterator(iterator); }

void Study::run(const std::shared_ptr<Iterator>& iterator) const
{
  if (!iterator)
    throw std::runtime_error("Study::run requires a non-null iterator.");
  run(*iterator);
}

Study::MethodFactory::MethodFactory(const Study& study_ref):
  study(study_ref)
{ }

std::shared_ptr<NonDLHSSampling>
Study::MethodFactory::sampling(const IRStore& method_store,
                               std::shared_ptr<Model> model) const
{
  return std::make_shared<NonDLHSSampling>(
    method_store, std::move(model), study.services());
}

std::shared_ptr<NonDLHSSampling>
Study::MethodFactory::sampling(const nlohmann::json& method_json,
                               std::shared_ptr<Model> model) const
{
  return sampling(validate_and_materialize_selected_method(method_json, "sampling"),
                  std::move(model));
}

std::shared_ptr<DOTOptimizer>
Study::MethodFactory::dot_bfgs(const IRStore& method_store,
                               std::shared_ptr<Model> model) const
{
#ifdef HAVE_DOT
  return std::make_shared<DOTOptimizer>(
    method_store, std::move(model), study.services());
#else
  (void)method_store;
  (void)model;
  throw std::runtime_error("Study::method().dot_bfgs requires Dakota to be built with DOT.");
#endif
}

std::shared_ptr<DOTOptimizer>
Study::MethodFactory::dot_bfgs(const nlohmann::json& method_json,
                               std::shared_ptr<Model> model) const
{
  return dot_bfgs(validate_and_materialize_selected_method(method_json, "dot_bfgs"),
                  std::move(model));
}

std::shared_ptr<ConcurrentMetaIterator>
Study::MethodFactory::multi_start(
  const IRStore& method_store, std::shared_ptr<Iterator> sub_iterator) const
{
  return std::make_shared<ConcurrentMetaIterator>(
    method_store, std::move(sub_iterator), study.services());
}

std::shared_ptr<ConcurrentMetaIterator>
Study::MethodFactory::multi_start(
  const nlohmann::json& method_json, std::shared_ptr<Iterator> sub_iterator) const
{
  return multi_start(
    validate_and_materialize_selected_method(method_json, "multi_start"),
    std::move(sub_iterator));
}

Study::ModelFactory::ModelFactory(const Study& study_ref):
  study(study_ref)
{ }

std::shared_ptr<SimulationModel>
Study::ModelFactory::simulation(const IRStore& model_store,
                                const Variables& variables,
                                std::shared_ptr<Interface> interface,
                                const Response& response) const
{
  return std::make_shared<SimulationModel>(
    model_store, variables, std::move(interface), response, study.services());
}

std::shared_ptr<SimulationModel>
Study::ModelFactory::simulation(const nlohmann::json& model_json,
                                const Variables& variables,
                                std::shared_ptr<Interface> interface,
                                const Response& response) const
{
  return simulation(validate_and_materialize_selected_model(model_json, "single"),
                    variables, std::move(interface), response);
}

std::shared_ptr<NestedModel>
Study::ModelFactory::nested(const IRStore& model_store,
                            std::shared_ptr<Iterator> sub_iterator,
                            std::shared_ptr<Interface> optional_interface,
                            const Variables& variables,
                            const Response& response) const
{
  return std::make_shared<NestedModel>(
    model_store, std::move(sub_iterator), std::move(optional_interface),
    variables, response, study.services());
}

std::shared_ptr<NestedModel>
Study::ModelFactory::nested(const nlohmann::json& model_json,
                            std::shared_ptr<Iterator> sub_iterator,
                            std::shared_ptr<Interface> optional_interface,
                            const Variables& variables,
                            const Response& response) const
{
  return nested(validate_and_materialize_selected_model(model_json, "nested"),
                std::move(sub_iterator), std::move(optional_interface),
                variables, response);
}

} // namespace Dakota
