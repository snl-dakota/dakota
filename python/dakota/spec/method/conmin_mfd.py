"""Generated Pydantic models for method.conmin_mfd"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.misc import MethodThreeOptionalKeywordsMixin
from dakota.spec.shared.optimization import MethodConminCommonOptsMixin


class ConminMfdConfig(MethodThreeOptionalKeywordsMixin, MethodConminCommonOptsMixin):
    "CONMIN method of feasible directions"

    pass


class ConminMfdSelection(MethodSelection):
    "Generated model for ConminMfdSelection"

    conmin_mfd: ConminMfdConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "CONMIN_MFD",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
