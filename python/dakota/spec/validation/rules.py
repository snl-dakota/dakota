"""Concrete validation rule implementations for Dakota models.

This module provides reusable validation rules for common patterns:
- DefaultBoundsReal/Int: Set default lower/upper bounds when not provided
- CompareLength: Validate list length equals a count field
- CompareLengthOne: Validate list length equals 1 or count (with auto-expansion)
- DefaultInitialPointReal/Int: Set/clamp initial_point based on bounds
- CheckNonnegativeList: Validate all list elements >= 0
- CheckPositiveList: Validate all list elements > 0
- CheckProbabilityList: Validate all list elements in [0, 1]
- CheckRealLowerBound: Validate all list elements > a bound
- CheckRealUpperBound: Validate all list elements < a bound
- CheckIntLowerBound: Validate all integer list elements > a bound
- CheckSumEqualsLength: Validate sum of one list equals length of another
- TrustRegionValidate: Cross-field validation for trust region parameters
- CheckInterfaceBlock: Validate interface block parameters

The actual validation logic is implemented in C++ (dakota_validation_cpp module)
and called via pybind11 bindings. This ensures consistent behavior between
Python/Pydantic validation and C++ DSL reader validation.

All validators use a standardized signature matching the JSON schema export:
    validator(instance, fields, literals, context) -> mutations
"""

from typing import Any, List
from .base import ValidationRule

# Try to import C++ backend; fall back to pure Python if unavailable
try:
    from . import dakota_validation_cpp as _cpp

    _USE_CPP = True
except ImportError:
    _USE_CPP = False
    import warnings

    warnings.warn(
        "dakota_validation_cpp not found, using pure Python fallback. "
        "Install the C++ module for better performance and consistency.",
        RuntimeWarning,
    )


def _call_cpp_validator(func, instance_dict, fields, literals, context):
    """Call a C++ validator function and convert RuntimeError to ValueError.

    On macOS, C++ exceptions thrown across shared library boundaries may not
    be properly translated by pybind11. This wrapper catches RuntimeError
    (which is how they appear in Python) and re-raises as ValueError.
    """
    try:
        return func(instance_dict, fields, literals, context)
    except RuntimeError as e:
        raise ValueError(str(e)) from None


def _apply_mutations(instance: Any, mutations: dict) -> None:
    """Apply field mutations to a Pydantic model instance.

    Uses object.__setattr__ to bypass Pydantic's __setattr__ hook,
    which would trigger re-validation and cause infinite recursion.

    Supports dotted paths for nested field mutations:
        {"child.value": 42} sets instance.child.value = 42
        {"a.b.c": [1,2,3]} sets instance.a.b.c = [1,2,3]

    Constraints:
        - All intermediate path components must be BaseModel instances
        - The leaf field must NOT be a BaseModel type (primitives/lists only)
        - You cannot replace an entire nested model, only its primitive fields

    Raises:
        RuntimeError: If a mutation path is invalid, targets an unknown field,
                      or attempts to replace a nested model instance.
    """
    if not mutations:
        return

    for field_path, value in mutations.items():
        _apply_single_mutation(instance, field_path, value)


def _apply_single_mutation(instance: Any, field_path: str, value: Any) -> None:
    """Apply a single mutation, potentially to a nested field.

    Args:
        instance: The root Pydantic model instance
        field_path: Dotted path to the field (e.g., "child.value" or "a.b.c")
        value: The value to set

    Raises:
        RuntimeError: If the path is invalid or targets a model type
    """
    from typing import get_origin, get_args, Union
    from pydantic import BaseModel

    components = field_path.split(".")

    # Navigate to the parent of the target field
    current = instance
    path_so_far = []

    for component in components[:-1]:
        path_so_far.append(component)
        model_fields = current.__class__.model_fields

        # Check that the component exists on current model
        if component not in model_fields:
            raise RuntimeError(
                f"Cannot apply mutation to '{field_path}': "
                f"field '{component}' not found on {current.__class__.__name__}. "
                f"Valid fields are: {', '.join(model_fields.keys())}"
            )

        next_obj = getattr(current, component)

        # Check that it's a BaseModel instance (required for further navigation)
        if not isinstance(next_obj, BaseModel):
            if next_obj is None:
                raise RuntimeError(
                    f"Cannot apply mutation to '{field_path}': "
                    f"intermediate field '{'.'.join(path_so_far)}' is None"
                )
            raise RuntimeError(
                f"Cannot apply mutation to '{field_path}': "
                f"intermediate field '{'.'.join(path_so_far)}' is not a model instance "
                f"(type: {type(next_obj).__name__}). Cannot navigate through non-model fields."
            )

        current = next_obj

    # Now 'current' is the parent of the leaf field
    leaf_field = components[-1]
    model_fields = current.__class__.model_fields

    # Check that the leaf field exists
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

    # Unwrap Optional/Union to get the actual type(s)
    if origin is Union:
        types = [t for t in get_args(annotation) if t is not type(None)]
    else:
        types = [annotation]

    for t in types:
        # Check if it's a BaseModel subclass (but not BaseModel itself)
        if isinstance(t, type) and issubclass(t, BaseModel) and t is not BaseModel:
            raise RuntimeError(
                f"Cannot apply mutation to '{field_path}': "
                f"target field '{leaf_field}' is a nested model type ({t.__name__}). "
                "Mutations must target primitive or list fields, not model instances."
            )

    # Apply the mutation
    object.__setattr__(current, leaf_field, value)


def _resolve_path(instance: Any, path: str) -> Any:
    """Resolve a dotted path on a Pydantic model instance.

    Args:
        instance: Pydantic model instance
        path: Dotted path like "field" or "child.field" or "a.b.c"

    Returns:
        The value at the path, or None if any component is None/missing
    """
    components = path.split(".")
    current = instance

    for component in components:
        if current is None:
            return None
        try:
            current = getattr(current, component)
        except AttributeError:
            return None

    return current


def _path_exists(instance: Any, path: str) -> bool:
    """Check if a dotted path exists and is not None."""
    return _resolve_path(instance, path) is not None


