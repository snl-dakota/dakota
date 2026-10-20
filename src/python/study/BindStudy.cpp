/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaStudyPython.hpp"

#include "IRState.hpp"
#include "ConcurrentMetaIterator.hpp"
#include "DataFitSurrModel.hpp"
#include "EffGlobalMinimizer.hpp"
#include "EnsembleSurrModel.hpp"
#ifdef HAVE_DOT
#include "DOTOptimizer.hpp"
#endif
#include "NestedModel.hpp"
#include "SimulationModel.hpp"
#include "DakotaInterface.hpp"
#include "DakotaIterator.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "DakotaVariables.hpp"
#include "Study.hpp"
#include "StudyConfig.hpp"

#include <nlohmann/json.hpp>
#include <pybind11_json/pybind11_json.hpp>
#include <pybind11/stl.h>

#include <memory>
#include <optional>
#include <utility>

namespace Dakota::python {

void bind_study_factories(py::module_& m)
{
  py::class_<Study::ModelFactory>(
    m, "ModelFactory",
    "Construct models that share the owning Study's runtime services.\n\n"
    "This factory is an implementation detail of Study and ordinarily should "
    "not be instantiated directly. Access it through Study.model. Inject "
    "variables, responses, "
    "interfaces, sub-iterators, and child models as objects rather than "
    "resolving input-file pointer strings.")
    .def("single",
         [](const Study::ModelFactory& factory,
            const py::object& model_json,
            const Variables& variables,
            std::shared_ptr<Interface> interface,
            const Response& response) {
           return factory.single(materialize_model(nlohmann::json{{"single",
                                     validate_single_fragment(model_json)}}),
                                     variables, std::move(interface), response);
         },
         py::arg("model"), py::arg("variables"), py::arg("interface"),
         py::arg("response"),
         "Construct a simulation model using the legacy config-first calling convention.")
    .def("simulation",
         [](const Study::ModelFactory& factory,
            const Variables& variables, std::shared_ptr<Interface> interface,
            const Response& response, const py::object& config,
            py::kwargs kwargs) {
           return factory.single(
             materialize_model(nlohmann::json{{"single",
               normalize_factory_config(config, kwargs, "ModelFactory.simulation")}}),
             variables, std::move(interface), response);
         },
         py::arg("variables").none(false), py::arg("interface").none(false),
         py::arg("response").none(false), py::arg("config") = py::none(),
         R"doc(Construct a simulation model from injected components.

:param variables: Variables evaluated by the model.
:type variables: dakota.study.Variables
:param interface: Interface used to perform evaluations.
:type interface: dakota.study.Interface
:param response: Response definition for evaluations.
:type response: dakota.study.Response
:param config: A ``SingleConfig`` or equivalent dictionary.
:param kwargs: Configuration fields used instead of ``config``.
:returns: The constructed simulation model.
:rtype: dakota.study.SimulationModel
:raises TypeError: If a dependency is null or both configuration forms are used.

See :ref:`single-model options <model-single>`.)doc")
    .def("nested",
         [](const Study::ModelFactory& factory,
            std::shared_ptr<Iterator> sub_iterator, const Variables& variables,
            const Response& response,
            std::shared_ptr<Interface> optional_interface,
            const py::object& config, py::kwargs kwargs) {
           return factory.nested(
             materialize_model(nlohmann::json{{"nested",
               normalize_factory_config(config, kwargs, "ModelFactory.nested")}}),
             std::move(sub_iterator), std::move(optional_interface), variables,
             response);
         },
         py::arg("sub_iterator").none(false), py::arg("variables").none(false),
         py::arg("response").none(false),
         py::arg("optional_interface") = py::none(),
         py::arg("config") = py::none(),
         R"doc(Construct a nested model from an injected sub-iterator.

:param sub_iterator: Method executed by the nested model.
:type sub_iterator: dakota.study.Iterator
:param variables: Variables exposed by the nested model.
:type variables: dakota.study.Variables
:param response: Response definition exposed by the nested model.
:type response: dakota.study.Response
:param optional_interface: Optional interface for non-nested responses.
:type optional_interface: dakota.study.Interface or None
:param config: A ``NestedConfig`` or equivalent dictionary.
:param kwargs: Configuration fields used instead of ``config``.
:returns: The constructed nested model.
:rtype: dakota.study.NestedModel
:raises TypeError: If a required dependency is null or both configuration forms are used.

See :ref:`nested-model options <model-nested>`.)doc")
    .def("global_surrogate",
         [](const Study::ModelFactory& factory,
            const Variables& variables, const Response& response,
            std::shared_ptr<Model> truth_model,
            std::shared_ptr<Iterator> dace_iterator,
            const py::object& config, py::kwargs kwargs) {
           const auto fragment = normalize_factory_config(
             config, kwargs, "ModelFactory.global_surrogate");
           return factory.global_surrogate(
             materialize_model(
               nlohmann::json{{"global_surrogate", fragment}}),
             variables, response, std::move(truth_model),
             std::move(dace_iterator));
         },
         py::arg("variables").none(false), py::arg("response").none(false),
         py::arg("truth_model") = py::none(),
         py::arg("dace_iterator") = py::none(),
         py::arg("config") = py::none(),
         R"doc(Construct a global data-fit surrogate.

:param variables: Variables exposed by the surrogate.
:type variables: dakota.study.Variables
:param response: Response definition exposed by the surrogate.
:type response: dakota.study.Response
:param truth_model: Optional model used to generate or check approximation data.
:type truth_model: dakota.study.Model or None
:param dace_iterator: Optional iterator used to generate approximation data.
:type dace_iterator: dakota.study.Iterator or None
:param config: A ``GlobalSurrogateConfig`` or equivalent dictionary.
:param kwargs: Configuration fields used instead of ``config``.
:returns: The constructed data-fit surrogate.
:rtype: dakota.study.DataFitSurrModel

See :ref:`global surrogate options <model-global_surrogate>`.)doc")
    .def("local_surrogate",
         [](const Study::ModelFactory& factory,
            std::shared_ptr<Model> truth_model,
            const Variables& variables, const Response& response,
            const py::object& config, py::kwargs kwargs) {
           const auto fragment = normalize_factory_config(
             config, kwargs, "ModelFactory.local_surrogate");
           return factory.local_surrogate(
             materialize_model(
               nlohmann::json{{"local_surrogate", fragment}}),
             std::move(truth_model), variables, response);
         },
         py::arg("truth_model").none(false),
         py::arg("variables").none(false), py::arg("response").none(false),
         py::arg("config") = py::none(),
         R"doc(Construct a local surrogate from an injected truth model.

:param truth_model: Model approximated by the local surrogate.
:type truth_model: dakota.study.Model
:param variables: Variables exposed by the surrogate.
:type variables: dakota.study.Variables
:param response: Response definition exposed by the surrogate.
:type response: dakota.study.Response
:param config: A ``LocalSurrogateConfig`` or equivalent dictionary.
:param kwargs: Configuration fields used instead of ``config``.
:returns: The constructed data-fit surrogate.
:rtype: dakota.study.DataFitSurrModel

See :ref:`local surrogate options <model-local_surrogate>`.)doc")
    .def("multipoint_surrogate",
         [](const Study::ModelFactory& factory,
            std::shared_ptr<Model> truth_model,
            const Variables& variables, const Response& response,
            const py::object& config, py::kwargs kwargs) {
           const auto fragment = normalize_factory_config(
             config, kwargs, "ModelFactory.multipoint_surrogate");
           return factory.multipoint_surrogate(
             materialize_model(
               nlohmann::json{{"multipoint_surrogate", fragment}}),
             std::move(truth_model), variables, response);
         },
         py::arg("truth_model").none(false),
         py::arg("variables").none(false), py::arg("response").none(false),
         py::arg("config") = py::none(),
         R"doc(Construct a multipoint surrogate from an injected truth model.

:param truth_model: Model approximated by the multipoint surrogate.
:type truth_model: dakota.study.Model
:param variables: Variables exposed by the surrogate.
:type variables: dakota.study.Variables
:param response: Response definition exposed by the surrogate.
:type response: dakota.study.Response
:param config: A ``MultipointSurrogateConfig`` or equivalent dictionary.
:param kwargs: Configuration fields used instead of ``config``.
:returns: The constructed data-fit surrogate.
:rtype: dakota.study.DataFitSurrModel

See :ref:`multipoint surrogate options <model-multipoint_surrogate>`.)doc")
    .def("ensemble_surrogate",
         [](const Study::ModelFactory& factory,
            const Variables& variables, const Response& response,
            std::optional<std::vector<std::shared_ptr<Model>>> ordered_models,
            std::shared_ptr<Model> truth_model,
            std::optional<std::vector<std::shared_ptr<Model>>> approximation_models,
            const py::object& config, py::kwargs kwargs) {
           const bool ordered_form = ordered_models.has_value();
           const bool explicit_form = truth_model || approximation_models.has_value();
           if (ordered_form == explicit_form)
             throw py::type_error(
               "ensemble_surrogate requires exactly one of ordered_models or "
               "truth_model with optional approximation_models");

           const auto fragment = normalize_factory_config(
             config, kwargs, "ModelFactory.ensemble_surrogate");
           auto model_store = materialize_model(
             nlohmann::json{{"ensemble_surrogate", fragment}});

           if (ordered_form) {
             if (ordered_models->empty())
               throw py::value_error("ordered_models must not be empty");
             for (const auto& model : *ordered_models)
               if (!model)
                 throw py::type_error(
                   "ordered_models must contain non-null Model instances");
             return factory.ensemble_surrogate(
               model_store, std::move(*ordered_models), variables, response);
           }

           if (!truth_model)
             throw py::type_error(
               "truth_model must be a non-null Model when using approximation_models");
           auto approximations = approximation_models.value_or(
             std::vector<std::shared_ptr<Model>>{});
           for (const auto& model : approximations)
             if (!model)
               throw py::type_error(
                 "approximation_models must contain non-null Model instances");
           return factory.ensemble_surrogate(
             model_store, std::move(truth_model), std::move(approximations),
             variables, response);
         },
         py::arg("variables").none(false), py::arg("response").none(false),
         py::kw_only(), py::arg("ordered_models") = py::none(),
         py::arg("truth_model") = py::none(),
         py::arg("approximation_models") = py::none(),
         py::arg("config") = py::none(),
         R"doc(Construct an ensemble surrogate from injected model objects.

Exactly one dependency form is required: provide ``ordered_models`` from low
to high fidelity, or provide ``truth_model`` with optional
``approximation_models``. The final ordered model is the truth model.

Pointer fields omitted from the configuration are filled with API-mode
sentinels during validation; the injected model objects replace those input-file
dependencies. Pass ``config`` or configuration keyword arguments, not both.

:param variables: Variables shared by the ensemble model.
:type variables: dakota.study.Variables
:param response: Response definition shared by the ensemble model.
:type response: dakota.study.Response
:param ordered_models: Models ordered from low to high fidelity.
:type ordered_models: sequence[dakota.study.Model] or None
:param truth_model: Explicit high-fidelity truth model.
:type truth_model: dakota.study.Model or None
:param approximation_models: Unordered approximation models. This may be
    omitted for a resolution-only hierarchy.
:type approximation_models: sequence[dakota.study.Model] or None
:param config: An ``EnsembleSurrogateConfig`` or equivalent dictionary.
:param kwargs: Configuration fields used instead of ``config``.
:returns: The constructed ensemble surrogate.
:rtype: dakota.study.EnsembleSurrModel
:raises TypeError: If dependency forms are missing, mixed, null, or otherwise
    invalid, or if both ``config`` and configuration keywords are supplied.
:raises ValueError: If ``ordered_models`` is empty or configuration validation
    fails.

See :ref:`ensemble surrogate options <model-ensemble_surrogate>`.)doc");
}

void bind_study(py::module_& m)
{
  py::class_<Study>(
    m, "Study",
    "Own the runtime services and factories for a library-mode Dakota study.\n\n"
    "Keep the Study alive while using components created from it. Construct "
    "components from the leaves inward, then pass the top-level method to run().")
    .def(py::init<const StudyConfig&>(), py::arg("config") = StudyConfig{},
         "Create a Study with optional output and run-phase configuration.")
    .def("variables",
         [](const Study& study, const py::object& config, py::kwargs kwargs) {
           return study.variables(
             materialize_variables(normalize_factory_config(config, kwargs, "Study.variables")));
         },
         py::arg("config") = py::none(),
         R"doc(Construct variables from the children of a variables block.

:param config: A ``VariablesConfig`` or equivalent dictionary. Omit it when
    passing configuration fields as keyword arguments.
:param kwargs: Configuration fields used instead of ``config``.
:returns: Variables owned by this study.
:rtype: dakota.study.Variables
:raises TypeError: If both ``config`` and configuration keywords are supplied.

See :ref:`variables options <variables>`.)doc")
    .def("responses",
         [](const Study& study, const Variables& variables,
            const py::object& config, py::kwargs kwargs) {
           return study.responses(
             materialize_responses(normalize_factory_config(config, kwargs, "Study.responses")),
             variables);
         },
         py::arg("variables").none(false), py::arg("config") = py::none(),
         R"doc(Construct a response definition associated with variables.

:param variables: Variables that determine active derivative components.
:type variables: dakota.study.Variables
:param config: A ``ResponsesConfig`` or equivalent dictionary.
:param kwargs: Configuration fields used instead of ``config``.
:returns: A response definition owned by this study.
:rtype: dakota.study.Response
:raises TypeError: If both ``config`` and configuration keywords are supplied.

See :ref:`response options <responses>`.)doc")
    .def("interface",
         [](const Study& study, const py::object& config, py::kwargs kwargs) {
           return study.interface(
             materialize_interface(normalize_factory_config(config, kwargs, "Study.interface")));
         },
         py::arg("config") = py::none(),
         R"doc(Construct a simulation interface from an interface fragment.

:param config: An ``InterfaceConfig`` or equivalent dictionary.
:param kwargs: Configuration fields used instead of ``config``.
:returns: An interface owned by this study.
:rtype: dakota.study.Interface
:raises TypeError: If both ``config`` and configuration keywords are supplied.

See :ref:`interface options <interface>`.)doc")
    .def_property_readonly(
      "method", &Study::method, py::return_value_policy::move,
      py::keep_alive<0, 1>(),
      "Method factory tied to this Study's runtime services.")
    .def_property_readonly(
      "model", &Study::model, py::return_value_policy::move,
      py::keep_alive<0, 1>(),
      "Model factory tied to this Study's runtime services.")
    .def("run",
         [](const Study& study, const std::shared_ptr<Iterator>& iterator) {
           study.run(iterator);
         },
         py::arg("method"),
         R"doc(Execute a method as the top-level iterator.

:param method: Iterator constructed with this study's runtime services.
:type method: dakota.study.Iterator
:returns: None
:raises RuntimeError: If the method is null or uses incompatible services.)doc");
}

} // namespace Dakota::python
