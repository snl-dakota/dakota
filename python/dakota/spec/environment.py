"""Generated Pydantic models for environment"""

from __future__ import annotations

from .base import DakotaBaseModel, DakotaField
from typing import Literal, Union


class TabularDataCustomAnnotatedConfig(DakotaBaseModel):
    """Generated model for TabularDataCustomAnnotatedConfig"""

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.tabular_format",
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
                    "ir_key": "environment.tabular_format",
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
                    "ir_key": "environment.tabular_format",
                    "stored_value": "TABULAR_IFACE_ID",
                    "storage_type": "AUGMENT_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class TabularDataAnnotated(DakotaBaseModel):
    """Generated model for TabularDataAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.tabular_format",
                    "stored_value": "TABULAR_ANNOTATED",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class TabularDataFreeform(DakotaBaseModel):
    """Generated model for TabularDataFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.tabular_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ReadRestart(DakotaBaseModel):
    """Generated model for ReadRestart"""

    filename: str = DakotaField(
        description="Base filename for restart file read",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.read_restart",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    stop_restart: int | None = DakotaField(
        default=None,
        ge=0,
        description="Restart record number at which to stop reading the restart file.",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.stop_restart",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )


class TopMethod(DakotaBaseModel):
    """Generated model for TopMethod"""

    top_method: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.model_evals_selection",
                    "stored_value": "MODEL_EVAL_STORE_TOP_METHOD",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ModelSelectionNone(DakotaBaseModel):
    """Generated model for ModelSelectionNone"""

    none: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.model_evals_selection",
                    "stored_value": "MODEL_EVAL_STORE_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class AllMethods(DakotaBaseModel):
    """Generated model for AllMethods"""

    all_methods: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.model_evals_selection",
                    "stored_value": "MODEL_EVAL_STORE_ALL_METHODS",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ModelSelectionAll(DakotaBaseModel):
    """Generated model for ModelSelectionAll"""

    all: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.model_evals_selection",
                    "stored_value": "MODEL_EVAL_STORE_ALL",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class InterfaceSelectionNone(DakotaBaseModel):
    """Generated model for InterfaceSelectionNone"""

    none: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.interface_evals_selection",
                    "stored_value": "INTERF_EVAL_STORE_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Simulation(DakotaBaseModel):
    """Generated model for Simulation"""

    simulation: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.interface_evals_selection",
                    "stored_value": "INTERF_EVAL_STORE_SIMULATION",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class InterfaceSelectionAll(DakotaBaseModel):
    """Generated model for InterfaceSelectionAll"""

    all: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.interface_evals_selection",
                    "stored_value": "INTERF_EVAL_STORE_ALL",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class OutputCustomAnnotatedConfig(DakotaBaseModel):
    """Generated model for OutputCustomAnnotatedConfig"""

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.pre_run_output_format",
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
                    "ir_key": "environment.pre_run_output_format",
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
                    "ir_key": "environment.pre_run_output_format",
                    "stored_value": "TABULAR_IFACE_ID",
                    "storage_type": "AUGMENT_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class OutputAnnotated(DakotaBaseModel):
    """Generated model for OutputAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.pre_run_output_format",
                    "stored_value": "TABULAR_ANNOTATED",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class OutputFreeform(DakotaBaseModel):
    """Generated model for OutputFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.pre_run_output_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Run(DakotaBaseModel):
    """Generated model for Run"""

    input: str | None = DakotaField(
        default=None,
        description="Base filename for run mode data input",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.run_input",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    output: str | None = DakotaField(
        default=None,
        description="Base filename for run mode data output",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.run_output",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class InputCustomAnnotatedConfig(DakotaBaseModel):
    """Generated model for InputCustomAnnotatedConfig"""

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.post_run_input_format",
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
                    "ir_key": "environment.post_run_input_format",
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
                    "ir_key": "environment.post_run_input_format",
                    "stored_value": "TABULAR_IFACE_ID",
                    "storage_type": "AUGMENT_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class InputAnnotated(DakotaBaseModel):
    """Generated model for InputAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.post_run_input_format",
                    "stored_value": "TABULAR_ANNOTATED",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class InputFreeform(DakotaBaseModel):
    """Generated model for InputFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.post_run_input_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class TabularDataCustomAnnotated(DakotaBaseModel):
    """Generated model for TabularDataCustomAnnotated"""

    custom_annotated: TabularDataCustomAnnotatedConfig = DakotaField(
        default_factory=TabularDataCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "TabularDataCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "environment.tabular_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class Hdf5(DakotaBaseModel):
    """Generated model for Hdf5"""

    model_selection: Union[
        TopMethod, ModelSelectionNone, AllMethods, ModelSelectionAll
    ] = DakotaField(
        default_factory=TopMethod,
        description="Select the models that write evaluation data to HDF5",
        dakota={"union_pattern": 1, "model_default": "TopMethod"},
    )
    interface_selection: Union[
        InterfaceSelectionNone, Simulation, InterfaceSelectionAll
    ] = DakotaField(
        default_factory=Simulation,
        description="Select the models that write evaluation data to HDF5",
        dakota={"union_pattern": 1, "model_default": "Simulation"},
    )


class OutputCustomAnnotated(DakotaBaseModel):
    """Generated model for OutputCustomAnnotated"""

    custom_annotated: OutputCustomAnnotatedConfig = DakotaField(
        default_factory=OutputCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "OutputCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "environment.pre_run_output_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class InputCustomAnnotated(DakotaBaseModel):
    """Generated model for InputCustomAnnotated"""

    custom_annotated: InputCustomAnnotatedConfig = DakotaField(
        default_factory=InputCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "InputCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "environment.post_run_input_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class TabularData(DakotaBaseModel):
    """Generated model for TabularData"""

    tabular_data_file: str = DakotaField(
        default="dakota_tabular.dat",
        description="File name for tabular data output",
        dakota={
            "aliases": ["tabular_graphics_file"],
            "materialization": [
                {
                    "ir_key": "environment.tabular_graphics_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    format: Union[
        TabularDataCustomAnnotated, TabularDataAnnotated, TabularDataFreeform
    ] = DakotaField(
        default_factory=TabularDataAnnotated,
        description="Tabular Data Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "TabularDataAnnotated",
        },
    )


class ResultsOutput(DakotaBaseModel):
    """Generated model for ResultsOutput"""

    results_output_file: str = DakotaField(
        default="dakota_results",
        description="The base file name of the results file",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.results_output_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    text: Literal[True] | None = DakotaField(
        default=None,
        description="Write results to file in text format",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.results_output_format",
                    "stored_value": "RESULTS_OUTPUT_TEXT",
                    "storage_type": "AUGMENT_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    hdf5: Hdf5 | None = DakotaField(
        default=None,
        description="Write results to file in HDF5 format",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.results_output_format",
                    "stored_value": "RESULTS_OUTPUT_HDF5",
                    "storage_type": "AUGMENT_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Output(DakotaBaseModel):
    """Generated model for Output"""

    filename: str = DakotaField(
        description="Base filename for pre-run mode data output",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.pre_run_output",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[OutputCustomAnnotated, OutputAnnotated, OutputFreeform] = DakotaField(
        default_factory=OutputAnnotated,
        description="Tabular Format",
        dakota={"anchor": True, "union_pattern": 1, "model_default": "OutputAnnotated"},
    )


class Input(DakotaBaseModel):
    """Generated model for Input"""

    filename: str = DakotaField(
        description="Base filename for post-run mode data input",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.post_run_input",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[InputCustomAnnotated, InputAnnotated, InputFreeform] = DakotaField(
        default_factory=InputAnnotated,
        description="Tabular Format",
        dakota={"anchor": True, "union_pattern": 1, "model_default": "InputAnnotated"},
    )


class PreRun(DakotaBaseModel):
    """Generated model for PreRun"""

    input: str | None = DakotaField(
        default=None,
        description="Base filename for pre-run mode data input",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.pre_run_input",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    output: Output | None = DakotaField(
        default=None,
        description="Base filename for pre-run mode data output",
        dakota={"argument": "filename"},
    )


class PostRun(DakotaBaseModel):
    """Generated model for PostRun"""

    input: Input | None = DakotaField(
        default=None,
        description="Base filename for post-run mode data input",
        dakota={"argument": "filename"},
    )
    output: str | None = DakotaField(
        default=None,
        description="Base filename for post-run mode data output",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.post_run_output",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class EnvironmentConfig(DakotaBaseModel):
    """Generated model for EnvironmentConfig"""

    tabular_data: TabularData | None = DakotaField(
        default=None,
        description="Write a tabular results file with variable and response history",
        dakota={
            "aliases": ["tabular_graphics_data"],
            "materialization": [
                {
                    "ir_key": "environment.tabular_graphics_data",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ],
        },
    )
    output_file: str | None = DakotaField(
        default=None,
        description="Base filename for output redirection",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.output_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    error_file: str | None = DakotaField(
        default=None,
        description="Base filename for error redirection",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.error_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    read_restart: ReadRestart | None = DakotaField(
        default=None,
        description="Base filename for restart file read",
        dakota={"argument": "filename"},
    )
    write_restart: str | None = DakotaField(
        default=None,
        description="Base filename for restart file write",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.write_restart",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    output_precision: int = DakotaField(
        default=0,
        ge=0,
        description="Control the output precision",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.output_precision",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    results_output: ResultsOutput | None = DakotaField(
        default=None,
        description="(Experimental) Write a summary file containing the final results",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.results_output",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    graphics: Literal[True] | None = DakotaField(
        default=None,
        description="(DEPRECATED) Display plots of variables and responses",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.graphics",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    check: Literal[True] | None = DakotaField(
        default=None,
        description="Invoke Dakota in input check mode",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.check",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    pre_run: PreRun | None = DakotaField(
        default=None,
        description="Invoke Dakota with pre-run mode active",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.pre_run",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    run: Run | None = DakotaField(
        default=None,
        description="Invoke Dakota with run mode active",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.run",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    post_run: PostRun | None = DakotaField(
        default=None,
        description="Invoke Dakota with post-run mode active",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.post_run",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    top_method_pointer: str | None = DakotaField(
        default=None,
        description="Identify which method leads the Dakota study",
        dakota={
            "block_pointer": "method",
            "aliases": ["method_pointer"],
            "materialization": [
                {
                    "ir_key": "environment.top_method_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )
