/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaStudyPython.hpp"

#include "IRState.hpp"
#include "DakotaVariables.hpp"

#include <nlohmann/json.hpp>
#include <pybind11_json/pybind11_json.hpp>

namespace Dakota::python {

void bind_variables(py::module_& m)
{
  py::class_<Variables>(m, "Variables", py::module_local())
    .def(py::init([](const nlohmann::json& variables_json) {
      return Variables(materialize_variables(variables_json));
    }))
    .def("num_active_cv", &Variables::cv,
         "Return number of active continuous variables");
}

} // namespace Dakota::python
