// bindings.cpp
// Python bindings for Dakota Schema Validator

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11_json/pybind11_json.hpp>
#include <dakota/schema_validator.hpp>

namespace py = pybind11;
using namespace dakota::schema;
using json = nlohmann::json;

PYBIND11_MODULE(dakota_schema_validator_cpp, m) {
    m.doc() = "Dakota Schema Validator - C++ JSON Schema validation for Dakota Pydantic models";
    
    // SchemaError struct
    py::class_<SchemaError>(m, "SchemaError")
        .def_readonly("path", &SchemaError::path)
        .def_readonly("message", &SchemaError::message)
        .def_readonly("rule", &SchemaError::rule)
        .def("format", &SchemaError::format)
        .def("__repr__", [](const SchemaError& e) {
            return "<SchemaError: " + e.format() + ">";
        });
    
    // SchemaValidationResult class
    py::class_<SchemaValidationResult>(m, "SchemaValidationResult")
        .def("is_valid", &SchemaValidationResult::is_valid)
        .def("errors", &SchemaValidationResult::errors, py::return_value_policy::reference)
        .def("format", &SchemaValidationResult::format)
        .def("__bool__", &SchemaValidationResult::is_valid)
        .def("__repr__", [](const SchemaValidationResult& r) {
            if (r.is_valid()) {
                return std::string("<SchemaValidationResult: valid>");
            }
            return "<SchemaValidationResult: " + std::to_string(r.errors().size()) + " errors>";
        });
    
    // SchemaValidationException
    py::register_exception<SchemaValidationException>(m, "SchemaValidationException");
    
    // UnionPattern enum
    py::enum_<UnionPattern>(m, "UnionPattern")
        .value("Pattern1", UnionPattern::Pattern1, "Optional with model default")
        .value("Pattern2", UnionPattern::Pattern2, "Optional defaults to None")
        .value("Pattern3", UnionPattern::Pattern3, "Three states with default")
        .value("Pattern4", UnionPattern::Pattern4, "Required union")
        .value("Pattern5", UnionPattern::Pattern5, "Three states no default")
        .value("Pattern6", UnionPattern::Pattern6, "Required but selection optional");
    
    // SchemaValidator class
    py::class_<SchemaValidator>(m, "SchemaValidator")
        .def(py::init<const json&>(), py::arg("schema"),
             "Create validator from schema (dict)")
        .def_static("from_file", &SchemaValidator::from_file, py::arg("path"),
                    "Load validator from JSON schema file")
        .def("validate", &SchemaValidator::validate, py::arg("instance"),
             "Validate instance, return validated/mutated instance. Raises SchemaValidationException on error.")
        .def("validate_safe", [](const SchemaValidator& v, const json& instance) {
            auto [result, validation] = v.validate_safe(instance);
            return py::make_tuple(result, validation);
        }, py::arg("instance"),
           "Validate instance, return (result, validation_result) tuple")
        .def("set_exclude_null", &SchemaValidator::set_exclude_null, py::arg("exclude"),
             "Set whether to exclude null values from output")
        .def_property("exclude_null", 
                      &SchemaValidator::exclude_null, 
                      &SchemaValidator::set_exclude_null,
                      "Whether to exclude null values from output");
    
    // Utility functions
    m.def("parse_ref_name", &parse_ref_name, py::arg("ref"),
          "Parse a $ref path to get the definition name");
    
    m.def("format_path", &format_path, py::arg("parts"),
          "Format path parts as a dot-notation string");
}
