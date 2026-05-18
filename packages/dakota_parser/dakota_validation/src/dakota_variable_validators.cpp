// dakota_variable_validators.cpp
// Implementation of distribution-specific validation functions

#include <dakota/variable_validators.hpp>
#include <sstream>
#include <algorithm>
#include <array>

namespace dakota::validation {

// ============================================================================
// Helper: Check if field is missing or null
// ============================================================================

static bool is_missing_or_null(const json& instance, const std::string& field) {
    return !instance.contains(field) || instance.at(field).is_null();
}

/// Resolve a dotted path (e.g. "parameters.option_1.means") in a JSON object.
/// Returns nullptr if any component is missing or null.
static const json* resolve_path(const json& instance, const std::string& path) {
    const json* current = &instance;
    std::istringstream ss(path);
    std::string component;
    while (std::getline(ss, component, '.')) {
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

static const std::string* initial_point_flag_field(
    const std::vector<std::string>& fields,
    std::size_t index
);

static void mark_initial_point_user_provided(
    json& mutations,
    const std::vector<std::string>& fields,
    std::size_t index
);

static void reject_internal_field_input(
    const json& instance,
    const std::string& field,
    const std::string& context
) {
    if (instance.contains(field) && !instance.at(field).is_null()) {
        throw ValidationError(
            "For " + context + ", '" + field + "' is an internal-only field and "
            "must not be provided by the user"
        );
    }
}

static void reject_initial_point_flag_input(
    const json& instance,
    const std::vector<std::string>& fields,
    std::size_t index,
    const std::string& context
) {
    if (const auto* flag_field = initial_point_flag_field(fields, index))
        reject_internal_field_input(instance, *flag_field, context);
}

// ============================================================================
// Continuous Aleatory Uncertain Validators
// ============================================================================

json normal_uncertain_bounds(
    const json& instance,
    const std::vector<std::string>& /*fields*/,
    const json& /*literals*/,
    const std::string& context
) {
    const std::string lower_field = "lower_bounds";
    const std::string upper_field = "upper_bounds";
    const std::string inf_lower_field = "inferred_lower_bounds";
    const std::string inf_upper_field = "inferred_upper_bounds";
    const std::string means_field = "means";
    const std::string std_field = "std_deviations";
    const std::string count_field = "count";

    reject_internal_field_input(instance, inf_lower_field, context);
    reject_internal_field_input(instance, inf_upper_field, context);

    int count = instance.at(count_field).get<int>();

    const auto& means = instance.at(means_field);
    const auto& std_devs = instance.at(std_field);
    bool lb_specified = !is_missing_or_null(instance, lower_field);
    bool ub_specified = !is_missing_or_null(instance, upper_field);

    constexpr double inf = std::numeric_limits<double>::infinity();
    json mutations;

    // --- lower bounds ---
    if (lb_specified) {
        mutations[inf_lower_field] = instance.at(lower_field);
    } else {
        json dist_lower = json::array();
        json inferred_lower = json::array();
        for (int j = 0; j < count; ++j) {
            double mean = means[j].get<double>();
            double stdev = std_devs[j].get<double>();
            dist_lower.push_back(-inf);
            inferred_lower.push_back(mean - 3.0 * stdev);
        }
        mutations[lower_field] = dist_lower;
        mutations[inf_lower_field] = inferred_lower;
    }

    // --- upper bounds ---
    if (ub_specified) {
        mutations[inf_upper_field] = instance.at(upper_field);
    } else {
        json dist_upper = json::array();
        json inferred_upper = json::array();
        for (int j = 0; j < count; ++j) {
            double mean = means[j].get<double>();
            double stdev = std_devs[j].get<double>();
            dist_upper.push_back(inf);
            inferred_upper.push_back(mean + 3.0 * stdev);
        }
        mutations[upper_field] = dist_upper;
        mutations[inf_upper_field] = inferred_upper;
    }

    return mutations;
}

json lognormal_uncertain_bounds(
    const json& instance,
    const std::vector<std::string>& /*fields*/,
    const json& /*literals*/,
    const std::string& context
) {
    // Field names are fixed for lognormal_uncertain — no reuse elsewhere.
    const std::string count_field       = "count";
    const std::string lower_field       = "lower_bounds";
    const std::string upper_field       = "upper_bounds";
    const std::string inf_upper_field   = "inferred_upper_bounds";
    const std::string opt1_means_path   = "parameters.option_1.means";
    const std::string opt1_stdev_path   = "parameters.option_1.std_deviations";
    const std::string opt2_means_path   = "parameters.option_2.means";
    const std::string opt2_errfact_path = "parameters.option_2.error_factors";
    const std::string opt3_lambda_path  = "parameters.option_3.lambdas";
    const std::string opt3_zeta_path    = "parameters.option_3.zetas";

    reject_internal_field_input(instance, inf_upper_field, context);

    int count = instance.at(count_field).get<int>();

    bool lb_specified = !is_missing_or_null(instance, lower_field);
    bool ub_specified = !is_missing_or_null(instance, upper_field);

    constexpr double inf = std::numeric_limits<double>::infinity();
    json mutations;

    // --- lower bounds: default to 0 (lognormal support > 0) ---
    if (!lb_specified) {
        json dist_lower = json::array();
        for (int j = 0; j < count; ++j) {
            dist_lower.push_back(0.0);
        }
        mutations[lower_field] = dist_lower;
    }

    // --- upper bounds ---
    if (ub_specified) {
        mutations[inf_upper_field] = instance.at(upper_field);
    } else {
        // Determine which parameterization is active and resolve arrays
        const json* opt1_means = resolve_path(instance, opt1_means_path);
        const json* opt1_stdev = resolve_path(instance, opt1_stdev_path);
        const json* opt2_means = resolve_path(instance, opt2_means_path);
        const json* opt2_errfact = resolve_path(instance, opt2_errfact_path);
        const json* opt3_lambda = resolve_path(instance, opt3_lambda_path);
        const json* opt3_zeta = resolve_path(instance, opt3_zeta_path);

        // Identify which option is present
        bool has_opt1 = opt1_means && opt1_stdev;
        bool has_opt2 = opt2_means && opt2_errfact;
        bool has_opt3 = opt3_lambda && opt3_zeta;

        if (!has_opt1 && !has_opt2 && !has_opt3) {
            // No parameterization available; just default dist upper to inf
            json dist_upper = json::array();
            for (int j = 0; j < count; ++j) {
                dist_upper.push_back(inf);
            }
            mutations[upper_field] = dist_upper;
            return mutations;
        }

        json dist_upper = json::array();
        json inferred_upper = json::array();
        for (int j = 0; j < count; ++j) {
            double mean, stdev;
            if (has_opt1) {
                mean = (*opt1_means)[j].get<double>();
                stdev = (*opt1_stdev)[j].get<double>();
            } else if (has_opt2) {
                mean = (*opt2_means)[j].get<double>();
                double ef = (*opt2_errfact)[j].get<double>();
                stdev = lognormal_stdev_from_error_factor(mean, ef);
            } else { // has_opt3
                double lam = (*opt3_lambda)[j].get<double>();
                double zet = (*opt3_zeta)[j].get<double>();
                auto [m, s] = lognormal_moments_from_params(lam, zet);
                mean = m;
                stdev = s;
            }
            dist_upper.push_back(inf);
            inferred_upper.push_back(mean + 3.0 * stdev);
        }
        mutations[upper_field] = dist_upper;
        mutations[inf_upper_field] = inferred_upper;
    }

    return mutations;
}

json normal_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& /*fields*/,
    const json& /*literals*/,
    const std::string& context
) {
    const std::string ip_field = "initial_point";
    const std::string means_field = "means";
    const std::string std_field = "std_deviations";
    const std::string lower_field = "lower_bounds";
    const std::string upper_field = "upper_bounds";
    const std::string count_field = "count";
    const std::string initial_point_user_provided_field =
      "initial_point_user_provided";
    
    reject_internal_field_input(instance, initial_point_user_provided_field, context);

    int count = instance.at(count_field).get<int>();
    
    // Check if initial_point needs to be set
    if (!is_missing_or_null(instance, ip_field)) {
        json mutations;
        mutations[initial_point_user_provided_field] = true;
        return mutations;
    }
    
    const auto& means = instance.at(means_field);
    const auto& std_devs = instance.at(std_field);
    
    // Determine bounds flags
    bool lb_specified = !is_missing_or_null(instance, lower_field);
    bool ub_specified = !is_missing_or_null(instance, upper_field);
    
    int bds = 0;
    if (lb_specified) bds |= 1;
    if (ub_specified) bds |= 2;
    
    json initial_point = json::array();
    
    for (int j = 0; j < count; ++j) {
        double mean = means[j].get<double>();
        double stdev = std_devs[j].get<double>();
        
        double val;
        if (bds == 0) {
            // No bounds: use mean directly
            val = mean;
        } else if (bds == 1) {
            // Only lower bounds
            double lb = instance.at(lower_field)[j].get<double>();
            if (mean <= lb) {
                val = lb + 0.5 * stdev;
            } else {
                val = mean;
            }
        } else if (bds == 2) {
            // Only upper bounds
            double ub = instance.at(upper_field)[j].get<double>();
            if (mean >= ub) {
                val = ub - 0.5 * stdev;
            } else {
                val = mean;
            }
        } else {
            // Both bounds - use nudge
            double lb = instance.at(lower_field)[j].get<double>();
            double ub = instance.at(upper_field)[j].get<double>();
            double nudge = 0.5 * std::min(stdev, ub - lb);
            double lower = lb + nudge;
            double upper = ub - nudge;
            
            if (mean < lower) {
                val = lower;
            } else if (mean > upper) {
                val = upper;
            } else {
                val = mean;
            }
        }
        initial_point.push_back(val);
    }
    
    json mutations;
    mutations[ip_field] = initial_point;
    return mutations;
}

json lognormal_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    (void)fields;
    (void)literals;

    // Field names are fixed for lognormal_uncertain.
    const std::string ip_field = "initial_point";
    const std::string lower_field = "lower_bounds";
    const std::string upper_field = "upper_bounds";
    const std::string count_field = "count";
    const std::string initial_point_user_provided_field =
      "initial_point_user_provided";

    const std::string opt1_means_path   = "parameters.option_1.means";
    const std::string opt1_stdev_path   = "parameters.option_1.std_deviations";
    const std::string opt2_means_path   = "parameters.option_2.means";
    const std::string opt2_errfact_path = "parameters.option_2.error_factors";
    const std::string opt3_lambda_path  = "parameters.option_3.lambdas";
    const std::string opt3_zeta_path    = "parameters.option_3.zetas";
    
    reject_internal_field_input(instance, initial_point_user_provided_field, context);

    int count = instance.at(count_field).get<int>();
    
    if (!is_missing_or_null(instance, ip_field)) {
        json mutations;
        mutations[initial_point_user_provided_field] = true;
        return mutations;
    }

    const json* opt1_means = resolve_path(instance, opt1_means_path);
    const json* opt1_stdev = resolve_path(instance, opt1_stdev_path);
    const json* opt2_means = resolve_path(instance, opt2_means_path);
    const json* opt2_errfact = resolve_path(instance, opt2_errfact_path);
    const json* opt3_lambda = resolve_path(instance, opt3_lambda_path);
    const json* opt3_zeta = resolve_path(instance, opt3_zeta_path);

    bool has_opt1 = opt1_means && opt1_stdev;
    bool has_opt2 = opt2_means && opt2_errfact;
    bool has_opt3 = opt3_lambda && opt3_zeta;

    if (!has_opt1 && !has_opt2 && !has_opt3) {
        return json::object();
    }

    bool lb_specified = !is_missing_or_null(instance, lower_field);
    bool ub_specified = !is_missing_or_null(instance, upper_field);
    
    json initial_point = json::array();
    constexpr double inf = std::numeric_limits<double>::infinity();
    
    for (int j = 0; j < count; ++j) {
        double mean, stdev;
        if (has_opt1) {
            mean = (*opt1_means)[j].get<double>();
            stdev = (*opt1_stdev)[j].get<double>();
        } else if (has_opt2) {
            mean = (*opt2_means)[j].get<double>();
            double ef = (*opt2_errfact)[j].get<double>();
            stdev = lognormal_stdev_from_error_factor(mean, ef);
        } else {
            double lam = (*opt3_lambda)[j].get<double>();
            double zet = (*opt3_zeta)[j].get<double>();
            auto [m, s] = lognormal_moments_from_params(lam, zet);
            mean = m;
            stdev = s;
        }
        double lb = lb_specified ? instance.at(lower_field)[j].get<double>() : 0.0;
        double ub = ub_specified ? instance.at(upper_field)[j].get<double>() : inf;
        
        // Repair mean to bounds with nudge
        double nudge = std::isfinite(ub) ? 0.5 * std::min(stdev, ub - lb) : 0.5 * stdev;
        double lower = lb + nudge;
        double upper = std::isfinite(ub) ? ub - nudge : inf;
        
        double val;
        if (mean < lower) {
            val = lower;
        } else if (std::isfinite(upper) && mean > upper) {
            val = upper;
        } else {
            val = mean;
        }
        initial_point.push_back(val);
    }
    
    json mutations;
    mutations[ip_field] = initial_point;
    return mutations;
}

json uniform_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields: [initial_point, lower_bounds, upper_bounds, count]
    const std::string& ip_field = fields[0];
    const std::string& lower_field = fields[1];
    const std::string& upper_field = fields[2];
    const std::string& count_field = fields[3];
    
