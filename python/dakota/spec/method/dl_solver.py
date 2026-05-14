"""Generated Pydantic models for method.dl_solver"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.misc import (
    DefaultScalingMixin,
    MethodMaxFunctionEvaluationsContext1Mixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
)


class DlSolverConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodMaxFunctionEvaluationsContext1Mixin,
    DefaultScalingMixin,
    MethodOptionalModelPointerMixin,
):
    "(Experimental) Dynamically-loaded solver"

    name: str = DakotaField(
        description="(Experimental) Dynamically-loaded solver",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.dl_solver.dlDetails",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        }
    )

class DlSolverSelection(MethodSelection):
    "Generated model for DlSolverSelection"

    dl_solver: DlSolverConfig = DakotaField(
        dakota={
            "argument": "name",
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DL_SOLVER",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
