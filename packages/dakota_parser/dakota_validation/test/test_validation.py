"""Tests for Dakota validation rules.

These tests verify both the C++ backend (via pybind11) and the Python fallback
implementations produce identical results.
"""

import pytest
import math
import importlib
from typing import ClassVar, List
from pydantic import BaseModel, Field, ValidationError

# Import validation components
import sys
import os

# Add the python directory to the path for testing
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'python'))

from dakota.spec.validation import (
    ValidationRule,
    VALIDATION_RULES_ATTR,
    collect_and_execute_validation_rules,
    CPP_BACKEND_AVAILABLE,
)
from dakota.spec.validation.rules import (
    CompareLength,
    CompareLengthOne,
    DefaultBoundsReal,
    DefaultBoundsInt,
    DefaultInitialPointReal,
    DefaultInitialPointInt,
    CheckNonnegativeList,
    CheckPositiveList,
    CheckProbabilityList,
    CheckRealLowerBound,
    CheckRealUpperBound,
    CheckIntLowerBound,
    CheckSumEqualsLength,
    TrustRegionValidate,
    CheckInterfaceBlock,
    CheckAnalysisDrivers,
    CheckResponseDescriptors,
    CheckFdGradientStepSize,
    CheckMixedGradients,
    CheckMixedHessians,
    DefaultInequalityBounds,
    DefaultEqualityTargets,
    CheckResponseDescriptorsLength,
    CheckDescriptorsValid,
    CheckPermittedValues,
    CheckScalesRequired,
    CheckConstraintBoundsOrdering,
    CheckVariableDescriptorsLength,
    DefaultVariableDescriptors,
    CheckVariableBoundsOrdering,
    CheckLinearInequalityBoundsOrdering,
    CheckAllVariableDescriptorsUnique,
    _USE_CPP,
)
from dakota.spec.validation import computed_field_schema
from dakota.spec.validation import computed_fields as computed_fields_module
from dakota.spec.validation.computed_fields import (
    HistogramBinBounds,
    HistogramPointRealBounds,
    ContinuousIntervalBounds,
    DiscreteSetStrBounds,
)


# ============================================================================
# Test fixtures - Simple models for testing
# ============================================================================

class SimpleModel(BaseModel):
    """A simple model for testing CompareLength."""
    count: int
    values: List[float] | None = None
    
    model_config = {'extra': 'forbid'}


class DesignModel(BaseModel):
    """A model with bounds and initial point for testing."""
    count: int = Field(gt=0)
    lower_bounds: List[float] | None = None
    upper_bounds: List[float] | None = None
    initial_point: List[float] | None = None
    scale_types: List[str] | None = None
    
    model_config = {'extra': 'forbid'}


class ListModel(BaseModel):
    """A model for testing list element validators."""
    values: List[float] | None = None
    int_values: List[int] | None = None
    probabilities: List[float] | None = None
    
    model_config = {'extra': 'forbid'}


class ResponseLevelModel(BaseModel):
    """A model for testing sum-equals-length validation."""
    num_response_levels: List[int] | None = None
    response_levels: List[float] | None = None
    
    model_config = {'extra': 'forbid'}


class TrustRegionModel(BaseModel):
    """A model for testing trust region validation."""
    initial_size: List[float] | None = None
    minimum_size: float | None = None
    contract_threshold: float | None = None
    expand_threshold: float | None = None
    contraction_factor: float | None = None
    expansion_factor: float | None = None
    
    model_config = {'extra': 'forbid'}


# ============================================================================
# Response Block Test Fixtures
# ============================================================================

from typing import Union, Literal, Optional

class NonlinearInequalityConstraints(BaseModel):
    """Nested model for inequality constraints."""
    count: int = 0
    lower_bounds: Optional[List[float]] = None
    upper_bounds: Optional[List[float]] = None
    scale_types: Optional[List[str]] = None
    
    model_config = {'extra': 'forbid'}


class NonlinearEqualityConstraints(BaseModel):
    """Nested model for equality constraints."""
    count: int = 0
    targets: Optional[List[float]] = None
    scale_types: Optional[List[str]] = None
    
    model_config = {'extra': 'forbid'}


class CalibrationTermsConfig(BaseModel):
    """Nested model for calibration_terms config."""
    count: int = 0
    nonlinear_inequality_constraints: Optional[NonlinearInequalityConstraints] = None
    nonlinear_equality_constraints: Optional[NonlinearEqualityConstraints] = None
    
    model_config = {'extra': 'forbid'}


class ObjectiveFunctionsConfig(BaseModel):
    """Nested model for objective_functions config."""
    count: int = 0
    nonlinear_inequality_constraints: Optional[NonlinearInequalityConstraints] = None
    nonlinear_equality_constraints: Optional[NonlinearEqualityConstraints] = None
    primary_scale_types: Optional[List[str]] = None
    
    model_config = {'extra': 'forbid'}


class ResponseFunctionsConfig(BaseModel):
    """Nested model for response_functions config."""
    count: int = 0
    
    model_config = {'extra': 'forbid'}


class CalibrationTerms(BaseModel):
    """Wrapper for calibration terms."""
    calibration_terms: CalibrationTermsConfig
    
    model_config = {'extra': 'forbid'}


class ObjectiveFunctions(BaseModel):
    """Wrapper for objective functions."""
    objective_functions: ObjectiveFunctionsConfig
    
    model_config = {'extra': 'forbid'}


class ResponseFunctions(BaseModel):
    """Wrapper for response functions."""
    response_functions: ResponseFunctionsConfig
    
    model_config = {'extra': 'forbid'}


# Gradient/Hessian mixin models (matching __init__.py structure)

class MixedGradientsMethodSourceVendor(BaseModel):
    """Vendor method source marker."""
    vendor: Literal[True] = True

class MixedGradientsMethodSourceDakota(BaseModel):
    """Dakota method source marker."""
    dakota: dict = Field(default_factory=dict)

class MixedGradientsConfig(BaseModel):
    """Config for mixed gradients."""
    id_numerical_gradients: List[int] = Field(default_factory=list)
    id_analytic_gradients: List[int] = Field(default_factory=list)
    method_source: Union[MixedGradientsMethodSourceDakota, MixedGradientsMethodSourceVendor] = Field(
        default_factory=MixedGradientsMethodSourceDakota
    )
    fd_step_size: Optional[List[float]] = None
    
    model_config = {'extra': 'forbid'}

class MixedGradients(BaseModel):
    """Mixed gradients wrapper."""
    mixed_gradients: MixedGradientsConfig
    
    model_config = {'extra': 'forbid'}

class NumericalGradientsConfig(BaseModel):
    """Config for numerical gradients."""
    method_source: Union[MixedGradientsMethodSourceDakota, MixedGradientsMethodSourceVendor] = Field(
        default_factory=MixedGradientsMethodSourceDakota
    )
    fd_step_size: Optional[List[float]] = None
    
    model_config = {'extra': 'forbid'}

class NumericalGradients(BaseModel):
    """Numerical gradients wrapper."""
    numerical_gradients: NumericalGradientsConfig
    
    model_config = {'extra': 'forbid'}

class NoGradients(BaseModel):
    """No gradients marker."""
    no_gradients: Literal[True] = True

class AnalyticGradients(BaseModel):
    """Analytic gradients marker."""
    analytic_gradients: Literal[True] = True

class IdNumericalHessians(BaseModel):
    """ID list for numerical Hessians."""
    values: List[int] = Field(default_factory=list)
    fd_step_size: Optional[List[float]] = None

class IdQuasiHessians(BaseModel):
    """ID list for quasi Hessians."""
    values: List[int] = Field(default_factory=list)

class MixedHessiansConfig(BaseModel):
    """Config for mixed Hessians."""
    id_analytic_hessians: Optional[List[int]] = None
    id_numerical_hessians: Optional[IdNumericalHessians] = None
    id_quasi_hessians: Optional[IdQuasiHessians] = None
    
    model_config = {'extra': 'forbid'}

class MixedHessians(BaseModel):
    """Mixed Hessians wrapper."""
    mixed_hessians: MixedHessiansConfig
    
    model_config = {'extra': 'forbid'}

class NoHessians(BaseModel):
    """No Hessians marker."""
    no_hessians: Literal[True] = True


class ResponsesConfig(BaseModel):
    """Model mimicking the actual ResponsesConfig structure."""
    descriptors: Optional[List[str]] = None
    response_type: Union[CalibrationTerms, ObjectiveFunctions, ResponseFunctions]
    
    # gradient_type from ResponseGradientsMixin - Union of gradient models
    gradient_type: Union[NoGradients, AnalyticGradients, MixedGradients, NumericalGradients] = Field(
        default_factory=NoGradients
    )
    
    # hessian_type from ResponseHessiansMixin - Union of hessian models
    hessian_type: Union[NoHessians, MixedHessians] = Field(
        default_factory=NoHessians
    )
    
    model_config = {'extra': 'forbid'}


# ============================================================================
# CompareLength tests
# ============================================================================

class TestCompareLength:
    """Tests for CompareLength validation rule."""
    
    def test_passes_when_length_matches(self):
        rule = CompareLength(context="test", list_field="values", target_field="count")
        instance = SimpleModel(count=3, values=[1.0, 2.0, 3.0])
        
        # Should not raise
        rule(instance)
    
    def test_skips_none_field(self):
        rule = CompareLength(context="test", list_field="values", target_field="count")
        instance = SimpleModel(count=3, values=None)
        
        # Should not raise
        rule(instance)
    
    def test_raises_on_length_mismatch(self):
        rule = CompareLength(context="test", list_field="values", target_field="count")
        instance = SimpleModel(count=3, values=[1.0, 2.0])
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "2" in str(exc_info.value)
        assert "3" in str(exc_info.value)
    
    def test_schema_data(self):
        rule = CompareLength(context="my_context", list_field="my_list", target_field="my_count")
        schema = rule.get_schema_data()
        
        assert schema["validationRuleName"] == "compare_len"
        assert schema["validationFields"] == ["my_list", "my_count"]
        assert schema["validationContext"] == "my_context"


# ============================================================================
# CompareLengthOne tests
# ============================================================================

