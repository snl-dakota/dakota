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
    """Generated model for DlSolverConfig"""

    name: str = DakotaField(description="(Experimental) Dynamically-loaded solver")


class DlSolverSelection(MethodSelection):
    """Generated model for DlSolverSelection"""

    dl_solver: DlSolverConfig
