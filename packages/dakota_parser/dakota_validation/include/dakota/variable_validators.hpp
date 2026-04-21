// dakota/variable_validators.hpp
// Distribution-specific validation functions for Dakota variable types
// Used by both Python (via pybind11) and C++ DSL reader

#ifndef DAKOTA_VARIABLE_VALIDATORS_HPP
#define DAKOTA_VARIABLE_VALIDATORS_HPP

#include <dakota/validation.hpp>
#include <cmath>

namespace dakota::validation {

// ============================================================================
// Distribution Moment Functions
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
/// mean = r*(1-p)/p, var = r*(1-p)/p^2
inline std::pair<double, double> negative_binomial_moments(int r, double p) {
    double mean = r * (1.0 - p) / p;
    double var = r * (1.0 - p) / (p * p);
    return {mean, std::sqrt(var)};
}

/// Geometric moments (failures before first success)
/// mean = (1-p)/p, var = (1-p)/p^2
inline std::pair<double, double> geometric_moments(double p) {
    double mean = (1.0 - p) / p;
    double var = (1.0 - p) / (p * p);
    return {mean, std::sqrt(var)};
}

/// Hypergeometric mean = n * K / N
inline double hypergeometric_mean(int total_pop, int selected_pop, int num_drawn) {
    return static_cast<double>(num_drawn * selected_pop) / total_pop;
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
    constexpr double euler_mascheroni = 0.5772156649015329;
    double mean = beta + euler_mascheroni / alpha;
    double stdev = M_PI / (std::sqrt(6.0) * alpha);
    return {mean, stdev};
}

/// Frechet moments (shape=alpha, scale=beta)
inline std::pair<double, double> frechet_moments(double alpha, double beta) {
    if (alpha <= 1.0) {
        return {beta, beta};
    } else if (alpha <= 2.0) {
        double mean = beta * std::tgamma(1.0 - 1.0/alpha);
        return {mean, mean};
    } else {
        double g1 = std::tgamma(1.0 - 1.0/alpha);
        double g2 = std::tgamma(1.0 - 2.0/alpha);
        double mean = beta * g1;
        double var = beta * beta * (g2 - g1 * g1);
        return {mean, std::sqrt(var)};
    }
}

/// Weibull moments (shape=alpha, scale=beta)
inline std::pair<double, double> weibull_moments(double alpha, double beta) {
    double g1 = std::tgamma(1.0 + 1.0/alpha);
    double g2 = std::tgamma(1.0 + 2.0/alpha);
    double mean = beta * g1;
    double var = beta * beta * (g2 - g1 * g1);
    return {mean, std::sqrt(var)};
}

/// Beta mean on [lower, upper]: lower + (upper-lower) * alpha/(alpha+beta)
inline double beta_mean(double alpha, double beta, double lower, double upper) {
    double std_mean = alpha / (alpha + beta);
    return lower + (upper - lower) * std_mean;
}

/// Uniform mean
inline double uniform_mean(double lower, double upper) {
    return (lower + upper) / 2.0;
}

/// Loguniform mean: (b - a) / (ln(b) - ln(a))
inline double loguniform_mean(double lower, double upper) {
    if (lower == upper) return lower;
    return (upper - lower) / (std::log(upper) - std::log(lower));
}

/// Triangular mean
inline double triangular_mean(double lower, double mode, double upper) {
    return (lower + mode + upper) / 3.0;
}

/// Lognormal moments from lambda/zeta parameterization.
/// mean = exp(lambda + zeta^2/2), stdev = mean * sqrt(expm1(zeta^2))
inline std::pair<double, double> lognormal_moments_from_params(double lambda, double zeta) {
    double zeta_sq = zeta * zeta;
    double mean = std::exp(lambda + zeta_sq / 2.0);
    double stdev = mean * std::sqrt(std::expm1(zeta_sq));
    return {mean, stdev};
}

/// Lognormal stdev from mean and error factor.
/// zeta = ln(error_factor) / Phi^{-1}(0.95), stdev = mean * sqrt(expm1(zeta^2))
/// Phi^{-1}(0.95) ~= 1.6448536269514729
inline double lognormal_stdev_from_error_factor(double mean, double error_factor) {
    constexpr double inv_cdf_095 = 1.6448536269514729;
    double zeta = std::log(error_factor) / inv_cdf_095;
    return mean * std::sqrt(std::expm1(zeta * zeta));
}

// ============================================================================
// Continuous Aleatory Uncertain Validators
// ============================================================================

/// Default and infer bounds for normal_uncertain.
///
/// If lower_bounds specified: inferred_lower_bounds = lower_bounds.
/// If not: lower_bounds = [-inf]*n, inferred_lower_bounds = [mean - 3*stdev]*n.
/// Same pattern for upper bounds with +3*stdev.
///
/// Fields: [lower_bounds, upper_bounds, inferred_lower_bounds,
///          inferred_upper_bounds, means, std_deviations, count]
/// Literals: []
json normal_uncertain_bounds(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Default and infer bounds for lognormal_uncertain.
///
/// If lower_bounds not specified: lower_bounds = [0]*n.
/// If upper_bounds specified: inferred_upper_bounds = upper_bounds.
/// If not: upper_bounds = [inf]*n, inferred_upper_bounds = [mean + 3*stdev]*n.
///
/// Mean and stdev are extracted from whichever parameterization is active:
///   option_1: means + std_deviations
///   option_2: means + error_factors (stdev derived via error factor)
///   option_3: lambdas + zetas (moments derived from lognormal params)
///
/// Field names are hardcoded — fields/literals parameters are ignored.
json lognormal_uncertain_bounds(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Default initial_point for normal_uncertain with truncation bounds.
/// 
/// Fields: [initial_point, means, std_deviations, lower_bounds, upper_bounds, count]
/// Literals: []
/// 
/// Special handling for truncated normal: uses nudge factor to keep initial
/// point away from bounds.
json normal_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Default initial_point for lognormal_uncertain.
/// 
/// Fields: [initial_point, means, std_deviations, lower_bounds, upper_bounds, count]
/// Literals: []
json lognormal_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Default initial_point for uniform_uncertain.
/// 
/// Fields: [initial_point, lower_bounds, upper_bounds, count]
/// Literals: []
json uniform_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Validate and default initial_point for loguniform_uncertain.
/// 
/// Fields: [initial_point, lower_bounds, upper_bounds, count]
/// Literals: []
/// 
/// @throws ValidationError if bounds are not positive and finite
json loguniform_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Validate and default initial_point for triangular_uncertain.
/// 
/// Fields: [initial_point, modes, lower_bounds, upper_bounds, count]
/// Literals: []
/// 
/// @throws ValidationError if lower > mode or mode > upper
json triangular_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Default initial_point for exponential_uncertain.
/// 
/// Fields: [initial_point, betas, count]
/// Literals: []
json exponential_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Default initial_point for beta_uncertain.
/// 
/// Fields: [initial_point, alphas, betas, lower_bounds, upper_bounds, count]
/// Literals: []
json beta_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Default initial_point for gamma_uncertain.
/// 
/// Fields: [initial_point, alphas, betas, count]
/// Literals: []
json gamma_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Default initial_point for gumbel_uncertain.
/// 
/// Fields: [initial_point, alphas, betas, count]
/// Literals: []
json gumbel_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Default initial_point for frechet_uncertain.
/// 
/// Fields: [initial_point, alphas, betas, count]
/// Literals: []
json frechet_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Default initial_point for weibull_uncertain.
/// 
/// Fields: [initial_point, alphas, betas, count]
/// Literals: []
json weibull_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

// ============================================================================
// Discrete Aleatory Uncertain Validators
// ============================================================================

/// Default initial_point for poisson_uncertain.
/// 
/// Fields: [initial_point, lambdas, count]
/// Literals: []
json poisson_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Default initial_point for binomial_uncertain.
/// 
/// Fields: [initial_point, probability_per_trial, num_trials, count]
/// Literals: []
json binomial_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Default initial_point for negative_binomial_uncertain.
/// 
/// Fields: [initial_point, probability_per_trial, num_trials, count]
/// Literals: []
json negative_binomial_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Default initial_point for geometric_uncertain.
/// 
/// Fields: [initial_point, probability_per_trial, count]
/// Literals: []
json geometric_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Default initial_point for hypergeometric_uncertain.
/// 
/// Fields: [initial_point, total_population, selected_population, num_drawn, count]
/// Literals: []
json hypergeometric_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

}  // namespace dakota::validation

// ============================================================================
// Epistemic Uncertain - Interval Types
// ============================================================================

namespace dakota::validation {

/// Default initial_point for continuous_interval_uncertain.
///
/// Fields: [initial_point, lower_bounds, upper_bounds, num_intervals, count]
/// Literals: []
json continuous_interval_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Default initial_point for discrete_interval_uncertain.
/// Uses truncation toward zero for midpoint (matching C++ semantics).
///
/// Fields: [initial_point, lower_bounds, upper_bounds, num_intervals, count]
/// Literals: []
json discrete_interval_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

// ============================================================================
// Histogram Uncertain
// ============================================================================

/// Default initial_point for histogram_bin_uncertain.
/// Computes probability-weighted mean of bin midpoints, matching
/// Pecos::HistogramBinRandomVariable::central_moments_from_params.
/// Handles both ordinates (density) and counts input formats.
///
/// Fields: [initial_point, pairs_per_variable, abscissas, density, count]
/// Literals: []
json histogram_bin_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Default initial_point for histogram_point_uncertain (int or real).
/// Computes weighted mean from abscissas and counts, snaps to nearest.
///
/// Fields: [initial_point, pairs_per_variable, abscissas, counts, count]
/// Literals: []
json histogram_point_numeric_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Default initial_point for histogram_point_uncertain (string).
/// Uses index-based weighted mean, rounds to nearest index.
///
/// Fields: [initial_point, pairs_per_variable, abscissas, counts, count]
/// Literals: []
json histogram_point_str_uncertain_initial(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

// ============================================================================
// Discrete Set Variables (generic for all set types)
// ============================================================================

/// Default initial value from flat set elements.
/// Uses middle element per variable (matching C++ mid_or_next_lower_index).
/// Validates that user-provided initial values are members of the element set.
/// Works for int, real, and string types via JSON value copying.
///
/// Fields: [ip_field, elements_per_variable, elements, count]
/// Literals: []
json default_initial_from_set(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Validate that elements are strictly increasing within each variable's subset.
///
/// Partitions the flat elements array by variable using elements_per_variable
/// (or even split if absent), then checks strict ordering within each partition.
/// Works for int, real, and string element types via JSON comparison.
///
/// Fields: [elements_per_variable, elements, count]
/// Literals: []
///
/// @throws ValidationError if any variable's elements are not strictly increasing
json check_set_elements_ordering(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Default or normalize set_probabilities for discrete uncertain set variables.
///
/// If set_probabilities is null/missing, generates uniform probabilities:
///   1/k for each element in each variable's subset (where k = subset size).
///
/// If set_probabilities is provided, normalizes each variable's subset
///   to sum to 1.0.
///
/// Fields: [set_probabilities, elements_per_variable, elements, count]
/// Literals: []
///
/// @return Mutations for set_probabilities (always returned since we
///         either generate or normalize)
json default_set_probabilities(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

/// Validate uncertain_correlation_matrix length equals n^2.
///
/// n is the sum of counts for continuous aleatory uncertain variable blocks:
/// normal, lognormal, uniform, loguniform, triangular, exponential, beta,
/// gamma, gumbel, frechet, weibull, and histogram_bin_uncertain.
///
/// Fields: [uncertain_correlation_matrix] (optional; defaults to this name)
/// Literals: []
///
/// @throws ValidationError if matrix length != n^2
json uncertain_correlation_matrix_size(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);

}  // namespace dakota::validation

#endif // DAKOTA_VARIABLE_VALIDATORS_HPP
