"""Generated Pydantic models for method.coliny_solis_wets"""

from __future__ import annotations

from ..base import DakotaField, DakotaFloat
from .base import MethodSelection
from typing import Literal

# Cross-module model imports
from dakota.spec.shared.misc import (
    ColinyCommonOptsMixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
)


class ColinySolisWetsConfig(
    MethodThreeOptionalKeywordsMixin,
    ColinyCommonOptsMixin,
    MethodOptionalModelPointerMixin,
):
    "Simple greedy local search method"

    contract_after_failure: int = DakotaField(
        default=0,
        description="The number of unsuccessful cycles prior to contraction.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.contract_after_failure",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    no_expansion: Literal[True] | None = DakotaField(
        default=None,
        description="Don't allow expansion of the search pattern",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.expansion",
                    "storage_type": "PRESENCE_FALSE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    expand_after_success: int = DakotaField(
        default=0,
        description="Set the factor by which a search pattern can be expanded",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.expand_after_success",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    constant_penalty: Literal[True] | None = DakotaField(
        default=None,
        description="Use a simple weighted penalty to manage feasibility",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.constant_penalty",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    contraction_factor: DakotaFloat = DakotaField(
        default=0.5,
        description="Amount by which step length is rescaled",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.contraction_factor",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    constraint_penalty: DakotaFloat = DakotaField(
        default=-1.0,
        description="Multiplier for the penalty function",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.constraint_penalty",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    initial_delta: DakotaFloat = DakotaField(
        default=-1.0,
        description="Initial step size for derivative-free optimizers",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.initial_delta",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    variable_tolerance: DakotaFloat = DakotaField(
        default=-1.0,
        description="Step length-based stopping criteria for derivative-free optimizers",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.variable_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class ColinySolisWetsSelection(MethodSelection):
    "Generated model for ColinySolisWetsSelection"

    coliny_solis_wets: ColinySolisWetsConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "COLINY_SOLIS_WETS",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
