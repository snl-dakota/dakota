"""Generated Pydantic models for shared.sampling"""

from __future__ import annotations

from ...base import DakotaBaseModel, DakotaField, DakotaFloat
from typing import Literal, Union
from typing import ClassVar, List
from ...validation import ValidationRule
from ...validation.rules import CheckNonnegativeList, CompareLength, CompareLengthOne


class MlmfPilotSamplesContext1Mixin(DakotaBaseModel):
    """Generated model for MlmfPilotSamplesContext1Mixin"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="mlmfpilotsamplescontext1mixin", list_field="pilot_samples"
        ),
    ]

    pilot_samples: list[int] | None = DakotaField(
        default=None,
        description="Initial set of samples for multilevel sampling methods.",
        dakota={
            "aliases": ["initial_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.pilot_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "SizetArray",
                }
            ],
        },
    )


class MlmfPilotSamplesContext2Mixin(DakotaBaseModel):
    """Generated model for MlmfPilotSamplesContext2Mixin"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="mlmfpilotsamplescontext2mixin", list_field="pilot_samples"
        ),
    ]

    pilot_samples: list[int] | None = DakotaField(
        default=None,
        description="Initial set of samples for multilevel/multifidelity sampling methods.",
        dakota={
            "aliases": ["initial_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.pilot_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "SizetArray",
                }
            ],
        },
    )


class SamplesOnEmulatorWithAliasMixin(DakotaBaseModel):
    """Generated model for SamplesOnEmulatorWithAliasMixin"""

    samples_on_emulator: int = DakotaField(
        default=0,
        description="Number of samples at which to evaluate an emulator (surrogate)",
        dakota={
            "aliases": ["samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.samples_on_emulator",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ],
        },
    )


