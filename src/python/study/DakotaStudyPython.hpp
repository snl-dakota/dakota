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

// Materialize already-validated Python configuration; no input validation here.
IRStore materialize_method(const nlohmann::json& method_json);
IRStore materialize_model(const nlohmann::json& model_json);
IRStore materialize_variables(const nlohmann::json& variables_json);
IRStore materialize_interface(const nlohmann::json& interface_json);
IRStore materialize_responses(const nlohmann::json& responses_json);

nlohmann::json normalize_factory_config(const py::object& config,
                                        const py::kwargs& kwargs,
                                        const char* factory_name);

nlohmann::json validate_variables_fragment(const py::object& value);
nlohmann::json validate_responses_fragment(const py::object& value);
nlohmann::json validate_interface_fragment(const py::object& value);
nlohmann::json validate_sampling_fragment(const py::object& value);
nlohmann::json validate_vector_parameter_study_fragment(const py::object& value);
nlohmann::json validate_list_parameter_study_fragment(const py::object& value);
nlohmann::json validate_centered_parameter_study_fragment(const py::object& value);
nlohmann::json validate_multidim_parameter_study_fragment(const py::object& value);
nlohmann::json validate_richardson_extrap_fragment(const py::object& value);
nlohmann::json validate_local_interval_est_fragment(const py::object& value);
nlohmann::json validate_global_interval_est_fragment(const py::object& value);
nlohmann::json validate_efficient_global_fragment(const py::object& value);
nlohmann::json validate_npsol_sqp_fragment(const py::object& value);
nlohmann::json validate_nl2sol_fragment(const py::object& value);
nlohmann::json validate_dot_bfgs_fragment(const py::object& value);
nlohmann::json validate_multi_start_fragment(const py::object& value);
nlohmann::json validate_single_fragment(const py::object& value);
nlohmann::json validate_nested_fragment(const py::object& value);
nlohmann::json validate_ensemble_surrogate_fragment(const py::object& value);

void bind_study_config(py::module_& m);
void bind_variables(py::module_& m);
void bind_response(py::module_& m);
void bind_interface(py::module_& m);
void bind_models(py::module_& m);
void bind_iterators(py::module_& m);
void bind_iterator_factories(py::module_& m);
void bind_study_factories(py::module_& m);
void bind_study(py::module_& m);

} // namespace python
} // namespace Dakota
