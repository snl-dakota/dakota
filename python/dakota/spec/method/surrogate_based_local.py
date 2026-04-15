"""Generated Pydantic models for method.surrogate_based_local"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField
from .base import MethodSelection
from typing import Literal, Union

# Cross-module model imports
from dakota.spec.shared.core import DefaultConstraintTolMixin, MethodConvergenceTolMixin
from dakota.spec.shared.misc import (
    DefaultTrustRegionContext1Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodThreeOptionalKeywordsMixin,
)


class SurrogateBasedLocalMethodPointer(DakotaBaseModel):
    """Generated model for SurrogateBasedLocalMethodPointer"""

    method_pointer: str = DakotaField(
        description="Pointer to sub-method to apply to a surrogate or branch-and-bound sub-problem",
        dakota={
            "block_pointer": "method",
            "aliases": ["approx_method_pointer"],
            "materialization": [
                {
                    "ir_key": "method.sub_method_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class SurrogateBasedLocalMethodName(DakotaBaseModel):
    """Generated model for SurrogateBasedLocalMethodName"""

    method_name: str = DakotaField(
        description="Specify sub-method by name",
        dakota={
            "aliases": ["approx_method_name"],
            "materialization": [
                {
                    "ir_key": "method.sub_method_name",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class OriginalPrimary(DakotaBaseModel):
    """Generated model for OriginalPrimary"""

    original_primary: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sbl.subproblem_objective",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ORIGINAL_PRIMARY",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class SingleObjective(DakotaBaseModel):
    """Generated model for SingleObjective"""

    single_objective: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sbl.subproblem_objective",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SINGLE_OBJECTIVE",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class AugmentedLagrangianObjective(DakotaBaseModel):
    """Generated model for AugmentedLagrangianObjective"""

    augmented_lagrangian_objective: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sbl.subproblem_objective",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "AUGMENTED_LAGRANGIAN_OBJECTIVE",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class LagrangianObjective(DakotaBaseModel):
    """Generated model for LagrangianObjective"""

    lagrangian_objective: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sbl.subproblem_objective",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LAGRANGIAN_OBJECTIVE",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class OriginalConstraints(DakotaBaseModel):
    """Generated model for OriginalConstraints"""

    original_constraints: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sbl.subproblem_constraints",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ORIGINAL_CONSTRAINTS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class LinearizedConstraints(DakotaBaseModel):
    """Generated model for LinearizedConstraints"""

    linearized_constraints: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sbl.subproblem_constraints",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LINEARIZED_CONSTRAINTS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class NoConstraints(DakotaBaseModel):
    """Generated model for NoConstraints"""

    no_constraints: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sbl.subproblem_constraints",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NO_CONSTRAINTS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PenaltyMerit(DakotaBaseModel):
    """Generated model for PenaltyMerit"""

    penalty_merit: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sbl.merit_function",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "PENALTY_MERIT",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class AdaptivePenaltyMerit(DakotaBaseModel):
    """Generated model for AdaptivePenaltyMerit"""

    adaptive_penalty_merit: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sbl.merit_function",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ADAPTIVE_PENALTY_MERIT",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class LagrangianMerit(DakotaBaseModel):
    """Generated model for LagrangianMerit"""

    lagrangian_merit: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sbl.merit_function",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LAGRANGIAN_MERIT",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class AugmentedLagrangianMerit(DakotaBaseModel):
    """Generated model for AugmentedLagrangianMerit"""

    augmented_lagrangian_merit: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sbl.merit_function",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "AUGMENTED_LAGRANGIAN_MERIT",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class TrRatio(DakotaBaseModel):
    """Generated model for TrRatio"""

    tr_ratio: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sbl.acceptance_logic",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TR_RATIO",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class Filter(DakotaBaseModel):
    """Generated model for Filter"""

    filter: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sbl.acceptance_logic",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "FILTER",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ConstraintRelax(DakotaBaseModel):
    """Generated model for ConstraintRelax"""

    homotopy: Literal[True] = DakotaField(
        description="Surrogate-Based local constraint relaxation method for infeasible iterates",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sbl.constraint_relax",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "HOMOTOPY",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ApproxSubproblem(DakotaBaseModel):
    """Generated model for ApproxSubproblem"""

    objective_formulation: Union[
        OriginalPrimary,
        SingleObjective,
        AugmentedLagrangianObjective,
        LagrangianObjective,
    ] = DakotaField(
        description="Objective Formulation", dakota={"anchor": True, "union_pattern": 4}
    )
    constraint_formulation: Union[
        OriginalConstraints, LinearizedConstraints, NoConstraints
    ] = DakotaField(
        description="Constraint Formulation",
        dakota={"anchor": True, "union_pattern": 4},
    )


class SurrogateBasedLocalConfig(
    MethodThreeOptionalKeywordsMixin,
    DefaultTrustRegionContext1Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodConvergenceTolMixin,
    DefaultConstraintTolMixin,
):
    """Generated model for SurrogateBasedLocalConfig"""

    sub_method: Union[
        SurrogateBasedLocalMethodPointer, SurrogateBasedLocalMethodName
    ] = DakotaField(
        description="Subproblem Optimizer Selection",
        dakota={"anchor": True, "union_pattern": 4},
    )
    model_pointer: str = DakotaField(
        description="Identifier for model block to be used by a method",
        dakota={
            "block_pointer": "model",
            "aliases": ["approx_model_pointer"],
            "materialization": [
                {
                    "ir_key": "method.model_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    soft_convergence_limit: int = DakotaField(
        default=0,
        description="Limit number of iterations w/ little improvement",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.soft_convergence_limit",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    truth_surrogate_bypass: Literal[True] | None = DakotaField(
        default=None,
        description="Bypass lower level surrogates when performing truth verifications on a top level surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sbl.truth_surrogate_bypass",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    approx_subproblem: ApproxSubproblem | None = DakotaField(
        default=None,
        description="Identify functions to be included in surrogate merit function",
    )
    merit_function: Union[
        PenaltyMerit, AdaptivePenaltyMerit, LagrangianMerit, AugmentedLagrangianMerit
    ] = DakotaField(
        default_factory=AugmentedLagrangianMerit,
        description="Select type of penalty or merit function",
        dakota={"union_pattern": 1, "model_default": "AugmentedLagrangianMerit"},
    )
    acceptance_logic: Union[TrRatio, Filter] = DakotaField(
        default_factory=Filter,
        description="Set criteria for trusted surrogate",
        dakota={"union_pattern": 1, "model_default": "Filter"},
    )
    constraint_relax: ConstraintRelax | None = DakotaField(
        default=None, description="Enable constraint relaxation"
    )


class SurrogateBasedLocalSelection(MethodSelection):
    """Generated model for SurrogateBasedLocalSelection"""

    surrogate_based_local: SurrogateBasedLocalConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SURROGATE_BASED_LOCAL",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
