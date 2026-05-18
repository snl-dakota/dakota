"""Base model for all Dakota specification models

This module provides DakotaBaseModel, which all generated Dakota models inherit from.
It provides:
- Automatic oneOf validation for Union fields
- Rejection of extra/unknown fields (extra='forbid')
- Re-validation on field assignment
- Consistent configuration across all models
- Special handling for Pattern 3 (optional Union with dict and default)
- Automatic integration of validation rules from mixins
- Dakota numeric constants (DBL_MAX, INT_MAX, etc.)
- Optional suppression of protected namespace warnings via DAKOTA_SHOW_PYDANTIC_WARNINGS env var
"""

import os
import math
from dataclasses import dataclass
from typing import Any, Type, get_origin, get_args, Union, Annotated
from pydantic import BaseModel, ConfigDict, model_validator, Field


def DakotaField(*, dakota: dict | None = None, **field_kwargs):
    """Dakota wrapper around pydantic.Field that emits Dakota JSON-schema extensions.

    Args:
        dakota: Dakota-specific field metadata expressed with Python-friendly keys.
            Known keys (Python -> JSON Schema key):
              - aliases -> x-aliases
              - block_pointer -> x-block-pointer
              - union_pattern -> x-union-pattern
              - model_default -> x-model-default
              - materialization -> x-materialization
              - argument -> argument
              - anchor -> anchor

            For forward-compatibility, any unknown key is emitted as 'x-<key>' unless
            it already starts with 'x-' or is one of ('argument','anchor').

        **field_kwargs: forwarded to pydantic.Field.

    Returns:
        A pydantic FieldInfo produced by pydantic.Field.
    """
    extra: dict = {}
    # Start from any user-provided json_schema_extra, but let `dakota` override on conflicts.
    existing = field_kwargs.pop("json_schema_extra", None)
    if isinstance(existing, dict):
        extra.update(existing)

    if dakota:
        friendly_map = {
            "aliases": "x-aliases",
            "block_pointer": "x-block-pointer",
            "union_pattern": "x-union-pattern",
            "model_default": "x-model-default",
            "materialization": "x-materialization",
            "internal_only": "x-internal-only",
            "argument": "argument",
            "anchor": "anchor",
        }
        for k, v in dakota.items():
            if v is None:
                continue
            out_key = friendly_map.get(k)
            if out_key is None:
                if k in ("argument", "anchor") or k.startswith("x-"):
                    out_key = k
                else:
                    out_key = f"x-{k}"
            extra[out_key] = v

    if extra:
        field_kwargs["json_schema_extra"] = extra

    return Field(**field_kwargs)


from pydantic.functional_serializers import PlainSerializer
import numpy as np


def _serialize_dakota_float(v: float) -> float | str:
    """Serialize float, converting non-finite values to JSON-safe strings.

    JSON does not support NaN, Infinity, or -Infinity as literals.
    This serializer converts them to string representations that can be
    parsed by the C++ nlohmann JSON library with custom handling.

    Returns:
        The original float for finite values, or "nan", "inf", "-inf" for non-finite.
    """
    if v is None:
        return None
    if math.isnan(v):
        return "nan"
    elif math.isinf(v):
        return "-inf" if v < 0 else "inf"
    return v


# Custom float type that serializes non-finite values as strings for JSON compatibility
# Use this instead of float for all Dakota model fields that may contain NaN/Inf values
DakotaFloat = Annotated[
    float,
    PlainSerializer(_serialize_dakota_float, return_type=float | str, when_used="json"),
]


@dataclass(frozen=True)
class ComputedFieldSpec:
    """Specification for a computed field exported to JSON schema.

    Attributes:
        function: Name of the C++ function in the registry
        description: Human-readable description of the field
        return_type: Python type (e.g., int, list[int], list[float])
        materialization: Optional IR materialization metadata to expose in schema
    """

    function: str
    description: str
    return_type: Type
    materialization: list[dict[str, Any]] | None = None

    def to_json_schema(self) -> dict:
        """Convert this spec to JSON schema format."""
        out = {
            "function": self.function,
            "description": self.description,
            **_python_type_to_json_schema(self.return_type),
        }
        if self.materialization:
            out["x-materialization"] = self.materialization
        return out


