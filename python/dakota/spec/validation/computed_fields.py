"""Computed field support for Dakota variable models.

This module provides helper classes for using C++-backed computed fields in Pydantic
models. The computed field logic is implemented in C++ to ensure consistency between
Python models and the C++ DSL reader, with pure Python fallbacks when C++ is unavailable.

Usage Pattern
-------------
Use the provided helper classes (e.g., PoissonBounds) in your Pydantic model:

    from pydantic import computed_field
    from dakota.spec.validation.computed_fields import PoissonBounds, computed_field_schema

    class PoissonUncertain(DakotaBaseModel):
        count: int
        lambdas: List[float]
        initial_point: List[int] | None = None

        @computed_field(**computed_field_schema("poisson_lower_bounds"))
        @property
        def lower_bounds(self) -> List[int]:
            return PoissonBounds.lower_bounds(self)

        @computed_field(**computed_field_schema("poisson_upper_bounds"))
        @property
        def upper_bounds(self) -> List[int]:
            return PoissonBounds.upper_bounds(self)

The helper classes (PoissonBounds, BinomialBounds, etc.) automatically use the C++
backend when available and fall back to pure Python otherwise.

Schema Generation
-----------------
Computed fields are emitted in a separate `x-computed-fields` section of the model
schema (not in `properties`, which represents parsed input). The model generator
handles this by converting Python type annotations to JSON Schema:

    {
      "x-computed-fields": {
        "lower_bounds": {
          "type": "array",
          "items": {"type": "integer"},
          "function": "poisson_lower_bounds",
          "description": "Lower bounds (always 0 for Poisson)"
        }
      }
    }

Available Helper Classes
------------------------
- PoissonBounds: lower_bounds (zeros), upper_bounds (ceil(λ + 3√λ))
- BinomialBounds: lower_bounds (zeros), upper_bounds (num_trials)
- NegativeBinomialBounds: lower_bounds (num_trials), upper_bounds (ceil(mean + 3*stdev))
- GeometricBounds: lower_bounds (zeros), upper_bounds (ceil(mean + 3*stdev))
- HypergeometricBounds: lower_bounds (zeros), upper_bounds (min(num_drawn, selected_pop))
"""

from typing import Any, List
import math

# Try to import C++ backend (computed fields are in the same module as validators)
try:
    from . import dakota_validation_cpp as _cpp

    _USE_CPP = True
except ImportError:
    _USE_CPP = False


def computed_field_schema(function_name: str) -> dict:
    """Generate json_schema_extra for a computed field.

    Args:
        function_name: Name of the C++ function in the registry

    Returns:
        Dict suitable for passing to @computed_field(**result)
    """
    return {"json_schema_extra": {"x-computed-field": {"function": function_name}}}


def cpp_computed_field(function_name: str, instance: Any) -> Any:
    """Call a C++ computed field function.

    Args:
        function_name: Name of the function in the registry
        instance: Pydantic model instance

    Returns:
        The computed field value
    """
    if _USE_CPP:
        # Serialize instance to dict for C++ consumption
        instance_dict = instance.model_dump(mode="python", exclude_computed_fields=True)
        return _cpp.compute(instance_dict, function_name)
    else:
        raise RuntimeError(
            f"C++ backend not available for computed field '{function_name}'. "
            "Install dakota_computed_fields_cpp or implement a Python fallback."
        )


# =============================================================================
# Pure Python fallback implementations
# =============================================================================


def _poisson_moments(lam: float) -> tuple[float, float]:
    return lam, math.sqrt(lam)


def _binomial_moments(n: int, p: float) -> tuple[float, float]:
    return n * p, math.sqrt(n * p * (1 - p))


def _negative_binomial_moments(r: int, p: float) -> tuple[float, float]:
    mean = r * (1 - p) / p
    var = r * (1 - p) / (p * p)
    return mean, math.sqrt(var)


def _geometric_moments(p: float) -> tuple[float, float]:
    mean = (1 - p) / p
    var = (1 - p) / (p * p)
    return mean, math.sqrt(var)


def _exponential_moments(beta: float) -> tuple[float, float]:
    return beta, beta


def _gamma_moments(alpha: float, beta: float) -> tuple[float, float]:
    mean = alpha * beta
    stdev = math.sqrt(alpha) * beta
    return mean, stdev


def _gumbel_moments(alpha: float, beta: float) -> tuple[float, float]:
    euler_gamma = 0.5772156649015329
    mean = beta + euler_gamma / alpha
    stdev = math.pi / (math.sqrt(6.0) * alpha)
    return mean, stdev


