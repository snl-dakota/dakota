"""Base classes and constants for the Dakota validation system.

This module provides:
- ValidationRule: Abstract base class for all validation functors
- VALIDATION_RULES_ATTR: The attribute name used to store rules on model classes
- SCHEMA_KEY: The JSON Schema key used to export validation info
"""

from typing import List, Any, Dict
from abc import ABC, abstractmethod
import math


# Attribute name for storing validation rules on model classes
VALIDATION_RULES_ATTR: str = "_VALIDATION_RULES"

# JSON Schema key for validation rule export
SCHEMA_KEY: str = "x-model-validations"


def _schema_safe_literal(value: Any) -> Any:
    """Convert non-finite Python floats into JSON-safe schema literals."""
    if isinstance(value, list):
        return [_schema_safe_literal(item) for item in value]
    if isinstance(value, dict):
        return {key: _schema_safe_literal(item) for key, item in value.items()}
    if isinstance(value, float):
        if math.isnan(value):
            return "nan"
        if math.isinf(value):
            return "-inf" if value < 0 else "inf"
    return value


class ValidationRule(ABC):
    """Abstract base class for all validation functors.

    Validation rules are callable objects that validate model instances
    and can export their specification to JSON Schema.

    Subclasses must implement __call__ to perform the actual validation.
    The validation logic is implemented in C++ and called via pybind11 bindings.

    Attributes:
        context: Context for the validation rule (e.g., "continuous_design")
        rule_name: Rule name exported to JSON schema
        fields: List of field names used in the validation
        literals: Literal values used in the validation
        error_message: Error message with substituted field names and literals
    """

    def __init__(
        self,
        *,
        context: str,
        rule_name: str,
        fields: List[str],
        literals: List[Any],
        error_message: str,
    ):
        """Initialize a validation rule.

        Args:
            context: Context for the validation rule and error message
            rule_name: Rule name that will be exported to JSON schema
            fields: List of field names used in the validation
            literals: Literal values used in the validation
            error_message: Error message template. Supports substitution:
                - {F0}, {F1}, ... for field names
                - {L0}, {L1}, ... for literals
                - {context} for the context
                - {rule_name} for the rule name
        """
        self.context = context
        self.rule_name = rule_name
        self.fields = fields
        self.literals = literals
        self.error_message = self._substitute_error_message(error_message)

    def _substitute_error_message(self, error_message: str) -> str:
        """Substitute tokens in error message with actual values."""
        token_map = {f"F{i}": field_name for i, field_name in enumerate(self.fields)}
        token_map.update(
            {f"L{i}": str(literal) for i, literal in enumerate(self.literals)}
        )
        token_map["context"] = self.context
        token_map["rule_name"] = self.rule_name
        try:
            return error_message.format_map(token_map)
        except KeyError as e:
            raise KeyError(
                f"Error in error message for rule '{self.rule_name}': Unsubstituted token {e}."
            ) from e

    @abstractmethod
    def __call__(self, instance: Any) -> None:
        """Execute the validation rule on a model instance.

        Args:
            instance: The model instance to validate

        Raises:
            ValueError: If validation fails
        """
        pass

    def get_schema_data(self) -> Dict[str, Any]:
        """Export rule specification for JSON Schema.

        Returns:
            Dictionary containing the rule specification
        """
        return {
            "validationRuleName": self.rule_name,
            "validationFields": self.fields,
            "validationLiterals": _schema_safe_literal(self.literals),
            "validationContext": self.context,
            "validationErrorMessage": self.error_message,
        }
