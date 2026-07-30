/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "LibraryRuntimeSupport.hpp"

#include "DakotaInterface.hpp"
#include "DakotaIterator.hpp"
#include "DakotaModel.hpp"
#include "StudyServices.hpp"

#include <sstream>
#include <stdexcept>

namespace Dakota {
namespace detail {

namespace {

std::string service_name(const char* owner_name, const char* dependency_name)
{
  std::ostringstream oss;
  oss << owner_name << " and " << dependency_name
      << " were constructed with different StudyServices instances.";
  return oss.str();
}

std::string missing_services_name(const char* owner_name)
{
  std::ostringstream oss;
  oss << owner_name << " requires non-null StudyServices for DI construction.";
  return oss.str();
}

} // namespace

RuntimeDependency::RuntimeDependency(const char* dependency_name,
                                     const Iterator* iterator):
  RuntimeDependency(dependency_name,
                    iterator ? iterator->study_services_ptr() : nullptr)
{ }

RuntimeDependency::RuntimeDependency(const char* dependency_name,
                                     const Model* model):
  RuntimeDependency(dependency_name,
                    model ? model->study_services_ptr() : nullptr)
{ }

RuntimeDependency::RuntimeDependency(const char* dependency_name,
                                     const Interface* interface):
  RuntimeDependency(dependency_name,
                    interface ? interface->study_services_ptr() : nullptr)
{ }

void validate_services(const char* owner_name,
                       const StudyServices* owner_services,
                       std::initializer_list<RuntimeDependency> dependencies)
{
  if (!owner_services)
    throw std::runtime_error(missing_services_name(owner_name));

  for (const RuntimeDependency& dependency: dependencies) {
    if (dependency.services && dependency.services != owner_services)
      throw std::runtime_error(
        service_name(owner_name, dependency.dependencyName));
  }
}

void validate_services(const char* owner_name,
                       const std::shared_ptr<StudyServices>& owner_services,
                       std::initializer_list<RuntimeDependency> dependencies)
{
  validate_services(owner_name, owner_services.get(), dependencies);
}

std::shared_ptr<StudyServices> require_services(
  const char* owner_name, std::shared_ptr<StudyServices> services)
{
  if (!services)
    throw std::runtime_error(missing_services_name(owner_name));

  return services;
}

} // namespace detail
} // namespace Dakota
