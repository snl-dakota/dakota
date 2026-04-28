"""Validation rule execution and JSON Schema export.

This module provides functions for:
- Collecting validation rules from model class hierarchies
- Executing validation rules on model instances
- Exporting validation rules to JSON Schema

These functions are called by DakotaBaseModel's __init_subclass__ hook
to automatically integrate validation for models with _VALIDATION_RULES.
"""

from typing import Dict, Any, List, Type
from pydantic import BaseModel, ValidationError
from .base import ValidationRule, VALIDATION_RULES_ATTR, SCHEMA_KEY


def collect_and_execute_validation_rules(
    cls: Type[BaseModel], instance: BaseModel
) -> BaseModel:
    """Collect validation rules from MRO and execute them.

    Walks the class hierarchy (base-to-derived) collecting _VALIDATION_RULES
    from each class that directly defines them, then executes all rules in order.

    Args:
        cls: The model class
        instance: The model instance to validate

    Returns:
        The validated instance

    Raises:
        ValidationError: If any validation rule fails
    """
    all_rules: List[ValidationRule] = []
    seen_rule_lists = set()

    # Collect rules from MRO (base-to-derived order)
    for base in reversed(cls.__mro__):
        if VALIDATION_RULES_ATTR in base.__dict__:
            rules = getattr(base, VALIDATION_RULES_ATTR)
            rules_id = id(rules)
            if rules_id not in seen_rule_lists:
                seen_rule_lists.add(rules_id)
                all_rules.extend(rules)

    # Execute all rules
    for rule in all_rules:
        try:
            rule(instance)
        except ValueError as e:
            full_msg = f"{rule.error_message}: {str(e)}"
            raise ValidationError.from_exception_data(
                title=cls.__name__,
                line_errors=[
                    {
                        "type": "value_error",
                        "loc": (rule.context, rule.rule_name, *rule.fields),
                        "input": instance.model_dump(mode="python"),
                        "ctx": {"error": ValueError(full_msg)},
                    }
                ],
            )

    return instance


def add_validation_schema(schema: Dict[str, Any], model_class: Type[BaseModel]) -> None:
    """Add validation rule specifications to a JSON schema.

    Collects all _VALIDATION_RULES from the model's class hierarchy
    and adds them to the schema under the 'x-model-validations' key.

    Args:
        schema: The JSON schema dictionary to modify
        model_class: The model class to collect rules from
    """
    all_rules: List[ValidationRule] = []
    seen_rule_lists = set()

    # Collect rules from MRO (base-to-derived order)
    for base in reversed(model_class.__mro__):
        if VALIDATION_RULES_ATTR in base.__dict__:
            rules = getattr(base, VALIDATION_RULES_ATTR)
            rules_id = id(rules)
            if rules_id not in seen_rule_lists:
                seen_rule_lists.add(rules_id)
                all_rules.extend(rules)

    # Export rule specifications
    schema[SCHEMA_KEY] = [rule.get_schema_data() for rule in all_rules]
