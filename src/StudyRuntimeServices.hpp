/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_STUDY_RUNTIME_SERVICES_H
#define DAKOTA_STUDY_RUNTIME_SERVICES_H

#include "IRStore.hpp"
#include "MPIManager.hpp"
#include "OutputManager.hpp"
#include "ParallelLibrary.hpp"
#include "ProgramOptions.hpp"

#include <memory>

namespace Dakota {

class StudyRuntimeServices
{
public:
  explicit StudyRuntimeServices(const IRStore& environment_store);
  ~StudyRuntimeServices();

  StudyRuntimeServices(const StudyRuntimeServices&) = delete;
  StudyRuntimeServices& operator=(const StudyRuntimeServices&) = delete;

  ParallelLibrary& parallel_library();
  ProgramOptions& program_options();
  OutputManager& output_manager();
  const IRStore& environment_store() const;

private:
  IRStore environmentStore;
  MPIManager mpiManager;
  ProgramOptions programOptions;
  OutputManager outputManager;
  ParallelLibrary parallelLib;
  bool outputTagActive;
};

std::shared_ptr<StudyRuntimeServices>
make_study_runtime_services(const IRStore& environment_store);

inline ParallelLibrary& StudyRuntimeServices::parallel_library()
{ return parallelLib; }

inline ProgramOptions& StudyRuntimeServices::program_options()
{ return programOptions; }

inline OutputManager& StudyRuntimeServices::output_manager()
{ return outputManager; }

inline const IRStore& StudyRuntimeServices::environment_store() const
{ return environmentStore; }

} // namespace Dakota

#endif
