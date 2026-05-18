"""Generated Pydantic models for method.rol"""

from __future__ import annotations

from ..base import DakotaField, DakotaFloat
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.core import DefaultConstraintTolMixin
from dakota.spec.shared.misc import (
    DefaultScalingMixin,
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
)


class RolConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodMaxIterationsContext1Mixin,
    DefaultConstraintTolMixin,
    DefaultScalingMixin,
    MethodOptionalModelPointerMixin,
):
    "Rapid Optimization Library (ROL) is a large-scale optimization package within Trilinos."

    variable_tolerance: DakotaFloat = DakotaField(
        default=0.01,
        description="Step length-based stopping criteria for derivative-free optimizers",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.variable_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    gradient_tolerance: DakotaFloat = DakotaField(
        default=0.0001,
        description="Stopping critiera based on L2 norm of gradient",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.gradient_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    options_file: str | None = DakotaField(
        default=None,
        description="File containing advanced ROL options",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.advanced_options_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class RolSelection(MethodSelection):
    "Generated model for RolSelection"

    rol: RolConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ROL",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
