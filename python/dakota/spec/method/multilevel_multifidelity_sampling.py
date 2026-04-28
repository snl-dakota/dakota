"""Generated Pydantic models for method.multilevel_multifidelity_sampling"""

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
    RngOptionsContext2Mixin,
)
from dakota.spec.shared.sampling import (
    MethodExportSamplesFormatMixin,
    MethodSampleTypeLhsMcMixin,
    MlmfPilotSamplesContext2Mixin,
)


class MultilevelMultifidelitySamplingExportSampleSequence(
    MethodExportSamplesFormatMixin
):
    "Enable export of multilevel/multifidelity sample sequences to individual files"

    pass


class MultilevelMultifidelitySamplingConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodSeedSequenceMixin,
    MlmfPilotSamplesContext2Mixin,
    MlmfSolutionModeMixin,
    MethodSampleTypeLhsMcMixin,
    MethodConvergenceTolWithTypeContext3Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodMaxFunctionEvaluationsContext2Mixin,
    RngOptionsContext2Mixin,
    MethodOptionalModelPointerMixin,
):
    "Multilevel-Multifidelity sampling methods for UQ"

    export_sample_sequence: (
        MultilevelMultifidelitySamplingExportSampleSequence | None
    ) = DakotaField(
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


class MultilevelMultifidelitySamplingSelection(MethodSelection):
    "Generated model for MultilevelMultifidelitySamplingSelection"

    multilevel_multifidelity_sampling: MultilevelMultifidelitySamplingConfig = (
        DakotaField(
            dakota={
                "aliases": ["multilevel_multifidelity_mc", "mlmfmc"],
                "materialization": [
                    {
                        "ir_key": "method.algorithm",
                        "storage_type": "PRESENCE_ENUM",
                        "stored_value": "MULTILEVEL_MULTIFIDELITY_SAMPLING",
                        "ir_value_type": "unsigned short",
                    }
                ],
            }
        )
    )
