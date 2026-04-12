"""Generated Pydantic models for method.fsu_quasi_mc"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField
from .base import MethodSelection
from typing import Literal, Union

# Cross-module model imports
from dakota.spec.shared.misc import (
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
    VbdSamplingMixin,
)


class FsuQuasiMcHalton(DakotaBaseModel):
    """Generated model for FsuQuasiMcHalton"""

    halton: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "FSU_HALTON",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Hammersley(DakotaBaseModel):
    """Generated model for Hammersley"""

    hammersley: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "FSU_HAMMERSLEY",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class FsuQuasiMcConfig(
    MethodThreeOptionalKeywordsMixin,
    VbdSamplingMixin,
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for FsuQuasiMcConfig"""

    sequence: Union[FsuQuasiMcHalton, Hammersley] = DakotaField(
        description="Sequence Type", dakota={"anchor": True, "union_pattern": 4}
    )
    latinize: Literal[True] | None = DakotaField(
        default=None,
        description="Adjust samples to improve the discrepancy of the marginal distributions",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.latinize",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    quality_metrics: Literal[True] | None = DakotaField(
        default=None,
        description="Calculate metrics to assess the quality of quasi-Monte Carlo samples",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.quality_metrics",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    samples: int = DakotaField(
        default=0,
        description="Number of samples for sampling-based methods",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    fixed_sequence: Literal[True] | None = DakotaField(
        default=None,
        description="Reuse the same sequence and samples for multiple sampling sets",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.fsu_quasi_mc.fixed_sequence",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    sequence_start: list[int] | None = DakotaField(
        default=None,
        description="Choose where to start sampling the sequence",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.fsu_quasi_mc.sequenceStart",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )
    sequence_leap: list[int] | None = DakotaField(
        default=None,
        description="Specify how often the sequence is sampled",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.fsu_quasi_mc.sequenceLeap",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )
    prime_base: list[int] | None = DakotaField(
        default=None,
        description="The prime numbers used to generate the sequence",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.fsu_quasi_mc.primeBase",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )


class FsuQuasiMcSelection(MethodSelection):
    """Generated model for FsuQuasiMcSelection"""

    fsu_quasi_mc: FsuQuasiMcConfig
