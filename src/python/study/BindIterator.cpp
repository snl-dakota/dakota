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
#include "DataFitSurrBasedLocalMinimizer.hpp"
#include "EffGlobalMinimizer.hpp"
#include "HierarchSurrBasedLocalMinimizer.hpp"
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
#include "SeqHybridMetaIterator.hpp"
#include "Study.hpp"
#include "SurrBasedGlobalMinimizer.hpp"
#include "SurrBasedLocalMinimizer.hpp"
#include "dakota_global_defs.hpp"

#include <memory>
#include <nlohmann/json.hpp>
#include <pybind11/stl.h>
#include <pybind11_json/pybind11_json.hpp>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

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

#ifdef HAVE_NOWPAC
#include "NOWPACOptimizer.hpp"
#endif

#ifdef HAVE_OPTPP
#include "SNLLLeastSq.hpp"
#include "SNLLOptimizer.hpp"
#endif

#ifdef HAVE_NPSOL
#include "NLSSOLLeastSq.hpp"
#endif

#ifdef HAVE_DDACE
#include "DDACEDesignCompExp.hpp"
#endif

#ifdef HAVE_FSUDACE
#include "FSUDesignCompExp.hpp"
#endif

#ifdef HAVE_PSUADE
#include "PSUADEDesignCompExp.hpp"
#endif

#ifdef HAVE_ROL
#include "DakotaROLOptimizer.hpp"
#endif

#ifdef HAVE_EXT_PYTHON_METHOD
#include "DakotaExtPythonMethod.hpp"
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
  register_iterator<SeqHybridMetaIterator>(m, "SeqHybridMetaIterator");
  register_iterator<SurrBasedLocalMinimizer>(m, "SurrBasedLocalMinimizer");
  py::class_<DataFitSurrBasedLocalMinimizer, SurrBasedLocalMinimizer,
             std::shared_ptr<DataFitSurrBasedLocalMinimizer>>(
    m, "DataFitSurrBasedLocalMinimizer", py::module_local());
  py::class_<HierarchSurrBasedLocalMinimizer, SurrBasedLocalMinimizer,
             std::shared_ptr<HierarchSurrBasedLocalMinimizer>>(
    m, "HierarchSurrBasedLocalMinimizer", py::module_local());
  register_iterator<SurrBasedGlobalMinimizer>(m, "SurrBasedGlobalMinimizer");
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
#ifdef HAVE_NOWPAC
  register_iterator<NOWPACOptimizer>(m, "NOWPACOptimizer");
#endif
#ifdef HAVE_OPTPP
  register_iterator<SNLLOptimizer>(m, "SNLLOptimizer");
  register_iterator<SNLLLeastSq>(m, "SNLLLeastSq");
#endif
#ifdef HAVE_NPSOL
  register_iterator<NLSSOLLeastSq>(m, "NLSSOLLeastSq");
#endif
#ifdef HAVE_DDACE
  register_iterator<DDACEDesignCompExp>(m, "DDACEDesignCompExp");
#endif
#ifdef HAVE_FSUDACE
  register_iterator<FSUDesignCompExp>(m, "FSUDesignCompExp");
#endif
#ifdef HAVE_PSUADE
  register_iterator<PSUADEDesignCompExp>(m, "PSUADEDesignCompExp");
#endif
#ifdef HAVE_ROL
  register_iterator<ROLOptimizer>(m, "ROLOptimizer");
#endif
#ifdef HAVE_EXT_PYTHON_METHOD
  register_iterator<ExtPythonMethod>(m, "ExtPythonMethod");
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
            const py::object& config,
            std::shared_ptr<Model> model,
            py::kwargs kwargs) {
           auto store = materialize_method(nlohmann::json{{"multi_start",
             normalize_factory_config(config, kwargs, "MethodFactory.multi_start")}});
           if (sub_iterator)
             return factory.multi_start(store, std::move(sub_iterator));
           if (model)
             return factory.multi_start(store, std::move(model));
           throw py::type_error(
             "MethodFactory.multi_start: either sub_iterator or model must be provided.");
         },
         py::arg("sub_iterator") = py::none(),
         py::arg("config") = py::none(),
         py::arg("model") = py::none(),
         R"doc(Construct a multi-start method.

**Name form** (``sub_method_name`` in config): pass ``model`` (required) and
omit ``sub_iterator``.  The sub-method is built on-the-fly from the named
method type using the given model.

