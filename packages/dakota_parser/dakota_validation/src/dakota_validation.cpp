// dakota_validation.cpp
// Implementation of core validation functions and registry

#define _USE_MATH_DEFINES // required for MSVS
#include <dakota/validation.hpp>
#include <dakota/variable_validators.hpp>
#include <math.h>
#include <cctype>
#include <string>
#include <iostream>
#include <unordered_set>
#include <regex>
#include <limits>

namespace dakota::validation {

// ============================================================================
// ValidationError Implementation
// ============================================================================
// Out-of-line to anchor typeinfo in this translation unit, ensuring
// exception catching works across shared library boundaries.

ValidationError::ValidationError(const std::string& msg) 
    : std::runtime_error(msg) {}

ValidationError::~ValidationError() = default;

// ============================================================================
// ValidatorRegistry Implementation
// ============================================================================

ValidatorRegistry::ValidatorRegistry() {
    // Register built-in validators
    register_validator("compare_len", compare_length);
    register_validator("compare_list_len", compare_list_lengths);
    register_validator("compare_len1", compare_length_one);
    register_validator("default_bounds_real", default_bounds_real);
    register_validator("default_bounds_int", default_bounds_int);
    register_validator("default_initial_point_real", default_initial_point_real);
    register_validator("default_initial_point_int", default_initial_point_int);
    
    // List element validators
    register_validator("check_nonnegative_list", check_nonnegative_list);
    register_validator("check_positive_list", check_positive_list);
    register_validator("check_probability_list", check_probability_list);
    register_validator("check_real_lower_bound", check_real_lower_bound);
    register_validator("check_real_upper_bound", check_real_upper_bound);
    register_validator("check_int_lower_bound", check_int_lower_bound);
    
    // Cross-field validators
    register_validator("check_sum_equals_length", check_sum_equals_length);
    register_validator("trust_region_validate", trust_region_validate);
    register_validator("check_interface_block", check_interface_block);
    register_validator("check_analysis_drivers", check_analysis_drivers);
    
    // Response block validators
    register_validator("check_response_descriptors", check_response_descriptors);
    register_validator("check_fd_gradient_step_size", check_fd_gradient_step_size);
    register_validator("check_mixed_gradients", check_mixed_gradients);
    register_validator("check_mixed_hessians", check_mixed_hessians);
    register_validator("default_inequality_bounds", default_inequality_bounds);
    register_validator("default_equality_targets", default_equality_targets);
    register_validator("check_response_descriptors_length", check_response_descriptors_length);
    register_validator("check_descriptors_valid", check_descriptors_valid);
    register_validator("check_permitted_values", check_permitted_values);
    register_validator("check_scales_required", check_scales_required);
    register_validator("check_constraint_bounds_ordering", check_constraint_bounds_ordering);
    
    // Variable validators
    register_validator("check_variable_descriptors_length", check_variable_descriptors_length);
    register_validator("default_variable_descriptors", default_variable_descriptors);
    register_validator("check_variable_bounds_ordering", check_variable_bounds_ordering);
    register_validator("check_linear_inequality_bounds_ordering", check_linear_inequality_bounds_ordering);
    register_validator("check_all_variable_descriptors_unique", check_all_variable_descriptors_unique);
    register_validator("check_adjacency_matrix", check_adjacency_matrix);
    register_validator("check_set_elements_ordering", check_set_elements_ordering);
    register_validator("default_set_probabilities", default_set_probabilities);
    register_validator("uncertain_correlation_matrix_size", uncertain_correlation_matrix_size);
    
    // Continuous aleatory uncertain
    register_validator("normal_uncertain_initial", normal_uncertain_initial);
    register_validator("lognormal_uncertain_initial", lognormal_uncertain_initial);
    register_validator("uniform_uncertain_initial", uniform_uncertain_initial);
    register_validator("loguniform_uncertain_initial", loguniform_uncertain_initial);
    register_validator("triangular_uncertain_initial", triangular_uncertain_initial);
    register_validator("exponential_uncertain_initial", exponential_uncertain_initial);
    register_validator("beta_uncertain_initial", beta_uncertain_initial);
    register_validator("gamma_uncertain_initial", gamma_uncertain_initial);
    register_validator("gumbel_uncertain_initial", gumbel_uncertain_initial);
    register_validator("frechet_uncertain_initial", frechet_uncertain_initial);
    register_validator("weibull_uncertain_initial", weibull_uncertain_initial);
    register_validator("normal_uncertain_bounds", normal_uncertain_bounds);
    register_validator("lognormal_uncertain_bounds", lognormal_uncertain_bounds);
    
    // Discrete aleatory uncertain
    register_validator("poisson_uncertain_initial", poisson_uncertain_initial);
    register_validator("binomial_uncertain_initial", binomial_uncertain_initial);
    register_validator("negative_binomial_uncertain_initial", negative_binomial_uncertain_initial);
    register_validator("geometric_uncertain_initial", geometric_uncertain_initial);
    register_validator("hypergeometric_uncertain_initial", hypergeometric_uncertain_initial);
    
    // Epistemic uncertain - interval types
    register_validator("continuous_interval_uncertain_initial", continuous_interval_uncertain_initial);
    register_validator("discrete_interval_uncertain_initial", discrete_interval_uncertain_initial);
    
    // Histogram uncertain
    register_validator("histogram_bin_uncertain_initial", histogram_bin_uncertain_initial);
    register_validator("histogram_point_int_uncertain_initial", histogram_point_numeric_uncertain_initial);
    register_validator("histogram_point_real_uncertain_initial", histogram_point_numeric_uncertain_initial);
    register_validator("histogram_point_str_uncertain_initial", histogram_point_str_uncertain_initial);
    
    // Discrete set types (design, state, uncertain) - all use default_initial_from_set
    register_validator("discrete_design_set_int_initial", default_initial_from_set);
    register_validator("discrete_design_set_str_initial", default_initial_from_set);
    register_validator("discrete_design_set_real_initial", default_initial_from_set);
    register_validator("discrete_state_set_int_initial", default_initial_from_set);
    register_validator("discrete_state_set_str_initial", default_initial_from_set);
    register_validator("discrete_state_set_real_initial", default_initial_from_set);
    register_validator("discrete_uncertain_set_int_initial", default_initial_from_set);
    register_validator("discrete_uncertain_set_str_initial", default_initial_from_set);
    register_validator("discrete_uncertain_set_real_initial", default_initial_from_set);
}

ValidatorRegistry& ValidatorRegistry::instance() {
    static ValidatorRegistry registry;
    return registry;
}

void ValidatorRegistry::register_validator(const std::string& name, ValidatorFn fn) {
    validators_[name] = std::move(fn);
}

bool ValidatorRegistry::has_validator(const std::string& name) const {
    return validators_.find(name) != validators_.end();
}

ValidatorFn ValidatorRegistry::get_validator(const std::string& name) const {
    auto it = validators_.find(name);
    if (it == validators_.end()) {
        throw std::runtime_error("Unknown validator: " + name);
    }
    return it->second;
}

json ValidatorRegistry::validate(
    const json& instance,
    const std::string& rule_name,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) const {
    auto validator = get_validator(rule_name);
    return validator(instance, fields, literals, context);
}

json ValidatorRegistry::validate_all(json instance, const json& validations) const {
    for (const auto& rule_spec : validations) {
        std::string rule_name = rule_spec.at("validationRuleName").get<std::string>();
        auto fields = rule_spec.at("validationFields").get<std::vector<std::string>>();
        const auto& literals = rule_spec.at("validationLiterals");
        std::string context = rule_spec.at("validationContext").get<std::string>();
        
        json mutations = validate(instance, rule_name, fields, literals, context);
        instance.merge_patch(mutations);
    }
    return instance;
}

// ============================================================================
// Helper Functions
// ============================================================================

namespace {
    /// Split a dotted path into components
    std::vector<std::string> split_path(const std::string& path) {
        std::vector<std::string> components;
        std::string::size_type start = 0;
        std::string::size_type end;
        
        while ((end = path.find('.', start)) != std::string::npos) {
            if (end > start) {
                components.push_back(path.substr(start, end - start));
            }
            start = end + 1;
        }
        if (start < path.size()) {
            components.push_back(path.substr(start));
        }
        return components;
    }
    