class TestCompareLengthOne:
    """Tests for CompareLengthOne validation rule."""
    
    def test_passes_when_length_matches(self):
        rule = CompareLengthOne(context="test", list_field="scale_types", target_field="count")
        instance = DesignModel(count=3, scale_types=["auto", "log", "none"])
        
        rule(instance)
        assert instance.scale_types == ["auto", "log", "none"]
    
    def test_expands_single_value(self):
        rule = CompareLengthOne(context="test", list_field="scale_types", target_field="count")
        instance = DesignModel(count=3, scale_types=["auto"])
        
        rule(instance)
        assert instance.scale_types == ["auto", "auto", "auto"]
    
    def test_expands_single_float(self):
        rule = CompareLengthOne(context="test", list_field="lower_bounds", target_field="count")
        instance = DesignModel(count=4, lower_bounds=[0.0])
        
        rule(instance)
        assert instance.lower_bounds == [0.0, 0.0, 0.0, 0.0]
    
    def test_raises_on_invalid_length(self):
        rule = CompareLengthOne(context="test", list_field="scale_types", target_field="count")
        instance = DesignModel(count=3, scale_types=["auto", "log"])
        
        with pytest.raises(ValueError):
            rule(instance)
    
    def test_skips_none_field(self):
        rule = CompareLengthOne(context="test", list_field="scale_types", target_field="count")
        instance = DesignModel(count=3, scale_types=None)
        
        rule(instance)
        assert instance.scale_types is None


# ============================================================================
# DefaultBoundsReal tests
# ============================================================================

class TestDefaultBoundsReal:
    """Tests for DefaultBoundsReal validation rule."""
    
    def test_sets_lower_when_none(self):
        rule = DefaultBoundsReal(context="test", literals=[-1e10, 1e10])
        instance = DesignModel(
            count=3,
            lower_bounds=None,
            upper_bounds=[1.0, 2.0, 3.0]
        )
        
        rule(instance)
        
        assert instance.lower_bounds == [-1e10, -1e10, -1e10]
        assert instance.upper_bounds == [1.0, 2.0, 3.0]  # Unchanged
    
    def test_sets_upper_when_none(self):
        rule = DefaultBoundsReal(context="test", literals=[-1e10, 1e10])
        instance = DesignModel(
            count=2,
            lower_bounds=[0.0, 0.0],
            upper_bounds=None
        )
        
        rule(instance)
        
        assert instance.lower_bounds == [0.0, 0.0]  # Unchanged
        assert instance.upper_bounds == [1e10, 1e10]
    
    def test_sets_both_when_both_none(self):
        rule = DefaultBoundsReal(context="test", literals=[-100.0, 100.0])
        instance = DesignModel(count=2)
        
        rule(instance)
        
        assert instance.lower_bounds == [-100.0, -100.0]
        assert instance.upper_bounds == [100.0, 100.0]
    
    def test_no_change_when_both_set(self):
        rule = DefaultBoundsReal(context="test", literals=[-1e10, 1e10])
        instance = DesignModel(
            count=2,
            lower_bounds=[0.0, 1.0],
            upper_bounds=[10.0, 20.0]
        )
        
        rule(instance)
        
        assert instance.lower_bounds == [0.0, 1.0]
        assert instance.upper_bounds == [10.0, 20.0]


# ============================================================================
# DefaultInitialPointReal tests
# ============================================================================

class TestDefaultInitialPointReal:
    """Tests for DefaultInitialPointReal validation rule."""
    
    def test_sets_midpoint_when_none(self):
        rule = DefaultInitialPointReal(context="test")
        instance = DesignModel(
            count=3,
            lower_bounds=[0.0, -10.0, 5.0],
            upper_bounds=[10.0, 10.0, 15.0],
            initial_point=None
        )
        
        rule(instance)
        
        assert instance.initial_point is not None
        assert len(instance.initial_point) == 3
        assert instance.initial_point[0] == pytest.approx(5.0)   # (0 + 10) / 2
        assert instance.initial_point[1] == pytest.approx(0.0)   # (-10 + 10) / 2
        assert instance.initial_point[2] == pytest.approx(10.0)  # (5 + 15) / 2
    
    def test_uses_finite_bound_when_one_infinite(self):
        rule = DefaultInitialPointReal(context="test")
        instance = DesignModel(
            count=2,
            lower_bounds=[float('-inf'), 5.0],
            upper_bounds=[10.0, float('inf')],
            initial_point=None
        )
        
        rule(instance)
        
        assert instance.initial_point[0] == pytest.approx(10.0)  # Uses upper
        assert instance.initial_point[1] == pytest.approx(5.0)   # Uses lower
    
    def test_uses_zero_when_both_infinite(self):
        rule = DefaultInitialPointReal(context="test")
        instance = DesignModel(
            count=1,
            lower_bounds=[float('-inf')],
            upper_bounds=[float('inf')],
            initial_point=None
        )
        
        rule(instance)
        
        assert instance.initial_point[0] == pytest.approx(0.0)
    
    def test_clamps_to_lower_bound(self):
        rule = DefaultInitialPointReal(context="test")
        instance = DesignModel(
            count=2,
            lower_bounds=[0.0, 5.0],
            upper_bounds=[10.0, 15.0],
            initial_point=[-5.0, 10.0]  # -5 is below lower[0]=0
        )
        
        rule(instance)
        
        assert instance.initial_point[0] == pytest.approx(0.0)   # Clamped
        assert instance.initial_point[1] == pytest.approx(10.0)  # Unchanged
    
    def test_clamps_to_upper_bound(self):
        rule = DefaultInitialPointReal(context="test")
        instance = DesignModel(
            count=2,
            lower_bounds=[0.0, 5.0],
            upper_bounds=[10.0, 15.0],
            initial_point=[5.0, 20.0]  # 20 is above upper[1]=15
        )
        
        rule(instance)
        
        assert instance.initial_point[0] == pytest.approx(5.0)   # Unchanged
        assert instance.initial_point[1] == pytest.approx(15.0)  # Clamped
    
    def test_no_change_when_within_bounds(self):
        rule = DefaultInitialPointReal(context="test")
        instance = DesignModel(
            count=2,
            lower_bounds=[0.0, 0.0],
            upper_bounds=[10.0, 10.0],
            initial_point=[5.0, 5.0]
        )
        
        original = instance.initial_point.copy()
        rule(instance)
        
        assert instance.initial_point == original


# ============================================================================
# CheckNonnegativeList tests
# ============================================================================

class TestCheckNonnegativeList:
    """Tests for CheckNonnegativeList validation rule."""
    
    def test_passes_all_positive(self):
        rule = CheckNonnegativeList(context="test", list_field="values")
        instance = ListModel(values=[1.0, 2.0, 3.0])
        
        rule(instance)  # Should not raise
    
    def test_passes_with_zero(self):
        rule = CheckNonnegativeList(context="test", list_field="values")
        instance = ListModel(values=[0.0, 1.0, 0.0])
        
        rule(instance)  # Should not raise
    
    def test_skips_none_field(self):
        rule = CheckNonnegativeList(context="test", list_field="values")
        instance = ListModel(values=None)
        
        rule(instance)  # Should not raise
    
    def test_raises_on_negative(self):
        rule = CheckNonnegativeList(context="test", list_field="values")
        instance = ListModel(values=[1.0, -2.0, 3.0])
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "element 1" in str(exc_info.value)
        assert "negative" in str(exc_info.value)
    
    def test_raises_on_first_negative(self):
        rule = CheckNonnegativeList(context="test", list_field="values")
        instance = ListModel(values=[-1.0, 2.0, 3.0])
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "element 0" in str(exc_info.value)
    
    def test_schema_data(self):
        rule = CheckNonnegativeList(context="my_context", list_field="my_list")
        schema = rule.get_schema_data()
        
        assert schema["validationRuleName"] == "check_nonnegative_list"
        assert schema["validationFields"] == ["my_list"]
        assert schema["validationLiterals"] == []


# ============================================================================
# CheckPositiveList tests
# ============================================================================

class TestCheckPositiveList:
    """Tests for CheckPositiveList validation rule."""
    
    def test_passes_all_positive(self):
        rule = CheckPositiveList(context="test", list_field="int_values")
        instance = ListModel(int_values=[1, 2, 3])
        
        rule(instance)  # Should not raise
    
    def test_skips_none_field(self):
        rule = CheckPositiveList(context="test", list_field="int_values")
        instance = ListModel(int_values=None)
        
        rule(instance)  # Should not raise
    
    def test_raises_on_zero(self):
        rule = CheckPositiveList(context="test", list_field="int_values")
        instance = ListModel(int_values=[1, 0, 3])
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "element 1" in str(exc_info.value)
        assert "not positive" in str(exc_info.value)
    
    def test_raises_on_negative(self):
        rule = CheckPositiveList(context="test", list_field="int_values")
        instance = ListModel(int_values=[1, -2, 3])
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "element 1" in str(exc_info.value)


# ============================================================================
# CheckProbabilityList tests
# ============================================================================

class TestCheckProbabilityList:
    """Tests for CheckProbabilityList validation rule."""
    
    def test_passes_valid_probabilities(self):
        rule = CheckProbabilityList(context="test", list_field="probabilities")
        instance = ListModel(probabilities=[0.0, 0.5, 1.0])
        
        rule(instance)  # Should not raise
    
    def test_skips_none_field(self):
        rule = CheckProbabilityList(context="test", list_field="probabilities")
        instance = ListModel(probabilities=None)
        
        rule(instance)  # Should not raise
    
    def test_raises_on_negative(self):
        rule = CheckProbabilityList(context="test", list_field="probabilities")
        instance = ListModel(probabilities=[0.5, -0.1, 0.8])
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "element 1" in str(exc_info.value)
        assert "not in [0, 1]" in str(exc_info.value)
    
    def test_raises_on_greater_than_one(self):
        rule = CheckProbabilityList(context="test", list_field="probabilities")
        instance = ListModel(probabilities=[0.5, 1.1, 0.8])
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "element 1" in str(exc_info.value)
        assert "not in [0, 1]" in str(exc_info.value)


# ============================================================================
# CheckRealLowerBound tests
# ============================================================================

