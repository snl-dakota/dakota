"""Generated Pydantic models for method.gpais"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.misc import (
    ImportBuildMixin,
    LevelMappingsNoRelContext1Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
    RngOptionsContext2Mixin,
)
from dakota.spec.shared.surrogate import MethodExportApproxFormatMixin


class GpaisExportApproxPointsFile(MethodExportApproxFormatMixin):
    "Output file for surrogate model value evaluations"

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


class GpaisConfig(
    MethodThreeOptionalKeywordsMixin,
    ImportBuildMixin,
    MethodMaxIterationsContext1Mixin,
    LevelMappingsNoRelContext1Mixin,
    RngOptionsContext2Mixin,
    MethodOptionalModelPointerMixin,
):
    "Gaussian Process Adaptive Importance Sampling"

    build_samples: int = DakotaField(
        default=0,
        description="Number of initial model evaluations used in build phase",
        dakota={
            "aliases": ["samples"],
            "materialization": [
                {
                    "ir_key": "method.build_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ],
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
    samples_on_emulator: int = DakotaField(
        default=0,
        description="Number of samples at which to evaluate an emulator (surrogate)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.samples_on_emulator",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    export_approx_points_file: GpaisExportApproxPointsFile | None = DakotaField(
        default=None,
        description="Output file for surrogate model value evaluations",
        dakota={"argument": "filename", "aliases": ["export_points_file"]},
    )


class GpaisSelection(MethodSelection):
    "Generated model for GpaisSelection"

    gpais: GpaisConfig = DakotaField(
        dakota={
            "aliases": ["gaussian_process_adaptive_importance_sampling"],
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "GPAIS",
                    "ir_value_type": "unsigned short",
                }
            ],
        }
    )