    reject_initial_point_flag_input(instance, fields, 4, context);

    int count = instance.at(count_field).get<int>();
    
    if (!is_missing_or_null(instance, ip_field)) {
        // Repair existing values to bounds
        json ip = instance.at(ip_field);
        const auto& lower = instance.at(lower_field);
        const auto& upper = instance.at(upper_field);
        bool changed = false;
        
        for (int j = 0; j < count; ++j) {
            double val = ip[j].get<double>();
            double lb = lower[j].get<double>();
            double ub = upper[j].get<double>();
            
            if (val < lb) {
                ip[j] = lb;
                changed = true;
            } else if (val > ub) {
                ip[j] = ub;
                changed = true;
            }
        }
        
        json mutations;
        if (changed)
            mutations[ip_field] = ip;
        mark_initial_point_user_provided(mutations, fields, 4);
        return mutations;
    }
    
    // Set default from mean
    const auto& lower = instance.at(lower_field);
    const auto& upper = instance.at(upper_field);
    
    json initial_point = json::array();
    for (int j = 0; j < count; ++j) {
        initial_point.push_back(uniform_mean(
            lower[j].get<double>(), 
            upper[j].get<double>()
        ));
    }
    
    json mutations;
    mutations[ip_field] = initial_point;
    return mutations;
}