def _frechet_moments(alpha: float, beta: float) -> tuple[float, float]:
    mean = beta * math.gamma(1.0 - 1.0 / alpha) if alpha > 1.0 else beta
    var = (
        (beta**2 * (math.gamma(1.0 - 2.0 / alpha) - math.gamma(1.0 - 1.0 / alpha) ** 2))
        if alpha > 2.0
        else beta**2
    )
    return mean, math.sqrt(var)


def _weibull_moments(alpha: float, beta: float) -> tuple[float, float]:
    g1 = math.gamma(1.0 + 1.0 / alpha)
    g2 = math.gamma(1.0 + 2.0 / alpha)
    mean = beta * g1
    var = beta**2 * (g2 - g1**2)
    return mean, math.sqrt(var)


class ExponentialBounds:
    """Computed bounds for exponential_uncertain."""

    @staticmethod
    def lower_bounds(instance: Any) -> List[float]:
        if _USE_CPP:
            return _cpp.exponential_lower_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        return [0.0] * instance.count

    @staticmethod
    def upper_bounds(instance: Any) -> List[float]:
        if _USE_CPP:
            return _cpp.exponential_upper_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        result = []
        for b in instance.betas:
            mean, stdev = _exponential_moments(b)
            result.append(mean + 3 * stdev)
        return result


class GammaBounds:
    """Computed bounds for gamma_uncertain."""

    @staticmethod
    def lower_bounds(instance: Any) -> List[float]:
        if _USE_CPP:
            return _cpp.gamma_lower_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        return [0.0] * instance.count

    @staticmethod
    def upper_bounds(instance: Any) -> List[float]:
        if _USE_CPP:
            return _cpp.gamma_upper_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        result = []
        for j in range(instance.count):
            mean, stdev = _gamma_moments(instance.alphas[j], instance.betas[j])
            result.append(mean + 3 * stdev)
        return result


class GumbelBounds:
    """Computed bounds for gumbel_uncertain."""

    @staticmethod
    def lower_bounds(instance: Any) -> List[float]:
        if _USE_CPP:
            return _cpp.gumbel_lower_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        result = []
        for j in range(instance.count):
            mean, stdev = _gumbel_moments(instance.alphas[j], instance.betas[j])
            result.append(mean - 3 * stdev)
        return result

    @staticmethod
    def upper_bounds(instance: Any) -> List[float]:
        if _USE_CPP:
            return _cpp.gumbel_upper_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        result = []
        for j in range(instance.count):
            mean, stdev = _gumbel_moments(instance.alphas[j], instance.betas[j])
            result.append(mean + 3 * stdev)
        return result


class FrechetBounds:
    """Computed bounds for frechet_uncertain."""

    @staticmethod
    def lower_bounds(instance: Any) -> List[float]:
        if _USE_CPP:
            return _cpp.frechet_lower_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        return [0.0] * instance.count

    @staticmethod
    def upper_bounds(instance: Any) -> List[float]:
        if _USE_CPP:
            return _cpp.frechet_upper_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        result = []
        for j in range(instance.count):
            mean, stdev = _frechet_moments(instance.alphas[j], instance.betas[j])
            result.append(mean + 3 * stdev)
        return result


class WeibullBounds:
    """Computed bounds for weibull_uncertain."""

    @staticmethod
    def lower_bounds(instance: Any) -> List[float]:
        if _USE_CPP:
            return _cpp.weibull_lower_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        return [0.0] * instance.count

    @staticmethod
    def upper_bounds(instance: Any) -> List[float]:
        if _USE_CPP:
            return _cpp.weibull_upper_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        result = []
        for j in range(instance.count):
            mean, stdev = _weibull_moments(instance.alphas[j], instance.betas[j])
            result.append(mean + 3 * stdev)
        return result


class PoissonBounds:
    """Computed bounds for poisson_uncertain."""

    @staticmethod
    def lower_bounds(instance: Any) -> List[int]:
        if _USE_CPP:
            return _cpp.poisson_lower_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        return [0] * instance.count

    @staticmethod
    def upper_bounds(instance: Any) -> List[int]:
        if _USE_CPP:
            return _cpp.poisson_upper_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        result = []
        for lam in instance.lambdas:
            mean, stdev = _poisson_moments(lam)
            result.append(int(math.ceil(mean + 3 * stdev)))
        return result


