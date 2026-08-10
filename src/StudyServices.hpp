/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#pragma once

#include "RunOptions.hpp"

#include <memory>

namespace Dakota {

class MPIManager;
class OutputManager;
class ParallelLibrary;
class ProgramOptions;

/// Shared ambient services used by DI/library-mode study components.
class StudyServices
{
public:
  /// Construct a coherent default library-mode service bundle.
  StudyServices();

  /// Construct from supplied services, filling only service-level gaps.
  StudyServices(std::shared_ptr<ParallelLibrary> parallel_lib,
                std::shared_ptr<OutputManager> output_mgr = nullptr,
                std::shared_ptr<RunOptions> run_options = nullptr);

  ~StudyServices();

  StudyServices(const StudyServices&) = delete;
  StudyServices& operator=(const StudyServices&) = delete;

  ParallelLibrary* parallel_library_ptr() const
  { return parallelLibrary.get(); }

  OutputManager* output_manager_ptr() const
  { return outputManager.get(); }

  RunOptions* run_options_ptr() const
  { return runOptions.get(); }

  std::shared_ptr<ParallelLibrary> parallel_library_shared() const
  { return parallelLibrary; }

  std::shared_ptr<OutputManager> output_manager_shared() const
  { return outputManager; }

  std::shared_ptr<RunOptions> run_options_shared() const
  { return runOptions; }

private:
  void initialize_owned_runtime(std::shared_ptr<OutputManager> output_mgr,
                                std::shared_ptr<RunOptions> run_options);

  std::shared_ptr<MPIManager> mpiManager;
  std::shared_ptr<ProgramOptions> programOptions;
  std::shared_ptr<OutputManager> outputManager;
  std::shared_ptr<ParallelLibrary> parallelLibrary;
  std::shared_ptr<RunOptions> runOptions;
  bool outputTagActive = false;
};

} // namespace Dakota
