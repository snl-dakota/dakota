"""Generated Pydantic models for environment"""

from __future__ import annotations

from .base import DakotaBaseModel, DakotaField
from typing import Literal, Union


class TabularDataCustomAnnotatedConfig(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.tabular_format",
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
                    "ir_key": "environment.tabular_format",
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
                    "ir_key": "environment.tabular_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class TabularDataAnnotated(DakotaBaseModel):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.tabular_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class TabularDataFreeform(DakotaBaseModel):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.tabular_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ReadRestart(DakotaBaseModel):
    "Base filename for restart file read"

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
    "Write evaluation data only for the top-level method's model to HDF5"

    top_method: Literal[True] = DakotaField(
        default=True,
        description="Write evaluation data only for the top-level method's model to HDF5",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.model_evals_selection",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MODEL_EVAL_STORE_TOP_METHOD",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ModelSelectionNone(DakotaBaseModel):
    "Write evaluation data for no models to HDF5"

    none: Literal[True] = DakotaField(
        default=True,
        description="Write evaluation data for no models to HDF5",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.model_evals_selection",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MODEL_EVAL_STORE_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class AllMethods(DakotaBaseModel):
    "Write evaluation data to HDF5 for all models that belong directly to methods"

    all_methods: Literal[True] = DakotaField(
        default=True,
        description="Write evaluation data to HDF5 for all models that belong directly to methods",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.model_evals_selection",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MODEL_EVAL_STORE_ALL_METHODS",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ModelSelectionAll(DakotaBaseModel):
    "Write evaluation data to HDF5 for all models"

    all: Literal[True] = DakotaField(
        default=True,
        description="Write evaluation data to HDF5 for all models",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.model_evals_selection",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MODEL_EVAL_STORE_ALL",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class InterfaceSelectionNone(DakotaBaseModel):
    "Write evaluation data for no interfaces to HDF5"

    none: Literal[True] = DakotaField(
        default=True,
        description="Write evaluation data for no interfaces to HDF5",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.interface_evals_selection",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "INTERF_EVAL_STORE_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Simulation(DakotaBaseModel):
    "Write evaluation data only for simulation interfaces to HDF5"

    simulation: Literal[True] = DakotaField(
        default=True,
        description="Write evaluation data only for simulation interfaces to HDF5",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.interface_evals_selection",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "INTERF_EVAL_STORE_SIMULATION",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class InterfaceSelectionAll(DakotaBaseModel):
    "Write evaluation data for all interfaces to HDF5"

    all: Literal[True] = DakotaField(
        default=True,
        description="Write evaluation data for all interfaces to HDF5",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.interface_evals_selection",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "INTERF_EVAL_STORE_ALL",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class OutputCustomAnnotatedConfig(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.pre_run_output_format",
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
                    "ir_key": "environment.pre_run_output_format",
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
                    "ir_key": "environment.pre_run_output_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class OutputAnnotated(DakotaBaseModel):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.pre_run_output_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class OutputFreeform(DakotaBaseModel):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.pre_run_output_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Run(DakotaBaseModel):
    "Invoke Dakota with run mode active"

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
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.post_run_input_format",
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
                    "ir_key": "environment.post_run_input_format",
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
                    "ir_key": "environment.post_run_input_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class InputAnnotated(DakotaBaseModel):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.post_run_input_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class InputFreeform(DakotaBaseModel):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "environment.post_run_input_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class TabularDataCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: TabularDataCustomAnnotatedConfig = DakotaField(
        default_factory=TabularDataCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "TabularDataCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "environment.tabular_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class Hdf5(DakotaBaseModel):
    "Write results to file in HDF5 format"

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
    "Selects custom-annotated tabular file format"

    custom_annotated: OutputCustomAnnotatedConfig = DakotaField(
        default_factory=OutputCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "OutputCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "environment.pre_run_output_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class InputCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: InputCustomAnnotatedConfig = DakotaField(
        default_factory=InputCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "InputCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "environment.post_run_input_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class TabularData(DakotaBaseModel):
    "Write a tabular results file with variable and response history"

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
    "(Experimental) Write a summary file containing the final results"

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
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "RESULTS_OUTPUT_TEXT",
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
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "RESULTS_OUTPUT_HDF5",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Output(DakotaBaseModel):
    "Base filename for pre-run mode data output"

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
    "Base filename for post-run mode data input"

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
    "Invoke Dakota with pre-run mode active"

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
    "Invoke Dakota with post-run mode active"

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
    "Top-level settings for Dakota execution"

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
