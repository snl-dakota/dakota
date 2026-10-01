/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaStudyPython.hpp"

#include "ConcurrentMetaIterator.hpp"
#include "DakotaIterator.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "EffGlobalMinimizer.hpp"
#include "NL2SOLLeastSq.hpp"
#include "NPSOLOptimizer.hpp"
#include "NonDACVSampling.hpp"
#include "NonDAdaptImpSampling.hpp"
#include "NonDGPImpSampling.hpp"
#include "NonDGenACVSampling.hpp"
#include "NonDGlobalEvidence.hpp"
#include "NonDGlobalReliability.hpp"
#include "NonDGlobalSingleInterval.hpp"
#include "NonDImportPoints.hpp"
#include "NonDLHSEvidence.hpp"
#include "NonDLHSSampling.hpp"
#include "NonDLHSSingleInterval.hpp"
#include "NonDLocalEvidence.hpp"
#include "NonDLocalReliability.hpp"
#include "NonDLocalSingleInterval.hpp"
#include "NonDMultifidelitySampling.hpp"
#include "NonDMultilevBLUESampling.hpp"
#include "NonDMultilevControlVarSampling.hpp"
#include "NonDMultilevelPolynomialChaos.hpp"
#include "NonDMultilevelSampling.hpp"
#include "NonDMultilevelStochCollocation.hpp"
#include "NonDPOFDarts.hpp"
#include "NonDPolynomialChaos.hpp"
#include "NonDRKDDarts.hpp"
#include "NonDStochCollocation.hpp"
#include "NonDSurrogateExpansion.hpp"
#include "NonDWASABIBayesCalibration.hpp"
#include "NonlinearCGOptimizer.hpp"
#include "OptDartsOptimizer.hpp"
#include "ParamStudy.hpp"
#include "RichExtrapVerification.hpp"
#include "Study.hpp"
#include "dakota_global_defs.hpp"

#include <memory>
#include <nlohmann/json.hpp>
#include <pybind11_json/pybind11_json.hpp>
#include <stdexcept>
#include <type_traits>
#include <utility>

#ifdef HAVE_DOT
#include "DOTOptimizer.hpp"
#endif

#ifdef HAVE_C3
#include "NonDC3FunctionTrain.hpp"
#include "NonDMultilevelFunctionTrain.hpp"
#endif

#ifdef HAVE_ADAPTIVE_SAMPLING
#include "NonDAdaptiveSampling.hpp"
#endif

#ifdef HAVE_NOMAD
#include "NomadOptimizer.hpp"
#endif

#ifdef HAVE_NCSU
#include "NCSUOptimizer.hpp"
#endif

#ifdef DAKOTA_HOPS
#include "APPSOptimizer.hpp"
#endif

#ifdef HAVE_NLPQL
#include "NLPQLPOptimizer.hpp"
#endif

#ifdef HAVE_CONMIN
#include "CONMINOptimizer.hpp"
#endif

#ifdef HAVE_OPTPP
#include "SNLLOptimizer.hpp"
#endif

#ifdef HAVE_ACRO
#include "COLINOptimizer.hpp"
#include "PEBBLMinimizer.hpp"
#endif

#ifdef HAVE_JEGA
#include "JEGAOptimizer.hpp"
#endif

#ifdef HAVE_QUESO
#include "NonDQUESOBayesCalibration.hpp"
#endif

#ifdef HAVE_QUESO_GPMSA
#include "NonDGPMSABayesCalibration.hpp"
#endif

#ifdef HAVE_DREAM
#include "NonDDREAMBayesCalibration.hpp"
#endif

#ifdef HAVE_MUQ
#include "NonDMUQBayesCalibration.hpp"
#endif

