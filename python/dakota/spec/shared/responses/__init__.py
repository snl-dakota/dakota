"""Generated Pydantic models for shared.responses"""

from __future__ import annotations

from ...base import DakotaBaseModel, DakotaField, DakotaFloat
from typing import Literal, Union
from typing import ClassVar, List
from ...validation import ValidationRule
from ...validation.rules import CheckSumEqualsLength, CompareLength, CompareLengthOne


class NoGradients(DakotaBaseModel):
    "Gradients will not be used"

    no_gradients: Literal[True] = DakotaField(
        default=True,
        description="Gradients will not be used",
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
    "Analysis driver will return gradients"

    analytic_gradients: Literal[True] = DakotaField(
        default=True,
        description="Analysis driver will return gradients",
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
    "(Default) Scale step size by the parameter value"

    relative: Literal[True] = DakotaField(
        default=True,
        description="(Default) Scale step size by the parameter value",
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
    "Do not scale step-size"

    absolute: Literal[True] = DakotaField(
        default=True,
        description="Do not scale step-size",
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
    "Scale step-size by the domain of the parameter"

    bounds: Literal[True] = DakotaField(
        default=True,
        description="Scale step-size by the domain of the parameter",
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
    "Use non-Dakota fd algorithm"

    vendor: Literal[True] = DakotaField(
        default=True,
        description="Use non-Dakota fd algorithm",
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
    "(Default) Use forward differences"

    forward: Literal[True] = DakotaField(
        default=True,
        description="(Default) Use forward differences",
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
    "Use central differences"

    central: Literal[True] = DakotaField(
        default=True,
        description="Use central differences",
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
    "(Default) Scale step size by the parameter value"

    relative: Literal[True] = DakotaField(
        default=True,
        description="(Default) Scale step size by the parameter value",
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
    "Do not scale step-size"

    absolute: Literal[True] = DakotaField(
        default=True,
        description="Do not scale step-size",
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
    "Scale step-size by the domain of the parameter"

    bounds: Literal[True] = DakotaField(
        default=True,
        description="Scale step-size by the domain of the parameter",
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
    "Use non-Dakota fd algorithm"

    vendor: Literal[True] = DakotaField(
        default=True,
        description="Use non-Dakota fd algorithm",
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
    "(Default) Use forward differences"

    forward: Literal[True] = DakotaField(
        default=True,
        description="(Default) Use forward differences",
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
    "Use central differences"

    central: Literal[True] = DakotaField(
        default=True,
        description="Use central differences",
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
    "Hessians will not be used"

    no_hessians: Literal[True] = DakotaField(
        default=True,
        description="Hessians will not be used",
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
    "(Default) Scale step size by the parameter value"

    relative: Literal[True] = DakotaField(
        default=True,
        description="(Default) Scale step size by the parameter value",
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
    "Do not scale step-size"

    absolute: Literal[True] = DakotaField(
        default=True,
        description="Do not scale step-size",
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
    "Scale step-size by the domain of the parameter"

    bounds: Literal[True] = DakotaField(
        default=True,
        description="Scale step-size by the domain of the parameter",
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
    "(Default) Use forward differences"

    forward: Literal[True] = DakotaField(
        default=True,
        description="(Default) Use forward differences",
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
    "Use central differences"

    central: Literal[True] = DakotaField(
        default=True,
        description="Use central differences",
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
    "Use BFGS method to compute quasi-hessians"

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
    "Use the Symmetric Rank 1 update method to compute quasi-Hessians"

    sr1: Literal[True] = DakotaField(
        default=True,
        description="Use the Symmetric Rank 1 update method to compute quasi-Hessians",
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
    "Hessians are needed and are available directly from the analysis driver"

    analytic_hessians: Literal[True] = DakotaField(
        default=True,
        description="Hessians are needed and are available directly from the analysis driver",
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
    "Identify which numerical-Hessian corresponds to which response"

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
    "(Default) Scale step size by the parameter value"

    relative: Literal[True] = DakotaField(
        default=True,
        description="(Default) Scale step size by the parameter value",
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
    "Do not scale step-size"

    absolute: Literal[True] = DakotaField(
        default=True,
        description="Do not scale step-size",
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
    "Scale step-size by the domain of the parameter"

    bounds: Literal[True] = DakotaField(
        default=True,
        description="Scale step-size by the domain of the parameter",
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
    "(Default) Use forward differences"

    forward: Literal[True] = DakotaField(
        default=True,
        description="(Default) Use forward differences",
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
    "Use central differences"

    central: Literal[True] = DakotaField(
        default=True,
        description="Use central differences",
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
    "Use BFGS method to compute quasi-hessians"

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
    "Use the Symmetric Rank 1 update method to compute quasi-Hessians"

    sr1: Literal[True] = DakotaField(
        default=True,
        description="Use the Symmetric Rank 1 update method to compute quasi-Hessians",
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
    "Computes probabilities associated with response levels"

    probabilities: Literal[True] = DakotaField(
        default=True,
        description="Computes probabilities associated with response levels",
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
    "Computes generalized reliabilities associated with response levels"

    gen_reliabilities: Literal[True] = DakotaField(
        default=True,
        description="Computes generalized reliabilities associated with response levels",
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
    "Aggregate response statistics assuming a series system"

    series: Literal[True] = DakotaField(
        default=True,
        description="Aggregate response statistics assuming a series system",
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
    "Aggregate response statistics assuming a parallel system"

    parallel: Literal[True] = DakotaField(
        default=True,
        description="Aggregate response statistics assuming a parallel system",
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
    "Computes probabilities associated with response levels"

    probabilities: Literal[True] = DakotaField(
        default=True,
        description="Computes probabilities associated with response levels",
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
    "Computes generalized reliabilities associated with response levels"

    gen_reliabilities: Literal[True] = DakotaField(
        default=True,
        description="Computes generalized reliabilities associated with response levels",
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
    "Aggregate response statistics assuming a series system"

    series: Literal[True] = DakotaField(
        default=True,
        description="Aggregate response statistics assuming a series system",
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
    "Aggregate response statistics assuming a parallel system"

    parallel: Literal[True] = DakotaField(
        default=True,
        description="Aggregate response statistics assuming a parallel system",
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
    "Selects custom-annotated tabular file format for experiment data"

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
    "Selects annotated tabular file format for experiment data"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format for experiment data",
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
    "Selects free-form tabular file format for experiment data"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects free-form tabular file format for experiment data",
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
    "(Default) Use internal Dakota finite differences algorithm"

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
    "(Default) Use internal Dakota finite differences algorithm"

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
    "Hessians are needed and will be approximated by finite differences"

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
    "Use BFGS method to compute quasi-hessians"

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
    "Use BFGS method to compute quasi-hessians"

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
    "Selection of statistics to compute at each response level"

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
    "Selection of statistics to compute at each response level"

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
    "Selects custom-annotated tabular file format for experiment data"

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
    "(Default) Use internal Dakota finite differences algorithm"

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
    "(Default) Use internal Dakota finite differences algorithm"

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
    "Hessians are needed and will be approximated by finite differences"

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
    "Hessians are needed and will be approximated by secant updates (BFGS or SR1) from a series of gradient evaluations"

    quasi_hessians: Union[QuasiHessiansBfgs, QuasiHessiansSr1] = DakotaField(
        description="Hessians are needed and will be approximated by secant updates (BFGS or SR1) from a series of gradient evaluations",
        dakota={
            "materialization": [
                {
                    "ir_key": "responses.hessian_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "quasi",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class IdQuasiHessians(DakotaBaseModel):
    "Identify which quasi-Hessian corresponds to which response"

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
    "Values at which to estimate desired statistics for each response"

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
    "Undocumented: Recursive k-d (RKD) Darts is an experimental capability."

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
    "Generated model for ResponseScalarDataFormatMixin"

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
    "Gradients are needed and will be obtained from a mix of numerical and analytic sources"

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
    "Gradients are needed and will be approximated by finite differences"

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
    'Hessians are needed and will be obtained from a mix of numerical, analytic, and \\"quasi\\" sources'

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
    "Generated model for ResponseLevelsComputeProbGenContext1Mixin"

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
    "Generated model for ResponseLevelsComputeProbGenContext2Mixin"

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
    "Gradients are needed and will be obtained from a mix of numerical and analytic sources"

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
    "Gradients are needed and will be approximated by finite differences"

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
    'Hessians are needed and will be obtained from a mix of numerical, analytic, and \\"quasi\\" sources'

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
    "Generated model for ResponseGradientsMixin"

    gradient_type: Union[
        NoGradients, AnalyticGradients, MixedGradients, NumericalGradients
    ] = DakotaField(
        description="Gradient Type", dakota={"anchor": True, "union_pattern": 4}
    )


class ResponseHessiansMixin(DakotaBaseModel):
    "Generated model for ResponseHessiansMixin"

    hessian_type: Union[
        NoHessians, NumericalHessians, QuasiHessians, AnalyticHessians, MixedHessians
    ] = DakotaField(
        description="Hessian Type", dakota={"anchor": True, "union_pattern": 4}
    )
