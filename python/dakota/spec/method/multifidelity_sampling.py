"""Generated Pydantic models for method.multifidelity_sampling"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.core import MethodConvergenceTolWithTypeContext3Mixin
from dakota.spec.shared.misc import (
    MethodMaxFunctionEvaluationsContext2Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
    MethodSeedSequenceMixin,
    MethodThreeOptionalKeywordsMixin,
    MlmfSolutionModeMixin,
    PromotedModelSelectionContext1Mixin,
    RngOptionsContext2Mixin,
)
from dakota.spec.shared.sampling import (
    MethodExportSamplesFormatMixin,
    MethodSampleTypeLhsMcMixin,
    MfmcNumericalSolveMixin,
    MlmfPilotSamplesContext2Mixin,
)


class MultifidelitySamplingExportSampleSequence(MethodExportSamplesFormatMixin):
    """Generated model for MultifidelitySamplingExportSampleSequence"""

    pass


class MultifidelitySamplingConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodSeedSequenceMixin,
    MlmfPilotSamplesContext2Mixin,
    MlmfSolutionModeMixin,
    MfmcNumericalSolveMixin,
    PromotedModelSelectionContext1Mixin,
    MethodSampleTypeLhsMcMixin,
    MethodConvergenceTolWithTypeContext3Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodMaxFunctionEvaluationsContext2Mixin,
    RngOptionsContext2Mixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for MultifidelitySamplingConfig"""

    export_sample_sequence: MultifidelitySamplingExportSampleSequence | None = (
        DakotaField(
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
    )


class MultifidelitySamplingSelection(MethodSelection):
    """Generated model for MultifidelitySamplingSelection"""

    multifidelity_sampling: MultifidelitySamplingConfig = DakotaField(
        dakota={
            "aliases": ["multifidelity_mc", "mfmc"],
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MULTIFIDELITY_SAMPLING",
                    "ir_value_type": "unsigned short",
                }
            ],
        }
    )