    /// Resolve a dotted path in a JSON object.
    /// Returns nullptr if path doesn't exist or any component is null.
    const json* resolve_path(const json& instance, const std::string& path) {
        auto components = split_path(path);
        const json* current = &instance;
        
        for (const auto& component : components) {
            if (!current->is_object() || !current->contains(component)) {
                return nullptr;
            }
            current = &(current->at(component));
            if (current->is_null()) {
                return nullptr;
            }
        }
        return current;
    }
    
    /// Check if a path exists and is not null
    bool path_exists(const json& instance, const std::string& path) {
        return resolve_path(instance, path) != nullptr;
    }
    
    /// Check if field is missing or null
    bool field_missing_or_null(const json& instance, const std::string& field) {
        if (!instance.contains(field)) return true;
        return instance.at(field).is_null();
    }
    
    /// Check if a string looks like a floating-point number.
    /// Used to validate that descriptors aren't numeric (which would be ambiguous in Dakota input).
    /// This matches Dakota's isfloat() behavior.
    bool looks_like_number(const std::string& s) {
        if (s.empty()) return false;
        
        // Regex to detect if a string looks like a number
        // Matches: optional sign, digits with optional decimal, optional exponent (e/E/d/D), or nan/inf
        // This matches Dakota's isfloat() regex pattern
        static const std::regex float_regex(
            R"([+-]?[0-9]*\.?[0-9]+\.?[0-9]*[eEdD]?[+-]?[0-9]*|[Nn][Aa][Nn]|[+-]?[Ii][Nn][Ff](?:[Ii][Nn][Ii][Tt][Yy])?)",
            std::regex::ECMAScript
        );
        
        return std::regex_match(s, float_regex);
    }
}

// ============================================================================
// Existing Validator Implementations
// ============================================================================

json compare_length(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields[0] = list_field (may be dotted path), fields[1] = target_field (may be dotted path)
    const std::string& list_field = fields[0];
    const std::string& target_field = fields[1];
    
    // Resolve list field path
    const json* list_ptr = resolve_path(instance, list_field);
    
    // Skip validation if list field is null/missing
    if (list_ptr == nullptr) {
        return json::object();
    }
    
    const json& list_value = *list_ptr;
    
    if (!list_value.is_array()) {
        throw ValidationError("field '" + list_field + "' is not an array");
    }
    
    // Resolve target field path
    const json* target_ptr = resolve_path(instance, target_field);
    if (target_ptr == nullptr) {
        throw ValidationError("target field '" + target_field + "' not found");
    }
    
    int required_length = target_ptr->get<int>();
    auto actual_length = static_cast<int>(list_value.size());
    
    if (actual_length != required_length) {
        throw ValidationError("(" + std::to_string(actual_length) + " != " + 
                              std::to_string(required_length) + ")");
    }
    
    return json::object();  // No mutations
}

json compare_list_lengths(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields[0] = list_a (may be dotted path), fields[1] = list_b (may be dotted path)
    const std::string& field_a = fields[0];
    const std::string& field_b = fields[1];
    
    const json* ptr_a = resolve_path(instance, field_a);
    if (ptr_a == nullptr) {
        return json::object();
    }
    
    const json* ptr_b = resolve_path(instance, field_b);
    if (ptr_b == nullptr) {
        return json::object();
    }
    
    if (!ptr_a->is_array()) {
        throw ValidationError("field '" + field_a + "' is not an array");
    }
    if (!ptr_b->is_array()) {
        throw ValidationError("field '" + field_b + "' is not an array");
    }
    
    auto len_a = static_cast<int>(ptr_a->size());
    auto len_b = static_cast<int>(ptr_b->size());
    
    if (len_a != len_b) {
        throw ValidationError(
            "length of " + field_a + " (" + std::to_string(len_a) +
            ") does not equal length of " + field_b +
            " (" + std::to_string(len_b) + ")"
        );
    }
    
    return json::object();
}

json compare_length_one(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields[0] = list_field (may be dotted path), fields[1] = target_field (may be dotted path)
    const std::string& list_field = fields[0];
    const std::string& target_field = fields[1];
    
    // Resolve list field path
    const json* list_ptr = resolve_path(instance, list_field);
    
    // Skip validation if list field is null/missing
    if (list_ptr == nullptr) {
        return json::object();
    }
    
    const json& list_value = *list_ptr;
    
    if (!list_value.is_array()) {
        throw ValidationError("field '" + list_field + "' is not an array");
    }
    
    // Resolve target field path
    const json* target_ptr = resolve_path(instance, target_field);
    if (target_ptr == nullptr) {
        throw ValidationError("target field '" + target_field + "' not found");
    }
    
    int required_length = target_ptr->get<int>();
    auto actual_length = static_cast<int>(list_value.size());
    
    if (actual_length == 1) {
        // Expand single value to required length
        json expanded = json::array();
        const auto& single_value = list_value[0];
        for (int i = 0; i < required_length; ++i) {
            expanded.push_back(single_value);
        }
        
        // Note: mutations use the full dotted path as the key.
        // The caller is responsible for applying mutations correctly.
        json mutations;
        mutations[list_field] = expanded;
        return mutations;
    }
    
    if (actual_length != required_length) {
        throw ValidationError("(" + std::to_string(actual_length) + " != " + 
                              std::to_string(required_length) + ")");
    }
    
    return json::object();  // No mutations needed
}

json default_bounds_real(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // Field names are hardcoded
    // literals[0] = lower_default, literals[1] = upper_default
    const std::string lower_field = "lower_bounds";
    const std::string upper_field = "upper_bounds";
    const std::string count_field = "count";
    
    double lower_default = literals[0].get<double>();
    double upper_default = literals[1].get<double>();
    
    if (!instance.contains(count_field)) {
        throw ValidationError("count field '" + count_field + "' not found");
    }
    
    int required_length = instance.at(count_field).get<int>();
    json mutations;
    
    bool lower_missing = !instance.contains(lower_field) || 
                         instance.at(lower_field).is_null();
    if (lower_missing) {
        mutations[lower_field] = json::array_t(required_length, lower_default);
    }
    
    bool upper_missing = !instance.contains(upper_field) || 
                         instance.at(upper_field).is_null();
    if (upper_missing) {
        mutations[upper_field] = json::array_t(required_length, upper_default);
    }
    
    return mutations;
}

json default_bounds_int(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // Field names are hardcoded
    // literals[0] = lower_default, literals[1] = upper_default
    const std::string lower_field = "lower_bounds";
    const std::string upper_field = "upper_bounds";
    const std::string count_field = "count";
    
    int lower_default = literals[0].get<int>();
    int upper_default = literals[1].get<int>();
    
    if (!instance.contains(count_field)) {
        throw ValidationError("count field '" + count_field + "' not found");
    }
    
    int required_length = instance.at(count_field).get<int>();
    json mutations;
    
    bool lower_missing = !instance.contains(lower_field) || 
                         instance.at(lower_field).is_null();
    if (lower_missing) {
        mutations[lower_field] = json::array_t(required_length, lower_default);
    }
    
    bool upper_missing = !instance.contains(upper_field) || 
                         instance.at(upper_field).is_null();
    if (upper_missing) {
        mutations[upper_field] = json::array_t(required_length, upper_default);
    }
    
    return mutations;
}

json default_initial_point_real(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields[0] = target field (initial_point or initial_state)
    // lower_bounds, upper_bounds, count are hardcoded
    const std::string& ip_field = fields[0];
    const std::string lower_field = "lower_bounds";
    const std::string upper_field = "upper_bounds";
    const std::string count_field = "count";
    
    if (!instance.contains(count_field)) {
        throw ValidationError("field '" + count_field + "' not found");
    }
    if (!instance.contains(lower_field) || instance.at(lower_field).is_null()) {
        throw ValidationError("field '" + lower_field + "' not found or null");
    }
    if (!instance.contains(upper_field) || instance.at(upper_field).is_null()) {
        throw ValidationError("field '" + upper_field + "' not found or null");
    }
    
    int required_length = instance.at(count_field).get<int>();
    const auto& lower_bounds = instance.at(lower_field);
    const auto& upper_bounds = instance.at(upper_field);
    
    bool ip_missing = !instance.contains(ip_field) || 
                       instance.at(ip_field).is_null();
    
    if (ip_missing) {
        json initial_point = json::array();
        
        for (int i = 0; i < required_length; ++i) {
            double lb = lower_bounds[i].get<double>();
            double ub = upper_bounds[i].get<double>();
            
            bool lb_finite = std::isfinite(lb);
            bool ub_finite = std::isfinite(ub);
            
            double val;
            if (lb_finite && ub_finite) {
                val = (lb + ub) / 2.0;
            } else if (lb_finite) {
                val = lb;
            } else if (ub_finite) {
                val = ub;
            } else {
                val = 0.0;
            }
            initial_point.push_back(val);
        }
        
        json mutations;
        mutations[ip_field] = initial_point;
        return mutations;
    } else {
        json initial_point = instance.at(ip_field);
        bool changed = false;
        
        for (int i = 0; i < required_length; ++i) {
            double val = initial_point[i].get<double>();
            double lb = lower_bounds[i].get<double>();
            double ub = upper_bounds[i].get<double>();
            
            if (val < lb) {
                initial_point[i] = lb;
                changed = true;
            } else if (val > ub) {
                initial_point[i] = ub;
                changed = true;
            }
        }
        
        if (changed) {
            json mutations;
            mutations[ip_field] = initial_point;
            return mutations;
        }
    }
    
    return json::object();
}

json default_initial_point_int(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields[0] = target field (initial_point or initial_state)
    // lower_bounds, upper_bounds, count are hardcoded
    const std::string& ip_field = fields[0];
    const std::string lower_field = "lower_bounds";
    const std::string upper_field = "upper_bounds";
    const std::string count_field = "count";
    
    if (!instance.contains(count_field)) {
        throw ValidationError("field '" + count_field + "' not found");
    }
    if (!instance.contains(lower_field) || instance.at(lower_field).is_null()) {
        throw ValidationError("field '" + lower_field + "' not found or null");
    }
    if (!instance.contains(upper_field) || instance.at(upper_field).is_null()) {
        throw ValidationError("field '" + upper_field + "' not found or null");
    }
    
    int required_length = instance.at(count_field).get<int>();
    const auto& lower_bounds = instance.at(lower_field);
    const auto& upper_bounds = instance.at(upper_field);
    
    bool ip_missing = !instance.contains(ip_field) || 
                       instance.at(ip_field).is_null();
    
    if (ip_missing) {
        json initial_point = json::array();
        const int int_min = std::numeric_limits<int>::min();
        const int int_max = std::numeric_limits<int>::max();
        
        for (int i = 0; i < required_length; ++i) {
            int lb = lower_bounds[i].get<int>();
            int ub = upper_bounds[i].get<int>();
            int val;
            if (int_min < lb && ub < int_max) {
                // Integer midpoint, truncating toward zero to match C++.
                val = lb + (ub - lb) / 2;
            } else if (lb > 0) {
                val = lb;
            } else if (ub < 0) {
                val = ub;
            } else {
                val = 0;
            }
            initial_point.push_back(val);
        }
        
        json mutations;
        mutations[ip_field] = initial_point;
        return mutations;
    } else {
        json initial_point = instance.at(ip_field);
        bool changed = false;
        
        for (int i = 0; i < required_length; ++i) {
            int val = initial_point[i].get<int>();
            int lb = lower_bounds[i].get<int>();
            int ub = upper_bounds[i].get<int>();
            
            if (val < lb) {
                initial_point[i] = lb;
                changed = true;
            } else if (val > ub) {
                initial_point[i] = ub;
                changed = true;
            }
        }
        
        if (changed) {
            json mutations;
            mutations[ip_field] = initial_point;
            return mutations;
        }
    }
    
    return json::object();
}

// ============================================================================
// List Element Validators
// ============================================================================

json check_nonnegative_list(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    const std::string& list_field = fields[0];
    
    const json* list_ptr = resolve_path(instance, list_field);
    if (list_ptr == nullptr) {
        return json::object();
    }
    
    const json& list_value = *list_ptr;
    if (!list_value.is_array()) {
        throw ValidationError("field '" + list_field + "' is not an array");
    }
    
    for (size_t i = 0; i < list_value.size(); ++i) {
        const auto& elem = list_value[i];
        bool is_negative = false;
        
        if (elem.is_number_integer()) {
            is_negative = elem.get<int64_t>() < 0;
        } else if (elem.is_number_float()) {
            is_negative = elem.get<double>() < 0.0;
        }
        
        if (is_negative) {
            throw ValidationError("element " + std::to_string(i) + " is negative");
        }
    }
    
    return json::object();
}

json check_positive_list(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    const std::string& list_field = fields[0];
    
    const json* list_ptr = resolve_path(instance, list_field);
    if (list_ptr == nullptr) {
        return json::object();
    }
    
    const json& list_value = *list_ptr;
    if (!list_value.is_array()) {
        throw ValidationError("field '" + list_field + "' is not an array");
    }
    
    for (size_t i = 0; i < list_value.size(); ++i) {
        const auto& elem = list_value[i];
        bool is_not_positive = false;
        
        if (elem.is_number_integer()) {
            is_not_positive = elem.get<int64_t>() <= 0;
        } else if (elem.is_number_float()) {
            is_not_positive = elem.get<double>() <= 0.0;
        }
        
        if (is_not_positive) {
            throw ValidationError("element " + std::to_string(i) + " is not positive");
        }
    }
    
    return json::object();
}

json check_probability_list(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    const std::string& list_field = fields[0];
    
    const json* list_ptr = resolve_path(instance, list_field);
    if (list_ptr == nullptr) {
        return json::object();
    }
    
    const json& list_value = *list_ptr;
    if (!list_value.is_array()) {
        throw ValidationError("field '" + list_field + "' is not an array");
    }
    
    for (size_t i = 0; i < list_value.size(); ++i) {
        double val = list_value[i].get<double>();
        
        if (val < 0.0 || val > 1.0) {
            throw ValidationError("element " + std::to_string(i) + " (" + 
                                  std::to_string(val) + ") is not in [0, 1]");
        }
    }
    
    return json::object();
}

json check_real_lower_bound(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    const std::string& list_field = fields[0];
    double lower_bound = literals[0].get<double>();
    bool inclusive = (literals.size() > 1) ? literals[1].get<bool>() : false;
    
    const json* list_ptr = resolve_path(instance, list_field);
    if (list_ptr == nullptr) {
        return json::object();
    }
    
    const json& list_value = *list_ptr;
    if (!list_value.is_array()) {
        throw ValidationError("field '" + list_field + "' is not an array");
    }
    
    for (size_t i = 0; i < list_value.size(); ++i) {
        double val = list_value[i].get<double>();
        
        bool violation = inclusive ? (val < lower_bound) : (val <= lower_bound);
        if (violation) {
            std::string op = inclusive ? ">=" : ">";
            throw ValidationError("element " + std::to_string(i) + " (" + 
                                  std::to_string(val) + ") is not " + op + " " + 
                                  std::to_string(lower_bound));
        }
    }
    
    return json::object();
}

json check_real_upper_bound(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    const std::string& list_field = fields[0];
    double upper_bound = literals[0].get<double>();
    bool inclusive = (literals.size() > 1) ? literals[1].get<bool>() : false;
    
    const json* list_ptr = resolve_path(instance, list_field);
    if (list_ptr == nullptr) {
        return json::object();
    }
    
    const json& list_value = *list_ptr;
    if (!list_value.is_array()) {
        throw ValidationError("field '" + list_field + "' is not an array");
    }
    
    for (size_t i = 0; i < list_value.size(); ++i) {
        double val = list_value[i].get<double>();
        
        bool violation = inclusive ? (val > upper_bound) : (val >= upper_bound);
        if (violation) {
            std::string op = inclusive ? "<=" : "<";
            throw ValidationError("element " + std::to_string(i) + " (" + 
                                  std::to_string(val) + ") is not " + op + " " + 
                                  std::to_string(upper_bound));
        }
    }
    
    return json::object();
}

json check_int_lower_bound(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    const std::string& list_field = fields[0];
    int64_t lower_bound = literals[0].get<int64_t>();
    
    const json* list_ptr = resolve_path(instance, list_field);
    if (list_ptr == nullptr) {
        return json::object();
    }
    
    const json& list_value = *list_ptr;
    if (!list_value.is_array()) {
        throw ValidationError("field '" + list_field + "' is not an array");
    }
    
    for (size_t i = 0; i < list_value.size(); ++i) {
        int64_t val = list_value[i].get<int64_t>();
        
        if (val <= lower_bound) {
            throw ValidationError("element " + std::to_string(i) + " (" + 
                                  std::to_string(val) + ") is not > " + 
                                  std::to_string(lower_bound));
        }
    }
    
    return json::object();
}

// ============================================================================
// Cross-Field Validators
// ============================================================================

json check_sum_equals_length(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields[0] = num_list_field (list of integers to sum)
    // fields[1] = levels_list_field (list whose length should equal the sum)
    const std::string& num_list_field = fields[0];
    const std::string& levels_list_field = fields[1];
    
    const json* num_ptr = resolve_path(instance, num_list_field);
    const json* levels_ptr = resolve_path(instance, levels_list_field);
    
    // Skip if either field is missing
    if (num_ptr == nullptr || levels_ptr == nullptr) {
        return json::object();
    }
    
    const json& num_list = *num_ptr;
    const json& levels_list = *levels_ptr;
    
    if (!num_list.is_array()) {
        throw ValidationError("field '" + num_list_field + "' is not an array");
    }
    if (!levels_list.is_array()) {
        throw ValidationError("field '" + levels_list_field + "' is not an array");
    }
    
    // Sum the num_list
    int64_t sum = 0;
    for (const auto& elem : num_list) {
        sum += elem.get<int64_t>();
    }
    
    auto levels_length = static_cast<int64_t>(levels_list.size());
    
    if (sum != levels_length) {
        throw ValidationError("sum of " + num_list_field + " (" + std::to_string(sum) + 
                              ") does not equal length of " + levels_list_field + 
                              " (" + std::to_string(levels_length) + ")");
    }
    
    return json::object();
}

json trust_region_validate(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields[0] = initial_size_field (list)
    // fields[1] = minimum_size_field (scalar)
    // fields[2] = contract_threshold_field (scalar)
    // fields[3] = expand_threshold_field (scalar)
    // fields[4] = contraction_factor_field (scalar)
    // fields[5] = expansion_factor_field (scalar)
    
    const std::string& initial_size_field = fields[0];
    const std::string& minimum_size_field = fields[1];
    const std::string& contract_threshold_field = fields[2];
    const std::string& expand_threshold_field = fields[3];
    const std::string& contraction_factor_field = fields[4];
    const std::string& expansion_factor_field = fields[5];
    
    // Get minimum_size (scalar, default to 0 if missing)
    double minimum_size = 0.0;
    if (instance.contains(minimum_size_field) && !instance.at(minimum_size_field).is_null()) {
        minimum_size = instance.at(minimum_size_field).get<double>();
        
        // Check minimum_size in [0, 1]
        if (minimum_size < 0.0 || minimum_size > 1.0) {
            throw ValidationError(minimum_size_field + " (" + std::to_string(minimum_size) + 
                                  ") must be in [0, 1]");
        }
    }
    
    // Validate initial_size list
    if (instance.contains(initial_size_field) && !instance.at(initial_size_field).is_null()) {
        const auto& initial_sizes = instance.at(initial_size_field);
        
        if (!initial_sizes.is_array()) {
            throw ValidationError(initial_size_field + " is not an array");
        }
        
        for (size_t i = 0; i < initial_sizes.size(); ++i) {
            double val = initial_sizes[i].get<double>();
            
            // Check in (0, 1]
            if (val <= 0.0 || val > 1.0) {
                throw ValidationError(initial_size_field + "[" + std::to_string(i) + "] (" + 
                                      std::to_string(val) + ") must be in (0, 1]");
            }
            
            // Check >= minimum_size
            if (val < minimum_size) {
                throw ValidationError(initial_size_field + "[" + std::to_string(i) + "] (" + 
                                      std::to_string(val) + ") must be >= " + 
                                      minimum_size_field + " (" + std::to_string(minimum_size) + ")");
            }
        }
    }
    
    // Get contract_threshold and expand_threshold
    double contract_threshold = 0.0;
    double expand_threshold = 1.0;
    
    if (instance.contains(contract_threshold_field) && !instance.at(contract_threshold_field).is_null()) {
        contract_threshold = instance.at(contract_threshold_field).get<double>();
    }
    if (instance.contains(expand_threshold_field) && !instance.at(expand_threshold_field).is_null()) {
        expand_threshold = instance.at(expand_threshold_field).get<double>();
    }
    
    // Check 0 < contract_threshold <= expand_threshold <= 1
    if (contract_threshold <= 0.0) {
        throw ValidationError(contract_threshold_field + " (" + 
                              std::to_string(contract_threshold) + ") must be > 0");
    }
    if (contract_threshold > expand_threshold) {
        throw ValidationError(contract_threshold_field + " (" + std::to_string(contract_threshold) + 
                              ") must be <= " + expand_threshold_field + " (" + 
                              std::to_string(expand_threshold) + ")");
    }
    if (expand_threshold > 1.0) {
        throw ValidationError(expand_threshold_field + " (" + 
                              std::to_string(expand_threshold) + ") must be <= 1");
    }
    
    // Validate contraction_factor in (0, 1]
    if (instance.contains(contraction_factor_field) && !instance.at(contraction_factor_field).is_null()) {
        double contraction_factor = instance.at(contraction_factor_field).get<double>();
        
        if (contraction_factor <= 0.0 || contraction_factor > 1.0) {
            throw ValidationError(contraction_factor_field + " (" + 
                                  std::to_string(contraction_factor) + ") must be in (0, 1]");
        }
    }
    
    // Validate expansion_factor >= 1
    if (instance.contains(expansion_factor_field) && !instance.at(expansion_factor_field).is_null()) {
        double expansion_factor = instance.at(expansion_factor_field).get<double>();
        
        if (expansion_factor < 1.0) {
            throw ValidationError(expansion_factor_field + " (" + 
                                  std::to_string(expansion_factor) + ") must be >= 1");
        }
    }
    
    return json::object();
}

json check_interface_block(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    json result = json::object();

    bool analysis_drivers = path_exists(instance, "analysis_drivers");
    int num_analsyis_drivers = (analysis_drivers) ? instance["analysis_drivers"]["drivers"].size() : 0;


    bool input_filter = path_exists(instance, "analysis_drivers.input_filter");
    bool output_filter = path_exists(instance, "analysis_drivers.output_filter");

    if(path_exists(instance, "concurrency.batch")) {
        if(num_analsyis_drivers > 1) {
            throw ValidationError("Batch concurrency is not supported with multiple analysis drivers.");
        }
        if(input_filter) {
            throw ValidationError("Batch concurrency is not supported with input filters.");
        }
        if(output_filter) {
            throw ValidationError("Batch concurrency is not supported with output filters.");
        }

        // if(path_exists(instance, "concurrency.batch.size")) {
        //     int batch_size = instance["concurrency"]["batch"]["size"].get<int>();
        //     if(batch_size == 1) {
        //         throw ValidationError("Batch size of 1 is not allowed; increase size or disable 'batch' concurrency.");
        //     }
        // }

        // failure_capture is a Union - only one mode active at a time.
        // In batch mode, only 'abort' and 'recover' are permitted.
        json fc_copy(instance["failure_capture"]);
        fc_copy.erase("abort");
        fc_copy.erase("recover");
        if (!fc_copy.empty()) {
            auto disallowed = fc_copy.items().begin();
            throw ValidationError("failure_capture mode '" + disallowed.key() + "' disallowed in 'batch' mode");
        }
    }
    bool algebraic_mappings = path_exists(instance, "algebraic_mappings");

    if(!(analysis_drivers || algebraic_mappings)) {
        throw ValidationError("Either 'analysis_drivers' or 'algebraic_mappings' block must be present.");
    }
    
    // const json* const asynch = resolve_path(instance, "concurrency.asynchronous");
    // if(asynch != nullptr) {
	//     bool has_ec = path_exists(*asynch, "evaluation_concurrency");
    //     bool has_ac = path_exists(*asynch, "analysis_concurrency");
    //     int ec = (has_ec) ? (*asynch)["evaluation_concurrency"].get<int>() : 0;
    //     int ac = (has_ac) ? (*asynch)["analysis_concurrency"].get<int>() : 0;
    //     if(ec == 1 && ac == 1) {
    //         throw ValidationError("Asynchronous concurrency with both 'evaluation_concurrency' and 'analysis_concurrency' set to 1 is not allowed.");
    //     }
    // }

    #ifdef _WIN32
    if(path_exists(instance, "analysis_drivers.interface_type.fork.work_directory.link_files") ||
         path_exists(instance, "analysis_drivers.interface_type.system.work_directory.link_files")
    ) {
        throw ValidationError("'link_files' is not supported on Windows platforms.");
    }
    #endif

    return result;
}

json check_analysis_drivers(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    json result = json::object();

    const json* drivers = resolve_path(instance, "drivers");
    if (!drivers || !drivers->is_array() || drivers->empty()) {
        throw ValidationError("analysis_drivers.drivers must contain at least one driver");
    }

    const json* analysis_components = resolve_path(instance, "analysis_components");
    if (!analysis_components) {
        return result;
    }
    if (!analysis_components->is_array()) {
        throw ValidationError("analysis_components must be an array when provided");
    }
    if (analysis_components->size() % drivers->size() != 0) {
        throw ValidationError(
            "number of analysis_components must be evenly divisible by number of analysis_drivers"
        );
    }

    return result;
}

// ============================================================================
// Response Block Validators
// Based on make_response_defaults() from NIDRProblemDescDB.cpp
// ============================================================================

namespace {
    /// Build numbered labels with a prefix.
    /// NIDRProblemDescDB had a special case for obj_fcns, but here we always
    // include the numeric suffix for consistency.
    std::vector<std::string> build_labels(const std::string& prefix, size_t start, size_t count) {
        std::vector<std::string> labels;
        labels.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            labels.push_back(prefix + std::to_string(start + i + 1));
        }
        return labels;
    }
    
