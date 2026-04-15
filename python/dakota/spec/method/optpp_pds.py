"""Generated Pydantic models for method.optpp_pds"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.core import MethodConvergenceTolMixin
from dakota.spec.shared.misc import (
    DefaultScalingMixin,
    MethodMaxFunctionEvaluationsContext1Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
)


class OptppPdsConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodMaxIterationsContext1Mixin,
    MethodConvergenceTolMixin,
    MethodMaxFunctionEvaluationsContext1Mixin,
    DefaultScalingMixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for OptppPdsConfig"""

    search_scheme_size: int = DakotaField(
        default=32,
        description="Number of points to be used in the direct search template",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.optpp.search_scheme_size",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )


class OptppPdsSelection(MethodSelection):
    """Generated model for OptppPdsSelection"""

    optpp_pds: OptppPdsConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "OPTPP_PDS",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
