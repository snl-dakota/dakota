"""Dakota validation system.

This package provides a validation framework for Dakota Pydantic models.
Validation rules are defined as callable classes that can validate model
instances and export their specifications to JSON Schema.

The core validation logic is implemented in C++ (dakota_validation_cpp module)
to ensure consistency between Python/Pydantic validation and C++ DSL reader
validation. Pure Python fallbacks are available if the C++ module is not installed.

Usage:
    from typing import ClassVar, List
    from dakota.spec.base import DakotaBaseModel
    from dakota.spec.validation import ValidationRule
    from dakota.spec.validation.rules import CompareLength

    class MyModel(DakotaBaseModel):
        _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
            CompareLength(context="my_model", list_field="values", target_field="count"),
        ]

        count: int
        values: list[float] | None = None

Validation is automatically integrated via DakotaBaseModel.__init_subclass__
for any model that defines _VALIDATION_RULES.

Modules:
    base: ValidationRule ABC and constants
    executor: Rule collection, execution, and schema export
    rules: Concrete validation rule implementations (import directly from .rules)
    variable_rules: Distribution-specific validators for Dakota variable types
    computed_fields: Computed field support for derived values (e.g., bounds)
"""

# Base classes and constants
from .base import (
    ValidationRule,
    VALIDATION_RULES_ATTR,
    SCHEMA_KEY,
)

# Execution functions (used by DakotaBaseModel)
from .executor import (
    collect_and_execute_validation_rules,
    add_validation_schema,
)

# Computed field utilities
from .computed_fields import (
    computed_field_schema,
    cpp_computed_field,
)

# Check if C++ backend is available
try:
    from . import dakota_validation_cpp

    CPP_BACKEND_AVAILABLE = True
except ImportError:
    CPP_BACKEND_AVAILABLE = False

__all__ = [
    # Base
    "ValidationRule",
    "VALIDATION_RULES_ATTR",
    "SCHEMA_KEY",
    # Executor
    "collect_and_execute_validation_rules",
    "add_validation_schema",
    # Computed fields
    "computed_field_schema",
    "cpp_computed_field",
    # Status
    "CPP_BACKEND_AVAILABLE",
]
