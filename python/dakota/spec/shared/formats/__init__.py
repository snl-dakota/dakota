"""Generated Pydantic models for shared.formats"""

from __future__ import annotations

from ...base import DakotaBaseModel, DakotaField
from typing import Literal, Union


class MethodImportBuildFormatCustomAnnotatedConfig(DakotaBaseModel):
    """Generated model for MethodImportBuildFormatCustomAnnotatedConfig"""

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
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
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
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
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodImportBuildFormatAnnotated(DakotaBaseModel):
    """Generated model for MethodImportBuildFormatAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodImportBuildFormatFreeform(DakotaBaseModel):
    """Generated model for MethodImportBuildFormatFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ModelFullSurfpackExportFormatFormats(DakotaBaseModel):
    """Generated model for ModelFullSurfpackExportFormatFormats"""

    text_archive: Literal[True] | None = DakotaField(
        default=None,
        description="Surrogate model plain-text archive file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.model_export_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TEXT_ARCHIVE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    binary_archive: Literal[True] | None = DakotaField(
        default=None,
        description="Surrogate model binary archive file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.model_export_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "BINARY_ARCHIVE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    algebraic_file: Literal[True] | None = DakotaField(
        default=None,
        description="Export surrogate model in algebraic format to a file",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.model_export_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "ALGEBRAIC_FILE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    algebraic_console: Literal[True] | None = DakotaField(
        default=None,
        description="Export surrogate model in algebraic format to the console",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.model_export_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "ALGEBRAIC_CONSOLE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodImportBuildFormatCustomAnnotated(DakotaBaseModel):
    """Generated model for MethodImportBuildFormatCustomAnnotated"""

    custom_annotated: MethodImportBuildFormatCustomAnnotatedConfig = DakotaField(
        default_factory=MethodImportBuildFormatCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "MethodImportBuildFormatCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class ModelFullSurfpackExportFormatExportModel(DakotaBaseModel):
    """Generated model for ModelFullSurfpackExportFormatExportModel"""

    filename_prefix: str = DakotaField(
        default="exported_surrogate",
        description="User-customizable portion of exported/imported surrogate model filenames",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.model_export_prefix",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    formats: ModelFullSurfpackExportFormatFormats = DakotaField(
        description="Formats for surrogate model export"
    )


class MethodImportBuildFormatMixin(DakotaBaseModel):
    """Generated model for MethodImportBuildFormatMixin"""

    format: Union[
        MethodImportBuildFormatCustomAnnotated,
        MethodImportBuildFormatAnnotated,
        MethodImportBuildFormatFreeform,
    ] = DakotaField(
        default_factory=MethodImportBuildFormatAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "MethodImportBuildFormatAnnotated",
        },
    )


class ModelFullSurfpackExportFormatMixin(DakotaBaseModel):
    """Generated model for ModelFullSurfpackExportFormatMixin"""

    export_model: ModelFullSurfpackExportFormatExportModel | None = DakotaField(
        default=None,
        description="Exports surrogate model in user-specified format(s)",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.export_surrogate",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