json loguniform_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields: [initial_point, lower_bounds, upper_bounds, count]
    const std::string& ip_field = fields[0];
    const std::string& lower_field = fields[1];
    const std::string& upper_field = fields[2];
    const std::string& count_field = fields[3];
    
    reject_initial_point_flag_input(instance, fields, 4, context);

    int count = instance.at(count_field).get<int>();
    
    const auto& lower = instance.at(lower_field);
    const auto& upper = instance.at(upper_field);
    
    // Validate bounds are positive and finite
    for (int j = 0; j < count; ++j) {
        double lb = lower[j].get<double>();
        double ub = upper[j].get<double>();
        
        if (lb <= 0 || ub <= 0) {
            throw ValidationError("loguniform bounds must be positive");
        }
        if (!std::isfinite(lb) || !std::isfinite(ub)) {
            throw ValidationError("loguniform bounds must be finite");
        }
        if (lb > ub) {
            throw ValidationError("loguniform lower bound greater than upper bound");
        }
    }
    
    if (!is_missing_or_null(instance, ip_field)) {
        // Repair existing values
        json ip = instance.at(ip_field);
        bool changed = false;
        
        for (int j = 0; j < count; ++j) {
            double val = ip[j].get<double>();
            double lb = lower[j].get<double>();
            double ub = upper[j].get<double>();
            
            if (val < lb) {
                ip[j] = lb;
                changed = true;
            } else if (val > ub) {
                ip[j] = ub;
                changed = true;
            }
        }
        
        json mutations;
        if (changed)
            mutations[ip_field] = ip;
        mark_initial_point_user_provided(mutations, fields, 4);
        return mutations;
    }
    
    // Set default from mean
    json initial_point = json::array();
    for (int j = 0; j < count; ++j) {
        initial_point.push_back(loguniform_mean(
            lower[j].get<double>(), 
            upper[j].get<double>()
        ));
    }
    
    json mutations;
    mutations[ip_field] = initial_point;
    return mutations;
}

json triangular_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields: [initial_point, modes, lower_bounds, upper_bounds, count]
    const std::string& ip_field = fields[0];
    const std::string& modes_field = fields[1];
    const std::string& lower_field = fields[2];
    const std::string& upper_field = fields[3];
    const std::string& count_field = fields[4];
    
    reject_initial_point_flag_input(instance, fields, 5, context);

    int count = instance.at(count_field).get<int>();
    
    const auto& modes = instance.at(modes_field);
    const auto& lower = instance.at(lower_field);
    const auto& upper = instance.at(upper_field);
    
    // Validate ordering
    for (int j = 0; j < count; ++j) {
        double lb = lower[j].get<double>();
        double mode = modes[j].get<double>();
        double ub = upper[j].get<double>();
        
        if (lb > mode || mode > ub) {
            throw ValidationError(
                "triangular uncertain variables must have "
                "lower_bounds <= modes <= upper_bounds"
            );
        }
    }
    
    if (!is_missing_or_null(instance, ip_field)) {
        // Repair existing values
        json ip = instance.at(ip_field);
        bool changed = false;
        
        for (int j = 0; j < count; ++j) {
            double val = ip[j].get<double>();
            double lb = lower[j].get<double>();
            double ub = upper[j].get<double>();
            
            if (val < lb) {
                ip[j] = lb;
                changed = true;
            } else if (val > ub) {
                ip[j] = ub;
                changed = true;
            }
        }
        
        json mutations;
        if (changed)
            mutations[ip_field] = ip;
        mark_initial_point_user_provided(mutations, fields, 5);
        return mutations;
    }
    
    // Set default from mean
    json initial_point = json::array();
    for (int j = 0; j < count; ++j) {
        initial_point.push_back(triangular_mean(
            lower[j].get<double>(),
            modes[j].get<double>(),
            upper[j].get<double>()
        ));
    }
    
    json mutations;
    mutations[ip_field] = initial_point;
    return mutations;
}

json exponential_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields: [initial_point, betas, count]
    const std::string& ip_field = fields[0];
    const std::string& betas_field = fields[1];
    const std::string& count_field = fields[2];
    
    reject_initial_point_flag_input(instance, fields, 3, context);

    int count = instance.at(count_field).get<int>();
    
    if (!is_missing_or_null(instance, ip_field)) {
        json mutations;
        mark_initial_point_user_provided(mutations, fields, 3);
        return mutations;
    }
    
    const auto& betas = instance.at(betas_field);
    
    json initial_point = json::array();
    for (int j = 0; j < count; ++j) {
        auto [mean, stdev] = exponential_moments(betas[j].get<double>());
        initial_point.push_back(mean);
    }
    
    json mutations;
    mutations[ip_field] = initial_point;
    return mutations;
}

