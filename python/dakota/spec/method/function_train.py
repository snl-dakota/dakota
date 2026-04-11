"""Generated Pydantic models for method.function_train"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.core import MethodExpConvergenceToleranceMixin
from dakota.spec.shared.expansion.function_train import (
    FtMethodOrderMixin,
    FtMethodRankMixin,
    FtMethodRefinementMixin,
    FtMethodRegressionMixin,
    FtMethodTolsMixin,
)
from dakota.spec.shared.expansion.options import ExpansionOptionsMixin
from dakota.spec.shared.misc import (
    MethodOptionalModelPointerMixin,
    MethodSeedMixin,
    MethodThreeOptionalKeywordsMixin,
)


class FtConfig(
    MethodThreeOptionalKeywordsMixin,
    FtMethodRefinementMixin,
    MethodExpConvergenceToleranceMixin,
    FtMethodRegressionMixin,
    FtMethodTolsMixin,
    FtMethodOrderMixin,
    FtMethodRankMixin,
    ExpansionOptionsMixin,
    MethodSeedMixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for FtConfig"""

    pass


class FtSelection(MethodSelection):
    """Generated model for FtSelection"""

    function_train: FtConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "C3_FUNCTION_TRAIN",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
