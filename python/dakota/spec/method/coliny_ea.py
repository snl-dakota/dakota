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
    "Create random initial solutions"

    simple_random: Literal[True] = DakotaField(
        default=True,
        description="Create random initial solutions",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.initialization_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "random",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ColinyEaInitializationTypeUniqueRandom(DakotaBaseModel):
    "Create random initial solutions, but enforce uniqueness (default)"

    unique_random: Literal[True] = DakotaField(
        default=True,
        description="Create random initial solutions, but enforce uniqueness (default)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.initialization_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "unique_random",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ColinyEaInitializationTypeFlatFile(DakotaBaseModel):
    "Read initial solutions from file"

    flat_file: str = DakotaField(
        description="Read initial solutions from file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.initialization_type",
                    "storage_type": "TYPE_DATA_COMBINED",
                    "stored_value": "flat_file",
                    "secondary_ir_key": "method.flat_file",
                    "secondary_ir_value_type": "String",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class LinearRank(DakotaBaseModel):
    "Set selection scaling"

    linear_rank: Literal[True] = DakotaField(
        default=True,
        description="Set selection scaling",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.fitness_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "linear_rank",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MeritFunction(DakotaBaseModel):
    "Balance goals of reducing objective function and satisfying constraints"

    merit_function: Literal[True] = DakotaField(
        default=True,
        description="Balance goals of reducing objective function and satisfying constraints",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.fitness_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "proportional",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ReplacementTypeRandom(DakotaBaseModel):
    "Create new population randomly"

    random: int = DakotaField(
        description="Create new population randomly",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.replacement_type",
                    "storage_type": "TYPE_DATA_COMBINED",
                    "stored_value": "random",
                    "secondary_ir_key": "method.coliny.number_retained",
                    "secondary_ir_value_type": "int",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Chc(DakotaBaseModel):
    "Create new population using replacement"

    chc: int = DakotaField(
        description="Create new population using replacement",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.replacement_type",
                    "storage_type": "TYPE_DATA_COMBINED",
                    "stored_value": "chc",
                    "secondary_ir_key": "method.coliny.number_retained",
                    "secondary_ir_value_type": "int",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ColinyEaReplacementTypeElitist(DakotaBaseModel):
    "Use the best designs to form a new population"

    elitist: int = DakotaField(
        description="Use the best designs to form a new population",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.replacement_type",
                    "storage_type": "TYPE_DATA_COMBINED",
                    "stored_value": "elitist",
                    "secondary_ir_key": "method.coliny.number_retained",
                    "secondary_ir_value_type": "int",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class TwoPoint(DakotaBaseModel):
    "Combine middle of one parent with end of another"

    two_point: Literal[True] = DakotaField(
        default=True,
        description="Combine middle of one parent with end of another",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.crossover_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "two_point",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Blend(DakotaBaseModel):
    "Random blend of parents"

    blend: Literal[True] = DakotaField(
        default=True,
        description="Random blend of parents",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.crossover_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "blend",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class CrossoverTypeUniform(DakotaBaseModel):
    "Randomly combine coordinates from parents"

    uniform: Literal[True] = DakotaField(
        default=True,
        description="Randomly combine coordinates from parents",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.crossover_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "uniform",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ColinyEaMutationTypeReplaceUniform(DakotaBaseModel):
    "Replace coordinate with randomly generated value"

    replace_uniform: Literal[True] = DakotaField(
        default=True,
        description="Replace coordinate with randomly generated value",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.mutation_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "replace_uniform",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ColinyEaMutationTypeOffsetNormal(DakotaBaseModel):
    "Set mutation offset to use a normal distribution"

    offset_normal: ColinyMutationScaleRangeMixin = DakotaField(
        description="Set mutation offset to use a normal distribution",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.mutation_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "offset_normal",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ColinyEaMutationTypeOffsetCauchy(DakotaBaseModel):
    "Use a Cauchy distribution for the mutation offset"

    offset_cauchy: ColinyMutationScaleRangeMixin = DakotaField(
        description="Use a Cauchy distribution for the mutation offset",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.mutation_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "offset_cauchy",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ColinyEaMutationTypeOffsetUniform(DakotaBaseModel):
    "Set mutation offset to use a uniform distribution"

    offset_uniform: ColinyMutationScaleRangeMixin = DakotaField(
        description="Set mutation offset to use a uniform distribution",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.mutation_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "offset_uniform",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ColinyEaMutationType(DakotaBaseModel):
    "Select a mutation type"

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
    "Evolutionary Algorithm"

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
    "Generated model for ColinyEaSelection"

    coliny_ea: ColinyEaConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "COLINY_EA",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
