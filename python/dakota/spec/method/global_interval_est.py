"""Generated Pydantic models for method.global_interval_est"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.core import MethodConvergenceTolMixin
from dakota.spec.shared.misc import (
    MethodGlobalSubProblemSolverContext2Mixin,
    MethodMaxFunctionEvaluationsContext1Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
    RngOptionsContext2Mixin,
)


class GlobalIntervalEstConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodMaxIterationsContext1Mixin,
    MethodConvergenceTolMixin,
    MethodMaxFunctionEvaluationsContext1Mixin,
    MethodGlobalSubProblemSolverContext2Mixin,
    RngOptionsContext2Mixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for GlobalIntervalEstConfig"""

    samples: int = DakotaField(
        default=0,
        description="Number of samples for sampling-based methods",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    seed: int | None = DakotaField(
        default=None,
        gt=0,
        description="Seed of the random number generator",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.random_seed",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )


class GlobalIntervalEstSelection(MethodSelection):
    """Generated model for GlobalIntervalEstSelection"""

    global_interval_est: GlobalIntervalEstConfig = DakotaField(
        dakota={
            "aliases": ["nond_global_interval_est"],
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "GLOBAL_INTERVAL_EST",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        }
    )
