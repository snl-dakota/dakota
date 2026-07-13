#ifndef DAKOTA_LIBRARY_RUNTIME_SUPPORT_H
#define DAKOTA_LIBRARY_RUNTIME_SUPPORT_H

#include <initializer_list>
#include <memory>

namespace Dakota {

class Interface;
class Iterator;
class Model;
class OutputManager;
class ParallelLibrary;
class RunOptions;
class StudyServices;

namespace detail {

class OwnedLibraryRuntime;

struct ResolvedRuntime
{
  /// Owned library-mode runtime bundle used when no services were supplied
  /// explicitly or inherited from dependencies.
  std::shared_ptr<OwnedLibraryRuntime> ownedRuntime;
  /// Shared ownership of an explicitly supplied ParallelLibrary, when present.
  std::shared_ptr<ParallelLibrary> sharedParallelLibrary;
  /// Shared ownership of an explicitly supplied OutputManager, when present.
  std::shared_ptr<OutputManager> sharedOutputManager;
  /// Shared ownership of explicitly supplied RunOptions, when present.
  std::shared_ptr<RunOptions> sharedRunOptions;
  /// Shared ownership of the selected StudyServices bundle, when present.
  std::shared_ptr<StudyServices> sharedStudyServices;
  /// Normalized raw access to the ParallelLibrary selected for this runtime.
  ParallelLibrary* parallelLibrary;
  /// Normalized raw access to the OutputManager selected for this runtime.
  OutputManager* outputManager;
  /// Normalized raw access to the RunOptions selected for this runtime.
  RunOptions* runOptions;

  ResolvedRuntime();
};

struct RuntimeDependency
{
  const char* dependencyName;
  ParallelLibrary* parallelLibrary;
  OutputManager* outputManager;
  RunOptions* runOptions;

  RuntimeDependency(const char* dependency_name,
                    ParallelLibrary* parallel_lib = nullptr,
                    OutputManager* output_mgr = nullptr,
                    RunOptions* run_options = nullptr):
    dependencyName(dependency_name),
    parallelLibrary(parallel_lib),
    outputManager(output_mgr),
    runOptions(run_options)
  {
  }

  RuntimeDependency(const char* dependency_name,
                    const Iterator* iterator);
  RuntimeDependency(const char* dependency_name,
                    const Model* model);
  RuntimeDependency(const char* dependency_name,
                    const Interface* interface);
};

template <class Component>
RuntimeDependency runtime_dependency(
  const char* dependency_name, const std::shared_ptr<Component>& component)
{
  return RuntimeDependency(dependency_name, component ? component.get() : nullptr);
}

ResolvedRuntime resolve_runtime(std::shared_ptr<ParallelLibrary> parallel_lib,
                                std::shared_ptr<OutputManager> output_mgr);

ResolvedRuntime resolve_runtime(std::shared_ptr<ParallelLibrary> parallel_lib,
                                std::shared_ptr<OutputManager> output_mgr,
                                std::shared_ptr<RunOptions> run_options);

ResolvedRuntime resolve_runtime(std::shared_ptr<StudyServices> services);

ResolvedRuntime resolve_runtime(std::shared_ptr<StudyServices> services,
                                std::initializer_list<RuntimeDependency> dependencies,
                                const char* owner_name);

ResolvedRuntime resolve_runtime(std::shared_ptr<ParallelLibrary> parallel_lib,
                                std::shared_ptr<OutputManager> output_mgr,
                                std::initializer_list<RuntimeDependency> dependencies,
                                const char* owner_name);

ResolvedRuntime resolve_runtime(std::shared_ptr<ParallelLibrary> parallel_lib,
                                std::shared_ptr<OutputManager> output_mgr,
                                ParallelLibrary* inherited_parallel_lib,
                                OutputManager* inherited_output_mgr,
                                const char* owner_name,
                                const char* dependency_name);

void validate_runtime_consistency(const char* owner_name,
                                  ParallelLibrary* owner_parallel_lib,
                                  OutputManager* owner_output_mgr,
                                  const char* dependency_name,
                                  ParallelLibrary* dependency_parallel_lib,
                                  OutputManager* dependency_output_mgr,
                                  RunOptions* owner_run_options = nullptr,
                                  RunOptions* dependency_run_options = nullptr);

} // namespace detail
} // namespace Dakota

#endif