class MethodExportSamplesFormatCustomAnnotatedConfig(DakotaBaseModel):
    """Generated model for MethodExportSamplesFormatCustomAnnotatedConfig"""

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_samples_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_samples_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_samples_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodExportSamplesFormatAnnotated(DakotaBaseModel):
    """Generated model for MethodExportSamplesFormatAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_samples_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodExportSamplesFormatFreeform(DakotaBaseModel):
    """Generated model for MethodExportSamplesFormatFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_samples_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodSampleTypeLhsMcLhs(DakotaBaseModel):
    """Generated model for MethodSampleTypeLhsMcLhs"""

    lhs: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sample_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_LHS",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodSampleTypeLhsMcRandom(DakotaBaseModel):
    """Generated model for MethodSampleTypeLhsMcRandom"""

    random: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sample_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_RANDOM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Fallback(DakotaBaseModel):
    """Generated model for Fallback"""

    fallback: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.numerical_solve_mode",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NUMERICAL_FALLBACK",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Override(DakotaBaseModel):
    """Generated model for Override"""

    override: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.numerical_solve_mode",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NUMERICAL_OVERRIDE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class AutoReorder(DakotaBaseModel):
    """Generated model for AutoReorder"""

    auto_reorder: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.model_reordering",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "REORDER_MODELS_ON_THE_FLY",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class FixedOrder(DakotaBaseModel):
    """Generated model for FixedOrder"""

    fixed_order: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.model_reordering",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "FIXED_MODEL_ORDERING",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class NumericalSolveSqp(DakotaBaseModel):
    """Generated model for NumericalSolveSqp"""

    sqp: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_NPSOL",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class NumericalSolveNip(DakotaBaseModel):
    """Generated model for NumericalSolveNip"""

    nip: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_OPTPP",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class NumericalSolveGlobalLocal(DakotaBaseModel):
    """Generated model for NumericalSolveGlobalLocal"""

    global_local: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_DIRECT_NPSOL_OPTPP",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class NumericalSolveCompetedLocal(DakotaBaseModel):
    """Generated model for NumericalSolveCompetedLocal"""

    competed_local: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_NPSOL_OPTPP",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class NumericalSolveSolverMetricAverageEstimatorVariance(DakotaBaseModel):
    """Generated model for NumericalSolveSolverMetricAverageEstimatorVariance"""

    average_estimator_variance: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.estimator_variance_metric",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "AVG_ESTVAR_METRIC",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class NumericalSolveSolverMetricNormEstimatorVarianceConfig(DakotaBaseModel):
    """Generated model for NumericalSolveSolverMetricNormEstimatorVarianceConfig"""

    norm_order: DakotaFloat = DakotaField(
        default=2.0,
        ge=1.0,
        description="Utilize the response covariance metric for guiding adaptive refinement during UQ.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.estimator_variance_metric_norm_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class NumericalSolveSolverMetricMaxEstimatorVariance(DakotaBaseModel):
    """Generated model for NumericalSolveSolverMetricMaxEstimatorVariance"""

    max_estimator_variance: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.estimator_variance_metric",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MAX_ESTVAR_METRIC",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PilotSamples(DakotaBaseModel):
    """Generated model for PilotSamples"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="mlmfgrouppilotsamplespilotsamples", list_field="counts"
        ),
    ]

    counts: list[int] = DakotaField(
        description="Initial set of samples for groups in the multilevel BLUE sampling method",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.pilot_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "SizetArray",
                }
            ]
        },
    )
    independent: Literal[True] | None = DakotaField(
        default=None,
        description="Independent pilot sampling for groups in multilevel BLUE",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.pilot_samples.mode",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "INDEPENDENT_PILOT",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodExportSamplesFormatCustomAnnotated(DakotaBaseModel):
    """Generated model for MethodExportSamplesFormatCustomAnnotated"""

    custom_annotated: MethodExportSamplesFormatCustomAnnotatedConfig = DakotaField(
        default_factory=MethodExportSamplesFormatCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "MethodExportSamplesFormatCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.nond.export_samples_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class MethodSampleTypeLhsMcMixin(DakotaBaseModel):
    """Generated model for MethodSampleTypeLhsMcMixin"""

    sample_type: Union[MethodSampleTypeLhsMcLhs, MethodSampleTypeLhsMcRandom] | None = (
        DakotaField(
            default=None,
            description="Selection of sampling strategy",
            dakota={"union_pattern": 2},
        )
    )


class NumericalSolveSolverMetricNormEstimatorVariance(DakotaBaseModel):
    """Generated model for NumericalSolveSolverMetricNormEstimatorVariance"""

    norm_estimator_variance: NumericalSolveSolverMetricNormEstimatorVarianceConfig = DakotaField(
        description="Utilize a p-norm over the vector of QoI estimator variances as the solver metric for sampling-based multifidelity methods.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.estimator_variance_metric",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NORM_ESTVAR_METRIC",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MlmfGroupPilotSamplesMixin(DakotaBaseModel):
    """Generated model for MlmfGroupPilotSamplesMixin"""

    pilot_samples: PilotSamples | None = DakotaField(
        default=None,
        description="Initial set of samples for groups in the multilevel BLUE sampling method",
        dakota={"argument": "counts", "aliases": ["initial_samples"]},
    )


class MethodExportSamplesFormatMixin(DakotaBaseModel):
    """Generated model for MethodExportSamplesFormatMixin"""

    format: Union[
        MethodExportSamplesFormatCustomAnnotated,
        MethodExportSamplesFormatAnnotated,
        MethodExportSamplesFormatFreeform,
    ] = DakotaField(
        default_factory=MethodExportSamplesFormatAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "MethodExportSamplesFormatAnnotated",
        },
    )


class NumericalSolve(DakotaBaseModel):
    """Generated model for NumericalSolve"""

    numerical_solve_strategy: Union[Fallback, Override] = DakotaField(
        default_factory=Fallback,
        description="Employ numerical solve",
        dakota={"anchor": True, "union_pattern": 1, "model_default": "Fallback"},
    )
    model_reordering: Union[AutoReorder, FixedOrder] = DakotaField(
        default_factory=AutoReorder,
        description="Model reordering strategy",
        dakota={"union_pattern": 1, "model_default": "AutoReorder"},
    )
    optimization_solver: (
        Union[
            NumericalSolveSqp,
            NumericalSolveNip,
            NumericalSolveGlobalLocal,
            NumericalSolveCompetedLocal,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Optimization Solver",
        dakota={"anchor": True, "union_pattern": 2},
    )
    solver_metric: (
        Union[
            NumericalSolveSolverMetricAverageEstimatorVariance,
            NumericalSolveSolverMetricNormEstimatorVariance,
            NumericalSolveSolverMetricMaxEstimatorVariance,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Metric employed during numerical solutions in sampling-based multifidelity UQ methods.",
        dakota={"union_pattern": 2},
    )


class MfmcNumericalSolveMixin(DakotaBaseModel):
    """Generated model for MfmcNumericalSolveMixin"""

    numerical_solve: NumericalSolve | None = DakotaField(
        default=None,
        description="Specify the situations where numerical optimization is used for MFMC sample allocation",
    )
