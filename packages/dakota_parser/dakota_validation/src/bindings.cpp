// bindings.cpp
// pybind11 bindings for Dakota validation functions

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "pybind11_json/pybind11_json.hpp"
#include <dakota/validation.hpp>
#include <dakota/variable_validators.hpp>
#include <dakota/computed_fields.hpp>

namespace py = pybind11;
using namespace dakota::validation;
using namespace dakota::computed_fields;

PYBIND11_MODULE(dakota_validation_cpp, m) {
    m.doc() = R"pbdoc(
        Dakota validation functions implemented in C++
        
        All validators follow a standard signature:
            validate(instance, fields, literals, context) -> mutations
        
        Where:
            instance: The full model as a dict
            fields: List of field names (from validationFields)
            literals: List of literal values (from validationLiterals)
            context: Validation context string (from validationContext)
            mutations: Dict of fields to update (empty if no changes)
        
        ValidationError is raised (as ValueError in Python) on failure.
    )pbdoc";
    
    // Register ValidationError as a Python exception derived from ValueError.
    // The ValidationError class has out-of-line constructor/destructor in 
    // libdakota_validation, which anchors the typeinfo there. This ensures
    // exception catching works correctly across shared library boundaries.
    static py::exception<ValidationError> exc(m, "ValidationError", PyExc_ValueError);

    // ========================================================================
    // Core validators with standard signature
    // ========================================================================
    
    m.def("compare_length", &compare_length,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Validate that a list field has length equal to a target field's value.
            
            Fields: [list_field, target_field]
            Literals: []
        )pbdoc"
    );
    
    m.def("compare_list_lengths", &compare_list_lengths,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Validate that two list fields have equal length.
            
            Fields: [list_a, list_b]
            Literals: []
        )pbdoc"
    );
    
    m.def("compare_length_one", &compare_length_one,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Validate list length is 1 (auto-expand) or equals target.
            
            Fields: [list_field, target_field]
            Literals: []
        )pbdoc"
    );
    
    m.def("default_bounds_real", &default_bounds_real,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Set default lower/upper bounds when not provided (real-valued).
            
            Fields: [lower_bounds_field, upper_bounds_field, count_field]
            Literals: [lower_default, upper_default] (as floats)
        )pbdoc"
    );
    
    m.def("default_bounds_int", &default_bounds_int,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Set default lower/upper bounds when not provided (integer-valued).
            
            Fields: [lower_bounds_field, upper_bounds_field, count_field]
            Literals: [lower_default, upper_default] (as ints)
        )pbdoc"
    );
    
    m.def("default_initial_point_real", &default_initial_point_real,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Set initial_point to midpoint of bounds, or clamp existing values (real-valued).
            
            Fields: [initial_point_field, lower_bounds_field, upper_bounds_field, count_field]
            Literals: []
        )pbdoc"
    );
    
    m.def("default_initial_point_int", &default_initial_point_int,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Set initial_point to midpoint of bounds, or clamp existing values (integer-valued).
            
            Fields: [initial_point_field, lower_bounds_field, upper_bounds_field, count_field]
            Literals: []
        )pbdoc"
    );
    
    // ========================================================================
    // List element validators
    // ========================================================================
    
    m.def("check_nonnegative_list", &check_nonnegative_list,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Validate that all elements in a list are >= 0.
            
            Fields: [list_field]
            Literals: []
            
            Used by: method_usharray, method_szarray, model_usharray
        )pbdoc"
    );
    
    m.def("check_positive_list", &check_positive_list,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Validate that all elements in a list are > 0.
            
            Fields: [list_field]
            Literals: []
            
            Used by: model_id_index_set
        )pbdoc"
    );
    
    m.def("check_probability_list", &check_probability_list,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Validate that all elements in a list are in [0, 1].
            
            Fields: [list_field]
            Literals: []
            
            Used by: method_resplevels01
        )pbdoc"
    );
    
    m.def("check_real_lower_bound", &check_real_lower_bound,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Validate that all elements in a real list are above a bound.
            
            Fields: [list_field]
            Literals: [lower_bound, inclusive (optional, default false)]
            
            When inclusive is false (default): checks element > lower_bound.
            When inclusive is true: checks element >= lower_bound.
            
            Used by: var_RealLb
        )pbdoc"
    );
    
    m.def("check_real_upper_bound", &check_real_upper_bound,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Validate that all elements in a real list are below a bound.
            
            Fields: [list_field]
            Literals: [upper_bound, inclusive (optional, default false)]
            
            When inclusive is false (default): checks element < upper_bound.
            When inclusive is true: checks element <= upper_bound.
            
            Used by: var_RealUb
        )pbdoc"
    );
    
    m.def("check_int_lower_bound", &check_int_lower_bound,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Validate that all elements in an integer list are > a bound.
            
            Fields: [list_field]
            Literals: [lower_bound]
            
            Used by: var_IntLb
        )pbdoc"
    );
    
    // ========================================================================
    // Cross-field validators
    // ========================================================================
    
    m.def("check_sum_equals_length", &check_sum_equals_length,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Validate that the sum of a list equals another list's length.
            
            Fields: [num_list_field, levels_list_field]
            Literals: []
            
            Used by: method_num_resplevs
        )pbdoc"
    );
    
    m.def("trust_region_validate", &trust_region_validate,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Validate trust_region parameters.
            
            Fields: [initial_size_field, minimum_size_field, contract_threshold_field,
                     expand_threshold_field, contraction_factor_field, expansion_factor_field]
            Literals: []
            
            Checks:
            - Each initial_size in (0, 1]
            - Each initial_size >= minimum_size
            - minimum_size in [0, 1]
            - 0 < contract_threshold <= expand_threshold <= 1
            - contraction_factor in (0, 1]
            - expansion_factor >= 1
        )pbdoc"
    );
 
    m.def("check_interface_block", &check_interface_block,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Validate interface block parameters.
            
            Fields: []
            Literals: []
            
            Checks:
            - Batch concurrency is not used with multiple analysis drivers.
            - Batch concurrency is not used with input/output filters.
            - Batch size is not 1.
            - failure_capture modes are valid in batch mode
            - Either analysis_drivers or algebraic_mappings block is present
            - If asynchronous concurrency is used, both evaluation_concurrency and analysis_concurrency are not 1
            - 'link_files' is not used on Windows platforms
        )pbdoc"
    );

    m.def("check_analysis_drivers", &check_analysis_drivers,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Validate analysis_drivers parameters.

            Fields: []
            Literals: []

            Checks:
            - drivers contains at least one entry
            - if analysis_components is present, its length is evenly divisible
              by the number of drivers
        )pbdoc"
    );
    
    // ========================================================================
    // Response Block Validators
    // ========================================================================
    
    m.def("check_response_descriptors", &check_response_descriptors,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Set default response descriptors when not provided.
            
            Generates labels based on response type:
            - calibration_terms: least_sq_term_N, nln_ineq_con_N, nln_eq_con_N
            - objective_functions: obj_fn or obj_fn_N, nln_ineq_con_N, nln_eq_con_N
            - response_functions: response_fn_N
            
            Fields: [] (navigates nested response_type structure)
            Literals: []
        )pbdoc"
    );
    
    m.def("check_fd_gradient_step_size", &check_fd_gradient_step_size,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Validate that vendor numerical gradients use single fd_step_size.
            
            Fields: [] (navigates gradient_type structure)
            Literals: []
        )pbdoc"
    );
    
    m.def("check_mixed_gradients", &check_mixed_gradients,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Validate mixed gradient ID lists cover all functions exactly once.
            
            Fields: [] (navigates gradient_type.mixed_gradients structure)
            Literals: []
        )pbdoc"
    );
    
    m.def("check_mixed_hessians", &check_mixed_hessians,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Validate mixed Hessian ID lists cover all functions exactly once.
            
            Fields: [] (navigates hessian_type.mixed_hessians structure)
            Literals: []
        )pbdoc"
    );
    
    m.def("default_inequality_bounds", &default_inequality_bounds,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Set default inequality constraint bounds when not provided.
            
            Applied to nested constraint models.
            Sets lower_bounds to -inf and upper_bounds to 0.0.
            
            Fields: [] (uses: count, lower_bounds, upper_bounds)
            Literals: []
        )pbdoc"
    );
    
    m.def("default_equality_targets", &default_equality_targets,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Set default equality constraint targets when not provided.
            
            Applied to nested constraint models.
            Sets targets to 0.0.
            
            Fields: [] (uses: count, targets)
            Literals: []
        )pbdoc"
    );
    
    m.def("check_response_descriptors_length", &check_response_descriptors_length,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Validate that descriptors length is 0 or equals total number of functions.
            
            Fields: [] (navigates nested response_type structure)
            Literals: []
        )pbdoc"
    );
    
    m.def("check_descriptors_valid", &check_descriptors_valid,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Validate that descriptors follow naming rules.
            
            Rules: no whitespace, not empty, cannot be a number.
            
            fields[0] = field name (defaults to "descriptors")
            literals[0] = check_uniqueness (boolean, defaults to false)
        )pbdoc"
    );
    
    m.def("check_permitted_values", &check_permitted_values,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Generic validator: every element of a string list must be in a permitted set.
            
            fields[0] = field name (e.g., "scale_types", "flags")
            literals = permitted values (e.g., ["value", "auto", "log", "none"])
        )pbdoc"
    );
    
    m.def("check_scales_required", &check_scales_required,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Generic validator: if any scale_types are 'value', scales must be provided.
            
            fields[0] = scale_types field name
            fields[1] = scales field name
        )pbdoc"
    );
    
    m.def("check_constraint_bounds_ordering", &check_constraint_bounds_ordering,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Validate that lower_bounds are element-wise <= upper_bounds.
            
            Applied to NonlinearInequalityConstraints.
            
            Fields: [] (uses: lower_bounds, upper_bounds)
            Literals: []
        )pbdoc"
    );
    
    // Variable validators
    m.def("check_variable_descriptors_length", &check_variable_descriptors_length,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Validate that descriptors length is 0 or equals count.
            
            Fields: [] (uses: descriptors, count)
            Literals: []
        )pbdoc"
    );
    
    m.def("default_variable_descriptors", &default_variable_descriptors,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Generate default variable descriptors with a specified prefix.
            
            Fields: []
            Literals: [prefix] - e.g., "cdv_", "nuv_"
        )pbdoc"
    );
    
    m.def("check_variable_bounds_ordering", &check_variable_bounds_ordering,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Validate that lower_bounds <= upper_bounds element-wise.
            
            Fields: [] (uses: lower_bounds, upper_bounds)
            Literals: []
        )pbdoc"
    );
    
    m.def("check_linear_inequality_bounds_ordering", &check_linear_inequality_bounds_ordering,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Validate linear inequality bounds ordering.
            
            Fields: [] (uses: linear_inequality_lower_bounds, linear_inequality_upper_bounds)
            Literals: []
        )pbdoc"
    );
    
    m.def("check_all_variable_descriptors_unique", &check_all_variable_descriptors_unique,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Check that all variable descriptors are unique across all variable types.
            
            Applied to VariablesConfig.
            
            Fields: []
            Literals: []
        )pbdoc"
    );
    
    m.def("check_adjacency_matrix", &check_adjacency_matrix,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Validate adjacency_matrix length and contents for categorical set variables.
            
            Fields: [adjacency_matrix_path, flags_path, elements_per_variable_path,
                     elements_path, count_path]
              - flags_path may be "" to indicate all variables are categorical
            Literals: []
        )pbdoc"
    );
    
    m.def("check_set_elements_ordering", &check_set_elements_ordering,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Validate that elements are strictly increasing within each variable's subset.
            
            Fields: [elements_per_variable, elements, count]
            Literals: []
        )pbdoc"
    );
    
    m.def("default_set_probabilities", &default_set_probabilities,
        py::arg("instance"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Default or normalize set_probabilities for discrete uncertain set variables.
            
            Fields: [set_probabilities, elements_per_variable, elements, count]
            Literals: []
        )pbdoc"
    );
    
    // ========================================================================
    // Registry for dispatch by name
    // ========================================================================
    
    py::class_<ValidatorRegistry>(m, "ValidatorRegistry",
        R"pbdoc(
            Registry for looking up and executing validators by name.
            
            Used by the DSL reader to dispatch validation based on JSON schema's
            x-model-validations entries.
        )pbdoc")
        .def_static("instance", &ValidatorRegistry::instance,
            py::return_value_policy::reference,
            "Get the singleton registry instance")
        .def("has_validator", &ValidatorRegistry::has_validator,
            py::arg("name"),
            "Check if a validator is registered")
        .def("validate", &ValidatorRegistry::validate,
            py::arg("instance"),
            py::arg("rule_name"),
            py::arg("fields"),
            py::arg("literals"),
            py::arg("context"),
            R"pbdoc(
                Execute a validator by name.
                
                Args:
                    instance: The model instance as a dict
                    rule_name: The validator name (validationRuleName)
                    fields: Field names (validationFields)
                    literals: Literal values (validationLiterals)
                    context: Validation context (validationContext)
                    
                Returns:
                    Dict of mutations to apply
            )pbdoc")
        .def("validate_all", &ValidatorRegistry::validate_all,
            py::arg("instance"),
            py::arg("validations"),
            R"pbdoc(
                Execute all validations from a schema's x-model-validations array.
                
                Args:
                    instance: The model instance as a dict
                    validations: The x-model-validations array from JSON schema
                    
                Returns:
                    The mutated instance dict
            )pbdoc");
    
    // Convenience function for direct registry access
    m.def("validate", 
        [](const json& instance, const std::string& rule_name,
           const std::vector<std::string>& fields, const json& literals,
           const std::string& context) {
            return ValidatorRegistry::instance().validate(
                instance, rule_name, fields, literals, context);
        },
        py::arg("instance"),
        py::arg("rule_name"),
        py::arg("fields"),
        py::arg("literals"),
        py::arg("context"),
        R"pbdoc(
            Execute a validator by name using the global registry.
            
            Convenience function equivalent to:
                ValidatorRegistry.instance().validate(...)
        )pbdoc"
    );
    
    m.def("validate_all",
        [](json instance, const json& validations) {
            return ValidatorRegistry::instance().validate_all(
                std::move(instance), validations);
        },
        py::arg("instance"),
        py::arg("validations"),
        R"pbdoc(
            Execute all validations from a schema's x-model-validations array.
            
            Convenience function equivalent to:
                ValidatorRegistry.instance().validate_all(...)
        )pbdoc"
    );
    
    // ========================================================================
    // Variable Validators - Continuous Aleatory Uncertain
    // ========================================================================
    
    m.def("normal_uncertain_bounds", &normal_uncertain_bounds,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Default and infer lower/upper bounds for normal_uncertain");
    
    m.def("lognormal_uncertain_bounds", &lognormal_uncertain_bounds,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Default and infer lower/upper bounds for lognormal_uncertain (all parameterizations)");
    
    m.def("normal_uncertain_initial", &normal_uncertain_initial,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set default initial_point for normal_uncertain with truncation nudge logic");
    
    m.def("lognormal_uncertain_initial", &lognormal_uncertain_initial,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set default initial_point for lognormal_uncertain");
    
    m.def("uniform_uncertain_initial", &uniform_uncertain_initial,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set/repair initial_point for uniform_uncertain");
    
    m.def("loguniform_uncertain_initial", &loguniform_uncertain_initial,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Validate bounds and set/repair initial_point for loguniform_uncertain");
    
    m.def("triangular_uncertain_initial", &triangular_uncertain_initial,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Validate ordering and set/repair initial_point for triangular_uncertain");
    
    m.def("exponential_uncertain_initial", &exponential_uncertain_initial,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set default initial_point for exponential_uncertain");
    
    m.def("beta_uncertain_initial", &beta_uncertain_initial,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set/repair initial_point for beta_uncertain");
    
    m.def("gamma_uncertain_initial", &gamma_uncertain_initial,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set default initial_point for gamma_uncertain");
    
    m.def("gumbel_uncertain_initial", &gumbel_uncertain_initial,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set default initial_point for gumbel_uncertain");
    
    m.def("frechet_uncertain_initial", &frechet_uncertain_initial,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set default initial_point for frechet_uncertain");
    
    m.def("weibull_uncertain_initial", &weibull_uncertain_initial,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set default initial_point for weibull_uncertain");
    
    // ========================================================================
    // Variable Validators - Discrete Aleatory Uncertain
    // ========================================================================
    
    m.def("poisson_uncertain_initial", &poisson_uncertain_initial,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set default initial_point for poisson_uncertain");
    
    m.def("binomial_uncertain_initial", &binomial_uncertain_initial,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set/repair initial_point for binomial_uncertain");
    
    m.def("negative_binomial_uncertain_initial", &negative_binomial_uncertain_initial,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set/repair initial_point for negative_binomial_uncertain");
    
    m.def("geometric_uncertain_initial", &geometric_uncertain_initial,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set default initial_point for geometric_uncertain");
    
    m.def("hypergeometric_uncertain_initial", &hypergeometric_uncertain_initial,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set/repair initial_point for hypergeometric_uncertain");
    
    // ========================================================================
    // Variable Validators - Epistemic Interval Types
    // ========================================================================
    
    m.def("continuous_interval_uncertain_initial", &continuous_interval_uncertain_initial,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set default initial_point for continuous_interval_uncertain");
    
    m.def("discrete_interval_uncertain_initial", &discrete_interval_uncertain_initial,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set default initial_point for discrete_interval_uncertain");
    
    // ========================================================================
    // Variable Validators - Histogram Types
    // ========================================================================
    
    m.def("histogram_bin_uncertain_initial", &histogram_bin_uncertain_initial,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set default initial_point for histogram_bin_uncertain via probability-weighted mean");
    
    m.def("histogram_point_int_uncertain_initial", &histogram_point_numeric_uncertain_initial,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set default initial_point for histogram_point_uncertain integer");
    
    m.def("histogram_point_real_uncertain_initial", &histogram_point_numeric_uncertain_initial,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set default initial_point for histogram_point_uncertain real");
    
    m.def("histogram_point_str_uncertain_initial", &histogram_point_str_uncertain_initial,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set default initial_point for histogram_point_uncertain string");
    
    // ========================================================================
    // Variable Validators - Discrete Set Types (design, state, uncertain)
    // ========================================================================
    
    m.def("discrete_design_set_int_initial", &default_initial_from_set,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set default initial_point for discrete_design_set integer");
    
    m.def("discrete_design_set_str_initial", &default_initial_from_set,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set default initial_point for discrete_design_set string");
    
    m.def("discrete_design_set_real_initial", &default_initial_from_set,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set default initial_point for discrete_design_set real");
    
    m.def("discrete_state_set_int_initial", &default_initial_from_set,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set default initial_point for discrete_state_set integer");
    
    m.def("discrete_state_set_str_initial", &default_initial_from_set,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set default initial_point for discrete_state_set string");
    
    m.def("discrete_state_set_real_initial", &default_initial_from_set,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set default initial_point for discrete_state_set real");
    
    m.def("uncertain_correlation_matrix_size", &uncertain_correlation_matrix_size,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Validate uncertain_correlation_matrix length equals n^2 for continuous aleatory uncertain variables");

    m.def("discrete_uncertain_set_int_initial", &default_initial_from_set,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set default initial_point for discrete_uncertain_set integer");
    
    m.def("discrete_uncertain_set_str_initial", &default_initial_from_set,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set default initial_point for discrete_uncertain_set string");
    
    m.def("discrete_uncertain_set_real_initial", &default_initial_from_set,
        py::arg("instance"), py::arg("fields"), py::arg("literals"), py::arg("context"),
        "Set default initial_point for discrete_uncertain_set real");
    
    // ========================================================================
    // Computed Fields - Continuous Aleatory Uncertain Bounds
    // ========================================================================
    
    m.def("exponential_lower_bounds", &exponential_lower_bounds,
        py::arg("instance"),
        "Compute lower_bounds for exponential_uncertain (always zeros)");
    
    m.def("exponential_upper_bounds", &exponential_upper_bounds,
        py::arg("instance"),
        "Compute upper_bounds for exponential_uncertain: mean + 3*stdev");
    
    m.def("gamma_lower_bounds", &gamma_lower_bounds,
        py::arg("instance"),
        "Compute lower_bounds for gamma_uncertain (always zeros)");
    
    m.def("gamma_upper_bounds", &gamma_upper_bounds,
        py::arg("instance"),
        "Compute upper_bounds for gamma_uncertain: mean + 3*stdev");
    
    m.def("gumbel_lower_bounds", &gumbel_lower_bounds,
        py::arg("instance"),
        "Compute lower_bounds for gumbel_uncertain: mean - 3*stdev");
    
    m.def("gumbel_upper_bounds", &gumbel_upper_bounds,
        py::arg("instance"),
        "Compute upper_bounds for gumbel_uncertain: mean + 3*stdev");
    
    m.def("frechet_lower_bounds", &frechet_lower_bounds,
        py::arg("instance"),
        "Compute lower_bounds for frechet_uncertain (always zeros)");
    
    m.def("frechet_upper_bounds", &frechet_upper_bounds,
        py::arg("instance"),
        "Compute upper_bounds for frechet_uncertain: mean + 3*stdev");
    
    m.def("weibull_lower_bounds", &weibull_lower_bounds,
        py::arg("instance"),
        "Compute lower_bounds for weibull_uncertain (always zeros)");
    
    m.def("weibull_upper_bounds", &weibull_upper_bounds,
        py::arg("instance"),
        "Compute upper_bounds for weibull_uncertain: mean + 3*stdev");
    
    // ========================================================================
    // Computed Fields - Discrete Aleatory Uncertain Bounds
    // ========================================================================
    
    m.def("poisson_lower_bounds", &poisson_lower_bounds,
        py::arg("instance"),
        "Compute lower_bounds for poisson_uncertain (always zeros)");
    
    m.def("poisson_upper_bounds", &poisson_upper_bounds,
        py::arg("instance"),
        "Compute upper_bounds for poisson_uncertain: ceil(lambda + 3*sqrt(lambda))");
    
    m.def("binomial_lower_bounds", &binomial_lower_bounds,
        py::arg("instance"),
        "Compute lower_bounds for binomial_uncertain (always zeros)");
    
    m.def("binomial_upper_bounds", &binomial_upper_bounds,
        py::arg("instance"),
        "Compute upper_bounds for binomial_uncertain (equals num_trials)");
    
    m.def("negative_binomial_lower_bounds", &negative_binomial_lower_bounds,
        py::arg("instance"),
        "Compute lower_bounds for negative_binomial_uncertain (equals num_trials)");
    
    m.def("negative_binomial_upper_bounds", &negative_binomial_upper_bounds,
        py::arg("instance"),
        "Compute upper_bounds for negative_binomial_uncertain: ceil(mean + 3*stdev)");
    
    m.def("geometric_lower_bounds", &geometric_lower_bounds,
        py::arg("instance"),
        "Compute lower_bounds for geometric_uncertain (always zeros)");
    
    m.def("geometric_upper_bounds", &geometric_upper_bounds,
        py::arg("instance"),
        "Compute upper_bounds for geometric_uncertain: ceil(mean + 3*stdev)");
    
    m.def("hypergeometric_lower_bounds", &hypergeometric_lower_bounds,
        py::arg("instance"),
        "Compute lower_bounds for hypergeometric_uncertain (always zeros)");
    
    m.def("hypergeometric_upper_bounds", &hypergeometric_upper_bounds,
        py::arg("instance"),
        "Compute upper_bounds for hypergeometric_uncertain: min(num_drawn, selected_population)");
    
    // ========================================================================
    // Computed Fields - Histogram Bounds
    // ========================================================================
    
    m.def("histogram_bin_lower_bounds", &histogram_bin_lower_bounds,
        py::arg("instance"),
        "Compute lower_bounds for histogram_bin_uncertain: first abscissa per variable");
    
    m.def("histogram_bin_upper_bounds", &histogram_bin_upper_bounds,
        py::arg("instance"),
        "Compute upper_bounds for histogram_bin_uncertain: last abscissa per variable");
    
    m.def("histogram_point_int_lower_bounds", &histogram_point_int_lower_bounds,
        py::arg("instance"),
        "Compute lower_bounds for histogram_point_uncertain/integer: first abscissa per variable");
    
    m.def("histogram_point_int_upper_bounds", &histogram_point_int_upper_bounds,
        py::arg("instance"),
        "Compute upper_bounds for histogram_point_uncertain/integer: last abscissa per variable");
    
    m.def("histogram_point_str_lower_bounds", &histogram_point_str_lower_bounds,
        py::arg("instance"),
        "Compute lower_bounds for histogram_point_uncertain/string: first abscissa per variable");
    
    m.def("histogram_point_str_upper_bounds", &histogram_point_str_upper_bounds,
        py::arg("instance"),
        "Compute upper_bounds for histogram_point_uncertain/string: last abscissa per variable");
    
    m.def("histogram_point_real_lower_bounds", &histogram_point_real_lower_bounds,
        py::arg("instance"),
        "Compute lower_bounds for histogram_point_uncertain/real: first abscissa per variable");
    
    m.def("histogram_point_real_upper_bounds", &histogram_point_real_upper_bounds,
        py::arg("instance"),
        "Compute upper_bounds for histogram_point_uncertain/real: last abscissa per variable");
    
    // ========================================================================
    // Computed Fields - Interval Uncertain Bounds
    // ========================================================================
    
    m.def("continuous_interval_inferred_lower_bounds", &continuous_interval_inferred_lower_bounds,
        py::arg("instance"),
        "Compute inferred_lower_bounds for continuous_interval_uncertain: min across all intervals");
    
    m.def("continuous_interval_inferred_upper_bounds", &continuous_interval_inferred_upper_bounds,
        py::arg("instance"),
        "Compute inferred_upper_bounds for continuous_interval_uncertain: max across all intervals");
    
    m.def("discrete_interval_inferred_lower_bounds", &discrete_interval_inferred_lower_bounds,
        py::arg("instance"),
        "Compute inferred_lower_bounds for discrete_interval_uncertain: min across all intervals");
    
    m.def("discrete_interval_inferred_upper_bounds", &discrete_interval_inferred_upper_bounds,
        py::arg("instance"),
        "Compute inferred_upper_bounds for discrete_interval_uncertain: max across all intervals");
    
    // ========================================================================
    // Computed Fields - Discrete Set Bounds
    // ========================================================================
    // The same C++ functions serve design, state, and uncertain set types.
    // computed_fields.py calls the generic names.
    
    m.def("discrete_set_int_lower_bounds", &discrete_set_int_lower_bounds,
        py::arg("instance"),
        "Compute lower_bounds for discrete set/integer: first sorted element per variable");
    
    m.def("discrete_set_int_upper_bounds", &discrete_set_int_upper_bounds,
        py::arg("instance"),
        "Compute upper_bounds for discrete set/integer: last sorted element per variable");
    
    m.def("discrete_set_str_lower_bounds", &discrete_set_str_lower_bounds,
        py::arg("instance"),
        "Compute lower_bounds for discrete set/string: first sorted element per variable");
    
    m.def("discrete_set_str_upper_bounds", &discrete_set_str_upper_bounds,
        py::arg("instance"),
        "Compute upper_bounds for discrete set/string: last sorted element per variable");
    
    m.def("discrete_set_real_lower_bounds", &discrete_set_real_lower_bounds,
        py::arg("instance"),
        "Compute lower_bounds for discrete set/real: first sorted element per variable");
    
    m.def("discrete_set_real_upper_bounds", &discrete_set_real_upper_bounds,
        py::arg("instance"),
        "Compute upper_bounds for discrete set/real: last sorted element per variable");
    
    // ========================================================================
    // Computed Field Registry
    // ========================================================================
    
    py::class_<ComputedFieldRegistry>(m, "ComputedFieldRegistry",
        R"pbdoc(
            Registry for looking up and executing computed field functions by name.
            
            Used by the DSL reader to compute derived fields based on JSON schema's
            x-computed-field entries.
        )pbdoc")
        .def_static("instance", &ComputedFieldRegistry::instance,
            py::return_value_policy::reference,
            "Get the singleton registry instance")
        .def("has_function", &ComputedFieldRegistry::has_function,
            py::arg("name"),
            "Check if a function is registered")
        .def("compute", &ComputedFieldRegistry::compute,
            py::arg("instance"),
            py::arg("function_name"),
            R"pbdoc(
                Compute a field by function name.
                
                Args:
                    instance: The model instance as a dict
                    function_name: The function name (from x-computed-field)
                    
                Returns:
                    The computed field value
            )pbdoc");
    
    // Convenience function for computed field registry
    m.def("compute",
        [](const json& instance, const std::string& function_name) {
            return ComputedFieldRegistry::instance().compute(instance, function_name);
        },
        py::arg("instance"),
        py::arg("function_name"),
        R"pbdoc(
            Compute a field by function name using the global registry.
            
            Convenience function equivalent to:
                ComputedFieldRegistry.instance().compute(...)
        )pbdoc"
    );
    
    // Version info
    m.attr("__version__") = "0.3.0";
}
