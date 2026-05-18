"""Generated Pydantic models for method.mesh_adaptive_search"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField, DakotaFloat
from .base import MethodSelection
from typing import Literal, Union

# Cross-module model imports
from dakota.spec.shared.misc import (
    DefaultScalingMixin,
    MethodMaxFunctionEvaluationsContext1Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
)


class InformSearch(DakotaBaseModel):
    "Surrogate informs evaluation order in mesh adaptive search"

    inform_search: Literal[True] = DakotaField(
        default=True,
        description="Surrogate informs evaluation order in mesh adaptive search",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.mesh_adaptive_search.use_surrogate",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "inform_search",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Optimize(DakotaBaseModel):
    "Surrogate is used in lieu of true model for mesh adaptive search"

    optimize: Literal[True] = DakotaField(
        default=True,
        description="Surrogate is used in lieu of true model for mesh adaptive search",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.mesh_adaptive_search.use_surrogate",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "optimize",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MeshAdaptiveSearchConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodMaxIterationsContext1Mixin,
    MethodMaxFunctionEvaluationsContext1Mixin,
    DefaultScalingMixin,
    MethodOptionalModelPointerMixin,
):
    "Finds optimal variable values using adaptive mesh-based search"

    initial_delta: DakotaFloat = DakotaField(
        default=1.0,
        description="Initial step size for derivative-free optimizers",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.mesh_adaptive_search.initial_delta",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    variable_tolerance: DakotaFloat = DakotaField(
        default=1.0e-6,
        description="Step length-based stopping criteria for derivative-free optimizers",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.mesh_adaptive_search.variable_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    function_precision: DakotaFloat = DakotaField(
        default=1.0e-10,
        description="Specify the maximum precision of the analysis code responses",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.function_precision",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    seed: int | None = DakotaField(
        default=None,
        gt=0,
        description="Seed of the random number generator",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.random_seed",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    history_file: str = DakotaField(
        default="mads_history",
        description="Name of file where mesh adaptive search records all evaluation points.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.mesh_adaptive_search.history_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    display_format: str = DakotaField(
        default="bbe obj",
        description="Information to be reported from mesh adaptive search's internal records.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.mesh_adaptive_search.display_format",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    variable_neighborhood_search: DakotaFloat = DakotaField(
        default=0.0,
        description="Percentage of evaluations to do to escape local minima.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.mesh_adaptive_search.variable_neighborhood_search",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    neighbor_order: int = DakotaField(
        default=1,
        gt=0,
        description="Number of dimensions in which to perturb categorical variables.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.mesh_adaptive_search.neighbor_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    display_all_evaluations: Literal[True] | None = DakotaField(
        default=None,
        description="Shows mesh adaptive search's internally held list of all evaluations",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.mesh_adaptive_search.display_all_evaluations",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    use_surrogate: Union[InformSearch, Optimize] | None = DakotaField(
        default=None,
        description="Surrogate model usage mode for mesh adaptive search",
        dakota={"union_pattern": 2},
    )


class MeshAdaptiveSearchSelection(MethodSelection):
    "Generated model for MeshAdaptiveSearchSelection"

    mesh_adaptive_search: MeshAdaptiveSearchConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MESH_ADAPTIVE_SEARCH",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
