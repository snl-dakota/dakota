/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaStudyPython.hpp"

#include "EnsembleSurrModel.hpp"
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

#include <memory>
#include <stdexcept>
#include <utility>

namespace Dakota::python {

namespace {

nlohmann::json normalize_ensemble_surrogate_fragment(const py::object& model_fragment)
{
  const nlohmann::json validated =
    validate_ensemble_surrogate_fragment(model_fragment);

  nlohmann::json ensemble_body = nlohmann::json::object();
  if (validated.contains("truth_model_pointer"))
    ensemble_body["truth_model_pointer"] = validated.at("truth_model_pointer");
  if (validated.contains("ordered_model_fidelities"))
    ensemble_body["ordered_model_fidelities"] =
      validated.at("ordered_model_fidelities");

  if (ensemble_body.empty()) {
    throw std::runtime_error(
      "ensemble_surrogate requires either truth_model_pointer or "
      "ordered_model_fidelities.");
  }

  return nlohmann::json{{"ensemble", std::move(ensemble_body)}};
}

} // namespace

void bind_study_factories(py::module_& m)
{
  py::class_<Study::ModelFactory>(m, "ModelFactory")
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
         py::arg("response"))
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
         "Construct a simulation from a config fragment (dict or Pydantic model) "
         "or kwargs, exclusively. An omitted config uses single-model defaults.")
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
         "Construct from a config fragment (dict or Pydantic model) or kwargs, "
         "exclusively. optional_interface may be omitted or None; required "
         "configuration fields still apply.")
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
             materialize_model(nlohmann::json{{"surrogate", {{"ensemble", fragment}}}}),
             std::move(truth_model), std::move(approx_models), variables, response);
         },
         py::arg("truth_model").none(false), py::arg("approx_models").none(false),
         py::arg("variables").none(false), py::arg("response").none(false),
         py::arg("config") = py::none(),
         "Construct an ensemble from truth/approximation models and either a "
         "dict/Pydantic config or kwargs. The legacy config-first overload remains available.")
    .def("ensemble_surrogate",
         [](const Study::ModelFactory& factory,
            const py::object& model_json,
            std::shared_ptr<Model> truth_model,
            std::vector<std::shared_ptr<Model>> approx_models,
            const Variables& variables,
            const Response& response) {
           return factory.ensemble_surrogate(
             materialize_model(nlohmann::json{{"surrogate",
               normalize_ensemble_surrogate_fragment(model_json)}}),
             std::move(truth_model),
             std::move(approx_models), variables, response);
         },
         py::arg("model"), py::arg("truth_model"),
         py::arg("approx_models"), py::arg("variables"),
         py::arg("response"));
}

void bind_study(py::module_& m)
{
  py::class_<Study>(m, "Study")
    .def(py::init<const StudyConfig&>(), py::arg("config") = StudyConfig{})
    .def("variables",
         [](const Study& study, const py::object& config, py::kwargs kwargs) {
           return study.variables(
             materialize_variables(normalize_factory_config(config, kwargs, "Study.variables")));
         },
         py::arg("config") = py::none(),
         "Construct from a config fragment (dict or Pydantic model) or kwargs, "
         "exclusively.")
    .def("responses",
         [](const Study& study, const Variables& variables,
            const py::object& config, py::kwargs kwargs) {
           return study.responses(
             materialize_responses(normalize_factory_config(config, kwargs, "Study.responses")),
             variables);
         },
         py::arg("variables").none(false), py::arg("config") = py::none(),
         "Construct from variables and a config fragment (dict or Pydantic "
         "model) or kwargs, exclusively.")
    .def("interface",
         [](const Study& study, const py::object& config, py::kwargs kwargs) {
           return study.interface(
             materialize_interface(normalize_factory_config(config, kwargs, "Study.interface")));
         },
         py::arg("config") = py::none(),
         "Construct from a config fragment (dict or Pydantic model) or kwargs, "
         "exclusively.")
    .def_property_readonly(
      "method", &Study::method, py::return_value_policy::move,
      py::keep_alive<0, 1>())
    .def_property_readonly(
      "model", &Study::model, py::return_value_policy::move,
      py::keep_alive<0, 1>())
    .def("run",
         [](const Study& study, const std::shared_ptr<Iterator>& iterator) {
           study.run(iterator);
         },
         py::arg("method"));
}

} // namespace Dakota::python
