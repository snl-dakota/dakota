"""Generated Pydantic models for shared.responses"""

from __future__ import annotations

from ...base import DakotaBaseModel, DakotaField, DakotaFloat
from typing import Literal, Union
from typing import ClassVar, List
from ...validation import ValidationRule
from ...validation.rules import CheckSumEqualsLength, CompareLength, CompareLengthOne


class NoGradients(DakotaBaseModel):
    """Generated model for NoGradients"""

    no_gradients: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.gradient_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "none",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class AnalyticGradients(DakotaBaseModel):
    """Generated model for AnalyticGradients"""

    analytic_gradients: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.gradient_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "analytic",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MixedGradientsMethodSourceDakotaRelative(DakotaBaseModel):
    """Generated model for MixedGradientsMethodSourceDakotaRelative"""

    relative: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.fd_gradient_step_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "relative",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MixedGradientsMethodSourceDakotaAbsolute(DakotaBaseModel):
    """Generated model for MixedGradientsMethodSourceDakotaAbsolute"""

    absolute: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.fd_gradient_step_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "absolute",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MixedGradientsMethodSourceDakotaBounds(DakotaBaseModel):
    """Generated model for MixedGradientsMethodSourceDakotaBounds"""

    bounds: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.fd_gradient_step_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "bounds",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MixedGradientsMethodSourceVendor(DakotaBaseModel):
    """Generated model for MixedGradientsMethodSourceVendor"""

    vendor: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.method_source",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "vendor",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MixedGradientsIntervalTypeForward(DakotaBaseModel):
    """Generated model for MixedGradientsIntervalTypeForward"""

    forward: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.interval_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "forward",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MixedGradientsIntervalTypeCentral(DakotaBaseModel):
    """Generated model for MixedGradientsIntervalTypeCentral"""

    central: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.interval_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "central",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class NumericalGradientsMethodSourceDakotaRelative(DakotaBaseModel):
    """Generated model for NumericalGradientsMethodSourceDakotaRelative"""

    relative: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.fd_gradient_step_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "relative",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class NumericalGradientsMethodSourceDakotaAbsolute(DakotaBaseModel):
    """Generated model for NumericalGradientsMethodSourceDakotaAbsolute"""

    absolute: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.fd_gradient_step_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "absolute",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class NumericalGradientsMethodSourceDakotaBounds(DakotaBaseModel):
    """Generated model for NumericalGradientsMethodSourceDakotaBounds"""

    bounds: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.fd_gradient_step_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "bounds",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class NumericalGradientsMethodSourceVendor(DakotaBaseModel):
    """Generated model for NumericalGradientsMethodSourceVendor"""

    vendor: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.method_source",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "vendor",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class NumericalGradientsIntervalTypeForward(DakotaBaseModel):
    """Generated model for NumericalGradientsIntervalTypeForward"""

    forward: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.interval_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "forward",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class NumericalGradientsIntervalTypeCentral(DakotaBaseModel):
    """Generated model for NumericalGradientsIntervalTypeCentral"""

    central: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.interval_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "central",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class NoHessians(DakotaBaseModel):
    """Generated model for NoHessians"""

    no_hessians: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.hessian_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "none",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class NumericalHessiansRelative(DakotaBaseModel):
    """Generated model for NumericalHessiansRelative"""

    relative: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.fd_hessian_step_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "relative",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class NumericalHessiansAbsolute(DakotaBaseModel):
    """Generated model for NumericalHessiansAbsolute"""

    absolute: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.fd_hessian_step_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "absolute",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class NumericalHessiansBounds(DakotaBaseModel):
    """Generated model for NumericalHessiansBounds"""

    bounds: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.fd_hessian_step_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "bounds",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class NumericalHessiansIntervalTypeForward(DakotaBaseModel):
    """Generated model for NumericalHessiansIntervalTypeForward"""

    forward: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.central_hess",
                    "storage_type": "PRESENCE_FALSE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class NumericalHessiansIntervalTypeCentral(DakotaBaseModel):
    """Generated model for NumericalHessiansIntervalTypeCentral"""

    central: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.central_hess",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class QuasiHessiansBfgsConfig(DakotaBaseModel):
    """Generated model for QuasiHessiansBfgsConfig"""

    damped: Literal[True] | None = DakotaField(
        default=None,
        description="Numerical safeguarding for BFGS updates",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.quasi_hessian_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "damped_bfgs",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class QuasiHessiansSr1(DakotaBaseModel):
    """Generated model for QuasiHessiansSr1"""

    sr1: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.quasi_hessian_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "sr1",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class AnalyticHessians(DakotaBaseModel):
    """Generated model for AnalyticHessians"""

    analytic_hessians: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.hessian_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "analytic",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class IdNumericalHessians(DakotaBaseModel):
    """Generated model for IdNumericalHessians"""

    values: list[int] = DakotaField(
        description="Identify which numerical-Hessian corresponds to which response",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.hessians.mixed.id_numerical",
                    "storage_type": "INT_SET",
                    "ir_value_type": "IntSet",
                }
            ]
        },
    )
    fd_step_size: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Step size used when computing gradients and Hessians",
        dakota={
            "aliases": ["fd_hessian_step_size"],
            "materialization": [
                {
                    "ir_key": "responses.fd_hessian_step_size",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )


class MixedHessiansRelative(DakotaBaseModel):
    """Generated model for MixedHessiansRelative"""

    relative: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.fd_hessian_step_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "relative",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MixedHessiansAbsolute(DakotaBaseModel):
    """Generated model for MixedHessiansAbsolute"""

    absolute: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.fd_hessian_step_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "absolute",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MixedHessiansBounds(DakotaBaseModel):
    """Generated model for MixedHessiansBounds"""

    bounds: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.fd_hessian_step_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "bounds",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MixedHessiansIntervalTypeForward(DakotaBaseModel):
    """Generated model for MixedHessiansIntervalTypeForward"""

    forward: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.central_hess",
                    "storage_type": "PRESENCE_FALSE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class MixedHessiansIntervalTypeCentral(DakotaBaseModel):
    """Generated model for MixedHessiansIntervalTypeCentral"""

    central: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.central_hess",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class IdQuasiHessiansBfgsConfig(DakotaBaseModel):
    """Generated model for IdQuasiHessiansBfgsConfig"""

    damped: Literal[True] | None = DakotaField(
        default=None,
        description="Numerical safeguarding for BFGS updates",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.quasi_hessian_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "damped_bfgs",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class IdQuasiHessiansSr1(DakotaBaseModel):
    """Generated model for IdQuasiHessiansSr1"""

    sr1: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.quasi_hessian_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "sr1",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ResponseLevelsComputeProbGenContext1Probabilities(DakotaBaseModel):
    """Generated model for ResponseLevelsComputeProbGenContext1Probabilities"""

    probabilities: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_level_target",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "PROBABILITIES",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ResponseLevelsComputeProbGenContext1GenReliabilities(DakotaBaseModel):
    """Generated model for ResponseLevelsComputeProbGenContext1GenReliabilities"""

    gen_reliabilities: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_level_target",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "GEN_RELIABILITIES",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ResponseLevelsComputeProbGenContext1SystemSeries(DakotaBaseModel):
    """Generated model for ResponseLevelsComputeProbGenContext1SystemSeries"""

    series: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_level_target_reduce",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SYSTEM_SERIES",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ResponseLevelsComputeProbGenContext1SystemParallel(DakotaBaseModel):
    """Generated model for ResponseLevelsComputeProbGenContext1SystemParallel"""

    parallel: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_level_target_reduce",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SYSTEM_PARALLEL",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ResponseLevelsComputeProbGenContext2Probabilities(DakotaBaseModel):
    """Generated model for ResponseLevelsComputeProbGenContext2Probabilities"""

    probabilities: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_level_target",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "PROBABILITIES",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ResponseLevelsComputeProbGenContext2GenReliabilities(DakotaBaseModel):
    """Generated model for ResponseLevelsComputeProbGenContext2GenReliabilities"""

    gen_reliabilities: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_level_target",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "GEN_RELIABILITIES",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ResponseLevelsComputeProbGenContext2SystemSeries(DakotaBaseModel):
    """Generated model for ResponseLevelsComputeProbGenContext2SystemSeries"""

    series: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_level_target_reduce",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SYSTEM_SERIES",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ResponseLevelsComputeProbGenContext2SystemParallel(DakotaBaseModel):
    """Generated model for ResponseLevelsComputeProbGenContext2SystemParallel"""

    parallel: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_level_target_reduce",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SYSTEM_PARALLEL",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ResponseScalarDataFormatCustomAnnotatedConfig(DakotaBaseModel):
    """Generated model for ResponseScalarDataFormatCustomAnnotatedConfig"""

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.scalar_data_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    exp_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable experiment ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.scalar_data_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ResponseScalarDataFormatAnnotated(DakotaBaseModel):
    """Generated model for ResponseScalarDataFormatAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.scalar_data_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_EXPER_ANNOT",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ResponseScalarDataFormatFreeform(DakotaBaseModel):
    """Generated model for ResponseScalarDataFormatFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.scalar_data_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MixedGradientsMethodSourceDakotaConfig(DakotaBaseModel):
    """Generated model for MixedGradientsMethodSourceDakotaConfig"""

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
        MixedGradientsMethodSourceDakotaRelative,
        MixedGradientsMethodSourceDakotaAbsolute,
        MixedGradientsMethodSourceDakotaBounds,
    ] = DakotaField(
        default_factory=MixedGradientsMethodSourceDakotaRelative,
        description="Step Scaling",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "MixedGradientsMethodSourceDakotaRelative",
        },
    )


class NumericalGradientsMethodSourceDakotaConfig(DakotaBaseModel):
    """Generated model for NumericalGradientsMethodSourceDakotaConfig"""

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
        NumericalGradientsMethodSourceDakotaRelative,
        NumericalGradientsMethodSourceDakotaAbsolute,
        NumericalGradientsMethodSourceDakotaBounds,
    ] = DakotaField(
        default_factory=NumericalGradientsMethodSourceDakotaRelative,
        description="Step Scaling",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "NumericalGradientsMethodSourceDakotaRelative",
        },
    )


