"""Generated Pydantic models for method.nlpql_sqp"""

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


class NlpqlSqpConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodMaxIterationsContext1Mixin,
    MethodConvergenceTolMixin,
    MethodMaxFunctionEvaluationsContext1Mixin,
    DefaultScalingMixin,
    MethodOptionalModelPointerMixin,
):
    "NLPQL Sequential Quadratic Program"

    pass


class NlpqlSqpSelection(MethodSelection):
    "Generated model for NlpqlSqpSelection"

    nlpql_sqp: NlpqlSqpConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NLPQL_SQP",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
