/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "StudyRuntimeServices.hpp"
#include "WorkdirHelper.hpp"

namespace Dakota {

namespace {

ProgramOptions make_program_options(int world_rank, const IRStore& environment_store)
{
  ProgramOptions prog_opts(world_rank);
  prog_opts.parse(environment_store);
  return prog_opts;
}

} // namespace

StudyRuntimeServices::StudyRuntimeServices(const IRStore& environment_store):
  environmentStore(environment_store),
  mpiManager(),
  programOptions(make_program_options(mpiManager.world_rank(), environment_store)),
  outputManager(programOptions, mpiManager.world_rank(),
                mpiManager.mpirun_flag()),
  parallelLib(mpiManager, programOptions, outputManager),
  outputTagActive(false)
{
  outputManager.parse(programOptions, environmentStore);
  outputManager.push_output_tag("", programOptions, false, true);
  outputTagActive = true;
}

StudyRuntimeServices::~StudyRuntimeServices()
{
  if (outputTagActive)
    outputManager.pop_output_tag();
}

std::shared_ptr<StudyRuntimeServices>
make_study_runtime_services(const IRStore& environment_store)
{
  WorkdirHelper::initialize();
  return std::make_shared<StudyRuntimeServices>(environment_store);
}

} // namespace Dakota
