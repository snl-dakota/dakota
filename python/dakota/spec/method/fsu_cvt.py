"""Generated Pydantic models for method.fsu_cvt"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField
from .base import MethodSelection
from typing import Literal, Union

# Cross-module model imports
from dakota.spec.shared.misc import (
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
    MethodSeedMixin,
    MethodThreeOptionalKeywordsMixin,
    VbdSamplingMixin,
)


class TrialTypeGrid(DakotaBaseModel):
    """Generated model for TrialTypeGrid"""

    grid: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.trial_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "grid",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class TrialTypeHalton(DakotaBaseModel):
    """Generated model for TrialTypeHalton"""

    halton: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.trial_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "halton",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class TrialTypeRandom(DakotaBaseModel):
    """Generated model for TrialTypeRandom"""

    random: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.trial_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "random",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class FsuCvtConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodSeedMixin,
    VbdSamplingMixin,
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for FsuCvtConfig"""

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
    trial_type: Union[TrialTypeGrid, TrialTypeHalton, TrialTypeRandom] | None = (
        DakotaField(
            default=None,
            description="Specify how the trial samples are generated",
            dakota={"union_pattern": 2},
        )
    )
    num_trials: int = DakotaField(
        default=10000,
        description="The number of secondary sample points generated to adjust the location of the primary sample points",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.fsu_cvt.num_trials",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )


class FsuCvtSelection(MethodSelection):
    """Generated model for FsuCvtSelection"""

    fsu_cvt: FsuCvtConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "FSU_CVT",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