namespace Dakota::python {
namespace {

// The DI constructors use two argument orders; keep that detail out of the API.
template<class T>
std::shared_ptr<Iterator> construct(const Study::MethodFactory& factory,
                                    const IRStore& store,
                                    std::shared_ptr<Model> model)
{
  if constexpr (std::is_constructible_v<T, std::shared_ptr<StudyServices>,
                                       const IRStore&, std::shared_ptr<Model>>)
    return std::make_shared<T>(factory.services(), store, std::move(model));
  else
    return std::make_shared<T>(store, std::move(model), factory.services());
}

template<class T>
void register_iterator(py::module_& m, const char* name)
{
  py::class_<T, Iterator, std::shared_ptr<T>>(m, name, py::module_local());
}

template<class Constructor>
void bind_method(py::class_<Study::MethodFactory>& factory, const char* name,
                 Constructor constructor)
{
  const std::string doc =
    std::string("Construct ``") + name + "`` for an injected model.\n\n"
    ":param model: Model evaluated by the method.\n"
    ":type model: dakota.study.Model\n"
    ":param config: The matching ``dakota.spec.method`` configuration model "
    "or an equivalent dictionary.\n"
    ":param kwargs: Configuration fields used instead of ``config``.\n"
    ":returns: The constructed method/iterator.\n"
    ":rtype: dakota.study.Iterator\n"
    ":raises TypeError: If ``model`` is null or both configuration forms are used.\n\n"
    "See :ref:`" + name + " options <method-" + name + ">`.";
  factory.def(name,
    [name, constructor](const Study::MethodFactory& self,
                        std::shared_ptr<Model> model,
                        const py::object& config, py::kwargs kwargs) {
      const auto fragment = normalize_factory_config(
        config, kwargs, (std::string("MethodFactory.") + name).c_str());
      const auto store = materialize_method(nlohmann::json{{name, fragment}});
      return constructor(self, store, std::move(model));
    }, py::arg("model").none(false), py::arg("config") = py::none(),
    doc.c_str());
}

} // namespace

// Register iterator classes and their instance methods before factories.
void bind_iterators(py::module_& m)
{
  py::class_<Iterator, std::shared_ptr<Iterator>>(
    m, "Iterator", py::module_local(),
    "Base method/iterator handle returned by MethodFactory.");
  py::class_<NonDLHSSampling, Iterator, std::shared_ptr<NonDLHSSampling>>(
    m, "NonDLHSSampling", py::module_local(),
    "Sampling iterator returned by MethodFactory.sampling().")
    .def("num_responses",
         [](const NonDLHSSampling& sampling) {
           return sampling.all_responses().size();
         },
         R"doc(Return the number of responses produced by a completed run.

:returns: Number of evaluated responses.
:rtype: int)doc")
    .def("first_response_value",
         [](const NonDLHSSampling& sampling) {
           const auto& responses = sampling.all_responses();
           if (responses.empty())
             throw std::runtime_error("No responses are available.");

           const auto& first_response = responses.begin()->second;
           if (first_response.num_functions() == 0)
             throw std::runtime_error("First response has no functions.");

           return first_response.function_value(0);
         },
         R"doc(Return the first function value from the first response.

:returns: The first recorded scalar function value.
:rtype: float
:raises RuntimeError: If the iterator has no responses or the first response
    has no functions.)doc");
  py::class_<ParamStudy, Iterator, std::shared_ptr<ParamStudy>>(
    m, "ParamStudy", py::module_local());
  py::class_<RichExtrapVerification, Iterator, std::shared_ptr<RichExtrapVerification>>(
    m, "RichExtrapVerification", py::module_local());
  py::class_<NonDLocalSingleInterval, Iterator, std::shared_ptr<NonDLocalSingleInterval>>(
    m, "NonDLocalSingleInterval", py::module_local());
  py::class_<NonDGlobalSingleInterval, Iterator, std::shared_ptr<NonDGlobalSingleInterval>>(
    m, "NonDGlobalSingleInterval", py::module_local());
  py::class_<NonDLHSSingleInterval, Iterator, std::shared_ptr<NonDLHSSingleInterval>>(
    m, "NonDLHSSingleInterval", py::module_local());
#ifdef HAVE_NCSU
  py::class_<EffGlobalMinimizer, Iterator, std::shared_ptr<EffGlobalMinimizer>>(
    m, "EffGlobalMinimizer", py::module_local());
#endif
#ifdef HAVE_NPSOL
  py::class_<NPSOLOptimizer, Iterator, std::shared_ptr<NPSOLOptimizer>>(
    m, "NPSOLOptimizer", py::module_local());
#endif
#ifdef HAVE_NL2SOL
  py::class_<NL2SOLLeastSq, Iterator, std::shared_ptr<NL2SOLLeastSq>>(
    m, "NL2SOLLeastSq", py::module_local());
#endif
#ifdef HAVE_DOT
  py::class_<DOTOptimizer, Iterator, std::shared_ptr<DOTOptimizer>>(
    m, "DOTOptimizer", py::module_local());
#endif
  py::class_<ConcurrentMetaIterator, Iterator,
             std::shared_ptr<ConcurrentMetaIterator>>(
    m, "ConcurrentMetaIterator", py::module_local());
  register_iterator<NonDLocalReliability>(m, "NonDLocalReliability");
  register_iterator<NonDGlobalReliability>(m, "NonDGlobalReliability");
  register_iterator<NonDLocalEvidence>(m, "NonDLocalEvidence");
  register_iterator<NonDPolynomialChaos>(m, "NonDPolynomialChaos");
  register_iterator<NonDMultilevelPolynomialChaos>(m, "NonDMultilevelPolynomialChaos");
  register_iterator<NonDStochCollocation>(m, "NonDStochCollocation");
  register_iterator<NonDMultilevelStochCollocation>(m, "NonDMultilevelStochCollocation");
  register_iterator<NonDSurrogateExpansion>(m, "NonDSurrogateExpansion");
  register_iterator<NonDGPImpSampling>(m, "NonDGPImpSampling");
  register_iterator<NonDPOFDarts>(m, "NonDPOFDarts");
  register_iterator<NonDRKDDarts>(m, "NonDRKDDarts");
  register_iterator<NonDAdaptImpSampling>(m, "NonDAdaptImpSampling");
  register_iterator<NonDImportPoints>(m, "NonDImportPoints");
  register_iterator<NonDMultilevControlVarSampling>(m, "NonDMultilevControlVarSampling");
  register_iterator<NonDMultilevBLUESampling>(m, "NonDMultilevBLUESampling");
  register_iterator<NonlinearCGOptimizer>(m, "NonlinearCGOptimizer");
  register_iterator<OptDartsOptimizer>(m, "OptDartsOptimizer");
#ifdef HAVE_C3
  register_iterator<NonDC3FunctionTrain>(m, "NonDC3FunctionTrain");
#endif
#ifdef HAVE_C3
  register_iterator<NonDMultilevelFunctionTrain>(m, "NonDMultilevelFunctionTrain");
#endif
#ifdef HAVE_ADAPTIVE_SAMPLING
  register_iterator<NonDAdaptiveSampling>(m, "NonDAdaptiveSampling");
#endif
#ifdef HAVE_NOMAD
  register_iterator<NomadOptimizer>(m, "NomadOptimizer");
#endif
#ifdef HAVE_NCSU
  register_iterator<NCSUOptimizer>(m, "NCSUOptimizer");
#endif
#ifdef DAKOTA_HOPS
  register_iterator<APPSOptimizer>(m, "APPSOptimizer");
#endif
#ifdef HAVE_NLPQL
  register_iterator<NLPQLPOptimizer>(m, "NLPQLPOptimizer");
#endif
#ifdef HAVE_CONMIN
  register_iterator<CONMINOptimizer>(m, "CONMINOptimizer");
#endif
#ifdef HAVE_OPTPP
  register_iterator<SNLLOptimizer>(m, "SNLLOptimizer");
#endif
#ifdef HAVE_ACRO
  register_iterator<COLINOptimizer>(m, "COLINOptimizer");
#endif
#ifdef HAVE_ACRO
  register_iterator<PebbldMinimizer>(m, "PebbldMinimizer");
#endif
#ifdef HAVE_JEGA
  register_iterator<JEGAOptimizer>(m, "JEGAOptimizer");
#endif
  register_iterator<NonDGlobalEvidence>(m, "NonDGlobalEvidence");
  register_iterator<NonDLHSEvidence>(m, "NonDLHSEvidence");
  register_iterator<NonDMultilevelSampling>(m, "NonDMultilevelSampling");
  register_iterator<NonDMultifidelitySampling>(m, "NonDMultifidelitySampling");
  register_iterator<NonDACVSampling>(m, "NonDACVSampling");
  register_iterator<NonDGenACVSampling>(m, "NonDGenACVSampling");
  register_iterator<NonDWASABIBayesCalibration>(m, "NonDWASABIBayesCalibration");
#ifdef HAVE_QUESO
  register_iterator<NonDQUESOBayesCalibration>(m, "NonDQUESOBayesCalibration");
#endif
#ifdef HAVE_QUESO_GPMSA
  register_iterator<NonDGPMSABayesCalibration>(m, "NonDGPMSABayesCalibration");
#endif
#ifdef HAVE_DREAM
  register_iterator<NonDDREAMBayesCalibration>(m, "NonDDREAMBayesCalibration");
#endif
#ifdef HAVE_MUQ
  register_iterator<NonDMUQBayesCalibration>(m, "NonDMUQBayesCalibration");
#endif
}

void bind_iterator_factories(py::module_& m)
{
  auto factory = py::class_<Study::MethodFactory>(
    m, "MethodFactory",
    "Construct methods that share the owning Study's runtime services.\n\n"
    "This factory is an implementation detail of Study and ordinarily should "
    "not be instantiated directly. Access it through Study.method. Factory "
    "availability depends "
    "on the capabilities enabled in the Dakota build.");
  factory
    .def("sampling",
         [](const Study::MethodFactory& factory,
            std::shared_ptr<Model> model,
            const py::object& config, py::kwargs kwargs) {
           return factory.sampling(
             materialize_method(nlohmann::json{{"sampling",
               normalize_factory_config(config, kwargs, "MethodFactory.sampling")}}),
             std::move(model));
         },
         py::arg("model").none(false), py::arg("config") = py::none(),
         R"doc(Construct a sampling method for an injected model.

:param model: Model evaluated by the sampling method.
:type model: dakota.study.Model
:param config: A ``SamplingConfig`` or equivalent dictionary.
:param kwargs: Configuration fields used instead of ``config``.
:returns: The constructed sampling iterator.
:rtype: dakota.study.NonDLHSSampling
:raises TypeError: If ``model`` is null or both configuration forms are used.

See :ref:`sampling options <method-sampling>`.)doc");
#ifdef HAVE_DOT
  factory
    .def("dot_bfgs",
         [](const Study::MethodFactory& factory,
            std::shared_ptr<Model> model,
            const py::object& config, py::kwargs kwargs) {
           return factory.dot_bfgs(
             materialize_method(nlohmann::json{{"dot_bfgs",
               normalize_factory_config(config, kwargs, "MethodFactory.dot_bfgs")}}),
             std::move(model));
         },
         py::arg("model").none(false), py::arg("config") = py::none(),
         R"doc(Construct a DOT BFGS method for an injected model.

:param model: Model optimized by DOT BFGS.
:type model: dakota.study.Model
:param config: A ``DotBfgsConfig`` or equivalent dictionary.
:param kwargs: Configuration fields used instead of ``config``.
:returns: The constructed optimizer.
:rtype: dakota.study.DOTOptimizer
:raises TypeError: If ``model`` is null or both configuration forms are used.

This method is present only when Dakota is built with DOT. See
:ref:`DOT BFGS options <method-dot_bfgs>`.)doc");
#endif
  factory
    .def("multi_start",
         [](const Study::MethodFactory& factory,
            std::shared_ptr<Iterator> sub_iterator,
            const py::object& config, py::kwargs kwargs) {
           return factory.multi_start(
             materialize_method(nlohmann::json{{"multi_start",
               normalize_factory_config(config, kwargs, "MethodFactory.multi_start")}}),
             std::move(sub_iterator));
         },
         py::arg("sub_iterator").none(false), py::arg("config") = py::none(),
         R"doc(Construct a multi-start method around an injected sub-iterator.

:param sub_iterator: Method executed from each starting point.
:type sub_iterator: dakota.study.Iterator
:param config: A ``MultiStartConfig`` or equivalent dictionary.
:param kwargs: Configuration fields used instead of ``config``.
:returns: The constructed meta-iterator.
:rtype: dakota.study.ConcurrentMetaIterator
:raises TypeError: If the dependency is null or both configuration forms are used.

See :ref:`multi-start options <method-multi_start>`.)doc")
    .def("vector_parameter_study",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.vector_parameter_study(
             materialize_method(nlohmann::json{{"vector_parameter_study",
               validate_vector_parameter_study_fragment(method_json)}}),
             std::move(model));
         },
         py::arg("method"), py::arg("model"),
         "Construct a vector parameter study from its configuration fragment and model.")
    .def("list_parameter_study",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.list_parameter_study(
             materialize_method(nlohmann::json{{"list_parameter_study",
               validate_list_parameter_study_fragment(method_json)}}),
             std::move(model));
         },
         py::arg("method"), py::arg("model"),
         "Construct a list parameter study from its configuration fragment and model.")
    .def("centered_parameter_study",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.centered_parameter_study(
             materialize_method(nlohmann::json{{"centered_parameter_study",
               validate_centered_parameter_study_fragment(method_json)}}),
             std::move(model));
         },
         py::arg("method"), py::arg("model"),
         "Construct a centered parameter study from its configuration fragment and model.")
    .def("multidim_parameter_study",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.multidim_parameter_study(
             materialize_method(nlohmann::json{{"multidim_parameter_study",
               validate_multidim_parameter_study_fragment(method_json)}}),
             std::move(model));
         },
         py::arg("method"), py::arg("model"),
         "Construct a multidimensional parameter study from its configuration fragment and model.")
    .def("richardson_extrap",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.richardson_extrap(
             materialize_method(nlohmann::json{{"richardson_extrap",
               validate_richardson_extrap_fragment(method_json)}}),
             std::move(model));
         },
         py::arg("method"), py::arg("model"),
         "Construct Richardson extrapolation from its configuration fragment and model.")
    .def("local_interval_est",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.local_interval_est(
             materialize_method(nlohmann::json{{"local_interval_est",
               validate_local_interval_est_fragment(method_json)}}),
             std::move(model));
         },
         py::arg("method"), py::arg("model"),
         "Construct local interval estimation from its configuration fragment and model.")
    .def("global_interval_est",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.global_interval_est(
             materialize_method(nlohmann::json{{"global_interval_est",
               validate_global_interval_est_fragment(method_json)}}),
             std::move(model));
         },
         py::arg("method"), py::arg("model"),
         "Construct global interval estimation from its configuration fragment and model.")
    .def("efficient_global",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.efficient_global(
             materialize_method(nlohmann::json{{"efficient_global",
               validate_efficient_global_fragment(method_json)}}),
             std::move(model));
         },
         py::arg("method"), py::arg("model"),
         "Construct efficient global optimization from its configuration fragment and model.")
    .def("npsol_sqp",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.npsol_sqp(
             materialize_method(nlohmann::json{{"npsol_sqp",
               validate_npsol_sqp_fragment(method_json)}}),
             std::move(model));
         },
         py::arg("method"), py::arg("model"),
         "Construct NPSOL SQP from its configuration fragment and model. "
         "Available only in builds with NPSOL.")
    .def("nl2sol",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.nl2sol(
             materialize_method(nlohmann::json{{"nl2sol",
               validate_nl2sol_fragment(method_json)}}),
             std::move(model));
         },
         py::arg("method"), py::arg("model"),
         "Construct NL2SOL from its configuration fragment and model. "
         "Available only in builds with NL2SOL.");

  bind_method(factory, "local_reliability", &construct<NonDLocalReliability>);
  bind_method(factory, "global_reliability", &construct<NonDGlobalReliability>);
  bind_method(factory, "local_evidence", &construct<NonDLocalEvidence>);
  bind_method(factory, "polynomial_chaos", &construct<NonDPolynomialChaos>);
  bind_method(factory, "multilevel_polynomial_chaos", &construct<NonDMultilevelPolynomialChaos>);
  bind_method(factory, "multifidelity_polynomial_chaos", &construct<NonDMultilevelPolynomialChaos>);
  bind_method(factory, "stoch_collocation", &construct<NonDStochCollocation>);
  bind_method(factory, "multifidelity_stoch_collocation", &construct<NonDMultilevelStochCollocation>);
  bind_method(factory, "surrogate_based_uq", &construct<NonDSurrogateExpansion>);
  bind_method(factory, "gpais", &construct<NonDGPImpSampling>);
  bind_method(factory, "pof_darts", &construct<NonDPOFDarts>);
  bind_method(factory, "rkd_darts", &construct<NonDRKDDarts>);
  bind_method(factory, "importance_sampling", &construct<NonDAdaptImpSampling>);
  bind_method(factory, "import_points", &construct<NonDImportPoints>);
  bind_method(factory, "multilevel_multifidelity_sampling", &construct<NonDMultilevControlVarSampling>);
  bind_method(factory, "multilevel_blue", &construct<NonDMultilevBLUESampling>);
  bind_method(factory, "nonlinear_cg", &construct<NonlinearCGOptimizer>);
  bind_method(factory, "genie_opt_darts", &construct<OptDartsOptimizer>);
  bind_method(factory, "genie_direct", &construct<OptDartsOptimizer>);