class TestCheckRealLowerBound:
    """Tests for CheckRealLowerBound validation rule."""
    
    def test_passes_above_bound(self):
        rule = CheckRealLowerBound(context="test", list_field="values", lower_bound=0.0)
        instance = ListModel(values=[0.1, 1.0, 5.0])
        
        rule(instance)  # Should not raise
    
    def test_skips_none_field(self):
        rule = CheckRealLowerBound(context="test", list_field="values", lower_bound=0.0)
        instance = ListModel(values=None)
        
        rule(instance)  # Should not raise
    
    def test_raises_on_equal_to_bound(self):
        rule = CheckRealLowerBound(context="test", list_field="values", lower_bound=0.0)
        instance = ListModel(values=[1.0, 0.0, 2.0])  # 0.0 is not > 0.0
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "element 1" in str(exc_info.value)
        assert "not > 0" in str(exc_info.value)
    
    def test_raises_on_below_bound(self):
        rule = CheckRealLowerBound(context="test", list_field="values", lower_bound=0.0)
        instance = ListModel(values=[1.0, -0.5, 2.0])
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "element 1" in str(exc_info.value)
    
    def test_schema_data(self):
        rule = CheckRealLowerBound(context="test", list_field="values", lower_bound=0.0)
        schema = rule.get_schema_data()
        
        assert schema["validationRuleName"] == "check_real_lower_bound"
        assert schema["validationFields"] == ["values"]
        assert schema["validationLiterals"] == [0.0, False]

    def test_inclusive_passes_on_equal(self):
        rule = CheckRealLowerBound(context="test", list_field="values", lower_bound=0.0, inclusive=True)
        instance = ListModel(values=[0.0, 1.0, 5.0])
        
        rule(instance)  # Should not raise

    def test_inclusive_raises_on_below(self):
        rule = CheckRealLowerBound(context="test", list_field="values", lower_bound=0.0, inclusive=True)
        instance = ListModel(values=[1.0, -0.1, 2.0])
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "element 1" in str(exc_info.value)
        assert "not >= 0" in str(exc_info.value)

    def test_inclusive_schema_data(self):
        rule = CheckRealLowerBound(context="test", list_field="values", lower_bound=0.0, inclusive=True)
        schema = rule.get_schema_data()
        
        assert schema["validationLiterals"] == [0.0, True]


# ============================================================================
# CheckRealUpperBound tests
# ============================================================================

class TestCheckRealUpperBound:
    """Tests for CheckRealUpperBound validation rule."""
    
    def test_passes_below_bound(self):
        rule = CheckRealUpperBound(context="test", list_field="values", upper_bound=1.0)
        instance = ListModel(values=[0.1, 0.5, 0.9])
        
        rule(instance)  # Should not raise
    
    def test_skips_none_field(self):
        rule = CheckRealUpperBound(context="test", list_field="values", upper_bound=1.0)
        instance = ListModel(values=None)
        
        rule(instance)  # Should not raise
    
    def test_raises_on_equal_to_bound(self):
        rule = CheckRealUpperBound(context="test", list_field="values", upper_bound=1.0)
        instance = ListModel(values=[0.5, 1.0, 0.8])  # 1.0 is not < 1.0
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "element 1" in str(exc_info.value)
        assert "not < 1" in str(exc_info.value)
    
    def test_raises_on_above_bound(self):
        rule = CheckRealUpperBound(context="test", list_field="values", upper_bound=1.0)
        instance = ListModel(values=[0.5, 1.5, 0.8])
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "element 1" in str(exc_info.value)

    def test_inclusive_passes_on_equal(self):
        rule = CheckRealUpperBound(context="test", list_field="values", upper_bound=1.0, inclusive=True)
        instance = ListModel(values=[0.5, 1.0, 0.8])
        
        rule(instance)  # Should not raise

    def test_inclusive_raises_on_above(self):
        rule = CheckRealUpperBound(context="test", list_field="values", upper_bound=1.0, inclusive=True)
        instance = ListModel(values=[0.5, 1.1, 0.8])
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "element 1" in str(exc_info.value)
        assert "not <= 1" in str(exc_info.value)


# ============================================================================
# CheckIntLowerBound tests
# ============================================================================

class TestCheckIntLowerBound:
    """Tests for CheckIntLowerBound validation rule."""
    
    def test_passes_above_bound(self):
        rule = CheckIntLowerBound(context="test", list_field="int_values", lower_bound=0)
        instance = ListModel(int_values=[1, 2, 3])
        
        rule(instance)  # Should not raise
    
    def test_skips_none_field(self):
        rule = CheckIntLowerBound(context="test", list_field="int_values", lower_bound=0)
        instance = ListModel(int_values=None)
        
        rule(instance)  # Should not raise
    
    def test_raises_on_equal_to_bound(self):
        rule = CheckIntLowerBound(context="test", list_field="int_values", lower_bound=0)
        instance = ListModel(int_values=[1, 0, 3])  # 0 is not > 0
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "element 1" in str(exc_info.value)
        assert "not > 0" in str(exc_info.value)
    
    def test_raises_on_below_bound(self):
        rule = CheckIntLowerBound(context="test", list_field="int_values", lower_bound=0)
        instance = ListModel(int_values=[1, -5, 3])
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "element 1" in str(exc_info.value)


# ============================================================================
# CheckSumEqualsLength tests
# ============================================================================

