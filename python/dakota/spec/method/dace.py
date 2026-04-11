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
    """Generated model for DaceGrid"""

    grid: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "stored_value": "SUBMETHOD_GRID",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class DaceRandom(DakotaBaseModel):
    """Generated model for DaceRandom"""

    random: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "stored_value": "SUBMETHOD_RANDOM",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Oas(DakotaBaseModel):
    """Generated model for Oas"""

    oas: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "stored_value": "SUBMETHOD_OAS",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class DaceLhs(DakotaBaseModel):
    """Generated model for DaceLhs"""

    lhs: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "stored_value": "SUBMETHOD_LHS",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class OaLhs(DakotaBaseModel):
    """Generated model for OaLhs"""

    oa_lhs: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "stored_value": "SUBMETHOD_OA_LHS",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BoxBehnken(DakotaBaseModel):
    """Generated model for BoxBehnken"""

    box_behnken: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "stored_value": "SUBMETHOD_BOX_BEHNKEN",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class CentralComposite(DakotaBaseModel):
    """Generated model for CentralComposite"""

    central_composite: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "stored_value": "SUBMETHOD_CENTRAL_COMPOSITE",
                    "storage_type": "PRESENCE_ENUM",
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
    """Generated model for DaceConfig"""

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
    """Generated model for DaceSelection"""

    dace: DaceConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "DACE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
