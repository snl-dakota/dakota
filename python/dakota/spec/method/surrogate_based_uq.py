"""Generated Pydantic models for method.surrogate_based_uq"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.expansion.options import ExpansionOptionsMixin
from dakota.spec.shared.misc import (
    MethodOptionalModelPointerMixin,
    MethodSeedMixin,
    MethodThreeOptionalKeywordsMixin,
)


class SurrogateBasedUqConfig(
    MethodThreeOptionalKeywordsMixin,
    ExpansionOptionsMixin,
    MethodSeedMixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for SurrogateBasedUqConfig"""

    pass


class SurrogateBasedUqSelection(MethodSelection):
    """Generated model for SurrogateBasedUqSelection"""

    surrogate_based_uq: SurrogateBasedUqConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "SURROGATE_BASED_UQ",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
