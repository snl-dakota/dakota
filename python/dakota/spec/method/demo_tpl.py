"""Generated Pydantic models for method.demo_tpl"""

from __future__ import annotations

from ..base import DakotaField, DakotaFloat
from .base import MethodSelection
from ..base import DBL_MAX

# Cross-module model imports
from dakota.spec.shared.core import MethodConvergenceTolMixin
from dakota.spec.shared.misc import (
    MethodMaxFunctionEvaluationsContext1Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodThreeOptionalKeywordsMixin,
)


class DemoTplConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodMaxFunctionEvaluationsContext1Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodConvergenceTolMixin,
):
    """Generated model for DemoTplConfig"""

    variable_tolerance: DakotaFloat = DakotaField(
        default=0.01,
        description="Step length-based stopping criteria for derivative-free optimizers",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.variable_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    solution_target: DakotaFloat = DakotaField(
        default=-DBL_MAX,
        description="Stopping criteria based on objective function value",
        dakota={
            "aliases": ["solution_accuracy"],
            "materialization": [
                {
                    "ir_key": "method.asynch_pattern_search.solution_target",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
                {
                    "ir_key": "method.solution_target",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
            ],
        },
    )
    options_file: str | None = DakotaField(
        default=None,
        description="Filename for a YAML file that specifies Gaussian process options",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.advanced_options_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class DemoTplSelection(MethodSelection):
    """Generated model for DemoTplSelection"""

    demo_tpl: DemoTplConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "DEMO_TPL",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