class BinomialBounds:
    """Computed bounds for binomial_uncertain."""

    @staticmethod
    def lower_bounds(instance: Any) -> List[int]:
        if _USE_CPP:
            return _cpp.binomial_lower_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        return [0] * instance.count

    @staticmethod
    def upper_bounds(instance: Any) -> List[int]:
        if _USE_CPP:
            return _cpp.binomial_upper_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        return list(instance.num_trials)


class NegativeBinomialBounds:
    """Computed bounds for negative_binomial_uncertain."""

    @staticmethod
    def lower_bounds(instance: Any) -> List[int]:
        if _USE_CPP:
            return _cpp.negative_binomial_lower_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        return list(instance.num_trials)

    @staticmethod
    def upper_bounds(instance: Any) -> List[int]:
        if _USE_CPP:
            return _cpp.negative_binomial_upper_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        result = []
        for j in range(instance.count):
            r = instance.num_trials[j]
            p = instance.probability_per_trial[j]
            mean, stdev = _negative_binomial_moments(r, p)
            result.append(int(math.ceil(mean + 3 * stdev)))
        return result


class GeometricBounds:
    """Computed bounds for geometric_uncertain."""

    @staticmethod
    def lower_bounds(instance: Any) -> List[int]:
        if _USE_CPP:
            return _cpp.geometric_lower_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        return [0] * instance.count

    @staticmethod
    def upper_bounds(instance: Any) -> List[int]:
        if _USE_CPP:
            return _cpp.geometric_upper_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        result = []
        for p in instance.probability_per_trial:
            mean, stdev = _geometric_moments(p)
            result.append(int(math.ceil(mean + 3 * stdev)))
        return result


class HypergeometricBounds:
    """Computed bounds for hypergeometric_uncertain."""

    @staticmethod
    def lower_bounds(instance: Any) -> List[int]:
        if _USE_CPP:
            return _cpp.hypergeometric_lower_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        return [0] * instance.count

    @staticmethod
    def upper_bounds(instance: Any) -> List[int]:
        if _USE_CPP:
            return _cpp.hypergeometric_upper_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        result = []
        for j in range(instance.count):
            drawn = instance.num_drawn[j]
            selected = instance.selected_population[j]
            result.append(min(drawn, selected))
        return result


def _get_apportionment(instance, per_var_attr: str, flat_attr: str) -> List[int]:
    """Get per-variable sizes from optional per_variable field or equal division."""
    per_var = getattr(instance, per_var_attr, None)
    if per_var is not None:
        return list(per_var)
    total = len(getattr(instance, flat_attr))
    per = total // instance.count
    return [per] * instance.count


def _partitioned_minmax(instance, per_var_attr: str, flat_attr: str, use_min: bool):
    """Compute min or max per variable partition."""
    sizes = _get_apportionment(instance, per_var_attr, flat_attr)
    values = getattr(instance, flat_attr)
    result = []
    offset = 0
    fn = min if use_min else max
    for n in sizes:
        result.append(fn(values[offset : offset + n]))
        offset += n
    return result


class HistogramBinBounds:
    """Computed bounds for histogram_bin_uncertain."""

    @staticmethod
    def lower_bounds(instance: Any) -> List[float]:
        if _USE_CPP:
            return _cpp.histogram_bin_lower_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        sizes = _get_apportionment(instance, "pairs_per_variable", "abscissas")
        abscissas = instance.abscissas
        result = []
        offset = 0
        for n in sizes:
            result.append(float(abscissas[offset]))
            offset += n
        return result

    @staticmethod
    def upper_bounds(instance: Any) -> List[float]:
        if _USE_CPP:
            return _cpp.histogram_bin_upper_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        sizes = _get_apportionment(instance, "pairs_per_variable", "abscissas")
        abscissas = instance.abscissas
        result = []
        offset = 0
        for n in sizes:
            result.append(float(abscissas[offset + n - 1]))
            offset += n
        return result


class HistogramPointIntBounds:
    """Computed bounds for histogram_point_uncertain integer."""

    @staticmethod
    def lower_bounds(instance: Any) -> List[int]:
        if _USE_CPP:
            return _cpp.histogram_point_int_lower_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        return _partitioned_minmax(instance, "pairs_per_variable", "abscissas", True)

    @staticmethod
    def upper_bounds(instance: Any) -> List[int]:
        if _USE_CPP:
            return _cpp.histogram_point_int_upper_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        return _partitioned_minmax(instance, "pairs_per_variable", "abscissas", False)


