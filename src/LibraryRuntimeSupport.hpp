#ifndef DAKOTA_LIBRARY_RUNTIME_SUPPORT_H
#define DAKOTA_LIBRARY_RUNTIME_SUPPORT_H

#include <initializer_list>
#include <memory>

namespace Dakota {

class OutputManager;
class ParallelLibrary;

namespace detail {

class OwnedLibraryRuntime;

struct ResolvedRuntime
{
  std::shared_ptr<OwnedLibraryRuntime> ownedRuntime;
  std::shared_ptr<ParallelLibrary> sharedParallelLibrary;
  std::shared_ptr<OutputManager> sharedOutputManager;
  ParallelLibrary* parallelLibrary;
  OutputManager* outputManager;

  ResolvedRuntime();
};

struct RuntimeDependency
{
  const char* dependencyName;
  ParallelLibrary* parallelLibrary;
  OutputManager* outputManager;

  RuntimeDependency(const char* dependency_name,
                    ParallelLibrary* parallel_lib = nullptr,
                    OutputManager* output_mgr = nullptr):
    dependencyName(dependency_name),
    parallelLibrary(parallel_lib),
    outputManager(output_mgr)
  {
  }
};

ResolvedRuntime resolve_runtime(std::shared_ptr<ParallelLibrary> parallel_lib,
                                std::shared_ptr<OutputManager> output_mgr);

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
                                  OutputManager* dependency_output_mgr);

} // namespace detail
} // namespace Dakota

#endif