    /// Get array length from a JSON field, handling null/missing.
    size_t get_array_length(const json& instance, const std::string& field) {
        if (!instance.contains(field) || instance.at(field).is_null()) {
            return 0;
        }
        const auto& arr = instance.at(field);
        if (!arr.is_array()) {
            return 0;
        }
        return arr.size();
    }
    
    /// Check if a field exists and is not null.
    bool field_exists(const json& instance, const std::string& field) {
        return instance.contains(field) && !instance.at(field).is_null();
    }
    
    /// Get value with default for missing/null fields.
    template<typename T>
    T get_or_default(const json& instance, const std::string& field, T default_val) {
        if (!instance.contains(field) || instance.at(field).is_null()) {
            return default_val;
        }
        return instance.at(field).get<T>();
    }
    
    /// Compute total number of functions from nested ResponsesConfig structure.
    size_t compute_num_functions_nested(const json& instance) {
        const json* response_type = resolve_path(instance, "response_type");
        if (response_type == nullptr) {
            return 0;
        }
        
        // CalibrationTerms case
        const json* calibration_terms = resolve_path(*response_type, "calibration_terms");
        if (calibration_terms != nullptr) {
            size_t num_cal = get_or_default<size_t>(*calibration_terms, "count", 0);
            const json* ineq = resolve_path(*calibration_terms, "nonlinear_inequality_constraints");
            const json* eq = resolve_path(*calibration_terms, "nonlinear_equality_constraints");
            size_t num_ineq = ineq ? get_or_default<size_t>(*ineq, "count", 0) : 0;
            size_t num_eq = eq ? get_or_default<size_t>(*eq, "count", 0) : 0;
            return num_cal + num_ineq + num_eq;
        }
        
        // ObjectiveFunctions case
        const json* objective_functions = resolve_path(*response_type, "objective_functions");
        if (objective_functions != nullptr) {
            size_t num_obj = get_or_default<size_t>(*objective_functions, "count", 0);
            const json* ineq = resolve_path(*objective_functions, "nonlinear_inequality_constraints");
            const json* eq = resolve_path(*objective_functions, "nonlinear_equality_constraints");
            size_t num_ineq = ineq ? get_or_default<size_t>(*ineq, "count", 0) : 0;
            size_t num_eq = eq ? get_or_default<size_t>(*eq, "count", 0) : 0;
            return num_obj + num_ineq + num_eq;
        }
        
        // ResponseFunctions case
        const json* response_functions = resolve_path(*response_type, "response_functions");
        if (response_functions != nullptr) {
            return get_or_default<size_t>(*response_functions, "count", 0);
        }
        
        return 0;
    }
    
