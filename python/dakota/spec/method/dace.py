"""Generated Pydantic models for method.dace"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField
from .base import MethodSelection
from typing import Literal, Union

# Cross-module model imports
from dakota.spec.shared.misc import (
    MethodOptionalModelPointerMixin,
    MethodSeedMixin,
    MethodThreeOptionalKeywordsMixin,
    VbdSamplingMixin,
)


class DaceGrid(DakotaBaseModel):
    "Grid Sampling"

    grid: Literal[True] = DakotaField(
        default=True,
        description="Grid Sampling",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_GRID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class DaceRandom(DakotaBaseModel):
    "Uses purely random Monte Carlo sampling to sample variables"

    random: Literal[True] = DakotaField(
        default=True,
        description="Uses purely random Monte Carlo sampling to sample variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_RANDOM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Oas(DakotaBaseModel):
    "Orthogonal Array Sampling"

    oas: Literal[True] = DakotaField(
        default=True,
        description="Orthogonal Array Sampling",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_OAS",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class DaceLhs(DakotaBaseModel):
    "Uses Latin Hypercube Sampling (LHS) to sample variables"

    lhs: Literal[True] = DakotaField(
        default=True,
        description="Uses Latin Hypercube Sampling (LHS) to sample variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_LHS",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class OaLhs(DakotaBaseModel):
    "Orthogonal Array Latin Hypercube Sampling"

    oa_lhs: Literal[True] = DakotaField(
        default=True,
        description="Orthogonal Array Latin Hypercube Sampling",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_OA_LHS",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BoxBehnken(DakotaBaseModel):
    "Box-Behnken Design"

    box_behnken: Literal[True] = DakotaField(
        default=True,
        description="Box-Behnken Design",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_BOX_BEHNKEN",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class CentralComposite(DakotaBaseModel):
    "Central Composite Design"

    central_composite: Literal[True] = DakotaField(
        default=True,
        description="Central Composite Design",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_CENTRAL_COMPOSITE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class DaceConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodSeedMixin,
    VbdSamplingMixin,
    MethodOptionalModelPointerMixin,
):
    "Design and Analysis of Computer Experiments"

    sub_method: Union[
        DaceGrid, DaceRandom, Oas, DaceLhs, OaLhs, BoxBehnken, CentralComposite
    ] = DakotaField(
        description="DACE type", dakota={"anchor": True, "union_pattern": 4}
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
    main_effects: Literal[True] | None = DakotaField(
        default=None,
        description="ANOVA",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.main_effects",
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
    symbols: int = DakotaField(
        default=0,
        description="Number of replications in the sample set",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.symbols",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )


class DaceSelection(MethodSelection):
    "Generated model for DaceSelection"

    dace: DaceConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DACE",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
