"""Generated Pydantic models for method.efficient_global"""

from __future__ import annotations

from ..base import DakotaField, DakotaFloat
from .base import MethodSelection
from typing import Literal
from ..base import DBL_MAX

# Cross-module model imports
from dakota.spec.shared.formats import MethodImportBuildFormatMixin
from dakota.spec.shared.misc import (
    MethodGpAlternativesWithExportContext2Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
    MethodSynchronizationMixin,
    MethodThreeOptionalKeywordsMixin,
)
from dakota.spec.shared.surrogate import MethodExportApproxFormatMixin


class BatchSize(MethodSynchronizationMixin):
    """Generated model for BatchSize"""

    count: int = DakotaField(
        default=1,
        ge=1,
        description="Total batch size in parallel EGO",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.batch_size",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    exploration: int = DakotaField(
        default=0,
        ge=0,
        description="Portion of batch size dedicated to exploration in parallel EGO",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.batch_size.exploration",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )


class EfficientGlobalImportBuildPointsFile(MethodImportBuildFormatMixin):
    """Generated model for EfficientGlobalImportBuildPointsFile"""

    filename: str = DakotaField(
        description="File containing points you wish to use to build a surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class EfficientGlobalExportApproxPointsFile(MethodExportApproxFormatMixin):
    """Generated model for EfficientGlobalExportApproxPointsFile"""

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


class EfficientGlobalConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodMaxIterationsContext1Mixin,
    MethodGpAlternativesWithExportContext2Mixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for EfficientGlobalConfig"""

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
    batch_size: BatchSize | None = DakotaField(
        default=None,
        description="Total batch size in parallel EGO",
        dakota={"argument": "count"},
    )
    convergence_tolerance: DakotaFloat = DakotaField(
        default=-DBL_MAX,
        description="Expected improvement convergence tolerance",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.convergence_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
                {
                    "ir_key": "method.jega.percent_change",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
            ]
        },
    )
    x_conv_tol: DakotaFloat = DakotaField(
        default=-1.0,
        description="x-convergence tolerance",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.x_conv_tol",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
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
    import_build_points_file: EfficientGlobalImportBuildPointsFile | None = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )
    export_approx_points_file: EfficientGlobalExportApproxPointsFile | None = (
        DakotaField(
            default=None,
            description="Output file for surrogate model value evaluations",
            dakota={"argument": "filename", "aliases": ["export_points_file"]},
        )
    )


class EfficientGlobalSelection(MethodSelection):
    """Generated model for EfficientGlobalSelection"""

    efficient_global: EfficientGlobalConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "EFFICIENT_GLOBAL",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