    /// Check mixed gradient/hessian IDs and update coverage array.
    /// Returns number of errors found.
    int check_mixed_ids(const json& id_array, int num_functions,
                       std::vector<int>& coverage, const std::string& what,
                       std::string& error_msg) {
        int errors = 0;
        for (const auto& id : id_array) {
            int idx = id.get<int>();
            if (idx < 1 || idx > num_functions) {
                if (!error_msg.empty()) error_msg += "; ";
                error_msg += what + " values must be between 1 and " + std::to_string(num_functions);
                errors++;
            } else {
                coverage[idx - 1]++;
            }
        }
        return errors;
    }
    
    /// Check that all functions are covered exactly once.
    void check_coverage(size_t num_functions, const std::vector<int>& coverage, 
                       const std::string& what) {
        for (size_t i = 0; i < num_functions; ++i) {
            if (coverage[i] == 0) {
                throw ValidationError("Function " + std::to_string(i + 1) + 
                                     " missing from mixed " + what + " lists");
            }
            if (coverage[i] > 1) {
                throw ValidationError("Function " + std::to_string(i + 1) + 
                                     " replicated in mixed " + what + " lists");
            }
        }
    }
}  // anonymous namespace

json check_response_descriptors(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // Check if descriptors already provided
    size_t descriptors_len = get_array_length(instance, "descriptors");
    if (descriptors_len > 0) {
        return json::object();  // Already set, no mutations
    }
    
    json mutations;
    std::vector<std::string> labels;
    
    // Navigate the nested response_type structure
    const json* response_type = resolve_path(instance, "response_type");
    if (response_type == nullptr) {
        return json::object();
    }
    
    // CalibrationTerms case: response_type.calibration_terms
    const json* calibration_terms = resolve_path(*response_type, "calibration_terms");
    if (calibration_terms != nullptr) {
        size_t num_cal = get_or_default<size_t>(*calibration_terms, "count", 0);
        
        // Get constraint counts from nested models
        const json* ineq = resolve_path(*calibration_terms, "nonlinear_inequality_constraints");
        const json* eq = resolve_path(*calibration_terms, "nonlinear_equality_constraints");
        size_t num_ineq = ineq ? get_or_default<size_t>(*ineq, "count", 0) : 0;
        size_t num_eq = eq ? get_or_default<size_t>(*eq, "count", 0) : 0;
        
        size_t nf = num_cal + num_ineq + num_eq;
        labels.reserve(nf);
        
        // Add least_sq_term labels
        auto lsq_labels = build_labels("least_sq_term_", 0, num_cal);
        labels.insert(labels.end(), lsq_labels.begin(), lsq_labels.end());
        
        // Add nonlinear inequality constraint labels
        auto ineq_labels = build_labels("nln_ineq_con_", 0, num_ineq);
        labels.insert(labels.end(), ineq_labels.begin(), ineq_labels.end());
        
        // Add nonlinear equality constraint labels
        auto eq_labels = build_labels("nln_eq_con_", 0, num_eq);
        labels.insert(labels.end(), eq_labels.begin(), eq_labels.end());
    }
    
    // ObjectiveFunctions case: response_type.objective_functions
    const json* objective_functions = resolve_path(*response_type, "objective_functions");
    if (objective_functions != nullptr) {
        size_t num_obj = get_or_default<size_t>(*objective_functions, "count", 0);
        
        // Get constraint counts from nested models
        const json* ineq = resolve_path(*objective_functions, "nonlinear_inequality_constraints");
        const json* eq = resolve_path(*objective_functions, "nonlinear_equality_constraints");
        size_t num_ineq = ineq ? get_or_default<size_t>(*ineq, "count", 0) : 0;
        size_t num_eq = eq ? get_or_default<size_t>(*eq, "count", 0) : 0;
        
        size_t nf = num_obj + num_ineq + num_eq;
        labels.reserve(nf);
        
        // Add objective function labels
        auto obj_labels = build_labels("obj_fn_", 0, num_obj);
        labels.insert(labels.end(), obj_labels.begin(), obj_labels.end());
        
        // Add nonlinear inequality constraint labels  
        auto ineq_labels = build_labels("nln_ineq_con_", 0, num_ineq);
        labels.insert(labels.end(), ineq_labels.begin(), ineq_labels.end());
        
        // Add nonlinear equality constraint labels
        auto eq_labels = build_labels("nln_eq_con_", 0, num_eq);
        labels.insert(labels.end(), eq_labels.begin(), eq_labels.end());
    }
    
    // ResponseFunctions case: response_type.response_functions
    const json* response_functions = resolve_path(*response_type, "response_functions");
    if (response_functions != nullptr) {
        size_t num_resp = get_or_default<size_t>(*response_functions, "count", 0);
        labels = build_labels("response_fn_", 0, num_resp);
    }
    
    if (!labels.empty()) {
        mutations["descriptors"] = labels;
    }
    
    return mutations;
}