#ifdef HAVE_C3
  bind_method(factory, "function_train", &construct<NonDC3FunctionTrain>);
#endif
#ifdef HAVE_C3
  bind_method(factory, "multilevel_function_train", &construct<NonDMultilevelFunctionTrain>);
#endif
#ifdef HAVE_C3
  bind_method(factory, "multifidelity_function_train", &construct<NonDMultilevelFunctionTrain>);
#endif
#ifdef HAVE_ADAPTIVE_SAMPLING
  bind_method(factory, "adaptive_sampling", &construct<NonDAdaptiveSampling>);
#endif
#ifdef HAVE_NOMAD
  bind_method(factory, "mesh_adaptive_search", &construct<NomadOptimizer>);
#endif
#ifdef HAVE_NCSU
  bind_method(factory, "ncsu_direct", &construct<NCSUOptimizer>);
#endif
#ifdef DAKOTA_HOPS
  bind_method(factory, "asynch_pattern_search", &construct<APPSOptimizer>);
#endif
#ifdef HAVE_NLPQL
  bind_method(factory, "nlpql_sqp", &construct<NLPQLPOptimizer>);
#endif
#ifdef HAVE_DOT
  bind_method(factory, "dot_frcg", &construct<DOTOptimizer>);
#endif
#ifdef HAVE_DOT
  bind_method(factory, "dot_mmfd", &construct<DOTOptimizer>);