class TestCheckSumEqualsLength:
    """Tests for CheckSumEqualsLength validation rule."""
    
    def test_passes_when_sum_matches_length(self):
        rule = CheckSumEqualsLength(
            context="test",
            num_list_field="num_response_levels",
            levels_list_field="response_levels"
        )
        instance = ResponseLevelModel(
            num_response_levels=[2, 3],  # sum = 5
            response_levels=[0.1, 0.2, 0.3, 0.4, 0.5]  # length = 5
        )
        
        rule(instance)  # Should not raise
    
    def test_skips_none_num_list(self):
        rule = CheckSumEqualsLength(
            context="test",
            num_list_field="num_response_levels",
            levels_list_field="response_levels"
        )
        instance = ResponseLevelModel(
            num_response_levels=None,
            response_levels=[0.1, 0.2, 0.3]
        )
        
        rule(instance)  # Should not raise
    
    def test_skips_none_levels_list(self):
        rule = CheckSumEqualsLength(
            context="test",
            num_list_field="num_response_levels",
            levels_list_field="response_levels"
        )
        instance = ResponseLevelModel(
            num_response_levels=[2, 3],
            response_levels=None
        )
        
        rule(instance)  # Should not raise
    
    def test_raises_on_mismatch(self):
        rule = CheckSumEqualsLength(
            context="test",
            num_list_field="num_response_levels",
            levels_list_field="response_levels"
        )
        instance = ResponseLevelModel(
            num_response_levels=[2, 3],  # sum = 5
            response_levels=[0.1, 0.2, 0.3]  # length = 3
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "5" in str(exc_info.value)
        assert "3" in str(exc_info.value)
    
    def test_schema_data(self):
        rule = CheckSumEqualsLength(
            context="test",
            num_list_field="num_response_levels",
            levels_list_field="response_levels"
        )
        schema = rule.get_schema_data()
        
        assert schema["validationRuleName"] == "check_sum_equals_length"
        assert schema["validationFields"] == ["num_response_levels", "response_levels"]


# ============================================================================
# TrustRegionValidate tests
# ============================================================================

class TestTrustRegionValidate:
    """Tests for TrustRegionValidate validation rule."""
    
    def test_passes_valid_params(self):
        rule = TrustRegionValidate(context="test")
        instance = TrustRegionModel(
            initial_size=[0.5, 0.6],
            minimum_size=0.1,
            contract_threshold=0.25,
            expand_threshold=0.75,
            contraction_factor=0.5,
            expansion_factor=2.0
        )
        
        rule(instance)  # Should not raise
    
    def test_passes_minimal_params(self):
        rule = TrustRegionValidate(context="test")
        instance = TrustRegionModel(
            contract_threshold=0.25,
            expand_threshold=0.75
        )
        
        rule(instance)  # Should not raise
    
    def test_raises_initial_size_zero(self):
        rule = TrustRegionValidate(context="test")
        instance = TrustRegionModel(
            initial_size=[0.5, 0.0],  # 0.0 not in (0, 1]
            contract_threshold=0.25,
            expand_threshold=0.75
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "initial_size[1]" in str(exc_info.value)
        assert "(0, 1]" in str(exc_info.value)
    
    def test_raises_initial_size_greater_than_one(self):
        rule = TrustRegionValidate(context="test")
        instance = TrustRegionModel(
            initial_size=[0.5, 1.5],  # 1.5 not in (0, 1]
            contract_threshold=0.25,
            expand_threshold=0.75
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "initial_size[1]" in str(exc_info.value)
    
    def test_raises_initial_size_less_than_minimum(self):
        rule = TrustRegionValidate(context="test")
        instance = TrustRegionModel(
            initial_size=[0.5, 0.2],
            minimum_size=0.3,  # 0.2 < 0.3
            contract_threshold=0.25,
            expand_threshold=0.75
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "initial_size[1]" in str(exc_info.value)
        assert "minimum_size" in str(exc_info.value)
    
    def test_raises_minimum_size_negative(self):
        rule = TrustRegionValidate(context="test")
        instance = TrustRegionModel(
            minimum_size=-0.1,
            contract_threshold=0.25,
            expand_threshold=0.75
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "minimum_size" in str(exc_info.value)
        assert "[0, 1]" in str(exc_info.value)
    
    def test_raises_contract_threshold_zero(self):
        rule = TrustRegionValidate(context="test")
        instance = TrustRegionModel(
            contract_threshold=0.0,  # must be > 0
            expand_threshold=0.75
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "contract_threshold" in str(exc_info.value)
        assert "> 0" in str(exc_info.value)
    
    def test_raises_contract_greater_than_expand(self):
        rule = TrustRegionValidate(context="test")
        instance = TrustRegionModel(
            contract_threshold=0.8,
            expand_threshold=0.5  # 0.8 > 0.5
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "contract_threshold" in str(exc_info.value)
        assert "expand_threshold" in str(exc_info.value)
    
    def test_raises_expand_threshold_greater_than_one(self):
        rule = TrustRegionValidate(context="test")
        instance = TrustRegionModel(
            contract_threshold=0.25,
            expand_threshold=1.5  # must be <= 1
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "expand_threshold" in str(exc_info.value)
        assert "<= 1" in str(exc_info.value)
    
    def test_raises_contraction_factor_zero(self):
        rule = TrustRegionValidate(context="test")
        instance = TrustRegionModel(
            contract_threshold=0.25,
            expand_threshold=0.75,
            contraction_factor=0.0  # must be in (0, 1]
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "contraction_factor" in str(exc_info.value)
        assert "(0, 1]" in str(exc_info.value)
    
    def test_raises_contraction_factor_greater_than_one(self):
        rule = TrustRegionValidate(context="test")
        instance = TrustRegionModel(
            contract_threshold=0.25,
            expand_threshold=0.75,
            contraction_factor=1.5  # must be in (0, 1]
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "contraction_factor" in str(exc_info.value)
    
    def test_raises_expansion_factor_less_than_one(self):
        rule = TrustRegionValidate(context="test")
        instance = TrustRegionModel(
            contract_threshold=0.25,
            expand_threshold=0.75,
            expansion_factor=0.5  # must be >= 1
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "expansion_factor" in str(exc_info.value)
        assert ">= 1" in str(exc_info.value)
    
    def test_contraction_factor_one_is_valid(self):
        """contraction_factor = 1.0 is valid (though Dakota warns about it)."""
        rule = TrustRegionValidate(context="test")
        instance = TrustRegionModel(
            contract_threshold=0.25,
            expand_threshold=0.75,
            contraction_factor=1.0
        )
        
        rule(instance)  # Should not raise
    
    def test_schema_data(self):
        rule = TrustRegionValidate(context="test")
        schema = rule.get_schema_data()
        
        assert schema["validationRuleName"] == "trust_region_validate"
        assert len(schema["validationFields"]) == 6


# ============================================================================
# Nested Mutation Tests
# ============================================================================

class InnerModel(BaseModel):
    """Inner model for testing nested mutations."""
    value: float | None = None
    values: List[float] | None = None
    count: int = 1
    
    model_config = {'extra': 'forbid'}


class DeepInnerModel(BaseModel):
    """Deeply nested model."""
    x: float | None = None
    
    model_config = {'extra': 'forbid'}


class MiddleModel(BaseModel):
    """Middle level model with further nesting."""
    inner: DeepInnerModel
    name: str = "default"
    
    model_config = {'extra': 'forbid'}


class OuterModel(BaseModel):
    """Outer model containing nested models for testing."""
    inner: InnerModel
    name: str = "outer"
    
    model_config = {'extra': 'forbid'}


class DeeplyNestedModel(BaseModel):
    """Model with deep nesting for testing multi-level paths."""
    middle: MiddleModel
    count: int = 1
    
    model_config = {'extra': 'forbid'}


class TestNestedMutations:
    """Tests for nested field mutation support."""
    
    def test_simple_nested_mutation(self):
        """Test mutation to a single-level nested field."""
        from dakota.spec.validation.rules import _apply_mutations
        
        instance = OuterModel(
            inner=InnerModel(count=3)
        )
        
        # Apply mutation to nested field
        _apply_mutations(instance, {"inner.value": 42.0})
        
        assert instance.inner.value == 42.0
        assert instance.inner.count == 3  # Unchanged
        assert instance.name == "outer"  # Unchanged
    
    def test_nested_list_mutation(self):
        """Test mutation to a nested list field."""
        from dakota.spec.validation.rules import _apply_mutations
        
        instance = OuterModel(
            inner=InnerModel(count=3)
        )
        
        _apply_mutations(instance, {"inner.values": [1.0, 2.0, 3.0]})
        
        assert instance.inner.values == [1.0, 2.0, 3.0]
    
    def test_deep_nested_mutation(self):
        """Test mutation to a deeply nested field (multiple levels)."""
        from dakota.spec.validation.rules import _apply_mutations
        
        instance = DeeplyNestedModel(
            middle=MiddleModel(
                inner=DeepInnerModel(x=None)
            )
        )
        
        _apply_mutations(instance, {"middle.inner.x": 99.5})
        
        assert instance.middle.inner.x == 99.5
    
    def test_multiple_nested_mutations(self):
        """Test applying multiple nested mutations at once."""
        from dakota.spec.validation.rules import _apply_mutations
        
        instance = OuterModel(
            inner=InnerModel(count=2)
        )
        
        _apply_mutations(instance, {
            "inner.value": 10.0,
            "inner.values": [1.0, 2.0],
            "name": "modified"
        })
        
        assert instance.inner.value == 10.0
        assert instance.inner.values == [1.0, 2.0]
        assert instance.name == "modified"
    
    def test_direct_field_still_works(self):
        """Test that non-nested mutations still work correctly."""
        from dakota.spec.validation.rules import _apply_mutations
        
        instance = OuterModel(
            inner=InnerModel(count=1)
        )
        
        _apply_mutations(instance, {"name": "new_name"})
        
        assert instance.name == "new_name"
    
    def test_rejects_mutation_to_model_field(self):
        """Test that mutating an entire nested model is rejected."""
        from dakota.spec.validation.rules import _apply_mutations
        
        instance = OuterModel(
            inner=InnerModel(count=1)
        )
        
        # Attempting to replace the entire 'inner' model should fail
        with pytest.raises(RuntimeError) as exc_info:
            _apply_mutations(instance, {"inner": {"value": 1.0, "count": 2}})
        
        assert "nested model type" in str(exc_info.value)
        assert "InnerModel" in str(exc_info.value)
    
    def test_rejects_invalid_intermediate_path(self):
        """Test that invalid intermediate path components are rejected."""
        from dakota.spec.validation.rules import _apply_mutations
        
        instance = OuterModel(
            inner=InnerModel(count=1)
        )
        
        # 'nonexistent' is not a field
        with pytest.raises(RuntimeError) as exc_info:
            _apply_mutations(instance, {"nonexistent.value": 1.0})
        
        assert "not found" in str(exc_info.value)
        assert "nonexistent" in str(exc_info.value)
    
    def test_rejects_null_intermediate(self):
        """Test that null intermediate fields are handled."""
        from dakota.spec.validation.rules import _apply_mutations
        
        # Create a model where the nested field could be None
        class OptionalNestedModel(BaseModel):
            inner: InnerModel | None = None
            model_config = {'extra': 'forbid'}
        
        instance = OptionalNestedModel(inner=None)
        
        with pytest.raises(RuntimeError) as exc_info:
            _apply_mutations(instance, {"inner.value": 1.0})
        
        assert "is None" in str(exc_info.value)
    
    def test_rejects_non_model_intermediate(self):
        """Test that navigating through non-model fields is rejected."""
        from dakota.spec.validation.rules import _apply_mutations
        
        # 'name' is a string, not a model
        instance = OuterModel(
            inner=InnerModel(count=1)
        )
        
        with pytest.raises(RuntimeError) as exc_info:
            _apply_mutations(instance, {"name.something": 1.0})
        
        assert "not a model instance" in str(exc_info.value)
    
    def test_rejects_invalid_leaf_field(self):
        """Test that invalid leaf field names are rejected."""
        from dakota.spec.validation.rules import _apply_mutations
        
        instance = OuterModel(
            inner=InnerModel(count=1)
        )
        
        with pytest.raises(RuntimeError) as exc_info:
            _apply_mutations(instance, {"inner.nonexistent": 1.0})
        
        assert "not found" in str(exc_info.value)
        assert "nonexistent" in str(exc_info.value)


# ============================================================================
# Integration tests
# ============================================================================

class TestIntegration:
    """Integration tests using multiple rules together."""
    
    def test_full_design_variable_validation(self):
        """Test a realistic design variable validation pipeline."""
        rules = [
            DefaultBoundsReal(context="continuous_design", literals=[-1e10, 1e10]),
            CompareLength(context="continuous_design", list_field="lower_bounds", target_field="count"),
            CompareLength(context="continuous_design", list_field="upper_bounds", target_field="count"),
            CompareLengthOne(context="continuous_design", list_field="scale_types", target_field="count"),
            DefaultInitialPointReal(context="continuous_design"),
        ]
        
        # Create instance with minimal input
        instance = DesignModel(count=3, scale_types=["auto"])
        
        # Run all validation rules
        for rule in rules:
            rule(instance)
        
        # Verify results
        assert instance.lower_bounds == [-1e10, -1e10, -1e10]
        assert instance.upper_bounds == [1e10, 1e10, 1e10]
        assert instance.scale_types == ["auto", "auto", "auto"]
        assert instance.initial_point == [0.0, 0.0, 0.0]  # Midpoint of inf bounds


# ============================================================================
# CheckInterfaceBlock test fixtures
# ============================================================================

class FailureCaptureAbort(BaseModel):
    """Abort failure capture mode."""
    abort: bool = True
    
    model_config = {'extra': 'forbid'}


class FailureCaptureRecover(BaseModel):
    """Recover failure capture mode."""
    recover: bool = True
    
    model_config = {'extra': 'forbid'}


class FailureCaptureRetry(BaseModel):
    """Retry failure capture mode."""
    retry: bool = True
    
    model_config = {'extra': 'forbid'}


class FailureCaptureContinuation(BaseModel):
    """Continuation failure capture mode."""
    continuation: bool = True
    
    model_config = {'extra': 'forbid'}


FailureCaptureModel = FailureCaptureAbort | FailureCaptureRecover | FailureCaptureRetry | FailureCaptureContinuation


class BatchConcurrencyModel(BaseModel):
    """Batch concurrency settings."""
    size: int | None = None
    
    model_config = {'extra': 'forbid'}


class AsyncConcurrencyModel(BaseModel):
    """Asynchronous concurrency settings."""
    evaluation_concurrency: int | None = None
    analysis_concurrency: int | None = None
    
    model_config = {'extra': 'forbid'}


class ConcurrencyModel(BaseModel):
    """Concurrency configuration."""
    batch: BatchConcurrencyModel | None = None
    asynchronous: AsyncConcurrencyModel | None = None
    
    model_config = {'extra': 'forbid'}


class AnalysisDriversModel(BaseModel):
    """Analysis drivers configuration."""
    drivers: List[str]
    input_filter: str | None = None
    output_filter: str | None = None
    analysis_components: List[str] | None = None
    
    model_config = {'extra': 'forbid'}


class InterfaceModel(BaseModel):
    """Interface block model for testing."""
    analysis_drivers: AnalysisDriversModel | None = None
    algebraic_mappings: str | None = None
    concurrency: ConcurrencyModel | None = None
    failure_capture: FailureCaptureModel = Field(default_factory=FailureCaptureAbort)
    
    model_config = {'extra': 'forbid'}


# ============================================================================
# CheckInterfaceBlock tests
# ============================================================================

class TestCheckInterfaceBlock:
    """Tests for CheckInterfaceBlock validation rule."""
    
    def test_passes_with_analysis_drivers(self):
        """Valid when analysis_drivers is present."""
        rule = CheckInterfaceBlock(context="interface")
        instance = InterfaceModel(
            analysis_drivers=AnalysisDriversModel(drivers=["simulator.py"])
        )
        rule(instance)  # Should not raise
    
    def test_passes_with_algebraic_mappings(self):
        """Valid when algebraic_mappings is present."""
        rule = CheckInterfaceBlock(context="interface")
        instance = InterfaceModel(
            algebraic_mappings="mappings.txt"
        )
        rule(instance)  # Should not raise
    
    def test_passes_with_both(self):
        """Valid when both analysis_drivers and algebraic_mappings present."""
        rule = CheckInterfaceBlock(context="interface")
        instance = InterfaceModel(
            analysis_drivers=AnalysisDriversModel(drivers=["simulator.py"]),
            algebraic_mappings="mappings.txt"
        )
        rule(instance)  # Should not raise
    
    def test_raises_when_neither_present(self):
        """Error when neither analysis_drivers nor algebraic_mappings."""
        rule = CheckInterfaceBlock(context="interface")
        instance = InterfaceModel()
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "analysis_drivers" in str(exc_info.value)
        assert "algebraic_mappings" in str(exc_info.value)
    
    def test_batch_rejects_multiple_drivers(self):
        """Batch mode rejects multiple analysis drivers."""
        rule = CheckInterfaceBlock(context="interface")
        instance = InterfaceModel(
            analysis_drivers=AnalysisDriversModel(drivers=["driver1.py", "driver2.py"]),
            concurrency=ConcurrencyModel(batch=BatchConcurrencyModel())
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "multiple analysis drivers" in str(exc_info.value).lower()
    
    def test_batch_rejects_input_filter(self):
        """Batch mode rejects input filters."""
        rule = CheckInterfaceBlock(context="interface")
        instance = InterfaceModel(
            analysis_drivers=AnalysisDriversModel(
                drivers=["simulator.py"],
                input_filter="preprocess.py"
            ),
            concurrency=ConcurrencyModel(batch=BatchConcurrencyModel())
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "input filter" in str(exc_info.value).lower()
    
    def test_batch_rejects_output_filter(self):
        """Batch mode rejects output filters."""
        rule = CheckInterfaceBlock(context="interface")
        instance = InterfaceModel(
            analysis_drivers=AnalysisDriversModel(
                drivers=["simulator.py"],
                output_filter="postprocess.py"
            ),
            concurrency=ConcurrencyModel(batch=BatchConcurrencyModel())
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "output filter" in str(exc_info.value).lower()
    
    # def test_batch_rejects_size_one(self):
    #     """Batch mode rejects batch size of 1."""
    #     rule = CheckInterfaceBlock(context="interface")
    #     instance = InterfaceModel(
    #         analysis_drivers=AnalysisDriversModel(drivers=["simulator.py"]),
    #         concurrency=ConcurrencyModel(batch=BatchConcurrencyModel(size=1))
    #     )
        
    #     with pytest.raises(ValueError) as exc_info:
    #         rule(instance)
        
    #     assert "size" in str(exc_info.value).lower()
    #     assert "1" in str(exc_info.value)
    
    def test_batch_allows_larger_size(self):
        """Batch mode allows batch size > 1."""
        rule = CheckInterfaceBlock(context="interface")
        instance = InterfaceModel(
            analysis_drivers=AnalysisDriversModel(drivers=["simulator.py"]),
            concurrency=ConcurrencyModel(batch=BatchConcurrencyModel(size=10))
        )
        rule(instance)  # Should not raise
    
    def test_batch_rejects_disallowed_failure_capture(self):
        """Batch mode rejects non-abort/recover failure capture modes."""
        rule = CheckInterfaceBlock(context="interface")
        instance = InterfaceModel(
            analysis_drivers=AnalysisDriversModel(drivers=["simulator.py"]),
            concurrency=ConcurrencyModel(batch=BatchConcurrencyModel()),
            failure_capture=FailureCaptureContinuation()
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "failure_capture" in str(exc_info.value)
        assert "continuation" in str(exc_info.value)
    
    def test_batch_rejects_retry_failure_capture(self):
        """Batch mode rejects retry failure capture mode."""
        rule = CheckInterfaceBlock(context="interface")
        instance = InterfaceModel(
            analysis_drivers=AnalysisDriversModel(drivers=["simulator.py"]),
            concurrency=ConcurrencyModel(batch=BatchConcurrencyModel()),
            failure_capture=FailureCaptureRetry()
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "retry" in str(exc_info.value)
    
    def test_batch_allows_abort(self):
        """Batch mode allows abort failure capture."""
        rule = CheckInterfaceBlock(context="interface")
        instance = InterfaceModel(
            analysis_drivers=AnalysisDriversModel(drivers=["simulator.py"]),
            concurrency=ConcurrencyModel(batch=BatchConcurrencyModel()),
            failure_capture=FailureCaptureAbort()
        )
        rule(instance)  # Should not raise
    
    def test_batch_allows_recover(self):
        """Batch mode allows recover failure capture."""
        rule = CheckInterfaceBlock(context="interface")
        instance = InterfaceModel(
            analysis_drivers=AnalysisDriversModel(drivers=["simulator.py"]),
            concurrency=ConcurrencyModel(batch=BatchConcurrencyModel()),
            failure_capture=FailureCaptureRecover()
        )
        rule(instance)  # Should not raise
    
    # def test_async_rejects_both_concurrency_one(self):
    #     """Async rejects both evaluation_concurrency and analysis_concurrency = 1."""
    #     rule = CheckInterfaceBlock(context="interface")
    #     instance = InterfaceModel(
    #         analysis_drivers=AnalysisDriversModel(drivers=["simulator.py"]),
    #         concurrency=ConcurrencyModel(
    #             asynchronous=AsyncConcurrencyModel(
    #                 evaluation_concurrency=1,
    #                 analysis_concurrency=1
    #             )
    #         )
    #     )
        
    #     with pytest.raises(ValueError) as exc_info:
    #         rule(instance)
        
    #     assert "evaluation_concurrency" in str(exc_info.value)
    #     assert "analysis_concurrency" in str(exc_info.value)
    
    def test_async_allows_higher_concurrency(self):
        """Async allows concurrency values > 1."""
        rule = CheckInterfaceBlock(context="interface")
        instance = InterfaceModel(
            analysis_drivers=AnalysisDriversModel(drivers=["simulator.py"]),
            concurrency=ConcurrencyModel(
                asynchronous=AsyncConcurrencyModel(
                    evaluation_concurrency=4,
                    analysis_concurrency=2
                )
            )
        )
        rule(instance)  # Should not raise
    
    def test_async_allows_one_concurrency_one(self):
        """Async allows one concurrency = 1 if the other is different."""
        rule = CheckInterfaceBlock(context="interface")
        instance = InterfaceModel(
            analysis_drivers=AnalysisDriversModel(drivers=["simulator.py"]),
            concurrency=ConcurrencyModel(
                asynchronous=AsyncConcurrencyModel(
                    evaluation_concurrency=1,
                    analysis_concurrency=4
                )
            )
        )
        rule(instance)  # Should not raise


class TestCheckAnalysisDrivers:
    """Tests for CheckAnalysisDrivers validation rule."""

    def test_passes_with_one_driver(self):
        rule = CheckAnalysisDrivers(context="analysis_drivers")
        instance = AnalysisDriversModel(drivers=["simulator.py"])
        rule(instance)

    def test_rejects_empty_drivers(self):
        rule = CheckAnalysisDrivers(context="analysis_drivers")
        instance = AnalysisDriversModel(drivers=[])

        with pytest.raises(ValueError) as exc_info:
            rule(instance)

        assert "at least one driver" in str(exc_info.value).lower()

    def test_passes_with_even_analysis_components(self):
        rule = CheckAnalysisDrivers(context="analysis_drivers")
        instance = AnalysisDriversModel(
            drivers=["d1", "d2"],
            analysis_components=["c1", "c2", "c3", "c4"]
        )
        rule(instance)

    def test_rejects_uneven_analysis_components(self):
        rule = CheckAnalysisDrivers(context="analysis_drivers")
        instance = AnalysisDriversModel(
            drivers=["d1", "d2"],
            analysis_components=["c1", "c2", "c3"]
        )

        with pytest.raises(ValueError) as exc_info:
            rule(instance)

        assert "evenly divisible" in str(exc_info.value).lower()


# ============================================================================
# CheckResponseDescriptors tests
# ============================================================================

class TestCheckResponseDescriptors:
    """Tests for CheckResponseDescriptors validation rule."""
    
    def test_skips_when_descriptors_provided(self):
        """Should skip when descriptors are already set."""
        rule = CheckResponseDescriptors(context="responses")
        instance = ResponsesConfig(
            descriptors=["custom1", "custom2"],
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(count=2)
            )
        )
        
        rule(instance)
        assert instance.descriptors == ["custom1", "custom2"]
    
    def test_generates_calibration_terms_labels(self):
        """Should generate least_sq_term labels for calibration."""
        rule = CheckResponseDescriptors(context="responses")
        instance = ResponsesConfig(
            response_type=CalibrationTerms(
                calibration_terms=CalibrationTermsConfig(
                    count=2,
                    nonlinear_inequality_constraints=NonlinearInequalityConstraints(count=1)
                )
            )
        )
        
        rule(instance)
        assert instance.descriptors == ["least_sq_term_1", "least_sq_term_2", "nln_ineq_con_1"]
    
    def test_generates_single_objective_label(self):
        """Single objective should have no numeric suffix."""
        rule = CheckResponseDescriptors(context="responses")
        instance = ResponsesConfig(
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(count=1)
            )
        )
        
        rule(instance)
        assert instance.descriptors == ["obj_fn_1"]
    
    def test_generates_multiple_objective_labels(self):
        """Multiple objectives should have numeric suffixes."""
        rule = CheckResponseDescriptors(context="responses")
        instance = ResponsesConfig(
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(count=3)
            )
        )
        
        rule(instance)
        assert instance.descriptors == ["obj_fn_1", "obj_fn_2", "obj_fn_3"]
    
    def test_generates_response_function_labels(self):
        """Should generate response_fn labels."""
        rule = CheckResponseDescriptors(context="responses")
        instance = ResponsesConfig(
            response_type=ResponseFunctions(
                response_functions=ResponseFunctionsConfig(count=2)
            )
        )
        
        rule(instance)
        assert instance.descriptors == ["response_fn_1", "response_fn_2"]
    
    def test_generates_full_optimization_labels(self):
        """Should generate labels for objective + constraints."""
        rule = CheckResponseDescriptors(context="responses")
        instance = ResponsesConfig(
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(
                    count=1,
                    nonlinear_inequality_constraints=NonlinearInequalityConstraints(count=2),
                    nonlinear_equality_constraints=NonlinearEqualityConstraints(count=1)
                )
            )
        )
        
        rule(instance)
        assert instance.descriptors == ["obj_fn_1", "nln_ineq_con_1", "nln_ineq_con_2", "nln_eq_con_1"]


# ============================================================================
# CheckFdGradientStepSize tests
# ============================================================================

class TestCheckFdGradientStepSize:
    """Tests for CheckFdGradientStepSize validation rule."""
    
    def test_passes_for_dakota_source(self):
        """Dakota source allows multiple step sizes."""
        rule = CheckFdGradientStepSize(context="responses")
        instance = ResponsesConfig(
            gradient_type=NumericalGradients(
                numerical_gradients=NumericalGradientsConfig(
                    method_source=MixedGradientsMethodSourceDakota(),
                    fd_step_size=[0.001, 0.002, 0.003]
                )
            ),
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(count=3)
            )
        )
        
        rule(instance)  # Should not raise
    
    def test_passes_for_vendor_single_step(self):
        """Vendor with single step size should pass."""
        rule = CheckFdGradientStepSize(context="responses")
        instance = ResponsesConfig(
            gradient_type=NumericalGradients(
                numerical_gradients=NumericalGradientsConfig(
                    method_source=MixedGradientsMethodSourceVendor(),
                    fd_step_size=[0.001]
                )
            ),
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(count=3)
            )
        )
        
        rule(instance)  # Should not raise
    
    def test_raises_for_vendor_multiple_steps(self):
        """Vendor gradients with multiple step sizes should raise."""
        rule = CheckFdGradientStepSize(context="responses")
        instance = ResponsesConfig(
            gradient_type=NumericalGradients(
                numerical_gradients=NumericalGradientsConfig(
                    method_source=MixedGradientsMethodSourceVendor(),
                    fd_step_size=[0.001, 0.002]
                )
            ),
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(count=3)
            )
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "vendor" in str(exc_info.value)


# ============================================================================
# CheckMixedGradients tests
# ============================================================================

class TestCheckMixedGradients:
    """Tests for CheckMixedGradients validation rule."""
    
    def test_skips_when_not_mixed(self):
        """Should skip when gradient_type is not MixedGradients."""
        rule = CheckMixedGradients(context="responses")
        instance = ResponsesConfig(
            gradient_type=NumericalGradients(
                numerical_gradients=NumericalGradientsConfig()
            ),
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(count=3)
            )
        )
        
        rule(instance)  # Should not raise
    
    def test_passes_with_valid_coverage(self):
        """Valid coverage of all functions should pass."""
        rule = CheckMixedGradients(context="responses")
        instance = ResponsesConfig(
            gradient_type=MixedGradients(
                mixed_gradients=MixedGradientsConfig(
                    id_analytic_gradients=[1, 3],
                    id_numerical_gradients=[2]
                )
            ),
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(count=3)
            )
        )
        
        rule(instance)  # Should not raise
    
    def test_raises_on_out_of_range_id(self):
        """ID outside valid range should raise."""
        rule = CheckMixedGradients(context="responses")
        instance = ResponsesConfig(
            gradient_type=MixedGradients(
                mixed_gradients=MixedGradientsConfig(
                    id_analytic_gradients=[1, 4],  # 4 is out of range
                    id_numerical_gradients=[2]
                )
            ),
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(count=3)
            )
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "1 and 3" in str(exc_info.value)
    
    def test_raises_on_missing_function(self):
        """Missing function in coverage should raise."""
        rule = CheckMixedGradients(context="responses")
        instance = ResponsesConfig(
            gradient_type=MixedGradients(
                mixed_gradients=MixedGradientsConfig(
                    id_analytic_gradients=[1],
                    id_numerical_gradients=[2]
                    # Function 3 is missing
                )
            ),
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(count=3)
            )
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "Function 3" in str(exc_info.value)
        assert "missing" in str(exc_info.value)
    
    def test_raises_on_duplicate_function(self):
        """Duplicate function in coverage should raise."""
        rule = CheckMixedGradients(context="responses")
        instance = ResponsesConfig(
            gradient_type=MixedGradients(
                mixed_gradients=MixedGradientsConfig(
                    id_analytic_gradients=[1, 2],
                    id_numerical_gradients=[2, 3]  # 2 is duplicated
                )
            ),
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(count=3)
            )
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "Function 2" in str(exc_info.value)
        assert "replicated" in str(exc_info.value)


# ============================================================================
# CheckMixedHessians tests
# ============================================================================

class TestCheckMixedHessians:
    """Tests for CheckMixedHessians validation rule."""
    
    def test_skips_when_not_mixed(self):
        """Should skip when hessian_type is not MixedHessians."""
        rule = CheckMixedHessians(context="responses")
        instance = ResponsesConfig(
            hessian_type=NoHessians(),
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(count=3)
            )
        )
        
        rule(instance)  # Should not raise
    
    def test_passes_with_valid_coverage(self):
        """Valid coverage across all three ID sets should pass."""
        rule = CheckMixedHessians(context="responses")
        instance = ResponsesConfig(
            hessian_type=MixedHessians(
                mixed_hessians=MixedHessiansConfig(
                    id_analytic_hessians=[1],
                    id_numerical_hessians=IdNumericalHessians(values=[2, 3]),
                    id_quasi_hessians=IdQuasiHessians(values=[4])
                )
            ),
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(count=4)
            )
        )
        
        rule(instance)  # Should not raise
    
    def test_raises_on_missing_function(self):
        """Missing function in coverage should raise."""
        rule = CheckMixedHessians(context="responses")
        instance = ResponsesConfig(
            hessian_type=MixedHessians(
                mixed_hessians=MixedHessiansConfig(
                    id_analytic_hessians=[1],
                    id_numerical_hessians=IdNumericalHessians(values=[2])
                    # Function 3 is missing
                )
            ),
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(count=3)
            )
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "Function 3" in str(exc_info.value)
        assert "missing" in str(exc_info.value)


# ============================================================================
# DefaultInequalityBounds tests
# ============================================================================

class TestDefaultInequalityBounds:
    """Tests for DefaultInequalityBounds validation rule."""
    
    def test_sets_inequality_bounds(self):
        """Should set default bounds when not provided."""
        rule = DefaultInequalityBounds(context="constraints")
        instance = NonlinearInequalityConstraints(count=3)
        
        rule(instance)
        
        assert instance.lower_bounds == [float('-inf')] * 3
        assert instance.upper_bounds == [0.0] * 3
    
    def test_skips_when_already_set(self):
        """Should skip if bounds already provided."""
        rule = DefaultInequalityBounds(context="constraints")
        instance = NonlinearInequalityConstraints(
            count=2,
            lower_bounds=[-10.0, -20.0],
            upper_bounds=[5.0, 10.0]
        )
        
        rule(instance)
        
        assert instance.lower_bounds == [-10.0, -20.0]
        assert instance.upper_bounds == [5.0, 10.0]
    
    def test_skips_when_count_zero(self):
        """Should skip when count is zero."""
        rule = DefaultInequalityBounds(context="constraints")
        instance = NonlinearInequalityConstraints(count=0)
        
        rule(instance)
        
        assert instance.lower_bounds is None
        assert instance.upper_bounds is None


# ============================================================================
# DefaultEqualityTargets tests
# ============================================================================

class TestDefaultEqualityTargets:
    """Tests for DefaultEqualityTargets validation rule."""
    
    def test_sets_equality_targets(self):
        """Should set default targets when not provided."""
        rule = DefaultEqualityTargets(context="constraints")
        instance = NonlinearEqualityConstraints(count=3)
        
        rule(instance)
        
        assert instance.targets == [0.0] * 3
    
    def test_skips_when_already_set(self):
        """Should skip if targets already provided."""
        rule = DefaultEqualityTargets(context="constraints")
        instance = NonlinearEqualityConstraints(
            count=2,
            targets=[1.0, 2.0]
        )
        
        rule(instance)
        
        assert instance.targets == [1.0, 2.0]
    
    def test_skips_when_count_zero(self):
        """Should skip when count is zero."""
        rule = DefaultEqualityTargets(context="constraints")
        instance = NonlinearEqualityConstraints(count=0)
        
        rule(instance)
        
        assert instance.targets is None


# ============================================================================
# CheckResponseDescriptorsLength tests
# ============================================================================

class TestCheckResponseDescriptorsLength:
    """Tests for CheckResponseDescriptorsLength validation rule."""
    
    def test_skips_when_no_descriptors(self):
        """Should skip when descriptors are not provided."""
        rule = CheckResponseDescriptorsLength(context="responses")
        instance = ResponsesConfig(
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(count=3)
            )
        )
        
        rule(instance)  # Should not raise
    
    def test_passes_when_length_matches(self):
        """Should pass when descriptors length matches function count."""
        rule = CheckResponseDescriptorsLength(context="responses")
        instance = ResponsesConfig(
            descriptors=["f1", "f2", "f3"],
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(count=3)
            )
        )
        
        rule(instance)  # Should not raise
    
    def test_passes_with_constraints(self):
        """Should count objectives + constraints correctly."""
        rule = CheckResponseDescriptorsLength(context="responses")
        instance = ResponsesConfig(
            descriptors=["obj", "c1", "c2"],
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(
                    count=1,
                    nonlinear_inequality_constraints=NonlinearInequalityConstraints(count=2)
                )
            )
        )
        
        rule(instance)  # Should not raise
    
    def test_raises_when_length_mismatch(self):
        """Should raise when descriptors length doesn't match."""
        rule = CheckResponseDescriptorsLength(context="responses")
        instance = ResponsesConfig(
            descriptors=["f1", "f2"],  # Only 2, but we have 3
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(count=3)
            )
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "2" in str(exc_info.value)
        assert "3" in str(exc_info.value)


# ============================================================================
# CheckResponseDescriptorsValid tests
# ============================================================================

class TestCheckDescriptorsValid:
    """Tests for CheckDescriptorsValid unified validation rule."""
    
    def test_skips_when_no_descriptors(self):
        """Should skip when descriptors are not provided."""
        rule = CheckDescriptorsValid(context="responses", check_uniqueness=True)
        instance = ResponsesConfig(
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(count=3)
            )
        )
        
        rule(instance)  # Should not raise
    
    def test_passes_valid_descriptors(self):
        """Should pass with valid descriptors."""
        rule = CheckDescriptorsValid(context="responses", check_uniqueness=True)
        instance = ResponsesConfig(
            descriptors=["obj_fn_1", "constraint_A", "_private"],
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(count=3)
            )
        )
        
        rule(instance)  # Should not raise
    
    def test_passes_special_characters(self):
        """Should pass with special characters like hyphens."""
        rule = CheckDescriptorsValid(context="responses", check_uniqueness=True)
        instance = ResponsesConfig(
            descriptors=["obj-fn-1", "constraint.A", "name#2"],
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(count=3)
            )
        )
        
        rule(instance)  # Should not raise
    
    def test_passes_starts_with_digit_but_not_number(self):
        """Should pass when descriptor starts with digit but isn't a pure number."""
        rule = CheckDescriptorsValid(context="responses", check_uniqueness=True)
        instance = ResponsesConfig(
            descriptors=["2bad", "3rd_place", "4ever"],
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(count=3)
            )
        )
        
        rule(instance)  # Should not raise - these are not pure numbers
    
    def test_raises_on_duplicate_when_uniqueness_checked(self):
        """Should raise on duplicate descriptors when check_uniqueness=True."""
        rule = CheckDescriptorsValid(context="responses", check_uniqueness=True)
        instance = ResponsesConfig(
            descriptors=["f1", "f2", "f1"],  # f1 is duplicated
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(count=3)
            )
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "Duplicate" in str(exc_info.value)
        assert "f1" in str(exc_info.value)
    
    def test_allows_duplicate_when_uniqueness_not_checked(self):
        """Should allow duplicates when check_uniqueness=False."""
        rule = CheckDescriptorsValid(context="variables", check_uniqueness=False)
        instance = ContinuousDesignModel(
            count=3,
            descriptors=["x1", "x1", "x1"]  # All same - allowed without uniqueness check
        )
        
        rule(instance)  # Should not raise
    
    def test_raises_on_pure_number(self):
        """Should raise when descriptor is a pure number."""
        rule = CheckDescriptorsValid(context="responses", check_uniqueness=True)
        instance = ResponsesConfig(
            descriptors=["f1", "123", "f3"],
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(count=3)
            )
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "123" in str(exc_info.value)
        assert "number" in str(exc_info.value)
    
    def test_raises_on_float_number(self):
        """Should raise when descriptor is a floating-point number."""
        rule = CheckDescriptorsValid(context="responses", check_uniqueness=True)
        instance = ResponsesConfig(
            descriptors=["f1", "3.14159", "f3"],
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(count=3)
            )
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "3.14159" in str(exc_info.value)
        assert "number" in str(exc_info.value)
    
    def test_raises_on_scientific_notation(self):
        """Should raise when descriptor is scientific notation."""
        rule = CheckDescriptorsValid(context="responses", check_uniqueness=True)
        instance = ResponsesConfig(
            descriptors=["f1", "1e10", "f3"],
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(count=3)
            )
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "1e10" in str(exc_info.value)
        assert "number" in str(exc_info.value)
    
    def test_raises_on_nan_inf(self):
        """Should raise when descriptor is nan or inf."""
        rule = CheckDescriptorsValid(context="responses", check_uniqueness=True)
        
        for num_str in ["nan", "NaN", "inf", "INF", "-inf", "+infinity"]:
            instance = ResponsesConfig(
                descriptors=["f1", num_str, "f3"],
                response_type=ObjectiveFunctions(
                    objective_functions=ObjectiveFunctionsConfig(count=3)
                )
            )
            
            with pytest.raises(ValueError) as exc_info:
                rule(instance)
            
            assert "number" in str(exc_info.value)
    
    def test_raises_on_whitespace(self):
        """Should raise when descriptor contains whitespace."""
        rule = CheckDescriptorsValid(context="responses", check_uniqueness=True)
        instance = ResponsesConfig(
            descriptors=["f1", "bad name", "f3"],
            response_type=ObjectiveFunctions(
                objective_functions=ObjectiveFunctionsConfig(count=3)
            )
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "whitespace" in str(exc_info.value)


# ============================================================================
# CheckPermittedValues tests
# ============================================================================

class TestCheckPermittedValues:
    """Tests for CheckPermittedValues generic validation rule."""
    
    def test_skips_when_not_provided(self):
        """Should skip when scale_types not provided."""
        rule = CheckPermittedValues(
            context="objective_functions",
            field_name="primary_scale_types",
            permitted_values=["value", "log", "none"]
        )
        instance = ObjectiveFunctionsConfig(count=3)
        
        rule(instance)  # Should not raise
    
    def test_passes_valid_types(self):
        """Should pass with valid scale types."""
        rule = CheckPermittedValues(
            context="objective_functions",
            field_name="primary_scale_types",
            permitted_values=["value", "log", "none"]
        )
        instance = ObjectiveFunctionsConfig(
            count=2,
            primary_scale_types=["value", "log"]
        )
        
        rule(instance)  # Should not raise
    
    def test_raises_on_invalid(self):
        """Should raise on invalid scale type."""
        rule = CheckPermittedValues(
            context="objective_functions",
            field_name="primary_scale_types",
            permitted_values=["value", "log", "none"]
        )
        instance = ObjectiveFunctionsConfig(
            count=2,
            primary_scale_types=["value", "invalid"]
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "invalid" in str(exc_info.value)
        assert "primary_scale_types[1]" in str(exc_info.value)
    
    def test_constraint_scale_types(self):
        """Should validate constraint scale types."""
        rule = CheckPermittedValues(
            context="constraints",
            field_name="scale_types",
            permitted_values=["value", "auto", "log", "none"]
        )
        instance = NonlinearInequalityConstraints(
            count=4,
            scale_types=["value", "auto", "log", "none"]
        )
        
        rule(instance)  # Should not raise
    
    def test_constraint_scale_types_invalid(self):
        """Should reject invalid constraint scale type."""
        rule = CheckPermittedValues(
            context="constraints",
            field_name="scale_types",
            permitted_values=["value", "auto", "log", "none"]
        )
        instance = NonlinearInequalityConstraints(
            count=2,
            scale_types=["value", "bad"]
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "bad" in str(exc_info.value)


# ============================================================================
# CheckScalesRequired tests
# ============================================================================

class TestCheckScalesRequired:
    """Tests for CheckScalesRequired generic validation rule."""
    
    def test_skips_when_no_scale_types(self):
        """Should skip when scale_types not provided."""
        rule = CheckScalesRequired(
            context="continuous_design",
            scale_types_field="scale_types",
            scales_field="scales"
        )
        instance = ContinuousDesignModel(count=3)
        
        rule(instance)  # Should not raise
    
    def test_passes_when_no_value_type(self):
        """Should pass when scale_types doesn't include 'value'."""
        rule = CheckScalesRequired(
            context="continuous_design",
            scale_types_field="scale_types",
            scales_field="scales"
        )
        instance = ContinuousDesignModel(
            count=2,
            scale_types=["auto", "log"]
        )
        
        rule(instance)  # Should not raise
    
    def test_passes_when_scales_provided(self):
        """Should pass when value type has scales."""
        rule = CheckScalesRequired(
            context="continuous_design",
            scale_types_field="scale_types",
            scales_field="scales"
        )
        instance = ContinuousDesignModel(
            count=2,
            scale_types=["value", "auto"],
            scales=[1.0, 2.0]
        )
        
        rule(instance)  # Should not raise
    
    def test_raises_when_value_without_scales(self):
        """Should raise when 'value' type but no scales."""
        rule = CheckScalesRequired(
            context="continuous_design",
            scale_types_field="scale_types",
            scales_field="scales"
        )
        instance = ContinuousDesignModel(
            count=2,
            scale_types=["value", "auto"]
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "value" in str(exc_info.value)
        assert "scales" in str(exc_info.value)


# ============================================================================
# CheckConstraintBoundsOrdering tests
# ============================================================================

class TestCheckConstraintBoundsOrdering:
    """Tests for CheckConstraintBoundsOrdering validation rule."""
    
    def test_skips_when_no_bounds(self):
        """Should skip when bounds not provided."""
        rule = CheckConstraintBoundsOrdering(context="constraints")
        instance = NonlinearInequalityConstraints(count=3)
        
        rule(instance)  # Should not raise
    
    def test_passes_valid_ordering(self):
        """Should pass when lower < upper."""
        rule = CheckConstraintBoundsOrdering(context="constraints")
        instance = NonlinearInequalityConstraints(
            count=3,
            lower_bounds=[-10.0, -5.0, 0.0],
            upper_bounds=[0.0, 5.0, 10.0]
        )
        
        rule(instance)  # Should not raise
    
    def test_passes_equal_bounds(self):
        """Should pass when lower == upper (equality constraint behavior)."""
        rule = CheckConstraintBoundsOrdering(context="constraints")
        instance = NonlinearInequalityConstraints(
            count=2,
            lower_bounds=[0.0, 5.0],
            upper_bounds=[0.0, 5.0]
        )
        
        rule(instance)  # Should not raise
    
    def test_raises_when_lower_exceeds_upper(self):
        """Should raise when lower > upper."""
        rule = CheckConstraintBoundsOrdering(context="constraints")
        instance = NonlinearInequalityConstraints(
            count=3,
            lower_bounds=[-10.0, 10.0, 0.0],  # 10.0 > 5.0
            upper_bounds=[0.0, 5.0, 10.0]
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "lower_bounds[1]" in str(exc_info.value)
        assert "10" in str(exc_info.value)
        assert "exceeds" in str(exc_info.value)


# ============================================================================
# Variable Validator Test Fixtures
# ============================================================================

class ContinuousDesignModel(BaseModel):
    """Model for testing continuous_design validators."""
    count: int = 3
    descriptors: Optional[List[str]] = None
    lower_bounds: Optional[List[float]] = None
    upper_bounds: Optional[List[float]] = None
    scale_types: Optional[List[str]] = None
    scales: Optional[List[float]] = None
    
    model_config = {'extra': 'forbid'}


# ============================================================================
# CheckVariableDescriptorsLength tests
# ============================================================================

class TestCheckVariableDescriptorsLength:
    """Tests for CheckVariableDescriptorsLength validation rule."""
    
    def test_skips_when_no_descriptors(self):
        """Should skip when descriptors are not provided."""
        rule = CheckVariableDescriptorsLength(context="continuous_design")
        instance = ContinuousDesignModel(count=3)
        
        rule(instance)  # Should not raise
    
    def test_passes_when_length_matches(self):
        """Should pass when descriptors length matches count."""
        rule = CheckVariableDescriptorsLength(context="continuous_design")
        instance = ContinuousDesignModel(
            count=3,
            descriptors=["x1", "x2", "x3"]
        )
        
        rule(instance)  # Should not raise
    
    def test_raises_when_length_mismatch(self):
        """Should raise when descriptors length doesn't match count."""
        rule = CheckVariableDescriptorsLength(context="continuous_design")
        instance = ContinuousDesignModel(
            count=3,
            descriptors=["x1", "x2"]  # Only 2, but count is 3
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "2" in str(exc_info.value)
        assert "3" in str(exc_info.value)


# ============================================================================
# DefaultVariableDescriptors tests
# ============================================================================

class TestDefaultVariableDescriptors:
    """Tests for DefaultVariableDescriptors validation rule."""
    
    def test_skips_when_already_set(self):
        """Should skip when descriptors already provided."""
        rule = DefaultVariableDescriptors(context="continuous_design", prefix="cdv_")
        instance = ContinuousDesignModel(
            count=2,
            descriptors=["custom1", "custom2"]
        )
        
        rule(instance)
        assert instance.descriptors == ["custom1", "custom2"]
    
    def test_generates_default_descriptors(self):
        """Should generate default descriptors."""
        rule = DefaultVariableDescriptors(context="continuous_design", prefix="cdv_")
        instance = ContinuousDesignModel(count=3)
        
        rule(instance)
        assert instance.descriptors == ["cdv_1", "cdv_2", "cdv_3"]


# ============================================================================
# CheckVariableBoundsOrdering tests
# ============================================================================

class TestCheckVariableBoundsOrdering:
    """Tests for CheckVariableBoundsOrdering validation rule."""
    
    def test_skips_when_no_bounds(self):
        """Should skip when bounds not provided."""
        rule = CheckVariableBoundsOrdering(context="continuous_design")
        instance = ContinuousDesignModel(count=3)
        
        rule(instance)  # Should not raise
    
    def test_passes_valid_ordering(self):
        """Should pass when lower < upper."""
        rule = CheckVariableBoundsOrdering(context="continuous_design")
        instance = ContinuousDesignModel(
            count=3,
            lower_bounds=[-10.0, 0.0, 1.0],
            upper_bounds=[0.0, 5.0, 10.0]
        )
        
        rule(instance)  # Should not raise
    
    def test_raises_when_lower_exceeds_upper(self):
        """Should raise when lower > upper."""
        rule = CheckVariableBoundsOrdering(context="continuous_design")
        instance = ContinuousDesignModel(
            count=3,
            lower_bounds=[-10.0, 10.0, 1.0],  # 10.0 > 5.0
            upper_bounds=[0.0, 5.0, 10.0]
        )
        
        with pytest.raises(ValueError) as exc_info:
            rule(instance)
        
        assert "lower_bounds[1]" in str(exc_info.value)
        assert "exceeds" in str(exc_info.value)


# ============================================================================
# Computed field tests
# ============================================================================

class HistogramBinModel(BaseModel):
    count: int
    pairs_per_variable: List[int] | None = None
    abscissas: List[float]

    model_config = {'extra': 'forbid'}


class HistogramPointRealModel(BaseModel):
    count: int
    pairs_per_variable: List[int] | None = None
    abscissas: List[float]

    model_config = {'extra': 'forbid'}


class ContinuousIntervalModel(BaseModel):
    count: int
    num_intervals: List[int] | None = None
    lower_bounds: List[float]
    upper_bounds: List[float]

    model_config = {'extra': 'forbid'}


class DiscreteSetStrModel(BaseModel):
    count: int
    elements_per_variable: List[int] | None = None
    elements: List[str]

    model_config = {'extra': 'forbid'}


class TestComputedFields:
    """Tests for computed field helpers and schema metadata."""

    def test_computed_field_schema(self):
        schema = computed_field_schema("histogram_bin_lower_bounds")
        assert schema == {
            "json_schema_extra": {
                "x-computed-field": {
                    "function": "histogram_bin_lower_bounds"
                }
            }
        }

    def test_histogram_bin_bounds_python_fallback(self, monkeypatch):
        monkeypatch.setattr(computed_fields_module, "_USE_CPP", False)
        instance = HistogramBinModel(
            count=2,
            pairs_per_variable=[2, 3],
            abscissas=[0.1, 0.4, 1.0, 1.5, 2.0],
        )

        assert HistogramBinBounds.lower_bounds(instance) == [0.1, 1.0]
        assert HistogramBinBounds.upper_bounds(instance) == [0.4, 2.0]

    def test_histogram_point_real_bounds_python_fallback_with_equal_partition(self, monkeypatch):
        monkeypatch.setattr(computed_fields_module, "_USE_CPP", False)
        instance = HistogramPointRealModel(
            count=2,
            pairs_per_variable=None,
            abscissas=[3.0, 1.0, 10.0, 7.0],
        )

        assert HistogramPointRealBounds.lower_bounds(instance) == [1.0, 7.0]
        assert HistogramPointRealBounds.upper_bounds(instance) == [3.0, 10.0]

    def test_continuous_interval_bounds_python_fallback(self, monkeypatch):
        monkeypatch.setattr(computed_fields_module, "_USE_CPP", False)
        instance = ContinuousIntervalModel(
            count=2,
            num_intervals=[2, 3],
            lower_bounds=[-1.0, -3.0, 10.0, 8.0, 9.0],
            upper_bounds=[4.0, 2.0, 11.0, 14.0, 12.0],
        )

        assert ContinuousIntervalBounds.inferred_lower_bounds(instance) == [-3.0, 8.0]
        assert ContinuousIntervalBounds.inferred_upper_bounds(instance) == [4.0, 14.0]

    def test_discrete_set_string_bounds_python_fallback(self, monkeypatch):
        monkeypatch.setattr(computed_fields_module, "_USE_CPP", False)
        instance = DiscreteSetStrModel(
            count=2,
            elements_per_variable=[3, 2],
            elements=["pear", "apple", "orange", "zulu", "bravo"],
        )

        assert DiscreteSetStrBounds.lower_bounds(instance) == ["apple", "bravo"]
        assert DiscreteSetStrBounds.upper_bounds(instance) == ["pear", "zulu"]


# ============================================================================
# Backend status test
# ============================================================================

class TestBackendStatus:
    """Tests for verifying backend status."""
    
    def test_cpp_backend_status(self):
        """Report which backend is being used."""
        if CPP_BACKEND_AVAILABLE:
            print("\n*** Using C++ backend (dakota_validation_cpp) ***")
            assert _USE_CPP is True
        else:
            print("\n*** Using pure Python fallback ***")
            assert _USE_CPP is False


def test_generated_models_reject_internal_only_fields():
    install_python = os.path.normpath(
        os.path.join(os.path.dirname(__file__), "..", "..", "install", "lib", "python")
    )
    original_path = list(sys.path)
    dakota_modules = {
        name: module
        for name, module in sys.modules.items()
        if name == "dakota" or name.startswith("dakota.")
    }

    try:
        for name in list(dakota_modules):
            sys.modules.pop(name, None)
        sys.path.insert(0, install_python)
        generated_variables = importlib.import_module("dakota.spec.variables")
        NormalUncertain = generated_variables.NormalUncertain

        with pytest.raises(ValueError, match="internal-only"):
            NormalUncertain(
                count=1,
                means=[1.0],
                std_deviations=[0.5],
                initial_point_user_provided=True,
            )

        with pytest.raises(ValueError, match="internal-only"):
            NormalUncertain(
                count=1,
                means=[1.0],
                std_deviations=[0.5],
                inferred_lower_bounds=[0.0],
            )

        LognormalUncertain = generated_variables.LognormalUncertain

        with pytest.raises(ValueError, match="internal-only"):
            LognormalUncertain(
                count=1,
                parameters={"option_1": {"means": [1.0], "std_deviations": [0.5]}},
                inferred_upper_bounds=[2.0],
            )
    finally:
        sys.path[:] = original_path
        for name in list(sys.modules):
            if name == "dakota" or name.startswith("dakota."):
                sys.modules.pop(name, None)
        sys.modules.update(dakota_modules)


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
