"""Generated Pydantic models for shared.core"""

from __future__ import annotations

from ...base import DakotaBaseModel, DakotaField, DakotaFloat
from typing import Literal, Union
from ...base import DBL_MAX
from typing import ClassVar, List
from ...validation import ValidationRule
from ...validation.rules import CheckSumEqualsLength, CompareLength, CompareLengthOne


class DefaultConstraintTolMixin(DakotaBaseModel):
    """Generated model for DefaultConstraintTolMixin"""

    constraint_tolerance: DakotaFloat = DakotaField(
        default=0.0,
        description="Maximum allowable constraint violation still considered feasible",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.constraint_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class MethodConvergenceTolMixin(DakotaBaseModel):
    """Generated model for MethodConvergenceTolMixin"""

    convergence_tolerance: DakotaFloat = DakotaField(
        default=-DBL_MAX,
        description="Stopping criterion based on objective function or statistics convergence",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.convergence_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
                {
                    "ir_key": "method.jega.percent_change",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
            ]
        },
    )


class MethodConvergenceTolWithTypeContext1Relative(DakotaBaseModel):
    """Generated model for MethodConvergenceTolWithTypeContext1Relative"""

    relative: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.convergence_tolerance_type",
                    "stored_value": "RELATIVE_CONVERGENCE_TOLERANCE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodConvergenceTolWithTypeContext1Absolute(DakotaBaseModel):
    """Generated model for MethodConvergenceTolWithTypeContext1Absolute"""

    absolute: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.convergence_tolerance_type",
                    "stored_value": "ABSOLUTE_CONVERGENCE_TOLERANCE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodConvergenceTolWithTypeContext2Relative(DakotaBaseModel):
    """Generated model for MethodConvergenceTolWithTypeContext2Relative"""

    relative: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.convergence_tolerance_type",
                    "stored_value": "RELATIVE_CONVERGENCE_TOLERANCE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodConvergenceTolWithTypeContext2Absolute(DakotaBaseModel):
    """Generated model for MethodConvergenceTolWithTypeContext2Absolute"""

    absolute: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.convergence_tolerance_type",
                    "stored_value": "ABSOLUTE_CONVERGENCE_TOLERANCE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodConvergenceTolWithTypeContext3Relative(DakotaBaseModel):
    """Generated model for MethodConvergenceTolWithTypeContext3Relative"""

    relative: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.convergence_tolerance_type",
                    "stored_value": "RELATIVE_CONVERGENCE_TOLERANCE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodConvergenceTolWithTypeContext3Absolute(DakotaBaseModel):
    """Generated model for MethodConvergenceTolWithTypeContext3Absolute"""

    absolute: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.convergence_tolerance_type",
                    "stored_value": "ABSOLUTE_CONVERGENCE_TOLERANCE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class Active(DakotaBaseModel):
    """Generated model for Active"""

    active: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.refinement_statistics_mode",
                    "stored_value": "ACTIVE_EXPANSION_STATS",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class StatisticsModeCombined(DakotaBaseModel):
    """Generated model for StatisticsModeCombined"""

    combined: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.refinement_statistics_mode",
                    "stored_value": "COMBINED_EXPANSION_STATS",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class LevelMappings(DakotaBaseModel):
    """Generated model for LevelMappings"""

    level_mappings: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_metric",
                    "stored_value": "LEVEL_STATS_METRIC",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class RefinementMetricCov(DakotaBaseModel):
    """Generated model for RefinementMetricCov"""

    covariance: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_metric",
                    "stored_value": "COVARIANCE_METRIC",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class NumericalGradientOptionsMethodSourceDakotaRelative(DakotaBaseModel):
    """Generated model for NumericalGradientOptionsMethodSourceDakotaRelative"""

    relative: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.fd_gradient_step_type",
                    "stored_value": "relative",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class NumericalGradientOptionsMethodSourceDakotaAbsolute(DakotaBaseModel):
    """Generated model for NumericalGradientOptionsMethodSourceDakotaAbsolute"""

    absolute: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.fd_gradient_step_type",
                    "stored_value": "absolute",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class NumericalGradientOptionsMethodSourceDakotaBounds(DakotaBaseModel):
    """Generated model for NumericalGradientOptionsMethodSourceDakotaBounds"""

    bounds: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.fd_gradient_step_type",
                    "stored_value": "bounds",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class NumericalGradientOptionsMethodSourceVendor(DakotaBaseModel):
    """Generated model for NumericalGradientOptionsMethodSourceVendor"""

    vendor: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.method_source",
                    "stored_value": "vendor",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class NumericalGradientOptionsIntervalTypeForward(DakotaBaseModel):
    """Generated model for NumericalGradientOptionsIntervalTypeForward"""

    forward: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.interval_type",
                    "stored_value": "forward",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class NumericalGradientOptionsIntervalTypeCentral(DakotaBaseModel):
    """Generated model for NumericalGradientOptionsIntervalTypeCentral"""

    central: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.interval_type",
                    "stored_value": "central",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ResponseLevelsComputeProbRelGenProbabilities(DakotaBaseModel):
    """Generated model for ResponseLevelsComputeProbRelGenProbabilities"""

    probabilities: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_level_target",
                    "stored_value": "PROBABILITIES",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ResponseLevelsComputeProbRelGenReliabilities(DakotaBaseModel):
    """Generated model for ResponseLevelsComputeProbRelGenReliabilities"""

    reliabilities: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_level_target",
                    "stored_value": "RELIABILITIES",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ResponseLevelsComputeProbRelGenGenReliabilities(DakotaBaseModel):
    """Generated model for ResponseLevelsComputeProbRelGenGenReliabilities"""

    gen_reliabilities: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_level_target",
                    "stored_value": "GEN_RELIABILITIES",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ResponseLevelsComputeProbRelGenSystemSeries(DakotaBaseModel):
    """Generated model for ResponseLevelsComputeProbRelGenSystemSeries"""

    series: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_level_target_reduce",
                    "stored_value": "SYSTEM_SERIES",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ResponseLevelsComputeProbRelGenSystemParallel(DakotaBaseModel):
    """Generated model for ResponseLevelsComputeProbRelGenSystemParallel"""

    parallel: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_level_target_reduce",
                    "stored_value": "SYSTEM_PARALLEL",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodConvergenceTolWithTypeContext1ConvergenceTol(DakotaBaseModel):
    """Generated model for MethodConvergenceTolWithTypeContext1ConvergenceTol"""

    value: DakotaFloat = DakotaField(
        default=-DBL_MAX,
        description="Stopping criterion based on relative error",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.convergence_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
                {
                    "ir_key": "method.jega.percent_change",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
            ]
        },
    )
    convergence_tolerance_type: (
        Union[
            MethodConvergenceTolWithTypeContext1Relative,
            MethodConvergenceTolWithTypeContext1Absolute,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Convergence tolerance type",
        dakota={"anchor": True, "union_pattern": 2},
    )


class MethodConvergenceTolWithTypeContext2ConvergenceTol(DakotaBaseModel):
    """Generated model for MethodConvergenceTolWithTypeContext2ConvergenceTol"""

    value: DakotaFloat = DakotaField(
        default=-DBL_MAX,
        description="Stopping criterion based on objective function or statistics convergence",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.convergence_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
                {
                    "ir_key": "method.jega.percent_change",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
            ]
        },
    )
    convergence_tolerance_type: (
        Union[
            MethodConvergenceTolWithTypeContext2Relative,
            MethodConvergenceTolWithTypeContext2Absolute,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Convergence tolerance type",
        dakota={"anchor": True, "union_pattern": 2},
    )


class MethodConvergenceTolWithTypeContext3ConvergenceTol(DakotaBaseModel):
    """Generated model for MethodConvergenceTolWithTypeContext3ConvergenceTol"""

    value: DakotaFloat = DakotaField(
        default=-DBL_MAX,
        description="Stopping criterion based on relative error reduction",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.convergence_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
                {
                    "ir_key": "method.jega.percent_change",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
            ]
        },
    )
    convergence_tolerance_type: (
        Union[
            MethodConvergenceTolWithTypeContext3Relative,
            MethodConvergenceTolWithTypeContext3Absolute,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Convergence tolerance type",
        dakota={"anchor": True, "union_pattern": 2},
    )


class MethodMfRefineStatsModeMixin(DakotaBaseModel):
    """Generated model for MethodMfRefineStatsModeMixin"""

    statistics_mode: Union[Active, StatisticsModeCombined] | None = DakotaField(
        default=None,
        description="type of statistical metric roll-up for multifidelity UQ methods",
        dakota={"union_pattern": 2},
    )


class MethodRefineMetricTypeMixin(DakotaBaseModel):
    """Generated model for MethodRefineMetricTypeMixin"""

    refinement_metric: Union[LevelMappings, RefinementMetricCov] | None = DakotaField(
        default=None,
        description="Metric used for guiding adaptive refinement during UQ.",
        dakota={"union_pattern": 2},
    )


class NumericalGradientOptionsMethodSourceDakotaConfig(DakotaBaseModel):
    """Generated model for NumericalGradientOptionsMethodSourceDakotaConfig"""

    ignore_bounds: Literal[True] | None = DakotaField(
        default=None,
        description="Do not respect bounds when computing gradients or Hessians",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.ignore_bounds",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    step_scaling: Union[
        NumericalGradientOptionsMethodSourceDakotaRelative,
        NumericalGradientOptionsMethodSourceDakotaAbsolute,
        NumericalGradientOptionsMethodSourceDakotaBounds,
    ] = DakotaField(
        default_factory=NumericalGradientOptionsMethodSourceDakotaRelative,
        description="Step Scaling",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "NumericalGradientOptionsMethodSourceDakotaRelative",
        },
    )


