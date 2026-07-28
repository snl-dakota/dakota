#ifndef DAKOTA_LIBRARY_RUNTIME_SUPPORT_H
#define DAKOTA_LIBRARY_RUNTIME_SUPPORT_H

#include <initializer_list>
#include <memory>

namespace Dakota {

class Interface;
class Iterator;
class Model;
class StudyServices;

namespace detail {

struct RuntimeDependency
{
  const char* dependencyName;
  const StudyServices* services;

  RuntimeDependency(const char* dependency_name,
                    const StudyServices* dependency_services = nullptr):
    dependencyName(dependency_name), services(dependency_services)
  { }

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

void validate_services(
  const char* owner_name, const StudyServices* owner_services,
  std::initializer_list<RuntimeDependency> dependencies = {});

void validate_services(
  const char* owner_name, const std::shared_ptr<StudyServices>& owner_services,
  std::initializer_list<RuntimeDependency> dependencies = {});

std::shared_ptr<StudyServices> require_services(
  const char* owner_name, std::shared_ptr<StudyServices> services);

} // namespace detail
} // namespace Dakota

#endif