json beta_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields: [initial_point, alphas, betas, lower_bounds, upper_bounds, count]
    const std::string& ip_field = fields[0];
    const std::string& alphas_field = fields[1];
    const std::string& betas_field = fields[2];
    const std::string& lower_field = fields[3];
    const std::string& upper_field = fields[4];
    const std::string& count_field = fields[5];
    
    reject_initial_point_flag_input(instance, fields, 6, context);

    int count = instance.at(count_field).get<int>();
    
    if (!is_missing_or_null(instance, ip_field)) {
        // Repair existing values
        json ip = instance.at(ip_field);
        const auto& lower = instance.at(lower_field);
        const auto& upper = instance.at(upper_field);
        bool changed = false;
        
        for (int j = 0; j < count; ++j) {
            double val = ip[j].get<double>();
            double lb = lower[j].get<double>();
            double ub = upper[j].get<double>();
            
            if (val < lb) {
                ip[j] = lb;
                changed = true;
            } else if (val > ub) {
                ip[j] = ub;
                changed = true;
            }
        }
        
        json mutations;
        if (changed)
            mutations[ip_field] = ip;
        mark_initial_point_user_provided(mutations, fields, 6);
        return mutations;
    }
    
    // Set default from mean
    const auto& alphas = instance.at(alphas_field);
    const auto& betas = instance.at(betas_field);
    const auto& lower = instance.at(lower_field);
    const auto& upper = instance.at(upper_field);
    
    json initial_point = json::array();
    for (int j = 0; j < count; ++j) {
        initial_point.push_back(beta_mean(
            alphas[j].get<double>(),
            betas[j].get<double>(),
            lower[j].get<double>(),
            upper[j].get<double>()
        ));
    }
    
    json mutations;
    mutations[ip_field] = initial_point;
    return mutations;
}

json gamma_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields: [initial_point, alphas, betas, count]
    const std::string& ip_field = fields[0];
    const std::string& alphas_field = fields[1];
    const std::string& betas_field = fields[2];
    const std::string& count_field = fields[3];
    
    reject_initial_point_flag_input(instance, fields, 4, context);

    int count = instance.at(count_field).get<int>();
    
    if (!is_missing_or_null(instance, ip_field)) {
        json mutations;
        mark_initial_point_user_provided(mutations, fields, 4);
        return mutations;
    }
    
    const auto& alphas = instance.at(alphas_field);
    const auto& betas = instance.at(betas_field);
    
    json initial_point = json::array();
    for (int j = 0; j < count; ++j) {
        auto [mean, stdev] = gamma_moments(
            alphas[j].get<double>(), 
            betas[j].get<double>()
        );
        initial_point.push_back(mean);
    }
    
    json mutations;
    mutations[ip_field] = initial_point;
    return mutations;
}

json gumbel_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields: [initial_point, alphas, betas, count]
    const std::string& ip_field = fields[0];
    const std::string& alphas_field = fields[1];
    const std::string& betas_field = fields[2];
    const std::string& count_field = fields[3];
    
    reject_initial_point_flag_input(instance, fields, 4, context);

    int count = instance.at(count_field).get<int>();
    
    if (!is_missing_or_null(instance, ip_field)) {
        json mutations;
        mark_initial_point_user_provided(mutations, fields, 4);
        return mutations;
    }
    
    const auto& alphas = instance.at(alphas_field);
    const auto& betas = instance.at(betas_field);
    
    json initial_point = json::array();
    for (int j = 0; j < count; ++j) {
        auto [mean, stdev] = gumbel_moments(
            alphas[j].get<double>(), 
            betas[j].get<double>()
        );
        initial_point.push_back(mean);
    }
    
    json mutations;
    mutations[ip_field] = initial_point;
    return mutations;
}

json frechet_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields: [initial_point, alphas, betas, count]
    const std::string& ip_field = fields[0];
    const std::string& alphas_field = fields[1];
    const std::string& betas_field = fields[2];
    const std::string& count_field = fields[3];
    
    reject_initial_point_flag_input(instance, fields, 4, context);

    int count = instance.at(count_field).get<int>();
    
    if (!is_missing_or_null(instance, ip_field)) {
        json mutations;
        mark_initial_point_user_provided(mutations, fields, 4);
        return mutations;
    }
    
    const auto& alphas = instance.at(alphas_field);
    const auto& betas = instance.at(betas_field);
    
    json initial_point = json::array();
    for (int j = 0; j < count; ++j) {
        auto [mean, stdev] = frechet_moments(
            alphas[j].get<double>(), 
            betas[j].get<double>()
        );
        initial_point.push_back(mean);
    }
    
    json mutations;
    mutations[ip_field] = initial_point;
    return mutations;
}

json weibull_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields: [initial_point, alphas, betas, count]
    const std::string& ip_field = fields[0];
    const std::string& alphas_field = fields[1];
    const std::string& betas_field = fields[2];
    const std::string& count_field = fields[3];
    
    reject_initial_point_flag_input(instance, fields, 4, context);

    int count = instance.at(count_field).get<int>();
    
    if (!is_missing_or_null(instance, ip_field)) {
        json mutations;
        mark_initial_point_user_provided(mutations, fields, 4);
        return mutations;
    }
    
    const auto& alphas = instance.at(alphas_field);
    const auto& betas = instance.at(betas_field);
    
    json initial_point = json::array();
    for (int j = 0; j < count; ++j) {
        auto [mean, stdev] = weibull_moments(
            alphas[j].get<double>(), 
            betas[j].get<double>()
        );
        initial_point.push_back(mean);
    }
    
    json mutations;
    mutations[ip_field] = initial_point;
    return mutations;
}

// ============================================================================
// Discrete Aleatory Uncertain Validators
// ============================================================================

json poisson_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields: [initial_point, lambdas, count]
    const std::string& ip_field = fields[0];
    const std::string& lambdas_field = fields[1];
    const std::string& count_field = fields[2];
    
    reject_initial_point_flag_input(instance, fields, 3, context);

    int count = instance.at(count_field).get<int>();
    
    if (!is_missing_or_null(instance, ip_field)) {
        json mutations;
        mark_initial_point_user_provided(mutations, fields, 4);
        return mutations;
    }
    
    const auto& lambdas = instance.at(lambdas_field);
    
    json initial_point = json::array();
    for (int j = 0; j < count; ++j) {
        auto [mean, stdev] = poisson_moments(lambdas[j].get<double>());
        initial_point.push_back(static_cast<int>(mean));
    }
    
    json mutations;
    mutations[ip_field] = initial_point;
    return mutations;
}