class ResponseLevelsComputeProbRelGenCompute(DakotaBaseModel):
    """Generated model for ResponseLevelsComputeProbRelGenCompute"""

    statistic: Union[
        ResponseLevelsComputeProbRelGenProbabilities,
        ResponseLevelsComputeProbRelGenReliabilities,
        ResponseLevelsComputeProbRelGenGenReliabilities,
    ] = DakotaField(
        description="Statistics to Compute", dakota={"anchor": True, "union_pattern": 4}
    )
    system: (
        Union[
            ResponseLevelsComputeProbRelGenSystemSeries,
            ResponseLevelsComputeProbRelGenSystemParallel,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Compute system reliability (series or parallel)",
        dakota={"union_pattern": 2},
    )


class MethodConvergenceTolWithTypeContext1Mixin(DakotaBaseModel):
    """Generated model for MethodConvergenceTolWithTypeContext1Mixin"""

    convergence_tolerance: MethodConvergenceTolWithTypeContext1ConvergenceTol | None = (
        DakotaField(
            default=None,
            description="Stopping criterion based on relative error",
            dakota={"argument": "value"},
        )
    )


class MethodConvergenceTolWithTypeContext2Mixin(DakotaBaseModel):
    """Generated model for MethodConvergenceTolWithTypeContext2Mixin"""

    convergence_tolerance: MethodConvergenceTolWithTypeContext2ConvergenceTol | None = (
        DakotaField(
            default=None,
            description="Stopping criterion based on objective function or statistics convergence",
            dakota={"argument": "value"},
        )
    )


class MethodConvergenceTolWithTypeContext3Mixin(DakotaBaseModel):
    """Generated model for MethodConvergenceTolWithTypeContext3Mixin"""

    convergence_tolerance: MethodConvergenceTolWithTypeContext3ConvergenceTol | None = (
        DakotaField(
            default=None,
            description="Stopping criterion based on relative error reduction",
            dakota={"argument": "value"},
        )
    )


class NumericalGradientOptionsMethodSourceDakota(DakotaBaseModel):
    """Generated model for NumericalGradientOptionsMethodSourceDakota"""

    dakota: NumericalGradientOptionsMethodSourceDakotaConfig = DakotaField(
        default_factory=NumericalGradientOptionsMethodSourceDakotaConfig,
        description="(Default) Use internal Dakota finite differences algorithm",
        dakota={
            "model_default": "NumericalGradientOptionsMethodSourceDakotaConfig",
            "materialization": [
                {
                    "ir_key": "responses.method_source",
                    "stored_value": "dakota",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class ResponseLevelsComputeProbRelGenResponseLevels(DakotaBaseModel):
    """Generated model for ResponseLevelsComputeProbRelGenResponseLevels"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckSumEqualsLength(
            context="responselevelscomputeprobrelgenresponselevels",
            num_list_field="num_response_levels",
            levels_list_field="values",
        ),
    ]

    values: list[DakotaFloat] = DakotaField(
        description="Values at which to estimate desired statistics for each response"
    )
    num_response_levels: list[int] | None = DakotaField(
        default=None,
        description="Number of values at which to estimate desired statistics for each response",
    )
    compute: ResponseLevelsComputeProbRelGenCompute | None = DakotaField(
        default=None,
        description="Selection of statistics to compute at each response level",
    )


class MethodExpConvergenceToleranceMixin(
    MethodRefineMetricTypeMixin, MethodConvergenceTolWithTypeContext2Mixin
):
    """Generated model for MethodExpConvergenceToleranceMixin"""

    pass


class MethodMfExpConvergenceToleranceMixin(
    MethodRefineMetricTypeMixin,
    MethodConvergenceTolWithTypeContext2Mixin,
    MethodMfRefineStatsModeMixin,
):
    """Generated model for MethodMfExpConvergenceToleranceMixin"""

    pass


class NumericalGradientOptionsMixin(DakotaBaseModel):
    """Generated model for NumericalGradientOptionsMixin"""

    method_source: Union[
        NumericalGradientOptionsMethodSourceDakota,
        NumericalGradientOptionsMethodSourceVendor,
    ] = DakotaField(
        default_factory=NumericalGradientOptionsMethodSourceDakota,
        description="Specify which finite difference routine is used",
        dakota={
            "union_pattern": 1,
            "model_default": "NumericalGradientOptionsMethodSourceDakota",
        },
    )
    interval_type: Union[
        NumericalGradientOptionsIntervalTypeForward,
        NumericalGradientOptionsIntervalTypeCentral,
    ] = DakotaField(
        default_factory=NumericalGradientOptionsIntervalTypeForward,
        description="Specify how to compute gradients and hessians",
        dakota={
            "union_pattern": 1,
            "model_default": "NumericalGradientOptionsIntervalTypeForward",
        },
    )
    fd_step_size: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Step size used when computing gradients and Hessians",
        dakota={
            "aliases": ["fd_gradient_step_size"],
            "materialization": [
                {
                    "ir_key": "responses.fd_gradient_step_size",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )


class ResponseLevelsComputeProbRelGenMixin(DakotaBaseModel):
    """Generated model for ResponseLevelsComputeProbRelGenMixin"""

    response_levels: ResponseLevelsComputeProbRelGenResponseLevels | None = DakotaField(
        default=None,
        description="Values at which to estimate desired statistics for each response",
        dakota={
            "argument": "values",
            "materialization": [
                {
                    "ir_key": "method.nond.response_levels",
                    "storage_type": "RESPONSE_LEVELS_ARRAY",
                    "ir_value_type": "RealVectorArray",
                }
            ],
        },
    )
