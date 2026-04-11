"""Variable-specific validation rules for Dakota models.

This module provides ValidationRule subclasses for Dakota variable types:
- Continuous/discrete design and state variables
- Continuous aleatory uncertain (normal, uniform, lognormal, etc.)
- Discrete aleatory uncertain (poisson, binomial, etc.)

The actual validation logic is implemented in C++ (dakota_validation_cpp module)
and called via pybind11 bindings.

All validators follow the standard signature matching JSON schema export:
    validator(instance, fields, literals, context) -> mutations
"""

from typing import Any
from .base import ValidationRule

# Try to import C++ backend
try:
    from . import dakota_validation_cpp as _cpp

    _USE_CPP = True
except ImportError:
    _USE_CPP = False


def _apply_mutations(instance: Any, mutations: dict) -> None:
    """Apply field mutations to a Pydantic model instance.

    Supports dotted paths for nested field mutations.
    See rules._apply_mutations for full documentation.
    """
    from typing import get_origin, get_args, Union
    from pydantic import BaseModel

    if not mutations:
        return

    for field_path, value in mutations.items():
        components = field_path.split(".")

        # Navigate to the parent of the target field
        current = instance
        path_so_far = []

        for component in components[:-1]:
            path_so_far.append(component)
            model_fields = current.__class__.model_fields

            if component not in model_fields:
                raise RuntimeError(
                    f"Cannot apply mutation to '{field_path}': "
                    f"field '{component}' not found on {current.__class__.__name__}. "
                    f"Valid fields are: {', '.join(model_fields.keys())}"
                )

            next_obj = getattr(current, component)

            if not isinstance(next_obj, BaseModel):
                if next_obj is None:
                    raise RuntimeError(
                        f"Cannot apply mutation to '{field_path}': "
                        f"intermediate field '{'.'.join(path_so_far)}' is None"
                    )
                raise RuntimeError(
                    f"Cannot apply mutation to '{field_path}': "
                    f"intermediate field '{'.'.join(path_so_far)}' is not a model instance"
                )

            current = next_obj

        # Apply mutation to the leaf field
        leaf_field = components[-1]
        model_fields = current.__class__.model_fields

        if leaf_field not in model_fields:
            raise RuntimeError(
                f"Cannot apply mutation to '{field_path}': "
                f"field '{leaf_field}' not found on {current.__class__.__name__}. "
                f"Valid fields are: {', '.join(model_fields.keys())}"
            )

        # Check that the leaf field is NOT a nested model type
        field_info = model_fields[leaf_field]
        annotation = field_info.annotation
        origin = get_origin(annotation)

        if origin is Union:
            types = [t for t in get_args(annotation) if t is not type(None)]
        else:
            types = [annotation]

        for t in types:
            if isinstance(t, type) and issubclass(t, BaseModel) and t is not BaseModel:
                raise RuntimeError(
                    f"Validator returned mutation for nested model field '{field_path}' "
                    f"(type {t.__name__}). Mutations to nested model fields are not supported."
                )

        object.__setattr__(current, leaf_field, value)


def _mark_initial_point_user_provided(instance: Any) -> None:
    """Record that the user explicitly supplied an initial_point value."""
    if "initial_point_user_provided" in instance.__class__.model_fields:
        object.__setattr__(instance, "initial_point_user_provided", True)


def _reject_internal_field_input(instance: Any, field_name: str, context: str) -> None:
    """Reject user input for internal-only fields."""
    if field_name in getattr(instance, "model_fields_set", set()):
        raise ValueError(
            f"For {context}, '{field_name}' is an internal-only field and must not "
            "be provided by the user"
        )


def _variable_model_dump(instance: Any) -> dict:
    """Dump only explicitly provided fields for the C++ variable validators."""
    return instance.model_dump(mode="python", exclude_defaults=True)


# =============================================================================
# Continuous Aleatory Uncertain Validators
# =============================================================================


