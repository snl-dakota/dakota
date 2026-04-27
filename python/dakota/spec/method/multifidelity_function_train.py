"""Generated Pydantic models for method.multifidelity_function_train"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.core import MethodMfExpConvergenceToleranceMixin
from dakota.spec.shared.expansion.function_train import (
    FtMethodRefinementMixin,
    FtMethodRegressionSequenceMixin,
    FtMethodTolsMixin,
    MlftMethodOrderSequenceMixin,
    MlftMethodRankSequenceMixin,
)
from dakota.spec.shared.expansion.options import ExpansionOptionsMixin
from dakota.spec.shared.misc import (
    DiscrepEmulationMixin,
    MethodOptionalModelPointerMixin,
    MethodSeedSequenceMixin,
    MethodThreeOptionalKeywordsMixin,
    MfAllocControlContext3Mixin,
)


class MultifidelityFtConfig(
    MethodThreeOptionalKeywordsMixin,
    FtMethodRefinementMixin,
    MethodMfExpConvergenceToleranceMixin,
    MfAllocControlContext3Mixin,
    DiscrepEmulationMixin,
    FtMethodTolsMixin,
    FtMethodRegressionSequenceMixin,
    MlftMethodOrderSequenceMixin,
    MlftMethodRankSequenceMixin,
    ExpansionOptionsMixin,
    MethodSeedSequenceMixin,
    MethodOptionalModelPointerMixin,
):
    "Multifidelity uncertainty quantification using function train expansions"

    pass


class MultifidelityFtSelection(MethodSelection):
    "Generated model for MultifidelityFtSelection"

    multifidelity_function_train: MultifidelityFtConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MULTIFIDELITY_FUNCTION_TRAIN",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
