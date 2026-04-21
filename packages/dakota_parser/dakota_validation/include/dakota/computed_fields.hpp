// dakota/computed_fields.hpp
// Computed field functions for Dakota variable types
// Used by both Python (via pybind11) and C++ DSL reader

#ifndef DAKOTA_COMPUTED_FIELDS_HPP
#define DAKOTA_COMPUTED_FIELDS_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <functional>
#include <unordered_map>
#include <stdexcept>

namespace dakota::computed_fields {

using json = nlohmann::json;

// ============================================================================
// Standard Computed Field Signature
// ============================================================================

/// Standard signature for all computed field functions.
/// 
/// @param instance  The full model instance as JSON
/// @return JSON value for the computed field
using ComputedFieldFn = std::function<json(const json& instance)>;

// ============================================================================
// Computed Field Registry
// ============================================================================

/// Registry for looking up computed field functions by name.
/// 
/// The DSL reader uses this to compute derived fields based on JSON schema's
/// x-computed-field entries.
class ComputedFieldRegistry {
public:
    /// Get the singleton instance
    static ComputedFieldRegistry& instance();
    
    /// Register a computed field function by name
    void register_function(const std::string& name, ComputedFieldFn fn);
    
    /// Check if a function is registered
    bool has_function(const std::string& name) const;
    
    /// Get a function by name (throws if not found)
    ComputedFieldFn get_function(const std::string& name) const;
    
