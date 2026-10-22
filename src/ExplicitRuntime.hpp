/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef EXPLICIT_RUNTIME_H
#define EXPLICIT_RUNTIME_H

#include "MPIManager.hpp"
#include "OutputManager.hpp"
#include "ParallelLibrary.hpp"
#include "ProgramOptions.hpp"
#include "StudyRuntime.hpp"
#include "StudyServices.hpp"
#include "RunOptions.hpp"
#include "WorkdirHelper.hpp"

#include <memory>

namespace Dakota {

/// Small helper for DI demo programs that need explicit library-mode runtime
/// services outside of Environment.
struct DemoRuntime {
  DemoRuntime():
    mpiManager(),
    programOptions(mpiManager.world_rank()),
    outputManager(std::make_shared<OutputManager>(
      programOptions, mpiManager.world_rank(), mpiManager.mpirun_flag())),
    runOptions(std::make_shared<RunOptions>(programOptions.user_modes())),
    parallelLibrary(std::make_shared<ParallelLibrary>(
      mpiManager, programOptions, *outputManager)),
    services(std::make_shared<StudyServices>(
      parallelLibrary, outputManager, runOptions)),
    studyRuntime(std::make_shared<StudyRuntime>(
      *parallelLibrary, outputManager.get()))
  {
    WorkdirHelper::initialize();
    outputManager->push_output_tag("", programOptions, false, true);
  }

  ~DemoRuntime()
  {
    outputManager->pop_output_tag();
  }

  void execute_iterator(Iterator& iterator)
  {
    studyRuntime->execute_iterator(iterator);
  }

  MPIManager mpiManager;
  ProgramOptions programOptions;
  std::shared_ptr<OutputManager> outputManager;
  std::shared_ptr<RunOptions> runOptions;
  std::shared_ptr<ParallelLibrary> parallelLibrary;
  std::shared_ptr<StudyServices> services;
  std::shared_ptr<StudyRuntime> studyRuntime;
};

} // namespace Dakota

#endif
