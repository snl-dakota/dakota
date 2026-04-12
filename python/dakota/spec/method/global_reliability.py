"""Generated Pydantic models for method.global_reliability"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField
from .base import MethodSelection
from typing import Literal, Union

# Cross-module model imports
from dakota.spec.shared.core import MethodConvergenceTolMixin
from dakota.spec.shared.misc import (
    ImportBuildMixin,
    LevelMappingsNoRelContext1Mixin,
    MethodExportModelContext1Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
    RngOptionsContext2Mixin,
)
from dakota.spec.shared.surrogate import MethodExportApproxFormatMixin


class XGP(DakotaBaseModel):
    """Generated model for XGP"""

    x_gaussian_process: Literal[True] = DakotaField(
        default=True,
        dakota={
            "aliases": ["x_kriging"],
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "stored_value": "SUBMETHOD_EGRA_X",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class UGP(DakotaBaseModel):
    """Generated model for UGP"""

    u_gaussian_process: Literal[True] = DakotaField(
        default=True,
        dakota={
            "aliases": ["u_kriging"],
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "stored_value": "SUBMETHOD_EGRA_U",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class GlobalReliabilitySurfpack(DakotaBaseModel):
    """Generated model for GlobalReliabilitySurfpack"""

    surfpack: MethodExportModelContext1Mixin = DakotaField(
        description="Use the Surfpack version of Gaussian Process surrogates",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "stored_value": "KRIGING_EMULATOR",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class GlobalReliabilityDakota(DakotaBaseModel):
    """Generated model for GlobalReliabilityDakota"""

    dakota: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "stored_value": "GP_EMULATOR",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class GlobalReliabilityExperimentalConfig(MethodExportModelContext1Mixin):
    """Generated model for GlobalReliabilityExperimentalConfig"""

    options_file: str | None = DakotaField(
        default=None,
        description="Filename for a YAML file that specifies Gaussian process options",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.advanced_options_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class GlobalReliabilityExportApproxPointsFile(MethodExportApproxFormatMixin):
    """Generated model for GlobalReliabilityExportApproxPointsFile"""

    filename: str = DakotaField(
        description="Output file for surrogate model value evaluations",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class GlobalReliabilityExperimental(DakotaBaseModel):
    """Generated model for GlobalReliabilityExperimental"""

    experimental: GlobalReliabilityExperimentalConfig = DakotaField(
        description="Use the experimental Gaussian Process surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "stored_value": "EXPGP_EMULATOR",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class GlobalReliabilityConfig(
    MethodThreeOptionalKeywordsMixin,
    ImportBuildMixin,
    RngOptionsContext2Mixin,
    LevelMappingsNoRelContext1Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodConvergenceTolMixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for GlobalReliabilityConfig"""

    initial_samples: int = DakotaField(
        default=0,
        description="Initial number of samples for sampling-based methods",
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
    space: Union[XGP, UGP] = DakotaField(
        description="Approximation", dakota={"anchor": True, "union_pattern": 4}
    )
    gp_implementation: (
        Union[
            GlobalReliabilitySurfpack,
            GlobalReliabilityDakota,
            GlobalReliabilityExperimental,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="GP Implementation",
        dakota={"anchor": True, "union_pattern": 2},
    )
    export_approx_points_file: GlobalReliabilityExportApproxPointsFile | None = (
        DakotaField(
            default=None,
            description="Output file for surrogate model value evaluations",
            dakota={"argument": "filename", "aliases": ["export_points_file"]},
        )
    )
    use_derivatives: Literal[True] | None = DakotaField(
        default=None,
        description="Use derivative data to construct surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.derivative_usage",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
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


class GlobalReliabilitySelection(MethodSelection):
    """Generated model for GlobalReliabilitySelection"""

    global_reliability: GlobalReliabilityConfig = DakotaField(
        dakota={
            "aliases": ["nond_global_reliability"],
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "GLOBAL_RELIABILITY",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        }
    )
