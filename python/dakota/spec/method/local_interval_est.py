"""Generated Pydantic models for method.local_interval_est"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.core import MethodConvergenceTolMixin
from dakota.spec.shared.misc import (
    MethodGradientSubProblemSolverMixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
)


class LocalIntervalEstConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodGradientSubProblemSolverMixin,
    MethodConvergenceTolMixin,
    MethodOptionalModelPointerMixin,
):
    "Interval analysis using local optimization"

    pass


class LocalIntervalEstSelection(MethodSelection):
    "Generated model for LocalIntervalEstSelection"

    local_interval_est: LocalIntervalEstConfig = DakotaField(
        dakota={
            "aliases": ["nond_local_interval_est"],
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LOCAL_INTERVAL_EST",
                    "ir_value_type": "unsigned short",
                }
            ],
        }
    )