class HistogramPointStrBounds:
    """Computed bounds for histogram_point_uncertain string."""

    @staticmethod
    def lower_bounds(instance: Any) -> List[str]:
        if _USE_CPP:
            return _cpp.histogram_point_str_lower_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        return _partitioned_minmax(instance, "pairs_per_variable", "abscissas", True)

    @staticmethod
    def upper_bounds(instance: Any) -> List[str]:
        if _USE_CPP:
            return _cpp.histogram_point_str_upper_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        return _partitioned_minmax(instance, "pairs_per_variable", "abscissas", False)


class HistogramPointRealBounds:
    """Computed bounds for histogram_point_uncertain real."""

    @staticmethod
    def lower_bounds(instance: Any) -> List[float]:
        if _USE_CPP:
            return _cpp.histogram_point_real_lower_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        return _partitioned_minmax(instance, "pairs_per_variable", "abscissas", True)

    @staticmethod
    def upper_bounds(instance: Any) -> List[float]:
        if _USE_CPP:
            return _cpp.histogram_point_real_upper_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        return _partitioned_minmax(instance, "pairs_per_variable", "abscissas", False)


class ContinuousIntervalBounds:
    """Computed overall bounds for continuous_interval_uncertain."""

    @staticmethod
    def inferred_lower_bounds(instance: Any) -> List[float]:
        if _USE_CPP:
            return _cpp.continuous_interval_inferred_lower_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        sizes = _get_apportionment(instance, "num_intervals", "lower_bounds")
        lb = instance.lower_bounds
        result = []
        offset = 0
        for n in sizes:
            result.append(min(lb[offset : offset + n]))
            offset += n
        return result

    @staticmethod
    def inferred_upper_bounds(instance: Any) -> List[float]:
        if _USE_CPP:
            return _cpp.continuous_interval_inferred_upper_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        sizes = _get_apportionment(instance, "num_intervals", "upper_bounds")
        ub = instance.upper_bounds
        result = []
        offset = 0
        for n in sizes:
            result.append(max(ub[offset : offset + n]))
            offset += n
        return result


class DiscreteIntervalBounds:
    """Computed overall bounds for discrete_interval_uncertain."""

    @staticmethod
    def inferred_lower_bounds(instance: Any) -> List[int]:
        if _USE_CPP:
            return _cpp.discrete_interval_inferred_lower_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        sizes = _get_apportionment(instance, "num_intervals", "lower_bounds")
        lb = instance.lower_bounds
        result = []
        offset = 0
        for n in sizes:
            result.append(min(lb[offset : offset + n]))
            offset += n
        return result

    @staticmethod
    def inferred_upper_bounds(instance: Any) -> List[int]:
        if _USE_CPP:
            return _cpp.discrete_interval_inferred_upper_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        sizes = _get_apportionment(instance, "num_intervals", "upper_bounds")
        ub = instance.upper_bounds
        result = []
        offset = 0
        for n in sizes:
            result.append(max(ub[offset : offset + n]))
            offset += n
        return result


class DiscreteSetIntBounds:
    """Computed bounds for discrete_uncertain_set integer."""

    @staticmethod
    def lower_bounds(instance: Any) -> List[int]:
        if _USE_CPP:
            return _cpp.discrete_set_int_lower_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        return _partitioned_minmax(instance, "elements_per_variable", "elements", True)

    @staticmethod
    def upper_bounds(instance: Any) -> List[int]:
        if _USE_CPP:
            return _cpp.discrete_set_int_upper_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        return _partitioned_minmax(instance, "elements_per_variable", "elements", False)


class DiscreteSetStrBounds:
    """Computed bounds for discrete_uncertain_set string."""

    @staticmethod
    def lower_bounds(instance: Any) -> List[str]:
        if _USE_CPP:
            return _cpp.discrete_set_str_lower_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        return _partitioned_minmax(instance, "elements_per_variable", "elements", True)

    @staticmethod
    def upper_bounds(instance: Any) -> List[str]:
        if _USE_CPP:
            return _cpp.discrete_set_str_upper_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        return _partitioned_minmax(instance, "elements_per_variable", "elements", False)


class DiscreteSetRealBounds:
    """Computed bounds for discrete_uncertain_set real."""

    @staticmethod
    def lower_bounds(instance: Any) -> List[float]:
        if _USE_CPP:
            return _cpp.discrete_set_real_lower_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        return _partitioned_minmax(instance, "elements_per_variable", "elements", True)

    @staticmethod
    def upper_bounds(instance: Any) -> List[float]:
        if _USE_CPP:
            return _cpp.discrete_set_real_upper_bounds(
                instance.model_dump(mode="python", exclude_computed_fields=True)
            )
        return _partitioned_minmax(instance, "elements_per_variable", "elements", False)