**Pointer form** (``sub_method_pointer`` in config): pass ``sub_iterator``
(required) and omit ``model``.

:param sub_iterator: Method executed from each starting point (pointer form).
:type sub_iterator: dakota.study.Iterator or None
:param model: Model to pass to the named sub-method (name form).
:type model: dakota.study.Model or None
:param config: A ``MultiStartConfig`` or equivalent dictionary.
:param kwargs: Configuration fields used instead of ``config``.
:returns: The constructed meta-iterator.
:rtype: dakota.study.ConcurrentMetaIterator
:raises TypeError: If neither sub_iterator nor model is provided, or both
    configuration forms are used.

See :ref:`multi-start options <method-multi_start>`.)doc");
  factory
    .def("pareto_set",
         [](const Study::MethodFactory& factory,
            std::shared_ptr<Iterator> sub_iterator,
            const py::object& config,
            std::shared_ptr<Model> model,
            py::kwargs kwargs) {
           auto store = materialize_method(nlohmann::json{{"pareto_set",
             normalize_factory_config(config, kwargs, "MethodFactory.pareto_set")}});
           if (sub_iterator)
             return factory.pareto_set(store, std::move(sub_iterator));
           if (model)
             return factory.pareto_set(store, std::move(model));
           throw py::type_error(
             "MethodFactory.pareto_set: either sub_iterator or model must be provided.");
         },
         py::arg("sub_iterator") = py::none(),
         py::arg("config") = py::none(),
         py::arg("model") = py::none(),
         R"doc(Construct a Pareto-set meta-iterator.

**Name form** (``sub_method_name`` in config): pass ``model`` (required) and
omit ``sub_iterator``.  The sub-method is built on-the-fly from the named
method type using the given model.

**Pointer form** (``sub_method_pointer`` in config): pass ``sub_iterator``
(required) and omit ``model``.

:param sub_iterator: Optimizer run for each weight combination (pointer form).
:type sub_iterator: dakota.study.Iterator or None
:param model: Model to pass to the named sub-method (name form).
:type model: dakota.study.Model or None
:param config: A ``ParetoSetConfig`` or equivalent dictionary.
:param kwargs: Configuration fields used instead of ``config``.
:returns: The constructed meta-iterator.
:rtype: dakota.study.ConcurrentMetaIterator
:raises TypeError: If neither sub_iterator nor model is provided, or both
    configuration forms are used.

See :ref:`pareto_set options <method-pareto_set>`.)doc");
  factory
    .def("hybrid_sequential",
         [](const Study::MethodFactory& factory,
            py::object methods_obj,
            py::object models_obj,
            const py::object& config, py::kwargs kwargs) {
           auto store = materialize_method(nlohmann::json{{"hybrid_sequential",
             normalize_factory_config(config, kwargs,
               "MethodFactory.hybrid_sequential")}});
           // Name form: no methods, pass models list
           if (methods_obj.is_none() || py::len(methods_obj) == 0) {
             if (models_obj.is_none())
               throw py::type_error(
                 "MethodFactory.hybrid_sequential: either methods or models must be provided.");
             auto models = models_obj.cast<std::vector<std::shared_ptr<Model>>>();
             return factory.hybrid_sequential(store, std::move(models));
           }
           auto methods = methods_obj.cast<std::vector<std::shared_ptr<Iterator>>>();
           for (const auto& m : methods)
             if (!m)
               throw py::type_error(
                 "MethodFactory.hybrid_sequential: methods must not contain None");
           return factory.hybrid_sequential(store, std::move(methods));
         },
         py::arg("methods") = py::none(), py::arg("models") = py::none(),
         py::arg("config") = py::none(),
         R"doc(Construct a sequential hybrid meta-iterator.

**Pointer form**: pass ``methods`` (required, non-None items).

**Name form** (``method_name_list`` in config): pass ``models`` (required)
and omit ``methods``.  Each sub-iterator is built on-the-fly from the
corresponding name and model.

:param methods: Ordered list of methods to execute in sequence (pointer form).
:type methods: list[dakota.study.Iterator] or None
:param models: List of models for the named sub-methods (name form).
    May be a single-element list to share one model across all sub-methods.
:type models: list[dakota.study.Model] or None
:param config: A ``HybridSequentialConfig`` or equivalent dictionary.
:param kwargs: Configuration fields used instead of ``config``.
:returns: The constructed sequential hybrid.
:rtype: dakota.study.SeqHybridMetaIterator
:raises TypeError: If neither methods nor models is provided, a method is null,
    or both configuration forms are used.

See :ref:`hybrid_sequential options <method-hybrid_sequential>`.)doc");

  bind_method(factory, "vector_parameter_study",
    [](const Study::MethodFactory& f, const IRStore& store,
       std::shared_ptr<Model> model) {
      return f.vector_parameter_study(store, std::move(model));
    });
  bind_method(factory, "list_parameter_study",
    [](const Study::MethodFactory& f, const IRStore& store,
       std::shared_ptr<Model> model) {
      return f.list_parameter_study(store, std::move(model));
    });
  bind_method(factory, "centered_parameter_study",
    [](const Study::MethodFactory& f, const IRStore& store,
       std::shared_ptr<Model> model) {
      return f.centered_parameter_study(store, std::move(model));
    });
  bind_method(factory, "multidim_parameter_study",
    [](const Study::MethodFactory& f, const IRStore& store,
       std::shared_ptr<Model> model) {
      return f.multidim_parameter_study(store, std::move(model));
    });
  bind_method(factory, "richardson_extrap",
    [](const Study::MethodFactory& f, const IRStore& store,
       std::shared_ptr<Model> model) {
      return f.richardson_extrap(store, std::move(model));
    });
  bind_method(factory, "local_interval_est",
    [](const Study::MethodFactory& f, const IRStore& store,
       std::shared_ptr<Model> model) {
      return f.local_interval_est(store, std::move(model));
    });
  bind_method(factory, "global_interval_est",
    [](const Study::MethodFactory& f, const IRStore& store,
       std::shared_ptr<Model> model) {
      return f.global_interval_est(store, std::move(model));
    });
  bind_method(factory, "efficient_global",
    [](const Study::MethodFactory& f, const IRStore& store,
       std::shared_ptr<Model> model) {
      return f.efficient_global(store, std::move(model));
    });
