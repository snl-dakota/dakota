"""Generated Pydantic models for method.nonlinear_cg"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.core import MethodConvergenceTolMixin
from dakota.spec.shared.misc import (
    DefaultScalingMixin,
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
)


class NonlinearCgConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodConvergenceTolMixin,
    MethodMaxIterationsContext1Mixin,
    DefaultScalingMixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for NonlinearCgConfig"""

    misc_options: list[str] | None = DakotaField(
        default=None,
        description="Options for nonlinear CG optimizer",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.misc_options",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )


class NonlinearCgSelection(MethodSelection):
    """Generated model for NonlinearCgSelection"""

    nonlinear_cg: NonlinearCgConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NONLINEAR_CG",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
