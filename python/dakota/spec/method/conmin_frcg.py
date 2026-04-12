"""Generated Pydantic models for method.conmin_frcg"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.misc import MethodThreeOptionalKeywordsMixin
from dakota.spec.shared.optimization import MethodConminCommonOptsMixin


class ConminFrcgConfig(MethodThreeOptionalKeywordsMixin, MethodConminCommonOptsMixin):
    """Generated model for ConminFrcgConfig"""

    pass


class ConminFrcgSelection(MethodSelection):
    """Generated model for ConminFrcgSelection"""

    conmin_frcg: ConminFrcgConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "CONMIN_FRCG",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
