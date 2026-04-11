"""Generated Pydantic models for method.ncsu_direct"""

from __future__ import annotations

from ..base import DakotaField, DakotaFloat
from .base import MethodSelection
from ..base import DBL_MAX

# Cross-module model imports
from dakota.spec.shared.core import MethodConvergenceTolMixin
from dakota.spec.shared.misc import (
    DefaultScalingMixin,
    MethodMaxFunctionEvaluationsContext1Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
)


class NcsuDirectConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodConvergenceTolMixin,
    MethodMaxIterationsContext1Mixin,
    MethodMaxFunctionEvaluationsContext1Mixin,
    DefaultScalingMixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for NcsuDirectConfig"""

    solution_target: DakotaFloat = DakotaField(
        default=-DBL_MAX,
        description="Specifies a globally optimal value toward which the optimizer should track",
        dakota={
            "aliases": ["solution_accuracy"],
            "materialization": [
                {
                    "ir_key": "method.asynch_pattern_search.solution_target",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
                {
                    "ir_key": "method.solution_target",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
            ],
        },
    )
    min_boxsize_limit: DakotaFloat = DakotaField(
        default=-1.0,
        description="Stopping Criterion based on shortest edge of hyperrectangle",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.min_boxsize_limit",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    volume_boxsize_limit: DakotaFloat = DakotaField(
        default=-1.0,
        description="Stopping criterion based on volume of search space",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.volume_boxsize_limit",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class NcsuDirectSelection(MethodSelection):
    """Generated model for NcsuDirectSelection"""

    ncsu_direct: NcsuDirectConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "NCSU_DIRECT",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
