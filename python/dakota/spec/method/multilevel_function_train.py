"""Generated Pydantic models for method.multilevel_function_train"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.core import MethodExpConvergenceToleranceMixin
from dakota.spec.shared.expansion.function_train import (
    FtMethodRegressionSequenceMixin,
    FtMethodTolsMixin,
    MlftAllocControlMixin,
    MlftMethodOrderSequenceMixin,
    MlftMethodRankSequenceMixin,
)
from dakota.spec.shared.expansion.options import ExpansionOptionsMixin
from dakota.spec.shared.misc import (
    DiscrepEmulationMixin,
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
    MethodSeedSequenceMixin,
    MethodThreeOptionalKeywordsMixin,
)


class MultilevelFtConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodMaxIterationsContext1Mixin,
    MlftAllocControlMixin,
    MethodExpConvergenceToleranceMixin,
    DiscrepEmulationMixin,
    FtMethodTolsMixin,
    FtMethodRegressionSequenceMixin,
    MlftMethodOrderSequenceMixin,
    MlftMethodRankSequenceMixin,
    ExpansionOptionsMixin,
    MethodSeedSequenceMixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for MultilevelFtConfig"""

    pass


class MultilevelFtSelection(MethodSelection):
    """Generated model for MultilevelFtSelection"""

    multilevel_function_train: MultilevelFtConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "MULTILEVEL_FUNCTION_TRAIN",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
