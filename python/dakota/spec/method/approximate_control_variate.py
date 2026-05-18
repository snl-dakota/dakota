"""Generated Pydantic models for method.approximate_control_variate"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField
from .base import MethodSelection
from typing import Literal, Union

# Cross-module model imports
from dakota.spec.shared.core import MethodConvergenceTolWithTypeContext3Mixin
from dakota.spec.shared.misc import (
    MethodMaxFunctionEvaluationsContext2Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodMlmfSolverMetricMixin,
    MethodMlmfSubProblemSolverMixin,
    MethodOptionalModelPointerMixin,
    MethodSeedSequenceMixin,
    MethodThreeOptionalKeywordsMixin,
    MlmfSolutionModeMixin,
    RngOptionsContext2Mixin,
)
from dakota.spec.shared.sampling import (
    MethodExportSamplesFormatMixin,
    MethodSampleTypeLhsMcMixin,
    MlmfPilotSamplesContext2Mixin,
)


class AcvIndependentSampling(DakotaBaseModel):
    "Sampling scheme within the approximate control variate (ACV) algorithm that employs independent samples (IS) across model pairings"

    acv_independent_sampling: Literal[True] = DakotaField(
        default=True,
        description="Sampling scheme within the approximate control variate (ACV) algorithm that employs independent samples (IS) across model pairings",
        dakota={
            "aliases": ["acv_is"],
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_ACV_IS",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class AcvMultifidelity(DakotaBaseModel):
    "Approximate control variate (ACV) algorithm that employs a nested pyramid sample pattern, similar to MFMC"

    acv_multifidelity: Literal[True] = DakotaField(
        default=True,
        description="Approximate control variate (ACV) algorithm that employs a nested pyramid sample pattern, similar to MFMC",
        dakota={
            "aliases": ["acv_mf"],
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_ACV_MF",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class AcvRecursiveDiff(DakotaBaseModel):
    "Sampling scheme within the approximate control variate (ACV) algorithm that employs weighted recursive differences (RD) across model pairings"

    acv_recursive_diff: Literal[True] = DakotaField(
        default=True,
        description="Sampling scheme within the approximate control variate (ACV) algorithm that employs weighted recursive differences (RD) across model pairings",
        dakota={
            "aliases": ["acv_rd"],
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_ACV_RD",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class ApproximateControlVariateSearchModelGraphsNoRecursion(DakotaBaseModel):
    "Do not recur over admissible DAGs for a given model ensemble"

    no_recursion: Literal[True] = DakotaField(
        default=True,
        description="Do not recur over admissible DAGs for a given model ensemble",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.search_model_graphs.recursion",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NO_GRAPH_RECURSION",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class KlRecursion(DakotaBaseModel):
    "Model graph enumeration that follows the ACV-KL partitioning scheme"

    kl_recursion: Literal[True] = DakotaField(
        default=True,
        description="Model graph enumeration that follows the ACV-KL partitioning scheme",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.search_model_graphs.recursion",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "KL_GRAPH_RECURSION",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PartialRecursionConfig(DakotaBaseModel):
    "Perform a partial recursion of admissible DAGs"

    depth_limit: int = DakotaField(
        description="Throttle the depth for a partial recursion of admissible DAGs",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.graph_depth_limit",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ApproximateControlVariateSearchModelGraphsFullRecursion(DakotaBaseModel):
    "Perform a full recursion of all admissible DAGs for a given model ensemble"

    full_recursion: Literal[True] = DakotaField(
        default=True,
        description="Perform a full recursion of all admissible DAGs for a given model ensemble",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.search_model_graphs.recursion",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "FULL_GRAPH_RECURSION",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ApproximateControlVariateExportSampleSequence(MethodExportSamplesFormatMixin):
    "Enable export of multilevel/multifidelity sample sequences to individual files"

    pass


class PartialRecursion(DakotaBaseModel):
    "Perform a partial recursion of admissible DAGs"

    partial_recursion: PartialRecursionConfig = DakotaField(
        description="Perform a partial recursion of admissible DAGs",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.search_model_graphs.recursion",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "PARTIAL_GRAPH_RECURSION",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ApproximateControlVariateSearchModelGraphs(DakotaBaseModel):
    "Perform a recursion of admissible DAGs for a given model ensemble"

    model_selection: Literal[True] | None = DakotaField(
        default=None,
        description="Perform a recursion of admissible model subsets for a given model ensemble",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.search_model_graphs.selection",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ALL_MODEL_COMBINATIONS",
                    "ir_value_type": "short",
                }
            ]
        },
    )
    dag_recursion: Union[
        ApproximateControlVariateSearchModelGraphsNoRecursion,
        KlRecursion,
        PartialRecursion,
        ApproximateControlVariateSearchModelGraphsFullRecursion,
    ] = DakotaField(
        description="DAG Ensemble Generation Option",
        dakota={"anchor": True, "union_pattern": 4},
    )


class ApproximateControlVariateConfig(
    MethodThreeOptionalKeywordsMixin,
    MlmfPilotSamplesContext2Mixin,
    MlmfSolutionModeMixin,
    MethodMlmfSubProblemSolverMixin,
    MethodMlmfSolverMetricMixin,
    MethodSeedSequenceMixin,
    MethodSampleTypeLhsMcMixin,
    MethodConvergenceTolWithTypeContext3Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodMaxFunctionEvaluationsContext2Mixin,
    RngOptionsContext2Mixin,
    MethodOptionalModelPointerMixin,
):
    "Approximate control variate (ACV) sampling methods for UQ"

    solution_approach: Union[
        AcvIndependentSampling, AcvMultifidelity, AcvRecursiveDiff
    ] = DakotaField(
        description="Solution Approach", dakota={"anchor": True, "union_pattern": 4}
    )
    search_model_graphs: ApproximateControlVariateSearchModelGraphs | None = (
        DakotaField(
            default=None,
            description="Perform a recursion of admissible DAGs for a given model ensemble",
        )
    )
    truth_fixed_by_pilot: Literal[True] | None = DakotaField(
        default=None,
        description="Option to suppress any increment to the number of truth samples",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.truth_fixed_by_pilot",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    export_sample_sequence: ApproximateControlVariateExportSampleSequence | None = (
        DakotaField(
            default=None,
            description="Enable export of multilevel/multifidelity sample sequences to individual files",
            dakota={
                "materialization": [
                    {
                        "ir_key": "method.nond.export_sample_sequence",
                        "storage_type": "PRESENCE_TRUE",
                        "ir_value_type": "bool",
                    }
                ]
            },
        )
    )


class ApproximateControlVariateSelection(MethodSelection):
    "Generated model for ApproximateControlVariateSelection"

    approximate_control_variate: ApproximateControlVariateConfig = DakotaField(
        dakota={
            "aliases": ["acv_sampling"],
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "APPROX_CONTROL_VARIATE",
                    "ir_value_type": "unsigned short",
                }
            ],
        }
    )