json binomial_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields: [initial_point, probability_per_trial, num_trials, count]
    const std::string& ip_field = fields[0];
    const std::string& prob_field = fields[1];
    const std::string& trials_field = fields[2];
    const std::string& count_field = fields[3];
    
    reject_initial_point_flag_input(instance, fields, 4, context);

    int count = instance.at(count_field).get<int>();
    
    if (!is_missing_or_null(instance, ip_field)) {
        // Repair to upper bound (num_trials)
        json ip = instance.at(ip_field);
        const auto& trials = instance.at(trials_field);
        bool changed = false;
        
        for (int j = 0; j < count; ++j) {
            int val = ip[j].get<int>();
            int ub = trials[j].get<int>();
            
            if (val > ub) {
                ip[j] = ub;
                changed = true;
            } else if (val < 0) {
                ip[j] = 0;
                changed = true;
            }
        }
        
        json mutations;
        if (changed)
            mutations[ip_field] = ip;
        mark_initial_point_user_provided(mutations, fields, 4);
        return mutations;
    }
    
    const auto& probs = instance.at(prob_field);
    const auto& trials = instance.at(trials_field);
    
    json initial_point = json::array();
    for (int j = 0; j < count; ++j) {
        auto [mean, stdev] = binomial_moments(
            trials[j].get<int>(),
            probs[j].get<double>()
        );
        initial_point.push_back(static_cast<int>(mean));
    }
    
    json mutations;
    mutations[ip_field] = initial_point;
    return mutations;
}

json negative_binomial_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields: [initial_point, probability_per_trial, num_trials, count]
    const std::string& ip_field = fields[0];
    const std::string& prob_field = fields[1];
    const std::string& trials_field = fields[2];
    const std::string& count_field = fields[3];
    
    reject_initial_point_flag_input(instance, fields, 4, context);

    int count = instance.at(count_field).get<int>();
    
    if (!is_missing_or_null(instance, ip_field)) {
        // Repair to lower bound (num_trials)
        json ip = instance.at(ip_field);
        const auto& trials = instance.at(trials_field);
        bool changed = false;
        
        for (int j = 0; j < count; ++j) {
            int val = ip[j].get<int>();
            int lb = trials[j].get<int>();
            
            if (val < lb) {
                ip[j] = lb;
                changed = true;
            }
        }
        
        json mutations;
        if (changed)
            mutations[ip_field] = ip;
        mark_initial_point_user_provided(mutations, fields, 4);
        return mutations;
    }
    
    const auto& probs = instance.at(prob_field);
    const auto& trials = instance.at(trials_field);
    
    json initial_point = json::array();
    for (int j = 0; j < count; ++j) {
        auto [mean, stdev] = negative_binomial_moments(
            trials[j].get<int>(),
            probs[j].get<double>()
        );
        initial_point.push_back(static_cast<int>(mean));
    }
    
    json mutations;
    mutations[ip_field] = initial_point;
    return mutations;
}

json geometric_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields: [initial_point, probability_per_trial, count]
    const std::string& ip_field = fields[0];
    const std::string& prob_field = fields[1];
    const std::string& count_field = fields[2];
    
    reject_initial_point_flag_input(instance, fields, 3, context);

    int count = instance.at(count_field).get<int>();
    
    if (!is_missing_or_null(instance, ip_field)) {
        json mutations;
        mark_initial_point_user_provided(mutations, fields, 3);
        return mutations;
    }
    
    const auto& probs = instance.at(prob_field);
    
    json initial_point = json::array();
    for (int j = 0; j < count; ++j) {
        auto [mean, stdev] = geometric_moments(probs[j].get<double>());
        initial_point.push_back(static_cast<int>(mean));
    }
    
    json mutations;
    mutations[ip_field] = initial_point;
    return mutations;
}

json hypergeometric_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields: [initial_point, total_population, selected_population, num_drawn, count]
    const std::string& ip_field = fields[0];
    const std::string& total_field = fields[1];
    const std::string& selected_field = fields[2];
    const std::string& drawn_field = fields[3];
    const std::string& count_field = fields[4];
    
    reject_initial_point_flag_input(instance, fields, 5, context);

    int count = instance.at(count_field).get<int>();
    
    if (!is_missing_or_null(instance, ip_field)) {
        // Repair to upper bound = min(num_drawn, selected_population)
        json ip = instance.at(ip_field);
        const auto& selected = instance.at(selected_field);
        const auto& drawn = instance.at(drawn_field);
        bool changed = false;
        
        for (int j = 0; j < count; ++j) {
            int val = ip[j].get<int>();
            int ub = std::min(drawn[j].get<int>(), selected[j].get<int>());
            
            if (val > ub) {
                ip[j] = ub;
                changed = true;
            } else if (val < 0) {
                ip[j] = 0;
                changed = true;
            }
        }
        
        json mutations;
        if (changed)
            mutations[ip_field] = ip;
        mark_initial_point_user_provided(mutations, fields, 5);
        return mutations;
    }
    
    const auto& total = instance.at(total_field);
    const auto& selected = instance.at(selected_field);
    const auto& drawn = instance.at(drawn_field);
    
    json initial_point = json::array();
    for (int j = 0; j < count; ++j) {
        double mean = hypergeometric_mean(
            total[j].get<int>(),
            selected[j].get<int>(),
            drawn[j].get<int>()
        );
        initial_point.push_back(static_cast<int>(mean));
    }
    
    json mutations;
    mutations[ip_field] = initial_point;
    return mutations;
}

// ============================================================================
// Helpers for flat-array apportionment
// ============================================================================

/// Compute per-variable counts from a per_variable field or even split.
static std::vector<int> compute_apportionment(
    int count,
    const json& instance,
    const std::string& pv_field,
    size_t total_len
) {
    std::vector<int> result;
    
    if (!is_missing_or_null(instance, pv_field)) {
        const auto& pv = instance.at(pv_field);
        for (int i = 0; i < count; ++i) {
            result.push_back(pv[i].get<int>());
        }
    } else {
        if (count == 0) return result;
        if (total_len % count != 0) {
            throw ValidationError(
                "Number of elements (" + std::to_string(total_len) +
                ") not evenly divisible by number of variables (" +
                std::to_string(count) + ")"
            );
        }
        int avg = static_cast<int>(total_len / count);
        result.assign(count, avg);
    }
    
    return result;
}

/// Return the middle index (or next lower for even n).
/// Matches C++ mid_or_next_lower_index: midpoint(0, n-1) = (n-1)/2
static int mid_or_next_lower_index(int n) {
    return (n - 1) / 2;
}

static const std::string* initial_point_flag_field(
    const std::vector<std::string>& fields,
    std::size_t index
) {
    if (fields.size() > index && !fields[index].empty())
        return &fields[index];
    return nullptr;
}

static void mark_initial_point_user_provided(
    json& mutations,
    const std::vector<std::string>& fields,
    std::size_t index
) {
    if (const auto* flag_field = initial_point_flag_field(fields, index))
        mutations[*flag_field] = true;
}

// ============================================================================
// Epistemic Uncertain - Interval Types
// ============================================================================

