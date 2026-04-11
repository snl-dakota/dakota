"""Generated Pydantic models for method.coliny_ea"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField, DakotaFloat
from .base import MethodSelection
from typing import Literal, Union

# Cross-module model imports
from dakota.spec.shared.misc import (
    ColinyCommonOptsMixin,
    ColinyMutationScaleRangeMixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
)


class ColinyEaInitializationTypeSimpleRandom(DakotaBaseModel):
    """Generated model for ColinyEaInitializationTypeSimpleRandom"""

    simple_random: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.initialization_type",
                    "stored_value": "random",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ColinyEaInitializationTypeUniqueRandom(DakotaBaseModel):
    """Generated model for ColinyEaInitializationTypeUniqueRandom"""

    unique_random: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.initialization_type",
                    "stored_value": "unique_random",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ColinyEaInitializationTypeFlatFile(DakotaBaseModel):
    """Generated model for ColinyEaInitializationTypeFlatFile"""

    flat_file: str = DakotaField(
        description="Read initial solutions from file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.initialization_type",
                    "stored_value": "flat_file",
                    "storage_type": "TYPE_DATA_COMBINED",
                    "secondary_ir_key": "method.flat_file",
                    "secondary_ir_value_type": "String",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class LinearRank(DakotaBaseModel):
    """Generated model for LinearRank"""

    linear_rank: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.fitness_type",
                    "stored_value": "linear_rank",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MeritFunction(DakotaBaseModel):
    """Generated model for MeritFunction"""

    merit_function: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.fitness_type",
                    "stored_value": "proportional",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ReplacementTypeRandom(DakotaBaseModel):
    """Generated model for ReplacementTypeRandom"""

    random: int = DakotaField(
        description="Create new population randomly",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.replacement_type",
                    "stored_value": "random",
                    "storage_type": "TYPE_DATA_COMBINED",
                    "secondary_ir_key": "method.coliny.number_retained",
                    "secondary_ir_value_type": "int",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Chc(DakotaBaseModel):
    """Generated model for Chc"""

    chc: int = DakotaField(
        description="Create new population using replacement",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.replacement_type",
                    "stored_value": "chc",
                    "storage_type": "TYPE_DATA_COMBINED",
                    "secondary_ir_key": "method.coliny.number_retained",
                    "secondary_ir_value_type": "int",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ColinyEaReplacementTypeElitist(DakotaBaseModel):
    """Generated model for ColinyEaReplacementTypeElitist"""

    elitist: int = DakotaField(
        description="Use the best designs to form a new population",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.replacement_type",
                    "stored_value": "elitist",
                    "storage_type": "TYPE_DATA_COMBINED",
                    "secondary_ir_key": "method.coliny.number_retained",
                    "secondary_ir_value_type": "int",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class TwoPoint(DakotaBaseModel):
    """Generated model for TwoPoint"""

    two_point: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.crossover_type",
                    "stored_value": "two_point",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Blend(DakotaBaseModel):
    """Generated model for Blend"""

    blend: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.crossover_type",
                    "stored_value": "blend",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class CrossoverTypeUniform(DakotaBaseModel):
    """Generated model for CrossoverTypeUniform"""

    uniform: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.crossover_type",
                    "stored_value": "uniform",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ColinyEaMutationTypeReplaceUniform(DakotaBaseModel):
    """Generated model for ColinyEaMutationTypeReplaceUniform"""

    replace_uniform: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.mutation_type",
                    "stored_value": "replace_uniform",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ColinyEaMutationTypeOffsetNormal(DakotaBaseModel):
    """Generated model for ColinyEaMutationTypeOffsetNormal"""

    offset_normal: ColinyMutationScaleRangeMixin = DakotaField(
        description="Set mutation offset to use a normal distribution",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.mutation_type",
                    "stored_value": "offset_normal",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ColinyEaMutationTypeOffsetCauchy(DakotaBaseModel):
    """Generated model for ColinyEaMutationTypeOffsetCauchy"""

    offset_cauchy: ColinyMutationScaleRangeMixin = DakotaField(
        description="Use a Cauchy distribution for the mutation offset",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.mutation_type",
                    "stored_value": "offset_cauchy",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ColinyEaMutationTypeOffsetUniform(DakotaBaseModel):
    """Generated model for ColinyEaMutationTypeOffsetUniform"""

    offset_uniform: ColinyMutationScaleRangeMixin = DakotaField(
        description="Set mutation offset to use a uniform distribution",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.mutation_type",
                    "stored_value": "offset_uniform",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ColinyEaMutationType(DakotaBaseModel):
    """Generated model for ColinyEaMutationType"""

    strategy: Union[
        ColinyEaMutationTypeReplaceUniform,
        ColinyEaMutationTypeOffsetNormal,
        ColinyEaMutationTypeOffsetCauchy,
        ColinyEaMutationTypeOffsetUniform,
    ] = DakotaField(
        description="Mutation Type", dakota={"anchor": True, "union_pattern": 4}
    )


class ColinyEaConfig(
    MethodThreeOptionalKeywordsMixin,
    ColinyCommonOptsMixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for ColinyEaConfig"""

    population_size: int = DakotaField(
        default=50,
        gt=0,
        description="Set the population size",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.population_size",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    initialization_type: Union[
        ColinyEaInitializationTypeSimpleRandom,
        ColinyEaInitializationTypeUniqueRandom,
        ColinyEaInitializationTypeFlatFile,
    ] = DakotaField(
        default_factory=ColinyEaInitializationTypeUniqueRandom,
        description="Specify how to initialize the population",
        dakota={
            "union_pattern": 1,
            "model_default": "ColinyEaInitializationTypeUniqueRandom",
        },
    )
    fitness_type: Union[LinearRank, MeritFunction] | None = DakotaField(
        default=None, description="Select fitness type", dakota={"union_pattern": 2}
    )
    replacement_type: (
        Union[ReplacementTypeRandom, Chc, ColinyEaReplacementTypeElitist] | None
    ) = DakotaField(
        default=None,
        description="Select a replacement type for SCOLIB evolutionary algorithm ( ``coliny_ea``)",
        dakota={"union_pattern": 2},
    )
    new_solutions_generated: int = DakotaField(
        default=-9999,
        description="Replace population with individuals chosen from population",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.new_solutions_generated",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    crossover_rate: DakotaFloat = DakotaField(
        default=-1.0,
        description="Specify the probability of a crossover event",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.crossover_rate",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    crossover_type: Union[TwoPoint, Blend, CrossoverTypeUniform] | None = DakotaField(
        default=None, description="Select a crossover type", dakota={"union_pattern": 2}
    )
    mutation_rate: DakotaFloat = DakotaField(
        default=1.0,
        description="Set probability of a mutation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.mutation_rate",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    mutation_type: ColinyEaMutationType | None = DakotaField(
        default=None, description="Select a mutation type"
    )
    non_adaptive: Literal[True] | None = DakotaField(
        default=None,
        description="Disable self-adaptive mutation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.mutation_adaptive",
                    "storage_type": "PRESENCE_FALSE",
                    "ir_value_type": "bool",
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


class ColinyEaSelection(MethodSelection):
    """Generated model for ColinyEaSelection"""

    coliny_ea: ColinyEaConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "COLINY_EA",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