json check_fd_gradient_step_size(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // Check that vendor numerical gradients only use single fd_step_size
    // Structure: gradient_type.numerical_gradients.method_source / fd_step_size
    //        or: gradient_type.mixed_gradients.method_source / fd_step_size
    
    const json* gradient_type = resolve_path(instance, "gradient_type");
    if (gradient_type == nullptr) {
        return json::object();
    }
    
    // Check NumericalGradients case
    const json* numerical_gradients = resolve_path(*gradient_type, "numerical_gradients");
    if (numerical_gradients != nullptr) {
        const json* method_source = resolve_path(*numerical_gradients, "method_source");
        if (method_source != nullptr && method_source->contains("vendor")) {
            size_t step_size_len = get_array_length(*numerical_gradients, "fd_step_size");
            if (step_size_len > 1) {
                throw ValidationError(
                    "vendor numerical gradients only support a single fd_step_size"
                );
            }
        }
    }
    
    // Check MixedGradients case
    const json* mixed_gradients = resolve_path(*gradient_type, "mixed_gradients");
    if (mixed_gradients != nullptr) {
        const json* method_source = resolve_path(*mixed_gradients, "method_source");
        if (method_source != nullptr && method_source->contains("vendor")) {
            size_t step_size_len = get_array_length(*mixed_gradients, "fd_step_size");
            if (step_size_len > 1) {
                throw ValidationError(
                    "vendor numerical gradients only support a single fd_step_size"
                );
            }
        }
    }
    
    return json::object();
}