def _python_type_to_json_schema(python_type: Type) -> dict:
    """Convert a Python type to JSON schema."""
    origin = get_origin(python_type)

    # Handle list[X] types
    if origin is list:
        args = get_args(python_type)
        if args:
            inner = args[0]
            items = {
                int: {"type": "integer"},
                float: {"type": "number"},
                str: {"type": "string"},
                bool: {"type": "boolean"},
            }.get(inner, {"type": "string"})
            return {"type": "array", "items": items}
        return {"type": "array"}

    # Handle simple types
    return {
        int: {"type": "integer"},
        float: {"type": "number"},
        str: {"type": "string"},
        bool: {"type": "boolean"},
    }.get(python_type, {"type": "string"})


# Dakota numeric constants
# These mirror Dakota's schema/parser-facing fixed-width semantics.
INT_MAX = int(np.iinfo(np.int32).max)
USHRT_MAX = int(np.iinfo(np.uint16).max)
SZ_MAX = int(np.iinfo(np.int64).max)
DBL_MAX = float(np.finfo(np.float64).max)
DBL_EPSILON = float(np.finfo(np.float64).eps)
NEG_DBL_MAX = -DBL_MAX


# Check environment for whether to show Pydantic warnings
SHOW_WARNINGS = os.getenv("DAKOTA_SHOW_PYDANTIC_WARNINGS", "false").lower() == "true"

# Base configuration for all Dakota models
base_model_config = {
    "extra": "forbid",  # Reject unknown fields
    "validate_assignment": True,  # Re-validate on assignment
    "str_strip_whitespace": True,  # Clean string inputs
    "use_enum_values": True,  # Future-proof for enums
}

# Only override protected_namespaces in production (suppress warnings)
# In dev mode (SHOW_WARNINGS=true), use Pydantic's default behavior
if not SHOW_WARNINGS:
    base_model_config["protected_namespaces"] = ()