#ifdef HAVE_NPSOL
  bind_method(factory, "npsol_sqp",
    [](const Study::MethodFactory& f, const IRStore& store,
       std::shared_ptr<Model> model) {
      return f.npsol_sqp(store, std::move(model));
    });
#endif
  bind_method(factory, "nl2sol",
    [](const Study::MethodFactory& f, const IRStore& store,
       std::shared_ptr<Model> model) {
      return f.nl2sol(store, std::move(model));
    });
  factory.def("surrogate_based_local",
    [](const Study::MethodFactory& self, std::shared_ptr<Model> model,
       std::shared_ptr<Iterator> method,
       const py::object& config, py::kwargs kwargs) {
      const auto store = materialize_method(nlohmann::json{{"surrogate_based_local",
        normalize_factory_config(config, kwargs,
          "MethodFactory.surrogate_based_local")}});
      if (model->surrogate_type() == "ensemble")
        return std::static_pointer_cast<Iterator>(
          std::make_shared<HierarchSurrBasedLocalMinimizer>(
            store, std::move(model), self.services(), std::move(method)));
      return std::static_pointer_cast<Iterator>(
        std::make_shared<DataFitSurrBasedLocalMinimizer>(
          store, std::move(model), self.services(), std::move(method)));
    },
    py::arg("model").none(false), py::arg("method") = py::none(),
    py::arg("config") = py::none(),
    R"doc(Construct a surrogate-based local minimizer.

:param model: Surrogate model iterated by the outer trust-region loop.
:type model: dakota.study.Model
:param method: Sub-problem optimizer applied to the approximate sub-problem.
    Required when ``sub_method = method_pointer`` is used; omitted when
    ``sub_method_name`` is used (the optimizer is built on-the-fly).
:type method: dakota.study.Iterator or None
:param config: A ``SurrogateBasedLocalConfig`` or equivalent dictionary.
:param kwargs: Configuration fields used instead of ``config``.
:returns: The constructed minimizer.
:rtype: dakota.study.Iterator
:raises RuntimeError: If ``method`` is missing when ``method_pointer`` is
    specified, or if the supplied model is not a surrogate model.

See :ref:`surrogate_based_local options <method-surrogate_based_local>`.)doc");
  factory.def("surrogate_based_global",
    [](const Study::MethodFactory& self, std::shared_ptr<Model> model,
       std::shared_ptr<Iterator> method,
       const py::object& config, py::kwargs kwargs) {
      const auto store = materialize_method(nlohmann::json{{"surrogate_based_global",
        normalize_factory_config(config, kwargs,
          "MethodFactory.surrogate_based_global")}});
      return std::static_pointer_cast<Iterator>(
        std::make_shared<SurrBasedGlobalMinimizer>(
          store, std::move(model), self.services(), std::move(method)));
    },
    py::arg("model").none(false), py::arg("method") = py::none(),
    py::arg("config") = py::none(),
    R"doc(Construct a surrogate-based global minimizer.

:param model: Surrogate model iterated by the outer loop.
:type model: dakota.study.Model
:param method: Sub-problem optimizer applied to the surrogate. Required
    with ``method_pointer``; omitted with ``method_name`` (built on the fly).
    If built on a different model, it is redirected to ``model``.
:type method: dakota.study.Iterator or None
:param config: A ``SurrogateBasedGlobalConfig`` or equivalent dictionary.
:param kwargs: Configuration fields used instead of ``config``.
:returns: The constructed minimizer.
:rtype: dakota.study.Iterator
:raises RuntimeError: If ``method`` is missing when ``method_pointer`` is
    specified, if ``method`` has scaling or calibration data transforms and
    was built on a different model, or if ``model`` is not a surrogate model.

See :ref:`surrogate_based_global options <method-surrogate_based_global>`.)doc");

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
#ifdef HAVE_NOWPAC
  bind_method(factory, "nowpac", &construct<NOWPACOptimizer>);