class NormalUncertainBounds(ValidationRule):
    """Default and infer bounds for normal_uncertain variables.

    If lower_bounds is specified, inferred_lower_bounds = lower_bounds.
    If not, lower_bounds defaults to [-inf]*n and
    inferred_lower_bounds = [mean - 3*stdev]*n.
    Same pattern for upper bounds with mean + 3*stdev.

    Must run BEFORE NormalUncertainInitial (which depends on lower/upper_bounds).

    This rule is specific to normal_uncertain, so the field names are fixed.
    """

    def __init__(self, context: str = "normal_uncertain"):
        super().__init__(
            context=context,
            rule_name="normal_uncertain_bounds",
            fields=[],
            literals=[],
            error_message="For {context}, failed to set default/inferred bounds",
        )

    def __call__(self, instance: Any) -> None:
        _reject_internal_field_input(instance, "inferred_lower_bounds", self.context)
        _reject_internal_field_input(instance, "inferred_upper_bounds", self.context)
        if _USE_CPP:
            instance_dict = _variable_model_dump(instance)
            mutations = _cpp.normal_uncertain_bounds(
                instance_dict, self.fields, self.literals, self.context
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback."""
        count = instance.count
        means = instance.means
        std_devs = instance.std_deviations
        lb_specified = instance.lower_bounds is not None
        ub_specified = instance.upper_bounds is not None
        inf = float("inf")

        # --- lower bounds ---
        if lb_specified:
            object.__setattr__(
                instance, "inferred_lower_bounds", list(instance.lower_bounds)
            )
        else:
            object.__setattr__(instance, "lower_bounds", [-inf] * count)
            object.__setattr__(
                instance,
                "inferred_lower_bounds",
                [means[j] - 3.0 * std_devs[j] for j in range(count)],
            )

        # --- upper bounds ---
        if ub_specified:
            object.__setattr__(
                instance, "inferred_upper_bounds", list(instance.upper_bounds)
            )
        else:
            object.__setattr__(instance, "upper_bounds", [inf] * count)
            object.__setattr__(
                instance,
                "inferred_upper_bounds",
                [means[j] + 3.0 * std_devs[j] for j in range(count)],
            )


class NormalUncertainInitial(ValidationRule):
    """Set default initial_point for normal_uncertain variables.

    Uses special nudge logic for truncated normal distributions to keep
    the initial point away from bounds.

    This rule is specific to normal_uncertain, so the field names are fixed.
    """

    def __init__(self, context: str = "normal_uncertain"):
        super().__init__(
            context=context,
            rule_name="normal_uncertain_initial",
            fields=[],
            literals=[],
            error_message="For {context}, failed to set initial_point defaults",
        )

    def __call__(self, instance: Any) -> None:
        _reject_internal_field_input(
            instance, "initial_point_user_provided", self.context
        )
        if _USE_CPP:
            instance_dict = _variable_model_dump(instance)
            mutations = _cpp.normal_uncertain_initial(
                instance_dict, self.fields, self.literals, self.context
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback."""
        count = instance.count
        if instance.initial_point is not None:
            object.__setattr__(instance, "initial_point_user_provided", True)
            return
        means = instance.means
        std_devs = instance.std_deviations

        lb_specified = instance.lower_bounds is not None
        ub_specified = instance.upper_bounds is not None

        bds = 0
        if lb_specified:
            bds |= 1
        if ub_specified:
            bds |= 2

        initial_point = []
        for j in range(count):
            mean = means[j]
            stdev = std_devs[j]

            if bds == 0:
                val = mean
            elif bds == 1:
                lb = instance.lower_bounds[j]
                val = lb + 0.5 * stdev if mean <= lb else mean
            elif bds == 2:
                ub = instance.upper_bounds[j]
                val = ub - 0.5 * stdev if mean >= ub else mean
            else:
                lb = instance.lower_bounds[j]
                ub = instance.upper_bounds[j]
                nudge = 0.5 * min(stdev, ub - lb)
                lower = lb + nudge
                upper = ub - nudge
                if mean < lower:
                    val = lower
                elif mean > upper:
                    val = upper
                else:
                    val = mean
            initial_point.append(val)

        object.__setattr__(instance, "initial_point", initial_point)


class LognormalUncertainBounds(ValidationRule):
    """Default and infer bounds for lognormal_uncertain variables.

    If lower_bounds not specified, defaults to [0]*n.
    If upper_bounds specified, inferred_upper_bounds = upper_bounds.
    If not, upper_bounds defaults to [inf]*n and
    inferred_upper_bounds = [mean + 3*stdev]*n.

    Mean and stdev are extracted from whichever parameterization is active:
      option_1: means + std_deviations
      option_2: means + error_factors (stdev derived via error factor)
      option_3: lambdas + zetas (moments derived from lognormal params)

    Must run BEFORE LognormalUncertainInitial.

    Field names are hardcoded — not parameterized.
    """

    def __init__(self, context: str = "lognormal_uncertain"):
        super().__init__(
            context=context,
            rule_name="lognormal_uncertain_bounds",
            fields=[],
            literals=[],
            error_message="For {context}, failed to set default/inferred bounds",
        )

    def __call__(self, instance: Any) -> None:
        _reject_internal_field_input(instance, "inferred_upper_bounds", self.context)
        if _USE_CPP:
            instance_dict = _variable_model_dump(instance)
            mutations = _cpp.lognormal_uncertain_bounds(
                instance_dict, self.fields, self.literals, self.context
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    @staticmethod
    def _resolve_params(instance):
        """Extract (means, std_devs) lists from whichever parameterization is active.

        Returns (means, std_devs) or (None, None) if parameters can't be resolved.
        """
        import math

        params = getattr(instance, "parameters", None)
        if params is None:
            return None, None

        # Option 1: means + std_deviations
        opt1 = getattr(params, "option_1", None)
        if opt1 is not None:
            means = opt1.means
            std_devs = opt1.std_deviations
            if means is not None and std_devs is not None:
                return list(means), list(std_devs)

        # Option 2: means + error_factors
        opt2 = getattr(params, "option_2", None)
        if opt2 is not None:
            means = opt2.means
            error_factors = opt2.error_factors
            if means is not None and error_factors is not None:
                inv_cdf_095 = 1.6448536269514729
                std_devs = []
                for m, ef in zip(means, error_factors):
                    zeta = math.log(ef) / inv_cdf_095
                    std_devs.append(m * math.sqrt(math.expm1(zeta * zeta)))
                return list(means), std_devs

        # Option 3: lambdas + zetas
        opt3 = getattr(params, "option_3", None)
        if opt3 is not None:
            lambdas = opt3.lambdas
            zetas = opt3.zetas
            if lambdas is not None and zetas is not None:
                means = []
                std_devs = []
                for lam, zet in zip(lambdas, zetas):
                    zeta_sq = zet * zet
                    mean = math.exp(lam + zeta_sq / 2.0)
                    stdev = mean * math.sqrt(math.expm1(zeta_sq))
                    means.append(mean)
                    std_devs.append(stdev)
                return means, std_devs

        return None, None

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback."""
        count = instance.count
        lb_specified = instance.lower_bounds is not None
        ub_specified = instance.upper_bounds is not None
        inf = float("inf")

        # --- lower bounds: default to 0 (lognormal support > 0) ---
        if not lb_specified:
            object.__setattr__(instance, "lower_bounds", [0.0] * count)

        # --- upper bounds ---
        if ub_specified:
            object.__setattr__(
                instance, "inferred_upper_bounds", list(instance.upper_bounds)
            )
        else:
            means, std_devs = self._resolve_params(instance)
            if means is None or std_devs is None:
                object.__setattr__(instance, "upper_bounds", [inf] * count)
                return

            object.__setattr__(instance, "upper_bounds", [inf] * count)
            object.__setattr__(
                instance,
                "inferred_upper_bounds",
                [means[j] + 3.0 * std_devs[j] for j in range(count)],
            )


class LognormalUncertainInitial(ValidationRule):
    """Set default initial_point for lognormal_uncertain variables."""

    def __init__(self, context: str = "lognormal_uncertain"):
        super().__init__(
            context=context,
            rule_name="lognormal_uncertain_initial",
            fields=[],
            literals=[],
            error_message="For {context}, failed to set initial_point defaults",
        )

    def __call__(self, instance: Any) -> None:
        _reject_internal_field_input(
            instance, "initial_point_user_provided", self.context
        )
        if _USE_CPP:
            instance_dict = _variable_model_dump(instance)
            mutations = _cpp.lognormal_uncertain_initial(
                instance_dict, self.fields, self.literals, self.context
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback."""
        from math import isfinite

        count = instance.count
        if instance.initial_point is not None:
            _mark_initial_point_user_provided(instance)
            return
        means, std_devs = LognormalUncertainBounds._resolve_params(instance)
        if means is None or std_devs is None:
            return

        lb_specified = instance.lower_bounds is not None
        ub_specified = instance.upper_bounds is not None

        inf = float("inf")
        initial_point = []

        for j in range(count):
            mean = means[j]
            stdev = std_devs[j]
            lb = instance.lower_bounds[j] if lb_specified else 0.0
            ub = instance.upper_bounds[j] if ub_specified else inf

            nudge = 0.5 * min(stdev, ub - lb) if isfinite(ub) else 0.5 * stdev
            lower = lb + nudge
            upper = ub - nudge if isfinite(ub) else inf

            if mean < lower:
                val = lower
            elif isfinite(upper) and mean > upper:
                val = upper
            else:
                val = mean
            initial_point.append(val)

        object.__setattr__(instance, "initial_point", initial_point)


class UniformUncertainInitial(ValidationRule):
    """Set default initial_point for uniform_uncertain variables.

    Sets initial_point to midpoint of bounds, or repairs existing values.

    Fields: [initial_point, lower_bounds, upper_bounds, count]
    Literals: []
    """

    def __init__(self, context: str = "uniform_uncertain"):
        super().__init__(
            context=context,
            rule_name="uniform_uncertain_initial",
            fields=[
                "initial_point",
                "lower_bounds",
                "upper_bounds",
                "count",
                "initial_point_user_provided",
            ],
            literals=[],
            error_message="For {context}, failed to set initial_point defaults",
        )

    def __call__(self, instance: Any) -> None:
        _reject_internal_field_input(
            instance, "initial_point_user_provided", self.context
        )
        if _USE_CPP:
            instance_dict = _variable_model_dump(instance)
            mutations = _cpp.uniform_uncertain_initial(
                instance_dict, self.fields, self.literals, self.context
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback."""
        count = instance.count
        lower = instance.lower_bounds
        upper = instance.upper_bounds

        if instance.initial_point is None:
            initial_point = [(lower[j] + upper[j]) / 2.0 for j in range(count)]
            object.__setattr__(instance, "initial_point", initial_point)
        else:
            _mark_initial_point_user_provided(instance)
            # Repair
            ip = list(instance.initial_point)
            changed = False
            for j in range(count):
                if ip[j] < lower[j]:
                    ip[j] = lower[j]
                    changed = True
                elif ip[j] > upper[j]:
                    ip[j] = upper[j]
                    changed = True
            if changed:
                object.__setattr__(instance, "initial_point", ip)


class LoguniformUncertainInitial(ValidationRule):
    """Validate and set default initial_point for loguniform_uncertain.

    Validates bounds are positive and finite.

    Fields: [initial_point, lower_bounds, upper_bounds, count]
    Literals: []
    """

    def __init__(self, context: str = "loguniform_uncertain"):
        super().__init__(
            context=context,
            rule_name="loguniform_uncertain_initial",
            fields=[
                "initial_point",
                "lower_bounds",
                "upper_bounds",
                "count",
                "initial_point_user_provided",
            ],
            literals=[],
            error_message="For {context}, loguniform bounds must be positive and finite",
        )

    def __call__(self, instance: Any) -> None:
        _reject_internal_field_input(
            instance, "initial_point_user_provided", self.context
        )
        if _USE_CPP:
            instance_dict = _variable_model_dump(instance)
            mutations = _cpp.loguniform_uncertain_initial(
                instance_dict, self.fields, self.literals, self.context
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback."""
        from math import log, isfinite

        count = instance.count
        lower = instance.lower_bounds
        upper = instance.upper_bounds

        # Validate
        for j in range(count):
            lb, ub = lower[j], upper[j]
            if lb <= 0 or ub <= 0:
                raise ValueError("loguniform bounds must be positive")
            if not isfinite(lb) or not isfinite(ub):
                raise ValueError("loguniform bounds must be finite")
            if lb > ub:
                raise ValueError("loguniform lower bound greater than upper bound")

        if instance.initial_point is None:
            # Loguniform mean: (b-a)/(ln(b)-ln(a))
            initial_point = []
            for j in range(count):
                lb, ub = lower[j], upper[j]
                if lb == ub:
                    initial_point.append(lb)
                else:
                    initial_point.append((ub - lb) / (log(ub) - log(lb)))
            object.__setattr__(instance, "initial_point", initial_point)
        else:
            _mark_initial_point_user_provided(instance)
            # Repair
            ip = list(instance.initial_point)
            changed = False
            for j in range(count):
                if ip[j] < lower[j]:
                    ip[j] = lower[j]
                    changed = True
                elif ip[j] > upper[j]:
                    ip[j] = upper[j]
                    changed = True
            if changed:
                object.__setattr__(instance, "initial_point", ip)


class TriangularUncertainInitial(ValidationRule):
    """Validate and set default initial_point for triangular_uncertain.

    Validates lower_bounds <= modes <= upper_bounds.

    Fields: [initial_point, modes, lower_bounds, upper_bounds, count]
    Literals: []
    """

    def __init__(self, context: str = "triangular_uncertain"):
        super().__init__(
            context=context,
            rule_name="triangular_uncertain_initial",
            fields=[
                "initial_point",
                "modes",
                "lower_bounds",
                "upper_bounds",
                "count",
                "initial_point_user_provided",
            ],
            literals=[],
            error_message="For {context}, triangular variables require lower <= mode <= upper",
        )

    def __call__(self, instance: Any) -> None:
        _reject_internal_field_input(
            instance, "initial_point_user_provided", self.context
        )
        if _USE_CPP:
            instance_dict = _variable_model_dump(instance)
            mutations = _cpp.triangular_uncertain_initial(
                instance_dict, self.fields, self.literals, self.context
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback."""
        count = instance.count
        modes = instance.modes
        lower = instance.lower_bounds
        upper = instance.upper_bounds

        # Validate ordering
        for j in range(count):
            if lower[j] > modes[j] or modes[j] > upper[j]:
                raise ValueError(
                    "triangular uncertain variables must have "
                    "lower_bounds <= modes <= upper_bounds"
                )

        if instance.initial_point is None:
            initial_point = [
                (lower[j] + modes[j] + upper[j]) / 3.0 for j in range(count)
            ]
            object.__setattr__(instance, "initial_point", initial_point)
        else:
            _mark_initial_point_user_provided(instance)
            ip = list(instance.initial_point)
            changed = False
            for j in range(count):
                if ip[j] < lower[j]:
                    ip[j] = lower[j]
                    changed = True
                elif ip[j] > upper[j]:
                    ip[j] = upper[j]
                    changed = True
            if changed:
                object.__setattr__(instance, "initial_point", ip)


class ExponentialUncertainInitial(ValidationRule):
    """Set default initial_point for exponential_uncertain.

    Fields: [initial_point, betas, count]
    Literals: []
    """

    def __init__(self, context: str = "exponential_uncertain"):
        super().__init__(
            context=context,
            rule_name="exponential_uncertain_initial",
            fields=["initial_point", "betas", "count", "initial_point_user_provided"],
            literals=[],
            error_message="For {context}, failed to set initial_point defaults",
        )

    def __call__(self, instance: Any) -> None:
        _reject_internal_field_input(
            instance, "initial_point_user_provided", self.context
        )
        if _USE_CPP:
            instance_dict = _variable_model_dump(instance)
            mutations = _cpp.exponential_uncertain_initial(
                instance_dict, self.fields, self.literals, self.context
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback."""
        if instance.initial_point is not None:
            _mark_initial_point_user_provided(instance)
            return
        # Mean = beta
        initial_point = list(instance.betas)
        object.__setattr__(instance, "initial_point", initial_point)


class BetaUncertainInitial(ValidationRule):
    """Set default initial_point for beta_uncertain.

    Fields: [initial_point, alphas, betas, lower_bounds, upper_bounds, count]
    Literals: []
    """

    def __init__(self, context: str = "beta_uncertain"):
        super().__init__(
            context=context,
            rule_name="beta_uncertain_initial",
            fields=[
                "initial_point",
                "alphas",
                "betas",
                "lower_bounds",
                "upper_bounds",
                "count",
                "initial_point_user_provided",
            ],
            literals=[],
            error_message="For {context}, failed to set initial_point defaults",
        )

    def __call__(self, instance: Any) -> None:
        _reject_internal_field_input(
            instance, "initial_point_user_provided", self.context
        )
        if _USE_CPP:
            instance_dict = _variable_model_dump(instance)
            mutations = _cpp.beta_uncertain_initial(
                instance_dict, self.fields, self.literals, self.context
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback."""
        count = instance.count
        alphas = instance.alphas
        betas = instance.betas
        lower = instance.lower_bounds
        upper = instance.upper_bounds

        if instance.initial_point is None:
            initial_point = []
            for j in range(count):
                std_mean = alphas[j] / (alphas[j] + betas[j])
                mean = lower[j] + (upper[j] - lower[j]) * std_mean
                initial_point.append(mean)
            object.__setattr__(instance, "initial_point", initial_point)
        else:
            _mark_initial_point_user_provided(instance)
            ip = list(instance.initial_point)
            changed = False
            for j in range(count):
                if ip[j] < lower[j]:
                    ip[j] = lower[j]
                    changed = True
                elif ip[j] > upper[j]:
                    ip[j] = upper[j]
                    changed = True
            if changed:
                object.__setattr__(instance, "initial_point", ip)


class GammaUncertainInitial(ValidationRule):
    """Set default initial_point for gamma_uncertain.

    Fields: [initial_point, alphas, betas, count]
    Literals: []
    """

    def __init__(self, context: str = "gamma_uncertain"):
        super().__init__(
            context=context,
            rule_name="gamma_uncertain_initial",
            fields=[
                "initial_point",
                "alphas",
                "betas",
                "count",
                "initial_point_user_provided",
            ],
            literals=[],
            error_message="For {context}, failed to set initial_point defaults",
        )

    def __call__(self, instance: Any) -> None:
        _reject_internal_field_input(
            instance, "initial_point_user_provided", self.context
        )
        if _USE_CPP:
            instance_dict = _variable_model_dump(instance)
            mutations = _cpp.gamma_uncertain_initial(
                instance_dict, self.fields, self.literals, self.context
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback."""
        if instance.initial_point is not None:
            _mark_initial_point_user_provided(instance)
            return
        # Mean = alpha * beta
        initial_point = [
            instance.alphas[j] * instance.betas[j] for j in range(instance.count)
        ]
        object.__setattr__(instance, "initial_point", initial_point)


class GumbelUncertainInitial(ValidationRule):
    """Set default initial_point for gumbel_uncertain.

    Fields: [initial_point, alphas, betas, count]
    Literals: []
    """

    def __init__(self, context: str = "gumbel_uncertain"):
        super().__init__(
            context=context,
            rule_name="gumbel_uncertain_initial",
            fields=[
                "initial_point",
                "alphas",
                "betas",
                "count",
                "initial_point_user_provided",
            ],
            literals=[],
            error_message="For {context}, failed to set initial_point defaults",
        )

    def __call__(self, instance: Any) -> None:
        _reject_internal_field_input(
            instance, "initial_point_user_provided", self.context
        )
        if _USE_CPP:
            instance_dict = _variable_model_dump(instance)
            mutations = _cpp.gumbel_uncertain_initial(
                instance_dict, self.fields, self.literals, self.context
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback."""
        if instance.initial_point is not None:
            _mark_initial_point_user_provided(instance)
            return
        euler_mascheroni = 0.5772156649015329
        initial_point = [
            instance.betas[j] + euler_mascheroni / instance.alphas[j]
            for j in range(instance.count)
        ]
        object.__setattr__(instance, "initial_point", initial_point)


class FrechetUncertainInitial(ValidationRule):
    """Set default initial_point for frechet_uncertain.

    Fields: [initial_point, alphas, betas, count]
    Literals: []
    """

    def __init__(self, context: str = "frechet_uncertain"):
        super().__init__(
            context=context,
            rule_name="frechet_uncertain_initial",
            fields=[
                "initial_point",
                "alphas",
                "betas",
                "count",
                "initial_point_user_provided",
            ],
            literals=[],
            error_message="For {context}, failed to set initial_point defaults",
        )

    def __call__(self, instance: Any) -> None:
        _reject_internal_field_input(
            instance, "initial_point_user_provided", self.context
        )
        if _USE_CPP:
            instance_dict = _variable_model_dump(instance)
            mutations = _cpp.frechet_uncertain_initial(
                instance_dict, self.fields, self.literals, self.context
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback."""
        from math import gamma

        if instance.initial_point is not None:
            _mark_initial_point_user_provided(instance)
            return
        initial_point = []
        for j in range(instance.count):
            alpha = instance.alphas[j]
            beta = instance.betas[j]
            if alpha <= 1:
                mean = beta
            else:
                mean = beta * gamma(1 - 1 / alpha)
            initial_point.append(mean)
        object.__setattr__(instance, "initial_point", initial_point)


class WeibullUncertainInitial(ValidationRule):
    """Set default initial_point for weibull_uncertain.

    Fields: [initial_point, alphas, betas, count]
    Literals: []
    """

    def __init__(self, context: str = "weibull_uncertain"):
        super().__init__(
            context=context,
            rule_name="weibull_uncertain_initial",
            fields=[
                "initial_point",
                "alphas",
                "betas",
                "count",
                "initial_point_user_provided",
            ],
            literals=[],
            error_message="For {context}, failed to set initial_point defaults",
        )

    def __call__(self, instance: Any) -> None:
        _reject_internal_field_input(
            instance, "initial_point_user_provided", self.context
        )
        if _USE_CPP:
            instance_dict = _variable_model_dump(instance)
            mutations = _cpp.weibull_uncertain_initial(
                instance_dict, self.fields, self.literals, self.context
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback."""
        from math import gamma

        if instance.initial_point is not None:
            _mark_initial_point_user_provided(instance)
            return
        initial_point = []
        for j in range(instance.count):
            alpha = instance.alphas[j]
            beta = instance.betas[j]
            mean = beta * gamma(1 + 1 / alpha)
            initial_point.append(mean)
        object.__setattr__(instance, "initial_point", initial_point)


# =============================================================================
# VariablesConfig Validators
# =============================================================================


class UncertainCorrelationMatrixSize(ValidationRule):
    """Validate uncertain_correlation_matrix length equals n^2.

    n is the sum of counts for continuous aleatory uncertain variable blocks:
    normal, lognormal, uniform, loguniform, triangular, exponential, beta,
    gamma, gumbel, frechet, weibull, and histogram_bin_uncertain.
    """

    _CONTINUOUS_ALEATORY_FIELDS = (
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
    )

    def __init__(self, context: str = "variables"):
        super().__init__(
            context=context,
            rule_name="uncertain_correlation_matrix_size",
            fields=["uncertain_correlation_matrix"],
            literals=[],
            error_message=(
                "For {context}, uncertain_correlation_matrix must have length n^2, "
                "where n is the total count of continuous aleatory uncertain variables"
            ),
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(mode="python")
            mutations = _cpp.uncertain_correlation_matrix_size(
                instance_dict, self.fields, self.literals, self.context
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        correlation_matrix = getattr(instance, "uncertain_correlation_matrix", None)
        if correlation_matrix is None:
            return

        n = 0
        for field_name in self._CONTINUOUS_ALEATORY_FIELDS:
            var_block = getattr(instance, field_name, None)
            if var_block is None:
                continue
            count = getattr(var_block, "count", None)
            if count is not None:
                n += int(count)

        expected = n * n
        actual = len(correlation_matrix)
        if actual != expected:
            raise ValueError(
                f"uncertain_correlation_matrix length must be {expected} "
                f"(n^2 with n={n}), but got {actual}"
            )

        for i, value in enumerate(correlation_matrix):
            if value < -1.0 or value > 1.0:
                raise ValueError(
                    f"uncertain_correlation_matrix[{i}] = {value} is outside [-1, 1]"
                )


# =============================================================================
# Discrete Aleatory Uncertain Validators
# =============================================================================


class PoissonUncertainInitial(ValidationRule):
    """Set default initial_point for poisson_uncertain.

    Fields: [initial_point, lambdas, count]
    Literals: []
    """

    def __init__(self, context: str = "poisson_uncertain"):
        super().__init__(
            context=context,
            rule_name="poisson_uncertain_initial",
            fields=["initial_point", "lambdas", "count", "initial_point_user_provided"],
            literals=[],
            error_message="For {context}, failed to set initial_point defaults",
        )

    def __call__(self, instance: Any) -> None:
        _reject_internal_field_input(
            instance, "initial_point_user_provided", self.context
        )
        if _USE_CPP:
            instance_dict = _variable_model_dump(instance)
            mutations = _cpp.poisson_uncertain_initial(
                instance_dict, self.fields, self.literals, self.context
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback."""
        if instance.initial_point is not None:
            _mark_initial_point_user_provided(instance)
            return
        initial_point = [int(lam) for lam in instance.lambdas]
        object.__setattr__(instance, "initial_point", initial_point)


class BinomialUncertainInitial(ValidationRule):
    """Set default initial_point for binomial_uncertain.

    Fields: [initial_point, probability_per_trial, num_trials, count]
    Literals: []
    """

    def __init__(self, context: str = "binomial_uncertain"):
        super().__init__(
            context=context,
            rule_name="binomial_uncertain_initial",
            fields=[
                "initial_point",
                "probability_per_trial",
                "num_trials",
                "count",
                "initial_point_user_provided",
            ],
            literals=[],
            error_message="For {context}, failed to set initial_point defaults",
        )

    def __call__(self, instance: Any) -> None:
        _reject_internal_field_input(
            instance, "initial_point_user_provided", self.context
        )
        if _USE_CPP:
            instance_dict = _variable_model_dump(instance)
            mutations = _cpp.binomial_uncertain_initial(
                instance_dict, self.fields, self.literals, self.context
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback."""
        count = instance.count

        if instance.initial_point is not None:
            _mark_initial_point_user_provided(instance)
            # Repair to bounds
            ip = list(instance.initial_point)
            changed = False
            for j in range(count):
                ub = instance.num_trials[j]
                if ip[j] > ub:
                    ip[j] = ub
                    changed = True
                elif ip[j] < 0:
                    ip[j] = 0
                    changed = True
            if changed:
                object.__setattr__(instance, "initial_point", ip)
            return

        initial_point = [
            int(instance.num_trials[j] * instance.probability_per_trial[j])
            for j in range(count)
        ]
        object.__setattr__(instance, "initial_point", initial_point)


class NegativeBinomialUncertainInitial(ValidationRule):
    """Set default initial_point for negative_binomial_uncertain.

    Fields: [initial_point, probability_per_trial, num_trials, count]
    Literals: []
    """

    def __init__(self, context: str = "negative_binomial_uncertain"):
        super().__init__(
            context=context,
            rule_name="negative_binomial_uncertain_initial",
            fields=[
                "initial_point",
                "probability_per_trial",
                "num_trials",
                "count",
                "initial_point_user_provided",
            ],
            literals=[],
            error_message="For {context}, failed to set initial_point defaults",
        )

    def __call__(self, instance: Any) -> None:
        _reject_internal_field_input(
            instance, "initial_point_user_provided", self.context
        )
        if _USE_CPP:
            instance_dict = _variable_model_dump(instance)
            mutations = _cpp.negative_binomial_uncertain_initial(
                instance_dict, self.fields, self.literals, self.context
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback."""
        count = instance.count

        if instance.initial_point is not None:
            _mark_initial_point_user_provided(instance)
            # Repair to lower bound
            ip = list(instance.initial_point)
            changed = False
            for j in range(count):
                lb = instance.num_trials[j]
                if ip[j] < lb:
                    ip[j] = lb
                    changed = True
            if changed:
                object.__setattr__(instance, "initial_point", ip)
            return

        # Mean = r * (1-p) / p
        initial_point = []
        for j in range(count):
            r = instance.num_trials[j]
            p = instance.probability_per_trial[j]
            mean = r * (1 - p) / p
            initial_point.append(int(mean))
        object.__setattr__(instance, "initial_point", initial_point)


class GeometricUncertainInitial(ValidationRule):
    """Set default initial_point for geometric_uncertain.

    Fields: [initial_point, probability_per_trial, count]
    Literals: []
    """

    def __init__(self, context: str = "geometric_uncertain"):
        super().__init__(
            context=context,
            rule_name="geometric_uncertain_initial",
            fields=[
                "initial_point",
                "probability_per_trial",
                "count",
                "initial_point_user_provided",
            ],
            literals=[],
            error_message="For {context}, failed to set initial_point defaults",
        )

    def __call__(self, instance: Any) -> None:
        _reject_internal_field_input(
            instance, "initial_point_user_provided", self.context
        )
        if _USE_CPP:
            instance_dict = _variable_model_dump(instance)
            mutations = _cpp.geometric_uncertain_initial(
                instance_dict, self.fields, self.literals, self.context
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback."""
        if instance.initial_point is not None:
            _mark_initial_point_user_provided(instance)
            return
        # Mean = (1-p)/p
        initial_point = [int((1 - p) / p) for p in instance.probability_per_trial]
        object.__setattr__(instance, "initial_point", initial_point)


class HypergeometricUncertainInitial(ValidationRule):
    """Set default initial_point for hypergeometric_uncertain.

    Fields: [initial_point, total_population, selected_population, num_drawn, count]
    Literals: []
    """

    def __init__(self, context: str = "hypergeometric_uncertain"):
        super().__init__(
            context=context,
            rule_name="hypergeometric_uncertain_initial",
            fields=[
                "initial_point",
                "total_population",
                "selected_population",
                "num_drawn",
                "count",
                "initial_point_user_provided",
            ],
            literals=[],
            error_message="For {context}, failed to set initial_point defaults",
        )

    def __call__(self, instance: Any) -> None:
        _reject_internal_field_input(
            instance, "initial_point_user_provided", self.context
        )
        if _USE_CPP:
            instance_dict = _variable_model_dump(instance)
            mutations = _cpp.hypergeometric_uncertain_initial(
                instance_dict, self.fields, self.literals, self.context
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback."""
        count = instance.count

        if instance.initial_point is not None:
            _mark_initial_point_user_provided(instance)
            # Repair to bounds
            ip = list(instance.initial_point)
            changed = False
            for j in range(count):
                ub = min(instance.num_drawn[j], instance.selected_population[j])
                if ip[j] > ub:
                    ip[j] = ub
                    changed = True
                elif ip[j] < 0:
                    ip[j] = 0
                    changed = True
            if changed:
                object.__setattr__(instance, "initial_point", ip)
            return

        # Mean = n * K / N
        initial_point = []
        for j in range(count):
            mean = (
                instance.num_drawn[j]
                * instance.selected_population[j]
                / instance.total_population[j]
            )
            initial_point.append(int(mean))
        object.__setattr__(instance, "initial_point", initial_point)


# =============================================================================
# Shared Helpers
# =============================================================================


def _compute_apportionment(count: int, per_variable, total_len: int) -> list[int]:
    """Compute per-variable element counts from a per_variable list or even split.

    Args:
        count: Number of variables
        per_variable: Per-variable counts (list or None for even split)
        total_len: Total length of the flat array

    Returns:
        List of element counts per variable
    """
    if per_variable is not None:
        return list(per_variable)
    if count == 0:
        return []
    if total_len % count != 0:
        raise ValueError(
            f"Number of elements ({total_len}) not evenly divisible "
            f"by number of variables ({count}); use elements/pairs_per_variable "
            f"for unequal apportionment"
        )
    avg = total_len // count
    return [avg] * count


def _mid_or_next_lower_index(n: int) -> int:
    """Return the middle index (or next lower for even n).

    Matches C++ mid_or_next_lower_index: midpoint(0, n-1) = (n-1)//2
    """
    return (n - 1) // 2


# =============================================================================
# Epistemic Uncertain - Interval Types
# =============================================================================


class ContinuousIntervalUncertainInitial(ValidationRule):
    """Set default initial_point for continuous_interval_uncertain variables.

    Computes per-variable global bounds from the flat interval arrays,
    then defaults initial_point to the midpoint (uniform mean) or clamps
    existing values.

    Fields: [initial_point, lower_bounds, upper_bounds, num_intervals, count]
    Literals: []
    """

    def __init__(self, context: str = "continuous_interval_uncertain"):
        super().__init__(
            context=context,
            rule_name="continuous_interval_uncertain_initial",
            fields=[
                "initial_point",
                "lower_bounds",
                "upper_bounds",
                "num_intervals",
                "count",
                "initial_point_user_provided",
            ],
            literals=[],
            error_message="For {context}, failed to set initial_point defaults",
        )

    def __call__(self, instance: Any) -> None:
        _reject_internal_field_input(
            instance, "initial_point_user_provided", self.context
        )
        if _USE_CPP:
            instance_dict = _variable_model_dump(instance)
            mutations = _cpp.continuous_interval_uncertain_initial(
                instance_dict, self.fields, self.literals, self.context
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback."""
        count = instance.count
        flat_lb = instance.lower_bounds
        flat_ub = instance.upper_bounds
        apportionment = _compute_apportionment(
            count, instance.num_intervals, len(flat_lb)
        )

        # Compute per-variable global bounds
        global_lb = []
        global_ub = []
        offset = 0
        for j in range(count):
            n_int = apportionment[j]
            var_lb = float("inf")
            var_ub = float("-inf")
            for k in range(n_int):
                lbk = flat_lb[offset + k]
                ubk = flat_ub[offset + k]
                if var_lb > lbk:
                    var_lb = lbk
                if var_ub < ubk:
                    var_ub = ubk
            global_lb.append(var_lb)
            global_ub.append(var_ub)
            offset += n_int

        if instance.initial_point is None:
            initial_point = [(global_lb[j] + global_ub[j]) / 2.0 for j in range(count)]
            object.__setattr__(instance, "initial_point", initial_point)
        else:
            _mark_initial_point_user_provided(instance)
            ip = list(instance.initial_point)
            changed = False
            for j in range(count):
                if ip[j] < global_lb[j]:
                    ip[j] = global_lb[j]
                    changed = True
                elif ip[j] > global_ub[j]:
                    ip[j] = global_ub[j]
                    changed = True
            if changed:
                object.__setattr__(instance, "initial_point", ip)


class DiscreteIntervalUncertainInitial(ValidationRule):
    """Set default initial_point for discrete_interval_uncertain variables.

    Same logic as continuous but with integer types. Uses truncation toward
    zero for midpoint to match C++ integer division semantics.

    Fields: [initial_point, lower_bounds, upper_bounds, num_intervals, count]
    Literals: []
    """

    def __init__(self, context: str = "discrete_interval_uncertain"):
        super().__init__(
            context=context,
            rule_name="discrete_interval_uncertain_initial",
            fields=[
                "initial_point",
                "lower_bounds",
                "upper_bounds",
                "num_intervals",
                "count",
                "initial_point_user_provided",
            ],
            literals=[],
            error_message="For {context}, failed to set initial_point defaults",
        )

    def __call__(self, instance: Any) -> None:
        _reject_internal_field_input(
            instance, "initial_point_user_provided", self.context
        )
        if _USE_CPP:
            instance_dict = _variable_model_dump(instance)
            mutations = _cpp.discrete_interval_uncertain_initial(
                instance_dict, self.fields, self.literals, self.context
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback."""
        count = instance.count
        flat_lb = instance.lower_bounds
        flat_ub = instance.upper_bounds
        apportionment = _compute_apportionment(
            count, instance.num_intervals, len(flat_lb)
        )

        global_lb = []
        global_ub = []
        offset = 0
        for j in range(count):
            n_int = apportionment[j]
            var_lb = 2**31 - 1  # INT_MAX
            var_ub = -(2**31)  # INT_MIN
            for k in range(n_int):
                lbk = flat_lb[offset + k]
                ubk = flat_ub[offset + k]
                if var_lb > lbk:
                    var_lb = lbk
                if var_ub < ubk:
                    var_ub = ubk
            global_lb.append(var_lb)
            global_ub.append(var_ub)
            offset += n_int

        if instance.initial_point is None:
            # Truncate toward zero to match C++ integer division semantics
            initial_point = [
                int((global_lb[j] + global_ub[j]) / 2) for j in range(count)
            ]
            object.__setattr__(instance, "initial_point", initial_point)
        else:
            _mark_initial_point_user_provided(instance)
            ip = list(instance.initial_point)
            changed = False
            for j in range(count):
                if ip[j] < global_lb[j]:
                    ip[j] = global_lb[j]
                    changed = True
                elif ip[j] > global_ub[j]:
                    ip[j] = global_ub[j]
                    changed = True
            if changed:
                object.__setattr__(instance, "initial_point", ip)


# =============================================================================
# Histogram Uncertain Validators
# =============================================================================


class HistogramBinUncertainInitial(ValidationRule):
    """Set default initial_point for histogram_bin_uncertain variables.

    Computes the probability-weighted mean of bin midpoints, matching
    Pecos::HistogramBinRandomVariable::central_moments_from_params.

    The density field is a union (Ordinates | Counts):
    - Ordinates: probability density values; mass_i = ordinate_i * bin_width_i
    - Counts: frequency counts; mass_i = count_i

    Mean = sum(mass_i * midpoint_i) / sum(mass_i)

    Clamping uses [first_abscissa, last_abscissa] per variable as bounds.

    Fields: [initial_point, pairs_per_variable, abscissas, density, count]
    Literals: []
    """

    def __init__(self, context: str = "histogram_bin_uncertain"):
        super().__init__(
            context=context,
            rule_name="histogram_bin_uncertain_initial",
            fields=[
                "initial_point",
                "pairs_per_variable",
                "abscissas",
                "density",
                "count",
                "initial_point_user_provided",
            ],
            literals=[],
            error_message="For {context}, failed to set initial_point defaults",
        )

    def __call__(self, instance: Any) -> None:
        _reject_internal_field_input(
            instance, "initial_point_user_provided", self.context
        )
        if _USE_CPP:
            instance_dict = _variable_model_dump(instance)
            mutations = _cpp.histogram_bin_uncertain_initial(
                instance_dict, self.fields, self.literals, self.context
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    @staticmethod
    def _extract_density(instance) -> tuple:
        """Extract density values and format from the union field.

        Returns:
            (values, is_counts) where values is the flat list and
            is_counts indicates whether they are counts (True) or
            ordinates/densities (False).
        """
        density = instance.density
        if density is None:
            return None, False
        # Pydantic model: check which variant of the union is active
        if hasattr(density, "counts") and density.counts is not None:
            return density.counts, True
        if hasattr(density, "ordinates") and density.ordinates is not None:
            return density.ordinates, False
        return None, False

    @staticmethod
    def _histogram_bin_mean(abscissas, density_values, is_counts, n_pairs):
        """Compute histogram bin mean matching Pecos central_moments_from_params.

        The abscissas define n_pairs bin edges (n_pairs - 1 bins).
        The last density value is the trailing zero (ignored for mass).

        For each bin i (0..n_bins-1):
          - lwr = abscissas[i], upr = abscissas[i+1]
          - If counts: mass_i = density_values[i]
          - If ordinates: mass_i = density_values[i] * (upr - lwr)

        mean = sum(mass_i * (lwr + upr)) / (2 * sum(mass_i))
        """
        n_bins = n_pairs - 1
        if n_bins < 1:
            return abscissas[0]

        sum_mass = 0.0
        sum_mass_midpoint2 = 0.0  # sum of mass * (lwr + upr)

        for i in range(n_bins):
            lwr = abscissas[i]
            upr = abscissas[i + 1]
            y = density_values[i]
            bin_width = upr - lwr

            if is_counts:
                mass = y
            else:
                mass = y * bin_width

            sum_mass += mass
            sum_mass_midpoint2 += mass * (lwr + upr)

        if sum_mass <= 0.0:
            return (abscissas[0] + abscissas[n_pairs - 1]) / 2.0

        return sum_mass_midpoint2 / (2.0 * sum_mass)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback."""
        count = instance.count
        abscissas = instance.abscissas
        density_values, is_counts = self._extract_density(instance)
        apportionment = _compute_apportionment(
            count, instance.pairs_per_variable, len(abscissas)
        )

        # Per-variable: bounds from first/last abscissa
        var_slices = []
        offset = 0
        for j in range(count):
            n_pairs = apportionment[j]
            var_slices.append((offset, n_pairs))
            offset += n_pairs

        if instance.initial_point is None:
            initial_point = []
            for j in range(count):
                off, n_p = var_slices[j]
                lb = abscissas[off]
                ub = abscissas[off + n_p - 1]

                if density_values is not None and n_p >= 2:
                    mean = self._histogram_bin_mean(
                        abscissas[off : off + n_p],
                        density_values[off : off + n_p],
                        is_counts,
                        n_p,
                    )
                else:
                    # Fallback to midpoint if density unavailable
                    mean = (lb + ub) / 2.0

                initial_point.append(mean)
            object.__setattr__(instance, "initial_point", initial_point)
        else:
            _mark_initial_point_user_provided(instance)
            # Clamp to [first_abscissa, last_abscissa] per variable
            ip = list(instance.initial_point)
            changed = False
            for j in range(count):
                off, n_p = var_slices[j]
                lb = abscissas[off]
                ub = abscissas[off + n_p - 1]
                if ip[j] < lb:
                    ip[j] = lb
                    changed = True
                elif ip[j] > ub:
                    ip[j] = ub
                    changed = True
            if changed:
                object.__setattr__(instance, "initial_point", ip)


class HistogramPointIntUncertainInitial(ValidationRule):
    """Set default initial_point for histogram_point_uncertain/integer.

    Computes the weighted mean from abscissas and counts, then snaps
    to the nearest abscissa value. Matches C++ Vgen_HistogramPtIntUnc.

    Fields: [initial_point, pairs_per_variable, abscissas, counts, count]
    Literals: []
    """

    def __init__(self, context: str = "histogram_point_int_uncertain"):
        super().__init__(
            context=context,
            rule_name="histogram_point_int_uncertain_initial",
            fields=[
                "initial_point",
                "pairs_per_variable",
                "abscissas",
                "counts",
                "count",
                "initial_point_user_provided",
            ],
            literals=[],
            error_message="For {context}, failed to set initial_point defaults",
        )

    def __call__(self, instance: Any) -> None:
        _reject_internal_field_input(
            instance, "initial_point_user_provided", self.context
        )
        if _USE_CPP:
            instance_dict = _variable_model_dump(instance)
            mutations = _cpp.histogram_point_int_uncertain_initial(
                instance_dict, self.fields, self.literals, self.context
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback."""
        count = instance.count
        abscissas = instance.abscissas
        counts = instance.counts
        apportionment = _compute_apportionment(
            count, instance.pairs_per_variable, len(abscissas)
        )

        # Per-variable: bounds from first/last abscissa
        var_slices = []
        offset = 0
        for j in range(count):
            n_pairs = apportionment[j]
            var_slices.append((offset, n_pairs))
            offset += n_pairs

        if instance.initial_point is None:
            initial_point = []
            for j in range(count):
                off, n_p = var_slices[j]
                abs_j = abscissas[off : off + n_p]
                cnt_j = counts[off : off + n_p]

                if n_p == 1:
                    initial_point.append(abs_j[0])
                else:
                    # Weighted mean, snap to nearest abscissa
                    total = sum(cnt_j)
                    if total > 0:
                        mean = sum(a * c for a, c in zip(abs_j, cnt_j)) / total
                    else:
                        mean = abs_j[0]
                    # Find nearest abscissa to mean
                    best = abs_j[0]
                    best_dist = abs(mean - best)
                    for a in abs_j[1:]:
                        d = abs(mean - a)
                        if d < best_dist:
                            best = a
                            best_dist = d
                    initial_point.append(int(best))
            object.__setattr__(instance, "initial_point", initial_point)
        else:
            _mark_initial_point_user_provided(instance)
            # Validate that each initial point is a member of its variable's abscissa set
            for j in range(count):
                off, n_p = var_slices[j]
                subset = abscissas[off : off + n_p]
                if instance.initial_point[j] not in subset:
                    raise ValueError(
                        f"initial_point[{j}] = {instance.initial_point[j]!r} is not a member "
                        f"of the abscissa set for variable {j}"
                    )


class HistogramPointStrUncertainInitial(ValidationRule):
    """Set default initial_point for histogram_point_uncertain/string.

    Computes the mean index (probability-weighted), rounds to nearest,
    and uses the corresponding abscissa. Matches C++ Vgen_HistogramPtStrUnc.

    Fields: [initial_point, pairs_per_variable, abscissas, counts, count]
    Literals: []
    """

    def __init__(self, context: str = "histogram_point_str_uncertain"):
        super().__init__(
            context=context,
            rule_name="histogram_point_str_uncertain_initial",
            fields=[
                "initial_point",
                "pairs_per_variable",
                "abscissas",
                "counts",
                "count",
                "initial_point_user_provided",
            ],
            literals=[],
            error_message="For {context}, failed to set initial_point defaults",
        )

    def __call__(self, instance: Any) -> None:
        _reject_internal_field_input(
            instance, "initial_point_user_provided", self.context
        )
        if _USE_CPP:
            instance_dict = _variable_model_dump(instance)
            mutations = _cpp.histogram_point_str_uncertain_initial(
                instance_dict, self.fields, self.literals, self.context
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback."""
        count = instance.count
        abscissas = instance.abscissas
        counts = instance.counts
        apportionment = _compute_apportionment(
            count, instance.pairs_per_variable, len(abscissas)
        )

        var_slices = []
        offset = 0
        for j in range(count):
            n_pairs = apportionment[j]
            var_slices.append((offset, n_pairs))
            offset += n_pairs

        if instance.initial_point is None:
            initial_point = []
            for j in range(count):
                off, n_p = var_slices[j]
                abs_j = abscissas[off : off + n_p]
                cnt_j = counts[off : off + n_p]

                if n_p == 1:
                    initial_point.append(abs_j[0])
                else:
                    # Mean index weighted by counts, round to nearest
                    total = sum(cnt_j)
                    if total > 0:
                        mean_idx = sum(i * c for i, c in enumerate(cnt_j)) / total
                    else:
                        mean_idx = 0
                    idx = min(round(mean_idx), n_p - 1)
                    initial_point.append(abs_j[idx])
            object.__setattr__(instance, "initial_point", initial_point)
        else:
            _mark_initial_point_user_provided(instance)
            # Validate that each initial point is a member of its variable's abscissa set
            for j in range(count):
                off, n_p = var_slices[j]
                subset = abscissas[off : off + n_p]
                if instance.initial_point[j] not in subset:
                    raise ValueError(
                        f"initial_point[{j}] = {instance.initial_point[j]!r} is not a member "
                        f"of the abscissa set for variable {j}"
                    )


class HistogramPointRealUncertainInitial(ValidationRule):
    """Set default initial_point for histogram_point_uncertain/real.

    Same logic as integer variant but with float values.

    Fields: [initial_point, pairs_per_variable, abscissas, counts, count]
    Literals: []
    """

    def __init__(self, context: str = "histogram_point_real_uncertain"):
        super().__init__(
            context=context,
            rule_name="histogram_point_real_uncertain_initial",
            fields=[
                "initial_point",
                "pairs_per_variable",
                "abscissas",
                "counts",
                "count",
                "initial_point_user_provided",
            ],
            literals=[],
            error_message="For {context}, failed to set initial_point defaults",
        )

    def __call__(self, instance: Any) -> None:
        _reject_internal_field_input(
            instance, "initial_point_user_provided", self.context
        )
        if _USE_CPP:
            instance_dict = _variable_model_dump(instance)
            mutations = _cpp.histogram_point_real_uncertain_initial(
                instance_dict, self.fields, self.literals, self.context
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback."""
        count = instance.count
        abscissas = instance.abscissas
        counts = instance.counts
        apportionment = _compute_apportionment(
            count, instance.pairs_per_variable, len(abscissas)
        )

        var_slices = []
        offset = 0
        for j in range(count):
            n_pairs = apportionment[j]
            var_slices.append((offset, n_pairs))
            offset += n_pairs

        if instance.initial_point is None:
            initial_point = []
            for j in range(count):
                off, n_p = var_slices[j]
                abs_j = abscissas[off : off + n_p]
                cnt_j = counts[off : off + n_p]

                if n_p == 1:
                    initial_point.append(abs_j[0])
                else:
                    total = sum(cnt_j)
                    if total > 0:
                        mean = sum(a * c for a, c in zip(abs_j, cnt_j)) / total
                    else:
                        mean = abs_j[0]
                    best = abs_j[0]
                    best_dist = abs(mean - best)
                    for a in abs_j[1:]:
                        d = abs(mean - a)
                        if d < best_dist:
                            best = a
                            best_dist = d
                    initial_point.append(best)
            object.__setattr__(instance, "initial_point", initial_point)
        else:
            # Validate that each initial point is a member of its variable's abscissa set
            for j in range(count):
                off, n_p = var_slices[j]
                subset = abscissas[off : off + n_p]
                if instance.initial_point[j] not in subset:
                    raise ValueError(
                        f"initial_point[{j}] = {instance.initial_point[j]!r} is not a member "
                        f"of the abscissa set for variable {j}"
                    )


# =============================================================================
# Discrete Set Variable Validators
# =============================================================================


class CheckSetElementsOrdering(ValidationRule):
    """Validate that elements are strictly increasing within each variable's subset.

    Partitions the flat elements array by variable using elements_per_variable
    (or even split if absent), then checks strict ordering within each partition.
    Works for int, real, and string element types.

    Fields: [elements_per_variable, elements, count]
    Literals: []
    """

    def __init__(
        self,
        context: str,
        elements_per_variable_field: str = "elements_per_variable",
        elements_field: str = "elements",
        count_field: str = "count",
    ):
        self._epv_field = elements_per_variable_field
        self._elements_field = elements_field
        self._count_field = count_field
        super().__init__(
            context=context,
            rule_name="check_set_elements_ordering",
            fields=[elements_per_variable_field, elements_field, count_field],
            literals=[],
            error_message="For {context}, set elements must be strictly increasing per variable.",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            cpp_fn = getattr(_cpp, "check_set_elements_ordering")
            cpp_fn(instance_dict, self.fields, self.literals, self.context)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        count = getattr(instance, self._count_field, None)
        if count is None or count <= 0:
            return

        elements = getattr(instance, self._elements_field, None)
        if elements is None or len(elements) == 0:
            return

        epv = getattr(instance, self._epv_field, None)
        apportionment = _compute_apportionment(count, epv, len(elements))

        offset = 0
        for j in range(count):
            n_e = apportionment[j]
            for k in range(1, n_e):
                prev = elements[offset + k - 1]
                curr = elements[offset + k]
                if not (prev < curr):
                    raise ValueError(
                        f"elements for variable {j} are not strictly increasing: "
                        f"elements[{offset + k - 1}] = {prev!r} "
                        f">= elements[{offset + k}] = {curr!r}"
                    )
            offset += n_e


class DefaultSetProbabilities(ValidationRule):
    """Default or normalize set_probabilities for discrete uncertain set variables.

    If set_probabilities is null, generates uniform probabilities (1/k per element).
    If provided, normalizes each variable's subset to sum to 1.0.

    Fields: [set_probabilities, elements_per_variable, elements, count]
    Literals: []
    """

    def __init__(
        self,
        context: str,
        set_probabilities_field: str = "set_probabilities",
        elements_per_variable_field: str = "elements_per_variable",
        elements_field: str = "elements",
        count_field: str = "count",
    ):
        self._prob_field = set_probabilities_field
        self._epv_field = elements_per_variable_field
        self._elements_field = elements_field
        self._count_field = count_field
        super().__init__(
            context=context,
            rule_name="default_set_probabilities",
            fields=[
                set_probabilities_field,
                elements_per_variable_field,
                elements_field,
                count_field,
            ],
            literals=[],
            error_message="For {context}, failed to set/normalize set_probabilities.",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            cpp_fn = getattr(_cpp, "default_set_probabilities")
            mutations = cpp_fn(instance_dict, self.fields, self.literals, self.context)
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        count = getattr(instance, self._count_field, None)
        if count is None or count <= 0:
            return

        elements = getattr(instance, self._elements_field, None)
        if elements is None or len(elements) == 0:
            return

        epv = getattr(instance, self._epv_field, None)
        apportionment = _compute_apportionment(count, epv, len(elements))

        probs = getattr(instance, self._prob_field, None)

        if probs is None:
            # Generate uniform probabilities
            result = []
            for j in range(count):
                n_e = apportionment[j]
                p = 1.0 / n_e if n_e > 0 else 0.0
                result.extend([p] * n_e)
            object.__setattr__(instance, self._prob_field, result)
        else:
            # Normalize each subset to sum to 1
            probs = list(probs)
            changed = False
            offset = 0
            for j in range(count):
                n_e = apportionment[j]
                s = sum(probs[offset : offset + n_e])
                if s != 1.0 and s > 0.0:
                    for k in range(n_e):
                        probs[offset + k] /= s
                    changed = True
                offset += n_e
            if changed:
                object.__setattr__(instance, self._prob_field, probs)


def _default_initial_from_set(
    instance, ip_field: str, epv_field: str, elements_field: str, count_field: str
) -> None:
    """Common logic for defaulting initial values from a flat set.

    Default = middle element of each variable's partition (matching C++
    mid_or_next_lower_index). Validates that user-provided values are
    members of the element set.

    Works for int, real, and string element types.
    """
    count = getattr(instance, count_field)
    elements = getattr(instance, elements_field)
    epv = getattr(instance, epv_field)
    apportionment = _compute_apportionment(count, epv, len(elements))

    current_ip = getattr(instance, ip_field)

    if current_ip is None:
        # Default: middle element per variable
        initial = []
        offset = 0
        for j in range(count):
            n_elems = apportionment[j]
            if n_elems == 0:
                # Should not happen but be defensive
                initial.append(elements[offset] if offset < len(elements) else 0)
            elif n_elems == 1:
                initial.append(elements[offset])
            else:
                mid_idx = _mid_or_next_lower_index(n_elems)
                initial.append(elements[offset + mid_idx])
            offset += n_elems
        object.__setattr__(instance, ip_field, initial)
    else:
        _mark_initial_point_user_provided(instance)
        # Validate that each initial point is a member of its variable's element set
        offset = 0
        for j in range(count):
            n_elems = apportionment[j]
            subset = elements[offset : offset + n_elems]
            if current_ip[j] not in subset:
                raise ValueError(
                    f"{ip_field}[{j}] = {current_ip[j]!r} is not a member "
                    f"of the element set for variable {j}"
                )
            offset += n_elems


class _SetInitialBase(ValidationRule):
    """Base class for discrete set variable initial value validators.

    Subclasses specify the field names and C++ function to call.
    All share the same logic: default initial to middle element.
    """

    def __init__(
        self,
        context: str,
        rule_name: str,
        ip_field: str,
        epv_field: str = "elements_per_variable",
        elements_field: str = "elements",
        count_field: str = "count",
        track_user_provided_flag: bool = False,
    ):
        self._ip_field = ip_field
        self._epv_field = epv_field
        self._elements_field = elements_field
        self._count_field = count_field
        self._track_user_provided_flag = track_user_provided_flag
        fields = [ip_field, epv_field, elements_field, count_field]
        if track_user_provided_flag:
            fields.append("initial_point_user_provided")
        super().__init__(
            context=context,
            rule_name=rule_name,
            fields=fields,
            literals=[],
            error_message=f"For {{context}}, failed to set {ip_field} defaults",
        )

    def __call__(self, instance: Any) -> None:
        if self._track_user_provided_flag:
            _reject_internal_field_input(
                instance, "initial_point_user_provided", self.context
            )
        if _USE_CPP:
            instance_dict = _variable_model_dump(instance)
            cpp_fn = getattr(_cpp, self.rule_name)
            mutations = cpp_fn(instance_dict, self.fields, self.literals, self.context)
            _apply_mutations(instance, mutations)
        else:
            _default_initial_from_set(
                instance,
                self._ip_field,
                self._epv_field,
                self._elements_field,
                self._count_field,
            )


# --- Design set types ---


class DiscreteDesignSetIntInitial(_SetInitialBase):
    """Default initial_point for discrete_design_set/integer."""

    def __init__(self, context: str = "discrete_design_set_integer"):
        super().__init__(
            context=context,
            rule_name="discrete_design_set_int_initial",
            ip_field="initial_point",
        )


class DiscreteDesignSetStrInitial(_SetInitialBase):
    """Default initial_point for discrete_design_set/string."""

    def __init__(self, context: str = "discrete_design_set_string"):
        super().__init__(
            context=context,
            rule_name="discrete_design_set_str_initial",
            ip_field="initial_point",
        )


class DiscreteDesignSetRealInitial(_SetInitialBase):
    """Default initial_point for discrete_design_set/real."""

    def __init__(self, context: str = "discrete_design_set_real"):
        super().__init__(
            context=context,
            rule_name="discrete_design_set_real_initial",
            ip_field="initial_point",
        )


# --- State set types ---


class DiscreteStateSetIntInitial(_SetInitialBase):
    """Default initial_state for discrete_state_set/integer."""

    def __init__(self, context: str = "discrete_state_set_integer"):
        super().__init__(
            context=context,
            rule_name="discrete_state_set_int_initial",
            ip_field="initial_state",
        )


class DiscreteStateSetStrInitial(_SetInitialBase):
    """Default initial_state for discrete_state_set/string."""

    def __init__(self, context: str = "discrete_state_set_string"):
        super().__init__(
            context=context,
            rule_name="discrete_state_set_str_initial",
            ip_field="initial_state",
        )


class DiscreteStateSetRealInitial(_SetInitialBase):
    """Default initial_state for discrete_state_set/real."""

    def __init__(self, context: str = "discrete_state_set_real"):
        super().__init__(
            context=context,
            rule_name="discrete_state_set_real_initial",
            ip_field="initial_state",
        )


# --- Uncertain set types ---


class DiscreteUncertainSetIntInitial(_SetInitialBase):
    """Default initial_point for discrete_uncertain_set/integer."""

    def __init__(self, context: str = "discrete_uncertain_set_integer"):
        super().__init__(
            context=context,
            rule_name="discrete_uncertain_set_int_initial",
            ip_field="initial_point",
            track_user_provided_flag=True,
        )


class DiscreteUncertainSetStrInitial(_SetInitialBase):
    """Default initial_point for discrete_uncertain_set/string."""

    def __init__(self, context: str = "discrete_uncertain_set_string"):
        super().__init__(
            context=context,
            rule_name="discrete_uncertain_set_str_initial",
            ip_field="initial_point",
            track_user_provided_flag=True,
        )


class DiscreteUncertainSetRealInitial(_SetInitialBase):
    """Default initial_point for discrete_uncertain_set/real."""

    def __init__(self, context: str = "discrete_uncertain_set_real"):
        super().__init__(
            context=context,
            rule_name="discrete_uncertain_set_real_initial",
            ip_field="initial_point",
            track_user_provided_flag=True,
        )
