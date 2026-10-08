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
#include <vector>

namespace Dakota {

/** \defgroup StudyAPI Library Study API
 * Public dependency-injection API for constructing and executing Dakota
 * studies without a complete input file.
 */

class ConcurrentMetaIterator;
class DataFitSurrModel;
class DOTOptimizer;
class EffGlobalMinimizer;
class EnsembleSurrModel;
class Interface;
class Iterator;
class Model;
class MPIManager;
class NL2SOLLeastSq;
class NPSOLOptimizer;
class NonDLocalSingleInterval;
class ParamStudy;
class NestedModel;
class NonDLHSSampling;
class OutputManager;
class ParallelLibrary;
class ProgramOptions;
class Response;
class RichExtrapVerification;
class RunOptions;
class SimulationModel;
class StudyRuntime;
class StudyServices;
class Variables;

/** \brief Owns runtime services and factories for a library-mode study.
 *
 * Construct variables, responses, interfaces, models, and methods from the
 * leaves inward, then pass the top-level Iterator to run().  The Study must
 * outlive every component constructed from it.  Components from different
 * Study instances may have incompatible runtime services and must not be
 * combined.
 * \ingroup StudyAPI
 */
class Study
{
public:
  class MethodFactory;
  class ModelFactory;

  /** \brief Construct a serial study context.
   * \param config Output and execution-phase settings.
   */
  explicit Study(const StudyConfig& config = StudyConfig{});
#ifdef DAKOTA_HAVE_MPI
  /** \brief Construct a study on an application-supplied MPI communicator.
   * \param dakota_mpi_comm Communicator used by Dakota; ownership remains with the caller.
   * \param config Output and execution-phase settings.
   */
  Study(MPI_Comm dakota_mpi_comm,
        const StudyConfig& config = StudyConfig{});
#endif
  /// Release the study context after all constructed components are finished.
  ~Study();

  /// Return the shared service bundle used by constructed components.
  std::shared_ptr<StudyServices> services() const;
  /// Return the parallel runtime owned by this study.
  std::shared_ptr<ParallelLibrary> parallel_library() const;
  /// Return the output manager owned by this study.
  std::shared_ptr<OutputManager> output_manager() const;
  /// Return the configured execution-phase options.
  std::shared_ptr<RunOptions> run_options() const;

  /** \brief Construct variables from materialized input representation.
   * \param variables_store Materialized variables block.
   * \return Variables associated with this study.
   */
  Variables variables(const IRStore& variables_store) const;
  /** \brief Validate and construct variables from a JSON block fragment.
   * \param variables_json Children of a variables block, without a top-level wrapper.
   * \return Variables associated with this study.
   * \throws std::exception if validation or materialization fails.
   */
  Variables variables(const nlohmann::json& variables_json) const;
  /** \brief Construct responses from materialized input representation.
   * \param responses_store Materialized responses block.
   * \param variables Variables whose active view determines response derivatives.
   * \return Response definition associated with variables.
   */
  Response responses(const IRStore& responses_store,
                     const Variables& variables) const;
  /** \brief Validate and construct responses from a JSON block fragment.
   * \param responses_json Children of a responses block, without a top-level wrapper.
   * \param variables Variables whose active view determines response derivatives.
   * \return Response definition associated with variables.
   * \throws std::exception if validation or materialization fails.
   */
  Response responses(const nlohmann::json& responses_json,
                     const Variables& variables) const;

  /** \brief Construct an interface from materialized input representation.
   * \param interface_store Materialized interface block.
   * \return Interface sharing this study's runtime services.
   * \throws std::runtime_error if the interface type is not supported by the DI API.
   */
  std::shared_ptr<Interface> interface(const IRStore& interface_store) const;
  /** \brief Validate and construct an interface from a JSON block fragment.
   * \param interface_json Children of an interface block, without a top-level wrapper.
   * \return Interface sharing this study's runtime services.
   * \throws std::exception if validation, materialization, or construction fails.
   */
  std::shared_ptr<Interface> interface(const nlohmann::json& interface_json) const;

  /// Return a method factory tied to this study's runtime services.
  MethodFactory method() const;
  /// Return a model factory tied to this study's runtime services.
  ModelFactory model() const;

  /** \brief Execute a top-level iterator.
   * \param iterator Iterator constructed with compatible study services.
   */
  void run(Iterator& iterator) const;
  /** \brief Execute a shared top-level iterator.
   * \param iterator Non-null iterator constructed with compatible study services.
   * \throws std::runtime_error if iterator is null.
   */
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

/** \brief Constructs methods using an owning Study's runtime services.
 *
 * JSON arguments contain the children of the selected method keyword, without
 * a method array or selector wrapper.  IRStore overloads are intended for
 * callers that already have validated, materialized configuration.
 * \ingroup StudyAPI
 */
class Study::MethodFactory
{
public:
  /// Construct a factory borrowing study; study must outlive the factory and its products.
  explicit MethodFactory(const Study& study);

  /// Runtime services for additional DI iterator factories.
  std::shared_ptr<StudyServices> services() const { return study.services(); }

