"""Generated Pydantic models for shared.surrogate"""

from __future__ import annotations

from ...base import DakotaBaseModel, DakotaField
from typing import Literal, Union


class MethodExportApproxFormatCustomAnnotatedConfig(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
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
                    "ir_key": "method.export_approx_format",
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
                    "ir_key": "method.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodExportApproxFormatAnnotated(DakotaBaseModel):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodExportApproxFormatFreeform(DakotaBaseModel):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodExportApproxPointsFileCustomAnnotatedConfig(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
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
                    "ir_key": "method.export_approx_format",
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
                    "ir_key": "method.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodExportApproxPointsFileAnnotated(DakotaBaseModel):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodExportApproxPointsFileFreeform(DakotaBaseModel):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodImportApproxFormatCustomAnnotatedConfig(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_approx_format",
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
                    "ir_key": "method.import_approx_format",
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
                    "ir_key": "method.import_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodImportApproxFormatAnnotated(DakotaBaseModel):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodImportApproxFormatFreeform(DakotaBaseModel):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ModelPartialSurrogateExportFormatFormats(DakotaBaseModel):
    "Formats for surrogate model export"

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


class TextArchive(DakotaBaseModel):
    "Surrogate model plain-text archive file format"

    text_archive: Literal[True] = DakotaField(
        default=True,
        description="Surrogate model plain-text archive file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.model_import_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TEXT_ARCHIVE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BinaryArchive(DakotaBaseModel):
    "Surrogate model binary archive file format"

    binary_archive: Literal[True] = DakotaField(
        default=True,
        description="Surrogate model binary archive file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.model_import_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "BINARY_ARCHIVE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ZerothOrder(DakotaBaseModel):
    "Specify that truth values must be matched."

    zeroth_order: Literal[True] = DakotaField(
        default=True,
        description="Specify that truth values must be matched.",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.correction_order",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": 0,
                    "ir_value_type": "short",
                }
            ]
        },
    )


class CorrectionFirstOrder(DakotaBaseModel):
    "Specify that truth values and gradients must be matched."

    first_order: Literal[True] = DakotaField(
        default=True,
        description="Specify that truth values and gradients must be matched.",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.correction_order",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": 1,
                    "ir_value_type": "short",
                }
            ]
        },
    )


class CorrectionSecondOrder(DakotaBaseModel):
    "Specify that truth values, gradients and Hessians must be matched."

    second_order: Literal[True] = DakotaField(
        default=True,
        description="Specify that truth values, gradients and Hessians must be matched.",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.correction_order",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": 2,
                    "ir_value_type": "short",
                }
            ]
        },
    )


class Additive(DakotaBaseModel):
    "Additive correction factor for local surrogate accuracy"

    additive: Literal[True] = DakotaField(
        default=True,
        description="Additive correction factor for local surrogate accuracy",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.correction_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ADDITIVE_CORRECTION",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class Multiplicative(DakotaBaseModel):
    "Multiplicative correction factor for local surrogate accuracy."

    multiplicative: Literal[True] = DakotaField(
        default=True,
        description="Multiplicative correction factor for local surrogate accuracy.",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.correction_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MULTIPLICATIVE_CORRECTION",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class CorrectionCombined(DakotaBaseModel):
    "Multipoint correction for a hierarchical surrogate"

    combined: Literal[True] = DakotaField(
        default=True,
        description="Multipoint correction for a hierarchical surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.correction_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "COMBINED_CORRECTION",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodExportApproxFormatCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: MethodExportApproxFormatCustomAnnotatedConfig = DakotaField(
        default_factory=MethodExportApproxFormatCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "MethodExportApproxFormatCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class MethodExportApproxPointsFileCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: MethodExportApproxPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=MethodExportApproxPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "MethodExportApproxPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class MethodImportApproxFormatCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: MethodImportApproxFormatCustomAnnotatedConfig = DakotaField(
        default_factory=MethodImportApproxFormatCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "MethodImportApproxFormatCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class ModelPartialSurrogateExportFormatExportModel(DakotaBaseModel):
    "Exports surrogate model in user-specified format(s)"

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
    formats: ModelPartialSurrogateExportFormatFormats = DakotaField(
        description="Formats for surrogate model export"
    )


class ImportModel(DakotaBaseModel):
    "Import surrogate model from archive file"

    filename_prefix: str = DakotaField(
        default="exported_surrogate",
        description="User-customizable portion of exported/imported surrogate model filenames",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.model_import_prefix",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    import_format: Union[TextArchive, BinaryArchive] = DakotaField(
        description="Surrogate Import Format",
        dakota={"anchor": True, "union_pattern": 4},
    )


class Correction(DakotaBaseModel):
    "Correction approaches for surrogate models"

    correction_order: Union[
        ZerothOrder, CorrectionFirstOrder, CorrectionSecondOrder
    ] = DakotaField(
        description="Correction Order", dakota={"anchor": True, "union_pattern": 4}
    )
    correction_type: Union[Additive, Multiplicative, CorrectionCombined] = DakotaField(
        description="Correction Type", dakota={"anchor": True, "union_pattern": 4}
    )


class MethodExportApproxFormatMixin(DakotaBaseModel):
    "Generated model for MethodExportApproxFormatMixin"

    format: Union[
        MethodExportApproxFormatCustomAnnotated,
        MethodExportApproxFormatAnnotated,
        MethodExportApproxFormatFreeform,
    ] = DakotaField(
        default_factory=MethodExportApproxFormatAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "MethodExportApproxFormatAnnotated",
        },
    )


class MethodExportApproxPointsFileExportApproxPointsFile(DakotaBaseModel):
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
    format: Union[
        MethodExportApproxPointsFileCustomAnnotated,
        MethodExportApproxPointsFileAnnotated,
        MethodExportApproxPointsFileFreeform,
    ] = DakotaField(
        default_factory=MethodExportApproxPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "MethodExportApproxPointsFileAnnotated",
        },
    )


class MethodImportApproxFormatMixin(DakotaBaseModel):
    "Generated model for MethodImportApproxFormatMixin"

    format: Union[
        MethodImportApproxFormatCustomAnnotated,
        MethodImportApproxFormatAnnotated,
        MethodImportApproxFormatFreeform,
    ] = DakotaField(
        default_factory=MethodImportApproxFormatAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "MethodImportApproxFormatAnnotated",
        },
    )


class ModelPartialSurrogateExportFormatMixin(DakotaBaseModel):
    "Generated model for ModelPartialSurrogateExportFormatMixin"

    export_model: ModelPartialSurrogateExportFormatExportModel | None = DakotaField(
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


class ModelSurrogateImportMixin(DakotaBaseModel):
    "Generated model for ModelSurrogateImportMixin"

    import_model: ImportModel | None = DakotaField(
        default=None,
        description="Import surrogate model from archive file",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.import_surrogate",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class SurrogateCorrectionMixin(DakotaBaseModel):
    "Generated model for SurrogateCorrectionMixin"

    correction: Correction | None = DakotaField(
        default=None, description="Correction approaches for surrogate models"
    )


class MethodExportApproxPointsFileMixin(DakotaBaseModel):
    "Generated model for MethodExportApproxPointsFileMixin"

    export_approx_points_file: (
        MethodExportApproxPointsFileExportApproxPointsFile | None
    ) = DakotaField(
        default=None,
        description="Output file for surrogate model value evaluations",
        dakota={"argument": "filename", "aliases": ["export_points_file"]},
    )
