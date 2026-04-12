"""Generated Pydantic models for responses"""

from __future__ import annotations

from .base import DakotaBaseModel, DakotaField, DakotaFloat
from typing import Literal, Union
from typing import ClassVar, List
from .validation import ValidationRule
from .validation.rules import (
    CheckConstraintBoundsOrdering,
    CheckDescriptorsValid,
    CheckFdGradientStepSize,
    CheckMixedGradients,
    CheckMixedHessians,
    CheckPermittedValues,
    CheckResponseDescriptors,
    CheckResponseDescriptorsLength,
    CheckScalesRequired,
    CompareLength,
    CompareLengthOne,
    DefaultEqualityTargets,
    DefaultInequalityBounds,
)

# Cross-module model imports
from dakota.spec.shared.responses import (
    ResponseGradientsMixin,
    ResponseHessiansMixin,
    ResponseScalarDataFormatMixin,
)


class ObjectiveFunctionsNonlinearInequalityConstraints(DakotaBaseModel):
    """Generated model for ObjectiveFunctionsNonlinearInequalityConstraints"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="responses_objective_functions_nonlinear_inequality_constraints",
            list_field="lower_bounds",
            target_field="count",
        ),
        CompareLength(
            context="responses_objective_functions_nonlinear_inequality_constraints",
            list_field="upper_bounds",
            target_field="count",
        ),
        CompareLengthOne(
            context="responses_objective_functions_nonlinear_inequality_constraints",
            list_field="scale_types",
            target_field="count",
        ),
        CompareLengthOne(
            context="responses_objective_functions_nonlinear_inequality_constraints",
            list_field="scales",
            target_field="count",
        ),
        DefaultInequalityBounds(context="nonlinear_inequality_constraints"),
        CheckPermittedValues(
            context="nonlinear_inequality_constraints",
            field_name="scale_types",
            permitted_values=["value", "auto", "log", "none"],
        ),
        CheckScalesRequired(
            context="nonlinear_inequality_constraints",
            scale_types_field="scale_types",
            scales_field="scales",
        ),
        CheckConstraintBoundsOrdering(context="nonlinear_inequality_constraints"),
    ]

    count: int = DakotaField(
        ge=0,
        description="Group to specify nonlinear inequality constraints",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.num_nonlinear_inequality_constraints",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    lower_bounds: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Specify minimum values",
        dakota={
            "aliases": ["nonlinear_inequality_lower_bounds"],
            "materialization": [
                {
                    "ir_key": "responses.nonlinear_inequality_lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    upper_bounds: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Specify maximium values",
        dakota={
            "aliases": ["nonlinear_inequality_upper_bounds"],
            "materialization": [
                {
                    "ir_key": "responses.nonlinear_inequality_upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    scale_types: list[str] | None = DakotaField(
        default=None,
        description="How to scale each nonlinear constraint",
        dakota={
            "aliases": ["nonlinear_inequality_scale_types"],
            "materialization": [
                {
                    "ir_key": "responses.nonlinear_inequality_scale_types",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )
    scales: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Characteristic values to scale each nonlinear constraint",
        dakota={
            "aliases": ["nonlinear_inequality_scales"],
            "materialization": [
                {
                    "ir_key": "responses.nonlinear_inequality_scales",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )


class ObjectiveFunctionsNonlinearEqualityConstraints(DakotaBaseModel):
    """Generated model for ObjectiveFunctionsNonlinearEqualityConstraints"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="responses_objective_functions_nonlinear_equality_constraints",
            list_field="targets",
            target_field="count",
        ),
        CompareLengthOne(
            context="responses_objective_functions_nonlinear_equality_constraints",
            list_field="scale_types",
            target_field="count",
        ),
        CompareLengthOne(
            context="responses_objective_functions_nonlinear_equality_constraints",
            list_field="scales",
            target_field="count",
        ),
        DefaultEqualityTargets(context="nonlinear_equality_constraints"),
        CheckPermittedValues(
            context="nonlinear_equality_constraints",
            field_name="scale_types",
            permitted_values=["value", "auto", "log", "none"],
        ),
        CheckScalesRequired(
            context="nonlinear_equality_constraints",
            scale_types_field="scale_types",
            scales_field="scales",
        ),
    ]

    count: int = DakotaField(
        ge=0,
        description="Group to specify nonlinear equality constraints",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.num_nonlinear_equality_constraints",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    targets: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Target values for the nonlinear equality constraint",
        dakota={
            "aliases": ["nonlinear_equality_targets"],
            "materialization": [
                {
                    "ir_key": "responses.nonlinear_equality_targets",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    scale_types: list[str] | None = DakotaField(
        default=None,
        description="How to scale each nonlinear constraint",
        dakota={
            "aliases": ["nonlinear_equality_scale_types"],
            "materialization": [
                {
                    "ir_key": "responses.nonlinear_equality_scale_types",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )
    scales: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Characteristic values to scale each nonlinear constraint",
        dakota={
            "aliases": ["nonlinear_equality_scales"],
            "materialization": [
                {
                    "ir_key": "responses.nonlinear_equality_scales",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )


class FieldObjectives(DakotaBaseModel):
    """Generated model for FieldObjectives"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="responses_objective_functions_field_objectives",
            list_field="lengths",
            target_field="count",
        ),
    ]

    count: int = DakotaField(
        ge=0,
        description="Number of field objective functions",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.num_field_objectives",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    lengths: list[int] = DakotaField(
        description="Lengths of field responses",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.lengths",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )
    num_coordinates_per_field: list[int] | None = DakotaField(
        default=None,
        description="Number of independent coordinates for field responses",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.num_coordinates_per_field",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )
    read_field_coordinates: Literal[True] | None = DakotaField(
        default=None,
        description="Add context to data: flag to indicate that field coordinates should be read",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.read_field_coordinates",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class FieldCalibrationTerms(DakotaBaseModel):
    """Generated model for FieldCalibrationTerms"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="responses_calibration_terms_field_calibration_terms",
            list_field="lengths",
            target_field="count",
        ),
    ]

    count: int = DakotaField(
        ge=0,
        description="Number of field calibration terms",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.num_field_calibration_terms",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    lengths: list[int] = DakotaField(
        description="Lengths of field responses",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.lengths",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )
    num_coordinates_per_field: list[int] | None = DakotaField(
        default=None,
        description="Number of independent coordinates for field responses",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.num_coordinates_per_field",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )
    read_field_coordinates: Literal[True] | None = DakotaField(
        default=None,
        description="Add context to data: flag to indicate that field coordinates should be read",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.read_field_coordinates",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class ScalarDataFile(ResponseScalarDataFormatMixin):
    """Generated model for ScalarDataFile"""

    filename: str = DakotaField(
        description="Specify a scalar data file to complement field data files (mixed case)",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.scalar_data_filename",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class CalibrationDataFileConfig(ResponseScalarDataFormatMixin):
    """Generated model for CalibrationDataFileConfig"""

    filename: str = DakotaField(
        description="Supply scalar calibration data only",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.scalar_data_filename",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    num_experiments: int = DakotaField(
        default=1,
        ge=0,
        description="Add context to data: number of different experiments",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.num_experiments",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    num_config_variables: int | None = DakotaField(
        default=None,
        ge=0,
        description="Add context to data: number of configuration variables.",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.num_config_vars",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    experiment_variance_type: list[str] | None = DakotaField(
        default=None,
        description="Add context to data: specify the type of experimental error",
        dakota={
            "aliases": ["variance_type"],
            "materialization": [
                {
                    "ir_key": "responses.variance_type",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )


class CalibrationTermsNonlinearInequalityConstraints(DakotaBaseModel):
    """Generated model for CalibrationTermsNonlinearInequalityConstraints"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="responses_calibration_terms_nonlinear_inequality_constraints",
            list_field="lower_bounds",
            target_field="count",
        ),
        CompareLength(
            context="responses_calibration_terms_nonlinear_inequality_constraints",
            list_field="upper_bounds",
            target_field="count",
        ),
        CompareLengthOne(
            context="responses_calibration_terms_nonlinear_inequality_constraints",
            list_field="scale_types",
            target_field="count",
        ),
        CompareLengthOne(
            context="responses_calibration_terms_nonlinear_inequality_constraints",
            list_field="scales",
            target_field="count",
        ),
        DefaultInequalityBounds(context="nonlinear_inequality_constraints"),
        CheckPermittedValues(
            context="nonlinear_inequality_constraints",
            field_name="scale_types",
            permitted_values=["value", "auto", "log", "none"],
        ),
        CheckScalesRequired(
            context="nonlinear_inequality_constraints",
            scale_types_field="scale_types",
            scales_field="scales",
        ),
        CheckConstraintBoundsOrdering(context="nonlinear_inequality_constraints"),
    ]

    count: int = DakotaField(
        ge=0,
        description="Group to specify nonlinear inequality constraints",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.num_nonlinear_inequality_constraints",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    lower_bounds: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Specify minimum values",
        dakota={
            "aliases": ["nonlinear_inequality_lower_bounds"],
            "materialization": [
                {
                    "ir_key": "responses.nonlinear_inequality_lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    upper_bounds: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Specify maximium values",
        dakota={
            "aliases": ["nonlinear_inequality_upper_bounds"],
            "materialization": [
                {
                    "ir_key": "responses.nonlinear_inequality_upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    scale_types: list[str] | None = DakotaField(
        default=None,
        description="How to scale each nonlinear constraint",
        dakota={
            "aliases": ["nonlinear_inequality_scale_types"],
            "materialization": [
                {
                    "ir_key": "responses.nonlinear_inequality_scale_types",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )
    scales: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Characteristic values to scale each nonlinear constraint",
        dakota={
            "aliases": ["nonlinear_inequality_scales"],
            "materialization": [
                {
                    "ir_key": "responses.nonlinear_inequality_scales",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )


class CalibrationTermsNonlinearEqualityConstraints(DakotaBaseModel):
    """Generated model for CalibrationTermsNonlinearEqualityConstraints"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="responses_calibration_terms_nonlinear_equality_constraints",
            list_field="targets",
            target_field="count",
        ),
        CompareLengthOne(
            context="responses_calibration_terms_nonlinear_equality_constraints",
            list_field="scale_types",
            target_field="count",
        ),
        CompareLengthOne(
            context="responses_calibration_terms_nonlinear_equality_constraints",
            list_field="scales",
            target_field="count",
        ),
        DefaultEqualityTargets(context="nonlinear_equality_constraints"),
        CheckPermittedValues(
            context="nonlinear_equality_constraints",
            field_name="scale_types",
            permitted_values=["value", "auto", "log", "none"],
        ),
        CheckScalesRequired(
            context="nonlinear_equality_constraints",
            scale_types_field="scale_types",
            scales_field="scales",
        ),
    ]

    count: int = DakotaField(
        ge=0,
        description="Group to specify nonlinear equality constraints",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.num_nonlinear_equality_constraints",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    targets: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Target values for the nonlinear equality constraint",
        dakota={
            "aliases": ["nonlinear_equality_targets"],
            "materialization": [
                {
                    "ir_key": "responses.nonlinear_equality_targets",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    scale_types: list[str] | None = DakotaField(
        default=None,
        description="How to scale each nonlinear constraint",
        dakota={
            "aliases": ["nonlinear_equality_scale_types"],
            "materialization": [
                {
                    "ir_key": "responses.nonlinear_equality_scale_types",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )
    scales: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Characteristic values to scale each nonlinear constraint",
        dakota={
            "aliases": ["nonlinear_equality_scales"],
            "materialization": [
                {
                    "ir_key": "responses.nonlinear_equality_scales",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )


class FieldResponses(DakotaBaseModel):
    """Generated model for FieldResponses"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="responses_response_functions_field_responses",
            list_field="lengths",
            target_field="count",
        ),
    ]

    count: int = DakotaField(
        ge=0,
        description="Number of field responses functions",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.num_field_responses",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    lengths: list[int] = DakotaField(
        description="Lengths of field responses",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.lengths",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )
    num_coordinates_per_field: list[int] | None = DakotaField(
        default=None,
        description="Number of independent coordinates for field responses",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.num_coordinates_per_field",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )
    read_field_coordinates: Literal[True] | None = DakotaField(
        default=None,
        description="Add context to data: flag to indicate that field coordinates should be read",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.read_field_coordinates",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class ObjectiveFunctionsConfig(DakotaBaseModel):
    """Generated model for ObjectiveFunctionsConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="objective_functions", list_field="weights", target_field="count"
        ),
        CompareLengthOne(
            context="objective_functions", list_field="sense", target_field="count"
        ),
        CompareLengthOne(
            context="objective_functions",
            list_field="primary_scale_types",
            target_field="count",
        ),
        CheckPermittedValues(
            context="objective_functions",
            field_name="primary_scale_types",
            permitted_values=["value", "log", "none"],
        ),
        CheckScalesRequired(
            context="objective_functions",
            scale_types_field="primary_scale_types",
            scales_field="primary_scales",
        ),
    ]

    count: int = DakotaField(
        ge=0,
        description="Response type suitable for optimization",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.num_objective_functions",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    sense: list[str] | None = DakotaField(
        default=None,
        description="Whether to minimize or maximize each objective function",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.primary_response_fn_sense",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )
    primary_scale_types: list[str] | None = DakotaField(
        default=None,
        description="How to scale each objective function",
        dakota={
            "aliases": ["objective_function_scale_types"],
            "materialization": [
                {
                    "ir_key": "responses.primary_response_fn_scale_types",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )
    primary_scales: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Characteristic values to scale each objective function",
        dakota={
            "aliases": ["objective_function_scales"],
            "materialization": [
                {
                    "ir_key": "responses.primary_response_fn_scales",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    weights: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Specify weights for each objective function",
        dakota={
            "aliases": ["multi_objective_weights"],
            "materialization": [
                {
                    "ir_key": "responses.primary_response_fn_weights",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    nonlinear_inequality_constraints: (
        ObjectiveFunctionsNonlinearInequalityConstraints | None
    ) = DakotaField(
        default=None,
        description="Group to specify nonlinear inequality constraints",
        dakota={
            "argument": "count",
            "aliases": ["num_nonlinear_inequality_constraints"],
        },
    )
    nonlinear_equality_constraints: (
        ObjectiveFunctionsNonlinearEqualityConstraints | None
    ) = DakotaField(
        default=None,
        description="Group to specify nonlinear equality constraints",
        dakota={"argument": "count", "aliases": ["num_nonlinear_equality_constraints"]},
    )
    scalar_objectives: int | None = DakotaField(
        default=None,
        ge=0,
        description="Number of scalar objective functions",
        dakota={
            "aliases": ["num_scalar_objectives"],
            "materialization": [
                {
                    "ir_key": "responses.num_scalar_objectives",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ],
        },
    )
    field_objectives: FieldObjectives | None = DakotaField(
        default=None,
        description="Number of field objective functions",
        dakota={"argument": "count", "aliases": ["num_field_objectives"]},
    )


class CalibrationDataConfig(DakotaBaseModel):
    """Generated model for CalibrationDataConfig"""

    data_directory: str | None = DakotaField(
        default=None,
        description="Specify a directory containing the calibration field data files used with the mixed case specification",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.data_directory",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    num_experiments: int = DakotaField(
        default=1,
        ge=0,
        description="Add context to data: number of different experiments",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.num_experiments",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    num_config_variables: int | None = DakotaField(
        default=None,
        ge=0,
        description="Add context to data: number of configuration variables.",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.num_config_vars",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    experiment_variance_type: list[str] | None = DakotaField(
        default=None,
        description="Add context to data: specify the type of experimental error",
        dakota={
            "aliases": ["variance_type"],
            "materialization": [
                {
                    "ir_key": "responses.variance_type",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )
    scalar_data_file: ScalarDataFile | None = DakotaField(
        default=None,
        description="Specify a scalar data file to complement field data files (mixed case)",
        dakota={"argument": "filename"},
    )
    interpolate: Literal[True] | None = DakotaField(
        default=None,
        description="Flag to indicate interpolation of simulation values.",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.interpolate",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class CalibrationDataFile(DakotaBaseModel):
    """Generated model for CalibrationDataFile"""

    calibration_data_file: CalibrationDataFileConfig = DakotaField(
        default=...,
        description="Supply scalar calibration data only",
        dakota={"argument": "filename", "aliases": ["least_squares_data_file"]},
    )


class ResponseFunctionsConfig(DakotaBaseModel):
    """Generated model for ResponseFunctionsConfig"""

    count: int = DakotaField(
        ge=0,
        description="Generic response type",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.num_response_functions",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    scalar_responses: int | None = DakotaField(
        default=None,
        ge=0,
        description="Number of scalar response functions",
        dakota={
            "aliases": ["num_scalar_responses"],
            "materialization": [
                {
                    "ir_key": "responses.num_scalar_responses",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ],
        },
    )
    field_responses: FieldResponses | None = DakotaField(
        default=None,
        description="Number of field responses functions",
        dakota={"argument": "count", "aliases": ["num_field_responses"]},
    )


class ObjectiveFunctions(DakotaBaseModel):
    """Generated model for ObjectiveFunctions"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckPermittedValues(
            context="objective_functions",
            field_name="primary_scale_types",
            permitted_values=["value", "log", "none"],
        ),
        CheckScalesRequired(
            context="objective_functions",
            scale_types_field="primary_scale_types",
            scales_field="primary_scales",
        ),
    ]

    objective_functions: ObjectiveFunctionsConfig = DakotaField(
        default=...,
        description="Response type suitable for optimization",
        dakota={"argument": "count", "aliases": ["num_objective_functions"]},
    )


class CalibrationData(DakotaBaseModel):
    """Generated model for CalibrationData"""

    calibration_data: CalibrationDataConfig = DakotaField(
        description="Supply field or mixed field/scalar calibration data",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.calibration_data",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class ResponseFunctions(DakotaBaseModel):
    """Generated model for ResponseFunctions"""

    response_functions: ResponseFunctionsConfig = DakotaField(
        default=...,
        description="Generic response type",
        dakota={"argument": "count", "aliases": ["num_response_functions"]},
    )


class CalibrationTermsConfig(DakotaBaseModel):
    """Generated model for CalibrationTermsConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="calibration_terms", list_field="weights", target_field="count"
        ),
        CompareLengthOne(
            context="calibration_terms",
            list_field="simulation_variance",
            target_field="count",
        ),
        CompareLengthOne(
            context="calibration_terms",
            list_field="data_specification.calibration_data.experiment_variance_type",
            target_field="count",
        ),
        CompareLengthOne(
            context="calibration_terms",
            list_field="data_specification.calibration_data_file.experiment_variance_type",
            target_field="count",
        ),
    ]

    count: int = DakotaField(
        ge=0,
        description="Response type suitable for calibration or least squares",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.num_calibration_terms",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    scalar_calibration_terms: int | None = DakotaField(
        default=None,
        ge=0,
        description="Number of scalar calibration terms",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.num_scalar_calibration_terms",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    field_calibration_terms: FieldCalibrationTerms | None = DakotaField(
        default=None,
        description="Number of field calibration terms",
        dakota={"argument": "count"},
    )
    primary_scales: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Characteristic values to scale each calibration term",
        dakota={
            "aliases": ["calibration_term_scales", "least_squares_term_scales"],
            "materialization": [
                {
                    "ir_key": "responses.primary_response_fn_scales",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    weights: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Specify weights for each objective function",
        dakota={
            "aliases": ["calibration_weights", "least_squares_weights"],
            "materialization": [
                {
                    "ir_key": "responses.primary_response_fn_weights",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    data_specification: Union[CalibrationData, CalibrationDataFile] | None = (
        DakotaField(
            default=None,
            description="Calibration Data",
            dakota={"anchor": True, "union_pattern": 2},
        )
    )
    simulation_variance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Variance applied to simulation responses",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.simulation_variance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    nonlinear_inequality_constraints: (
        CalibrationTermsNonlinearInequalityConstraints | None
    ) = DakotaField(
        default=None,
        description="Group to specify nonlinear inequality constraints",
        dakota={
            "argument": "count",
            "aliases": ["num_nonlinear_inequality_constraints"],
        },
    )
    nonlinear_equality_constraints: (
        CalibrationTermsNonlinearEqualityConstraints | None
    ) = DakotaField(
        default=None,
        description="Group to specify nonlinear equality constraints",
        dakota={"argument": "count", "aliases": ["num_nonlinear_equality_constraints"]},
    )


class CalibrationTerms(DakotaBaseModel):
    """Generated model for CalibrationTerms"""

    calibration_terms: CalibrationTermsConfig = DakotaField(
        default=...,
        description="Response type suitable for calibration or least squares",
        dakota={
            "argument": "count",
            "aliases": ["least_squares_terms", "num_least_squares_terms"],
        },
    )


class ResponsesConfig(ResponseGradientsMixin, ResponseHessiansMixin):
    """Generated model for ResponsesConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckResponseDescriptorsLength(context="responses"),
        CheckDescriptorsValid(context="responses", check_uniqueness=True),
        CheckResponseDescriptors(context="responses"),
        CheckFdGradientStepSize(context="responses"),
        CheckMixedGradients(context="responses"),
        CheckMixedHessians(context="responses"),
    ]

    id_responses: str | None = DakotaField(
        default=None,
        description="Name the responses block; helpful when there are multiple",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.id",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the responses",
        dakota={
            "aliases": ["response_descriptors"],
            "materialization": [
                {
                    "ir_key": "responses.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )
    response_type: Union[ObjectiveFunctions, CalibrationTerms, ResponseFunctions] = (
        DakotaField(
            description="Response Type", dakota={"anchor": True, "union_pattern": 4}
        )
    )
    metadata: list[str] | None = DakotaField(
        default=None,
        description="(Experimental) Labels for floating point response metadata",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.metadata_labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )
