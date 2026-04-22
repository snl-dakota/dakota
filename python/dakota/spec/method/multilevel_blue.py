"""Generated Pydantic models for method.multilevel_blue"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.core import MethodConvergenceTolWithTypeContext3Mixin
from dakota.spec.shared.misc import (
    MethodMaxFunctionEvaluationsContext2Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodMlmfSolverMetricMixin,
    MethodMlmfSubProblemSolverMixin,
    MethodOptionalModelPointerMixin,
    MethodSeedSequenceMixin,
    MethodThreeOptionalKeywordsMixin,
    MlmfGroupThrottleMixin,
    MlmfSolutionModeMixin,
    RngOptionsContext2Mixin,
)
from dakota.spec.shared.sampling import (
    MethodExportSamplesFormatMixin,
    MethodSampleTypeLhsMcMixin,
    MlmfGroupPilotSamplesMixin,
)


class MultilevelBlueExportSampleSequence(MethodExportSamplesFormatMixin):
    "Enable export of multilevel/multifidelity sample sequences to individual files"

    pass


class MultilevelBlueConfig(
    MethodThreeOptionalKeywordsMixin,
    MlmfGroupThrottleMixin,
    MlmfGroupPilotSamplesMixin,
    MlmfSolutionModeMixin,
    MethodMlmfSubProblemSolverMixin,
    MethodMlmfSolverMetricMixin,
    MethodSeedSequenceMixin,
    MethodSampleTypeLhsMcMixin,
    MethodConvergenceTolWithTypeContext3Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodMaxFunctionEvaluationsContext2Mixin,
    RngOptionsContext2Mixin,
    MethodOptionalModelPointerMixin,
):
    "The multilevel best linear unbiased estimator (ML BLUE) sampling method for UQ"

    export_sample_sequence: MultilevelBlueExportSampleSequence | None = DakotaField(
        default=None,
        description="Enable export of multilevel/multifidelity sample sequences to individual files",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_sample_sequence",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class MultilevelBlueSelection(MethodSelection):
    "Generated model for MultilevelBlueSelection"

    multilevel_blue: MultilevelBlueConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MULTILEVEL_BLUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
