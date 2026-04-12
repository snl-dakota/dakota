"""Generated Pydantic models for shared.expansion/function_train"""

from __future__ import annotations

from ....base import DakotaBaseModel, DakotaField, DakotaFloat
from typing import Literal, Union
from ....base import SZ_MAX, USHRT_MAX
from typing import ClassVar, List
from ....validation import ValidationRule
from ....validation.rules import CheckNonnegativeList, CompareLength, CompareLengthOne


class FtMethodOrderScalarsMixin(DakotaBaseModel):
    """Generated model for FtMethodOrderScalarsMixin"""

    adapt_order: Literal[True] | None = DakotaField(
        default=None,
        description="Activate adaptive procedure for determining the best basis order",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.adapt_order",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    kick_order: int = DakotaField(
        default=1,
        gt=0,
        description="increment used when adapting the basis order in function train methods",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.kick_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    max_order: int = DakotaField(
        default=USHRT_MAX,
        ge=0,
        description="Maximum polynomial order of each univariate function within the functional tensor train.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.max_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    max_cv_order_candidates: int = DakotaField(
        default=USHRT_MAX,
        ge=0,
        description="Limit the number of cross-validation candidates for basis order",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.max_order_candidates",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class FtMethodRankMixin(DakotaBaseModel):
    """Generated model for FtMethodRankMixin"""

    start_rank: int = DakotaField(
        default=2,
        ge=0,
        description="The initial rank used for the starting point during a rank adaptation.",
        dakota={
            "aliases": ["rank"],
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.start_rank",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ],
        },
    )
    adapt_rank: Literal[True] | None = DakotaField(
        default=None,
        description="Activate adaptive procedure for determining best rank representation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.adapt_rank",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    kick_rank: int = DakotaField(
        default=1,
        gt=0,
        description="The increment in rank employed during each iteration of the rank adaptation.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.kick_rank",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    max_rank: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Limits the maximum rank that is explored during a rank adaptation.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.max_rank",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    max_cv_rank_candidates: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Limit the number of cross-validation candidates for rank",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.max_rank_candidates",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class FtMethodRankScalarsMixin(DakotaBaseModel):
    """Generated model for FtMethodRankScalarsMixin"""

    adapt_rank: Literal[True] | None = DakotaField(
        default=None,
        description="Activate adaptive procedure for determining best rank representation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.adapt_rank",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    kick_rank: int = DakotaField(
        default=1,
        gt=0,
        description="The increment in rank employed during each iteration of the rank adaptation.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.kick_rank",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    max_rank: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Limits the maximum rank that is explored during a rank adaptation.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.max_rank",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    max_cv_rank_candidates: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Limit the number of cross-validation candidates for rank",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.max_rank_candidates",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class FtMethodTolsMixin(DakotaBaseModel):
    """Generated model for FtMethodTolsMixin"""

    rounding_tolerance: DakotaFloat = DakotaField(
        default=1.0e-10,
        description="An accuracy tolerance that is used to guide rounding during rank adaptation.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.solver_rounding_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    arithmetic_tolerance: DakotaFloat = DakotaField(
        default=1.0e-10,
        description="A secondary rounding tolerance used for post-processing",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.stats_rounding_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class FtModelRankMixin(DakotaBaseModel):
    """Generated model for FtModelRankMixin"""

    start_rank: int = DakotaField(
        default=2,
        ge=0,
        description="The initial rank used for the starting point during a rank adaptation.",
        dakota={
            "aliases": ["rank"],
            "materialization": [
                {
                    "ir_key": "model.c3function_train.start_rank",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ],
        },
    )
    adapt_rank: Literal[True] | None = DakotaField(
        default=None,
        description="Activate adaptive procedure for determining best rank representation",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.c3function_train.adapt_rank",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    kick_rank: int = DakotaField(
        default=1,
        gt=0,
        description="The increment in rank employed during each iteration of the rank adaptation.",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.c3function_train.kick_rank",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    max_rank: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Limits the maximum rank that is explored during a rank adaptation.",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.c3function_train.max_rank",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    max_cv_rank_candidates: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Limit the number of cross-validation candidates for rank",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.c3function_train.cross_validation.max_rank_candidates",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class FtModelTolsMixin(DakotaBaseModel):
    """Generated model for FtModelTolsMixin"""

    rounding_tolerance: DakotaFloat = DakotaField(
        default=1.0e-10,
        description="An accuracy tolerance that is used to guide rounding during rank adaptation.",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.c3function_train.solver_rounding_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    arithmetic_tolerance: DakotaFloat = DakotaField(
        default=1.0e-10,
        description="A secondary rounding tolerance used for post-processing",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.c3function_train.stats_rounding_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class MlftMethodRankSequenceMixin(DakotaBaseModel):
    """Generated model for MlftMethodRankSequenceMixin"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="mlftmethodranksequencemixin", list_field="start_rank_sequence"
        ),
    ]

    start_rank_sequence: list[int] | None = DakotaField(
        default=None,
        description="Sequence of start ranks used in a multi-stage expansion",
        dakota={
            "aliases": ["rank_sequence"],
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.start_rank_sequence",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "SizetArray",
                }
            ],
        },
    )
    adapt_rank: Literal[True] | None = DakotaField(
        default=None,
        description="Activate adaptive procedure for determining best rank representation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.adapt_rank",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    kick_rank: int = DakotaField(
        default=1,
        gt=0,
        description="The increment in rank employed during each iteration of the rank adaptation.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.kick_rank",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    max_rank: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Limits the maximum rank that is explored during a rank adaptation.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.max_rank",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    max_cv_rank_candidates: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Limit the number of cross-validation candidates for rank",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.max_rank_candidates",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class FtMethodOrderStartOrder(DakotaBaseModel):
    """Generated model for FtMethodOrderStartOrder"""

    value: int = DakotaField(
        default=2,
        ge=0,
        description="(Initial) polynomial order of each univariate function within the functional tensor train.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.start_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    dimension_preference: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="A set of weights specifying the realtive importance of each uncertain variable (dimension)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dimension_preference",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class IncrementStartRank(DakotaBaseModel):
    """Generated model for IncrementStartRank"""

    increment_start_rank: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.advancement_type",
                    "stored_value": "START_RANK_ADVANCEMENT",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class IncrementStartOrder(DakotaBaseModel):
    """Generated model for IncrementStartOrder"""

    increment_start_order: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.advancement_type",
                    "stored_value": "START_ORDER_ADVANCEMENT",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class IncrementMaxRank(DakotaBaseModel):
    """Generated model for IncrementMaxRank"""

    increment_max_rank: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.advancement_type",
                    "stored_value": "MAX_RANK_ADVANCEMENT",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class IncrementMaxOrder(DakotaBaseModel):
    """Generated model for IncrementMaxOrder"""

    increment_max_order: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.advancement_type",
                    "stored_value": "MAX_ORDER_ADVANCEMENT",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class IncrementMaxRankOrder(DakotaBaseModel):
    """Generated model for IncrementMaxRankOrder"""

    increment_max_rank_order: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.advancement_type",
                    "stored_value": "MAX_RANK_ORDER_ADVANCEMENT",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class FtMethodRegressionTypeLs(DakotaBaseModel):
    """Generated model for FtMethodRegressionTypeLs"""

    ls: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "stored_value": "FT_LS",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class FtMethodRegressionTypeRls2Config(DakotaBaseModel):
    """Generated model for FtMethodRegressionTypeRls2Config"""

    l2_penalty: DakotaFloat = DakotaField(
        description="Penalty value applied in regularized regression solver for function train approximations",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_penalty",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class FtMethodRegressionCollocPoints(DakotaBaseModel):
    """Generated model for FtMethodRegressionCollocPoints"""

    collocation_points: int = DakotaField(
        description="Number of collocation points used to estimate expansion coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_points",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class FtMethodRegressionCollocRatio(DakotaBaseModel):
    """Generated model for FtMethodRegressionCollocRatio"""

    collocation_ratio: DakotaFloat = DakotaField(
        gt=0,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class FtMethodRegressionOptsTypeLs(DakotaBaseModel):
    """Generated model for FtMethodRegressionOptsTypeLs"""

    ls: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "stored_value": "FT_LS",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class FtMethodRegressionOptsTypeRls2Config(DakotaBaseModel):
    """Generated model for FtMethodRegressionOptsTypeRls2Config"""

    l2_penalty: DakotaFloat = DakotaField(
        description="Penalty value applied in regularized regression solver for function train approximations",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_penalty",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class FtMethodRegressionSequenceTypeLs(DakotaBaseModel):
    """Generated model for FtMethodRegressionSequenceTypeLs"""

    ls: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "stored_value": "FT_LS",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class FtMethodRegressionSequenceTypeRls2Config(DakotaBaseModel):
    """Generated model for FtMethodRegressionSequenceTypeRls2Config"""

    l2_penalty: DakotaFloat = DakotaField(
        description="Penalty value applied in regularized regression solver for function train approximations",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_penalty",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class FtModelOrderStartOrder(DakotaBaseModel):
    """Generated model for FtModelOrderStartOrder"""

    value: int = DakotaField(
        default=2,
        ge=0,
        description="(Initial) polynomial order of each univariate function within the functional tensor train.",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.c3function_train.start_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    dimension_preference: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="A set of weights specifying the realtive importance of each uncertain variable (dimension)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dimension_preference",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class FtModelRegressionTypeLs(DakotaBaseModel):
    """Generated model for FtModelRegressionTypeLs"""

    ls: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.regression_type",
                    "stored_value": "FT_LS",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class FtModelRegressionTypeRls2Config(DakotaBaseModel):
    """Generated model for FtModelRegressionTypeRls2Config"""

    l2_penalty: DakotaFloat = DakotaField(
        description="Penalty value applied in regularized regression solver for function train approximations",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.regression_penalty",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class FtModelRegressionOptsTypeLs(DakotaBaseModel):
    """Generated model for FtModelRegressionOptsTypeLs"""

    ls: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.regression_type",
                    "stored_value": "FT_LS",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class FtModelRegressionOptsTypeRls2Config(DakotaBaseModel):
    """Generated model for FtModelRegressionOptsTypeRls2Config"""

    l2_penalty: DakotaFloat = DakotaField(
        description="Penalty value applied in regularized regression solver for function train approximations",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.regression_penalty",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class MlftAllocControlAllocationControlEstimatorVarianceConfig(DakotaBaseModel):
    """Generated model for MlftAllocControlAllocationControlEstimatorVarianceConfig"""

    estimator_rate: DakotaFloat = DakotaField(
        default=2.0,
        description="Rate of convergence of mean estimator within multilevel polynomial chaos",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.multilevel_estimator_rate",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class RankSampling(DakotaBaseModel):
    """Generated model for RankSampling"""

    rank_sampling: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.multilevel_allocation_control",
                    "stored_value": "RANK_SAMPLING",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class StartOrderSequence(DakotaBaseModel):
    """Generated model for StartOrderSequence"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="mlftmethodordersequencestartordersequence", list_field="sequence"
        ),
    ]

    sequence: list[int] = DakotaField(
        description="Sequence of start orders used in a multi-stage expansion",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.start_order_sequence",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "UShortArray",
                }
            ]
        },
    )
    dimension_preference: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="A set of weights specifying the realtive importance of each uncertain variable (dimension)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dimension_preference",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class FtMethodOrderMixin(DakotaBaseModel):
    """Generated model for FtMethodOrderMixin"""

    start_order: FtMethodOrderStartOrder | None = DakotaField(
        default=None,
        description="(Initial) polynomial order of each univariate function within the functional tensor train.",
        dakota={"argument": "value", "aliases": ["order"]},
    )
    adapt_order: Literal[True] | None = DakotaField(
        default=None,
        description="Activate adaptive procedure for determining the best basis order",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.adapt_order",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    kick_order: int = DakotaField(
        default=1,
        gt=0,
        description="increment used when adapting the basis order in function train methods",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.kick_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    max_order: int = DakotaField(
        default=USHRT_MAX,
        ge=0,
        description="Maximum polynomial order of each univariate function within the functional tensor train.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.max_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    max_cv_order_candidates: int = DakotaField(
        default=USHRT_MAX,
        ge=0,
        description="Limit the number of cross-validation candidates for basis order",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.max_order_candidates",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class FtMethodRefinementPRefinement(DakotaBaseModel):
    """Generated model for FtMethodRefinementPRefinement"""

    uniform: Union[
        IncrementStartRank,
        IncrementStartOrder,
        IncrementMaxRank,
        IncrementMaxOrder,
        IncrementMaxRankOrder,
    ] = DakotaField(
        description="Refine an expansion uniformly in all dimensions.",
        dakota={
            "union_pattern": 4,
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_control",
                    "stored_value": "UNIFORM_CONTROL",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class FtMethodRegressionTypeRls2(DakotaBaseModel):
    """Generated model for FtMethodRegressionTypeRls2"""

    rls2: FtMethodRegressionTypeRls2Config = DakotaField(
        description="Use regularized regression solver for forming function train approximations",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "stored_value": "FT_RLS2",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class FtMethodRegressionOptsTypeRls2(DakotaBaseModel):
    """Generated model for FtMethodRegressionOptsTypeRls2"""

    rls2: FtMethodRegressionOptsTypeRls2Config = DakotaField(
        description="Use regularized regression solver for forming function train approximations",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "stored_value": "FT_RLS2",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class FtMethodRegressionSequenceTypeRls2(DakotaBaseModel):
    """Generated model for FtMethodRegressionSequenceTypeRls2"""

    rls2: FtMethodRegressionSequenceTypeRls2Config = DakotaField(
        description="Use regularized regression solver for forming function train approximations",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "stored_value": "FT_RLS2",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class FtModelOrderMixin(DakotaBaseModel):
    """Generated model for FtModelOrderMixin"""

    start_order: FtModelOrderStartOrder | None = DakotaField(
        default=None,
        description="(Initial) polynomial order of each univariate function within the functional tensor train.",
        dakota={"argument": "value", "aliases": ["order"]},
    )
    adapt_order: Literal[True] | None = DakotaField(
        default=None,
        description="Activate adaptive procedure for determining the best basis order",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.c3function_train.adapt_order",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    kick_order: int = DakotaField(
        default=1,
        gt=0,
        description="increment used when adapting the basis order in function train methods",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.c3function_train.kick_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    max_order: int = DakotaField(
        default=USHRT_MAX,
        ge=0,
        description="Maximum polynomial order of each univariate function within the functional tensor train.",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.c3function_train.max_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    max_cv_order_candidates: int = DakotaField(
        default=USHRT_MAX,
        ge=0,
        description="Limit the number of cross-validation candidates for basis order",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.c3function_train.cross_validation.max_order_candidates",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class FtModelRegressionTypeRls2(DakotaBaseModel):
    """Generated model for FtModelRegressionTypeRls2"""

    rls2: FtModelRegressionTypeRls2Config = DakotaField(
        description="Use regularized regression solver for forming function train approximations",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.regression_type",
                    "stored_value": "FT_RLS2",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class FtModelRegressionOptsTypeRls2(DakotaBaseModel):
    """Generated model for FtModelRegressionOptsTypeRls2"""

    rls2: FtModelRegressionOptsTypeRls2Config = DakotaField(
        description="Use regularized regression solver for forming function train approximations",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.regression_type",
                    "stored_value": "FT_RLS2",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MlftAllocControlAllocationControlEstimatorVariance(DakotaBaseModel):
    """Generated model for MlftAllocControlAllocationControlEstimatorVariance"""

    estimator_variance: MlftAllocControlAllocationControlEstimatorVarianceConfig = (
        DakotaField(
            description="Variance of mean estimator within multilevel polynomial chaos",
            dakota={
                "materialization": [
                    {
                        "ir_key": "method.nond.multilevel_allocation_control",
                        "stored_value": "ESTIMATOR_VARIANCE",
                        "storage_type": "PRESENCE_ENUM",
                        "ir_value_type": "short",
                    }
                ]
            },
        )
    )


class MlftMethodOrderSequenceMixin(DakotaBaseModel):
    """Generated model for MlftMethodOrderSequenceMixin"""

    start_order_sequence: StartOrderSequence | None = DakotaField(
        default=None,
        description="Sequence of start orders used in a multi-stage expansion",
        dakota={"argument": "sequence", "aliases": ["order_sequence"]},
    )
    adapt_order: Literal[True] | None = DakotaField(
        default=None,
        description="Activate adaptive procedure for determining the best basis order",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.adapt_order",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    kick_order: int = DakotaField(
        default=1,
        gt=0,
        description="increment used when adapting the basis order in function train methods",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.kick_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    max_order: int = DakotaField(
        default=USHRT_MAX,
        ge=0,
        description="Maximum polynomial order of each univariate function within the functional tensor train.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.max_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    max_cv_order_candidates: int = DakotaField(
        default=USHRT_MAX,
        ge=0,
        description="Limit the number of cross-validation candidates for basis order",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.max_order_candidates",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class FtMethodRefinementMixin(DakotaBaseModel):
    """Generated model for FtMethodRefinementMixin"""

    p_refinement: FtMethodRefinementPRefinement | None = DakotaField(
        default=None,
        description="Automatic polynomial order refinement",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_type",
                    "stored_value": "P_REFINEMENT",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )
    max_refinement_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Maximum number of expansion refinement iterations",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.max_refinement_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class FtMethodRegressionMixin(DakotaBaseModel):
    """Generated model for FtMethodRegressionMixin"""

    regression_type: (
        Union[FtMethodRegressionTypeLs, FtMethodRegressionTypeRls2] | None
    ) = DakotaField(
        default=None,
        description="Type of solver for forming function train approximations by regression",
        dakota={"union_pattern": 2},
    )
    max_solver_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Maximum iterations in determining polynomial coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.max_solver_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    max_cross_iterations: int = DakotaField(
        default=1,
        ge=0,
        description="Maximum number of iterations for cross-approximation during a rank adaptation.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.max_cross_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    solver_tolerance: DakotaFloat = DakotaField(
        default=1.0e-10,
        description="Convergence tolerance for the optimizer used during the regression solve.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.solver_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    response_scaling: Literal[True] | None = DakotaField(
        default=None,
        description="Perform bounds-scaling on response values prior to surrogate emulation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_scaling",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    tensor_grid: Literal[True] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.tensor_grid",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    collocation_control: Union[
        FtMethodRegressionCollocPoints, FtMethodRegressionCollocRatio
    ] = DakotaField(
        description="Collocation Control", dakota={"anchor": True, "union_pattern": 4}
    )


class FtMethodRegressionOptsMixin(DakotaBaseModel):
    """Generated model for FtMethodRegressionOptsMixin"""

    regression_type: (
        Union[FtMethodRegressionOptsTypeLs, FtMethodRegressionOptsTypeRls2] | None
    ) = DakotaField(
        default=None,
        description="Type of solver for forming function train approximations by regression",
        dakota={"union_pattern": 2},
    )
    max_solver_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Maximum iterations in determining polynomial coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.max_solver_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    max_cross_iterations: int = DakotaField(
        default=1,
        ge=0,
        description="Maximum number of iterations for cross-approximation during a rank adaptation.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.max_cross_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    solver_tolerance: DakotaFloat = DakotaField(
        default=1.0e-10,
        description="Convergence tolerance for the optimizer used during the regression solve.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.solver_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    response_scaling: Literal[True] | None = DakotaField(
        default=None,
        description="Perform bounds-scaling on response values prior to surrogate emulation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_scaling",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    tensor_grid: Literal[True] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.tensor_grid",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class FtMethodRegressionSequenceMixin(DakotaBaseModel):
    """Generated model for FtMethodRegressionSequenceMixin"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="ftmethodregressionsequencemixin",
            list_field="collocation_points_sequence",
        ),
    ]

    regression_type: (
        Union[FtMethodRegressionSequenceTypeLs, FtMethodRegressionSequenceTypeRls2]
        | None
    ) = DakotaField(
        default=None,
        description="Type of solver for forming function train approximations by regression",
        dakota={"union_pattern": 2},
    )
    max_solver_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Maximum iterations in determining polynomial coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.max_solver_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    max_cross_iterations: int = DakotaField(
        default=1,
        ge=0,
        description="Maximum number of iterations for cross-approximation during a rank adaptation.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.max_cross_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    solver_tolerance: DakotaFloat = DakotaField(
        default=1.0e-10,
        description="Convergence tolerance for the optimizer used during the regression solve.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.c3function_train.solver_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    response_scaling: Literal[True] | None = DakotaField(
        default=None,
        description="Perform bounds-scaling on response values prior to surrogate emulation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_scaling",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    tensor_grid: Literal[True] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.tensor_grid",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    collocation_points_sequence: list[int] | None = DakotaField(
        default=None,
        description="Sequence of collocation point counts used in a multi-stage expansion",
        dakota={
            "aliases": ["pilot_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_points_sequence",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "SizetArray",
                }
            ],
        },
    )
    collocation_ratio: DakotaFloat | None = DakotaField(
        default=None,
        gt=0,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class FtModelRegressionMixin(DakotaBaseModel):
    """Generated model for FtModelRegressionMixin"""

    regression_type: (
        Union[FtModelRegressionTypeLs, FtModelRegressionTypeRls2] | None
    ) = DakotaField(
        default=None,
        description="Type of solver for forming function train approximations by regression",
        dakota={"union_pattern": 2},
    )
    max_solver_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Maximum iterations in determining polynomial coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.max_solver_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    max_cross_iterations: int = DakotaField(
        default=1,
        ge=0,
        description="Maximum number of iterations for cross-approximation during a rank adaptation.",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.c3function_train.max_cross_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    solver_tolerance: DakotaFloat = DakotaField(
        default=1.0e-10,
        description="Convergence tolerance for the optimizer used during the regression solve.",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.c3function_train.solver_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    response_scaling: Literal[True] | None = DakotaField(
        default=None,
        description="Perform bounds-scaling on response values prior to surrogate emulation",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.response_scaling",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    tensor_grid: Literal[True] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.c3function_train.tensor_grid",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class FtModelRegressionOptsMixin(DakotaBaseModel):
    """Generated model for FtModelRegressionOptsMixin"""

    regression_type: (
        Union[FtModelRegressionOptsTypeLs, FtModelRegressionOptsTypeRls2] | None
    ) = DakotaField(
        default=None,
        description="Type of solver for forming function train approximations by regression",
        dakota={"union_pattern": 2},
    )
    max_solver_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Maximum iterations in determining polynomial coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.max_solver_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    max_cross_iterations: int = DakotaField(
        default=1,
        ge=0,
        description="Maximum number of iterations for cross-approximation during a rank adaptation.",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.c3function_train.max_cross_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    solver_tolerance: DakotaFloat = DakotaField(
        default=1.0e-10,
        description="Convergence tolerance for the optimizer used during the regression solve.",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.c3function_train.solver_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    response_scaling: Literal[True] | None = DakotaField(
        default=None,
        description="Perform bounds-scaling on response values prior to surrogate emulation",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.response_scaling",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    tensor_grid: Literal[True] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.c3function_train.tensor_grid",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class MlftAllocControlMixin(DakotaBaseModel):
    """Generated model for MlftAllocControlMixin"""

    allocation_control: (
        Union[MlftAllocControlAllocationControlEstimatorVariance, RankSampling] | None
    ) = DakotaField(
        default=None,
        description="Sample allocation approach for multilevel expansions",
        dakota={"union_pattern": 2},
    )
