#include "LibraryRuntimeSupport.hpp"

#include "DakotaInterface.hpp"
#include "DakotaIterator.hpp"
#include "DakotaModel.hpp"
#include "MPIManager.hpp"
#include "OutputManager.hpp"
#include "ParallelLibrary.hpp"
#include "ProgramOptions.hpp"
#include "RunOptions.hpp"
#include "StudyServices.hpp"
#include "WorkdirHelper.hpp"

#include <sstream>
#include <stdexcept>
#include <string>

namespace Dakota {
namespace detail {

namespace {

std::string service_name(const char* owner_name, const char* dependency_name,
                         const char* service_name)
{
  std::ostringstream oss;
  oss << owner_name << " and " << dependency_name
      << " were constructed with different " << service_name
      << " instances.";
  return oss.str();
}

std::string service_value_name(const char* owner_name,
                               const char* dependency_name,
                               const char* service_name)
{
  std::ostringstream oss;
  oss << owner_name << " and " << dependency_name
      << " were constructed with incompatible " << service_name
      << " values.";
  return oss.str();
}

std::string missing_parallel_library_name(const char* owner_name)
{
  std::ostringstream oss;
  oss << owner_name << " could not resolve a ParallelLibrary from the "
      << "provided runtime services or dependencies.";
  return oss.str();
}

std::shared_ptr<RunOptions> run_options_from_parallel_library(
  const std::shared_ptr<ParallelLibrary>& parallel_lib)
{
  return parallel_lib ?
    std::make_shared<RunOptions>(parallel_lib->user_modes()) : nullptr;
}

void validate_runtime_against_dependencies(
  const char* owner_name,
  ParallelLibrary* owner_parallel_lib,
  OutputManager* owner_output_mgr,
  RunOptions* owner_run_options,
  std::initializer_list<RuntimeDependency> dependencies)
{
  for (const RuntimeDependency& dependency: dependencies)
    validate_runtime_consistency(owner_name, owner_parallel_lib,
                                 owner_output_mgr,
                                 dependency.dependencyName,
                                 dependency.parallelLibrary,
                                 dependency.outputManager,
                                 owner_run_options,
                                 dependency.runOptions);
}

} // namespace

class OwnedLibraryRuntime
{
public:
  explicit OwnedLibraryRuntime(std::shared_ptr<OutputManager> output_mgr,
                               std::shared_ptr<RunOptions> run_options = nullptr):
    mpiManager(),
    programOptions(mpiManager.world_rank()),
    outputManager(output_mgr ? std::move(output_mgr)
                             : std::make_shared<OutputManager>(
                                 programOptions,
                                 mpiManager.world_rank(),
                                 mpiManager.mpirun_flag())),
    runOptions(run_options ? std::move(run_options)
                           : std::make_shared<RunOptions>(
                               programOptions.user_modes())),
    parallelLibrary(std::make_shared<ParallelLibrary>(
      mpiManager, programOptions, *outputManager)),
    outputTagActive(false)
  {
    WorkdirHelper::initialize();
    outputManager->push_output_tag("", programOptions, false, true);
    outputTagActive = true;
  }

  ~OwnedLibraryRuntime()
  {
    if (outputTagActive)
      outputManager->pop_output_tag();
  }

  ParallelLibrary& parallel_library() const
  { return *parallelLibrary; }

  OutputManager* output_manager() const
  { return outputManager.get(); }

  RunOptions* run_options() const
  { return runOptions.get(); }

  std::shared_ptr<ParallelLibrary> parallel_library_shared() const
  { return parallelLibrary; }

  std::shared_ptr<OutputManager> output_manager_shared() const
  { return outputManager; }