class DefaultBoundsReal(ValidationRule):
    """Set default lower/upper bounds when not provided (real-valued).

    When lower_bounds is None, sets it to [L0, L0, ...] of length count.
    When upper_bounds is None, sets it to [L1, L1, ...] of length count.

    This rule should run BEFORE CompareLength rules for bounds.

    Fields: [] (lower_bounds, upper_bounds, count are hardcoded)
    Literals: [lower_default, upper_default] (as floats)
    """

    def __init__(self, context: str, literals: List[Any]):
        super().__init__(
            context=context,
            rule_name="default_bounds_real",
            fields=[],
            literals=literals,
            error_message="Bounds setup failed: lower_bounds or upper_bounds could not be set.",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            mutations = _call_cpp_validator(
                _cpp.default_bounds_real,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        required_length = instance.count
        lower_default = float(self.literals[0])
        upper_default = float(self.literals[1])

        if instance.lower_bounds is None:
            object.__setattr__(
                instance, "lower_bounds", [lower_default] * required_length
            )
        if instance.upper_bounds is None:
            object.__setattr__(
                instance, "upper_bounds", [upper_default] * required_length
            )


class DefaultBoundsInt(ValidationRule):
    """Set default lower/upper bounds when not provided (integer-valued).

    When lower_bounds is None, sets it to [L0, L0, ...] of length count.
    When upper_bounds is None, sets it to [L1, L1, ...] of length count.

    This rule should run BEFORE CompareLength rules for bounds.

    Fields: [] (lower_bounds, upper_bounds, count are hardcoded)
    Literals: [lower_default, upper_default] (as ints)
    """

    def __init__(self, context: str, literals: List[Any]):
        super().__init__(
            context=context,
            rule_name="default_bounds_int",
            fields=[],
            literals=literals,
            error_message="Bounds setup failed: lower_bounds or upper_bounds could not be set.",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            mutations = _call_cpp_validator(
                _cpp.default_bounds_int,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        required_length = instance.count
        lower_default = int(self.literals[0])
        upper_default = int(self.literals[1])

        if instance.lower_bounds is None:
            object.__setattr__(
                instance, "lower_bounds", [lower_default] * required_length
            )
        if instance.upper_bounds is None:
            object.__setattr__(
                instance, "upper_bounds", [upper_default] * required_length
            )


class CompareLength(ValidationRule):
    """Validate that a list field has length equal to a target field.

    Skips validation if the list field is None.
    Supports dotted paths for accessing nested model fields (e.g., "child.count").

    Fields: [list_field, target_field]
    Literals: []

    Args:
        context: Validation context (e.g., "continuous_design")
        list_field: Name/path of the list field to check
        target_field: Name/path of the field containing the required length
    """

    def __init__(self, context: str, list_field: str, target_field: str):
        super().__init__(
            context=context,
            rule_name="compare_len",
            fields=[list_field, target_field],
            literals=[],
            error_message="For {context}, length of {F0} must equal {F1}.",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            mutations = _call_cpp_validator(
                _cpp.compare_length,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        list_field, target_field = self.fields
        list_value = _resolve_path(instance, list_field)
        required_length = _resolve_path(instance, target_field)

        if list_value is None:
            return

        if required_length is None:
            raise ValueError(f"target field '{target_field}' not found")

        if len(list_value) != required_length:
            raise ValueError(f"({len(list_value)} != {required_length})")


class CompareListLengths(ValidationRule):
    """Validate that two list fields have equal length.

    Skips validation if either field is None.
    Supports dotted paths for accessing nested model fields.

    Fields: [list_a, list_b]
    Literals: []

    Args:
        context: Validation context
        list_a: Name/path of the first list field
        list_b: Name/path of the second list field
    """

    def __init__(self, context: str, list_a: str, list_b: str):
        super().__init__(
            context=context,
            rule_name="compare_list_len",
            fields=[list_a, list_b],
            literals=[],
            error_message="For {context}, length of {F0} must equal length of {F1}.",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            _call_cpp_validator(
                _cpp.compare_list_lengths,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        list_a_field, list_b_field = self.fields
        list_a = _resolve_path(instance, list_a_field)
        list_b = _resolve_path(instance, list_b_field)

        if list_a is None or list_b is None:
            return

        if len(list_a) != len(list_b):
            raise ValueError(
                f"length of {list_a_field} ({len(list_a)}) does not equal "
                f"length of {list_b_field} ({len(list_b)})"
            )


class CompareLengthOne(ValidationRule):
    """Validate that a list field has length equal to 1 or a target field.

    If the list has length 1, it is automatically expanded to the target length.
    Skips validation if the list field is None.
    Supports dotted paths for both list_field and target_field.

    Fields: [list_field, target_field]
    Literals: []

    Args:
        context: Validation context (e.g., "continuous_design")
        list_field: Name/path of the list field to check
        target_field: Name/path of the field containing the required length
    """

    def __init__(self, context: str, list_field: str, target_field: str):
        super().__init__(
            context=context,
            rule_name="compare_len1",
            fields=[list_field, target_field],
            literals=[],
            error_message="For {context}, length of {F0} must equal 1 or {F1}.",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            mutations = _call_cpp_validator(
                _cpp.compare_length_one,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        list_field, target_field = self.fields
        list_value = _resolve_path(instance, list_field)
        required_length = _resolve_path(instance, target_field)

        if list_value is None:
            return

        if required_length is None:
            raise ValueError(f"target field '{target_field}' not found")

        if len(list_value) == 1:
            # Expand single value to required length
            # Use _apply_single_mutation to handle nested paths
            _apply_single_mutation(instance, list_field, list_value * required_length)
            return

        if len(list_value) != required_length:
            raise ValueError(f"({len(list_value)} != {required_length})")


class DefaultInitialPointReal(ValidationRule):
    """Set default initial point/state to midpoint of bounds, or clamp existing values (real-valued).

    If the target field is None:
        - Sets each element to midpoint of (lower_bound, upper_bound)
        - If one bound is infinite, uses the finite bound
        - If both bounds are infinite, uses 0.0

    If the target field is provided:
        - Clamps each value to be within [lower_bound, upper_bound]

    This rule should run AFTER DefaultBoundsReal.

    Fields: [target_field] (lower_bounds, upper_bounds, count are hardcoded)
    Literals: []

    Args:
        context: Validation context (e.g., "continuous_design")
        target_field: Name of the field to set (e.g., "initial_point" or "initial_state")
    """

    def __init__(self, context: str, target_field: str = "initial_point"):
        super().__init__(
            context=context,
            rule_name="default_initial_point_real",
            fields=[target_field],
            literals=[],
            error_message=f"For {{context}}, {target_field} must be within bounds.",
        )
        self._target_field = target_field

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            mutations = _call_cpp_validator(
                _cpp.default_initial_point_real,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        from math import isfinite

        required_length = instance.count
        lower_bounds = instance.lower_bounds
        upper_bounds = instance.upper_bounds
        target_value = getattr(instance, self._target_field)

        if target_value is None:
            # Compute midpoint of bounds
            initial_point = []
            for lb, ub in zip(lower_bounds, upper_bounds):
                if isfinite(lb) and isfinite(ub):
                    initial_point.append((lb + ub) / 2.0)
                elif isfinite(lb):
                    initial_point.append(lb)
                elif isfinite(ub):
                    initial_point.append(ub)
                else:
                    initial_point.append(0.0)
            object.__setattr__(instance, self._target_field, initial_point)
        else:
            # Clamp existing values to bounds
            initial_point = list(target_value)  # Make a copy
            changed = False
            for i in range(required_length):
                if initial_point[i] < lower_bounds[i]:
                    initial_point[i] = lower_bounds[i]
                    changed = True
                elif initial_point[i] > upper_bounds[i]:
                    initial_point[i] = upper_bounds[i]
                    changed = True
            if changed:
                object.__setattr__(instance, self._target_field, initial_point)


class DefaultInitialPointInt(ValidationRule):
    """Set default initial point/state to midpoint of bounds, or clamp existing values (integer-valued).

    If the target field is None:
        - Sets each element to integer midpoint of (lower_bound, upper_bound)

    If the target field is provided:
        - Clamps each value to be within [lower_bound, upper_bound]

    This rule should run AFTER DefaultBoundsInt.

    Fields: [target_field] (lower_bounds, upper_bounds, count are hardcoded)
    Literals: []

    Args:
        context: Validation context (e.g., "discrete_design_range")
        target_field: Name of the field to set (e.g., "initial_point" or "initial_state")
    """

    def __init__(self, context: str, target_field: str = "initial_point"):
        super().__init__(
            context=context,
            rule_name="default_initial_point_int",
            fields=[target_field],
            literals=[],
            error_message=f"For {{context}}, {target_field} must be within bounds.",
        )
        self._target_field = target_field

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            mutations = _call_cpp_validator(
                _cpp.default_initial_point_int,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        required_length = instance.count
        lower_bounds = instance.lower_bounds
        upper_bounds = instance.upper_bounds
        target_value = getattr(instance, self._target_field)
        int_min = -(2**31)
        int_max = 2**31 - 1

        if target_value is None:
            # Mirror Dakota's Vgen_DiscreteDesRange / Vgen_DiscreteStateRange:
            # midpoint only when both bounds are finite sentinels, otherwise
            # fall back to the bounded side or 0.
            initial_point = []
            for lb, ub in zip(lower_bounds, upper_bounds):
                if int_min < lb and ub < int_max:
                    initial_point.append(lb + (ub - lb) // 2)
                elif lb > 0:
                    initial_point.append(lb)
                elif ub < 0:
                    initial_point.append(ub)
                else:
                    initial_point.append(0)
            object.__setattr__(instance, self._target_field, initial_point)
        else:
            # Clamp existing values to bounds
            initial_point = list(target_value)  # Make a copy
            changed = False
            for i in range(required_length):
                if initial_point[i] < lower_bounds[i]:
                    initial_point[i] = lower_bounds[i]
                    changed = True
                elif initial_point[i] > upper_bounds[i]:
                    initial_point[i] = upper_bounds[i]
                    changed = True
            if changed:
                object.__setattr__(instance, self._target_field, initial_point)


# ============================================================================
# List Element Validators
# ============================================================================


class CheckNonnegativeList(ValidationRule):
    """Validate that all elements in a list are >= 0.

    Skips validation if the list field is None.

    Fields: [list_field]
    Literals: []

    Used by keyword handlers: method_usharray, method_szarray, model_usharray

    Args:
        context: Validation context
        list_field: Name of the list field to check
    """

    def __init__(self, context: str, list_field: str):
        super().__init__(
            context=context,
            rule_name="check_nonnegative_list",
            fields=[list_field],
            literals=[],
            error_message="For {context}, all elements of {F0} must be >= 0.",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            _call_cpp_validator(
                _cpp.check_nonnegative_list,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        list_field = self.fields[0]
        list_value = _resolve_path(instance, list_field)

        if list_value is None:
            return

        for i, elem in enumerate(list_value):
            if elem < 0:
                raise ValueError(f"element {i} is negative")


class CheckPositiveList(ValidationRule):
    """Validate that all elements in a list are > 0.

    Skips validation if the list field is None.

    Fields: [list_field]
    Literals: []

    Used by keyword handlers: model_id_index_set

    Args:
        context: Validation context
        list_field: Name of the list field to check
    """

    def __init__(self, context: str, list_field: str):
        super().__init__(
            context=context,
            rule_name="check_positive_list",
            fields=[list_field],
            literals=[],
            error_message="For {context}, all elements of {F0} must be > 0.",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            _call_cpp_validator(
                _cpp.check_positive_list,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        list_field = self.fields[0]
        list_value = _resolve_path(instance, list_field)

        if list_value is None:
            return

        for i, elem in enumerate(list_value):
            if elem <= 0:
                raise ValueError(f"element {i} is not positive")


class CheckProbabilityList(ValidationRule):
    """Validate that all elements in a list are in [0, 1].

    Skips validation if the list field is None.

    Fields: [list_field]
    Literals: []

    Used by keyword handlers: method_resplevels01

    Args:
        context: Validation context
        list_field: Name of the list field to check
    """

    def __init__(self, context: str, list_field: str):
        super().__init__(
            context=context,
            rule_name="check_probability_list",
            fields=[list_field],
            literals=[],
            error_message="For {context}, all elements of {F0} must be in [0, 1].",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            _call_cpp_validator(
                _cpp.check_probability_list,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        list_field = self.fields[0]
        list_value = _resolve_path(instance, list_field)

        if list_value is None:
            return

        for i, elem in enumerate(list_value):
            if elem < 0.0 or elem > 1.0:
                raise ValueError(f"element {i} ({elem}) is not in [0, 1]")


class CheckRealLowerBound(ValidationRule):
    """Validate that all elements in a real list are above a bound.

    Skips validation if the list field is None.

    Fields: [list_field]
    Literals: [lower_bound, inclusive]

    Used by keyword handlers: var_RealLb

    Args:
        context: Validation context
        list_field: Name of the list field to check
        lower_bound: The lower bound
        inclusive: If False (default), checks > lower_bound.
                   If True, checks >= lower_bound.
    """

    def __init__(
        self, context: str, list_field: str, lower_bound: float, inclusive: bool = False
    ):
        op = ">=" if inclusive else ">"
        super().__init__(
            context=context,
            rule_name="check_real_lower_bound",
            fields=[list_field],
            literals=[lower_bound, inclusive],
            error_message=f"For {{context}}, all elements of {{F0}} must be {op} {lower_bound}.",
        )
        self._inclusive = inclusive

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            _call_cpp_validator(
                _cpp.check_real_lower_bound,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        list_field = self.fields[0]
        lower_bound = float(self.literals[0])
        inclusive = self._inclusive
        list_value = _resolve_path(instance, list_field)

        if list_value is None:
            return

        for i, elem in enumerate(list_value):
            if inclusive:
                if elem < lower_bound:
                    raise ValueError(f"element {i} ({elem}) is not >= {lower_bound}")
            else:
                if elem <= lower_bound:
                    raise ValueError(f"element {i} ({elem}) is not > {lower_bound}")


class CheckRealUpperBound(ValidationRule):
    """Validate that all elements in a real list are below a bound.

    Skips validation if the list field is None.

    Fields: [list_field]
    Literals: [upper_bound, inclusive]

    Used by keyword handlers: var_RealUb

    Args:
        context: Validation context
        list_field: Name of the list field to check
        upper_bound: The upper bound
        inclusive: If False (default), checks < upper_bound.
                   If True, checks <= upper_bound.
    """

    def __init__(
        self, context: str, list_field: str, upper_bound: float, inclusive: bool = False
    ):
        op = "<=" if inclusive else "<"
        super().__init__(
            context=context,
            rule_name="check_real_upper_bound",
            fields=[list_field],
            literals=[upper_bound, inclusive],
            error_message=f"For {{context}}, all elements of {{F0}} must be {op} {upper_bound}.",
        )
        self._inclusive = inclusive

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            _call_cpp_validator(
                _cpp.check_real_upper_bound,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        list_field = self.fields[0]
        upper_bound = float(self.literals[0])
        inclusive = self._inclusive
        list_value = _resolve_path(instance, list_field)

        if list_value is None:
            return

        for i, elem in enumerate(list_value):
            if inclusive:
                if elem > upper_bound:
                    raise ValueError(f"element {i} ({elem}) is not <= {upper_bound}")
            else:
                if elem >= upper_bound:
                    raise ValueError(f"element {i} ({elem}) is not < {upper_bound}")


class CheckIntLowerBound(ValidationRule):
    """Validate that all elements in an integer list are > a bound.

    Skips validation if the list field is None.

    Fields: [list_field]
    Literals: [lower_bound]

    Used by keyword handlers: var_IntLb

    Args:
        context: Validation context
        list_field: Name of the list field to check
        lower_bound: The exclusive lower bound
    """

    def __init__(self, context: str, list_field: str, lower_bound: int):
        super().__init__(
            context=context,
            rule_name="check_int_lower_bound",
            fields=[list_field],
            literals=[lower_bound],
            error_message=f"For {{context}}, all elements of {{F0}} must be > {lower_bound}.",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            _call_cpp_validator(
                _cpp.check_int_lower_bound,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        list_field = self.fields[0]
        lower_bound = int(self.literals[0])
        list_value = _resolve_path(instance, list_field)

        if list_value is None:
            return

        for i, elem in enumerate(list_value):
            if elem <= lower_bound:
                raise ValueError(f"element {i} ({elem}) is not > {lower_bound}")


# ============================================================================
# Cross-Field Validators
# ============================================================================


class CheckSumEqualsLength(ValidationRule):
    """Validate that the sum of a list equals the length of another list.

    Skips validation if either field is None.

    Fields: [num_list_field, levels_list_field]
    Literals: []

    Used by keyword handlers: method_num_resplevs

    Args:
        context: Validation context
        num_list_field: Name of the list to sum
        levels_list_field: Name of the list whose length should equal the sum
    """

    def __init__(self, context: str, num_list_field: str, levels_list_field: str):
        super().__init__(
            context=context,
            rule_name="check_sum_equals_length",
            fields=[num_list_field, levels_list_field],
            literals=[],
            error_message="For {context}, sum of {F0} must equal length of {F1}.",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            _call_cpp_validator(
                _cpp.check_sum_equals_length,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        num_list_field, levels_list_field = self.fields
        num_list = _resolve_path(instance, num_list_field)
        levels_list = _resolve_path(instance, levels_list_field)

        if num_list is None or levels_list is None:
            return

        total = sum(num_list)
        levels_length = len(levels_list)

        if total != levels_length:
            raise ValueError(
                f"sum of {num_list_field} ({total}) does not equal "
                f"length of {levels_list_field} ({levels_length})"
            )


class TrustRegionValidate(ValidationRule):
    """Validate trust_region parameters.

    Checks:
    - Each initial_size in (0, 1]
    - Each initial_size >= minimum_size
    - minimum_size in [0, 1]
    - 0 < contract_threshold <= expand_threshold <= 1
    - contraction_factor in (0, 1]
    - expansion_factor >= 1

    Fields: [initial_size_field, minimum_size_field, contract_threshold_field,
             expand_threshold_field, contraction_factor_field, expansion_factor_field]
    Literals: []

    Used by keyword handlers: method_tr_final

    Args:
        context: Validation context
        initial_size: Field name for initial size list
        minimum_size: Field name for minimum size scalar
        contract_threshold: Field name for contract threshold scalar
        expand_threshold: Field name for expand threshold scalar
        contraction_factor: Field name for contraction factor scalar
        expansion_factor: Field name for expansion factor scalar
    """

    def __init__(
        self,
        context: str,
        initial_size: str = "initial_size",
        minimum_size: str = "minimum_size",
        contract_threshold: str = "contract_threshold",
        expand_threshold: str = "expand_threshold",
        contraction_factor: str = "contraction_factor",
        expansion_factor: str = "expansion_factor",
    ):
        super().__init__(
            context=context,
            rule_name="trust_region_validate",
            fields=[
                initial_size,
                minimum_size,
                contract_threshold,
                expand_threshold,
                contraction_factor,
                expansion_factor,
            ],
            literals=[],
            error_message="For {context}, trust region parameters are invalid.",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            _call_cpp_validator(
                _cpp.trust_region_validate,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        (
            initial_size_field,
            minimum_size_field,
            contract_threshold_field,
            expand_threshold_field,
            contraction_factor_field,
            expansion_factor_field,
        ) = self.fields

        # Get minimum_size (default 0 if missing)
        minimum_size = _resolve_path(instance, minimum_size_field)
        if minimum_size is None:
            minimum_size = 0.0
        else:
            if minimum_size < 0.0 or minimum_size > 1.0:
                raise ValueError(
                    f"{minimum_size_field} ({minimum_size}) must be in [0, 1]"
                )

        # Validate initial_size list
        initial_sizes = _resolve_path(instance, initial_size_field)
        if initial_sizes is not None:
            for i, val in enumerate(initial_sizes):
                if val <= 0.0 or val > 1.0:
                    raise ValueError(
                        f"{initial_size_field}[{i}] ({val}) must be in (0, 1]"
                    )
                if val < minimum_size:
                    raise ValueError(
                        f"{initial_size_field}[{i}] ({val}) must be >= "
                        f"{minimum_size_field} ({minimum_size})"
                    )

        # Get thresholds (defaults)
        contract_threshold = _resolve_path(instance, contract_threshold_field)
        if contract_threshold is None:
            contract_threshold = 0.0  # Will fail the > 0 check if actually used
        expand_threshold = _resolve_path(instance, expand_threshold_field)
        if expand_threshold is None:
            expand_threshold = 1.0

        # Check 0 < contract_threshold <= expand_threshold <= 1
        if contract_threshold <= 0.0:
            raise ValueError(
                f"{contract_threshold_field} ({contract_threshold}) must be > 0"
            )
        if contract_threshold > expand_threshold:
            raise ValueError(
                f"{contract_threshold_field} ({contract_threshold}) must be <= "
                f"{expand_threshold_field} ({expand_threshold})"
            )
        if expand_threshold > 1.0:
            raise ValueError(
                f"{expand_threshold_field} ({expand_threshold}) must be <= 1"
            )

        # Validate contraction_factor in (0, 1]
        contraction_factor = _resolve_path(instance, contraction_factor_field)
        if contraction_factor is not None:
            if contraction_factor <= 0.0 or contraction_factor > 1.0:
                raise ValueError(
                    f"{contraction_factor_field} ({contraction_factor}) must be in (0, 1]"
                )

        # Validate expansion_factor >= 1
        expansion_factor = _resolve_path(instance, expansion_factor_field)
        if expansion_factor is not None:
            if expansion_factor < 1.0:
                raise ValueError(
                    f"{expansion_factor_field} ({expansion_factor}) must be >= 1"
                )


class CheckInterfaceBlock(ValidationRule):
    """Validate interface parameters."""

    def __init__(self, context: str):
        super().__init__(
            context=context,
            rule_name="check_interface_block",
            fields=[],
            literals=[],
            error_message="For interface, some block parameters are invalid",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            _call_cpp_validator(
                _cpp.check_interface_block,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        import sys

        # Check for analysis_drivers and count them
        analysis_drivers = _resolve_path(instance, "analysis_drivers")
        num_analysis_drivers = 0
        if analysis_drivers is not None:
            drivers = _resolve_path(instance, "analysis_drivers.drivers")
            if drivers is not None:
                num_analysis_drivers = len(drivers)

        input_filter = _path_exists(instance, "analysis_drivers.input_filter")
        output_filter = _path_exists(instance, "analysis_drivers.output_filter")

        # Batch concurrency validation
        if _path_exists(instance, "concurrency.batch"):
            if num_analysis_drivers > 1:
                raise ValueError(
                    "Batch concurrency is not supported with multiple analysis drivers."
                )
            if input_filter:
                raise ValueError(
                    "Batch concurrency is not supported with input filters."
                )
            if output_filter:
                raise ValueError(
                    "Batch concurrency is not supported with output filters."
                )

            batch_size = _resolve_path(instance, "concurrency.batch.size")
            if batch_size is not None and batch_size == 1:
                raise ValueError(
                    "Batch size of 1 is not allowed; increase size or disable 'batch' concurrency."
                )

            # Validate failure_capture modes in batch mode - only abort/recover allowed
            # failure_capture is a Union, so only one mode is set at a time
            failure_capture = _resolve_path(instance, "failure_capture")
            if failure_capture is not None:
                # Get dict representation, excluding abort and recover
                if hasattr(failure_capture, "model_dump"):
                    fc_dict = failure_capture.model_dump(exclude_none=True)
                elif isinstance(failure_capture, dict):
                    fc_dict = {
                        k: v for k, v in failure_capture.items() if v is not None
                    }
                else:
                    fc_dict = {}

                fc_dict.pop("abort", None)
                fc_dict.pop("recover", None)

                for key in fc_dict:
                    raise ValueError(
                        f"failure_capture mode '{key}' disallowed in 'batch' mode"
                    )

        # Either analysis_drivers or algebraic_mappings must be present
        algebraic_mappings = _path_exists(instance, "algebraic_mappings")
        if not (analysis_drivers is not None or algebraic_mappings):
            raise ValueError(
                "Either 'analysis_drivers' or 'algebraic_mappings' block must be present."
            )

        # Asynchronous concurrency validation
        asynch = _resolve_path(instance, "concurrency.asynchronous")
        if asynch is not None:
            ec = _resolve_path(asynch, "evaluation_concurrency")
            ac = _resolve_path(asynch, "analysis_concurrency")
            has_ec = ec is not None
            has_ac = ac is not None
            ec_val = ec if has_ec else 0
            ac_val = ac if has_ac else 0
            if ec_val == 1 and ac_val == 1:
                raise ValueError(
                    "Asynchronous concurrency with both 'evaluation_concurrency' and "
                    "'analysis_concurrency' set to 1 is not allowed."
                )

        # Platform-specific: link_files not supported on Windows
        if sys.platform == "win32":
            if _path_exists(
                instance,
                "analysis_drivers.interface_type.fork.work_directory.link_files",
            ) or _path_exists(
                instance,
                "analysis_drivers.interface_type.system.work_directory.link_files",
            ):
                raise ValueError("'link_files' is not supported on Windows platforms.")


class CheckAnalysisDrivers(ValidationRule):
    """Validate analysis_drivers-specific constraints."""

    def __init__(self, context: str):
        super().__init__(
            context=context,
            rule_name="check_analysis_drivers",
            fields=[],
            literals=[],
            error_message="For analysis_drivers, some block parameters are invalid",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            _call_cpp_validator(
                _cpp.check_analysis_drivers,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        drivers = _resolve_path(instance, "drivers")
        if drivers is None or len(drivers) == 0:
            raise ValueError(
                "analysis_drivers.drivers must contain at least one driver"
            )

        analysis_components = _resolve_path(instance, "analysis_components")
        if analysis_components is None:
            return

        if len(analysis_components) % len(drivers) != 0:
            raise ValueError(
                "number of analysis_components must be evenly divisible by number of analysis_drivers"
            )


# ============================================================================
# Response Block Validators
# Based on make_response_defaults() from NIDRProblemDescDB.cpp
# ============================================================================


def _get_attr_or_default(instance: Any, field: str, default: Any) -> Any:
    """Get attribute value or return default if None/missing."""
    val = getattr(instance, field, None)
    return default if val is None else val


def _build_labels(prefix: str, start: int, count: int) -> List[str]:
    """Build numbered labels with a prefix.

    For count=1, returns [prefix] (no number suffix, trailing underscore stripped).
    For count>1, returns [prefix1, prefix2, ...prefixN].
    """
    if count == 0:
        return []
    if count == 1:
        # Single item - no numeric suffix, strip trailing underscore
        label = prefix.rstrip("_")
        return [label]
    return [f"{prefix}{start + i + 1}" for i in range(count)]


class CheckResponseDescriptors(ValidationRule):
    """Set default response descriptors when not provided.

    This validator runs on ResponsesConfig and generates default descriptors
    based on the nested response_type Union variant.

    Generates labels based on response type:
    - calibration_terms: least_sq_term_N, nln_ineq_con_N, nln_eq_con_N
    - objective_functions: obj_fn or obj_fn_N, nln_ineq_con_N, nln_eq_con_N
    - response_functions: response_fn_N

    Fields: [] (navigates nested response_type structure)
    Literals: []
    """

    def __init__(self, context: str):
        super().__init__(
            context=context,
            rule_name="check_response_descriptors",
            fields=[],
            literals=[],
            error_message="For {context}, failed to set response descriptors.",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            mutations = _call_cpp_validator(
                _cpp.check_response_descriptors,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        # Check if descriptors already provided
        descriptors = getattr(instance, "descriptors", None)
        if descriptors is not None and len(descriptors) > 0:
            return

        labels = []
        response_type = getattr(instance, "response_type", None)
        if response_type is None:
            return

        # CalibrationTerms case: response_type.calibration_terms
        calibration_terms = _resolve_path(response_type, "calibration_terms")
        if calibration_terms is not None:
            num_cal = _get_attr_or_default(calibration_terms, "count", 0)

            ineq = _resolve_path(calibration_terms, "nonlinear_inequality_constraints")
            eq = _resolve_path(calibration_terms, "nonlinear_equality_constraints")
            num_ineq = _get_attr_or_default(ineq, "count", 0) if ineq else 0
            num_eq = _get_attr_or_default(eq, "count", 0) if eq else 0

            labels.extend(_build_labels("least_sq_term_", 0, num_cal))
            labels.extend(_build_labels("nln_ineq_con_", 0, num_ineq))
            labels.extend(_build_labels("nln_eq_con_", 0, num_eq))

        # ObjectiveFunctions case: response_type.objective_functions
        objective_functions = _resolve_path(response_type, "objective_functions")
        if objective_functions is not None:
            num_obj = _get_attr_or_default(objective_functions, "count", 0)

            ineq = _resolve_path(
                objective_functions, "nonlinear_inequality_constraints"
            )
            eq = _resolve_path(objective_functions, "nonlinear_equality_constraints")
            num_ineq = _get_attr_or_default(ineq, "count", 0) if ineq else 0
            num_eq = _get_attr_or_default(eq, "count", 0) if eq else 0

            labels.extend(_build_labels("obj_fn_", 0, num_obj))
            labels.extend(_build_labels("nln_ineq_con_", 0, num_ineq))
            labels.extend(_build_labels("nln_eq_con_", 0, num_eq))

        # ResponseFunctions case: response_type.response_functions
        response_functions = _resolve_path(response_type, "response_functions")
        if response_functions is not None:
            num_resp = _get_attr_or_default(response_functions, "count", 0)
            labels.extend(_build_labels("response_fn_", 0, num_resp))

        if labels:
            object.__setattr__(instance, "descriptors", labels)


class CheckFdGradientStepSize(ValidationRule):
    """Validate that vendor numerical gradients use single fd_step_size.

    This validator runs on ResponsesConfig and checks if the gradient_type
    is NumericalGradients or MixedGradients with vendor method_source.

    Structure from ResponseGradientsMixin:
    - gradient_type: Union[NoGradients, AnalyticGradients, MixedGradients, NumericalGradients]
    - For NumericalGradients: numerical_gradients.method_source is a Union
    - For MixedGradients: mixed_gradients.method_source is a Union
    - method_source can be MixedGradientsMethodSourceVendor or similar

    Fields: [] (navigates gradient_type structure)
    Literals: []
    """

    def __init__(self, context: str):
        super().__init__(
            context=context,
            rule_name="check_fd_gradient_step_size",
            fields=[],
            literals=[],
            error_message="For {context}, vendor numerical gradients only support a single fd_step_size.",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            _call_cpp_validator(
                _cpp.check_fd_gradient_step_size,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        gradient_type = getattr(instance, "gradient_type", None)
        if gradient_type is None:
            return

        # Check NumericalGradients case
        numerical_gradients = getattr(gradient_type, "numerical_gradients", None)
        if numerical_gradients is not None:
            method_source = getattr(numerical_gradients, "method_source", None)
            fd_step_size = getattr(numerical_gradients, "fd_step_size", None)

            if (
                self._is_vendor(method_source)
                and fd_step_size is not None
                and len(fd_step_size) > 1
            ):
                raise ValueError(
                    "vendor numerical gradients only support a single fd_step_size"
                )

        # Check MixedGradients case
        mixed_gradients = getattr(gradient_type, "mixed_gradients", None)
        if mixed_gradients is not None:
            method_source = getattr(mixed_gradients, "method_source", None)
            fd_step_size = getattr(mixed_gradients, "fd_step_size", None)

            if (
                self._is_vendor(method_source)
                and fd_step_size is not None
                and len(fd_step_size) > 1
            ):
                raise ValueError(
                    "vendor numerical gradients only support a single fd_step_size"
                )

    def _is_vendor(self, method_source: Any) -> bool:
        """Check if method_source is a vendor variant."""
        if method_source is None:
            return False
        # Check for vendor attribute (MixedGradientsMethodSourceVendor or NumericalGradientsMethodSourceVendor)
        return hasattr(method_source, "vendor")


class CheckMixedGradients(ValidationRule):
    """Validate mixed gradient ID lists cover all functions exactly once.

    This validator runs on ResponsesConfig. When gradient_type is MixedGradients,
    validates that id_analytic_gradients and id_numerical_gradients cover all
    functions exactly once.

    Structure from ResponseGradientsMixin:
    - gradient_type: Union[NoGradients, AnalyticGradients, MixedGradients, NumericalGradients]
    - For MixedGradients:
      - mixed_gradients: MixedGradientsConfig
        - id_analytic_gradients: list[int]
        - id_numerical_gradients: list[int]

    Fields: [] (navigates gradient_type structure)
    Literals: []
    """

    def __init__(self, context: str):
        super().__init__(
            context=context,
            rule_name="check_mixed_gradients",
            fields=[],
            literals=[],
            error_message="For {context}, mixed gradient ID lists must cover all functions exactly once.",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            _call_cpp_validator(
                _cpp.check_mixed_gradients,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        gradient_type = getattr(instance, "gradient_type", None)
        if gradient_type is None:
            return

        # Check if this is MixedGradients
        mixed_gradients = getattr(gradient_type, "mixed_gradients", None)
        if mixed_gradients is None:
            return  # Not mixed gradients, nothing to validate

        # Calculate total functions from nested response_type structure
        nf = self._compute_num_functions(instance)
        if nf == 0:
            return

        coverage = [0] * nf

        # Check analytic gradients
        analytic_ids = getattr(mixed_gradients, "id_analytic_gradients", None) or []
        self._check_ids(analytic_ids, nf, coverage, "id_analytic_gradients")

        # Check numerical gradients
        numerical_ids = getattr(mixed_gradients, "id_numerical_gradients", None) or []
        self._check_ids(numerical_ids, nf, coverage, "id_numerical_gradients")

        # Validate coverage
        self._check_coverage(nf, coverage, "gradient")

    def _compute_num_functions(self, instance: Any) -> int:
        """Compute total number of functions from nested ResponsesConfig structure."""
        response_type = getattr(instance, "response_type", None)
        if response_type is None:
            return 0

        # CalibrationTerms case
        calibration_terms = _resolve_path(response_type, "calibration_terms")
        if calibration_terms is not None:
            num_cal = _get_attr_or_default(calibration_terms, "count", 0)
            ineq = _resolve_path(calibration_terms, "nonlinear_inequality_constraints")
            eq = _resolve_path(calibration_terms, "nonlinear_equality_constraints")
            num_ineq = _get_attr_or_default(ineq, "count", 0) if ineq else 0
            num_eq = _get_attr_or_default(eq, "count", 0) if eq else 0
            return num_cal + num_ineq + num_eq

        # ObjectiveFunctions case
        objective_functions = _resolve_path(response_type, "objective_functions")
        if objective_functions is not None:
            num_obj = _get_attr_or_default(objective_functions, "count", 0)
            ineq = _resolve_path(
                objective_functions, "nonlinear_inequality_constraints"
            )
            eq = _resolve_path(objective_functions, "nonlinear_equality_constraints")
            num_ineq = _get_attr_or_default(ineq, "count", 0) if ineq else 0
            num_eq = _get_attr_or_default(eq, "count", 0) if eq else 0
            return num_obj + num_ineq + num_eq

        # ResponseFunctions case
        response_functions = _resolve_path(response_type, "response_functions")
        if response_functions is not None:
            return _get_attr_or_default(response_functions, "count", 0)

        return 0

    def _check_ids(
        self, ids: List[int], nf: int, coverage: List[int], what: str
    ) -> None:
        """Check ID list and update coverage."""
        for id_val in ids:
            if id_val < 1 or id_val > nf:
                raise ValueError(f"{what} values must be between 1 and {nf}")
            coverage[id_val - 1] += 1

    def _check_coverage(self, nf: int, coverage: List[int], what: str) -> None:
        """Validate all functions covered exactly once."""
        for i in range(nf):
            if coverage[i] == 0:
                raise ValueError(f"Function {i + 1} missing from mixed {what} lists")
            elif coverage[i] > 1:
                raise ValueError(f"Function {i + 1} replicated in mixed {what} lists")


class CheckMixedHessians(ValidationRule):
    """Validate mixed Hessian ID lists cover all functions exactly once.

    This validator runs on ResponsesConfig. When hessian_type is MixedHessians,
    validates that id_analytic_hessians, id_numerical_hessians, and id_quasi_hessians
    cover all functions exactly once.

    Structure from ResponseHessiansMixin:
    - hessian_type: Union[NoHessians, NumericalHessians, QuasiHessians, AnalyticHessians, MixedHessians]
    - For MixedHessians:
      - mixed_hessians: MixedHessiansConfig
        - id_analytic_hessians: list[int] | None
        - id_numerical_hessians: IdNumericalHessians | None (has `values` attribute)
        - id_quasi_hessians: IdQuasiHessians | None (has `values` attribute)

    Fields: [] (navigates hessian_type structure)
    Literals: []
    """

    def __init__(self, context: str):
        super().__init__(
            context=context,
            rule_name="check_mixed_hessians",
            fields=[],
            literals=[],
            error_message="For {context}, mixed Hessian ID lists must cover all functions exactly once.",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            _call_cpp_validator(
                _cpp.check_mixed_hessians,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        hessian_type = getattr(instance, "hessian_type", None)
        if hessian_type is None:
            return

        # Check if this is MixedHessians
        mixed_hessians = getattr(hessian_type, "mixed_hessians", None)
        if mixed_hessians is None:
            return  # Not mixed hessians, nothing to validate

        # Calculate total functions from nested response_type structure (same as gradients)
        nf = self._compute_num_functions(instance)
        if nf == 0:
            return

        coverage = [0] * nf

        # Check analytic Hessians (direct list)
        analytic_ids = getattr(mixed_hessians, "id_analytic_hessians", None) or []
        self._check_ids(analytic_ids, nf, coverage, "id_analytic_hessians")

        # Check numerical Hessians (IdNumericalHessians has `values` attribute)
        numerical_hessians = getattr(mixed_hessians, "id_numerical_hessians", None)
        if numerical_hessians is not None:
            numerical_ids = getattr(numerical_hessians, "values", None) or []
            self._check_ids(numerical_ids, nf, coverage, "id_numerical_hessians")

        # Check quasi Hessians (IdQuasiHessians has `values` attribute)
        quasi_hessians = getattr(mixed_hessians, "id_quasi_hessians", None)
        if quasi_hessians is not None:
            quasi_ids = getattr(quasi_hessians, "values", None) or []
            self._check_ids(quasi_ids, nf, coverage, "id_quasi_hessians")

        # Validate coverage
        self._check_coverage(nf, coverage, "Hessian")

    def _compute_num_functions(self, instance: Any) -> int:
        """Compute total number of functions from nested ResponsesConfig structure."""
        response_type = getattr(instance, "response_type", None)
        if response_type is None:
            return 0

        # CalibrationTerms case
        calibration_terms = _resolve_path(response_type, "calibration_terms")
        if calibration_terms is not None:
            num_cal = _get_attr_or_default(calibration_terms, "count", 0)
            ineq = _resolve_path(calibration_terms, "nonlinear_inequality_constraints")
            eq = _resolve_path(calibration_terms, "nonlinear_equality_constraints")
            num_ineq = _get_attr_or_default(ineq, "count", 0) if ineq else 0
            num_eq = _get_attr_or_default(eq, "count", 0) if eq else 0
            return num_cal + num_ineq + num_eq

        # ObjectiveFunctions case
        objective_functions = _resolve_path(response_type, "objective_functions")
        if objective_functions is not None:
            num_obj = _get_attr_or_default(objective_functions, "count", 0)
            ineq = _resolve_path(
                objective_functions, "nonlinear_inequality_constraints"
            )
            eq = _resolve_path(objective_functions, "nonlinear_equality_constraints")
            num_ineq = _get_attr_or_default(ineq, "count", 0) if ineq else 0
            num_eq = _get_attr_or_default(eq, "count", 0) if eq else 0
            return num_obj + num_ineq + num_eq

        # ResponseFunctions case
        response_functions = _resolve_path(response_type, "response_functions")
        if response_functions is not None:
            return _get_attr_or_default(response_functions, "count", 0)

        return 0

    def _check_ids(
        self, ids: List[int], nf: int, coverage: List[int], what: str
    ) -> None:
        """Check ID list and update coverage."""
        for id_val in ids:
            if id_val < 1 or id_val > nf:
                raise ValueError(f"{what} values must be between 1 and {nf}")
            coverage[id_val - 1] += 1

    def _check_coverage(self, nf: int, coverage: List[int], what: str) -> None:
        """Validate all functions covered exactly once."""
        for i in range(nf):
            if coverage[i] == 0:
                raise ValueError(f"Function {i + 1} missing from mixed {what} lists")
            elif coverage[i] > 1:
                raise ValueError(f"Function {i + 1} replicated in mixed {what} lists")


class DefaultInequalityBounds(ValidationRule):
    """Set default inequality constraint bounds when not provided.

    Applied to nested constraint models like:
    - ObjectiveFunctionsNonlinearInequalityConstraints
    - CalibrationTermsNonlinearInequalityConstraints

    Sets defaults:
    - lower_bounds: -inf for each constraint
    - upper_bounds: 0.0 for each constraint (one-sided g(x) <= 0)

    Fields: [] (uses: count, lower_bounds, upper_bounds)
    Literals: []
    """

    def __init__(self, context: str):
        super().__init__(
            context=context,
            rule_name="default_inequality_bounds",
            fields=[],
            literals=[],
            error_message="For {context}, failed to set inequality constraint bounds.",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            mutations = _call_cpp_validator(
                _cpp.default_inequality_bounds,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        import math

        count = _get_attr_or_default(instance, "count", 0)
        if count == 0:
            return

        # Default lower bounds to -inf
        lower_bounds = getattr(instance, "lower_bounds", None)
        if lower_bounds is None or len(lower_bounds) == 0:
            object.__setattr__(instance, "lower_bounds", [-math.inf] * count)

        # Default upper bounds to 0.0 (one-sided g(x) <= 0)
        upper_bounds = getattr(instance, "upper_bounds", None)
        if upper_bounds is None or len(upper_bounds) == 0:
            object.__setattr__(instance, "upper_bounds", [0.0] * count)


class DefaultEqualityTargets(ValidationRule):
    """Set default equality constraint targets when not provided.

    Applied to nested constraint models like:
    - ObjectiveFunctionsNonlinearEqualityConstraints
    - CalibrationTermsNonlinearEqualityConstraints

    Sets defaults:
    - targets: 0.0 for each constraint

    Fields: [] (uses: count, targets)
    Literals: []
    """

    def __init__(self, context: str):
        super().__init__(
            context=context,
            rule_name="default_equality_targets",
            fields=[],
            literals=[],
            error_message="For {context}, failed to set equality constraint targets.",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            mutations = _call_cpp_validator(
                _cpp.default_equality_targets,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        count = _get_attr_or_default(instance, "count", 0)
        if count == 0:
            return

        # Default targets to 0.0
        targets = getattr(instance, "targets", None)
        if targets is None or len(targets) == 0:
            object.__setattr__(instance, "targets", [0.0] * count)


class CheckResponseDescriptorsLength(ValidationRule):
    """Validate that descriptors length is 0 or equals total number of functions.

    Runs on ResponsesConfig. Ensures user-provided descriptors list has
    exactly the right length to match all functions.

    Fields: [] (navigates nested response_type structure)
    Literals: []
    """

    def __init__(self, context: str):
        super().__init__(
            context=context,
            rule_name="check_response_descriptors_length",
            fields=[],
            literals=[],
            error_message="For {context}, descriptors length must match total number of responses.",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            _call_cpp_validator(
                _cpp.check_response_descriptors_length,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        descriptors = getattr(instance, "descriptors", None)
        if descriptors is None or len(descriptors) == 0:
            return  # Will be set to defaults

        # Calculate total number of functions
        nf = self._compute_num_functions(instance)

        if len(descriptors) != nf:
            raise ValueError(
                f"Number of response descriptors ({len(descriptors)}) must equal "
                f"total number of responses ({nf})"
            )

    def _compute_num_functions(self, instance: Any) -> int:
        """Compute total number of functions from nested ResponsesConfig structure."""
        response_type = getattr(instance, "response_type", None)
        if response_type is None:
            return 0

        # CalibrationTerms case
        calibration_terms = _resolve_path(response_type, "calibration_terms")
        if calibration_terms is not None:
            num_cal = _get_attr_or_default(calibration_terms, "count", 0)
            ineq = _resolve_path(calibration_terms, "nonlinear_inequality_constraints")
            eq = _resolve_path(calibration_terms, "nonlinear_equality_constraints")
            num_ineq = _get_attr_or_default(ineq, "count", 0) if ineq else 0
            num_eq = _get_attr_or_default(eq, "count", 0) if eq else 0
            return num_cal + num_ineq + num_eq

        # ObjectiveFunctions case
        objective_functions = _resolve_path(response_type, "objective_functions")
        if objective_functions is not None:
            num_obj = _get_attr_or_default(objective_functions, "count", 0)
            ineq = _resolve_path(
                objective_functions, "nonlinear_inequality_constraints"
            )
            eq = _resolve_path(objective_functions, "nonlinear_equality_constraints")
            num_ineq = _get_attr_or_default(ineq, "count", 0) if ineq else 0
            num_eq = _get_attr_or_default(eq, "count", 0) if eq else 0
            return num_obj + num_ineq + num_eq

        # ResponseFunctions case
        response_functions = _resolve_path(response_type, "response_functions")
        if response_functions is not None:
            return _get_attr_or_default(response_functions, "count", 0)

        return 0


import re

# Regex to detect if a string looks like a number
# Matches: optional sign, digits with optional decimal, optional exponent (e/E/d/D), or nan/inf
# This matches Dakota's isfloat() regex pattern
_FLOAT_REGEX = re.compile(
    r"^[+-]?[0-9]*\.?[0-9]+\.?[0-9]*[eEdD]?[+-]?[0-9]*$|^[Nn][Aa][Nn]$|^[+-]?[Ii][Nn][Ff](?:[Ii][Nn][Ii][Tt][Yy])?$"
)


def _looks_like_number(s: str) -> bool:
    """Check if a string looks like a floating-point number.

    Used to validate that descriptors aren't numeric (which would be ambiguous in Dakota input).
    This matches Dakota's isfloat() behavior.
    """
    if not s:
        return False
    return bool(_FLOAT_REGEX.match(s))


class CheckDescriptorsValid(ValidationRule):
    """Validate that descriptors follow naming rules.

    Rules:
    - Not zero-length
    - No whitespace
    - Cannot be a number (would be ambiguous in Dakota input)
    - Optionally checks for uniqueness

    kwargs:
    - field_name: str (defaults to "descriptors")
    - check_uniqueness: bool (defaults to False)
    """

    def __init__(
        self,
        context: str,
        field_name: str = "descriptors",
        check_uniqueness: bool = False,
    ):
        super().__init__(
            context=context,
            rule_name="check_descriptors_valid",
            fields=[field_name],
            literals=[check_uniqueness],
            error_message="For {context}, descriptors must be valid.",
        )
        self.field_name = field_name
        self.check_uniqueness = check_uniqueness

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            _call_cpp_validator(
                _cpp.check_descriptors_valid,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        descriptors = getattr(instance, self.field_name, None)
        if descriptors is None or len(descriptors) == 0:
            return

        seen = set()

        for i, desc in enumerate(descriptors):
            # Check for uniqueness if requested
            if self.check_uniqueness:
                if desc in seen:
                    raise ValueError(f"Duplicate descriptor '{desc}' at index {i}")
                seen.add(desc)

            # Check for empty string
            if not desc:
                raise ValueError(f"Descriptor at index {i} cannot be empty")

            # Check for whitespace
            for c in desc:
                if c.isspace():
                    raise ValueError(f"Descriptor '{desc}' contains whitespace")

            # Check if it looks like a number
            if _looks_like_number(desc):
                raise ValueError(f"Descriptor '{desc}' cannot be a number")


class CheckPermittedValues(ValidationRule):
    """Generic validator: every element of a string list must be in a permitted set.

    fields[0] = field name (e.g., "scale_types", "flags")
    literals = permitted values (e.g., ["value", "auto", "log", "none"])
    """

    def __init__(self, context: str, field_name: str, permitted_values: List[str]):
        super().__init__(
            context=context,
            rule_name="check_permitted_values",
            fields=[field_name],
            literals=permitted_values,
            error_message=f"For {context}, {field_name} must be one of {permitted_values}.",
        )
        self.field_name = field_name
        self.permitted_values = set(permitted_values)

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            _call_cpp_validator(
                _cpp.check_permitted_values,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        values = getattr(instance, self.field_name, None)
        if values is None:
            return

        for i, val in enumerate(values):
            if val.lower() not in self.permitted_values:
                valid_options = ", ".join(
                    f"'{v}'" for v in sorted(self.permitted_values)
                )
                raise ValueError(
                    f"{self.field_name}[{i}] = '{val}' is invalid; must be one of: {valid_options}"
                )


class CheckAdjacencyMatrix(ValidationRule):
    """Validate adjacency_matrix length and contents for categorical set variables.

    Checks:
    1. All entries are 0 or 1
    2. Total length equals sum(k_i^2) for each categorical variable,
       where k_i is the number of elements for that variable

    The number of categorical variables is determined by counting T/t entries
    in the flags field. If flags_path is empty, all variables are categorical
    (used for string set types which have no flags).

    k_i is taken from elements_per_variable[i] if provided, otherwise
    computed as len(elements) / count.

    Fields: [adjacency_matrix_path, flags_path, elements_per_variable_path,
             elements_path, count_path]
    Literals: []
    """

    def __init__(
        self,
        context: str,
        adjacency_matrix_path: str,
        flags_path: str,
        elements_per_variable_path: str,
        elements_path: str,
        count_path: str,
    ):
        super().__init__(
            context=context,
            rule_name="check_adjacency_matrix",
            fields=[
                adjacency_matrix_path,
                flags_path,
                elements_per_variable_path,
                elements_path,
                count_path,
            ],
            literals=[],
            error_message="For {context}, adjacency_matrix size is incorrect.",
        )
        self.adjacency_matrix_path = adjacency_matrix_path
        self.flags_path = flags_path
        self.elements_per_variable_path = elements_per_variable_path
        self.elements_path = elements_path
        self.count_path = count_path

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            _call_cpp_validator(
                _cpp.check_adjacency_matrix,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        adj = _resolve_path(instance, self.adjacency_matrix_path)
        if adj is None:
            return

        # Validate all entries are 0 or 1
        for i, val in enumerate(adj):
            if val not in (0, 1):
                raise ValueError(
                    f"{self.adjacency_matrix_path}[{i}] = {val} is invalid; "
                    "adjacency matrix entries must be 0 or 1"
                )

        count_val = _resolve_path(instance, self.count_path)
        if count_val is None or count_val <= 0:
            return

        # Determine categorical variable indices
        if self.flags_path:
            flags = _resolve_path(instance, self.flags_path)
            if flags is None:
                return
            cat_indices = [i for i, f in enumerate(flags) if f.lower()[0] in ("t", "y")]
        else:
            cat_indices = list(range(count_val))

        if not cat_indices:
            if len(adj) > 0:
                raise ValueError(
                    f"adjacency_matrix has {len(adj)} elements "
                    "but no variables are categorical"
                )
            return

        elements = _resolve_path(instance, self.elements_path)
        if elements is None:
            return

        epv = _resolve_path(instance, self.elements_per_variable_path)

        # Compute expected length
        expected = 0
        for idx in cat_indices:
            if epv is not None and len(epv) > 0:
                k = epv[idx]
            else:
                k = len(elements) // count_val
            expected += k * k

        actual = len(adj)
        if actual != expected:
            raise ValueError(
                f"adjacency_matrix has {actual} elements but expected {expected} "
                f"(sum of k*k for {len(cat_indices)} categorical variable(s))"
            )


class CheckScalesRequired(ValidationRule):
    """Generic validator: if any scale_types are 'value', scales must be provided.

    fields[0] = scale_types field name
    fields[1] = scales field name
    """

    def __init__(self, context: str, scale_types_field: str, scales_field: str):
        super().__init__(
            context=context,
            rule_name="check_scales_required",
            fields=[scale_types_field, scales_field],
            literals=[],
            error_message=f"For {context}, {scales_field} must be provided when {scale_types_field} includes 'value'.",
        )
        self.scale_types_field = scale_types_field
        self.scales_field = scales_field

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            _call_cpp_validator(
                _cpp.check_scales_required,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        scale_types = getattr(instance, self.scale_types_field, None)
        if scale_types is None:
            return

        has_value_type = any(st == "value" for st in scale_types)
        if not has_value_type:
            return

        scales = getattr(instance, self.scales_field, None)
        if scales is None or len(scales) == 0:
            raise ValueError(
                f"When {self.scale_types_field} includes 'value', {self.scales_field} must be provided"
            )


class CheckConstraintBoundsOrdering(ValidationRule):
    """Validate that lower_bounds are element-wise less than or equal to upper_bounds.

    Applied to NonlinearInequalityConstraints.

    Fields: [] (uses: lower_bounds, upper_bounds)
    Literals: []
    """

    def __init__(self, context: str):
        super().__init__(
            context=context,
            rule_name="check_constraint_bounds_ordering",
            fields=[],
            literals=[],
            error_message="For {context}, lower_bounds must not exceed upper_bounds.",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            _call_cpp_validator(
                _cpp.check_constraint_bounds_ordering,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        lower_bounds = getattr(instance, "lower_bounds", None)
        upper_bounds = getattr(instance, "upper_bounds", None)

        if lower_bounds is None or upper_bounds is None:
            return

        n = min(len(lower_bounds), len(upper_bounds))

        for i in range(n):
            lb = lower_bounds[i]
            ub = upper_bounds[i]

            # Allow equal bounds but lower must not exceed upper
            if lb > ub:
                raise ValueError(
                    f"lower_bounds[{i}] = {lb} exceeds upper_bounds[{i}] = {ub}"
                )


# ============================================================================
# Variable validators
# ============================================================================


class CheckVariableDescriptorsLength(ValidationRule):
    """Validate that descriptors length is 0 or equals count.

    Applied to individual variable type models.

    Fields: [] (uses: descriptors, count)
    Literals: []
    """

    def __init__(self, context: str):
        super().__init__(
            context=context,
            rule_name="check_variable_descriptors_length",
            fields=[],
            literals=[],
            error_message="For {context}, descriptors length must equal count.",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            _call_cpp_validator(
                _cpp.check_variable_descriptors_length,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        descriptors = getattr(instance, "descriptors", None)
        if descriptors is None or len(descriptors) == 0:
            return  # Will be set to defaults

        count = getattr(instance, "count", 0)

        if len(descriptors) != count:
            raise ValueError(
                f"Number of descriptors ({len(descriptors)}) must equal count ({count})"
            )


class DefaultVariableDescriptors(ValidationRule):
    """Generate default variable descriptors with a specified prefix.

    Format: prefix + "1", prefix + "2", etc.

    Fields: []
    Literals: [prefix] - e.g., "cdv_", "nuv_"
    """

    def __init__(self, context: str, prefix: str):
        super().__init__(
            context=context,
            rule_name="default_variable_descriptors",
            fields=[],
            literals=[prefix],
            error_message="For {context}, failed to set variable descriptors.",
        )
        self.prefix = prefix

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            mutations = _call_cpp_validator(
                _cpp.default_variable_descriptors,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        descriptors = getattr(instance, "descriptors", None)
        if descriptors is not None and len(descriptors) > 0:
            return  # Already has descriptors

        count = getattr(instance, "count", 0)
        if count == 0:
            return

        labels = [f"{self.prefix}{i}" for i in range(1, count + 1)]
        object.__setattr__(instance, "descriptors", labels)


class CheckVariableBoundsOrdering(ValidationRule):
    """Validate that lower_bounds are element-wise less than upper_bounds.

    Applied to variable types with user-provided bounds.

    Fields: [] (uses: lower_bounds, upper_bounds)
    Literals: []
    """

    def __init__(self, context: str):
        super().__init__(
            context=context,
            rule_name="check_variable_bounds_ordering",
            fields=[],
            literals=[],
            error_message="For {context}, lower_bounds must not exceed upper_bounds.",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            _call_cpp_validator(
                _cpp.check_variable_bounds_ordering,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        lower_bounds = getattr(instance, "lower_bounds", None)
        upper_bounds = getattr(instance, "upper_bounds", None)

        if lower_bounds is None or upper_bounds is None:
            return

        n = min(len(lower_bounds), len(upper_bounds))

        for i in range(n):
            lb = lower_bounds[i]
            ub = upper_bounds[i]

            if lb > ub:
                raise ValueError(
                    f"lower_bounds[{i}] = {lb} exceeds upper_bounds[{i}] = {ub}"
                )


class CheckLinearInequalityBoundsOrdering(ValidationRule):
    """Validate linear inequality bounds ordering.

    Fields: [] (uses: linear_inequality_lower_bounds, linear_inequality_upper_bounds)
    Literals: []
    """

    def __init__(self, context: str):
        super().__init__(
            context=context,
            rule_name="check_linear_inequality_bounds_ordering",
            fields=[],
            literals=[],
            error_message="For {context}, linear_inequality_lower_bounds must not exceed upper_bounds.",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            _call_cpp_validator(
                _cpp.check_linear_inequality_bounds_ordering,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        lower_bounds = getattr(instance, "linear_inequality_lower_bounds", None)
        upper_bounds = getattr(instance, "linear_inequality_upper_bounds", None)

        if lower_bounds is None or upper_bounds is None:
            return

        n = min(len(lower_bounds), len(upper_bounds))

        for i in range(n):
            lb = lower_bounds[i]
            ub = upper_bounds[i]

            if lb > ub:
                raise ValueError(
                    f"linear_inequality_lower_bounds[{i}] = {lb} exceeds "
                    f"linear_inequality_upper_bounds[{i}] = {ub}"
                )


class CheckAllVariableDescriptorsUnique(ValidationRule):
    """Check that all variable descriptors are unique across all variable types.

    Applied to VariablesConfig.

    Fields: []
    Literals: []
    """

    # List of all variable type field paths that have descriptors
    VAR_PATHS = [
        "continuous_design.descriptors",
        "discrete_design_range.descriptors",
        "discrete_design_set.integer.descriptors",
        "discrete_design_set.string.descriptors",
        "discrete_design_set.real.descriptors",
        "normal_uncertain.descriptors",
        "lognormal_uncertain.descriptors",
        "uniform_uncertain.descriptors",
        "loguniform_uncertain.descriptors",
        "triangular_uncertain.descriptors",
        "exponential_uncertain.descriptors",
        "beta_uncertain.descriptors",
        "gamma_uncertain.descriptors",
        "gumbel_uncertain.descriptors",
        "frechet_uncertain.descriptors",
        "weibull_uncertain.descriptors",
        "histogram_bin_uncertain.descriptors",
        "poisson_uncertain.descriptors",
        "binomial_uncertain.descriptors",
        "negative_binomial_uncertain.descriptors",
        "geometric_uncertain.descriptors",
        "hypergeometric_uncertain.descriptors",
        "histogram_point_uncertain.integer.descriptors",
        "histogram_point_uncertain.string.descriptors",
        "histogram_point_uncertain.real.descriptors",
        "continuous_interval_uncertain.descriptors",
        "discrete_interval_uncertain.descriptors",
        "discrete_uncertain_set.integer.descriptors",
        "discrete_uncertain_set.string.descriptors",
        "discrete_uncertain_set.real.descriptors",
        "continuous_state.descriptors",
        "discrete_state_range.descriptors",
        "discrete_state_set.integer.descriptors",
        "discrete_state_set.string.descriptors",
        "discrete_state_set.real.descriptors",
    ]

    def __init__(self, context: str):
        super().__init__(
            context=context,
            rule_name="check_all_variable_descriptors_unique",
            fields=[],
            literals=[],
            error_message="For {context}, all variable descriptors must be unique.",
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(
                mode="python", exclude_computed_fields=True
            )
            _call_cpp_validator(
                _cpp.check_all_variable_descriptors_unique,
                instance_dict,
                self.fields,
                self.literals,
                self.context,
            )
        else:
            self._python_impl(instance)

    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        all_descriptors = set()

        for path in self.VAR_PATHS:
            descriptors = _resolve_path(instance, path)
            if descriptors is None:
                continue

            for descriptor in descriptors:
                if descriptor in all_descriptors:
                    raise ValueError(
                        f"Duplicate variable descriptor '{descriptor}' found across variable types"
                    )
                all_descriptors.add(descriptor)
