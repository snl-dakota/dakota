"""Generated Pydantic models for shared.optimization"""

from __future__ import annotations

from ...base import DakotaBaseModel, DakotaField

# Cross-module model imports
from dakota.spec.shared.core import DefaultConstraintTolMixin, MethodConvergenceTolMixin
from dakota.spec.shared.misc import (
    DefaultScalingMixin,
    DefaultSpeculativeMixin,
    MethodMaxFunctionEvaluationsContext1Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
)


class MethodConminCommonOptsMixin(
    DefaultConstraintTolMixin,
    DefaultScalingMixin,
    DefaultSpeculativeMixin,
    MethodConvergenceTolMixin,
    MethodMaxFunctionEvaluationsContext1Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for MethodConminCommonOptsMixin"""

    pass
