"""Generated Pydantic models for method.local_reliability"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField
from .base import MethodSelection
from typing import Literal, Union

# Cross-module model imports
from dakota.spec.shared.core import MethodConvergenceTolMixin
from dakota.spec.shared.misc import (
    DefaultFinalMomentsMixin,
    LevelMappingsMixin,
    MethodGradientSubProblemSolverMixin,
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
)


class XTaylorMean(DakotaBaseModel):
    'Form Taylor series approximation in \\"x-space\\" at variable means'

    x_taylor_mean: Literal[True] = DakotaField(
        default=True,
        description='Form Taylor series approximation in "x-space" at variable means',
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_AMV_X",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class UTaylorMean(DakotaBaseModel):
    'Form Taylor series approximation in \\"u-space\\" at variable means'

    u_taylor_mean: Literal[True] = DakotaField(
        default=True,
        description='Form Taylor series approximation in "u-space" at variable means',
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_AMV_U",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class XTaylorMpp(DakotaBaseModel):
    "X-space Taylor series approximation with iterative updates"

    x_taylor_mpp: Literal[True] = DakotaField(
        default=True,
        description="X-space Taylor series approximation with iterative updates",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_AMV_PLUS_X",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class UTaylorMpp(DakotaBaseModel):
    "U-space Taylor series approximation with iterative updates"

    u_taylor_mpp: Literal[True] = DakotaField(
        default=True,
        description="U-space Taylor series approximation with iterative updates",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_AMV_PLUS_U",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class XTwoPoint(DakotaBaseModel):
    'Predict MPP using Two-point Adaptive Nonlinear Approximation in \\"x-space\\"'

    x_two_point: Literal[True] = DakotaField(
        default=True,
        description='Predict MPP using Two-point Adaptive Nonlinear Approximation in "x-space"',
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_TANA_X",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class UTwoPoint(DakotaBaseModel):
    'Predict MPP using Two-point Adaptive Nonlinear Approximation in \\"u-space\\"'

    u_two_point: Literal[True] = DakotaField(
        default=True,
        description='Predict MPP using Two-point Adaptive Nonlinear Approximation in "u-space"',
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_TANA_U",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class XMultiPoint(DakotaBaseModel):
    "MPP search for local reliability based on QMEA multi-point approximation in u-space"

    x_multi_point: Literal[True] = DakotaField(
        default=True,
        description="MPP search for local reliability based on QMEA multi-point approximation in u-space",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_QMEA_X",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class UMultiPoint(DakotaBaseModel):
    "MPP search for local reliability based on QMEA multi-point approximation in x-space"

    u_multi_point: Literal[True] = DakotaField(
        default=True,
        description="MPP search for local reliability based on QMEA multi-point approximation in x-space",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_QMEA_U",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class NoApprox(DakotaBaseModel):
    "Perform MPP search on original response functions (use no approximation)"

    no_approx: Literal[True] = DakotaField(
        default=True,
        description="Perform MPP search on original response functions (use no approximation)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_NO_APPROX",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class IntegrationFirstOrder(DakotaBaseModel):
    "First-order integration scheme"

    first_order: Literal[True] = DakotaField(
        default=True,
        description="First-order integration scheme",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.reliability_integration",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "first_order",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class IntegrationSecondOrder(DakotaBaseModel):
    "Second-order integration scheme"

    second_order: Literal[True] = DakotaField(
        default=True,
        description="Second-order integration scheme",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.reliability_integration",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "second_order",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class IntegrationProbabilityRefinementImportance(DakotaBaseModel):
    "Importance sampling option for probability refinement"

    importance: Literal[True] = DakotaField(
        default=True,
        description="Importance sampling option for probability refinement",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.integration_refinement",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "IS",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class IntegrationProbabilityRefinementAdaptImport(DakotaBaseModel):
    "Importance sampling option for probability refinement"

    adapt_import: Literal[True] = DakotaField(
        default=True,
        description="Importance sampling option for probability refinement",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.integration_refinement",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "AIS",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class IntegrationProbabilityRefinementMmAdaptImport(DakotaBaseModel):
    "Importance sampling option for probability refinement"

    mm_adapt_import: Literal[True] = DakotaField(
        default=True,
        description="Importance sampling option for probability refinement",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.integration_refinement",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MMAIS",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class IntegrationProbabilityRefinement(DakotaBaseModel):
    "Allow refinement of probability and generalized reliability results using importance sampling"

    approach: Union[
        IntegrationProbabilityRefinementImportance,
        IntegrationProbabilityRefinementAdaptImport,
        IntegrationProbabilityRefinementMmAdaptImport,
    ] = DakotaField(
        description="Importance Sampling Approach",
        dakota={"anchor": True, "union_pattern": 4},
    )
    refinement_samples: list[int] | None = DakotaField(
        default=None,
        description="Number of samples used to refine a probability estimate or sampling design.",
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
    seed: int | None = DakotaField(
        default=None,
        gt=0,
        description="Seed of the random number generator",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.random_seed",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )


class Integration(DakotaBaseModel):
    "Integration approach"

    order: Union[IntegrationFirstOrder, IntegrationSecondOrder] = DakotaField(
        description="Integration Order", dakota={"anchor": True, "union_pattern": 4}
    )
    probability_refinement: IntegrationProbabilityRefinement | None = DakotaField(
        default=None,
        description="Allow refinement of probability and generalized reliability results using importance sampling",
        dakota={"aliases": ["sample_refinement"]},
    )


class MppSearch(MethodGradientSubProblemSolverMixin):
    "Specify which MPP search option to use"

    sub_method: Union[
        XTaylorMean,
        UTaylorMean,
        XTaylorMpp,
        UTaylorMpp,
        XTwoPoint,
        UTwoPoint,
        XMultiPoint,
        UMultiPoint,
        NoApprox,
    ] = DakotaField(
        description="MPP Approximation", dakota={"anchor": True, "union_pattern": 4}
    )
    integration: Integration | None = DakotaField(
        default=None, description="Integration approach"
    )


class LocalReliabilityConfig(
    MethodThreeOptionalKeywordsMixin,
    LevelMappingsMixin,
    MethodMaxIterationsContext1Mixin,
    MethodConvergenceTolMixin,
    DefaultFinalMomentsMixin,
    MethodOptionalModelPointerMixin,
):
    "Local reliability method"

    mpp_search: MppSearch | None = DakotaField(
        default=None, description="Specify which MPP search option to use"
    )


class LocalReliabilitySelection(MethodSelection):
    "Generated model for LocalReliabilitySelection"

    local_reliability: LocalReliabilityConfig = DakotaField(
        dakota={
            "aliases": ["nond_local_reliability"],
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LOCAL_RELIABILITY",
                    "ir_value_type": "unsigned short",
                }
            ],
        }
    )