class NumericalHessiansConfig(DakotaBaseModel):
    """Generated model for NumericalHessiansConfig"""

    fd_step_size: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Step size used when computing gradients and Hessians",
        dakota={
            "aliases": ["fd_hessian_step_size"],
            "materialization": [
                {
                    "ir_key": "responses.fd_hessian_step_size",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    step_scaling: Union[
        NumericalHessiansRelative, NumericalHessiansAbsolute, NumericalHessiansBounds
    ] = DakotaField(
        default_factory=NumericalHessiansRelative,
        description="Step Scaling",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "NumericalHessiansRelative",
        },
    )
    interval_type: Union[
        NumericalHessiansIntervalTypeForward, NumericalHessiansIntervalTypeCentral
    ] = DakotaField(
        default_factory=NumericalHessiansIntervalTypeForward,
        description="Specify how to compute gradients and hessians",
        dakota={
            "union_pattern": 1,
            "model_default": "NumericalHessiansIntervalTypeForward",
        },
    )


class QuasiHessiansBfgs(DakotaBaseModel):
    """Generated model for QuasiHessiansBfgs"""

    bfgs: QuasiHessiansBfgsConfig = DakotaField(
        description="Use BFGS method to compute quasi-hessians",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.quasi_hessian_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "bfgs",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class IdQuasiHessiansBfgs(DakotaBaseModel):
    """Generated model for IdQuasiHessiansBfgs"""

    bfgs: IdQuasiHessiansBfgsConfig = DakotaField(
        description="Use BFGS method to compute quasi-hessians",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.quasi_hessian_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "bfgs",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ResponseLevelsComputeProbGenContext1Compute(DakotaBaseModel):
    """Generated model for ResponseLevelsComputeProbGenContext1Compute"""

    statistic: Union[
        ResponseLevelsComputeProbGenContext1Probabilities,
        ResponseLevelsComputeProbGenContext1GenReliabilities,
    ] = DakotaField(
        description="Statistics to Compute", dakota={"anchor": True, "union_pattern": 4}
    )
    system: (
        Union[
            ResponseLevelsComputeProbGenContext1SystemSeries,
            ResponseLevelsComputeProbGenContext1SystemParallel,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Compute system reliability (series or parallel)",
        dakota={"union_pattern": 2},
    )


class ResponseLevelsComputeProbGenContext2Compute(DakotaBaseModel):
    """Generated model for ResponseLevelsComputeProbGenContext2Compute"""

    statistic: Union[
        ResponseLevelsComputeProbGenContext2Probabilities,
        ResponseLevelsComputeProbGenContext2GenReliabilities,
    ] = DakotaField(
        description="Statistics to Compute", dakota={"anchor": True, "union_pattern": 4}
    )
    system: (
        Union[
            ResponseLevelsComputeProbGenContext2SystemSeries,
            ResponseLevelsComputeProbGenContext2SystemParallel,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Compute system reliability (series or parallel)",
        dakota={"union_pattern": 2},
    )


class ResponseScalarDataFormatCustomAnnotated(DakotaBaseModel):
    """Generated model for ResponseScalarDataFormatCustomAnnotated"""

    custom_annotated: ResponseScalarDataFormatCustomAnnotatedConfig = DakotaField(
        default_factory=ResponseScalarDataFormatCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format for experiment data",
        dakota={
            "model_default": "ResponseScalarDataFormatCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "responses.scalar_data_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class MixedGradientsMethodSourceDakota(DakotaBaseModel):
    """Generated model for MixedGradientsMethodSourceDakota"""

    dakota: MixedGradientsMethodSourceDakotaConfig = DakotaField(
        default_factory=MixedGradientsMethodSourceDakotaConfig,
        description="(Default) Use internal Dakota finite differences algorithm",
        dakota={
            "model_default": "MixedGradientsMethodSourceDakotaConfig",
            "materialization": [
                {
                    "ir_key": "responses.method_source",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "dakota",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class NumericalGradientsMethodSourceDakota(DakotaBaseModel):
    """Generated model for NumericalGradientsMethodSourceDakota"""

    dakota: NumericalGradientsMethodSourceDakotaConfig = DakotaField(
        default_factory=NumericalGradientsMethodSourceDakotaConfig,
        description="(Default) Use internal Dakota finite differences algorithm",
        dakota={
            "model_default": "NumericalGradientsMethodSourceDakotaConfig",
            "materialization": [
                {
                    "ir_key": "responses.method_source",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "dakota",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class NumericalHessians(DakotaBaseModel):
    """Generated model for NumericalHessians"""

    numerical_hessians: NumericalHessiansConfig = DakotaField(
        description="Hessians are needed and will be approximated by finite differences",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.hessian_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "numerical",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class QuasiHessians(DakotaBaseModel):
    """Generated model for QuasiHessians"""

    quasi_hessians: Union[QuasiHessiansBfgs, QuasiHessiansSr1] = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.hessian_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "quasi",
                    "ir_value_type": "String",
                }
            ]
        }
    )


class IdQuasiHessians(DakotaBaseModel):
    """Generated model for IdQuasiHessians"""

    values: list[int] = DakotaField(
        description="Identify which quasi-Hessian corresponds to which response",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.hessians.mixed.id_quasi",
                    "storage_type": "INT_SET",
                    "ir_value_type": "IntSet",
                }
            ]
        },
    )
    approximation: Union[IdQuasiHessiansBfgs, IdQuasiHessiansSr1] = DakotaField(
        description="Quasi-Hessian Approximation",
        dakota={"anchor": True, "union_pattern": 4},
    )


class ResponseLevelsComputeProbGenContext1ResponseLevels(DakotaBaseModel):
    """Generated model for ResponseLevelsComputeProbGenContext1ResponseLevels"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckSumEqualsLength(
            context="responselevelscomputeprobgencontext1responselevels",
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
    compute: ResponseLevelsComputeProbGenContext1Compute | None = DakotaField(
        default=None,
        description="Selection of statistics to compute at each response level",
    )


class ResponseLevelsComputeProbGenContext2ResponseLevels(DakotaBaseModel):
    """Generated model for ResponseLevelsComputeProbGenContext2ResponseLevels"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckSumEqualsLength(
            context="responselevelscomputeprobgencontext2responselevels",
            num_list_field="num_response_levels",
            levels_list_field="values",
        ),
    ]

    values: list[DakotaFloat] = DakotaField(
        description="Undocumented: Recursive k-d (RKD) Darts is an experimental capability."
    )
    num_response_levels: list[int] | None = DakotaField(
        default=None,
        description="Number of values at which to estimate desired statistics for each response",
    )
    compute: ResponseLevelsComputeProbGenContext2Compute | None = DakotaField(
        default=None,
        description="Selection of statistics to compute at each response level",
    )


class ResponseScalarDataFormatMixin(DakotaBaseModel):
    """Generated model for ResponseScalarDataFormatMixin"""

    format: Union[
        ResponseScalarDataFormatCustomAnnotated,
        ResponseScalarDataFormatAnnotated,
        ResponseScalarDataFormatFreeform,
    ] = DakotaField(
        default_factory=ResponseScalarDataFormatAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "ResponseScalarDataFormatAnnotated",
        },
    )


class MixedGradientsConfig(DakotaBaseModel):
    """Generated model for MixedGradientsConfig"""

    id_numerical_gradients: list[int] = DakotaField(
        description="Identify which numerical gradient corresponds to which response",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.gradients.mixed.id_numerical",
                    "storage_type": "INT_SET",
                    "ir_value_type": "IntSet",
                }
            ]
        },
    )
    id_analytic_gradients: list[int] = DakotaField(
        description="Identify which analytical gradient corresponds to which response",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.gradients.mixed.id_analytic",
                    "storage_type": "INT_SET",
                    "ir_value_type": "IntSet",
                }
            ]
        },
    )
    method_source: Union[
        MixedGradientsMethodSourceDakota, MixedGradientsMethodSourceVendor
    ] = DakotaField(
        default_factory=MixedGradientsMethodSourceDakota,
        description="Specify which finite difference routine is used",
        dakota={
            "union_pattern": 1,
            "model_default": "MixedGradientsMethodSourceDakota",
        },
    )
    interval_type: Union[
        MixedGradientsIntervalTypeForward, MixedGradientsIntervalTypeCentral
    ] = DakotaField(
        default_factory=MixedGradientsIntervalTypeForward,
        description="Specify how to compute gradients and hessians",
        dakota={
            "union_pattern": 1,
            "model_default": "MixedGradientsIntervalTypeForward",
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


class NumericalGradientsConfig(DakotaBaseModel):
    """Generated model for NumericalGradientsConfig"""

    method_source: Union[
        NumericalGradientsMethodSourceDakota, NumericalGradientsMethodSourceVendor
    ] = DakotaField(
        default_factory=NumericalGradientsMethodSourceDakota,
        description="Specify which finite difference routine is used",
        dakota={
            "union_pattern": 1,
            "model_default": "NumericalGradientsMethodSourceDakota",
        },
    )
    interval_type: Union[
        NumericalGradientsIntervalTypeForward, NumericalGradientsIntervalTypeCentral
    ] = DakotaField(
        default_factory=NumericalGradientsIntervalTypeForward,
        description="Specify how to compute gradients and hessians",
        dakota={
            "union_pattern": 1,
            "model_default": "NumericalGradientsIntervalTypeForward",
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


class MixedHessiansConfig(DakotaBaseModel):
    """Generated model for MixedHessiansConfig"""

    id_numerical_hessians: IdNumericalHessians | None = DakotaField(
        default=None,
        description="Identify which numerical-Hessian corresponds to which response",
        dakota={"argument": "values"},
    )
    step_scaling: Union[
        MixedHessiansRelative, MixedHessiansAbsolute, MixedHessiansBounds
    ] = DakotaField(
        default_factory=MixedHessiansRelative,
        description="Step Scaling",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "MixedHessiansRelative",
        },
    )
    interval_type: Union[
        MixedHessiansIntervalTypeForward, MixedHessiansIntervalTypeCentral
    ] = DakotaField(
        default_factory=MixedHessiansIntervalTypeForward,
        description="Specify how to compute gradients and hessians",
        dakota={
            "union_pattern": 1,
            "model_default": "MixedHessiansIntervalTypeForward",
        },
    )
    id_quasi_hessians: IdQuasiHessians | None = DakotaField(
        default=None,
        description="Identify which quasi-Hessian corresponds to which response",
        dakota={"argument": "values"},
    )
    id_analytic_hessians: list[int] | None = DakotaField(
        default=None,
        description="Identify which analytical Hessian corresponds to which response",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.hessians.mixed.id_analytic",
                    "storage_type": "INT_SET",
                    "ir_value_type": "IntSet",
                }
            ]
        },
    )


class ResponseLevelsComputeProbGenContext1Mixin(DakotaBaseModel):
    """Generated model for ResponseLevelsComputeProbGenContext1Mixin"""

    response_levels: ResponseLevelsComputeProbGenContext1ResponseLevels | None = (
        DakotaField(
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
    )


class ResponseLevelsComputeProbGenContext2Mixin(DakotaBaseModel):
    """Generated model for ResponseLevelsComputeProbGenContext2Mixin"""

    response_levels: ResponseLevelsComputeProbGenContext2ResponseLevels | None = (
        DakotaField(
            default=None,
            description="Undocumented: Recursive k-d (RKD) Darts is an experimental capability.",
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
    )


class MixedGradients(DakotaBaseModel):
    """Generated model for MixedGradients"""

    mixed_gradients: MixedGradientsConfig = DakotaField(
        description="Gradients are needed and will be obtained from a mix of numerical and analytic sources",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.gradient_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "mixed",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class NumericalGradients(DakotaBaseModel):
    """Generated model for NumericalGradients"""

    numerical_gradients: NumericalGradientsConfig = DakotaField(
        description="Gradients are needed and will be approximated by finite differences",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.gradient_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "numerical",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MixedHessians(DakotaBaseModel):
    """Generated model for MixedHessians"""

    mixed_hessians: MixedHessiansConfig = DakotaField(
        description='Hessians are needed and will be obtained from a mix of numerical, analytic, and "quasi" sources',
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.hessian_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "mixed",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ResponseGradientsMixin(DakotaBaseModel):
    """Generated model for ResponseGradientsMixin"""

    gradient_type: Union[
        NoGradients, AnalyticGradients, MixedGradients, NumericalGradients
    ] = DakotaField(
        description="Gradient Type", dakota={"anchor": True, "union_pattern": 4}
    )


class ResponseHessiansMixin(DakotaBaseModel):
    """Generated model for ResponseHessiansMixin"""

    hessian_type: Union[
        NoHessians, NumericalHessians, QuasiHessians, AnalyticHessians, MixedHessians
    ] = DakotaField(
        description="Hessian Type", dakota={"anchor": True, "union_pattern": 4}
    )