json check_mixed_gradients(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // Validate mixed gradient ID lists cover all functions exactly once
    // Structure: gradient_type.mixed_gradients.id_analytic_gradients / id_numerical_gradients
    
    const json* gradient_type = resolve_path(instance, "gradient_type");
    if (gradient_type == nullptr) {
        return json::object();
    }
    
    // Check if this is MixedGradients
    const json* mixed_gradients = resolve_path(*gradient_type, "mixed_gradients");
    if (mixed_gradients == nullptr) {
        return json::object();  // Not mixed gradients
    }
    
    // Calculate total number of functions using nested structure
    size_t nf = compute_num_functions_nested(instance);
    
    if (nf == 0) {
        return json::object();
    }
    
    std::vector<int> coverage(nf, 0);
    std::string error_msg;
    int errors = 0;
    
    // Check analytic gradients
    if (field_exists(*mixed_gradients, "id_analytic_gradients")) {
        errors += check_mixed_ids(
            mixed_gradients->at("id_analytic_gradients"),
            static_cast<int>(nf), coverage, 
            "id_analytic_gradients", error_msg
        );
    }
    
    // Check numerical gradients
    if (field_exists(*mixed_gradients, "id_numerical_gradients")) {
        errors += check_mixed_ids(
            mixed_gradients->at("id_numerical_gradients"),
            static_cast<int>(nf), coverage,
            "id_numerical_gradients", error_msg
        );
    }
    
    if (errors > 0) {
        throw ValidationError(error_msg);
    }
    
    // Validate coverage
    check_coverage(nf, coverage, "gradient");
    
    return json::object();
}

json check_mixed_hessians(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // Validate mixed Hessian ID lists cover all functions exactly once
    // Structure: hessian_type.mixed_hessians.id_analytic_hessians (list)
    //           hessian_type.mixed_hessians.id_numerical_hessians.values (list)
    //           hessian_type.mixed_hessians.id_quasi_hessians.values (list)
    
    const json* hessian_type = resolve_path(instance, "hessian_type");
    if (hessian_type == nullptr) {
        return json::object();
    }
    
    // Check if this is MixedHessians
    const json* mixed_hessians = resolve_path(*hessian_type, "mixed_hessians");
    if (mixed_hessians == nullptr) {
        return json::object();  // Not mixed Hessians
    }
    
    // Calculate total number of functions using nested structure
    size_t nf = compute_num_functions_nested(instance);
    
    if (nf == 0) {
        return json::object();
    }
    
    std::vector<int> coverage(nf, 0);
    std::string error_msg;
    int errors = 0;
    
    // Check analytic Hessians (direct list)
    if (field_exists(*mixed_hessians, "id_analytic_hessians")) {
        errors += check_mixed_ids(
            mixed_hessians->at("id_analytic_hessians"),
            static_cast<int>(nf), coverage,
            "id_analytic_hessians", error_msg
        );
    }
    
    // Check numerical Hessians (has `values` attribute)
    const json* numerical_hessians = resolve_path(*mixed_hessians, "id_numerical_hessians");
    if (numerical_hessians != nullptr && field_exists(*numerical_hessians, "values")) {
        errors += check_mixed_ids(
            numerical_hessians->at("values"),
            static_cast<int>(nf), coverage,
            "id_numerical_hessians", error_msg
        );
    }
    
    // Check quasi Hessians (has `values` attribute)
    const json* quasi_hessians = resolve_path(*mixed_hessians, "id_quasi_hessians");
    if (quasi_hessians != nullptr && field_exists(*quasi_hessians, "values")) {
        errors += check_mixed_ids(
            quasi_hessians->at("values"),
            static_cast<int>(nf), coverage,
            "id_quasi_hessians", error_msg
        );
    }
    
    if (errors > 0) {
        throw ValidationError(error_msg);
    }
    
    // Validate coverage
    check_coverage(nf, coverage, "Hessian");
    
    return json::object();
}

json default_inequality_bounds(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // Set default values for inequality constraint bounds
    // Applied to nested models like ObjectiveFunctionsNonlinearInequalityConstraints
    // Uses fields: count, lower_bounds, upper_bounds
    
    double dbl_inf = std::numeric_limits<double>::infinity();
    json mutations;
    
    size_t count = get_or_default<size_t>(instance, "count", 0);
    if (count == 0) {
        return json::object();
    }
    
    // Default lower bounds to -inf
    size_t lower_len = get_array_length(instance, "lower_bounds");
    if (lower_len == 0) {
        mutations["lower_bounds"] = json::array_t(count, -dbl_inf);
    }
    
    // Default upper bounds to 0.0 (one-sided g(x) <= 0)
    size_t upper_len = get_array_length(instance, "upper_bounds");
    if (upper_len == 0) {
        mutations["upper_bounds"] = json::array_t(count, 0.0);
    }
    
    return mutations;
}

json default_equality_targets(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // Set default values for equality constraint targets
    // Applied to nested models like ObjectiveFunctionsNonlinearEqualityConstraints
    // Uses fields: count, targets
    
    json mutations;
    
    size_t count = get_or_default<size_t>(instance, "count", 0);
    if (count == 0) {
        return json::object();
    }
    
    // Default targets to 0.0
    size_t targets_len = get_array_length(instance, "targets");
    if (targets_len == 0) {
        mutations["targets"] = json::array_t(count, 0.0);
    }
    
    return mutations;
}

json check_response_descriptors_length(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // Validate that descriptors length is 0 or equals total number of functions
    
    size_t descriptors_len = get_array_length(instance, "descriptors");
    if (descriptors_len == 0) {
        return json::object();  // Will be set to defaults by check_response_descriptors
    }
    
    // Calculate total number of functions
    size_t nf = compute_num_functions_nested(instance);
    
    if (descriptors_len != nf) {
        throw ValidationError(
            "Number of response descriptors (" + std::to_string(descriptors_len) + 
            ") must equal total number of responses (" + std::to_string(nf) + ")"
        );
    }
    
    return json::object();
}

