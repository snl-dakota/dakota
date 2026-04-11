"""Generated Pydantic models for method.coliny_beta"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.misc import (
    ColinyCommonOptsMixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
)


class ColinyBetaConfig(
    MethodThreeOptionalKeywordsMixin,
    ColinyCommonOptsMixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for ColinyBetaConfig"""

    beta_solver_name: str = DakotaField(
        description="Use an in-development SCOLIB solver",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.beta_solver_name",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ColinyBetaSelection(MethodSelection):
    """Generated model for ColinyBetaSelection"""

    coliny_beta: ColinyBetaConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "COLINY_BETA",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
