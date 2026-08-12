/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaStudyPython.hpp"

#include <pybind11_json/pybind11_json.hpp>

namespace Dakota::python {
namespace {

nlohmann::json validate_fragment(const py::object& value, const char* function_name)
{
  py::module_ validation = py::module_::import("dakota.study._validation");
  return validation.attr(function_name)(value).cast<nlohmann::json>();
}

} // namespace

nlohmann::json validate_variables_fragment(const py::object& value)
{ return validate_fragment(value, "validate_variables_fragment"); }

nlohmann::json validate_responses_fragment(const py::object& value)
{ return validate_fragment(value, "validate_responses_fragment"); }

nlohmann::json validate_interface_fragment(const py::object& value)
{ return validate_fragment(value, "validate_interface_fragment"); }

nlohmann::json validate_sampling_fragment(const py::object& value)
{ return validate_fragment(value, "validate_sampling_fragment"); }

nlohmann::json validate_dot_bfgs_fragment(const py::object& value)
{ return validate_fragment(value, "validate_dot_bfgs_fragment"); }

nlohmann::json validate_multi_start_fragment(const py::object& value)
{ return validate_fragment(value, "validate_multi_start_fragment"); }

nlohmann::json validate_simulation_model_fragment(const py::object& value)
{ return validate_fragment(value, "validate_simulation_model_fragment"); }

nlohmann::json validate_nested_model_fragment(const py::object& value)
{ return validate_fragment(value, "validate_nested_model_fragment"); }

PYBIND11_MODULE(_study, m) {
  m.doc() = "Dependency-injection/library-mode Dakota study construction.";

  Dakota::python::bind_study_config(m);
  Dakota::python::bind_variables(m);
  Dakota::python::bind_response(m);
  Dakota::python::bind_interface(m);
  Dakota::python::bind_models(m);
  Dakota::python::bind_iterators(m);
  Dakota::python::bind_study_factories(m);
  Dakota::python::bind_study(m);
}

} // namespace Dakota::python