json continuous_interval_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    const std::string& ip_field = fields[0];
    const std::string& lb_field = fields[1];
    const std::string& ub_field = fields[2];
    const std::string& ni_field = fields[3];
    const std::string& count_field = fields[4];
    
    reject_initial_point_flag_input(instance, fields, 5, context);

    int count = instance.at(count_field).get<int>();
    
    const auto& flat_lb = instance.at(lb_field);
    const auto& flat_ub = instance.at(ub_field);
    auto apportionment = compute_apportionment(count, instance, ni_field, flat_lb.size());
    
    double dbl_inf = std::numeric_limits<double>::infinity();
    std::vector<double> global_lb(count), global_ub(count);
    int offset = 0;
    for (int j = 0; j < count; ++j) {
        double var_lb = dbl_inf, var_ub = -dbl_inf;
        for (int k = 0; k < apportionment[j]; ++k) {
            double lbk = flat_lb[offset + k].get<double>();
            double ubk = flat_ub[offset + k].get<double>();
            if (var_lb > lbk) var_lb = lbk;
            if (var_ub < ubk) var_ub = ubk;
        }
        global_lb[j] = var_lb;
        global_ub[j] = var_ub;
        offset += apportionment[j];
    }
    
    if (!is_missing_or_null(instance, ip_field)) {
        json ip = instance.at(ip_field);
        bool changed = false;
        for (int j = 0; j < count; ++j) {
            double val = ip[j].get<double>();
            if (val < global_lb[j]) { ip[j] = global_lb[j]; changed = true; }
            else if (val > global_ub[j]) { ip[j] = global_ub[j]; changed = true; }
        }
        json m;
        if (changed)
            m[ip_field] = ip;
        mark_initial_point_user_provided(m, fields, 5);
        return m;
    }
    
    json initial_point = json::array();
    for (int j = 0; j < count; ++j)
        initial_point.push_back((global_lb[j] + global_ub[j]) / 2.0);
    
    json m; m[ip_field] = initial_point; return m;
}

json discrete_interval_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    const std::string& ip_field = fields[0];
    const std::string& lb_field = fields[1];
    const std::string& ub_field = fields[2];
    const std::string& ni_field = fields[3];
    const std::string& count_field = fields[4];
    
    reject_initial_point_flag_input(instance, fields, 5, context);

    int count = instance.at(count_field).get<int>();
    
    const auto& flat_lb = instance.at(lb_field);
    const auto& flat_ub = instance.at(ub_field);
    auto apportionment = compute_apportionment(count, instance, ni_field, flat_lb.size());
    
    std::vector<int> global_lb(count), global_ub(count);
    int offset = 0;
    for (int j = 0; j < count; ++j) {
        int var_lb = std::numeric_limits<int>::max();
        int var_ub = std::numeric_limits<int>::min();
        for (int k = 0; k < apportionment[j]; ++k) {
            int lbk = flat_lb[offset + k].get<int>();
            int ubk = flat_ub[offset + k].get<int>();
            if (var_lb > lbk) var_lb = lbk;
            if (var_ub < ubk) var_ub = ubk;
        }
        global_lb[j] = var_lb;
        global_ub[j] = var_ub;
        offset += apportionment[j];
    }
    
    if (!is_missing_or_null(instance, ip_field)) {
        json ip = instance.at(ip_field);
        bool changed = false;
        for (int j = 0; j < count; ++j) {
            int val = ip[j].get<int>();
            if (val < global_lb[j]) { ip[j] = global_lb[j]; changed = true; }
            else if (val > global_ub[j]) { ip[j] = global_ub[j]; changed = true; }
        }
        json m;
        if (changed)
            m[ip_field] = ip;
        mark_initial_point_user_provided(m, fields, 5);
        return m;
    }
    
    // Truncate toward zero (C++ integer division semantics)
    json initial_point = json::array();
    for (int j = 0; j < count; ++j)
        initial_point.push_back((global_lb[j] + global_ub[j]) / 2);
    
    json m; m[ip_field] = initial_point; return m;
}

// ============================================================================
// Histogram Uncertain
// ============================================================================

json histogram_bin_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields: [initial_point, pairs_per_variable, abscissas, density, count]
    const std::string& ip_field = fields[0];
    const std::string& ppv_field = fields[1];
    const std::string& abs_field = fields[2];
    const std::string& density_field = fields[3];
    const std::string& count_field = fields[4];
    
    reject_initial_point_flag_input(instance, fields, 5, context);

    int count = instance.at(count_field).get<int>();
    
    const auto& abscissas = instance.at(abs_field);
    auto apport = compute_apportionment(count, instance, ppv_field, abscissas.size());
    
    // Extract density values from the union (Ordinates | Counts)
    bool has_density = false;
    bool is_counts = false;
    const json* density_values = nullptr;
    if (!is_missing_or_null(instance, density_field)) {
        const auto& density = instance.at(density_field);
        if (density.contains("counts") && !density.at("counts").is_null()) {
            density_values = &density.at("counts");
            is_counts = true;
            has_density = true;
        } else if (density.contains("ordinates") && !density.at("ordinates").is_null()) {
            density_values = &density.at("ordinates");
            is_counts = false;
            has_density = true;
        }
    }
    
    // Compute per-variable slices and bounds
    struct VarSlice { int offset; int n_pairs; };
    std::vector<VarSlice> slices(count);
    int off = 0;
    for (int j = 0; j < count; ++j) {
        slices[j] = {off, apport[j]};
        off += apport[j];
    }
    
    if (!is_missing_or_null(instance, ip_field)) {
        // Clamp to [first_abscissa, last_abscissa]
        json ip = instance.at(ip_field);
        bool changed = false;
        for (int j = 0; j < count; ++j) {
            double val = ip[j].get<double>();
            double lb = abscissas[slices[j].offset].get<double>();
            double ub = abscissas[slices[j].offset + slices[j].n_pairs - 1].get<double>();
            if (val < lb) { ip[j] = lb; changed = true; }
            else if (val > ub) { ip[j] = ub; changed = true; }
        }
        json m;
        if (changed)
            m[ip_field] = ip;
        mark_initial_point_user_provided(m, fields, 5);
        return m;
    }
    
    // Default: compute mean matching Pecos central_moments_from_params
    json initial_point = json::array();
    for (int j = 0; j < count; ++j) {
        int s_off = slices[j].offset;
        int n_p = slices[j].n_pairs;
        double lb = abscissas[s_off].get<double>();
        double ub = abscissas[s_off + n_p - 1].get<double>();
        
        if (has_density && n_p >= 2) {
            int n_bins = n_p - 1;
            double sum_mass = 0.0;
            double sum_mass_midpoint2 = 0.0;  // sum of mass * (lwr + upr)
            
            for (int i = 0; i < n_bins; ++i) {
                double lwr = abscissas[s_off + i].get<double>();
                double upr = abscissas[s_off + i + 1].get<double>();
                double y = (*density_values)[s_off + i].get<double>();
                double bin_width = upr - lwr;
                
                double mass = is_counts ? y : y * bin_width;
                sum_mass += mass;
                sum_mass_midpoint2 += mass * (lwr + upr);
            }
            
            if (sum_mass > 0.0) {
                initial_point.push_back(sum_mass_midpoint2 / (2.0 * sum_mass));
            } else {
                initial_point.push_back((lb + ub) / 2.0);
            }
        } else {
            // Fallback to midpoint if density unavailable
            initial_point.push_back((lb + ub) / 2.0);
        }
    }
    
    json m; m[ip_field] = initial_point; return m;
}