#endif
#ifdef HAVE_NOWPAC
  bind_method(factory, "snowpac", &construct<NOWPACOptimizer>);
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
#ifdef HAVE_OPTPP
  bind_method(factory, "optpp_g_newton", &construct<SNLLLeastSq>);
#endif
#ifdef HAVE_NPSOL
  bind_method(factory, "nlssol_sqp", &construct<NLSSOLLeastSq>);
#endif
#ifdef HAVE_DDACE
  bind_method(factory, "dace", &construct<DDACEDesignCompExp>);
#endif
#ifdef HAVE_FSUDACE
  bind_method(factory, "fsu_quasi_mc", &construct<FSUDesignCompExp>);
  bind_method(factory, "fsu_cvt", &construct<FSUDesignCompExp>);
#endif
#ifdef HAVE_PSUADE
  bind_method(factory, "psuade_moat", &construct<PSUADEDesignCompExp>);
#endif
#ifdef HAVE_ROL
  bind_method(factory, "rol", &construct<ROLOptimizer>);
#endif
#ifdef HAVE_EXT_PYTHON_METHOD
  bind_method(factory, "external_python", &construct<ExtPythonMethod>);
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
  factory
    .def("branch_and_bound",
         [](const Study::MethodFactory& factory,
            std::shared_ptr<Model> model, std::shared_ptr<Iterator> method,
            const py::object& config, py::kwargs kwargs) {
           return std::make_shared<PebbldMinimizer>(
             materialize_method(nlohmann::json{{"branch_and_bound",
               normalize_factory_config(config, kwargs,
                 "MethodFactory.branch_and_bound")}}),
             std::move(model), std::move(method), factory.services());
         },
         py::arg("model") = py::none(), py::arg("method") = py::none(),
         py::arg("config") = py::none(),
         R"doc(Construct a branch-and-bound minimizer.

:param model: Model with relaxed discrete variables to branch on (build its
    variables with ``domain={'relaxed': True}``; unlike an input file, the
    domain is not inferred from the method). Required
    with ``method_name``; with ``method_pointer`` it defaults to the model of
    ``method`` and, if given, must be that model.
:type model: dakota.study.Model or None
:param method: Sub-problem minimizer; required with ``method_pointer`` and
    omitted with ``method_name``.
:type method: dakota.study.Iterator or None
:param config: A ``BranchAndBoundConfig`` or equivalent dictionary.
:param kwargs: Configuration fields used instead of ``config``.
:returns: The constructed minimizer.
:rtype: dakota.study.PebbldMinimizer
:raises TypeError: If both configuration forms are used.
:raises RuntimeError: If ``model`` or ``method`` is missing or unexpected for
    the selected sub-method, or ``method`` iterates on a different model.

This method is present only when Dakota is built with ACRO. See
:ref:`branch_and_bound options <method-branch_and_bound>`.)doc");
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
