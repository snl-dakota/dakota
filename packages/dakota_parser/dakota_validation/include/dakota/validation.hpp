// dakota/validation.hpp
// Core validation functions for Dakota models
// Used by both Python (via pybind11) and C++ DSL reader

#ifndef DAKOTA_VALIDATION_HPP
#define DAKOTA_VALIDATION_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <stdexcept>

namespace dakota::validation {

using json = nlohmann::json;

/// Exception type for validation failures.
/// pybind11 translates this to Python's ValueError.
/// 
/// Note: Constructor and destructor are defined out-of-line in dakota_validation.cpp
/// to anchor the typeinfo to that translation unit. This ensures exception catching
/// works correctly across shared library boundaries.
class ValidationError : public std::runtime_error {
public:
    explicit ValidationError(const std::string& msg);
    ~ValidationError() override;
};

// ============================================================================
// Standard Validator Signature
// ============================================================================

/// Standard signature for all validation functions.
/// 
/// @param instance   The full model instance as JSON
/// @param fields     Field names used by this validator (from validationFields)
/// @param literals   Literal values used by this validator (from validationLiterals, JSON array)
/// @param context    Validation context for error messages (from validationContext)
/// @return JSON object containing mutations to apply (empty if no changes)
/// @throws ValidationError if validation fails
using ValidatorFn = std::function<json(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
)>;

// ============================================================================
// Validator Registry
// ============================================================================

/// Registry for looking up validators by name.
/// 
/// The DSL reader uses this to dispatch validation based on JSON schema's
/// x-model-validations entries.
class ValidatorRegistry {
public:
    /// Get the singleton instance
    static ValidatorRegistry& instance();
    
    /// Register a validator function by name
    void register_validator(const std::string& name, ValidatorFn fn);
    
    /// Check if a validator is registered
    bool has_validator(const std::string& name) const;
    
    /// Get a validator by name (throws if not found)
    ValidatorFn get_validator(const std::string& name) const;
    
    /// Execute a validator by name
    /// 
    /// @param instance   The model instance as JSON
    /// @param rule_name  The validator name (validationRuleName)
    /// @param fields     Field names (validationFields)
    /// @param literals   Literal values (validationLiterals as JSON array)
    /// @param context    Validation context (validationContext)
    /// @return JSON object containing mutations
    /// @throws std::runtime_error if validator not found
    /// @throws ValidationError if validation fails
    json validate(
        const json& instance,
        const std::string& rule_name,
        const std::vector<std::string>& fields,
        const json& literals,
        const std::string& context
    ) const;
    