json check_descriptors_valid(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // Validate that descriptors follow naming rules:
    // - Not zero-length
    // - No whitespace
    // - Cannot be a number (would be ambiguous in Dakota input)
    //
    // fields[0] = field name (defaults to "descriptors" if not provided)
    // literals[0] = check_uniqueness (boolean, defaults to false)
    
    std::string field_name = "descriptors";
    if (!fields.empty() && !fields[0].empty()) {
        field_name = fields[0];
    }
    
    bool check_uniqueness = false;
    if (!literals.empty() && literals[0].is_boolean()) {
        check_uniqueness = literals[0].get<bool>();
    }
    
    const json* descriptors_ptr = resolve_path(instance, field_name);
    if (descriptors_ptr == nullptr) {
        return json::object();
    }
    
    const json& descriptors = *descriptors_ptr;
    if (!descriptors.is_array() || descriptors.empty()) {
        return json::object();
    }
    
    // Regex to detect if a string looks like a number
    // Matches: optional sign, digits with optional decimal, optional exponent (e/E/d/D), or nan/inf
    // This matches Dakota's isfloat() regex pattern
    static const std::regex float_regex(
        R"([+-]?[0-9]*\.?[0-9]+\.?[0-9]*[eEdD]?[+-]?[0-9]*|[Nn][Aa][Nn]|[+-]?[Ii][Nn][Ff](?:[Ii][Nn][Ii][Tt][Yy])?)",
        std::regex::ECMAScript
    );
    
    std::unordered_set<std::string> seen;
    
    for (size_t i = 0; i < descriptors.size(); ++i) {
        const std::string& desc = descriptors[i].get<std::string>();
        
        // Check for uniqueness if requested
        if (check_uniqueness) {
            if (seen.count(desc) > 0) {
                throw ValidationError(
                    "Duplicate descriptor '" + desc + "' at index " + std::to_string(i)
                );
            }
            seen.insert(desc);
        }
        
        // Check for empty string
        if (desc.empty()) {
            throw ValidationError(
                "Descriptor at index " + std::to_string(i) + " cannot be empty"
            );
        }
        
        // Check for whitespace
        for (char c : desc) {
            if (std::isspace(static_cast<unsigned char>(c))) {
                throw ValidationError(
                    "Descriptor '" + desc + "' contains whitespace"
                );
            }
        }
        
        // Check if it looks like a number
        if (std::regex_match(desc, float_regex)) {
            throw ValidationError(
                "Descriptor '" + desc + "' cannot be a number"
            );
        }
    }
    
    return json::object();
}

json check_permitted_values(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // Generic validator: every element of a string list must be in a permitted set.
    // fields[0] = field name (e.g., "scale_types", "flags")
    // literals = permitted values (e.g., ["value", "auto", "log", "none"])
    // Comparison is case-insensitive: input values are lowercased before checking.
    
    if (fields.empty()) {
        return json::object();
    }
    
    const std::string& field_name = fields[0];
    const json* values_ptr = resolve_path(instance, field_name);
    if (values_ptr == nullptr) {
        return json::object();
    }
    
    const json& values = *values_ptr;
    if (!values.is_array()) {
        return json::object();
    }
    
    // Build set of permitted values from literals (assumed already lowercase)
    std::unordered_set<std::string> permitted;
    for (const auto& lit : literals) {
        if (lit.is_string()) {
            permitted.insert(lit.get<std::string>());
        }
    }
    
    // Build error message showing valid options
    std::string valid_options;
    for (size_t i = 0; i < literals.size(); ++i) {
        if (i > 0) valid_options += ", ";
        valid_options += "'" + literals[i].get<std::string>() + "'";
    }
    
    for (size_t i = 0; i < values.size(); ++i) {
        std::string val = values[i].get<std::string>();
        // Lowercase for comparison
        for (auto& c : val) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        if (permitted.count(val) == 0) {
            throw ValidationError(
                field_name + "[" + std::to_string(i) + "] = '" +
                values[i].get<std::string>() + 
                "' is invalid; must be one of: " + valid_options
            );
        }
    }
    
    return json::object();
}

json check_scales_required(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // Generic validator: if any scale_types are 'value', scales must be provided.
    // fields[0] = scale_types field name
    // fields[1] = scales field name
    
    if (fields.size() < 2) {
        return json::object();
    }
    
    const std::string& scale_types_field = fields[0];
    const std::string& scales_field = fields[1];
    
    const json* scale_types_ptr = resolve_path(instance, scale_types_field);
    if (scale_types_ptr == nullptr) {
        return json::object();
    }
    
    const json& scale_types = *scale_types_ptr;
    if (!scale_types.is_array()) {
        return json::object();
    }
    
    bool has_value_type = false;
    for (const auto& st : scale_types) {
        if (st.is_string() && st.get<std::string>() == "value") {
            has_value_type = true;
            break;
        }
    }
    
    if (!has_value_type) {
        return json::object();
    }
    
    // Check that scales is provided
    const json* scales_ptr = resolve_path(instance, scales_field);
    if (scales_ptr == nullptr || !scales_ptr->is_array() || scales_ptr->empty()) {
        throw ValidationError(
            "When " + scale_types_field + " includes 'value', " + scales_field + " must be provided"
        );
    }
    
    return json::object();
}

json check_constraint_bounds_ordering(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // Verify that lower_bounds are element-wise less than upper_bounds
    // Applied to nonlinear_inequality_constraints
    
    const json* lower_ptr = resolve_path(instance, "lower_bounds");
    const json* upper_ptr = resolve_path(instance, "upper_bounds");
    
    if (lower_ptr == nullptr || upper_ptr == nullptr) {
        return json::object();
    }
    
    const json& lower = *lower_ptr;
    const json& upper = *upper_ptr;
    
    if (!lower.is_array() || !upper.is_array()) {
        return json::object();
    }
    
    size_t n = std::min(lower.size(), upper.size());
    
    for (size_t i = 0; i < n; ++i) {
        double lb = lower[i].get<double>();
        double ub = upper[i].get<double>();
        
        // Allow equal bounds (equality constraint behavior)
        // but lower must not exceed upper
        if (lb > ub) {
            throw ValidationError(
                "lower_bounds[" + std::to_string(i) + "] = " + std::to_string(lb) +
                " exceeds upper_bounds[" + std::to_string(i) + "] = " + std::to_string(ub)
            );
        }
    }
    
    return json::object();
}

// ============================================================================
// Variable validators
// ============================================================================

json check_variable_descriptors_length(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // Validate that descriptors length is 0 or equals count
    
    size_t descriptors_len = get_array_length(instance, "descriptors");
    if (descriptors_len == 0) {
        return json::object();  // Will be set to defaults
    }
    
    size_t count = get_or_default<size_t>(instance, "count", 0);
    
    if (descriptors_len != count) {
        throw ValidationError(
            "Number of descriptors (" + std::to_string(descriptors_len) + 
            ") must equal count (" + std::to_string(count) + ")"
        );
    }
    
    return json::object();
}

json default_variable_descriptors(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // Generate default descriptors with prefix from literals[0]
    // Format: prefix + "1", prefix + "2", etc.
    
    size_t descriptors_len = get_array_length(instance, "descriptors");
    if (descriptors_len > 0) {
        return json::object();  // Already has descriptors
    }
    
    size_t count = get_or_default<size_t>(instance, "count", 0);
    if (count == 0) {
        return json::object();
    }
    
    // Get prefix from literals
    std::string prefix = "var_";
    if (!literals.empty() && literals[0].is_string()) {
        prefix = literals[0].get<std::string>();
    }
    
    json mutations;
    json labels = json::array();
    for (size_t i = 1; i <= count; ++i) {
        labels.push_back(prefix + std::to_string(i));
    }
    mutations["descriptors"] = labels;
    
    return mutations;
}

