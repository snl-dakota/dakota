/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaStudyPython.hpp"

#include "DakotaResponse.hpp"

namespace Dakota::python {

void bind_response(py::module_& m)
{
  py::class_<Response>(m, "Response", py::module_local())
    .def("function_value",
         static_cast<const Real& (Response::*)(size_t) const>(
           &Response::function_value),
         py::arg("i"));
}

} // namespace Dakota::python
