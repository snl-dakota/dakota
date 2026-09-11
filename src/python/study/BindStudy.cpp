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
#include "EffGlobalMinimizer.hpp"
#ifdef HAVE_DOT
#include "DOTOptimizer.hpp"
#endif
#include "NestedModel.hpp"
#include "NL2SOLLeastSq.hpp"
#include "NPSOLOptimizer.hpp"
#include "NonDLocalSingleInterval.hpp"
#include "ParamStudy.hpp"
#include "RichExtrapVerification.hpp"
#include "NonDLHSSampling.hpp"
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
  auto method_factory = py::class_<Study::MethodFactory>(m, "MethodFactory");
  method_factory
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
         "Construct from a config fragment (dict or Pydantic model) or kwargs, "
         "exclusively.");
#ifdef HAVE_DOT
  method_factory
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
         "Construct from a config fragment (dict or Pydantic model) or kwargs, "
         "exclusively.");
#endif
  method_factory
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
         "Construct from a config fragment (dict or Pydantic model) or kwargs, "
         "exclusively.")
    .def("vector_parameter_study",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.vector_parameter_study(
             materialize_method(nlohmann::json{{"vector_parameter_study",
               validate_vector_parameter_study_fragment(method_json)}}),
             std::move(model));
         },
         py::arg("method"), py::arg("model"))
    .def("list_parameter_study",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.list_parameter_study(
             materialize_method(nlohmann::json{{"list_parameter_study",
               validate_list_parameter_study_fragment(method_json)}}),
             std::move(model));
         },
         py::arg("method"), py::arg("model"))
    .def("centered_parameter_study",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.centered_parameter_study(
             materialize_method(nlohmann::json{{"centered_parameter_study",
               validate_centered_parameter_study_fragment(method_json)}}),
             std::move(model));
         },
         py::arg("method"), py::arg("model"))
    .def("multidim_parameter_study",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.multidim_parameter_study(
             materialize_method(nlohmann::json{{"multidim_parameter_study",
               validate_multidim_parameter_study_fragment(method_json)}}),
             std::move(model));
         },
         py::arg("method"), py::arg("model"))
    .def("richardson_extrap",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.richardson_extrap(
             materialize_method(nlohmann::json{{"richardson_extrap",
               validate_richardson_extrap_fragment(method_json)}}),
             std::move(model));
         },
         py::arg("method"), py::arg("model"))
    .def("local_interval_est",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.local_interval_est(
             materialize_method(nlohmann::json{{"local_interval_est",
               validate_local_interval_est_fragment(method_json)}}),
             std::move(model));
         },
         py::arg("method"), py::arg("model"))
    .def("global_interval_est",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.global_interval_est(
             materialize_method(nlohmann::json{{"global_interval_est",
               validate_global_interval_est_fragment(method_json)}}),
             std::move(model));
         },
         py::arg("method"), py::arg("model"))
    .def("efficient_global",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.efficient_global(
             materialize_method(nlohmann::json{{"efficient_global",
               validate_efficient_global_fragment(method_json)}}),
             std::move(model));
         },
         py::arg("method"), py::arg("model"))
    .def("npsol_sqp",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.npsol_sqp(
             materialize_method(nlohmann::json{{"npsol_sqp",
               validate_npsol_sqp_fragment(method_json)}}),
             std::move(model));
         },
         py::arg("method"), py::arg("model"))
    .def("nl2sol",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.nl2sol(
             materialize_method(nlohmann::json{{"nl2sol",
               validate_nl2sol_fragment(method_json)}}),
             std::move(model));
         },
         py::arg("method"), py::arg("model"));

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