  /// Construct sampling from materialized configuration and a non-null model.
  std::shared_ptr<NonDLHSSampling>
  sampling(const IRStore& method_store, std::shared_ptr<Model> model) const;
  /// Validate a method-sampling fragment and construct sampling on model.
  std::shared_ptr<NonDLHSSampling>
  sampling(const nlohmann::json& method_json, std::shared_ptr<Model> model) const;

  /// Construct a vector parameter study from materialized configuration and model.
  std::shared_ptr<ParamStudy>
  vector_parameter_study(const IRStore& method_store, std::shared_ptr<Model> model) const;
  /// Validate a method-vector_parameter_study fragment and construct the method.
  std::shared_ptr<ParamStudy>
  vector_parameter_study(const nlohmann::json& method_json, std::shared_ptr<Model> model) const;

  /// Construct a list parameter study from materialized configuration and model.
  std::shared_ptr<ParamStudy>
  list_parameter_study(const IRStore& method_store, std::shared_ptr<Model> model) const;
  /// Validate a method-list_parameter_study fragment and construct the method.
  std::shared_ptr<ParamStudy>
  list_parameter_study(const nlohmann::json& method_json, std::shared_ptr<Model> model) const;

  /// Construct a centered parameter study from materialized configuration and model.
  std::shared_ptr<ParamStudy>
  centered_parameter_study(const IRStore& method_store, std::shared_ptr<Model> model) const;
  /// Validate a method-centered_parameter_study fragment and construct the method.
  std::shared_ptr<ParamStudy>
  centered_parameter_study(const nlohmann::json& method_json, std::shared_ptr<Model> model) const;

  /// Construct a multidimensional parameter study from materialized configuration and model.
  std::shared_ptr<ParamStudy>
  multidim_parameter_study(const IRStore& method_store, std::shared_ptr<Model> model) const;
  /// Validate a method-multidim_parameter_study fragment and construct the method.
  std::shared_ptr<ParamStudy>
  multidim_parameter_study(const nlohmann::json& method_json, std::shared_ptr<Model> model) const;

  /// Construct Richardson extrapolation from materialized configuration and model.
  std::shared_ptr<RichExtrapVerification>
  richardson_extrap(const IRStore& method_store, std::shared_ptr<Model> model) const;
  /// Validate a method-richardson_extrap fragment and construct the method.
  std::shared_ptr<RichExtrapVerification>
  richardson_extrap(const nlohmann::json& method_json, std::shared_ptr<Model> model) const;

  /// Construct local interval estimation from materialized configuration and model.
  std::shared_ptr<NonDLocalSingleInterval>
  local_interval_est(const IRStore& method_store, std::shared_ptr<Model> model) const;
  /// Validate a method-local_interval_est fragment and construct the method.
  std::shared_ptr<NonDLocalSingleInterval>
  local_interval_est(const nlohmann::json& method_json, std::shared_ptr<Model> model) const;

  /// Construct global interval estimation from materialized configuration and model.
  std::shared_ptr<Iterator>
  global_interval_est(const IRStore& method_store, std::shared_ptr<Model> model) const;
  /// Validate a method-global_interval_est fragment and construct the method.
  std::shared_ptr<Iterator>
  global_interval_est(const nlohmann::json& method_json, std::shared_ptr<Model> model) const;

  /// Construct efficient global optimization from materialized configuration and model.
  std::shared_ptr<EffGlobalMinimizer>
  efficient_global(const IRStore& method_store, std::shared_ptr<Model> model) const;
  /// Validate a method-efficient_global fragment and construct the method.
  std::shared_ptr<EffGlobalMinimizer>
  efficient_global(const nlohmann::json& method_json, std::shared_ptr<Model> model) const;

  /// Construct NPSOL SQP; the build must provide NPSOL.
  std::shared_ptr<NPSOLOptimizer>
  npsol_sqp(const IRStore& method_store, std::shared_ptr<Model> model) const;
  /// Validate an npsol_sqp fragment and construct NPSOL SQP; the build must provide NPSOL.
  std::shared_ptr<NPSOLOptimizer>
  npsol_sqp(const nlohmann::json& method_json, std::shared_ptr<Model> model) const;

  /// Construct NL2SOL; the build must provide NL2SOL.
  std::shared_ptr<NL2SOLLeastSq>
  nl2sol(const IRStore& method_store, std::shared_ptr<Model> model) const;
  /// Validate an nl2sol fragment and construct NL2SOL; the build must provide NL2SOL.
  std::shared_ptr<NL2SOLLeastSq>
  nl2sol(const nlohmann::json& method_json, std::shared_ptr<Model> model) const;

  /// Construct DOT BFGS; throws if Dakota was built without DOT.
  std::shared_ptr<DOTOptimizer>
  dot_bfgs(const IRStore& method_store, std::shared_ptr<Model> model) const;
  /// Validate a dot_bfgs fragment and construct it; throws if DOT is unavailable.
  std::shared_ptr<DOTOptimizer>
  dot_bfgs(const nlohmann::json& method_json, std::shared_ptr<Model> model) const;

