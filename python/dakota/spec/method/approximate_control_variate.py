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
    """Generated model for AcvIndependentSampling"""

    acv_independent_sampling: Literal[True] = DakotaField(
        default=True,
        dakota={
            "aliases": ["acv_is"],
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "stored_value": "SUBMETHOD_ACV_IS",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class AcvMultifidelity(DakotaBaseModel):
    """Generated model for AcvMultifidelity"""

    acv_multifidelity: Literal[True] = DakotaField(
        default=True,
        dakota={
            "aliases": ["acv_mf"],
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "stored_value": "SUBMETHOD_ACV_MF",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class AcvRecursiveDiff(DakotaBaseModel):
    """Generated model for AcvRecursiveDiff"""

    acv_recursive_diff: Literal[True] = DakotaField(
        default=True,
        dakota={
            "aliases": ["acv_rd"],
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "stored_value": "SUBMETHOD_ACV_RD",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class ApproximateControlVariateSearchModelGraphsNoRecursion(DakotaBaseModel):
    """Generated model for ApproximateControlVariateSearchModelGraphsNoRecursion"""

    no_recursion: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.search_model_graphs.recursion",
                    "stored_value": "NO_GRAPH_RECURSION",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class KlRecursion(DakotaBaseModel):
    """Generated model for KlRecursion"""

    kl_recursion: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.search_model_graphs.recursion",
                    "stored_value": "KL_GRAPH_RECURSION",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PartialRecursionConfig(DakotaBaseModel):
    """Generated model for PartialRecursionConfig"""

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
    """Generated model for ApproximateControlVariateSearchModelGraphsFullRecursion"""

    full_recursion: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.search_model_graphs.recursion",
                    "stored_value": "FULL_GRAPH_RECURSION",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ApproximateControlVariateExportSampleSequence(MethodExportSamplesFormatMixin):
    """Generated model for ApproximateControlVariateExportSampleSequence"""

    pass


class PartialRecursion(DakotaBaseModel):
    """Generated model for PartialRecursion"""

    partial_recursion: PartialRecursionConfig = DakotaField(
        description="Perform a partial recursion of admissible DAGs",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.search_model_graphs.recursion",
                    "stored_value": "PARTIAL_GRAPH_RECURSION",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ApproximateControlVariateSearchModelGraphs(DakotaBaseModel):
    """Generated model for ApproximateControlVariateSearchModelGraphs"""

    model_selection: Literal[True] | None = DakotaField(
        default=None,
        description="Perform a recursion of admissible model subsets for a given model ensemble",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.search_model_graphs.selection",
                    "stored_value": "ALL_MODEL_COMBINATIONS",
                    "storage_type": "PRESENCE_ENUM",
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
    """Generated model for ApproximateControlVariateConfig"""

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
    """Generated model for ApproximateControlVariateSelection"""

    approximate_control_variate: ApproximateControlVariateConfig = DakotaField(
        dakota={
            "aliases": ["acv_sampling"],
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "APPROX_CONTROL_VARIATE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        }
    )
