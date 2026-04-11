"""Top-level Dakota study specification

This module provides the DakotaStudy model that combines all major sections
of a Dakota input specification.
"""

from .base import DakotaBaseModel, DakotaField
from .environment import EnvironmentConfig
from .method import MethodUnion
from .model import ModelUnion
from .interface import InterfaceConfig
from .responses import ResponsesConfig
from .variables import VariablesConfig
from pydantic import field_validator, model_validator
from typing import Any


class DakotaStudy(DakotaBaseModel):
    """Complete Dakota study specification

    A Dakota study consists of:
    - environment (optional): Execution environment settings
    - method (required): One or more iterative algorithms
    - model (optional): One or more model configurations (surrogate, nested, etc.)
    - variables (required): One or more variable specifications
    - responses (required): One or more response function definitions
    - interface (optional): One or more simulation interface configurations

    Each section can have multiple instances to support advanced Dakota workflows.

    The method and model fields support three input styles:
    1. Dict with discriminator: method=[{"sampling": {"samples": 10}}]
    2. Selection instance: method=[SamplingSelection(sampling=SamplingConfig(samples=10))]
    3. Bare config (auto-wrapped): method=[SamplingConfig(samples=10)]
    """

    environment: EnvironmentConfig | None = DakotaField(
        default=None, description="Execution environment settings (optional)"
    )

    method: list[MethodUnion] = DakotaField(
        description="Iterative algorithm configurations (required, one or more)"
    )

    model: list[ModelUnion] | None = DakotaField(
        default=None, description="Model configurations (optional, zero or more)"
    )

    variables: list[VariablesConfig] = DakotaField(
        description="Variable specifications (required, one or more)"
    )

    responses: list[ResponsesConfig] = DakotaField(
        description="Response function definitions (required, one or more)"
    )

    interface: list[InterfaceConfig] | None = DakotaField(
        default=None,
        description="Simulation interface configurations (optional, zero or more)",
    )

    @staticmethod
    def _validate_selection_list(
        value: Any, field_name: str, selection_class: type, is_optional: bool = False
    ) -> list:
        """Shared validation logic for method/model lists

        Args:
            value: The input value to validate
            field_name: Name of the field being validated (for error messages)
            selection_class: The Selection base class (MethodSelection or ModelSelection)
            is_optional: Whether None is allowed

        Returns:
            List of validated Selection instances
        """
        if value is None:
            if not is_optional:
                raise ValueError(f"{field_name} cannot be None")
            return value

        if not isinstance(value, list):
            raise ValueError(
                f"{field_name} must be a list{' or None' if is_optional else ''}"
            )

        # Build maps from registry
        registry = selection_class._registry
        discriminators = sorted(registry.keys())
        wrapper_types = tuple(registry.values())

        # Validate Selection classes have exactly one field
        for disc, wrapper_cls in registry.items():
            fields = list(wrapper_cls.model_fields.keys())
            if len(fields) != 1:
                raise TypeError(
                    f"{wrapper_cls.__name__} must have exactly one field, got {len(fields)}"
                )

        # Map config types to their discriminators
        config_to_discriminator = {}
        for disc, wrapper_cls in registry.items():
            field_type = wrapper_cls.model_fields[disc].annotation
            config_to_discriminator[field_type] = disc

        wrapped = []
        for idx, item in enumerate(value):
            # Handle already-validated Selection instances
            if isinstance(item, wrapper_types):
                wrapped.append(item)
                continue

            # Handle bare Config instances (Style 3)
            if isinstance(item, DakotaBaseModel):
                item_type = type(item)
                if item_type in config_to_discriminator:
                    discriminator = config_to_discriminator[item_type]
                    wrapped.append({discriminator: item})
                    continue
                else:
                    raise ValueError(
                        f"{field_name}[{idx}]: invalid type {item_type.__name__}. "
                        f"Expected: dict, *Config instance, or *Selection instance"
                    )

            # Handle dict specifications (Style 1)
            if isinstance(item, dict):
                if len(item) == 0:
                    raise ValueError(
                        f"{field_name}[{idx}]: empty dict not allowed. "
                        f"Must specify exactly one type: {', '.join(discriminators)}"
                    )

                if len(item) > 1:
                    provided = ", ".join(f"'{k}'" for k in item.keys())
                    raise ValueError(
                        f"{field_name}[{idx}]: multiple types provided: {provided}. "
                        f"Must specify exactly one type."
                    )

                discriminator = list(item.keys())[0]
                if discriminator not in registry:
                    raise ValueError(
                        f"{field_name}[{idx}]: unknown type '{discriminator}'. "
                        f"Valid types: {', '.join(discriminators)}"
                    )

                # Validate with appropriate Selection class
                wrapper_cls = registry[discriminator]
                try:
                    validated = wrapper_cls(**item)
                    wrapped.append(validated)
                except Exception as e:
                    raise ValueError(
                        f"{field_name}[{idx}]: validation failed for '{discriminator}': {str(e)}"
                    )
                continue

            # Invalid type
            raise ValueError(
                f"{field_name}[{idx}]: invalid type {type(item).__name__}. "
                f"Expected: dict, *Config instance, or *Selection instance"
            )

        return wrapped

    @field_validator("method", mode="before")
    @classmethod
    def wrap_method_configs(cls, value):
        """Auto-wrap bare Config instances and validate method specifications"""
        from .method import MethodSelection

        return cls._validate_selection_list(value, "method", MethodSelection)

    @field_validator("model", mode="before")
    @classmethod
    def wrap_model_configs(cls, value):
        """Auto-wrap bare Config instances and validate model specifications"""
        from .model import ModelSelection

        return cls._validate_selection_list(
            value, "model", ModelSelection, is_optional=True
        )

    @model_validator(mode="after")
    def validate_block_pointer_references(self) -> "DakotaStudy":
        """Validate that all block pointer references point to existing blocks.

        Scans the entire study tree for fields with x-block-pointer schema annotation
        and verifies that referenced IDs exist in the corresponding section.
        """
        # Collect all IDs from each section
        # IDs are always on the Config objects (e.g., id_method on MethodConfig)
        available_ids = {
            "method": set(),
            "model": set(),
            "interface": set(),
            "variables": set(),
            "responses": set(),
        }

        # ID field names for each section
        id_field_map = {
            "method": "id_method",
            "model": "id_model",
            "interface": "id_interface",
            "variables": "id_variables",
            "responses": "id_responses",
        }

        # Collect IDs from each section's Config objects
        for section_name, id_field in id_field_map.items():
            section_data = getattr(self, section_name, None)
            if section_data is None:
                continue
            items = section_data if isinstance(section_data, list) else [section_data]
            for item in items:
                # For Selection wrappers, get the inner Config
                config = self._get_config_from_item(item)
                if config is not None and hasattr(config, id_field):
                    id_value = getattr(config, id_field)
                    if id_value is not None:
                        available_ids[section_name].add(id_value)

        # Validate all references
        errors = []
        for section_name in [
            "environment",
            "method",
            "model",
            "interface",
            "variables",
            "responses",
        ]:
            section_data = getattr(self, section_name, None)
            if section_data is None:
                continue
            items = section_data if isinstance(section_data, list) else [section_data]
            for idx, item in enumerate(items):
                path = (
                    f"{section_name}[{idx}]"
                    if isinstance(section_data, list)
                    else section_name
                )
                self._validate_block_pointer_refs(item, path, available_ids, errors)

        if errors:
            raise ValueError("Invalid block pointer references:\n" + "\n".join(errors))

        return self

    def _get_config_from_item(self, item: Any) -> Any:
        """Extract the Config object from a Selection wrapper or return the item itself."""
        if item is None:
            return None
        # Selection wrappers have exactly one field containing the Config
        if hasattr(item, "model_fields") and len(item.model_fields) == 1:
            field_name = list(item.model_fields.keys())[0]
            return getattr(item, field_name, None)
        return item

    def _validate_block_pointer_refs(
        self, obj: Any, path: str, available_ids: dict, errors: list
    ) -> None:
        """Recursively validate block pointer references in an object tree."""
        if obj is None:
            return

        if isinstance(obj, DakotaBaseModel):
            # Get the model's schema to check for x-block-pointer annotations
            schema = obj.model_json_schema()
            properties = schema.get("properties", {})

            # Also check $defs for nested schemas
            defs = schema.get("$defs", {})

            for field_name, field_info in obj.model_fields.items():
                field_value = getattr(obj, field_name, None)
                if field_value is None:
                    continue

                field_path = f"{path}.{field_name}"

                # Check if this field has x-block-pointer in schema
                field_schema = properties.get(field_name, {})

                # Handle $ref - resolve from $defs
                if "$ref" in field_schema:
                    ref_name = field_schema["$ref"].split("/")[-1]
                    field_schema = defs.get(ref_name, {})

                block_type = field_schema.get("x-block-pointer")

                if block_type:
                    # Validate the reference(s)
                    if isinstance(field_value, str):
                        if field_value not in available_ids.get(block_type, set()):
                            errors.append(
                                f"{field_path}: references '{field_value}' but no {block_type} "
                                f"with id_{block_type}='{field_value}' exists"
                            )
                    elif isinstance(field_value, list):
                        for i, ref in enumerate(field_value):
                            if isinstance(ref, str) and ref not in available_ids.get(
                                block_type, set()
                            ):
                                errors.append(
                                    f"{field_path}[{i}]: references '{ref}' but no {block_type} "
                                    f"with id_{block_type}='{ref}' exists"
                                )

                # Recurse into nested objects
                if isinstance(field_value, (DakotaBaseModel, list)):
                    self._validate_block_pointer_refs(
                        field_value, field_path, available_ids, errors
                    )

        elif isinstance(obj, list):
            for idx, item in enumerate(obj):
                self._validate_block_pointer_refs(
                    item, f"{path}[{idx}]", available_ids, errors
                )