json histogram_point_numeric_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields: [initial_point, pairs_per_variable, abscissas, counts, count]
    const std::string& ip_field = fields[0];
    const std::string& ppv_field = fields[1];
    const std::string& abs_field = fields[2];
    const std::string& cnt_field = fields[3];
    const std::string& count_field = fields[4];
    
    reject_initial_point_flag_input(instance, fields, 5, context);

    int count = instance.at(count_field).get<int>();
    
    const auto& abscissas = instance.at(abs_field);
    const auto& counts = instance.at(cnt_field);
    auto apport = compute_apportionment(count, instance, ppv_field, abscissas.size());
    
    // Compute per-variable slices
    struct VarSlice { int offset; int n_pairs; };
    std::vector<VarSlice> slices(count);
    int off = 0;
    for (int j = 0; j < count; ++j) {
        slices[j] = {off, apport[j]};
        off += apport[j];
    }
    
    if (!is_missing_or_null(instance, ip_field)) {
        // Validate that each initial point is a member of its variable's abscissa set
        const json& ip = instance.at(ip_field);
        for (int j = 0; j < count; ++j) {
            bool found = false;
            for (int k = 0; k < slices[j].n_pairs; ++k) {
                if (ip[j] == abscissas[slices[j].offset + k]) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                throw ValidationError(
                    ip_field + "[" + std::to_string(j) + "] = " + ip[j].dump() +
                    " is not a member of the abscissa set for variable " +
                    std::to_string(j)
                );
            }
        }
        json mutations;
        mark_initial_point_user_provided(mutations, fields, 5);
        return mutations;
    }
    
    json initial_point = json::array();
    for (int j = 0; j < count; ++j) {
        int s_off = slices[j].offset;
        int n_p = slices[j].n_pairs;
        
        if (n_p == 1) {
            initial_point.push_back(abscissas[s_off]);
            continue;
        }
        
        // Weighted mean, snap to nearest abscissa
        double total = 0, weighted_sum = 0;
        for (int k = 0; k < n_p; ++k) {
            double a = abscissas[s_off + k].get<double>();
            double c = counts[s_off + k].get<double>();
            total += c;
            weighted_sum += a * c;
        }
        double mean = (total > 0) ? weighted_sum / total : abscissas[s_off].get<double>();
        
        // Find nearest abscissa
        double best_val = abscissas[s_off].get<double>();
        double best_dist = std::abs(mean - best_val);
        for (int k = 1; k < n_p; ++k) {
            double a = abscissas[s_off + k].get<double>();
            double d = std::abs(mean - a);
            if (d < best_dist) { best_val = a; best_dist = d; }
        }
        
        // Preserve original JSON type (int or real)
        initial_point.push_back(abscissas[s_off]);  // placeholder
        initial_point.back() = json(best_val);
        // If the source is integer-typed, cast back
        if (abscissas[s_off].is_number_integer()) {
            initial_point.back() = static_cast<int>(std::round(best_val));
        }
    }
    
    json m; m[ip_field] = initial_point; return m;
}

json histogram_point_str_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields: [initial_point, pairs_per_variable, abscissas, counts, count]
    const std::string& ip_field = fields[0];
    const std::string& ppv_field = fields[1];
    const std::string& abs_field = fields[2];
    const std::string& cnt_field = fields[3];
    const std::string& count_field = fields[4];
    
    reject_initial_point_flag_input(instance, fields, 5, context);

    int count = instance.at(count_field).get<int>();
    
    const auto& abscissas = instance.at(abs_field);
    const auto& counts = instance.at(cnt_field);
    auto apport = compute_apportionment(count, instance, ppv_field, abscissas.size());
    
    struct VarSlice { int offset; int n_pairs; };
    std::vector<VarSlice> slices(count);
    int off = 0;
    for (int j = 0; j < count; ++j) {
        slices[j] = {off, apport[j]};
        off += apport[j];
    }
    
    if (!is_missing_or_null(instance, ip_field)) {
        // Validate that each initial point is a member of its variable's abscissa set
        const json& ip = instance.at(ip_field);
        for (int j = 0; j < count; ++j) {
            bool found = false;
            for (int k = 0; k < slices[j].n_pairs; ++k) {
                if (ip[j] == abscissas[slices[j].offset + k]) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                throw ValidationError(
                    ip_field + "[" + std::to_string(j) + "] = " + ip[j].dump() +
                    " is not a member of the abscissa set for variable " +
                    std::to_string(j)
                );
            }
        }
        json m;
        mark_initial_point_user_provided(m, fields, 5);
        return m;
    }
    
    json initial_point = json::array();
    for (int j = 0; j < count; ++j) {
        int s_off = slices[j].offset;
        int n_p = slices[j].n_pairs;
        
        if (n_p == 1) {
            initial_point.push_back(abscissas[s_off]);
            continue;
        }
        
        // Mean index weighted by counts, round to nearest
        double total = 0, weighted_sum = 0;
        for (int k = 0; k < n_p; ++k) {
            double c = counts[s_off + k].get<double>();
            total += c;
            weighted_sum += k * c;
        }
        double mean_idx = (total > 0) ? weighted_sum / total : 0.0;
        int idx = std::min(static_cast<int>(std::round(mean_idx)), n_p - 1);
        initial_point.push_back(abscissas[s_off + idx]);
    }
    
    json m; m[ip_field] = initial_point; return m;
}

// ============================================================================
// Discrete Set Variables (generic)
// ============================================================================

json default_initial_from_set(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields: [ip_field, elements_per_variable, elements, count]
    const std::string& ip_field = fields[0];
    const std::string& epv_field = fields[1];
    const std::string& elem_field = fields[2];
    const std::string& count_field = fields[3];
    
    reject_initial_point_flag_input(instance, fields, 4, context);

    int count = instance.at(count_field).get<int>();
    
    const auto& elements = instance.at(elem_field);
    auto apport = compute_apportionment(count, instance, epv_field, elements.size());
    
    if (!is_missing_or_null(instance, ip_field)) {
        // Validate that each initial point is a member of its variable's element set
        const json& ip = instance.at(ip_field);
        int offset = 0;
        for (int j = 0; j < count; ++j) {
            int n_e = apport[j];
            bool found = false;
            for (int k = 0; k < n_e; ++k) {
                if (ip[j] == elements[offset + k]) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                throw ValidationError(
                    ip_field + "[" + std::to_string(j) + "] = " + ip[j].dump() +
                    " is not a member of the element set for variable " +
                    std::to_string(j)
                );
            }
            offset += n_e;
        }
        json m;
        mark_initial_point_user_provided(m, fields, 4);
        return m;
    }
    
    // Default: middle element per variable
    json initial = json::array();
    int offset = 0;
    for (int j = 0; j < count; ++j) {
        int n_e = apport[j];
        if (n_e <= 1) {
            initial.push_back(elements[offset]);
        } else {
            int mid_idx = mid_or_next_lower_index(n_e);
            initial.push_back(elements[offset + mid_idx]);
        }
        offset += n_e;
    }
    
    json m; m[ip_field] = initial; return m;
}

