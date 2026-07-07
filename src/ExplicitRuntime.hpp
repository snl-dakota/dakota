#ifndef EXPLICIT_RUNTIME_H
#define EXPLICIT_RUNTIME_H

#include "MPIManager.hpp"
#include "OutputManager.hpp"
#include "ParallelLibrary.hpp"
#include "ProgramOptions.hpp"
#include "StudyRuntime.hpp"
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
    parallelLibrary(std::make_shared<ParallelLibrary>(
      mpiManager, programOptions, *outputManager)),
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
  std::shared_ptr<ParallelLibrary> parallelLibrary;
  std::shared_ptr<StudyRuntime> studyRuntime;
};

} // namespace Dakota

#endif