    /// Compute a field by name
    /// 
    /// @param instance      The model instance as JSON
    /// @param function_name The function name (from x-computed-field)
    /// @return JSON value for the computed field
    /// @throws std::runtime_error if function not found
    json compute(const json& instance, const std::string& function_name) const;

private:
    ComputedFieldRegistry();
    std::unordered_map<std::string, ComputedFieldFn> functions_;
};

// ============================================================================
// Distribution Moment Functions (shared with validators)
// ============================================================================

/// Poisson moments: mean = lambda, std_dev = sqrt(lambda)
inline std::pair<double, double> poisson_moments(double lambda) {
    return {lambda, std::sqrt(lambda)};
}

/// Binomial moments: mean = n*p, std_dev = sqrt(n*p*(1-p))
inline std::pair<double, double> binomial_moments(int n, double p) {
    double mean = n * p;
    double stdev = std::sqrt(n * p * (1.0 - p));
    return {mean, stdev};
}

/// Negative binomial moments (failures before r successes)
inline std::pair<double, double> negative_binomial_moments(int r, double p) {
    double mean = r * (1.0 - p) / p;
    double var = r * (1.0 - p) / (p * p);
    return {mean, std::sqrt(var)};
}

/// Geometric moments (failures before first success)
inline std::pair<double, double> geometric_moments(double p) {
    double mean = (1.0 - p) / p;
    double var = (1.0 - p) / (p * p);
    return {mean, std::sqrt(var)};
}

/// Exponential moments: mean = beta, std_dev = beta
inline std::pair<double, double> exponential_moments(double beta) {
    return {beta, beta};
}

/// Gamma moments: mean = alpha*beta, std_dev = sqrt(alpha)*beta
inline std::pair<double, double> gamma_moments(double alpha, double beta) {
    double mean = alpha * beta;
    double stdev = std::sqrt(alpha) * beta;
    return {mean, stdev};
}

/// Gumbel moments using Pecos/Dakota parameterization:
/// mean = beta + EulerGamma/alpha, std_dev = pi/(sqrt(6)*alpha)
inline std::pair<double, double> gumbel_moments(double alpha, double beta) {
    constexpr double euler_gamma = 0.5772156649015329;
    double mean = beta + euler_gamma / alpha;
    double stdev = M_PI / (std::sqrt(6.0) * alpha);
    return {mean, stdev};
}

/// Frechet moments: mean = beta*Gamma(1-1/alpha), std_dev from variance formula
inline std::pair<double, double> frechet_moments(double alpha, double beta) {
    double mean = (alpha > 1.0) ? beta * std::tgamma(1.0 - 1.0/alpha) : beta;
    double var = (alpha > 2.0) ?
        beta * beta * (std::tgamma(1.0 - 2.0/alpha) -
                       std::pow(std::tgamma(1.0 - 1.0/alpha), 2.0)) :
        beta * beta;
    return {mean, std::sqrt(var)};
}

/// Weibull moments: mean = beta*Gamma(1+1/alpha), std_dev from variance formula
inline std::pair<double, double> weibull_moments(double alpha, double beta) {
    double g1 = std::tgamma(1.0 + 1.0/alpha);
    double g2 = std::tgamma(1.0 + 2.0/alpha);
    double mean = beta * g1;
    double var = beta * beta * (g2 - g1 * g1);
    return {mean, std::sqrt(var)};
}

// ============================================================================
// Continuous Aleatory Uncertain - Computed Fields
// ============================================================================

// Exponential
json exponential_lower_bounds(const json& instance);
json exponential_upper_bounds(const json& instance);

// Gamma
json gamma_lower_bounds(const json& instance);
json gamma_upper_bounds(const json& instance);

// Gumbel
json gumbel_lower_bounds(const json& instance);
json gumbel_upper_bounds(const json& instance);

// Frechet
json frechet_lower_bounds(const json& instance);
json frechet_upper_bounds(const json& instance);

// Weibull
json weibull_lower_bounds(const json& instance);
json weibull_upper_bounds(const json& instance);

// ============================================================================
// Discrete Aleatory Uncertain - Computed Fields
// ============================================================================

// Poisson
json poisson_lower_bounds(const json& instance);
json poisson_upper_bounds(const json& instance);

// Binomial
json binomial_lower_bounds(const json& instance);
json binomial_upper_bounds(const json& instance);

// Negative Binomial
json negative_binomial_lower_bounds(const json& instance);
json negative_binomial_upper_bounds(const json& instance);

// Geometric
json geometric_lower_bounds(const json& instance);
json geometric_upper_bounds(const json& instance);

// Hypergeometric
json hypergeometric_lower_bounds(const json& instance);
json hypergeometric_upper_bounds(const json& instance);

// ============================================================================
// Helper: compute per-variable apportionment from count + optional per_variable
// ============================================================================

/// Given total flat list size, count of variables, and optional per_variable array,
/// return vector of sizes per variable.
inline std::vector<int> get_apportionment(const json& instance,
    const char* per_var_key, const char* flat_key, const char* count_key = "count")
{
    int count = instance.at(count_key).get<int>();
    std::vector<int> sizes;
    if (instance.contains(per_var_key) && !instance[per_var_key].is_null()) {
        sizes = instance[per_var_key].get<std::vector<int>>();
    } else {
        int total = instance.at(flat_key).size();
        int per = total / count;
        sizes.assign(count, per);
    }
    return sizes;
}

// ============================================================================
// Histogram Bin Uncertain - Computed Fields
// ============================================================================

json histogram_bin_lower_bounds(const json& instance);
json histogram_bin_upper_bounds(const json& instance);

// ============================================================================
// Histogram Point Uncertain - Computed Fields
// ============================================================================

json histogram_point_int_lower_bounds(const json& instance);
json histogram_point_int_upper_bounds(const json& instance);
json histogram_point_str_lower_bounds(const json& instance);
json histogram_point_str_upper_bounds(const json& instance);
json histogram_point_real_lower_bounds(const json& instance);
json histogram_point_real_upper_bounds(const json& instance);

// ============================================================================
// Interval Uncertain - Computed Fields (overall bounds across intervals)
// ============================================================================

json continuous_interval_inferred_lower_bounds(const json& instance);
json continuous_interval_inferred_upper_bounds(const json& instance);
json discrete_interval_inferred_lower_bounds(const json& instance);
json discrete_interval_inferred_upper_bounds(const json& instance);

// ============================================================================
// Discrete Uncertain Set - Computed Fields
// ============================================================================

json discrete_set_int_lower_bounds(const json& instance);
json discrete_set_int_upper_bounds(const json& instance);
json discrete_set_str_lower_bounds(const json& instance);
json discrete_set_str_upper_bounds(const json& instance);
json discrete_set_real_lower_bounds(const json& instance);
json discrete_set_real_upper_bounds(const json& instance);

}  // namespace dakota::computed_fields

#endif // DAKOTA_COMPUTED_FIELDS_HPP