json check_variable_bounds_ordering(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // Verify that lower_bounds are element-wise less than upper_bounds
    // For variable types with user-provided bounds
    
    const json* lower_ptr = resolve_path(instance, "lower_bounds");
    const json* upper_ptr = resolve_path(instance, "upper_bounds");
    
    if (lower_ptr == nullptr || upper_ptr == nullptr) {
        return json::object();
    }
    
    const json& lower = *lower_ptr;
    const json& upper = *upper_ptr;
    
    if (!lower.is_array() || !upper.is_array()) {
        return json::object();
    }
    
    size_t n = std::min(lower.size(), upper.size());
    
    for (size_t i = 0; i < n; ++i) {
        double lb = lower[i].get<double>();
        double ub = upper[i].get<double>();
        
        if (lb > ub) {
            throw ValidationError(
                "lower_bounds[" + std::to_string(i) + "] = " + std::to_string(lb) +
                " exceeds upper_bounds[" + std::to_string(i) + "] = " + std::to_string(ub)
            );
        }
    }
    
    return json::object();
}

json check_linear_inequality_bounds_ordering(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // Verify linear_inequality_lower_bounds <= linear_inequality_upper_bounds element-wise
    
    const json* lower_ptr = resolve_path(instance, "linear_inequality_lower_bounds");
    const json* upper_ptr = resolve_path(instance, "linear_inequality_upper_bounds");
    
    if (lower_ptr == nullptr || upper_ptr == nullptr) {
        return json::object();
    }
    
    const json& lower = *lower_ptr;
    const json& upper = *upper_ptr;
    
    if (!lower.is_array() || !upper.is_array()) {
        return json::object();
    }
    
    size_t n = std::min(lower.size(), upper.size());
    
    for (size_t i = 0; i < n; ++i) {
        double lb = lower[i].get<double>();
        double ub = upper[i].get<double>();
        
        if (lb > ub) {
            throw ValidationError(
                "linear_inequality_lower_bounds[" + std::to_string(i) + "] = " + std::to_string(lb) +
                " exceeds linear_inequality_upper_bounds[" + std::to_string(i) + "] = " + std::to_string(ub)
            );
        }
    }
    
    return json::object();
}

json check_all_variable_descriptors_unique(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // Check that all variable descriptors across all variable types are unique
    // Runs on VariablesConfig
    
    // List of all variable type field paths that have descriptors
    static const std::vector<std::string> var_paths = {
        "continuous_design.descriptors",
        "discrete_design_range.descriptors",
        "discrete_design_set.integer.descriptors",
        "discrete_design_set.string.descriptors",
        "discrete_design_set.real.descriptors",
        "normal_uncertain.descriptors",
        "lognormal_uncertain.descriptors",
        "uniform_uncertain.descriptors",
        "loguniform_uncertain.descriptors",
        "triangular_uncertain.descriptors",
        "exponential_uncertain.descriptors",
        "beta_uncertain.descriptors",
        "gamma_uncertain.descriptors",
        "gumbel_uncertain.descriptors",
        "frechet_uncertain.descriptors",
        "weibull_uncertain.descriptors",
        "histogram_bin_uncertain.descriptors",
        "poisson_uncertain.descriptors",
        "binomial_uncertain.descriptors",
        "negative_binomial_uncertain.descriptors",
        "geometric_uncertain.descriptors",
        "hypergeometric_uncertain.descriptors",
        "histogram_point_uncertain.integer.descriptors",
        "histogram_point_uncertain.string.descriptors",
        "histogram_point_uncertain.real.descriptors",
        "continuous_interval_uncertain.descriptors",
        "discrete_interval_uncertain.descriptors",
        "discrete_uncertain_set.integer.descriptors",
        "discrete_uncertain_set.string.descriptors",
        "discrete_uncertain_set.real.descriptors",
        "continuous_state.descriptors",
        "discrete_state_range.descriptors",
        "discrete_state_set.integer.descriptors",
        "discrete_state_set.string.descriptors",
        "discrete_state_set.real.descriptors"
    };
    
    std::unordered_set<std::string> all_descriptors;
    
    for (const auto& path : var_paths) {
        const json* descriptors_ptr = resolve_path(instance, path);
        if (descriptors_ptr == nullptr || !descriptors_ptr->is_array()) {
            continue;
        }
        
        for (const auto& desc : *descriptors_ptr) {
            if (!desc.is_string()) {
                continue;
            }
            const std::string& descriptor = desc.get<std::string>();
            
            if (all_descriptors.count(descriptor) > 0) {
                throw ValidationError(
                    "Duplicate variable descriptor '" + descriptor + "' found across variable types"
                );
            }
            all_descriptors.insert(descriptor);
        }
    }
    
    return json::object();
}


json check_adjacency_matrix(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields[0] = adjacency_matrix path
    // fields[1] = flags path ("" means all variables are categorical)
    // fields[2] = elements_per_variable path
    // fields[3] = elements path
    // fields[4] = count path
    
    if (fields.size() < 5) {
        return json::object();
    }
    
    const std::string& adj_path = fields[0];
    const std::string& flags_path = fields[1];
    const std::string& epv_path = fields[2];
    const std::string& elements_path = fields[3];
    const std::string& count_path = fields[4];
    
    // Get adjacency_matrix - skip if null
    const json* adj_ptr = resolve_path(instance, adj_path);
    if (adj_ptr == nullptr || !adj_ptr->is_array()) {
        return json::object();
    }
    const json& adj = *adj_ptr;
    
    // Validate all entries are 0 or 1
    for (size_t i = 0; i < adj.size(); ++i) {
        int64_t val = adj[i].get<int64_t>();
        if (val != 0 && val != 1) {
            throw ValidationError(
                adj_path + "[" + std::to_string(i) + "] = " + std::to_string(val) +
                " is invalid; adjacency matrix entries must be 0 or 1"
            );
        }
    }
    
    // Get count
    const json* count_ptr = resolve_path(instance, count_path);
    if (count_ptr == nullptr) {
        return json::object();
    }
    int64_t count = count_ptr->get<int64_t>();
    if (count <= 0) {
        return json::object();
    }
    
    // Determine categorical variable indices
    std::vector<int64_t> cat_indices;
    if (flags_path.empty()) {
        // All variables are categorical (string set types)
        for (int64_t i = 0; i < count; ++i) {
            cat_indices.push_back(i);
        }
    } else {
        const json* flags_ptr = resolve_path(instance, flags_path);
        if (flags_ptr == nullptr || !flags_ptr->is_array()) {
            return json::object();
        }
        const json& flags = *flags_ptr;
        for (size_t i = 0; i < flags.size(); ++i) {
            const std::string& wholeflag = flags[i].get<std::string>();
            const char flag = static_cast<char>(std::tolower(static_cast<char>(wholeflag[0])));
            if (flag == 'y' || flag == 't') {
                cat_indices.push_back(static_cast<int64_t>(i));
            }
        }
    }
    
    if (cat_indices.empty()) {
        // No categorical variables — adjacency_matrix should be empty or absent
        if (!adj.empty()) {
            throw ValidationError(
                "adjacency_matrix has " + std::to_string(adj.size()) +
                " elements but no variables are categorical"
            );
        }
        return json::object();
    }
    
    // Get elements and elements_per_variable
    const json* elements_ptr = resolve_path(instance, elements_path);
    if (elements_ptr == nullptr || !elements_ptr->is_array()) {
        return json::object();
    }
    auto elements_len = static_cast<int64_t>(elements_ptr->size());
    
    const json* epv_ptr = resolve_path(instance, epv_path);
    
    // Compute k for each categorical variable and expected total length
    int64_t expected = 0;
    for (int64_t idx : cat_indices) {
        int64_t k;
        if (epv_ptr != nullptr && epv_ptr->is_array() && !epv_ptr->empty()) {
            k = (*epv_ptr)[static_cast<size_t>(idx)].get<int64_t>();
        } else {
            // Uniform: each variable has elements_len / count elements
            k = elements_len / count;
        }
        expected += k * k;
    }
    
    auto actual = static_cast<int64_t>(adj.size());
    if (actual != expected) {
        throw ValidationError(
            "adjacency_matrix has " + std::to_string(actual) +
            " elements but expected " + std::to_string(expected) +
            " (sum of k*k for " + std::to_string(cat_indices.size()) +
            " categorical variable(s))"
        );
    }
    
    return json::object();
}


}  // namespace dakota::validation
