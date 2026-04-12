"""Generated Pydantic models for method.nl2sol"""

from __future__ import annotations

from ..base import DakotaField, DakotaFloat
from .base import MethodSelection
from typing import Literal

# Cross-module model imports
from dakota.spec.shared.core import MethodConvergenceTolMixin
from dakota.spec.shared.misc import (
    DefaultScalingMixin,
    DefaultSpeculativeMixin,
    MethodMaxFunctionEvaluationsContext1Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
)


class Nl2solConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodConvergenceTolMixin,
    MethodMaxIterationsContext1Mixin,
    DefaultSpeculativeMixin,
    MethodMaxFunctionEvaluationsContext1Mixin,
    DefaultScalingMixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for Nl2solConfig"""

    function_precision: DakotaFloat = DakotaField(
        default=1.0e-10,
        description="Specify the maximum precision of the analysis code responses",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.function_precision",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    absolute_conv_tol: DakotaFloat = DakotaField(
        default=-1.0,
        description="Absolute convergence tolerance",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nl2sol.absolute_conv_tol",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    x_conv_tol: DakotaFloat = DakotaField(
        default=-1.0,
        description="X-convergence tolerance",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.x_conv_tol",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    singular_conv_tol: DakotaFloat = DakotaField(
        default=-1.0,
        description="Singular convergence tolerance",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nl2sol.singular_conv_tol",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    singular_radius: DakotaFloat = DakotaField(
        default=-1.0,
        description="Singular radius",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nl2sol.singular_radius",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    false_conv_tol: DakotaFloat = DakotaField(
        default=-1.0,
        description="False convergence tolerance",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nl2sol.false_conv_tol",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    initial_trust_radius: DakotaFloat = DakotaField(
        default=-1.0,
        description="Initial trust region radius",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nl2sol.initial_trust_radius",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    covariance: int = DakotaField(
        default=0,
        description="Determine how the final covariance matrix is computed",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nl2sol.covariance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    regression_diagnostics: Literal[True] | None = DakotaField(
        default=None,
        description="Turn on regression diagnostics",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nl2sol.regression_diagnostics",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class Nl2solSelection(MethodSelection):
    """Generated model for Nl2solSelection"""

    nl2sol: Nl2solConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "NL2SOL",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
