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
#include "EnsembleSurrModel.hpp"
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
#include <pybind11/stl.h>

#include <memory>
#include <utility>

namespace Dakota::python {

void bind_study_factories(py::module_& m)
{
  auto method_factory = py::class_<Study::MethodFactory>(m, "MethodFactory");
  method_factory
    .def("sampling",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.sampling(method_json.cast<nlohmann::json>(),
                                   std::move(model));
         },
         py::arg("method"), py::arg("model"));
#ifdef HAVE_DOT
  method_factory
    .def("dot_bfgs",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.dot_bfgs(method_json.cast<nlohmann::json>(),
                                   std::move(model));
         },
         py::arg("method"), py::arg("model"));
#endif
  method_factory
    .def("multi_start",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Iterator> sub_iterator) {
           return factory.multi_start(method_json.cast<nlohmann::json>(),
                                      std::move(sub_iterator));
         },
         py::arg("method"), py::arg("sub_iterator"))
    .def("vector_parameter_study",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.vector_parameter_study(
             method_json.cast<nlohmann::json>(),
             std::move(model));
         },
         py::arg("method"), py::arg("model"))
    .def("list_parameter_study",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.list_parameter_study(
             method_json.cast<nlohmann::json>(),
             std::move(model));
         },
         py::arg("method"), py::arg("model"))
    .def("centered_parameter_study",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.centered_parameter_study(
             method_json.cast<nlohmann::json>(),
             std::move(model));
         },
         py::arg("method"), py::arg("model"))
    .def("multidim_parameter_study",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.multidim_parameter_study(
             method_json.cast<nlohmann::json>(),
             std::move(model));
         },
         py::arg("method"), py::arg("model"))
    .def("richardson_extrap",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.richardson_extrap(
             method_json.cast<nlohmann::json>(),
             std::move(model));
         },
         py::arg("method"), py::arg("model"))
    .def("local_interval_est",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.local_interval_est(
             method_json.cast<nlohmann::json>(),
             std::move(model));
         },
         py::arg("method"), py::arg("model"))
    .def("global_interval_est",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.global_interval_est(
             method_json.cast<nlohmann::json>(),
             std::move(model));
         },
         py::arg("method"), py::arg("model"))
    .def("efficient_global",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.efficient_global(
             method_json.cast<nlohmann::json>(),
             std::move(model));
         },
         py::arg("method"), py::arg("model"))
    .def("npsol_sqp",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.npsol_sqp(
             method_json.cast<nlohmann::json>(),
             std::move(model));
         },
         py::arg("method"), py::arg("model"))
    .def("nl2sol",
         [](const Study::MethodFactory& factory,
            const py::object& method_json,
            std::shared_ptr<Model> model) {
           return factory.nl2sol(
             method_json.cast<nlohmann::json>(),
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
           return factory.single(model_json.cast<nlohmann::json>(),
                                     variables, std::move(interface), response);
         },
         py::arg("model"), py::arg("variables"), py::arg("interface"),
         py::arg("response"))
    .def("nested",
         [](const Study::ModelFactory& factory,
            const py::object& model_json,
            std::shared_ptr<Iterator> sub_iterator,
            std::shared_ptr<Interface> optional_interface,
            const Variables& variables,
            const Response& response) {
           return factory.nested(model_json.cast<nlohmann::json>(),
                                 std::move(sub_iterator),
                                 std::move(optional_interface), variables,
                                 response);
         },
         py::arg("model"), py::arg("sub_iterator"),
         py::arg("optional_interface"), py::arg("variables"),
         py::arg("response"))
    .def("ensemble_surrogate",
         [](const Study::ModelFactory& factory,
            const py::object& model_json,
            std::shared_ptr<Model> truth_model,
            std::vector<std::shared_ptr<Model>> approximation_models,
            const Variables& variables,
            const Response& response) {
           return factory.ensemble_surrogate(
             model_json.cast<nlohmann::json>(), std::move(truth_model),
             std::move(approximation_models), variables, response);
         },
         py::arg("model"), py::arg("truth_model"),
         py::arg("approximation_models"), py::arg("variables"),
         py::arg("response"))
    .def("ensemble_surrogate",
         [](const Study::ModelFactory& factory,
            const py::object& model_json,
            std::vector<std::shared_ptr<Model>> ordered_models,
            const Variables& variables,
            const Response& response) {
           return factory.ensemble_surrogate(
             model_json.cast<nlohmann::json>(), std::move(ordered_models),
             variables, response);
         },
         py::arg("model"), py::arg("ordered_models"),
         py::arg("variables"), py::arg("response"));
}

void bind_study(py::module_& m)
{
  py::class_<Study>(m, "Study")
    .def(py::init<const StudyConfig&>(), py::arg("config") = StudyConfig{})
    .def("variables",
         [](const Study& study, const py::object& variables_json) {
           return study.variables(variables_json.cast<nlohmann::json>());
         },
         py::arg("variables"))
    .def("responses",
         [](const Study& study, const py::object& responses_json,
            const Variables& variables) {
           return study.responses(responses_json.cast<nlohmann::json>(),
                                  variables);
         },
         py::arg("responses"), py::arg("variables"))
    .def("interface",
         [](const Study& study, const py::object& interface_json) {
           return study.interface(interface_json.cast<nlohmann::json>());
         },
         py::arg("interface"))
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
