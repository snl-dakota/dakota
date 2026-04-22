"""Generated Pydantic models for method.npsol_sqp"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.misc import (
    MethodNpsolCommonOptsMixin,
    MethodThreeOptionalKeywordsMixin,
)


class NpsolSqpConfig(MethodThreeOptionalKeywordsMixin, MethodNpsolCommonOptsMixin):
    "NPSOL Sequential Quadratic Program"

    pass


class NpsolSqpSelection(MethodSelection):
    "Generated model for NpsolSqpSelection"

    npsol_sqp: NpsolSqpConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NPSOL_SQP",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
