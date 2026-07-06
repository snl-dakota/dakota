#include "LibraryRuntimeSupport.hpp"

#include "MPIManager.hpp"
#include "OutputManager.hpp"
#include "ParallelLibrary.hpp"
#include "ProgramOptions.hpp"
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

std::string missing_parallel_library_name(const char* owner_name)
{
  std::ostringstream oss;
  oss << owner_name << " could not resolve a ParallelLibrary from the "
      << "provided runtime services or dependencies.";
  return oss.str();
}

void validate_runtime_against_dependencies(
  const char* owner_name,
  ParallelLibrary* owner_parallel_lib,
  OutputManager* owner_output_mgr,
  std::initializer_list<RuntimeDependency> dependencies)
{
  for (const RuntimeDependency& dependency: dependencies)
    validate_runtime_consistency(owner_name, owner_parallel_lib,
                                 owner_output_mgr,
                                 dependency.dependencyName,
                                 dependency.parallelLibrary,
                                 dependency.outputManager);
}

} // namespace

class OwnedLibraryRuntime
{
public:
  explicit OwnedLibraryRuntime(std::shared_ptr<OutputManager> output_mgr):
    mpiManager(),
    programOptions(mpiManager.world_rank()),
    outputManager(output_mgr ? std::move(output_mgr)
                             : std::make_shared<OutputManager>(
                                 programOptions,
                                 mpiManager.world_rank(),
                                 mpiManager.mpirun_flag())),
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

private:
  MPIManager mpiManager;
  ProgramOptions programOptions;
  std::shared_ptr<OutputManager> outputManager;
  std::shared_ptr<ParallelLibrary> parallelLibrary;
  bool outputTagActive;
};

ResolvedRuntime::ResolvedRuntime():
  parallelLibrary(nullptr),
  outputManager(nullptr)
{ }

void validate_runtime_consistency(const char* owner_name,
                                  ParallelLibrary* owner_parallel_lib,
                                  OutputManager* owner_output_mgr,
                                  const char* dependency_name,
                                  ParallelLibrary* dependency_parallel_lib,
                                  OutputManager* dependency_output_mgr)
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
}

ResolvedRuntime resolve_runtime(std::shared_ptr<ParallelLibrary> parallel_lib,
                                std::shared_ptr<OutputManager> output_mgr)
{
  ResolvedRuntime runtime;
  // This check can be eliminated after ParallelLibrary is refactored
  // to remove OutputManager
  if (parallel_lib && output_mgr &&
      &parallel_lib->output_manager() != output_mgr.get()) {
    throw std::runtime_error(
      "ParallelLibrary and OutputManager arguments refer to different "
      "runtime services.");
  }

  // If the caller explicitly provided shared runtime services, preserve
  // that ownership model and normalize raw pointer access for downstream
  // component constructors.
  runtime.sharedParallelLibrary = std::move(parallel_lib);
  runtime.sharedOutputManager = std::move(output_mgr);

  if (runtime.sharedParallelLibrary) {
    runtime.parallelLibrary = runtime.sharedParallelLibrary.get();
    runtime.outputManager = runtime.sharedOutputManager ?
      runtime.sharedOutputManager.get() :
      &runtime.sharedParallelLibrary->output_manager();
    return runtime;
  }

  // No ParallelLibrary was supplied, so create a small owned runtime bundle
  // that bootstraps MPI/program options/output setup for library-mode use.
  // Raw pointer access still flows through the normalized fields below.
  runtime.ownedRuntime =
    std::make_shared<OwnedLibraryRuntime>(runtime.sharedOutputManager);
  runtime.parallelLibrary = &runtime.ownedRuntime->parallel_library();
  runtime.outputManager = runtime.sharedOutputManager ?
    runtime.sharedOutputManager.get() : runtime.ownedRuntime->output_manager();
  return runtime;
}

ResolvedRuntime resolve_runtime(std::shared_ptr<ParallelLibrary> parallel_lib,
                                std::shared_ptr<OutputManager> output_mgr,
                                std::initializer_list<RuntimeDependency> dependencies,
                                const char* owner_name)
{
  // Case 1: the caller supplied explicit services. Validate them against
  // dependency-owned services, then preserve the caller's ownership model.
  if (parallel_lib || output_mgr) {
    ResolvedRuntime runtime =
      resolve_runtime(std::move(parallel_lib), std::move(output_mgr));
    validate_runtime_against_dependencies(owner_name, runtime.parallelLibrary,
                                          runtime.outputManager,
                                          dependencies);
    return runtime;
  }

  // Case 2 setup: no explicit services were provided, so scan dependencies
  // to determine whether they collectively imply a single inherited runtime,
  // i.e. they are have a consistent ParallelLibrary and OutputManager.
  ParallelLibrary* inherited_parallel_lib = nullptr;
  OutputManager* inherited_output_mgr = nullptr;
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
  }

  // Case 2: dependencies agreed on an existing runtime. Borrow those
  // services by pointer; the dependency graph remains responsible for
  // their lifetime.
  if (inherited_parallel_lib || inherited_output_mgr) {
    if (!inherited_parallel_lib)
      throw std::runtime_error(missing_parallel_library_name(owner_name));

    ResolvedRuntime inherited_runtime;
    inherited_runtime.parallelLibrary = inherited_parallel_lib;
    inherited_runtime.outputManager = inherited_output_mgr ?
      inherited_output_mgr : &inherited_parallel_lib->output_manager();
    return inherited_runtime;
  }

  // Case 3: neither the caller nor dependencies provide runtime services.
  // Fall back to constructing an owned library-mode runtime bundle.
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
