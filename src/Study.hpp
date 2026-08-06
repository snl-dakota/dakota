/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#pragma once

#include "StudyConfig.hpp"
#include "IRStore.hpp"

#ifdef DAKOTA_HAVE_MPI
#include <mpi.h>
#endif

#include <memory>
#include <nlohmann/json_fwd.hpp>

namespace Dakota {

class ConcurrentMetaIterator;
class DOTOptimizer;
class Interface;
class Iterator;
class Model;
class MPIManager;
class NestedModel;
class NonDLHSSampling;
class OutputManager;
class ParallelLibrary;
class ProgramOptions;
class Response;
class RunOptions;
class SimulationModel;
class StudyRuntime;
class StudyServices;
class Variables;

/// DI/library-mode study context owning runtime services and factories.
class Study
{
public:
  class MethodFactory;
  class ModelFactory;

  explicit Study(const StudyConfig& config = StudyConfig{});
#ifdef DAKOTA_HAVE_MPI
  Study(MPI_Comm dakota_mpi_comm,
        const StudyConfig& config = StudyConfig{});
#endif
  ~Study();

  std::shared_ptr<StudyServices> services() const;
  std::shared_ptr<ParallelLibrary> parallel_library() const;
  std::shared_ptr<OutputManager> output_manager() const;
  std::shared_ptr<RunOptions> run_options() const;

  Variables variables(const nlohmann::json& variables_json) const;
  Response responses(const nlohmann::json& responses_json,
                     const Variables& variables) const;

  std::shared_ptr<Interface> interface(const IRStore& interface_store) const;
  std::shared_ptr<Interface> interface(const nlohmann::json& interface_json) const;

  MethodFactory method() const;
  ModelFactory model() const;

  void run(Iterator& iterator) const;
  void run(const std::shared_ptr<Iterator>& iterator) const;

private:
  Study(std::shared_ptr<MPIManager> mpi_manager, const StudyConfig& config);

  std::shared_ptr<MPIManager> mpiManager;
  std::shared_ptr<ProgramOptions> programOptions;
  std::shared_ptr<OutputManager> outputManager;
  std::shared_ptr<RunOptions> runOptions;
  std::shared_ptr<ParallelLibrary> parallelLibrary;
  std::shared_ptr<StudyServices> studyServices;
  std::shared_ptr<StudyRuntime> studyRuntime;
};

class Study::MethodFactory
{
public:
  explicit MethodFactory(const Study& study);

  std::shared_ptr<NonDLHSSampling>
  sampling(const IRStore& method_store, std::shared_ptr<Model> model) const;
  std::shared_ptr<NonDLHSSampling>
  sampling(const nlohmann::json& method_json, std::shared_ptr<Model> model) const;

  std::shared_ptr<DOTOptimizer>
  dot_bfgs(const IRStore& method_store, std::shared_ptr<Model> model) const;
  std::shared_ptr<DOTOptimizer>
  dot_bfgs(const nlohmann::json& method_json, std::shared_ptr<Model> model) const;

  std::shared_ptr<ConcurrentMetaIterator>
  multi_start(const IRStore& method_store,
              std::shared_ptr<Iterator> sub_iterator) const;
  std::shared_ptr<ConcurrentMetaIterator>
  multi_start(const nlohmann::json& method_json,
              std::shared_ptr<Iterator> sub_iterator) const;

private:
  const Study& study;
};

class Study::ModelFactory
{
public:
  explicit ModelFactory(const Study& study);

  std::shared_ptr<SimulationModel>
  simulation(const IRStore& model_store, const Variables& variables,
             std::shared_ptr<Interface> interface,
             const Response& response) const;
  std::shared_ptr<SimulationModel>
  simulation(const nlohmann::json& model_json, const Variables& variables,
             std::shared_ptr<Interface> interface,
             const Response& response) const;

  std::shared_ptr<NestedModel>
  nested(const IRStore& model_store, std::shared_ptr<Iterator> sub_iterator,
         std::shared_ptr<Interface> optional_interface,
         const Variables& variables, const Response& response) const;
  std::shared_ptr<NestedModel>
  nested(const nlohmann::json& model_json, std::shared_ptr<Iterator> sub_iterator,
         std::shared_ptr<Interface> optional_interface,
         const Variables& variables, const Response& response) const;

private:
  const Study& study;
};

} // namespace Dakota
