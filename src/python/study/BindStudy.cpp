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
#include <utility>

namespace Dakota::python {

void bind_study_factories(py::module_& m)
{
  py::class_<Study::ModelFactory>(
    m, "ModelFactory",
    "Construct models that share the owning Study's runtime services.\n\n"
    "Access this factory through Study.model. Inject variables, responses, "
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
         "Construct a simulation model from injected components.\n\n"
         "Pass a dict, SingleConfig, or configuration keyword arguments, "
         "exclusively. An omitted config uses single-model defaults.")
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
         "Construct a nested model from an injected sub-iterator.\n\n"
         "Pass a dict, NestedConfig, or configuration keyword arguments, "
         "exclusively. optional_interface may be omitted or None; required "
         "configuration fields still apply.")
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
         "Construct a global data-fit surrogate.\n\n"
         "truth_model and dace_iterator are optional injected dependencies. "
         "Pass a dict, GlobalSurrogateConfig, or configuration keyword "
         "arguments, exclusively.")
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
         "Construct a local data-fit surrogate from an injected truth model.\n\n"
         "Pass a dict, LocalSurrogateConfig, or configuration keyword "
         "arguments, exclusively.")
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
         "Construct a multipoint surrogate from an injected truth model.\n\n"
         "Pass a dict, MultipointSurrogateConfig, or configuration keyword "
         "arguments, exclusively.")
    .def("ensemble_surrogate",
         [](const Study::ModelFactory& factory,
            std::shared_ptr<Model> truth_model,
            std::vector<std::shared_ptr<Model>> approx_models,
            const Variables& variables, const Response& response,
            const py::object& config, py::kwargs kwargs) {
           for (const auto& approximation : approx_models)
             if (!approximation)
               throw py::type_error("approx_models must contain non-null Model instances");
           const auto fragment = normalize_factory_config(
             config, kwargs, "ModelFactory.ensemble_surrogate");
           return factory.ensemble_surrogate(
             materialize_model(nlohmann::json{{"ensemble_surrogate", fragment}}),
             std::move(truth_model), std::move(approx_models), variables, response);
         },
         py::arg("truth_model").none(false), py::arg("approx_models").none(false),
         py::arg("variables").none(false), py::arg("response").none(false),
         py::arg("config") = py::none(),
         "Construct an ensemble from a truth model and approximation models.\n\n"
         "Pass a dict, EnsembleSurrogateConfig, or configuration keyword "
         "arguments, exclusively. The legacy config-first overload remains available.")
    .def("ensemble_surrogate",
         [](const Study::ModelFactory& factory,
            std::vector<std::shared_ptr<Model>> ordered_models,
            const Variables& variables, const Response& response,
            const py::object& config, py::kwargs kwargs) {
           for (const auto& model : ordered_models)
             if (!model)
               throw py::type_error("ordered_models must contain non-null Model instances");
           const auto fragment = normalize_factory_config(
             config, kwargs, "ModelFactory.ensemble_surrogate");
           return factory.ensemble_surrogate(
             materialize_model(nlohmann::json{{"ensemble_surrogate", fragment}}),
             std::move(ordered_models), variables, response);
         },
         py::arg("ordered_models").none(false), py::arg("variables").none(false),
         py::arg("response").none(false), py::arg("config") = py::none(),
         "Construct an ensemble from models ordered from low to high fidelity.\n\n"
         "The final model is the truth model. Pass a dict, "
         "EnsembleSurrogateConfig, or configuration keyword arguments, exclusively.")
    .def("ensemble_surrogate",
         [](const Study::ModelFactory& factory,
            const py::object& model_json,
            std::shared_ptr<Model> truth_model,
            std::vector<std::shared_ptr<Model>> approx_models,
            const Variables& variables,
            const Response& response) {
           for (const auto& approximation : approx_models)
             if (!approximation)
               throw py::type_error("approx_models must contain non-null Model instances");
           return factory.ensemble_surrogate(
             materialize_model(nlohmann::json{{"ensemble_surrogate",
               validate_ensemble_surrogate_fragment(model_json)}}),
             std::move(truth_model),
             std::move(approx_models), variables, response);
         },
         py::arg("model"), py::arg("truth_model"),
         py::arg("approx_models"), py::arg("variables"),
         py::arg("response"),
         "Construct an ensemble using the legacy config-first calling convention.")
    .def("ensemble_surrogate",
         [](const Study::ModelFactory& factory,
            const py::object& model_json,
            std::vector<std::shared_ptr<Model>> ordered_models,
            const Variables& variables,
            const Response& response) {
           for (const auto& model : ordered_models)
             if (!model)
               throw py::type_error("ordered_models must contain non-null Model instances");
           return factory.ensemble_surrogate(
             materialize_model(nlohmann::json{{"ensemble_surrogate",
               validate_ensemble_surrogate_fragment(model_json)}}),
             std::move(ordered_models),
             variables, response);
         },
         py::arg("model"), py::arg("ordered_models"),
         py::arg("variables"), py::arg("response"),
         "Construct an ordered ensemble using the legacy config-first calling convention.");
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
         "Construct variables from the children of a variables block.\n\n"
         "Pass a dict, VariablesConfig, or configuration keyword arguments, exclusively.")
    .def("responses",
         [](const Study& study, const Variables& variables,
            const py::object& config, py::kwargs kwargs) {
           return study.responses(
             materialize_responses(normalize_factory_config(config, kwargs, "Study.responses")),
             variables);
         },
         py::arg("variables").none(false), py::arg("config") = py::none(),
         "Construct a response definition associated with variables.\n\n"
         "Pass a dict, ResponsesConfig, or configuration keyword arguments, exclusively.")
    .def("interface",
         [](const Study& study, const py::object& config, py::kwargs kwargs) {
           return study.interface(
             materialize_interface(normalize_factory_config(config, kwargs, "Study.interface")));
         },
         py::arg("config") = py::none(),
         "Construct a simulation interface from the children of an interface block.\n\n"
         "Pass a dict, InterfaceConfig, or configuration keyword arguments, exclusively.")
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
         "Execute method as the top-level iterator.\n\n"
         "The method and its dependencies must remain valid and use compatible Study services.");
}

} // namespace Dakota::python
