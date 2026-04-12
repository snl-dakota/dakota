"""Generated Pydantic models for method.list_parameter_study"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField, DakotaFloat
from .base import MethodSelection
from typing import Literal, Union

# Cross-module model imports
from dakota.spec.shared.misc import (
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
)


class ListOfPoints(DakotaBaseModel):
    """Generated model for ListOfPoints"""

    list_of_points: list[DakotaFloat] = DakotaField(
        description="List of variable values to evaluate in a list parameter study",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.parameter_study.list_of_points",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class ListParameterStudyImportPointsFileCustomAnnotatedConfig(DakotaBaseModel):
    """Generated model for ListParameterStudyImportPointsFileCustomAnnotatedConfig"""

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.pstudy.import_format",
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
                    "ir_key": "method.pstudy.import_format",
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
                    "ir_key": "method.pstudy.import_format",
                    "stored_value": "TABULAR_IFACE_ID",
                    "storage_type": "AUGMENT_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ListParameterStudyImportPointsFileAnnotated(DakotaBaseModel):
    """Generated model for ListParameterStudyImportPointsFileAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.pstudy.import_format",
                    "stored_value": "TABULAR_ANNOTATED",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ListParameterStudyImportPointsFileFreeform(DakotaBaseModel):
    """Generated model for ListParameterStudyImportPointsFileFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.pstudy.import_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ListParameterStudyImportPointsFileCustomAnnotated(DakotaBaseModel):
    """Generated model for ListParameterStudyImportPointsFileCustomAnnotated"""

    custom_annotated: ListParameterStudyImportPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=ListParameterStudyImportPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "ListParameterStudyImportPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.pstudy.import_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class ImportPointsFileConfig(DakotaBaseModel):
    """Generated model for ImportPointsFileConfig"""

    filename: str = DakotaField(
        description="File containing list of variable values to evaluate in a list parameter study",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.pstudy.import_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        ListParameterStudyImportPointsFileCustomAnnotated,
        ListParameterStudyImportPointsFileAnnotated,
        ListParameterStudyImportPointsFileFreeform,
    ] = DakotaField(
        default_factory=ListParameterStudyImportPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "ListParameterStudyImportPointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.pstudy.import_active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class ListParameterStudyImportPointsFile(DakotaBaseModel):
    """Generated model for ListParameterStudyImportPointsFile"""

    import_points_file: ImportPointsFileConfig = DakotaField(
        default=...,
        description="File containing list of variable values to evaluate in a list parameter study",
        dakota={"argument": "filename"},
    )


class ListParameterStudyConfig(
    MethodThreeOptionalKeywordsMixin, MethodOptionalModelPointerMixin
):
    """Generated model for ListParameterStudyConfig"""

    source: Union[ListOfPoints, ListParameterStudyImportPointsFile] = DakotaField(
        description="Points Source", dakota={"anchor": True, "union_pattern": 4}
    )


class ListParameterStudySelection(MethodSelection):
    """Generated model for ListParameterStudySelection"""

    list_parameter_study: ListParameterStudyConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "LIST_PARAMETER_STUDY",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