    /// Execute all validations from a schema's x-model-validations array
    /// 
    /// @param instance     The model instance as JSON (will be mutated)
    /// @param validations  The x-model-validations array from JSON schema
    /// @return The mutated instance
    json validate_all(json instance, const json& validations) const;

private:
    ValidatorRegistry();
    std::unordered_map<std::string, ValidatorFn> validators_;
};

// ============================================================================
// Built-in Validators
// ============================================================================

/// Validate that a list field has length equal to a target field's value.
/// 
/// Fields: [list_field, target_field]
/// Literals: []
/// 
/// Both list_field and target_field support dotted paths for accessing
/// nested model fields (e.g., "child.values", "config.size").
/// 
/// Skips validation if list field is null/missing (at any path level).
/// 
/// @throws ValidationError if list length doesn't match target
json compare_length(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Validate that two list fields have equal length.
/// 
/// Fields: [list_a, list_b]
/// Literals: []
/// 
/// Both fields support dotted paths. Skips validation if either field
/// is null/missing.
/// 
/// @throws ValidationError if list lengths differ
json compare_list_lengths(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Validate that a list field has length 1 (auto-expand) or equals target.
/// 
/// Fields: [list_field, target_field]
/// Literals: []
/// 
/// Both list_field and target_field support dotted paths for accessing
/// nested model fields (e.g., "child.values", "config.size").
/// 
/// If length is 1, returns mutation to expand list to target length.
/// Note: When list_field is a dotted path, the mutation key will be
/// the full dotted path; the caller must apply it appropriately.
/// 
/// Skips validation if list field is null/missing.
/// 
/// @throws ValidationError if list length is neither 1 nor target
json compare_length_one(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Set default lower/upper bounds when not provided (real-valued).
/// 
/// Fields: [] (lower_bounds, upper_bounds, count are hardcoded)
/// Literals: [lower_default, upper_default] (as doubles)
/// 
/// @return Mutations for lower_bounds and/or upper_bounds if they were null
json default_bounds_real(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Set default lower/upper bounds when not provided (integer-valued).
/// 
/// Fields: [] (lower_bounds, upper_bounds, count are hardcoded)
/// Literals: [lower_default, upper_default] (as ints)
/// 
/// @return Mutations for lower_bounds and/or upper_bounds if they were null
json default_bounds_int(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Set initial point/state to midpoint of bounds, or clamp existing values (real-valued).
/// 
/// Fields: [target_field] - The field to set (e.g., "initial_point" or "initial_state")
/// Literals: []
/// 
/// Hardcoded field names: lower_bounds, upper_bounds, count
/// 
/// If target field is null:
///   - Sets each element to midpoint of (lower, upper)
///   - If one bound is infinite, uses the finite bound
///   - If both infinite, uses 0.0
/// 
/// If target field exists:
///   - Clamps each value to [lower, upper]
/// 
/// @return Mutation for target field if set or clamped
json default_initial_point_real(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Set initial point/state to midpoint of bounds, or clamp existing values (integer-valued).
/// 
/// Fields: [target_field] - The field to set (e.g., "initial_point" or "initial_state")
/// Literals: []
/// 
/// Hardcoded field names: lower_bounds, upper_bounds, count
/// 
/// If target field is null:
///   - Sets each element to integer midpoint of (lower, upper)
/// 
/// If target field exists:
///   - Clamps each value to [lower, upper]
/// 
/// @return Mutation for target field if set or clamped
json default_initial_point_int(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

// ============================================================================
// List Element Validators
// ============================================================================

/// Validate that all elements in a list are >= 0.
/// 
/// Fields: [list_field]
/// Literals: []
/// 
/// Used by: method_usharray, method_szarray, model_usharray
/// 
/// Skips validation if list field is null/missing.
/// 
/// @throws ValidationError if any element < 0
json check_nonnegative_list(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Validate that all elements in a list are > 0.
/// 
/// Fields: [list_field]
/// Literals: []
/// 
/// Used by: model_id_index_set
/// 
/// Skips validation if list field is null/missing.
/// 
/// @throws ValidationError if any element <= 0
json check_positive_list(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Validate that all elements in a list are in [0, 1].
/// 
/// Fields: [list_field]
/// Literals: []
/// 
/// Used by: method_resplevels01
/// 
/// Skips validation if list field is null/missing.
/// 
/// @throws ValidationError if any element outside [0, 1]
json check_probability_list(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Validate that all elements in a real list are above a bound.
/// 
/// Fields: [list_field]
/// Literals: [lower_bound, inclusive (optional, default false)]
/// 
/// When inclusive is false (default): checks element > lower_bound.
/// When inclusive is true: checks element >= lower_bound.
/// 
/// Used by: var_RealLb
/// 
/// Skips validation if list field is null/missing.
/// 
/// @throws ValidationError if any element violates the bound
json check_real_lower_bound(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Validate that all elements in a real list are below a bound.
/// 
/// Fields: [list_field]
/// Literals: [upper_bound, inclusive (optional, default false)]
/// 
/// When inclusive is false (default): checks element < upper_bound.
/// When inclusive is true: checks element <= upper_bound.
/// 
/// Used by: var_RealUb
/// 
/// Skips validation if list field is null/missing.
/// 
/// @throws ValidationError if any element violates the bound
json check_real_upper_bound(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Validate that all elements in an integer list are > a bound.
/// 
/// Fields: [list_field]
/// Literals: [lower_bound]
/// 
/// Used by: var_IntLb
/// 
/// Skips validation if list field is null/missing.
/// 
/// @throws ValidationError if any element <= lower_bound
json check_int_lower_bound(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

// ============================================================================
// Cross-Field Validators
// ============================================================================

/// Validate that the sum of a list equals a target field's value.
/// 
/// Fields: [num_list_field, levels_list_field]
/// Literals: []
/// 
/// Used by: method_num_resplevs
/// The sum of integers in num_list_field must equal the length of levels_list_field.
/// 
/// Skips validation if either field is null/missing.
/// 
/// @throws ValidationError if sum doesn't match target length
json check_sum_equals_length(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Validate trust_region parameters.
/// 
/// Fields: [initial_size_field, minimum_size_field, contract_threshold_field,
///          expand_threshold_field, contraction_factor_field, expansion_factor_field]
/// Literals: []
/// 
/// Checks:
/// - Each initial_size in (0, 1]
/// - Each initial_size >= minimum_size
/// - minimum_size in [0, 1]
/// - 0 < contract_threshold <= expand_threshold <= 1
/// - contraction_factor in (0, 1]
/// - expansion_factor >= 1
/// 
/// @throws ValidationError if any constraint is violated
json trust_region_validate(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Validate interface block parameters
///
/// Fields: no supplied fiels
/// Literals: no supplied literals
///
/// Checks:
/// - Batch concurrency is not used with multiple analysis drivers
/// - Batch concurrency is not used with input/output filters
/// - Batch size is not 1
/// - failure_capture modes are valid in batch mode
/// - Either analysis_drivers or algebraic_mappings block is present
/// - If asynchronous concurrency is used, both evaluation_concurrency and analysis_concurrency are not 1
/// - 'link_files' is not used on Windows platforms
/// @throws ValidationError if any constraint is violated

json check_interface_block(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

json check_analysis_drivers(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

// ============================================================================
// Response Block Validators
// Based on make_response_defaults() from NIDRProblemDescDB.cpp
// ============================================================================

/// Set default response descriptors when not provided.
/// 
/// This validator runs on ResponsesConfig and generates default descriptors
/// based on the response_type Union variant (nested structure).
/// 
/// Generates labels based on response type:
/// - calibration_terms: least_sq_term_N, nln_ineq_con_N, nln_eq_con_N
/// - objective_functions: obj_fn or obj_fn_N, nln_ineq_con_N, nln_eq_con_N  
/// - response_functions: response_fn_N
///
/// Fields: [] (navigates nested response_type structure)
/// Literals: []
/// 
/// @return Mutations for 'descriptors' field if set
json check_response_descriptors(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Validate that vendor numerical gradients use single fd_step_size.
/// 
/// Checks gradient_type.numerical_gradients.method_source or
/// gradient_type.mixed_gradients.method_source for vendor key.
/// 
/// Fields: [] (navigates gradient_type structure)
/// Literals: []
/// 
/// @throws ValidationError if vendor gradients have multiple step sizes
json check_fd_gradient_step_size(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Validate mixed gradient ID lists cover all functions exactly once.
/// 
/// When gradient_type contains mixed_gradients, validates that:
/// - All IDs in id_analytic_gradients and id_numerical_gradients are in [1, num_functions]
/// - Every function index is covered exactly once across both lists
/// 
/// Computes num_functions from nested response_type structure.
/// 
/// Fields: [] (navigates gradient_type.mixed_gradients structure)
/// Literals: []
/// 
/// @throws ValidationError if IDs out of range or coverage incomplete/duplicate
json check_mixed_gradients(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Validate mixed Hessian ID lists cover all functions exactly once.
/// 
/// When hessian_type contains mixed_hessians, validates that:
/// - All IDs in id_analytic_hessians, id_numerical_hessians.values, id_quasi_hessians.values 
///   are in [1, num_functions]
/// - Every function index is covered exactly once across all three lists
/// 
/// Computes num_functions from nested response_type structure.
/// 
/// Fields: [] (navigates hessian_type.mixed_hessians structure)
/// Literals: []
/// 
/// @throws ValidationError if IDs out of range or coverage incomplete/duplicate
json check_mixed_hessians(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Set default inequality constraint bounds when not provided.
/// 
/// Applied to nested constraint models like:
/// - ObjectiveFunctionsNonlinearInequalityConstraints
/// - CalibrationTermsNonlinearInequalityConstraints
/// 
/// Sets defaults:
/// - lower_bounds: -inf for each constraint
/// - upper_bounds: 0.0 for each constraint (one-sided g(x) <= 0)
/// 
/// Fields: [] (uses: count, lower_bounds, upper_bounds)
/// Literals: []
/// 
/// @return Mutations for bounds that were set to defaults
json default_inequality_bounds(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Set default equality constraint targets when not provided.
/// 
/// Applied to nested constraint models like:
/// - ObjectiveFunctionsNonlinearEqualityConstraints
/// - CalibrationTermsNonlinearEqualityConstraints
/// 
/// Sets defaults:
/// - targets: 0.0 for each constraint
/// 
/// Fields: [] (uses: count, targets)
/// Literals: []
/// 
/// @return Mutations for targets if set to defaults
json default_equality_targets(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Validate that descriptors length is 0 or equals total number of functions.
/// 
/// Runs on ResponsesConfig. Ensures user-provided descriptors list has
/// exactly the right length to match all functions (objectives/calibration
/// terms plus constraints).
/// 
/// Fields: [] (navigates nested response_type structure)
/// Literals: []
/// 
/// @throws ValidationError if descriptors length doesn't match function count
json check_response_descriptors_length(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Validate that descriptors follow naming rules.
/// 
/// Rules:
/// - Not zero-length
/// - No whitespace
/// - Cannot be a number (would be ambiguous in Dakota input)
/// - Optionally checks for uniqueness
/// 
/// fields[0] = field name (defaults to "descriptors" if not provided)
/// literals[0] = check_uniqueness (boolean, defaults to false)
/// 
/// @throws ValidationError if any descriptor violates rules
json check_descriptors_valid(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Generic validator: every element of a string list must be in a permitted set.
/// 
/// fields[0] = field name (e.g., "scale_types", "flags")
/// literals = permitted values (e.g., ["value", "auto", "log", "none"])
/// 
/// @throws ValidationError if any element is not in permitted values
json check_permitted_values(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Generic validator: if any scale_types are 'value', scales must be provided.
/// 
/// fields[0] = scale_types field name
/// fields[1] = scales field name
/// 
/// @throws ValidationError if 'value' scale type but scales not provided
json check_scales_required(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Validate that lower_bounds are element-wise less than or equal to upper_bounds.
/// 
/// Applied to NonlinearInequalityConstraints.
/// 
/// Fields: [] (uses: lower_bounds, upper_bounds)
/// Literals: []
/// 
/// @throws ValidationError if any lower bound exceeds corresponding upper bound
json check_constraint_bounds_ordering(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

// ============================================================================
// Variable validators
// ============================================================================

/// Validate that descriptors length is 0 or equals count.
/// 
/// Applied to individual variable type models.
/// 
/// Fields: [] (uses: descriptors, count)
/// Literals: []
/// 
/// @throws ValidationError if descriptors length doesn't match count
json check_variable_descriptors_length(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Generate default variable descriptors with a specified prefix.
/// 
/// Format: prefix + "1", prefix + "2", etc.
/// 
/// Fields: []
/// Literals: [prefix] - e.g., "cdv_", "nuv_"
/// 
/// @return Mutations for descriptors if set to defaults
json default_variable_descriptors(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Validate that lower_bounds are element-wise less than upper_bounds.
/// 
/// Applied to variable types with user-provided bounds.
/// 
/// Fields: [] (uses: lower_bounds, upper_bounds)
/// Literals: []
/// 
/// @throws ValidationError if any lower bound exceeds upper bound
json check_variable_bounds_ordering(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Validate linear inequality bounds ordering.
/// 
/// Fields: [] (uses: linear_inequality_lower_bounds, linear_inequality_upper_bounds)
/// Literals: []
/// 
/// @throws ValidationError if any lower bound exceeds upper bound
json check_linear_inequality_bounds_ordering(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Check that all variable descriptors are unique across all variable types.
/// 
/// Applied to VariablesConfig.
/// 
/// Fields: []
/// Literals: []
/// 
/// @throws ValidationError if duplicate descriptor found
json check_all_variable_descriptors_unique(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Validate adjacency_matrix length and contents for categorical set variables.
///
/// Checks:
/// 1. All entries in adjacency_matrix are 0 or 1
/// 2. Total length equals sum of k_i^2 for each categorical variable,
///    where k_i is the number of elements for that variable
///
/// The number of categorical variables is determined by counting T/t entries
/// in the flags field. If flags_path is empty, all variables are categorical
/// (used for string set types which have no flags).
///
/// k_i is taken from elements_per_variable[i] if provided, otherwise
/// computed as len(elements) / count (uniform distribution).
///
/// Fields: [adjacency_matrix_path, flags_path, elements_per_variable_path,
///          elements_path, count_path]
///   - flags_path may be "" to indicate all variables are categorical
/// Literals: []
///
/// @throws ValidationError if entries are not 0/1 or length is wrong
json check_adjacency_matrix(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

}  // namespace dakota::validation

#endif // DAKOTA_VALIDATION_HPP