#endif
#ifdef HAVE_DOT
  bind_method(factory, "dot_slp", &construct<DOTOptimizer>);
#endif
#ifdef HAVE_DOT
  bind_method(factory, "dot_sqp", &construct<DOTOptimizer>);
#endif
#ifdef HAVE_CONMIN
  bind_method(factory, "conmin_frcg", &construct<CONMINOptimizer>);
#endif
#ifdef HAVE_CONMIN
  bind_method(factory, "conmin_mfd", &construct<CONMINOptimizer>);
#endif
#ifdef HAVE_OPTPP
  bind_method(factory, "optpp_q_newton", &construct<SNLLOptimizer>);
#endif
#ifdef HAVE_OPTPP
  bind_method(factory, "optpp_fd_newton", &construct<SNLLOptimizer>);
#endif
#ifdef HAVE_OPTPP
  bind_method(factory, "optpp_newton", &construct<SNLLOptimizer>);
#endif
#ifdef HAVE_OPTPP
  bind_method(factory, "optpp_cg", &construct<SNLLOptimizer>);
#endif
#ifdef HAVE_OPTPP
  bind_method(factory, "optpp_pds", &construct<SNLLOptimizer>);
#endif
#ifdef HAVE_ACRO
  bind_method(factory, "coliny_beta", &construct<COLINOptimizer>);
