"""Generated Pydantic models for method.sampling"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField, DakotaFloat
from .base import MethodSelection
from typing import Literal, Union

# Cross-module model imports
from dakota.spec.shared.misc import (
    DefaultFinalMomentsMixin,
    DigitalNetOptionsMixin,
    LevelMappingsMixin,
    MethodOptionalModelPointerMixin,
    MethodSeedMixin,
    MethodThreeOptionalKeywordsMixin,
    Rank1LatticeOptionsMixin,
    RngOptionsContext2Mixin,
    VbdSamplingMixin,
)


class SamplingSampleTypeLhs(DakotaBaseModel):
    """Generated model for SamplingSampleTypeLhs"""

    lhs: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sample_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_LHS",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class SamplingSampleTypeRandom(DakotaBaseModel):
    """Generated model for SamplingSampleTypeRandom"""

    random: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sample_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_RANDOM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class IncrementalLhs(DakotaBaseModel):
    """Generated model for IncrementalLhs"""

    incremental_lhs: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sample_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_LHS",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class IncrementalRandom(DakotaBaseModel):
    """Generated model for IncrementalRandom"""

    incremental_random: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sample_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_RANDOM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Rank1Lattice(Rank1LatticeOptionsMixin):
    """Generated model for Rank1Lattice"""

    pass


class DigitalNet(DigitalNetOptionsMixin):
    """Generated model for DigitalNet"""

    pass


class CandidateDesigns(DakotaBaseModel):
    """Generated model for CandidateDesigns"""

    candidate_designs: int = DakotaField(
        gt=0,
        description="Number of candidate sampling designs from which to select the most D-optimal",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.num_candidate_designs",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class LejaOversampleRatio(DakotaBaseModel):
    """Generated model for LejaOversampleRatio"""

    leja_oversample_ratio: DakotaFloat = DakotaField(
        description="Oversampling ratio for generating candidate point set",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class SamplingPrincipalComponents(DakotaBaseModel):
    """Generated model for SamplingPrincipalComponents"""

    percent_variance_explained: DakotaFloat = DakotaField(
        default=0.95,
        description="Specifies the number of components to retain to explain the specified percent variance.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.percent_variance_explained",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class Wilks(DakotaBaseModel):
    """Generated model for Wilks"""

    order: int = DakotaField(
        default=1,
        description="The order of the statistics to use when determining sample sizes for random sampling using Wilks order statistics.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    confidence_level: DakotaFloat = DakotaField(
        default=0.95,
        description="The confidence level to be used when determining sample sizes for random sampling using Wilks order statistics.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.confidence_level",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    one_sided_lower: Literal[True] | None = DakotaField(
        default=None,
        description="Specifies one-sided lower portion order statistics to be used when determining sample sizes for random sampling using Wilks order statistics.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.wilks.sided_interval",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ONE_SIDED_LOWER",
                    "ir_value_type": "short",
                }
            ]
        },
    )
    one_sided_upper: Literal[True] | None = DakotaField(
        default=None,
        description="Specifies one-sided upper portion order statistics to be used when determining sample sizes for random sampling using Wilks order statistics.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.wilks.sided_interval",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ONE_SIDED_UPPER",
                    "ir_value_type": "short",
                }
            ]
        },
    )
    two_sided: Literal[True] | None = DakotaField(
        default=None,
        description="Specifies two-sided order statistics (an interval) to be used when determining sample sizes for random sampling using Wilks order statistics.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.wilks.sided_interval",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TWO_SIDED",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class SamplingTolIntervals(DakotaBaseModel):
    """Generated model for SamplingTolIntervals"""

    coverage: DakotaFloat = DakotaField(
        default=0.95,
        ge=0,
        le=1,
        description="The coverage to be used for the calculation of the lower and upper ends of the interval covering the user supplied samples.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.ti_coverage",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    confidence_level: DakotaFloat = DakotaField(
        default=0.90,
        ge=0,
        le=1,
        description="The confidence level to be used to determine the standard deviation of the equivalent normal distribution.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.ti_confidence_level",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class LowDiscrepancyConfig(DakotaBaseModel):
    """Generated model for LowDiscrepancyConfig"""

    rank_1_lattice: Rank1Lattice | None = DakotaField(
        default=None,
        description="Uses rank-1 lattice points to sample variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.rank_1_lattice",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    digital_net: DigitalNet | None = DakotaField(
        default=None,
        description="Uses digital net points to sample variables",
        dakota={
            "aliases": ["sobol_sequence"],
            "materialization": [
                {
                    "ir_key": "method.digital_net",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ],
        },
    )


class LowDiscrepancy(DakotaBaseModel):
    """Generated model for LowDiscrepancy"""

    low_discrepancy: LowDiscrepancyConfig = DakotaField(
        description="Uses low-discrepancy points to sample variables",
        dakota={
            "aliases": ["qmc"],
            "materialization": [
                {
                    "ir_key": "method.sample_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_LOW_DISCREPANCY_SAMPLING",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class SamplingConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodSeedMixin,
    VbdSamplingMixin,
    DefaultFinalMomentsMixin,
    LevelMappingsMixin,
    RngOptionsContext2Mixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for SamplingConfig"""

    samples: int = DakotaField(
        default=0,
        description="Number of samples for sampling-based methods",
        dakota={
            "aliases": ["initial_samples"],
            "materialization": [
                {
                    "ir_key": "method.samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ],
        },
    )
    sample_type: (
        Union[
            SamplingSampleTypeLhs,
            SamplingSampleTypeRandom,
            IncrementalLhs,
            IncrementalRandom,
            LowDiscrepancy,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Selection of sampling strategy",
        dakota={"union_pattern": 2},
    )
    refinement_samples: list[int] | None = DakotaField(
        default=None,
        description="Performs an incremental Latin Hypercube Sampling (LHS) study",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.refinement_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )
    d_optimal: Union[CandidateDesigns, LejaOversampleRatio, dict] | None = DakotaField(
        default=None,
        description="Generate a D-optimal sampling design",
        dakota={
            "union_pattern": 5,
            "materialization": [
                {
                    "ir_key": "method.nond.d_optimal",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ],
        },
    )
    backfill: Literal[True] | None = DakotaField(
        default=None,
        description="Ensures that the samples of discrete variables with finite support are unique",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.backfill",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    principal_components: SamplingPrincipalComponents | None = DakotaField(
        default=None,
        description="Activates principal components analysis of the response matrix of N samples * L responses.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.principal_components",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    wilks: Wilks | None = DakotaField(
        default=None,
        description="Number of samples for random sampling using Wilks statistics",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.wilks",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    std_regression_coeffs: Literal[True] | None = DakotaField(
        default=None,
        description="Output Standardized Regression Coefficients and R^2 for samples",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.std_regression_coeffs",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    tolerance_intervals: SamplingTolIntervals | None = DakotaField(
        default=None,
        description="Computes the double sided tolerance interval equivalent normal distribuion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.tolerance_intervals",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class SamplingSelection(MethodSelection):
    """Generated model for SamplingSelection"""

    sampling: SamplingConfig = DakotaField(
        dakota={
            "aliases": ["nond_sampling"],
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RANDOM_SAMPLING",
                    "ir_value_type": "unsigned short",
                }
            ],
        }
    )