class DakotaBaseModel(BaseModel):
    """Base model for all Dakota specification models

    Provides automatic validation of oneOf (Union) fields and consistent
    configuration for all Dakota models.
    """

    model_config = ConfigDict(**base_model_config)

    @model_validator(mode="before")
    @classmethod
    def _validate_union_fields(cls, data):
        """Validate that Union fields have exactly one variant

        This validator automatically checks all Union-typed fields in the model
        and ensures they contain exactly one of their allowed variants.
        For optional Union fields, applies defaults when the field is omitted.

        Only validates typing.Union (not pipe syntax) since all unions are
        generated using Union[...] syntax for Python 3.9+ compatibility.
        """
        if not isinstance(data, dict):
            return data

        for field_name, field_info in cls.model_fields.items():
            extra = field_info.json_schema_extra
            if (
                field_name in data
                and isinstance(extra, dict)
                and extra.get("x-internal-only")
            ):
                raise ValueError(
                    f"{field_name} is internal-only and cannot be provided by users"
                )

        for field_name, field_info in cls.model_fields.items():
            # Check if this is a Union field
            annotation = field_info.annotation
            origin = get_origin(annotation)
            if origin is not Union:
                continue

            # Get Union variants (excluding None)
            args = get_args(annotation)
            variants = [arg for arg in args if arg is not type(None)]
            is_optional = type(None) in args

            # Analyze Union structure
            dict_allowed = dict in variants
            model_variants = [
                v for v in variants if v is not dict and hasattr(v, "model_fields")
            ]

            # Build discriminator map for model variants
            key_to_model = {}
            for variant in model_variants:
                fields = list(variant.model_fields.keys())
                if len(fields) == 1:
                    key_to_model[fields[0]] = variant

            # Validate Union structure
            # If there's only one model variant, this is just an optional field, not oneOf
            # Let Pydantic handle it normally (skip oneOf validation)
            if len(model_variants) <= 1:
                continue

            if not key_to_model and not dict_allowed:
                raise TypeError(
                    f"Union field '{field_name}' has no valid variants with single-field models or dict. "
                    f"Cannot perform oneOf validation."
                )

            # Handle missing field
            if field_name not in data:
                # Special case Pattern 3: Optional Union with dict, defaulting to a Model
                # Convert omission to None instead of using the Model default
                if (
                    is_optional
                    and dict_allowed
                    and field_info.default_factory is not None
                    and field_info.default_factory in model_variants
                ):
                    data[field_name] = None
                    continue

                # For other optional fields, let Pydantic apply defaults
                # For required fields, let Pydantic raise the error
                continue

            field_value = data[field_name]

            # Handle None value for optional fields
            if field_value is None:
                continue

            # Handle already-validated model instances
            if isinstance(field_value, BaseModel):
                if not any(isinstance(field_value, v) for v in model_variants):
                    variant_names = [v.__name__ for v in model_variants]
                    raise ValueError(
                        f"{field_name} must be an instance of one of: {', '.join(variant_names)}, "
                        f"got {type(field_value).__name__}"
                    )
                continue

            # From here on, field_value must be a dict
            if not isinstance(field_value, dict):
                raise ValueError(
                    f"{field_name} must be a dictionary, got {type(field_value).__name__}"
                )

            # Handle empty dict case
            if len(field_value) == 0:
                if dict_allowed:
                    # Special case Pattern 3: {} with Model default should instantiate default
                    if (
                        field_info.default_factory is not None
                        and field_info.default_factory in model_variants
                    ):
                        data[field_name] = field_info.default_factory()
                    # Otherwise empty dict is valid as-is (Patterns 5, 6)
                    continue
                valid_keys = sorted(key_to_model.keys())
                raise ValueError(
                    f"{field_name} requires exactly one of: {', '.join(valid_keys)}"
                )

            # Handle non-empty dict - must match exactly one discriminator key
            keys = list(field_value.keys())
            if len(keys) > 1:
                raise ValueError(
                    f"{field_name} accepts only one option, got: {', '.join(keys)}"
                )

            provided_key = keys[0]
            if provided_key not in key_to_model:
                valid_keys = sorted(key_to_model.keys())
                raise ValueError(
                    f"{field_name} received unknown option '{provided_key}'. "
                    f"Must be one of: {', '.join(valid_keys)}"
                )

            # Validate using the selected branch
            selected_model = key_to_model[provided_key]
            try:
                validated_instance = selected_model(**field_value)
                data[field_name] = validated_instance
            except Exception as e:
                raise ValueError(
                    f"{field_name} failed validation for '{provided_key}': {str(e)}"
                )

        return data

    @model_validator(mode="after")
    def universal_validator(self):
        """Example universal validator that runs for all models"""
        # Could add logging, tracking, etc.
        return self

    def __init_subclass__(cls, **kwargs):
        """Automatically add validation and computed fields support to subclasses"""
        super().__init_subclass__(**kwargs)

        # Check what this class defines
        VALIDATION_RULES_ATTR = "_VALIDATION_RULES"
        COMPUTED_FIELDS_ATTR = "_COMPUTED_FIELDS"

        # Check annotations for ClassVar declarations
        annotations = getattr(cls, "__annotations__", {})
        has_validation_rules = VALIDATION_RULES_ATTR in annotations
        has_computed_fields = COMPUTED_FIELDS_ATTR in annotations

        # Early exit if nothing to do
        if not has_validation_rules and not has_computed_fields:
            return

        # Try to import validation tools (needed for validation rules)
        collect_and_execute_validation_rules = None
        add_validation_schema = None
        if has_validation_rules:
            try:
                from dakota.spec.validation import (
                    collect_and_execute_validation_rules as collect_func,
                    add_validation_schema as schema_func,
                )

                collect_and_execute_validation_rules = collect_func
                add_validation_schema = schema_func
            except ImportError:
                pass

        validation_available = add_validation_schema is not None

        # Add validation collector if we have validation rules and tools
        if has_validation_rules and validation_available:

            @model_validator(mode="after")
            def _validation_collector_method(self):
                return collect_and_execute_validation_rules(self.__class__, self)

            validator_name = f"_validation_collector_{id(cls)}"
            setattr(cls, validator_name, _validation_collector_method)

        # Build schema customization function
        # Capture add_validation_schema in closure
        _add_validation_schema = add_validation_schema

        def schema_extra_with_validation(schema: dict) -> None:
            if _add_validation_schema is not None:
                _add_validation_schema(schema, cls)

        def schema_extra_with_computed(schema: dict) -> None:
            computed_fields = getattr(cls, COMPUTED_FIELDS_ATTR, None)
            if computed_fields:
                schema["x-computed-fields"] = {
                    name: spec.to_json_schema()
                    for name, spec in computed_fields.items()
                }

        # Get current model_config and update json_schema_extra
        existing_extra = cls.model_config.get("json_schema_extra")

        def combined_schema_extra(schema: dict) -> None:
            if existing_extra is not None:
                if callable(existing_extra):
                    existing_extra(schema)
                else:
                    schema.update(existing_extra)
            if has_validation_rules and validation_available:
                schema_extra_with_validation(schema)
            if has_computed_fields:
                schema_extra_with_computed(schema)

        # Update model_config with new json_schema_extra
        new_config = dict(cls.model_config)
        new_config["json_schema_extra"] = combined_schema_extra
        cls.model_config = ConfigDict(**new_config)
