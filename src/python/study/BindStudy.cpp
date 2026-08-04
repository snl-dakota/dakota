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
#ifdef HAVE_DOT
#include "DOTOptimizer.hpp"
#endif
#include "NestedModel.hpp"
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
#include <utility>

namespace Dakota::python {

void bind_study_factories(py::module_& m)
{
  auto method_factory = py::class_<Study::MethodFactory>(m, "MethodFactory");
  method_factory
    .def("sampling",
         [](const Study::MethodFactory& factory,
            const nlohmann::json& method_json,
            std::shared_ptr<Model> model) {
           require_keyword(method_json, "sampling", "study.method.sampling");
           return factory.sampling(materialize_method(method_json),
                                   std::move(model));
         },
         py::arg("method"), py::arg("model"));
#ifdef HAVE_DOT
  method_factory
    .def("dot_bfgs",
         [](const Study::MethodFactory& factory,
            const nlohmann::json& method_json,
            std::shared_ptr<Model> model) {
           require_keyword(method_json, "dot_bfgs", "study.method.dot_bfgs");
           return factory.dot_bfgs(materialize_method(method_json),
                                   std::move(model));
         },
         py::arg("method"), py::arg("model"));
#endif
  method_factory
    .def("multi_start",
         [](const Study::MethodFactory& factory,
            const nlohmann::json& method_json,
            std::shared_ptr<Iterator> sub_iterator) {
           require_keyword(method_json, "multi_start",
                           "study.method.multi_start");
           return factory.multi_start(materialize_method(method_json),
                                      std::move(sub_iterator));
         },
         py::arg("method"), py::arg("sub_iterator"));

  py::class_<Study::ModelFactory>(m, "ModelFactory")
    .def("simulation",
         [](const Study::ModelFactory& factory,
            const nlohmann::json& model_json,
            const Variables& variables,
            std::shared_ptr<Interface> interface,
            const Response& response) {
           return factory.simulation(materialize_model(model_json), variables,
                                     std::move(interface), response);
         },
         py::arg("model"), py::arg("variables"), py::arg("interface"),
         py::arg("response"))
    .def("nested",
         [](const Study::ModelFactory& factory,
            const nlohmann::json& model_json,
            std::shared_ptr<Iterator> sub_iterator,
            std::shared_ptr<Interface> optional_interface,
            const Variables& variables,
            const Response& response) {
           require_keyword(model_json, "nested", "study.model.nested");
           return factory.nested(materialize_model(model_json),
                                 std::move(sub_iterator),
                                 std::move(optional_interface), variables,
                                 response);
         },
         py::arg("model"), py::arg("sub_iterator"),
         py::arg("optional_interface"), py::arg("variables"),
         py::arg("response"));
}

void bind_study(py::module_& m)
{
  py::class_<Study>(m, "Study")
    .def(py::init<const StudyConfig&>(), py::arg("config") = StudyConfig{})
    .def("interface",
         [](const Study& study, const nlohmann::json& interface_json) {
           return study.interface(materialize_interface(interface_json));
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
