"""Generated Pydantic models for interface"""

from __future__ import annotations

from .base import DakotaBaseModel, DakotaField, DakotaFloat
from typing import Literal, Union
from .base import DBL_EPSILON
from typing import ClassVar, List
from .validation import ValidationRule
from .validation.rules import CheckAnalysisDrivers, CheckInterfaceBlock


class SystemParametersFormatStandard(DakotaBaseModel):
    """Generated model for SystemParametersFormatStandard"""

    standard: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.parameters_file_format",
                    "stored_value": "PARAMETERS_FILE_STANDARD",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class SystemParametersFormatAprepro(DakotaBaseModel):
    """Generated model for SystemParametersFormatAprepro"""

    aprepro: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.parameters_file_format",
                    "stored_value": "PARAMETERS_FILE_APREPRO",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class SystemParametersFormatJsonFormat(DakotaBaseModel):
    """Generated model for SystemParametersFormatJsonFormat"""

    json_format: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.parameters_file_format",
                    "stored_value": "PARAMETERS_FILE_JSON",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class SystemResultsFormatStandardConfig(DakotaBaseModel):
    """Generated model for SystemResultsFormatStandardConfig"""

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
    """Generated model for SystemResultsFormatJsonFormat"""

    json_format: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.results_file_format",
                    "stored_value": "RESULTS_FILE_JSON",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class SystemWorkDirectory(DakotaBaseModel):
    """Generated model for SystemWorkDirectory"""

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
    """Generated model for ForkParametersFormatStandard"""

    standard: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.parameters_file_format",
                    "stored_value": "PARAMETERS_FILE_STANDARD",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ForkParametersFormatAprepro(DakotaBaseModel):
    """Generated model for ForkParametersFormatAprepro"""

    aprepro: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.parameters_file_format",
                    "stored_value": "PARAMETERS_FILE_APREPRO",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ForkParametersFormatJsonFormat(DakotaBaseModel):
    """Generated model for ForkParametersFormatJsonFormat"""

    json_format: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.parameters_file_format",
                    "stored_value": "PARAMETERS_FILE_JSON",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ForkResultsFormatStandardConfig(DakotaBaseModel):
    """Generated model for ForkResultsFormatStandardConfig"""

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
    """Generated model for ForkResultsFormatJsonFormat"""

    json_format: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.results_file_format",
                    "stored_value": "RESULTS_FILE_JSON",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ForkWorkDirectory(DakotaBaseModel):
    """Generated model for ForkWorkDirectory"""

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
    """Generated model for DirectConfig"""

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
    """Generated model for PluginConfig"""

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
    """Generated model for Matlab"""

    matlab: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.type",
                    "stored_value": "MATLAB_INTERFACE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class PythonConfig(DakotaBaseModel):
    """Generated model for PythonConfig"""

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
    """Generated model for Scilab"""

    scilab: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.type",
                    "stored_value": "SCILAB_INTERFACE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class AnalysisDriversGrid(DakotaBaseModel):
    """Generated model for AnalysisDriversGrid"""

    grid: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.type",
                    "stored_value": "GRID_INTERFACE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Abort(DakotaBaseModel):
    """Generated model for Abort"""

    abort: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.failure_capture.action",
                    "stored_value": "abort",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Retry(DakotaBaseModel):
    """Generated model for Retry"""

    retry: int = DakotaField(
        description="Rerun failed analyses",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.failure_capture.action",
                    "stored_value": "retry",
                    "storage_type": "TYPE_DATA_COMBINED",
                    "secondary_ir_key": "interface.failure_capture.retry_limit",
                    "secondary_ir_value_type": "int",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Recover(DakotaBaseModel):
    """Generated model for Recover"""

    recover: list[DakotaFloat] = DakotaField(
        description="Substitute dummy values for the responses",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.failure_capture.action",
                    "stored_value": "recover",
                    "storage_type": "TYPE_DATA_COMBINED",
                    "secondary_ir_key": "interface.failure_capture.recovery_fn_vals",
                    "secondary_ir_value_type": "RealVector",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Continuation(DakotaBaseModel):
    """Generated model for Continuation"""

    continuation: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.failure_capture.action",
                    "stored_value": "continuation",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class StrictCacheEquality(DakotaBaseModel):
    """Generated model for StrictCacheEquality"""

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
    """Generated model for BatchConfig"""

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
    """Generated model for LocalEvaluationSchedulingDynamic"""

    dynamic: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.local_evaluation_scheduling",
                    "stored_value": "DYNAMIC_SCHEDULING",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class LocalEvaluationSchedulingStatic(DakotaBaseModel):
    """Generated model for LocalEvaluationSchedulingStatic"""

    static: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.local_evaluation_scheduling",
                    "stored_value": "STATIC_SCHEDULING",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class EvaluationSchedulingDedicated(DakotaBaseModel):
    """Generated model for EvaluationSchedulingDedicated"""

    dedicated: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.evaluation_scheduling",
                    "stored_value": "DEDICATED_SCHEDULER_DYNAMIC",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PeerDynamic(DakotaBaseModel):
    """Generated model for PeerDynamic"""

    dynamic: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.evaluation_scheduling",
                    "stored_value": "PEER_DYNAMIC_SCHEDULING",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PeerStatic(DakotaBaseModel):
    """Generated model for PeerStatic"""

    static: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.evaluation_scheduling",
                    "stored_value": "PEER_STATIC_SCHEDULING",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class AnalysisSchedulingDedicated(DakotaBaseModel):
    """Generated model for AnalysisSchedulingDedicated"""

    dedicated: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.analysis_scheduling",
                    "stored_value": "DEDICATED_SCHEDULER_DYNAMIC",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class AnalysisSchedulingPeer(DakotaBaseModel):
    """Generated model for AnalysisSchedulingPeer"""

    peer: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.analysis_scheduling",
                    "stored_value": "PEER_SCHEDULING",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class SystemResultsFormatStandard(DakotaBaseModel):
    """Generated model for SystemResultsFormatStandard"""

    standard: SystemResultsFormatStandardConfig = DakotaField(
        description="Expect results files in standard format",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.results_file_format",
                    "stored_value": "RESULTS_FILE_STANDARD",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ForkResultsFormatStandard(DakotaBaseModel):
    """Generated model for ForkResultsFormatStandard"""

    standard: ForkResultsFormatStandardConfig = DakotaField(
        description="Expect results files in standard format",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.application.results_file_format",
                    "stored_value": "RESULTS_FILE_STANDARD",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Direct(DakotaBaseModel):
    """Generated model for Direct"""

    direct: DirectConfig = DakotaField(
        description="Run analysis drivers that are linked-to or compiled-with Dakota",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.type",
                    "stored_value": "TEST_INTERFACE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Plugin(DakotaBaseModel):
    """Generated model for Plugin"""

    plugin: PluginConfig = DakotaField(
        description="Dynamically load a plugin analysis driver",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.type",
                    "stored_value": "PLUGIN_INTERFACE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Python(DakotaBaseModel):
    """Generated model for Python"""

    python: PythonConfig = DakotaField(
        description="Run Python through a Pybind11-based direct interface - requires a special Dakota build",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.type",
                    "stored_value": "PYTHON_INTERFACE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Deactivate(DakotaBaseModel):
    """Generated model for Deactivate"""

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
    """Generated model for Batch"""

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
    """Generated model for AsynchronousConfig"""

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
    """Generated model for EvaluationSchedulingPeer"""

    peer: Union[PeerDynamic, PeerStatic]


class SystemConfig(DakotaBaseModel):
    """Generated model for SystemConfig"""

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
                    "stored_value": "PARAMETERS_FILE_STANDARD",
                    "storage_type": "PRESENCE_ENUM",
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
                    "stored_value": "RESULTS_FILE_STANDARD",
                    "storage_type": "PRESENCE_ENUM",
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
    """Generated model for ForkConfig"""

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
                    "stored_value": "PARAMETERS_FILE_STANDARD",
                    "storage_type": "PRESENCE_ENUM",
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
                    "stored_value": "RESULTS_FILE_STANDARD",
                    "storage_type": "PRESENCE_ENUM",
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
    """Generated model for Asynchronous"""

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
    """Generated model for System"""

    system: SystemConfig = DakotaField(
        description="(Not recommended) Launch analysis drivers with a system call",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.type",
                    "stored_value": "SYSTEM_INTERFACE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Fork(DakotaBaseModel):
    """Generated model for Fork"""

    fork: ForkConfig = DakotaField(
        description="Launch analysis drivers using fork command",
        dakota={
            "materialization": [
                {
                    "ir_key": "interface.type",
                    "stored_value": "FORK_INTERFACE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class AnalysisDrivers(DakotaBaseModel):
    """Generated model for AnalysisDrivers"""

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
    """Generated model for InterfaceConfig"""

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