  std::shared_ptr<RunOptions> run_options_shared() const
  { return runOptions; }

private:
  MPIManager mpiManager;
  ProgramOptions programOptions;
  std::shared_ptr<OutputManager> outputManager;
  std::shared_ptr<RunOptions> runOptions;
  std::shared_ptr<ParallelLibrary> parallelLibrary;
  bool outputTagActive;
};

ResolvedRuntime::ResolvedRuntime():
  parallelLibrary(nullptr),
  outputManager(nullptr),
  runOptions(nullptr)
{ }

RuntimeDependency::RuntimeDependency(const char* dependency_name,
                                     const Iterator* iterator):
  RuntimeDependency(dependency_name,
                    iterator ? iterator->parallel_library_ptr() : nullptr,
                    iterator ? iterator->output_manager_ptr() : nullptr,
                    iterator ? iterator->run_options_ptr() : nullptr)
{ }

RuntimeDependency::RuntimeDependency(const char* dependency_name,
                                     const Model* model):
  RuntimeDependency(dependency_name,
                    model ? model->parallel_library_ptr() : nullptr,
                    model ? model->output_manager_ptr() : nullptr,
                    model ? model->run_options_ptr() : nullptr)
{ }

RuntimeDependency::RuntimeDependency(const char* dependency_name,
                                     const Interface* interface):
  RuntimeDependency(dependency_name,
                    interface ? interface->parallel_library_ptr() : nullptr,
                    interface ? interface->output_manager_ptr() : nullptr,
                    interface ? interface->run_options_ptr() : nullptr)
{ }

void validate_runtime_consistency(const char* owner_name,
                                  ParallelLibrary* owner_parallel_lib,
                                  OutputManager* owner_output_mgr,
                                  const char* dependency_name,
                                  ParallelLibrary* dependency_parallel_lib,
                                  OutputManager* dependency_output_mgr,
                                  RunOptions* owner_run_options,
                                  RunOptions* dependency_run_options)
{
  if (owner_parallel_lib && dependency_parallel_lib &&
      owner_parallel_lib != dependency_parallel_lib) {
    throw std::runtime_error(
      service_name(owner_name, dependency_name, "ParallelLibrary"));
  }

  if (owner_output_mgr && dependency_output_mgr &&
      owner_output_mgr != dependency_output_mgr) {
    throw std::runtime_error(
      service_name(owner_name, dependency_name, "OutputManager"));
  }

  if (owner_run_options && dependency_run_options &&
      *owner_run_options != *dependency_run_options) {
    throw std::runtime_error(
      service_value_name(owner_name, dependency_name, "RunOptions"));
  }
}

ResolvedRuntime resolve_runtime(std::shared_ptr<ParallelLibrary> parallel_lib,
                                std::shared_ptr<OutputManager> output_mgr)
{
  return resolve_runtime(std::move(parallel_lib), std::move(output_mgr),
                         std::shared_ptr<RunOptions>());
}

ResolvedRuntime resolve_runtime(std::shared_ptr<ParallelLibrary> parallel_lib,
                                std::shared_ptr<OutputManager> output_mgr,
                                std::shared_ptr<RunOptions> run_options)
{
  ResolvedRuntime runtime;
  if (parallel_lib && output_mgr &&
      &parallel_lib->output_manager() != output_mgr.get()) {
    throw std::runtime_error(
      "ParallelLibrary and OutputManager arguments refer to different "
      "runtime services.");
  }

  runtime.sharedParallelLibrary = std::move(parallel_lib);
  runtime.sharedOutputManager = std::move(output_mgr);
  runtime.sharedRunOptions = std::move(run_options);

  if (runtime.sharedParallelLibrary) {
    if (!runtime.sharedRunOptions)
      runtime.sharedRunOptions =
        run_options_from_parallel_library(runtime.sharedParallelLibrary);

    runtime.parallelLibrary = runtime.sharedParallelLibrary.get();
    runtime.outputManager = runtime.sharedOutputManager ?
      runtime.sharedOutputManager.get() :
      &runtime.sharedParallelLibrary->output_manager();
    runtime.runOptions = runtime.sharedRunOptions.get();
    if (!runtime.sharedOutputManager)
      runtime.sharedOutputManager = std::shared_ptr<OutputManager>(
        runtime.sharedParallelLibrary, runtime.outputManager);

    runtime.sharedStudyServices = std::make_shared<StudyServices>(
      runtime.sharedParallelLibrary, runtime.sharedOutputManager,
      runtime.sharedRunOptions);
    return runtime;
  }

  runtime.ownedRuntime =
    std::make_shared<OwnedLibraryRuntime>(runtime.sharedOutputManager,
                                          runtime.sharedRunOptions);
  runtime.parallelLibrary = &runtime.ownedRuntime->parallel_library();
  runtime.outputManager = runtime.sharedOutputManager ?
    runtime.sharedOutputManager.get() : runtime.ownedRuntime->output_manager();
  runtime.sharedRunOptions = runtime.ownedRuntime->run_options_shared();
  runtime.runOptions = runtime.sharedRunOptions.get();
  runtime.sharedStudyServices = std::make_shared<StudyServices>(
    runtime.ownedRuntime->parallel_library_shared(),
    runtime.sharedOutputManager ? runtime.sharedOutputManager
                                : runtime.ownedRuntime->output_manager_shared(),
    runtime.sharedRunOptions);
  return runtime;
}

ResolvedRuntime resolve_runtime(std::shared_ptr<StudyServices> services)
{
  if (!services)
    return resolve_runtime(std::shared_ptr<ParallelLibrary>(),
                           std::shared_ptr<OutputManager>(),
                           std::shared_ptr<RunOptions>());

  const bool complete_services = services->parallel_library_shared() &&
    services->output_manager_shared() && services->run_options_shared();

  ResolvedRuntime runtime = resolve_runtime(
    services->parallel_library_shared(), services->output_manager_shared(),
    services->run_options_shared());
  if (complete_services)
    runtime.sharedStudyServices = std::move(services);
  return runtime;
}

ResolvedRuntime resolve_runtime(std::shared_ptr<StudyServices> services,
                                std::initializer_list<RuntimeDependency> dependencies,
                                const char* owner_name)
{
  if (services) {
    ResolvedRuntime runtime = resolve_runtime(std::move(services));
    validate_runtime_against_dependencies(owner_name, runtime.parallelLibrary,
                                          runtime.outputManager,
                                          runtime.runOptions, dependencies);
    return runtime;
  }

  return resolve_runtime(std::shared_ptr<ParallelLibrary>(),
                         std::shared_ptr<OutputManager>(),
                         dependencies, owner_name);
}

ResolvedRuntime resolve_runtime(std::shared_ptr<ParallelLibrary> parallel_lib,
                                std::shared_ptr<OutputManager> output_mgr,
                                std::initializer_list<RuntimeDependency> dependencies,
                                const char* owner_name)
{
  if (parallel_lib || output_mgr) {
    ResolvedRuntime runtime =
      resolve_runtime(std::move(parallel_lib), std::move(output_mgr));
    validate_runtime_against_dependencies(owner_name, runtime.parallelLibrary,
                                          runtime.outputManager,
                                          runtime.runOptions, dependencies);
    return runtime;
  }

  ParallelLibrary* inherited_parallel_lib = nullptr;
  OutputManager* inherited_output_mgr = nullptr;
  RunOptions* inherited_run_options = nullptr;
  for (const RuntimeDependency& dependency: dependencies) {
    if (!inherited_parallel_lib)
      inherited_parallel_lib = dependency.parallelLibrary;
    else if (dependency.parallelLibrary &&
             dependency.parallelLibrary != inherited_parallel_lib) {
      throw std::runtime_error(
        service_name(owner_name, dependency.dependencyName, "ParallelLibrary"));
    }

    if (!inherited_output_mgr)
      inherited_output_mgr = dependency.outputManager;
    else if (dependency.outputManager &&
             dependency.outputManager != inherited_output_mgr) {
      throw std::runtime_error(
        service_name(owner_name, dependency.dependencyName, "OutputManager"));
    }

    if (!inherited_run_options)
      inherited_run_options = dependency.runOptions;
    else if (dependency.runOptions &&
             *dependency.runOptions != *inherited_run_options) {
      throw std::runtime_error(
        service_value_name(owner_name, dependency.dependencyName,
                           "RunOptions"));
    }
  }

  if (inherited_parallel_lib || inherited_output_mgr) {
    if (!inherited_parallel_lib)
      throw std::runtime_error(missing_parallel_library_name(owner_name));

    ResolvedRuntime inherited_runtime;
    inherited_runtime.parallelLibrary = inherited_parallel_lib;
    inherited_runtime.outputManager = inherited_output_mgr ?
      inherited_output_mgr : &inherited_parallel_lib->output_manager();
    inherited_runtime.runOptions = inherited_run_options ?
      inherited_run_options :
      const_cast<RunOptions*>(&inherited_parallel_lib->user_modes());
    return inherited_runtime;
  }

  if (inherited_run_options)
    return resolve_runtime(std::shared_ptr<ParallelLibrary>(),
                           std::shared_ptr<OutputManager>(),
                           std::make_shared<RunOptions>(*inherited_run_options));

  return resolve_runtime(std::shared_ptr<ParallelLibrary>(),
                         std::shared_ptr<OutputManager>());
}

ResolvedRuntime resolve_runtime(std::shared_ptr<ParallelLibrary> parallel_lib,
                                std::shared_ptr<OutputManager> output_mgr,
                                ParallelLibrary* inherited_parallel_lib,
                                OutputManager* inherited_output_mgr,
                                const char* owner_name,
                                const char* dependency_name)
{
  return resolve_runtime(
    std::move(parallel_lib), std::move(output_mgr),
    {RuntimeDependency(dependency_name, inherited_parallel_lib,
                       inherited_output_mgr)},
    owner_name);
}

} // namespace detail
} // namespace Dakota