json check_set_elements_ordering(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields: [elements_per_variable, elements, count]
    const std::string& epv_field = fields[0];
    const std::string& elem_field = fields[1];
    const std::string& count_field = fields[2];
    
    if (is_missing_or_null(instance, count_field))
        return json::object();
    int count = instance.at(count_field).get<int>();
    if (count <= 0)
        return json::object();
    
    if (is_missing_or_null(instance, elem_field))
        return json::object();
    
    const auto& elements = instance.at(elem_field);
    if (!elements.is_array() || elements.empty())
        return json::object();
    
    auto apport = compute_apportionment(count, instance, epv_field, elements.size());
    
    int offset = 0;
    for (int j = 0; j < count; ++j) {
        int n_e = apport[j];
        for (int k = 1; k < n_e; ++k) {
            const auto& prev = elements[offset + k - 1];
            const auto& curr = elements[offset + k];
            // JSON operator< handles int, float, and string comparison
            if (!(prev < curr)) {
                std::string prev_str = prev.dump();
                std::string curr_str = curr.dump();
                throw ValidationError(
                    "elements for variable " + std::to_string(j) +
                    " are not strictly increasing: elements[" +
                    std::to_string(offset + k - 1) + "] = " + prev_str +
                    " >= elements[" + std::to_string(offset + k) + "] = " + curr_str
                );
            }
        }
        offset += n_e;
    }
    
    return json::object();
}

json default_set_probabilities(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields: [set_probabilities, elements_per_variable, elements, count]
    const std::string& prob_field = fields[0];
    const std::string& epv_field = fields[1];
    const std::string& elem_field = fields[2];
    const std::string& count_field = fields[3];
    
    if (is_missing_or_null(instance, count_field))
        return json::object();
    int count = instance.at(count_field).get<int>();
    if (count <= 0)
        return json::object();
    
    if (is_missing_or_null(instance, elem_field))
        return json::object();
    
    const auto& elements = instance.at(elem_field);
    auto apport = compute_apportionment(count, instance, epv_field, elements.size());
    
    if (is_missing_or_null(instance, prob_field)) {
        // Generate uniform probabilities: 1/k per element
        json probs = json::array();
        for (int j = 0; j < count; ++j) {
            int n_e = apport[j];
            double p = (n_e > 0) ? 1.0 / n_e : 0.0;
            for (int k = 0; k < n_e; ++k) {
                probs.push_back(p);
            }
        }
        json m;
        m[prob_field] = probs;
        return m;
    }
    
    // Normalize each variable's subset to sum to 1
    json probs = instance.at(prob_field);
    int offset = 0;
    bool changed = false;
    for (int j = 0; j < count; ++j) {
        int n_e = apport[j];
        double sum = 0.0;
        for (int k = 0; k < n_e; ++k) {
            sum += probs[offset + k].get<double>();
        }
        if (sum != 1.0 && sum > 0.0) {
            for (int k = 0; k < n_e; ++k) {
                probs[offset + k] = probs[offset + k].get<double>() / sum;
            }
            changed = true;
        }
        offset += n_e;
    }
    
    if (changed) {
        json m;
        m[prob_field] = probs;
        return m;
    }
    return json::object();
}

json uncertain_correlation_matrix_size(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    const std::string matrix_field =
        (!fields.empty()) ? fields[0] : "uncertain_correlation_matrix";

    if (is_missing_or_null(instance, matrix_field)) {
        return json::object();
    }

    const auto& matrix = instance.at(matrix_field);
    if (!matrix.is_array()) {
        return json::object();
    }

    static const std::array<const char*, 18> aleatory_blocks = {{
        "normal_uncertain",
        "lognormal_uncertain",
        "uniform_uncertain",
        "loguniform_uncertain",
        "triangular_uncertain",
        "exponential_uncertain",
        "beta_uncertain",
        "gamma_uncertain",
        "gumbel_uncertain",
        "frechet_uncertain",
        "weibull_uncertain",
        "histogram_bin_uncertain",
        "poisson_uncertain",
        "binomial_uncertain",
        "negative_binomial_uncertain",
        "geometric_uncertain",
        "hypergeometric_uncertain",
        "histogram_point_uncertain"
    }};

    std::size_t n = 0;
    for (const char* block_name : aleatory_blocks) {
        if (!instance.contains(block_name) || instance.at(block_name).is_null()) {
            continue;
        }
        const auto& block = instance.at(block_name);
        if (!block.is_object()) {
            continue;
        }
        if (block.contains("count") && !block.at("count").is_null()) {
            n += static_cast<std::size_t>(block.at("count").get<int>());
            continue;
        }

        if (std::string_view(block_name) == "histogram_point_uncertain") {
            static const std::array<const char*, 3> histogram_point_subtypes = {{
                "integer",
                "string",
                "real"
            }};
            for (const char* subtype_name : histogram_point_subtypes) {
                if (!block.contains(subtype_name) || block.at(subtype_name).is_null()) {
                    continue;
                }
                const auto& subtype_block = block.at(subtype_name);
                if (!subtype_block.is_object() ||
                    !subtype_block.contains("count") ||
                    subtype_block.at("count").is_null()) {
                    continue;
                }
                n += static_cast<std::size_t>(subtype_block.at("count").get<int>());
            }
        }
    }

    const std::size_t expected = n * n;
    const std::size_t actual = matrix.size();
    if (actual != expected) {
        throw ValidationError(
            "uncertain_correlation_matrix length must be " + std::to_string(expected) +
            " (n^2 with n=" + std::to_string(n) + "), but got " + std::to_string(actual)
        );
    }

    for (std::size_t i = 0; i < actual; ++i) {
        const double value = matrix[i].get<double>();
        if (value < -1.0 || value > 1.0) {
            throw ValidationError(
                "uncertain_correlation_matrix[" + std::to_string(i) + "] = " +
                std::to_string(value) + " is outside [-1, 1]"
            );
        }
    }

    return json::object();
}

}  // namespace dakota::validation
