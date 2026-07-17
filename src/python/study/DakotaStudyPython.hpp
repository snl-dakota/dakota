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

IRStore materialize_method(const nlohmann::json& method_json);
IRStore materialize_model(const nlohmann::json& model_json);
IRStore materialize_variables(const nlohmann::json& variables_json);
IRStore materialize_interface(const nlohmann::json& interface_json);
IRStore materialize_responses(const nlohmann::json& responses_json);

void require_keyword(const nlohmann::json& block_json, const char* keyword,
                     const char* factory_name);

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