  /// Construct multi-start around a non-null injected sub-iterator.
  std::shared_ptr<ConcurrentMetaIterator>
  multi_start(const IRStore& method_store,
              std::shared_ptr<Iterator> sub_iterator) const;
  /// Validate a multi_start fragment and construct it around sub_iterator.
  std::shared_ptr<ConcurrentMetaIterator>
  multi_start(const nlohmann::json& method_json,
              std::shared_ptr<Iterator> sub_iterator) const;

private:
  const Study& study;
};

/** \brief Constructs models using an owning Study's runtime services.
 *
 * JSON arguments contain the children of the selected model keyword, without
 * a model array or selector wrapper.  Child models, methods, interfaces,
 * variables, and responses are injected as objects rather than resolved from
 * pointer strings.
 * \ingroup StudyAPI
 */
class Study::ModelFactory
{
public:
  /// Construct a factory borrowing study; study must outlive the factory and its products.
  explicit ModelFactory(const Study& study);

  /// Construct a simulation model from materialized configuration and injected components.
  std::shared_ptr<SimulationModel>
  single(const IRStore& model_store, const Variables& variables,
         std::shared_ptr<Interface> interface,
         const Response& response) const;
  /// Validate a model-single fragment and construct a simulation model.
  std::shared_ptr<SimulationModel>
  single(const nlohmann::json& model_json, const Variables& variables,
         std::shared_ptr<Interface> interface,
         const Response& response) const;

  /// Construct a nested model with a required sub-iterator and optional interface.
  std::shared_ptr<NestedModel>
  nested(const IRStore& model_store, std::shared_ptr<Iterator> sub_iterator,
         std::shared_ptr<Interface> optional_interface,
         const Variables& variables, const Response& response) const;
  /// Validate a model-nested fragment and construct a nested model.
  std::shared_ptr<NestedModel>
  nested(const nlohmann::json& model_json, std::shared_ptr<Iterator> sub_iterator,
         std::shared_ptr<Interface> optional_interface,
         const Variables& variables, const Response& response) const;

  /// Construct an ensemble from a truth model and approximation models.
  std::shared_ptr<EnsembleSurrModel>
  ensemble_surrogate(const IRStore& model_store,
                     std::shared_ptr<Model> truth_model,
                     std::vector<std::shared_ptr<Model>> approximation_models,
                     const Variables& variables, const Response& response) const;
  /// Validate an ensemble_surrogate fragment and construct the ensemble.
  std::shared_ptr<EnsembleSurrModel>
  ensemble_surrogate(const nlohmann::json& model_json,
                     std::shared_ptr<Model> truth_model,
                     std::vector<std::shared_ptr<Model>> approximation_models,
                     const Variables& variables, const Response& response) const;

  /// Construct a global surrogate with optional truth-model and DACE dependencies.
  std::shared_ptr<DataFitSurrModel>
  global_surrogate(const IRStore& model_store, const Variables& variables,
                   const Response& response,
                   std::shared_ptr<Model> truth_model = nullptr,
                   std::shared_ptr<Iterator> dace_iterator = nullptr) const;
  /// Validate a global_surrogate fragment and construct the surrogate.
  std::shared_ptr<DataFitSurrModel>
  global_surrogate(const nlohmann::json& model_json,
                   const Variables& variables, const Response& response,
                   std::shared_ptr<Model> truth_model = nullptr,
                   std::shared_ptr<Iterator> dace_iterator = nullptr) const;

  /// Construct a local surrogate around a non-null truth model.
  std::shared_ptr<DataFitSurrModel>
  local_surrogate(const IRStore& model_store,
                  std::shared_ptr<Model> truth_model,
                  const Variables& variables, const Response& response) const;
  /// Validate a local_surrogate fragment and construct the surrogate.
  std::shared_ptr<DataFitSurrModel>
  local_surrogate(const nlohmann::json& model_json,
                  std::shared_ptr<Model> truth_model,
                  const Variables& variables, const Response& response) const;

  /// Construct a multipoint surrogate around a non-null truth model.
  std::shared_ptr<DataFitSurrModel>
  multipoint_surrogate(const IRStore& model_store,
                       std::shared_ptr<Model> truth_model,
                       const Variables& variables,
                       const Response& response) const;
  /// Validate a multipoint_surrogate fragment and construct the surrogate.
  std::shared_ptr<DataFitSurrModel>
  multipoint_surrogate(const nlohmann::json& model_json,
                       std::shared_ptr<Model> truth_model,
                       const Variables& variables,
                       const Response& response) const;

  /// Construct an ensemble from models ordered low-to-high; the last is truth.
  std::shared_ptr<EnsembleSurrModel>
  ensemble_surrogate(const IRStore& model_store,
                     std::vector<std::shared_ptr<Model>> ordered_models,
                     const Variables& variables, const Response& response) const;
  /// Validate an ensemble_surrogate fragment and construct an ordered ensemble.
  std::shared_ptr<EnsembleSurrModel>
  ensemble_surrogate(const nlohmann::json& model_json,
                     std::vector<std::shared_ptr<Model>> ordered_models,
                     const Variables& variables, const Response& response) const;

private:
  const Study& study;
};

} // namespace Dakota
