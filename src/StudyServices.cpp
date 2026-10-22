/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "StudyServices.hpp"

#include "MPIManager.hpp"
#include "OutputManager.hpp"
#include "ParallelLibrary.hpp"
#include "ProgramOptions.hpp"
#include "WorkdirHelper.hpp"

#include <stdexcept>
#include <utility>

namespace Dakota {

StudyServices::StudyServices()
{ initialize_owned_runtime(nullptr, nullptr); }

StudyServices::StudyServices(std::shared_ptr<ParallelLibrary> parallel_lib,
                             std::shared_ptr<OutputManager> output_mgr,
                             std::shared_ptr<RunOptions> run_options)
{
  if (!parallel_lib) {
    initialize_owned_runtime(std::move(output_mgr), std::move(run_options));
    return;
  }

  if (output_mgr && &parallel_lib->output_manager() != output_mgr.get())
    throw std::runtime_error(
      "StudyServices ParallelLibrary and OutputManager refer to different "
      "runtime services.");

  parallelLibrary = std::move(parallel_lib);
  outputManager = output_mgr ? std::move(output_mgr) :
    std::shared_ptr<OutputManager>(parallelLibrary,
                                   &parallelLibrary->output_manager());
  runOptions = run_options ? std::move(run_options) :
    std::make_shared<RunOptions>(parallelLibrary->user_modes());
}

StudyServices::~StudyServices()
{
  if (outputTagActive && parallelLibrary)
    parallelLibrary->pop_output_tag(*parallelLibrary->w_parallel_level_iterator());
}

void StudyServices::initialize_owned_runtime(
  std::shared_ptr<OutputManager> output_mgr,
  std::shared_ptr<RunOptions> run_options)
{
  WorkdirHelper::initialize();

  mpiManager = std::make_shared<MPIManager>();
  programOptions = std::make_shared<ProgramOptions>(mpiManager->world_rank());
  outputManager = output_mgr ? std::move(output_mgr) :
    std::make_shared<OutputManager>(*programOptions, mpiManager->world_rank(),
                                    mpiManager->mpirun_flag());
  runOptions = run_options ? std::move(run_options) :
    std::make_shared<RunOptions>(programOptions->user_modes());
  parallelLibrary = std::make_shared<ParallelLibrary>(
    *mpiManager, *programOptions, *outputManager);

  parallelLibrary->push_output_tag(*parallelLibrary->w_parallel_level_iterator());
  outputTagActive = true;
}

} // namespace Dakota
