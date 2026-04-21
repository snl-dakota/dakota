// dakota_computed_fields.cpp
// Implementation of computed field functions

#include <dakota/computed_fields.hpp>
#include <cmath>
#include <algorithm>

namespace dakota::computed_fields {

// ============================================================================
// ComputedFieldRegistry Implementation
// ============================================================================

ComputedFieldRegistry::ComputedFieldRegistry() {
    // Register continuous aleatory uncertain computed fields
    register_function("exponential_lower_bounds", exponential_lower_bounds);
    register_function("exponential_upper_bounds", exponential_upper_bounds);
    register_function("gamma_lower_bounds", gamma_lower_bounds);
    register_function("gamma_upper_bounds", gamma_upper_bounds);
    register_function("gumbel_lower_bounds", gumbel_lower_bounds);
    register_function("gumbel_upper_bounds", gumbel_upper_bounds);
    register_function("frechet_lower_bounds", frechet_lower_bounds);
    register_function("frechet_upper_bounds", frechet_upper_bounds);
    register_function("weibull_lower_bounds", weibull_lower_bounds);
    register_function("weibull_upper_bounds", weibull_upper_bounds);

    // Register discrete aleatory uncertain computed fields
    register_function("poisson_lower_bounds", poisson_lower_bounds);
    register_function("poisson_upper_bounds", poisson_upper_bounds);
    register_function("binomial_lower_bounds", binomial_lower_bounds);
    register_function("binomial_upper_bounds", binomial_upper_bounds);
    register_function("negative_binomial_lower_bounds", negative_binomial_lower_bounds);
    register_function("negative_binomial_upper_bounds", negative_binomial_upper_bounds);
    register_function("geometric_lower_bounds", geometric_lower_bounds);
    register_function("geometric_upper_bounds", geometric_upper_bounds);
    register_function("hypergeometric_lower_bounds", hypergeometric_lower_bounds);
    register_function("hypergeometric_upper_bounds", hypergeometric_upper_bounds);

    // Register histogram bin computed fields
    register_function("histogram_bin_lower_bounds", histogram_bin_lower_bounds);
    register_function("histogram_bin_upper_bounds", histogram_bin_upper_bounds);

    // Register histogram point computed fields
    register_function("histogram_point_int_lower_bounds", histogram_point_int_lower_bounds);
    register_function("histogram_point_int_upper_bounds", histogram_point_int_upper_bounds);
    register_function("histogram_point_str_lower_bounds", histogram_point_str_lower_bounds);
    register_function("histogram_point_str_upper_bounds", histogram_point_str_upper_bounds);
    register_function("histogram_point_real_lower_bounds", histogram_point_real_lower_bounds);
    register_function("histogram_point_real_upper_bounds", histogram_point_real_upper_bounds);

    // Register interval computed fields
    register_function("continuous_interval_inferred_lower_bounds", continuous_interval_inferred_lower_bounds);
    register_function("continuous_interval_inferred_upper_bounds", continuous_interval_inferred_upper_bounds);
    register_function("discrete_interval_inferred_lower_bounds", discrete_interval_inferred_lower_bounds);
    register_function("discrete_interval_inferred_upper_bounds", discrete_interval_inferred_upper_bounds);

    // Register discrete set computed fields
    register_function("discrete_set_int_lower_bounds", discrete_set_int_lower_bounds);
    register_function("discrete_set_int_upper_bounds", discrete_set_int_upper_bounds);
    register_function("discrete_set_str_lower_bounds", discrete_set_str_lower_bounds);
    register_function("discrete_set_str_upper_bounds", discrete_set_str_upper_bounds);
    register_function("discrete_set_real_lower_bounds", discrete_set_real_lower_bounds);
    register_function("discrete_set_real_upper_bounds", discrete_set_real_upper_bounds);
}

ComputedFieldRegistry& ComputedFieldRegistry::instance() {
    static ComputedFieldRegistry registry;
    return registry;
}

void ComputedFieldRegistry::register_function(const std::string& name, ComputedFieldFn fn) {
    functions_[name] = std::move(fn);
}

bool ComputedFieldRegistry::has_function(const std::string& name) const {
    return functions_.find(name) != functions_.end();
}

ComputedFieldFn ComputedFieldRegistry::get_function(const std::string& name) const {
    auto it = functions_.find(name);
    if (it == functions_.end()) {
        throw std::runtime_error("Unknown computed field function: " + name);
    }
    return it->second;
}

json ComputedFieldRegistry::compute(const json& instance, const std::string& function_name) const {
    auto fn = get_function(function_name);
    return fn(instance);
}

// ============================================================================
// Exponential Computed Fields
// ============================================================================

json exponential_lower_bounds(const json& instance) {
    int count = instance.at("count").get<int>();
    json result = json::array();
    for (int j = 0; j < count; ++j)
        result.push_back(0.0);
    return result;
}

json exponential_upper_bounds(const json& instance) {
    const auto& betas = instance.at("betas");
    json result = json::array();
    for (const auto& b : betas) {
        auto [mean, stdev] = exponential_moments(b.get<double>());
        result.push_back(mean + 3.0 * stdev);
    }
    return result;
}

// ============================================================================
// Gamma Computed Fields
// ============================================================================

json gamma_lower_bounds(const json& instance) {
    int count = instance.at("count").get<int>();
    json result = json::array();
    for (int j = 0; j < count; ++j)
        result.push_back(0.0);
    return result;
}

json gamma_upper_bounds(const json& instance) {
    const auto& alphas = instance.at("alphas");
    const auto& betas = instance.at("betas");
    json result = json::array();
    for (size_t j = 0; j < alphas.size(); ++j) {
        auto [mean, stdev] = gamma_moments(alphas[j].get<double>(),
                                            betas[j].get<double>());
        result.push_back(mean + 3.0 * stdev);
    }
    return result;
}

// ============================================================================
// Gumbel Computed Fields
// ============================================================================

json gumbel_lower_bounds(const json& instance) {
    const auto& alphas = instance.at("alphas");
    const auto& betas = instance.at("betas");
    json result = json::array();
    for (size_t j = 0; j < alphas.size(); ++j) {
        auto [mean, stdev] = gumbel_moments(alphas[j].get<double>(),
                                             betas[j].get<double>());
        result.push_back(mean - 3.0 * stdev);
    }
    return result;
}

json gumbel_upper_bounds(const json& instance) {
    const auto& alphas = instance.at("alphas");
    const auto& betas = instance.at("betas");
    json result = json::array();
    for (size_t j = 0; j < alphas.size(); ++j) {
        auto [mean, stdev] = gumbel_moments(alphas[j].get<double>(),
                                             betas[j].get<double>());
        result.push_back(mean + 3.0 * stdev);
    }
    return result;
}

// ============================================================================
// Frechet Computed Fields
// ============================================================================

json frechet_lower_bounds(const json& instance) {
    int count = instance.at("count").get<int>();
    json result = json::array();
    for (int j = 0; j < count; ++j)
        result.push_back(0.0);
    return result;
}

json frechet_upper_bounds(const json& instance) {
    const auto& alphas = instance.at("alphas");
    const auto& betas = instance.at("betas");
    json result = json::array();
    for (size_t j = 0; j < alphas.size(); ++j) {
        auto [mean, stdev] = frechet_moments(alphas[j].get<double>(),
                                              betas[j].get<double>());
        result.push_back(mean + 3.0 * stdev);
    }
    return result;
}

// ============================================================================
// Weibull Computed Fields
// ============================================================================

json weibull_lower_bounds(const json& instance) {
    int count = instance.at("count").get<int>();
    json result = json::array();
    for (int j = 0; j < count; ++j)
        result.push_back(0.0);
    return result;
}

json weibull_upper_bounds(const json& instance) {
    const auto& alphas = instance.at("alphas");
    const auto& betas = instance.at("betas");
    json result = json::array();
    for (size_t j = 0; j < alphas.size(); ++j) {
        auto [mean, stdev] = weibull_moments(alphas[j].get<double>(),
                                              betas[j].get<double>());
        result.push_back(mean + 3.0 * stdev);
    }
    return result;
}

// ============================================================================
// Poisson Computed Fields
// ============================================================================

json poisson_lower_bounds(const json& instance) {
    int count = instance.at("count").get<int>();
    return json::array_t(count, 0);
}

json poisson_upper_bounds(const json& instance) {
    const auto& lambdas = instance.at("lambdas");
    json result = json::array();
    
    for (const auto& lam : lambdas) {
        auto [mean, stdev] = poisson_moments(lam.get<double>());
        result.push_back(static_cast<int>(std::ceil(mean + 3.0 * stdev)));
    }
    
    return result;
}

// ============================================================================
// Binomial Computed Fields
// ============================================================================

json binomial_lower_bounds(const json& instance) {
    int count = instance.at("count").get<int>();
    return json::array_t(count, 0);
}

json binomial_upper_bounds(const json& instance) {
    // Upper bound = num_trials (can't have more successes than trials)
    return instance.at("num_trials");
}

// ============================================================================
// Negative Binomial Computed Fields
// ============================================================================

json negative_binomial_lower_bounds(const json& instance) {
    // Lower bound = num_trials (minimum successes needed)
    return instance.at("num_trials");
}

json negative_binomial_upper_bounds(const json& instance) {
    const auto& probs = instance.at("probability_per_trial");
    const auto& trials = instance.at("num_trials");
    json result = json::array();
    
    for (size_t j = 0; j < probs.size(); ++j) {
        int r = trials[j].get<int>();
        double p = probs[j].get<double>();
        auto [mean, stdev] = negative_binomial_moments(r, p);
        result.push_back(static_cast<int>(std::ceil(mean + 3.0 * stdev)));
    }
    
    return result;
}

// ============================================================================
// Geometric Computed Fields
// ============================================================================

json geometric_lower_bounds(const json& instance) {
    int count = instance.at("count").get<int>();
    return json::array_t(count, 0);
}

json geometric_upper_bounds(const json& instance) {
    const auto& probs = instance.at("probability_per_trial");
    json result = json::array();
    
    for (const auto& prob : probs) {
        auto [mean, stdev] = geometric_moments(prob.get<double>());
        result.push_back(static_cast<int>(std::ceil(mean + 3.0 * stdev)));
    }
    
    return result;
}

// ============================================================================
// Hypergeometric Computed Fields
// ============================================================================

json hypergeometric_lower_bounds(const json& instance) {
    int count = instance.at("count").get<int>();
    return json::array_t(count, 0);
}

json hypergeometric_upper_bounds(const json& instance) {
    // Upper bound = min(num_drawn, selected_population)
    const auto& num_drawn = instance.at("num_drawn");
    const auto& selected_pop = instance.at("selected_population");
    json result = json::array();
    
    for (size_t j = 0; j < num_drawn.size(); ++j) {
        int drawn = num_drawn[j].get<int>();
        int selected = selected_pop[j].get<int>();
        result.push_back(std::min(drawn, selected));
    }
    
    return result;
}

// ============================================================================
// Histogram Bin Computed Fields
// ============================================================================

json histogram_bin_lower_bounds(const json& instance) {
    auto sizes = get_apportionment(instance, "pairs_per_variable", "abscissas");
    const auto& abscissas = instance.at("abscissas");
    json result = json::array();
    size_t offset = 0;
    for (int n : sizes) {
        result.push_back(abscissas[offset].get<double>());
        offset += n;
    }
    return result;
}

json histogram_bin_upper_bounds(const json& instance) {
    auto sizes = get_apportionment(instance, "pairs_per_variable", "abscissas");
    const auto& abscissas = instance.at("abscissas");
    json result = json::array();
    size_t offset = 0;
    for (int n : sizes) {
        result.push_back(abscissas[offset + n - 1].get<double>());
        offset += n;
    }
    return result;
}

// ============================================================================
// Histogram Point Computed Fields
// ============================================================================

// Generic min/max over partitioned flat list
template <typename T>
static json partitioned_min(const json& instance, const char* per_var_key,
                            const char* flat_key) {
    auto sizes = get_apportionment(instance, per_var_key, flat_key);
    const auto& values = instance.at(flat_key);
    json result = json::array();
    size_t offset = 0;
    for (int n : sizes) {
        T mn = values[offset].get<T>();
        for (int k = 1; k < n; ++k)
            mn = std::min(mn, values[offset + k].get<T>());
        result.push_back(mn);
        offset += n;
    }
    return result;
}

template <typename T>
static json partitioned_max(const json& instance, const char* per_var_key,
                            const char* flat_key) {
    auto sizes = get_apportionment(instance, per_var_key, flat_key);
    const auto& values = instance.at(flat_key);
    json result = json::array();
    size_t offset = 0;
    for (int n : sizes) {
        T mx = values[offset].get<T>();
        for (int k = 1; k < n; ++k)
            mx = std::max(mx, values[offset + k].get<T>());
        result.push_back(mx);
        offset += n;
    }
    return result;
}

json histogram_point_int_lower_bounds(const json& instance) {
    return partitioned_min<int>(instance, "pairs_per_variable", "abscissas");
}
json histogram_point_int_upper_bounds(const json& instance) {
    return partitioned_max<int>(instance, "pairs_per_variable", "abscissas");
}
json histogram_point_str_lower_bounds(const json& instance) {
    return partitioned_min<std::string>(instance, "pairs_per_variable", "abscissas");
}
json histogram_point_str_upper_bounds(const json& instance) {
    return partitioned_max<std::string>(instance, "pairs_per_variable", "abscissas");
}
json histogram_point_real_lower_bounds(const json& instance) {
    return partitioned_min<double>(instance, "pairs_per_variable", "abscissas");
}
json histogram_point_real_upper_bounds(const json& instance) {
    return partitioned_max<double>(instance, "pairs_per_variable", "abscissas");
}

// ============================================================================
// Interval Computed Fields
// ============================================================================

json continuous_interval_inferred_lower_bounds(const json& instance) {
    auto sizes = get_apportionment(instance, "num_intervals", "lower_bounds");
    const auto& lb = instance.at("lower_bounds");
    json result = json::array();
    size_t offset = 0;
    for (int n : sizes) {
        double mn = lb[offset].get<double>();
        for (int k = 1; k < n; ++k)
            mn = std::min(mn, lb[offset + k].get<double>());
        result.push_back(mn);
        offset += n;
    }
    return result;
}

json continuous_interval_inferred_upper_bounds(const json& instance) {
    auto sizes = get_apportionment(instance, "num_intervals", "upper_bounds");
    const auto& ub = instance.at("upper_bounds");
    json result = json::array();
    size_t offset = 0;
    for (int n : sizes) {
        double mx = ub[offset].get<double>();
        for (int k = 1; k < n; ++k)
            mx = std::max(mx, ub[offset + k].get<double>());
        result.push_back(mx);
        offset += n;
    }
    return result;
}

json discrete_interval_inferred_lower_bounds(const json& instance) {
    auto sizes = get_apportionment(instance, "num_intervals", "lower_bounds");
    const auto& lb = instance.at("lower_bounds");
    json result = json::array();
    size_t offset = 0;
    for (int n : sizes) {
        int mn = lb[offset].get<int>();
        for (int k = 1; k < n; ++k)
            mn = std::min(mn, lb[offset + k].get<int>());
        result.push_back(mn);
        offset += n;
    }
    return result;
}

json discrete_interval_inferred_upper_bounds(const json& instance) {
    auto sizes = get_apportionment(instance, "num_intervals", "upper_bounds");
    const auto& ub = instance.at("upper_bounds");
    json result = json::array();
    size_t offset = 0;
    for (int n : sizes) {
        int mx = ub[offset].get<int>();
        for (int k = 1; k < n; ++k)
            mx = std::max(mx, ub[offset + k].get<int>());
        result.push_back(mx);
        offset += n;
    }
    return result;
}

// ============================================================================
// Discrete Uncertain Set Computed Fields
// ============================================================================

json discrete_set_int_lower_bounds(const json& instance) {
    return partitioned_min<int>(instance, "elements_per_variable", "elements");
}
json discrete_set_int_upper_bounds(const json& instance) {
    return partitioned_max<int>(instance, "elements_per_variable", "elements");
}
json discrete_set_str_lower_bounds(const json& instance) {
    return partitioned_min<std::string>(instance, "elements_per_variable", "elements");
}
json discrete_set_str_upper_bounds(const json& instance) {
    return partitioned_max<std::string>(instance, "elements_per_variable", "elements");
}
json discrete_set_real_lower_bounds(const json& instance) {
    return partitioned_min<double>(instance, "elements_per_variable", "elements");
}
json discrete_set_real_upper_bounds(const json& instance) {
    return partitioned_max<double>(instance, "elements_per_variable", "elements");
}

}  // namespace dakota::computed_fields