#endif
#ifdef HAVE_ACRO
  bind_method(factory, "coliny_cobyla", &construct<COLINOptimizer>);
#endif
#ifdef HAVE_ACRO
  bind_method(factory, "coliny_direct", &construct<COLINOptimizer>);
#endif
#ifdef HAVE_ACRO
  bind_method(factory, "coliny_ea", &construct<COLINOptimizer>);
#endif
#ifdef HAVE_ACRO
  bind_method(factory, "coliny_pattern_search", &construct<COLINOptimizer>);
#endif
#ifdef HAVE_ACRO
  bind_method(factory, "coliny_solis_wets", &construct<COLINOptimizer>);
#endif
#ifdef HAVE_ACRO
  bind_method(factory, "branch_and_bound", &construct<PebbldMinimizer>);
#endif
#ifdef HAVE_JEGA
  bind_method(factory, "moga", &construct<JEGAOptimizer>);
#endif
#ifdef HAVE_JEGA
  bind_method(factory, "soga", &construct<JEGAOptimizer>);
#endif

  bind_method(factory, "global_evidence",
    [](const Study::MethodFactory& self, const IRStore& store,
       std::shared_ptr<Model> model) {
      if (store.get<unsigned short>("nond.opt_subproblem_solver") == SUBMETHOD_LHS)
        return construct<NonDLHSEvidence>(self, store, std::move(model));
      return construct<NonDGlobalEvidence>(self, store, std::move(model));
    });
  bind_method(factory, "multilevel_sampling",
    [](const Study::MethodFactory& self, const IRStore& store,
       std::shared_ptr<Model> model) {
      if (store.get<unsigned short>("sub_method") == SUBMETHOD_WEIGHTED_MLMC)
        return construct<NonDGenACVSampling>(self, store, std::move(model));
      return construct<NonDMultilevelSampling>(self, store, std::move(model));
    });
  bind_method(factory, "multifidelity_sampling",
    [](const Study::MethodFactory& self, const IRStore& store,
       std::shared_ptr<Model> model) {
      if (store.get<short>("nond.search_model_graphs.recursion") ||
          store.get<short>("nond.search_model_graphs.selection"))
        return construct<NonDGenACVSampling>(self, store, std::move(model));
      return construct<NonDMultifidelitySampling>(self, store, std::move(model));
    });
  bind_method(factory, "approximate_control_variate",
    [](const Study::MethodFactory& self, const IRStore& store,
       std::shared_ptr<Model> model) {
      if (store.get<short>("nond.search_model_graphs.recursion") ||
          store.get<short>("nond.search_model_graphs.selection") ||
          store.get<unsigned short>("sub_method") == SUBMETHOD_ACV_RD)
        return construct<NonDGenACVSampling>(self, store, std::move(model));
      return construct<NonDACVSampling>(self, store, std::move(model));
    });
  bind_method(factory, "bayes_calibration",
    [](const Study::MethodFactory& self, const IRStore& store,
       std::shared_ptr<Model> model) -> std::shared_ptr<Iterator> {
      switch (store.get<unsigned short>("sub_method")) {
#ifdef HAVE_QUESO
      case SUBMETHOD_QUESO:
        return construct<NonDQUESOBayesCalibration>(self, store, std::move(model));
#endif
#ifdef HAVE_QUESO_GPMSA
      case SUBMETHOD_GPMSA:
        return construct<NonDGPMSABayesCalibration>(self, store, std::move(model));
#endif
#ifdef HAVE_DREAM
      case SUBMETHOD_DREAM:
        return construct<NonDDREAMBayesCalibration>(self, store, std::move(model));
#endif
#ifdef HAVE_MUQ
      case SUBMETHOD_MUQ:
        return construct<NonDMUQBayesCalibration>(self, store, std::move(model));
#endif
      case SUBMETHOD_WASABI:
        return construct<NonDWASABIBayesCalibration>(self, store, std::move(model));
      default:
        throw py::value_error("The selected Bayesian calibration backend is unavailable in this build.");
      }
    });
}

} // namespace Dakota::python
