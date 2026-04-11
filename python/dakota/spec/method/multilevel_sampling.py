"""Generated Pydantic models for method.multilevel_sampling"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField, DakotaFloat
from .base import MethodSelection
from typing import Literal, Union

# Cross-module model imports
from dakota.spec.shared.core import MethodConvergenceTolWithTypeContext1Mixin
from dakota.spec.shared.misc import (
    MethodMaxFunctionEvaluationsContext2Mixin,
    MethodMaxIterationsContext2Mixin,
    MethodMlmfSolverMetricMixin,
    MethodMlmfSubProblemSolverMixin,
    MethodOptionalModelPointerMixin,
    MethodSeedSequenceMixin,
    MethodThreeOptionalKeywordsMixin,
    MlmfSolutionModeMixin,
    PromotedModelSelectionContext2Mixin,
    RngOptionsContext2Mixin,
)
from dakota.spec.shared.sampling import (
    MethodExportSamplesFormatMixin,
    MethodSampleTypeLhsMcMixin,
    MlmfPilotSamplesContext1Mixin,
)


class Weighted(
    PromotedModelSelectionContext2Mixin,
    MethodMlmfSubProblemSolverMixin,
    MethodMlmfSolverMetricMixin,
):
    """Generated model for Weighted"""

    pass


class MultilevelSamplingExportSampleSequence(MethodExportSamplesFormatMixin):
    """Generated model for MultilevelSamplingExportSampleSequence"""

    pass


class Mean(DakotaBaseModel):
    """Generated model for Mean"""

    mean: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.allocation_target",
                    "stored_value": "TARGET_MEAN",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class VarianceConfig(DakotaBaseModel):
    """Generated model for VarianceConfig"""

    optimization: Literal[True] | None = DakotaField(
        default=None,
        description="Solve the optimization problem for the sample allocation by numerical optimization in the case of sampling estimator targeting the variance.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.allocation_target.optimization",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class StandardDeviationConfig(DakotaBaseModel):
    """Generated model for StandardDeviationConfig"""

    optimization: Literal[True] | None = DakotaField(
        default=None,
        description="Solve the optimization problem for the sample allocation by numerical optimization in the case of sampling estimator targeting the standard_deviation.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.allocation_target.optimization",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class ScalarizationConfig(DakotaBaseModel):
    """Generated model for ScalarizationConfig"""

    scalarization_response_mapping: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Coefficients for linear scalarization (combination) of responses",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.scalarization_response_mapping",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    optimization: Literal[True] | None = DakotaField(
        default=None,
        description="Solve the optimization problem for the sample allocation by numerical optimization in the case of sampling estimator targeting the scalarization.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.allocation_target.optimization",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class Sum(DakotaBaseModel):
    """Generated model for Sum"""

    sum: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.qoi_aggregation",
                    "stored_value": "QOI_AGGREGATION_SUM",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class Max(DakotaBaseModel):
    """Generated model for Max"""

    max: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.qoi_aggregation",
                    "stored_value": "QOI_AGGREGATION_MAX",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class VarianceConstraint(DakotaBaseModel):
    """Generated model for VarianceConstraint"""

    variance_constraint: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.convergence_tolerance_target",
                    "stored_value": "VARIANCE_CONSTRAINT_TARGET",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class CostConstraint(DakotaBaseModel):
    """Generated model for CostConstraint"""

    cost_constraint: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.convergence_tolerance_target",
                    "stored_value": "COST_CONSTRAINT_TARGET",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class Variance(DakotaBaseModel):
    """Generated model for Variance"""

    variance: VarianceConfig = DakotaField(
        default_factory=VarianceConfig,
        description="Fit MLMC sample allocation to control the variance of the estimator for the variance.",
        dakota={
            "model_default": "VarianceConfig",
            "materialization": [
                {
                    "ir_key": "method.nond.allocation_target",
                    "stored_value": "TARGET_VARIANCE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class StandardDeviation(DakotaBaseModel):
    """Generated model for StandardDeviation"""

    standard_deviation: StandardDeviationConfig = DakotaField(
        default_factory=StandardDeviationConfig,
        description="Fit MLMC sample allocation to control the variance of the estimator for the standard deviation.",
        dakota={
            "model_default": "StandardDeviationConfig",
            "materialization": [
                {
                    "ir_key": "method.nond.allocation_target",
                    "stored_value": "TARGET_SIGMA",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class Scalarization(DakotaBaseModel):
    """Generated model for Scalarization"""

    scalarization: ScalarizationConfig = DakotaField(
        default_factory=ScalarizationConfig,
        description="Fit MLMC sample allocation to a mixture of terms of means and standard deviations.",
        dakota={
            "model_default": "ScalarizationConfig",
            "materialization": [
                {
                    "ir_key": "method.nond.allocation_target",
                    "stored_value": "TARGET_SCALARIZATION",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class MultilevelSamplingConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodSeedSequenceMixin,
    MlmfPilotSamplesContext1Mixin,
    MlmfSolutionModeMixin,
    MethodSampleTypeLhsMcMixin,
    MethodConvergenceTolWithTypeContext1Mixin,
    MethodMaxIterationsContext2Mixin,
    MethodMaxFunctionEvaluationsContext2Mixin,
    RngOptionsContext2Mixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for MultilevelSamplingConfig"""

    weighted: Weighted | None = DakotaField(
        default=None,
        description="Include control variate weights for each of the recursive differences using in multilevel Monte Carlo (MLMC)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "stored_value": "SUBMETHOD_WEIGHTED_MLMC",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    export_sample_sequence: MultilevelSamplingExportSampleSequence | None = DakotaField(
        default=None,
        description="Enable export of multilevel/multifidelity sample sequences to individual files",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_sample_sequence",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    allocation_target: Union[Mean, Variance, StandardDeviation, Scalarization] = (
        DakotaField(
            default_factory=Mean,
            description="Allocation statistics/target for the MLMC sample allocation.",
            dakota={"union_pattern": 1, "model_default": "Mean"},
        )
    )
    qoi_aggregation: Union[Sum, Max] = DakotaField(
        default_factory=Sum,
        description="Aggregation strategy for the QoIs statistics for problems with multiple responses in the MLMC algorithm",
        dakota={"union_pattern": 1, "model_default": "Sum"},
    )
    convergence_tolerance_target: Union[VarianceConstraint, CostConstraint] = (
        DakotaField(
            default_factory=VarianceConstraint,
            description="Select target for MLMC sample allocation",
            dakota={"union_pattern": 1, "model_default": "VarianceConstraint"},
        )
    )


class MultilevelSamplingSelection(MethodSelection):
    """Generated model for MultilevelSamplingSelection"""

    multilevel_sampling: MultilevelSamplingConfig = DakotaField(
        dakota={
            "aliases": ["multilevel_mc", "mlmc"],
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "MULTILEVEL_SAMPLING",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        }
    )
