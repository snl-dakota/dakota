#!/usr/bin/env python3
"""Example: PoissonUncertain model with validators and computed fields.

This example demonstrates how validators (for initial_point) and computed fields
(for bounds) work together in a Dakota variable model.

Both use C++ implementations for consistency with the DSL reader.

Run this example:
    # From the build directory after building the project:
    PYTHONPATH=. python3 ../examples/poisson_uncertain_example.py
"""

from __future__ import annotations
from typing import Any, ClassVar, List
import math

from pydantic import BaseModel, Field, computed_field, model_validator

# Import validation infrastructure
# In real usage, these would be:
#   from dakota.spec.validation import ValidationRule
#   from dakota.spec.validation.variable_rules import PoissonUncertainInitial
#   from dakota.spec.validation.computed_fields import computed_field_schema, PoissonBounds

# For this standalone example, we include simplified versions inline:

# Try to import C++ backend
try:
    import dakota_validation_cpp as _cpp
    _USE_CPP = True
    print("Using C++ backend for validation and computed fields")
except ImportError:
    _USE_CPP = False
    print("C++ backend not available, using pure Python fallback")


def _poisson_moments(lam: float) -> tuple[float, float]:
    """Poisson distribution: mean = lambda, std_dev = sqrt(lambda)"""
    return lam, math.sqrt(lam)


def computed_field_schema(function_name: str) -> dict:
    """Generate json_schema_extra for a computed field."""
    return {
        "json_schema_extra": {
            "x-computed-field": {
                "function": function_name
            }
        }
    }


class PoissonBounds:
    """Helper class for computing Poisson distribution bounds.
    
    Uses C++ when available, falls back to pure Python otherwise.
    """
    
    @staticmethod
    def lower_bounds(instance: Any) -> List[int]:
        """Lower bounds are always 0 for Poisson."""
        if _USE_CPP:
            return _cpp.poisson_lower_bounds(instance.model_dump(mode='python'))
        return [0] * instance.count
    
    @staticmethod
    def upper_bounds(instance: Any) -> List[int]:
        """Upper bounds: ceil(lambda + 3*sqrt(lambda))"""
        if _USE_CPP:
            return _cpp.poisson_upper_bounds(instance.model_dump(mode='python'))
        result = []
        for lam in instance.lambdas:
            mean, stdev = _poisson_moments(lam)
            result.append(int(math.ceil(mean + 3 * stdev)))
        return result


class ValidationRule:
    """Base class for validation rules (simplified)."""
    def __init__(self, context: str, rule_name: str, fields: list, literals: list, error_message: str):
        self.context = context
        self.rule_name = rule_name
        self.fields = fields
        self.literals = literals
        self.error_message = error_message


class PoissonUncertainInitial(ValidationRule):
    """Set default initial_point for poisson_uncertain."""
    
    def __init__(self, context: str = "poisson_uncertain"):
        super().__init__(
            context=context,
            rule_name="poisson_uncertain_initial",
            fields=["initial_point", "lambdas", "count"],
            literals=[],
            error_message="For {context}, failed to set initial_point defaults"
        )
    
    def __call__(self, instance) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(mode='python')
            mutations = _cpp.poisson_uncertain_initial(
                instance_dict, self.fields, self.literals, self.context
            )
            for field_name, value in mutations.items():
                object.__setattr__(instance, field_name, value)
        else:
            self._python_impl(instance)
    
    def _python_impl(self, instance) -> None:
        """Pure Python fallback."""
        if instance.initial_point is not None:
            return
        if instance.lambdas is None:
            return
        
        initial_point = []
        for lam in instance.lambdas:
            mean, _ = _poisson_moments(lam)
            initial_point.append(int(mean))
        object.__setattr__(instance, 'initial_point', initial_point)


# =============================================================================
# The Pydantic Model
# =============================================================================

class PoissonUncertain(BaseModel):
    """Poisson uncertain variable specification.
    
    This model demonstrates:
    1. Computed fields (lower_bounds, upper_bounds) - derived from distribution params
    2. Validators (initial_point) - set defaults or repair user values
    
    Both use C++ implementations for consistency with the DSL reader.
    """
    
    model_config = {"validate_default": True}
    
    # -------------------------------------------------------------------------
    # Required fields (from DSL)
    # -------------------------------------------------------------------------
    count: int = Field(gt=0, description="Number of variables")
    lambdas: List[float] = Field(description="Rate parameters (lambda > 0)")
    
    # -------------------------------------------------------------------------
    # Optional fields (validator sets defaults)
    # -------------------------------------------------------------------------
    initial_point: List[int] | None = Field(
        default=None, 
        description="Initial values for variables"
    )
    descriptors: List[str] | None = Field(
        default=None,
        description="Labels for the variables"
    )
    
    # -------------------------------------------------------------------------
    # Validation rules
    # -------------------------------------------------------------------------
    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        PoissonUncertainInitial(context="poisson_uncertain"),
    ]
    
    @model_validator(mode='after')
    def _run_validation_rules(self) -> 'PoissonUncertain':
        """Run all validation rules after Pydantic's field validation."""
        for rule in self._VALIDATION_RULES:
            rule(self)
        return self
    
    # -------------------------------------------------------------------------
    # Computed fields (read-only, derived from distribution parameters)
    # Use the helper class which handles C++/Python fallback automatically
    # -------------------------------------------------------------------------
    @computed_field(**computed_field_schema("poisson_lower_bounds"))
    @property
    def lower_bounds(self) -> List[int]:
        """Poisson is non-negative, so lower bound is always 0."""
        return PoissonBounds.lower_bounds(self)
    
    @computed_field(**computed_field_schema("poisson_upper_bounds"))
    @property
    def upper_bounds(self) -> List[int]:
        """Upper bound provides reasonable range for discrete sampling."""
        return PoissonBounds.upper_bounds(self)


# =============================================================================
# Demo
# =============================================================================

def main():
    import json
    
    print("=" * 70)
    print("Example 1: Model with defaults")
    print("=" * 70)
    
    p = PoissonUncertain(
        count=3,
        lambdas=[5.0, 10.0, 20.0]
    )
    
    print(f"Input:")
    print(f"  count = 3")
    print(f"  lambdas = [5.0, 10.0, 20.0]")
    print()
    print(f"Computed/defaulted values:")
    print(f"  lower_bounds (computed): {p.lower_bounds}")
    print(f"  upper_bounds (computed): {p.upper_bounds}")
    print(f"  initial_point (validator): {p.initial_point}")
    print()
    
    print("=" * 70)
    print("Example 2: Model with user-specified initial_point")
    print("=" * 70)
    
    p2 = PoissonUncertain(
        count=2,
        lambdas=[8.0, 15.0],
        initial_point=[7, 14]
    )
    
    print(f"Input:")
    print(f"  count = 2")
    print(f"  lambdas = [8.0, 15.0]")
    print(f"  initial_point = [7, 14]")
    print()
    print(f"Values:")
    print(f"  lower_bounds: {p2.lower_bounds}")
    print(f"  upper_bounds: {p2.upper_bounds}")
    print(f"  initial_point: {p2.initial_point}  (user value preserved)")
    print()
    
    print("=" * 70)
    print("model_dump() output (includes computed fields)")
    print("=" * 70)
    print(json.dumps(p.model_dump(), indent=2))
    print()
    
    print("=" * 70)
    print("Note on JSON Schema")
    print("=" * 70)
    print("Computed fields are NOT included in model_json_schema(mode='validation').")
    print("The model generator emits them separately in 'x-computed-fields'.")
    print()
    
    print("=" * 70)
    print("Using C++ registry directly")
    print("=" * 70)
    if _USE_CPP:
        instance = {"count": 2, "lambdas": [4.0, 9.0]}
        print(f"Instance: {instance}")
        print(f"Registry compute 'poisson_upper_bounds': {_cpp.compute(instance, 'poisson_upper_bounds')}")
        print(f"Registry validate 'poisson_uncertain_initial':")
        mutations = _cpp.validate(
            {**instance, "initial_point": None}, 
            "poisson_uncertain_initial",
            ["initial_point", "lambdas", "count"],
            [],
            "example"
        )
        print(f"  Mutations: {mutations}")
    else:
        print("C++ backend not available")


if __name__ == "__main__":
    main()
