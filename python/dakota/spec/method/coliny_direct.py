"""Generated Pydantic models for method.coliny_direct"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField, DakotaFloat
from .base import MethodSelection
from typing import Literal, Union

# Cross-module model imports
from dakota.spec.shared.misc import (
    ColinyCommonOptsMixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
)


class MajorDim(DakotaBaseModel):
    """Generated model for MajorDim"""

    major_dimension: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.division",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "major_dimension",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class AllDims(DakotaBaseModel):
    """Generated model for AllDims"""

    all_dimensions: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.division",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all_dimensions",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ColinyDirectConfig(
    MethodThreeOptionalKeywordsMixin,
    ColinyCommonOptsMixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for ColinyDirectConfig"""

    division: Union[MajorDim, AllDims] | None = DakotaField(
        default=None,
        description="Determine how rectangles are subdivided",
        dakota={"union_pattern": 2},
    )
    global_balance_parameter: DakotaFloat = DakotaField(
        default=-1.0,
        description="Tolerance for whether a subregion is worth dividing",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.global_balance_parameter",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    local_balance_parameter: DakotaFloat = DakotaField(
        default=-1.0,
        description="Tolerance for whether a subregion is worth dividing",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.local_balance_parameter",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    max_boxsize_limit: DakotaFloat = DakotaField(
        default=-1.0,
        description="Stopping Criterion based on longest edge of hyperrectangle",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.max_boxsize_limit",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    min_boxsize_limit: DakotaFloat = DakotaField(
        default=-1.0,
        description="Stopping Criterion based on shortest edge of hyperrectangle",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.min_boxsize_limit",
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


class ColinyDirectSelection(MethodSelection):
    """Generated model for ColinyDirectSelection"""

    coliny_direct: ColinyDirectConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "COLINY_DIRECT",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
