"""Generated Pydantic models for method.import_points"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField, DakotaFloat
from .base import MethodSelection
from typing import Literal, Union

# Cross-module model imports
from dakota.spec.shared.misc import (
    DefaultFinalMomentsMixin,
    LevelMappingsMixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
)


class ImportPointsFileCustomAnnotatedHeader(DakotaBaseModel):
    """Generated model for ImportPointsFileCustomAnnotatedHeader"""

    use_variable_labels: Literal[True] | None = DakotaField(
        default=None,
        description="Validate/use variable labels from tabular file header",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_points.use_variable_labels",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class ImportPointsFileAnnotatedConfig(DakotaBaseModel):
    """Generated model for ImportPointsFileAnnotatedConfig"""

    use_variable_labels: Literal[True] | None = DakotaField(
        default=None,
        description="Validate/use variable labels from tabular file header",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_points.use_variable_labels",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class ImportPointsImportPointsFileFreeform(DakotaBaseModel):
    """Generated model for ImportPointsImportPointsFileFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_points_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ImportPointsVarianceBasedDecomp(DakotaBaseModel):
    """Generated model for ImportPointsVarianceBasedDecomp"""

    drop_tolerance: DakotaFloat = DakotaField(
        default=-1.0,
        description="Suppresses output of sensitivity indices with values lower than this tolerance",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.vbd_drop_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    num_bins: int = DakotaField(
        default=-1,
        description="Number of bins used to compute the variance-based decomposition",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.vbd_via_sampling_num_bins",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )


class ImportPointsTolIntervals(DakotaBaseModel):
    """Generated model for ImportPointsTolIntervals"""

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


class ImportPointsImportPointsFileCustomAnnotatedConfig(DakotaBaseModel):
    """Generated model for ImportPointsImportPointsFileCustomAnnotatedConfig"""

    header: ImportPointsFileCustomAnnotatedHeader | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_points_format",
                    "stored_value": "TABULAR_HEADER",
                    "storage_type": "AUGMENT_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_points_format",
                    "stored_value": "TABULAR_EVAL_ID",
                    "storage_type": "AUGMENT_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_points_format",
                    "stored_value": "TABULAR_IFACE_ID",
                    "storage_type": "AUGMENT_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ImportPointsImportPointsFileAnnotated(DakotaBaseModel):
    """Generated model for ImportPointsImportPointsFileAnnotated"""

    annotated: ImportPointsFileAnnotatedConfig = DakotaField(
        default_factory=ImportPointsFileAnnotatedConfig,
        description="Selects annotated tabular file format",
        dakota={
            "model_default": "ImportPointsFileAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_points_format",
                    "stored_value": "TABULAR_ANNOTATED",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class ImportPointsImportPointsFileCustomAnnotated(DakotaBaseModel):
    """Generated model for ImportPointsImportPointsFileCustomAnnotated"""

    custom_annotated: ImportPointsImportPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=ImportPointsImportPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "ImportPointsImportPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_points_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class ImportPointsImportPointsFile(DakotaBaseModel):
    """Generated model for ImportPointsImportPointsFile"""

    filename: str = DakotaField(
        description="File containing list of variable and response values to import for analysis.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        ImportPointsImportPointsFileCustomAnnotated,
        ImportPointsImportPointsFileAnnotated,
        ImportPointsImportPointsFileFreeform,
    ] = DakotaField(
        default_factory=ImportPointsImportPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "ImportPointsImportPointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_points.active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class ImportPointsConfig(
    MethodThreeOptionalKeywordsMixin,
    DefaultFinalMomentsMixin,
    LevelMappingsMixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for ImportPointsConfig"""

    import_points_file: ImportPointsImportPointsFile = DakotaField(
        default=...,
        description="File containing list of variable and response values to import for analysis.",
        dakota={"argument": "filename"},
    )
    variance_based_decomp: ImportPointsVarianceBasedDecomp | None = DakotaField(
        default=None,
        description="Computes Sobol' main effects using a binned approach",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.variance_based_decomp",
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
    tolerance_intervals: ImportPointsTolIntervals | None = DakotaField(
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


class ImportPointsSelection(MethodSelection):
    """Generated model for ImportPointsSelection"""

    import_points: ImportPointsConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "IMPORT_POINTS",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
