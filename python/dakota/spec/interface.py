"""Generated Pydantic models for interface"""

from __future__ import annotations

from .base import DakotaBaseModel, DakotaField, DakotaFloat
from typing import Literal, Union
from .base import DBL_EPSILON
from typing import ClassVar, List
from .validation import ValidationRule
from .validation.rules import CheckAnalysisDrivers, CheckInterfaceBlock


class SystemParametersFormatStandard(DakotaBaseModel):
    "Use the standard format for parameters files"

    standard: Literal[True] = DakotaField(
        default=True,
        description="Use the standard format for parameters files",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.parameters_file_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "PARAMETERS_FILE_STANDARD",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class SystemParametersFormatAprepro(DakotaBaseModel):
    "Use the APREPRO format for parameters files"

    aprepro: Literal[True] = DakotaField(
        default=True,
        description="Use the APREPRO format for parameters files",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.parameters_file_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "PARAMETERS_FILE_APREPRO",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class SystemParametersFormatJsonFormat(DakotaBaseModel):
    "Use the JSON format for parameters files"

    json_format: Literal[True] = DakotaField(
        default=True,
        description="Use the JSON format for parameters files",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.parameters_file_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "PARAMETERS_FILE_JSON",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class SystemResultsFormatStandardConfig(DakotaBaseModel):
    "Expect results files in standard format"

    labeled: Literal[True] | None = DakotaField(
        default=None,
        description="Requires correct function value labels in results file",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.labeled_results",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class SystemResultsFormatJsonFormat(DakotaBaseModel):
    "Expect results files in JSON format"

    json_format: Literal[True] = DakotaField(
        default=True,
        description="Expect results files in JSON format",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.results_file_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RESULTS_FILE_JSON",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class SystemWorkDirectory(DakotaBaseModel):
    "Perform each function evaluation in a separate working directory"

    named: str | None = DakotaField(
        default=None,
        description="The base name of the work directory created by Dakota",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.workDir",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    directory_tag: Literal[True] | None = DakotaField(
        default=None,
        description="Tag each work directory with the function evaluation number",
        dakota={
            "aliases": ["dir_tag"],
            "materialization": [
                {
                    "ir_key": "interface.dirTag",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ],
        },
    )
    directory_save: Literal[True] | None = DakotaField(
        default=None,
        description="Preserve the work directory after function evaluation completion",
        dakota={
            "aliases": ["dir_save"],
            "materialization": [
                {
                    "ir_key": "interface.dirSave",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ],
        },
    )
    link_files: list[str] | None = DakotaField(
        default=None,
        description="Paths to be linked into each working directory",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.linkFiles",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )
    copy_files: list[str] | None = DakotaField(
        default=None,
        description="Files and directories to be copied into each working directory",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.copyFiles",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )
    replace: Literal[True] | None = DakotaField(
        default=None,
        description="Overwrite existing files within a work directory",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.templateReplace",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class ForkParametersFormatStandard(DakotaBaseModel):
    "Use the standard format for parameters files"

    standard: Literal[True] = DakotaField(
        default=True,
        description="Use the standard format for parameters files",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.parameters_file_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "PARAMETERS_FILE_STANDARD",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ForkParametersFormatAprepro(DakotaBaseModel):
    "Use the APREPRO format for parameters files"

    aprepro: Literal[True] = DakotaField(
        default=True,
        description="Use the APREPRO format for parameters files",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.parameters_file_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "PARAMETERS_FILE_APREPRO",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ForkParametersFormatJsonFormat(DakotaBaseModel):
    "Use the JSON format for parameters files"

    json_format: Literal[True] = DakotaField(
        default=True,
        description="Use the JSON format for parameters files",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.parameters_file_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "PARAMETERS_FILE_JSON",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ForkResultsFormatStandardConfig(DakotaBaseModel):
    "Expect results files in standard format"

    labeled: Literal[True] | None = DakotaField(
        default=None,
        description="Requires correct function value labels in results file",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.labeled_results",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class ForkResultsFormatJsonFormat(DakotaBaseModel):
    "Expect results files in JSON format"

    json_format: Literal[True] = DakotaField(
        default=True,
        description="Expect results files in JSON format",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.results_file_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RESULTS_FILE_JSON",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ForkWorkDirectory(DakotaBaseModel):
    "Perform each function evaluation in a separate working directory"

    named: str | None = DakotaField(
        default=None,
        description="The base name of the work directory created by Dakota",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.workDir",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    directory_tag: Literal[True] | None = DakotaField(
        default=None,
        description="Tag each work directory with the function evaluation number",
        dakota={
            "aliases": ["dir_tag"],
            "materialization": [
                {
                    "ir_key": "interface.dirTag",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ],
        },
    )
    directory_save: Literal[True] | None = DakotaField(
        default=None,
        description="Preserve the work directory after function evaluation completion",
        dakota={
            "aliases": ["dir_save"],
            "materialization": [
                {
                    "ir_key": "interface.dirSave",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ],
        },
    )
    link_files: list[str] | None = DakotaField(
        default=None,
        description="Paths to be linked into each working directory",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.linkFiles",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )
    copy_files: list[str] | None = DakotaField(
        default=None,
        description="Files and directories to be copied into each working directory",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.copyFiles",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )
    replace: Literal[True] | None = DakotaField(
        default=None,
        description="Overwrite existing files within a work directory",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.templateReplace",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class DirectConfig(DakotaBaseModel):
    "Run analysis drivers that are linked-to or compiled-with Dakota"

    processors_per_analysis: int | None = DakotaField(
        default=None,
        gt=0,
        description="Specify the number of processors per analysis when Dakota is run in parallel",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.direct.processors_per_analysis",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )


class PluginConfig(DakotaBaseModel):
    "Dynamically load a plugin analysis driver"

    library_path: str = DakotaField(
        description="Path to the plugin shared object file",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.plugin_library_path",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Matlab(DakotaBaseModel):
    "Run Matlab through a direct interface - requires special Dakota build"

    matlab: Literal[True] = DakotaField(
        default=True,
        description="Run Matlab through a direct interface - requires special Dakota build",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MATLAB_INTERFACE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class PythonConfig(DakotaBaseModel):
    "Run Python through a Pybind11-based direct interface - requires a special Dakota build"

    numpy: Literal[True] | None = DakotaField(
        default=None,
        description="Enable the use of numpy in Dakota's Python interface",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.python.numpy",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class Scilab(DakotaBaseModel):
    "Run Scilab through a direct interface - requires special Dakota build"

    scilab: Literal[True] = DakotaField(
        default=True,
        description="Run Scilab through a direct interface - requires special Dakota build",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SCILAB_INTERFACE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class AnalysisDriversGrid(DakotaBaseModel):
    "Deprecated grid computing interface"

    grid: Literal[True] = DakotaField(
        default=True,
        description="Deprecated grid computing interface",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "GRID_INTERFACE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Abort(DakotaBaseModel):
    "(Default) Abort the Dakota job"

    abort: Literal[True] = DakotaField(
        default=True,
        description="(Default) Abort the Dakota job",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.failure_capture.action",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "abort",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Retry(DakotaBaseModel):
    "Rerun failed analyses"

    retry: int = DakotaField(
        description="Rerun failed analyses",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.failure_capture.action",
                    "storage_type": "TYPE_DATA_COMBINED",
                    "stored_value": "retry",
                    "secondary_ir_key": "interface.failure_capture.retry_limit",
                    "secondary_ir_value_type": "int",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Recover(DakotaBaseModel):
    "Substitute dummy values for the responses"

    recover: list[DakotaFloat] = DakotaField(
        description="Substitute dummy values for the responses",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.failure_capture.action",
                    "storage_type": "TYPE_DATA_COMBINED",
                    "stored_value": "recover",
                    "secondary_ir_key": "interface.failure_capture.recovery_fn_vals",
                    "secondary_ir_value_type": "RealVector",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Continuation(DakotaBaseModel):
    'Cause Dakota to step toward the failed \\"target\\" simulation from a nearby successful \\"source\\"'

    continuation: Literal[True] = DakotaField(
        default=True,
        description='Cause Dakota to step toward the failed "target" simulation from a nearby successful "source"',
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.failure_capture.action",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "continuation",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class StrictCacheEquality(DakotaBaseModel):
    "Do not require strict cache equality when finding duplicates"

    cache_tolerance: DakotaFloat = DakotaField(
        default=DBL_EPSILON,
        description="Specify tolerance when identifying duplicate function evaluations",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.nearby_evaluation_cache_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class BatchConfig(DakotaBaseModel):
    "Perform evaluations in batches"

    size: int | None = DakotaField(
        default=None,
        gt=0,
        description="Limit the number of evaluations in a batch",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.asynch_local_evaluation_concurrency",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )


class LocalEvaluationSchedulingDynamic(DakotaBaseModel):
    "Dynamic local scheduling (sequential)"

    dynamic: Literal[True] = DakotaField(
        default=True,
        description="Dynamic local scheduling (sequential)",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.local_evaluation_scheduling",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DYNAMIC_SCHEDULING",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class LocalEvaluationSchedulingStatic(DakotaBaseModel):
    "Static local scheduling (tiled)"

    static: Literal[True] = DakotaField(
        default=True,
        description="Static local scheduling (tiled)",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.local_evaluation_scheduling",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "STATIC_SCHEDULING",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class EvaluationSchedulingDedicated(DakotaBaseModel):
    "Specify a dedicated scheduler partition for concurrent evaluation scheduling"

    dedicated: Literal[True] = DakotaField(
        default=True,
        description="Specify a dedicated scheduler partition for concurrent evaluation scheduling",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.evaluation_scheduling",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DEDICATED_SCHEDULER_DYNAMIC",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PeerDynamic(DakotaBaseModel):
    "Specify dynamic scheduling in a peer partition when Dakota is run in parallel."

    dynamic: Literal[True] = DakotaField(
        default=True,
        description="Specify dynamic scheduling in a peer partition when Dakota is run in parallel.",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.evaluation_scheduling",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "PEER_DYNAMIC_SCHEDULING",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PeerStatic(DakotaBaseModel):
    "Specify static scheduling in a peer partition when Dakota is run in parallel."

    static: Literal[True] = DakotaField(
        default=True,
        description="Specify static scheduling in a peer partition when Dakota is run in parallel.",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.evaluation_scheduling",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "PEER_STATIC_SCHEDULING",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class AnalysisSchedulingDedicated(DakotaBaseModel):
    "Specify a dedicated scheduler partition for concurrent analysis executions"

    dedicated: Literal[True] = DakotaField(
        default=True,
        description="Specify a dedicated scheduler partition for concurrent analysis executions",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.analysis_scheduling",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DEDICATED_SCHEDULER_DYNAMIC",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class AnalysisSchedulingPeer(DakotaBaseModel):
    "Specify a peer partition for parallel analysis scheduling"

    peer: Literal[True] = DakotaField(
        default=True,
        description="Specify a peer partition for parallel analysis scheduling",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.analysis_scheduling",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "PEER_SCHEDULING",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class SystemResultsFormatStandard(DakotaBaseModel):
    "Expect results files in standard format"

    standard: SystemResultsFormatStandardConfig = DakotaField(
        description="Expect results files in standard format",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.results_file_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RESULTS_FILE_STANDARD",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ForkResultsFormatStandard(DakotaBaseModel):
    "Expect results files in standard format"

    standard: ForkResultsFormatStandardConfig = DakotaField(
        description="Expect results files in standard format",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.results_file_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RESULTS_FILE_STANDARD",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Direct(DakotaBaseModel):
    "Run analysis drivers that are linked-to or compiled-with Dakota"

    direct: DirectConfig = DakotaField(
        description="Run analysis drivers that are linked-to or compiled-with Dakota",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TEST_INTERFACE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Plugin(DakotaBaseModel):
    "Dynamically load a plugin analysis driver"

    plugin: PluginConfig = DakotaField(
        description="Dynamically load a plugin analysis driver",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "PLUGIN_INTERFACE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Python(DakotaBaseModel):
    "Run Python through a Pybind11-based direct interface - requires a special Dakota build"

    python: PythonConfig = DakotaField(
        description="Run Python through a Pybind11-based direct interface - requires a special Dakota build",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "PYTHON_INTERFACE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Deactivate(DakotaBaseModel):
    "Deactivate Dakota interface features for simplicity or efficiency"

    active_set_vector: Literal[True] | None = DakotaField(
        default=None,
        description="Deactivate the Active Set Vector",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.active_set_vector",
                    "storage_type": "PRESENCE_FALSE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    evaluation_cache: Literal[True] | None = DakotaField(
        default=None,
        description="Do not retain function evaluation history in memory",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.evaluation_cache",
                    "storage_type": "PRESENCE_FALSE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    strict_cache_equality: StrictCacheEquality | None = DakotaField(
        default=None,
        description="Do not require strict cache equality when finding duplicates",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.nearby_evaluation_cache",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    restart_file: Literal[True] | None = DakotaField(
        default=None,
        description="Deactivate writing to the restart file",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.restart_file",
                    "storage_type": "PRESENCE_FALSE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class Batch(DakotaBaseModel):
    "Perform evaluations in batches"

    batch: BatchConfig = DakotaField(
        description="Perform evaluations in batches",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.batch",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class AsynchronousConfig(DakotaBaseModel):
    "Specify local evaluation or analysis concurrency"

    evaluation_concurrency: int | None = DakotaField(
        default=None,
        gt=0,
        description="Determine how many concurrent evaluations Dakota will schedule",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.asynch_local_evaluation_concurrency",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    local_evaluation_scheduling: (
        Union[LocalEvaluationSchedulingDynamic, LocalEvaluationSchedulingStatic] | None
    ) = DakotaField(
        default=None,
        description="Control how local asynchronous jobs are scheduled",
        dakota={"union_pattern": 2},
    )
    analysis_concurrency: int | None = DakotaField(
        default=None,
        gt=0,
        description="Limit the number of analysis drivers within an evaluation that Dakota will schedule",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.asynch_local_analysis_concurrency",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )


class EvaluationSchedulingPeer(DakotaBaseModel):
    "Specify a peer partition for parallel evaluation scheduling"

    peer: Union[PeerDynamic, PeerStatic] = DakotaField(
        description="Specify a peer partition for parallel evaluation scheduling"
    )


class SystemConfig(DakotaBaseModel):
    "(Not recommended) Launch analysis drivers with a system call"

    parameters_file: str | None = DakotaField(
        default=None,
        description="Specify the name of the parameters file",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.parameters_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    results_file: str | None = DakotaField(
        default=None,
        description="Specify the name of the results file",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.results_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    parameters_format: (
        Union[
            SystemParametersFormatStandard,
            SystemParametersFormatAprepro,
            SystemParametersFormatJsonFormat,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Format of the parameters file",
        dakota={
            "union_pattern": 2,
            "materialization": [
                {
                    "ir_key": "interface.application.parameters_file_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "PARAMETERS_FILE_STANDARD",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )
    results_format: (
        Union[SystemResultsFormatStandard, SystemResultsFormatJsonFormat] | None
    ) = DakotaField(
        default=None,
        description="Format of the results file",
        dakota={
            "union_pattern": 2,
            "materialization": [
                {
                    "ir_key": "interface.application.results_file_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RESULTS_FILE_STANDARD",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )
    file_tag: Literal[True] | None = DakotaField(
        default=None,
        description="Tag each parameters & results file name with the function evaluation number",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.file_tag",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    file_save: Literal[True] | None = DakotaField(
        default=None,
        description="Keep the parameters & results files after the analysis driver completes",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.file_save",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    work_directory: SystemWorkDirectory | None = DakotaField(
        default=None,
        description="Perform each function evaluation in a separate working directory",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.useWorkdir",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    allow_existing_results: Literal[True] | None = DakotaField(
        default=None,
        description="Change how Dakota deals with existing results files",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.allow_existing_results",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    verbatim: Literal[True] | None = DakotaField(
        default=None,
        description="Specify the command Dakota uses to launch analysis driver(s) and filters",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.verbatim",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class ForkConfig(DakotaBaseModel):
    "Launch analysis drivers using fork command"

    parameters_file: str | None = DakotaField(
        default=None,
        description="Specify the name of the parameters file",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.parameters_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    results_file: str | None = DakotaField(
        default=None,
        description="Specify the name of the results file",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.results_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    parameters_format: (
        Union[
            ForkParametersFormatStandard,
            ForkParametersFormatAprepro,
            ForkParametersFormatJsonFormat,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Format of the parameters file",
        dakota={
            "union_pattern": 2,
            "materialization": [
                {
                    "ir_key": "interface.application.parameters_file_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "PARAMETERS_FILE_STANDARD",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )
    results_format: (
        Union[ForkResultsFormatStandard, ForkResultsFormatJsonFormat] | None
    ) = DakotaField(
        default=None,
        description="Format of the results file",
        dakota={
            "union_pattern": 2,
            "materialization": [
                {
                    "ir_key": "interface.application.results_file_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RESULTS_FILE_STANDARD",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )
    file_tag: Literal[True] | None = DakotaField(
        default=None,
        description="Tag each parameters & results file name with the function evaluation number",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.file_tag",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    file_save: Literal[True] | None = DakotaField(
        default=None,
        description="Keep the parameters & results files after the analysis driver completes",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.file_save",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    work_directory: ForkWorkDirectory | None = DakotaField(
        default=None,
        description="Perform each function evaluation in a separate working directory",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.useWorkdir",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    allow_existing_results: Literal[True] | None = DakotaField(
        default=None,
        description="Change how Dakota deals with existing results files",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.allow_existing_results",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    verbatim: Literal[True] | None = DakotaField(
        default=None,
        description="Specify the command Dakota uses to launch analysis driver(s) and filters",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.verbatim",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class Asynchronous(DakotaBaseModel):
    "Specify local evaluation or analysis concurrency"

    asynchronous: AsynchronousConfig = DakotaField(
        description="Specify local evaluation or analysis concurrency",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.asynch",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class System(DakotaBaseModel):
    "(Not recommended) Launch analysis drivers with a system call"

    system: SystemConfig = DakotaField(
        description="(Not recommended) Launch analysis drivers with a system call",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SYSTEM_INTERFACE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Fork(DakotaBaseModel):
    "Launch analysis drivers using fork command"

    fork: ForkConfig = DakotaField(
        description="Launch analysis drivers using fork command",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "FORK_INTERFACE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class AnalysisDrivers(DakotaBaseModel):
    "Define how Dakota should run a function evaluation"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckAnalysisDrivers(context="analysis_drivers"),
    ]

    drivers: list[str] = DakotaField(
        description="Define how Dakota should run a function evaluation",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.analysis_drivers",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )
    input_filter: str | None = DakotaField(
        default=None,
        description="Run a pre-processing script before the analysis drivers",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.input_filter",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    output_filter: str | None = DakotaField(
        default=None,
        description="Run a post-processing script after the analysis drivers",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.output_filter",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    interface_type: Union[
        System, Fork, Direct, Plugin, Matlab, Python, Scilab, AnalysisDriversGrid
    ] = DakotaField(
        description="Interface Type", dakota={"anchor": True, "union_pattern": 4}
    )
    analysis_components: list[str] | None = DakotaField(
        default=None,
        description="Provide additional identifiers to analysis drivers.",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.analysis_components",
                    "storage_type": "ANALYSIS_COMPONENTS",
                    "ir_value_type": "String2DArray",
                }
            ]
        },
    )


class InterfaceConfig(DakotaBaseModel):
    "Specifies how function evaluations will be performed in order to map the variables into the responses."

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckInterfaceBlock(context="interface"),
    ]

    id_interface: str | None = DakotaField(
        default=None,
        description="Name the interface block; helpful when there are multiple",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.id",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    analysis_drivers: AnalysisDrivers | None = DakotaField(
        default=None,
        description="Define how Dakota should run a function evaluation",
        dakota={"argument": "drivers"},
    )
    algebraic_mappings: str | None = DakotaField(
        default=None,
        description="Use AMPL to define algebraic input-output mappings",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.algebraic_mappings",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    failure_capture: Union[Abort, Retry, Recover, Continuation] = DakotaField(
        default_factory=Abort,
        description="Determine how Dakota responds to analysis driver failure",
        dakota={"union_pattern": 1, "model_default": "Abort"},
    )
    deactivate: Deactivate | None = DakotaField(
        default=None,
        description="Deactivate Dakota interface features for simplicity or efficiency",
    )
    concurrency: Union[Batch, Asynchronous] | None = DakotaField(
        default=None,
        description="Concurrency Strategy",
        dakota={"anchor": True, "union_pattern": 2},
    )
    evaluation_servers: int | None = DakotaField(
        default=None,
        gt=0,
        description="Specify the number of evaluation servers when Dakota is run in parallel",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.evaluation_servers",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    evaluation_scheduling: (
        Union[EvaluationSchedulingDedicated, EvaluationSchedulingPeer] | None
    ) = DakotaField(
        default=None,
        description="Specify the scheduling of concurrent evaluations when Dakota is run in parallel",
        dakota={"union_pattern": 2},
    )
    processors_per_evaluation: int | None = DakotaField(
        default=None,
        gt=0,
        description="Specify the number of processors per evaluation server when Dakota is run in parallel",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.processors_per_evaluation",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    analysis_servers: int | None = DakotaField(
        default=None,
        gt=0,
        description="Specify the number of analysis servers when Dakota is run in parallel",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.analysis_servers",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    analysis_scheduling: (
        Union[AnalysisSchedulingDedicated, AnalysisSchedulingPeer] | None
    ) = DakotaField(
        default=None,
        description="Specify the scheduling of concurrent analyses when Dakota is run in parallel",
        dakota={"union_pattern": 2},
    )
