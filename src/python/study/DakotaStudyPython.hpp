/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#pragma once

#include "generated_ir_table_types.hpp"
#include "dakota_data_types.hpp"

#include <nlohmann/json_fwd.hpp>
#include <pybind11/pybind11.h>

namespace Dakota {

class IRStore;

namespace python {

namespace py = pybind11;
namespace irgen = dakota::irgen;

nlohmann::json validate_variables_fragment(const py::object& value);
nlohmann::json validate_responses_fragment(const py::object& value);
nlohmann::json validate_interface_fragment(const py::object& value);
nlohmann::json validate_sampling_fragment(const py::object& value);
nlohmann::json validate_dot_bfgs_fragment(const py::object& value);
nlohmann::json validate_multi_start_fragment(const py::object& value);
nlohmann::json validate_simulation_model_fragment(const py::object& value);
nlohmann::json validate_nested_model_fragment(const py::object& value);

void bind_study_config(py::module_& m);
void bind_variables(py::module_& m);
void bind_response(py::module_& m);
void bind_interface(py::module_& m);
void bind_models(py::module_& m);
void bind_iterators(py::module_& m);
void bind_study_factories(py::module_& m);
void bind_study(py::module_& m);

} // namespace python
} // namespace Dakota
