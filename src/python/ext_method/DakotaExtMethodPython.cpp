/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaExtPythonMethod.hpp"
#include "python_utils.hpp"

using namespace pybind11::literals;
using namespace Dakota;

PYBIND11_MODULE(ext_method, m) {

  // Executor evaluator helper
  py::class_<Dakota::ModelExecutor>(m, "Executor")
    .def("function_value"
         , static_cast<std::vector<double> (Dakota::ModelExecutor::*)(std::vector<double>&)>
                        (&Dakota::ModelExecutor::value)
         , "Return function value for continuous values"
         , py::arg("x"))
    
    .def("function_value"
         , static_cast<VectorXd (Dakota::ModelExecutor::*)(VectorXd&)>
                        (&Dakota::ModelExecutor::value)
         , "Return function value for continuous values using numpy/Eigen data"
         , py::arg("x"))
    
    .def("function_value"
         , static_cast<std::vector<double> (Dakota::ModelExecutor::*)(py::dict &)>
                        (&Dakota::ModelExecutor::value)
         , "Return function value for mixed parameter values"
         , py::arg("vars"))
    
    .def("gradient_values", (&Dakota::ModelExecutor::gradient)
         , "Return function gradients for continuous values"
         , py::arg("x"))
    
    .def("hessian_values", (&Dakota::ModelExecutor::hessian)
         , "Return function hessians for continuous values"
         , py::arg("x"))
    
    .def("evaluate", (&Dakota::ModelExecutor::evaluate)
         , "Return function, gradients, and hessian for continuous values"
         , py::arg("x"), py::arg("asv"))
    
  // Executor output helper
    .def("output_central_moments", &Dakota::ModelExecutor::compute_and_print_moments
         , "Compute and print central moments for response array"
         , py::arg("resp"))

  // Executor output helper
    .def("dak_print", &Dakota::ModelExecutor::dak_print
         , "Print python string to Cout"
         , py::arg("s"))

  // Executor helper alias
    .def("initial_values",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list>(
                ModelUtils::continuous_variables(me.model()));
        })

  // Executor query helpers
    .def("has_gradient", &Dakota::ModelExecutor::has_gradient
         , "Query if the model supports gradient evaluations")

    .def("has_hessian", &Dakota::ModelExecutor::has_hessian
         , "Query if the model supports hessian evaluations")


  // --------------
  // Model wrappers
  // --------------

    .def("tv",
        [](const Dakota::ModelExecutor &me) {
            return ModelUtils::tv(me.model()); }, "returns total number of vars")
        
    .def("cv",
        [](const Dakota::ModelExecutor &me) {
        return ModelUtils::cv(me.model()); }, "returns number of continuous variables")

    .def("div",
        [](const Dakota::ModelExecutor &me) {
        return ModelUtils::div(me.model()); }, "returns number of discrete integer vars")

    .def("dsv",
        [](const Dakota::ModelExecutor &me) {
        return ModelUtils::dsv(me.model()); }, "returns number of discrete string vars")

    .def("drv",
        [](const Dakota::ModelExecutor &me) {
        return ModelUtils::drv(me.model()); }, "returns number of discrete real vars")

    .def("continuous_variables",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list>(
                ModelUtils::continuous_variables(me.model()));
        }, "return the continuous variables")

    .def("discrete_int_variables",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list>(
                ModelUtils::discrete_int_variables(me.model()));
        }, "return the discrete integer variables")

    .def("discrete_string_variables",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list,StringMultiArrayConstView,String>(
                ModelUtils::discrete_string_variables(me.model()));
        }, "return the discrete string variables")

    .def("discrete_real_variables",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list>(
                ModelUtils::discrete_real_variables(me.model()));
        }, "return the discrete real variables")

    .def("continuous_variable_labels",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list,StringMultiArray,String>(
                ModelUtils::continuous_variable_labels(me.model()));
        }, "return the continuous variable labels")

    .def("discrete_int_variable_labels",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list,StringMultiArray,String>(
                ModelUtils::discrete_int_variable_labels(me.model()));
        }, "return the discrete int variable labels")

    .def("discrete_string_variable_labels",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list,StringMultiArray,String>(
                ModelUtils::discrete_string_variable_labels(me.model()));
        }, "return the discrete string variable labels")

    .def("discrete_real_variable_labels",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list,StringMultiArray,String>(
                ModelUtils::discrete_real_variable_labels(me.model()));
        }, "return the discrete real variable labels")

    .def("response_size",
        [](const Dakota::ModelExecutor &me) {
            return ModelUtils::response_size(me.model());
        }, "return the number of responses")

    .def("response_labels",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list,StringArray,String>(
                ModelUtils::response_labels(me.model()));
        }, "return the response labels")

    .def("continuous_lower_bounds",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list>(
                ModelUtils::continuous_lower_bounds(me.model()));
        }, "return the continuous lower bounds")

    .def("continuous_upper_bounds",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list>(
                ModelUtils::continuous_upper_bounds(me.model()));
        }, "return the continuous upper bounds")

    .def("discrete_int_lower_bounds",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list>(
                ModelUtils::discrete_int_lower_bounds(me.model()));
        }, "return the discrete int lower bounds")

    .def("discrete_int_upper_bounds",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list>(
                ModelUtils::discrete_int_upper_bounds(me.model()));
        }, "return the discrete int upper bounds")

    .def("discrete_real_lower_bounds",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list>(
                ModelUtils::discrete_real_lower_bounds(me.model()));
        }, "return the discrete real lower bounds")

    .def("discrete_real_upper_bounds",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list>(
                ModelUtils::discrete_real_upper_bounds(me.model()));
        }, "return the discrete real upper bounds")
    ;
}
