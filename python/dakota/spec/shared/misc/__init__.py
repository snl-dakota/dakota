"""Generated Pydantic models for shared.misc"""

from __future__ import annotations

from ...base import DakotaBaseModel, DakotaField, DakotaFloat
from typing import Literal, Union
from ...base import DBL_MAX, SZ_MAX, USHRT_MAX
from typing import ClassVar, List
from ...validation import ValidationRule
from ...validation.rules import (
    CheckNonnegativeList,
    CheckProbabilityList,
    CheckSumEqualsLength,
    CompareLength,
    CompareLengthOne,
    TrustRegionValidate,
)

# Cross-module model imports
from dakota.spec.shared.core import (
    DefaultConstraintTolMixin,
    MethodConvergenceTolMixin,
    ResponseLevelsComputeProbRelGenMixin,
)
from dakota.spec.shared.responses import (
    ResponseLevelsComputeProbGenContext1Mixin,
    ResponseLevelsComputeProbGenContext2Mixin,
)


class ColinyCommonOptsMixin(DakotaBaseModel):
    "Generated model for ColinyCommonOptsMixin"

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
    show_misc_options: Literal[True] | None = DakotaField(
        default=None,
        description="Show algorithm parameters not exposed in Dakota input",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.show_misc_options",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    misc_options: list[str] | None = DakotaField(
        default=None,
        description="Set method options not available through Dakota spec",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.misc_options",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )
    max_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Number of iterations allowed for optimizers and adaptive UQ methods",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.max_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    convergence_tolerance: DakotaFloat = DakotaField(
        default=-DBL_MAX,
        description="Stopping criterion based on objective function or statistics convergence",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.convergence_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
                {
                    "ir_key": "method.jega.percent_change",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
            ]
        },
    )
    max_function_evaluations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Number of function evaluations allowed for optimizers",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.max_function_evaluations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    scaling: Literal[True] | None = DakotaField(
        default=None,
        description="Turn on scaling for variables, responses, and constraints",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.scaling",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class ColinyMutationScaleRangeMixin(DakotaBaseModel):
    "Generated model for ColinyMutationScaleRangeMixin"

    mutation_scale: DakotaFloat = DakotaField(
        default=0.1,
        description="Scales mutation across range of parameter",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.mutation_scale",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    mutation_range: int = DakotaField(
        default=-9999,
        description="Set uniform offset control for discrete parameters",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.mutation_range",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )


class DefaultScalingMixin(DakotaBaseModel):
    "Generated model for DefaultScalingMixin"

    scaling: Literal[True] | None = DakotaField(
        default=None,
        description="Turn on scaling for variables, responses, and constraints",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.scaling",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class DefaultSpeculativeMixin(DakotaBaseModel):
    "Generated model for DefaultSpeculativeMixin"

    speculative: Literal[True] | None = DakotaField(
        default=None,
        description="Compute speculative gradients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.speculative",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class MethodGaMutationScaleMixin(DakotaBaseModel):
    "Generated model for MethodGaMutationScaleMixin"

    mutation_scale: DakotaFloat = DakotaField(
        default=0.1,
        ge=0,
        le=1,
        description="Scales mutation across range of parameter",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.mutation_scale",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class MethodMaxFunctionEvaluationsContext1Mixin(DakotaBaseModel):
    "Generated model for MethodMaxFunctionEvaluationsContext1Mixin"

    max_function_evaluations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Number of function evaluations allowed for optimizers",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.max_function_evaluations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class MethodMaxFunctionEvaluationsContext2Mixin(DakotaBaseModel):
    "Generated model for MethodMaxFunctionEvaluationsContext2Mixin"

    max_function_evaluations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Stopping criterion based on maximum function evaluations",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.max_function_evaluations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class MethodMaxIterationsContext1Mixin(DakotaBaseModel):
    "Generated model for MethodMaxIterationsContext1Mixin"

    max_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Number of iterations allowed for optimizers and adaptive UQ methods",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.max_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class MethodMaxIterationsContext2Mixin(DakotaBaseModel):
    "Generated model for MethodMaxIterationsContext2Mixin"

    max_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Stopping criterion based on number of refinement iterations within the multilevel sample allocation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.max_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class MethodMaxRefinementIterationsMixin(DakotaBaseModel):
    "Generated model for MethodMaxRefinementIterationsMixin"

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


class MethodMaxSolverIterationsMixin(DakotaBaseModel):
    "Generated model for MethodMaxSolverIterationsMixin"

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


class MethodNpsolCommonOptsMixin(DakotaBaseModel):
    "Generated model for MethodNpsolCommonOptsMixin"

    verify_level: int = DakotaField(
        default=-1,
        description="Verify the quality of analytic gradients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.npsol.verify_level",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
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
    linesearch_tolerance: DakotaFloat = DakotaField(
        default=0.9,
        description="Choose how accurately the algorithm will compute the minimum in a line search",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.npsol.linesearch_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    convergence_tolerance: DakotaFloat = DakotaField(
        default=-DBL_MAX,
        description="Stopping criterion based on objective function or statistics convergence",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.convergence_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
                {
                    "ir_key": "method.jega.percent_change",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
            ]
        },
    )
    max_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Number of iterations allowed for optimizers and adaptive UQ methods",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.max_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    constraint_tolerance: DakotaFloat = DakotaField(
        default=0.0,
        description="Maximum allowable constraint violation still considered feasible",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.constraint_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    speculative: Literal[True] | None = DakotaField(
        default=None,
        description="Compute speculative gradients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.speculative",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    max_function_evaluations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Number of function evaluations allowed for optimizers",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.max_function_evaluations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    scaling: Literal[True] | None = DakotaField(
        default=None,
        description="Turn on scaling for variables, responses, and constraints",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.scaling",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    model_pointer: str | None = DakotaField(
        default=None,
        description="Identifier for model block to be used by a method",
        dakota={
            "block_pointer": "model",
            "materialization": [
                {
                    "ir_key": "method.model_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class MethodOptionalModelPointerMixin(DakotaBaseModel):
    "Generated model for MethodOptionalModelPointerMixin"

    model_pointer: str | None = DakotaField(
        default=None,
        description="Identifier for model block to be used by a method",
        dakota={
            "block_pointer": "model",
            "materialization": [
                {
                    "ir_key": "method.model_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class MethodOptppGradBasedOptsMixin(DakotaBaseModel):
    "Generated model for MethodOptppGradBasedOptsMixin"

    max_step: DakotaFloat = DakotaField(
        default=1.0e3,
        description="Max change in design point",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.optpp.max_step",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    gradient_tolerance: DakotaFloat = DakotaField(
        default=0.0001,
        description="Stopping critiera based on L2 norm of gradient",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.gradient_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    max_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Number of iterations allowed for optimizers and adaptive UQ methods",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.max_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    convergence_tolerance: DakotaFloat = DakotaField(
        default=-DBL_MAX,
        description="Stopping criterion based on objective function or statistics convergence",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.convergence_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
                {
                    "ir_key": "method.jega.percent_change",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
            ]
        },
    )
    speculative: Literal[True] | None = DakotaField(
        default=None,
        description="Compute speculative gradients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.speculative",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    max_function_evaluations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Number of function evaluations allowed for optimizers",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.max_function_evaluations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    scaling: Literal[True] | None = DakotaField(
        default=None,
        description="Turn on scaling for variables, responses, and constraints",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.scaling",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    model_pointer: str | None = DakotaField(
        default=None,
        description="Identifier for model block to be used by a method",
        dakota={
            "block_pointer": "model",
            "materialization": [
                {
                    "ir_key": "method.model_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class MethodSeedMixin(DakotaBaseModel):
    "Generated model for MethodSeedMixin"

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
    fixed_seed: Literal[True] | None = DakotaField(
        default=None,
        description="Reuses the same seed value for multiple random sampling sets",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.fixed_seed",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class MethodSeedSequenceMixin(DakotaBaseModel):
    "Generated model for MethodSeedSequenceMixin"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="methodseedsequencemixin", list_field="seed_sequence"
        ),
    ]

    seed_sequence: list[int] | None = DakotaField(
        default=None,
        description="Sequence of seed values for multi-stage random sampling",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.random_seed_sequence",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "SizetArray",
                }
            ]
        },
    )
    fixed_seed: Literal[True] | None = DakotaField(
        default=None,
        description="Reuses the same seed value for multiple random sampling sets",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.fixed_seed",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class MethodUseDerivativesMixin(DakotaBaseModel):
    "Generated model for MethodUseDerivativesMixin"

    use_derivatives: Literal[True] | None = DakotaField(
        default=None,
        description="Use derivative data to construct surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.derivative_usage",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class ModelFourOptionalKeywordsMixin(DakotaBaseModel):
    "Generated model for ModelFourOptionalKeywordsMixin"

    id_model: str | None = DakotaField(
        default=None,
        description="Give the model block an identifying name, in case of multiple model blocks",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.id",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    variables_pointer: str | None = DakotaField(
        default=None,
        description="Specify which variables block will be included with this model block",
        dakota={
            "block_pointer": "variables",
            "materialization": [
                {
                    "ir_key": "model.variables_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    responses_pointer: str | None = DakotaField(
        default=None,
        description="Specify which reponses block will be used by this model block",
        dakota={
            "block_pointer": "responses",
            "materialization": [
                {
                    "ir_key": "model.responses_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    hierarchical_tagging: Literal[True] | None = DakotaField(
        default=None,
        description="Enables hierarchical evaluation tagging",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.hierarchical_tags",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class DefaultFinalMomentsNoneKeyword(DakotaBaseModel):
    "Omit moments from the set of final statistics."

    none: Literal[True] = DakotaField(
        default=True,
        description="Omit moments from the set of final statistics.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.final_moments",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NO_MOMENTS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class DefaultFinalMomentsStandard(DakotaBaseModel):
    "Output standardized moments and include them within the set of final statistics."

    standard: Literal[True] = DakotaField(
        default=True,
        description="Output standardized moments and include them within the set of final statistics.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.final_moments",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "STANDARD_MOMENTS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class DefaultFinalMomentsCentral(DakotaBaseModel):
    "Output central moments and include them within the set of final statistics."

    central: Literal[True] = DakotaField(
        default=True,
        description="Output central moments and include them within the set of final statistics.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.final_moments",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "CENTRAL_MOMENTS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class DefaultTrustRegionContext1TrustRegion(DakotaBaseModel):
    "Specification group for trust region model management"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        TrustRegionValidate(
            context="trust_region",
            initial_size="initial_size",
            minimum_size="minimum_size",
            contract_threshold="contract_threshold",
            expand_threshold="expand_threshold",
            contraction_factor="contraction_factor",
            expansion_factor="expansion_factor",
        ),
    ]

    initial_size: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Trust region initial size (relative to bounds)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.trust_region.initial_size",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    minimum_size: DakotaFloat = DakotaField(
        default=1.0e-6,
        description="Trust region minimum size",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.trust_region.minimum_size",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    contract_threshold: DakotaFloat = DakotaField(
        default=0.25,
        description="Shrink trust region if trust region ratio is below this value",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.trust_region.contract_threshold",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    expand_threshold: DakotaFloat = DakotaField(
        default=0.75,
        description="Expand trust region if trust region ratio is above this value",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.trust_region.expand_threshold",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    contraction_factor: DakotaFloat = DakotaField(
        default=0.25,
        description="Amount by which step length is rescaled",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.trust_region.contraction_factor",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    expansion_factor: DakotaFloat = DakotaField(
        default=2.0,
        description="Trust region expansion factor",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.trust_region.expansion_factor",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class DefaultTrustRegionContext2TrustRegion(DakotaBaseModel):
    "Use trust region as the globalization strategy."

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        TrustRegionValidate(
            context="trust_region",
            initial_size="initial_size",
            minimum_size="minimum_size",
            contract_threshold="contract_threshold",
            expand_threshold="expand_threshold",
            contraction_factor="contraction_factor",
            expansion_factor="expansion_factor",
        ),
    ]

    initial_size: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Trust region initial size (relative to bounds)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.trust_region.initial_size",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    minimum_size: DakotaFloat = DakotaField(
        default=1.0e-6,
        description="Trust region minimum size",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.trust_region.minimum_size",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    contract_threshold: DakotaFloat = DakotaField(
        default=0.25,
        description="Shrink trust region if trust region ratio is below this value",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.trust_region.contract_threshold",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    expand_threshold: DakotaFloat = DakotaField(
        default=0.75,
        description="Expand trust region if trust region ratio is above this value",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.trust_region.expand_threshold",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    contraction_factor: DakotaFloat = DakotaField(
        default=0.25,
        description="Amount by which step length is rescaled",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.trust_region.contraction_factor",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    expansion_factor: DakotaFloat = DakotaField(
        default=2.0,
        description="Trust region expansion factor",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.trust_region.expansion_factor",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class MostSignificantBitFirst(DakotaBaseModel):
    "Assume integers are stored with most significant bit first"

    most_significant_bit_first: Literal[True] = DakotaField(
        default=True,
        description="Assume integers are stored with most significant bit first",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.most_significant_bit_first",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class LeastSignificantBitFirst(DakotaBaseModel):
    "Assume integers are stored with least significant bit first"

    least_significant_bit_first: Literal[True] = DakotaField(
        default=True,
        description="Assume integers are stored with least significant bit first",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.least_significant_bit_first",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class GeneratingMatricesInline(DakotaBaseModel):
    "Specify inline generating matrices"

    inline: list[int] = DakotaField(
        description="Specify inline generating matrices",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.generating_matrices.inline",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )


class GeneratingMatricesFile(DakotaBaseModel):
    "Specify generating matrices read from file"

    file: str = DakotaField(
        description="Specify generating matrices read from file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.generating_matrices.file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class JoeKuo(DakotaBaseModel):
    "Generating matrices that provide up to 2\\ :sup:`32` points in up to 250 dimensions"

    joe_kuo: Literal[True] = DakotaField(
        default=True,
        description="Generating matrices that provide up to 2\\ :sup:`32` points in up to 250 dimensions",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.ld.digitalnet.generating_matrix_scheme",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "JOE_KUO",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class SobolOrder2(DakotaBaseModel):
    "Generating matrices that provide up to 2\\ :sup:`32` points in up to 1024 dimensions"

    sobol_order_2: Literal[True] = DakotaField(
        default=True,
        description="Generating matrices that provide up to 2\\ :sup:`32` points in up to 1024 dimensions",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.ld.digitalnet.generating_matrix_scheme",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SOBOL_ORDER_2",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class DigitalNetOptionsOrderingNatural(DakotaBaseModel):
    "Natural ordering of the points of this digital net"

    natural: Literal[True] = DakotaField(
        default=True,
        description="Natural ordering of the points of this digital net",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.ld.digitalnet.ordering",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DIGITAL_NET_NATURAL_ORDERING",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class GrayCode(DakotaBaseModel):
    "Gray code ordering of the points of this digital net"

    gray_code: Literal[True] = DakotaField(
        default=True,
        description="Gray code ordering of the points of this digital net",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.ld.digitalnet.ordering",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DIGITAL_NET_GRAY_CODE_ORDERING",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class DiscrepEmulationDiscrepancyEmulationDistinct(DakotaBaseModel):
    "Distinct formulation for emulation of model discrepancies."

    distinct: Literal[True] = DakotaField(
        default=True,
        description="Distinct formulation for emulation of model discrepancies.",
        dakota={
            "aliases": ["paired"],
            "materialization": [
                {
                    "ir_key": "method.nond.multilevel_discrepancy_emulation",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DISTINCT_EMULATION",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class DiscrepEmulationDiscrepancyEmulationRecursive(DakotaBaseModel):
    "Recursive formulation for emulation of model discrepancies."

    recursive: Literal[True] = DakotaField(
        default=True,
        description="Recursive formulation for emulation of model discrepancies.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.multilevel_discrepancy_emulation",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RECURSIVE_EMULATION",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class DistributionCumulComplContext2Cumulative(DakotaBaseModel):
    "Placeholder for future capabilities"

    cumulative: Literal[True] = DakotaField(
        default=True,
        description="Placeholder for future capabilities",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.distribution",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "CUMULATIVE",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class DistributionCumulComplContext2Complementary(DakotaBaseModel):
    "Placeholder for future capabilities"

    complementary: Literal[True] = DakotaField(
        default=True,
        description="Placeholder for future capabilities",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.distribution",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "COMPLEMENTARY",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class DistributionCumulComplContext1Cumulative(DakotaBaseModel):
    "Computes statistics according to cumulative functions"

    cumulative: Literal[True] = DakotaField(
        default=True,
        description="Computes statistics according to cumulative functions",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.distribution",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "CUMULATIVE",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class DistributionCumulComplContext1Complementary(DakotaBaseModel):
    "Computes statistics according to complementary cumulative functions"

    complementary: Literal[True] = DakotaField(
        default=True,
        description="Computes statistics according to complementary cumulative functions",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.distribution",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "COMPLEMENTARY",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class EgoSubProblemSolverContext2GPSurfpackExportModelFormats(DakotaBaseModel):
    "Formats for surrogate model export"

    text_archive: Literal[True] | None = DakotaField(
        default=None,
        description="Surrogate model plain-text archive file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TEXT_ARCHIVE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    binary_archive: Literal[True] | None = DakotaField(
        default=None,
        description="Surrogate model binary archive file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "BINARY_ARCHIVE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class EgoSubProblemSolverContext2GPDakota(DakotaBaseModel):
    "Select the built in Gaussian Process surrogate"

    dakota: Literal[True] = DakotaField(
        default=True,
        description="Select the built in Gaussian Process surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "GP_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class EgoSubProblemSolverContext2ImportBuildPointsFileCustomAnnotatedConfig(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class EgoSubProblemSolverContext2ImportBuildPointsFileAnnotated(DakotaBaseModel):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class EgoSubProblemSolverContext2ImportBuildPointsFileFreeform(DakotaBaseModel):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class EgoSubProblemSolverContext2ExportApproxPointsFileCustomAnnotatedConfig(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class EgoSubProblemSolverContext2ExportApproxPointsFileAnnotated(DakotaBaseModel):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class EgoSubProblemSolverContext2ExportApproxPointsFileFreeform(DakotaBaseModel):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class EgoSubProblemSolverContext1GPSurfpackExportModelFormats(DakotaBaseModel):
    "export_model not active"

    text_archive: Literal[True] | None = DakotaField(
        default=None,
        description="export_model not active",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TEXT_ARCHIVE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    binary_archive: Literal[True] | None = DakotaField(
        default=None,
        description="export_model not active",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "BINARY_ARCHIVE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class EgoSubProblemSolverContext1GPDakota(DakotaBaseModel):
    "Select the built in Gaussian Process surrogate"

    dakota: Literal[True] = DakotaField(
        default=True,
        description="Select the built in Gaussian Process surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "GP_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class EgoSubProblemSolverContext1ImportBuildPointsFileCustomAnnotatedConfig(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class EgoSubProblemSolverContext1ImportBuildPointsFileAnnotated(DakotaBaseModel):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class EgoSubProblemSolverContext1ImportBuildPointsFileFreeform(DakotaBaseModel):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class EgoSubProblemSolverContext1ExportApproxPointsFileCustomAnnotatedConfig(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class EgoSubProblemSolverContext1ExportApproxPointsFileAnnotated(DakotaBaseModel):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class EgoSubProblemSolverContext1ExportApproxPointsFileFreeform(DakotaBaseModel):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class GenReliabilityLevelsGenReliabilityLevels(DakotaBaseModel):
    "Specify generalized relability levels at which to estimate the corresponding response value"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckSumEqualsLength(
            context="genreliabilitylevelsgenreliabilitylevels",
            num_list_field="num_gen_reliability_levels",
            levels_list_field="values",
        ),
    ]

    values: list[DakotaFloat] = DakotaField(
        description="Specify generalized relability levels at which to estimate the corresponding response value"
    )
    num_gen_reliability_levels: list[int] | None = DakotaField(
        default=None,
        description="Specify which ``gen_reliability_levels`` correspond to which response",
    )


class ImportBuildPointsFileCustomAnnotatedConfig(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ImportBuildPointsFileAnnotated(DakotaBaseModel):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ImportBuildPointsFileFreeform(DakotaBaseModel):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodCovTypeDiagCov(DakotaBaseModel):
    "Display only the diagonal terms of the covariance matrix"

    diagonal_covariance: Literal[True] = DakotaField(
        default=True,
        description="Display only the diagonal terms of the covariance matrix",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.covariance_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DIAGONAL_COVARIANCE",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodCovTypeFullCov(DakotaBaseModel):
    "Display the full covariance matrix"

    full_covariance: Literal[True] = DakotaField(
        default=True,
        description="Display the full covariance matrix",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.covariance_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "FULL_COVARIANCE",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodExportModelContext2Formats(DakotaBaseModel):
    "export_model not active"

    text_archive: Literal[True] | None = DakotaField(
        default=None,
        description="export_model not active",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TEXT_ARCHIVE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    binary_archive: Literal[True] | None = DakotaField(
        default=None,
        description="export_model not active",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "BINARY_ARCHIVE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodExportModelContext1Formats(DakotaBaseModel):
    "Formats for surrogate model export"

    text_archive: Literal[True] | None = DakotaField(
        default=None,
        description="Surrogate model plain-text archive file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TEXT_ARCHIVE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    binary_archive: Literal[True] | None = DakotaField(
        default=None,
        description="Surrogate model binary archive file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "BINARY_ARCHIVE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGPSurfpack(DakotaBaseModel):
    "surfpack"

    surfpack: Literal[True] = DakotaField(
        default=True,
        description="surfpack",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "KRIGING_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodGPDakota(DakotaBaseModel):
    "dakota"

    dakota: Literal[True] = DakotaField(
        default=True,
        description="dakota",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "GP_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext2SbgoGPSurfpack(DakotaBaseModel):
    "Use the Surfpack version of Gaussian Process surrogates"

    surfpack: Literal[True] = DakotaField(
        default=True,
        description="Use the Surfpack version of Gaussian Process surrogates",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "KRIGING_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext2SbgoGPDakota(DakotaBaseModel):
    "Select the built in Gaussian Process surrogate"

    dakota: Literal[True] = DakotaField(
        default=True,
        description="Select the built in Gaussian Process surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "GP_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext2SbgoGPExperimentalConfig(DakotaBaseModel):
    "Use the experimental Gaussian Process surrogate"

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


class MethodGlobalSubProblemSolverContext2SbgoImportBuildPointsFileCustomAnnotatedConfig(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext2SbgoImportBuildPointsFileAnnotated(
    DakotaBaseModel
):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext2SbgoImportBuildPointsFileFreeform(
    DakotaBaseModel
):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext2SbgoExportApproxPointsFileCustomAnnotatedConfig(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext2SbgoExportApproxPointsFileAnnotated(
    DakotaBaseModel
):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext2SbgoExportApproxPointsFileFreeform(
    DakotaBaseModel
):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext2EgoGPSurfpackExportModelFormats(
    DakotaBaseModel
):
    "Formats for surrogate model export"

    text_archive: Literal[True] | None = DakotaField(
        default=None,
        description="Surrogate model plain-text archive file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TEXT_ARCHIVE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    binary_archive: Literal[True] | None = DakotaField(
        default=None,
        description="Surrogate model binary archive file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "BINARY_ARCHIVE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext2EgoGPDakota(DakotaBaseModel):
    "Select the built in Gaussian Process surrogate"

    dakota: Literal[True] = DakotaField(
        default=True,
        description="Select the built in Gaussian Process surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "GP_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext2EgoGPExperimentalExportModelFormats(
    DakotaBaseModel
):
    "Formats for surrogate model export"

    text_archive: Literal[True] | None = DakotaField(
        default=None,
        description="Surrogate model plain-text archive file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TEXT_ARCHIVE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    binary_archive: Literal[True] | None = DakotaField(
        default=None,
        description="Surrogate model binary archive file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "BINARY_ARCHIVE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext2EgoImportBuildPointsFileCustomAnnotatedConfig(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext2EgoImportBuildPointsFileAnnotated(
    DakotaBaseModel
):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext2EgoImportBuildPointsFileFreeform(
    DakotaBaseModel
):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext2EgoExportApproxPointsFileCustomAnnotatedConfig(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext2EgoExportApproxPointsFileAnnotated(
    DakotaBaseModel
):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext2EgoExportApproxPointsFileFreeform(
    DakotaBaseModel
):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext2Ea(DakotaBaseModel):
    "Use an evolutionary algorithm"

    ea: Literal[True] = DakotaField(
        default=True,
        description="Use an evolutionary algorithm",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_EA",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext2Lhs(DakotaBaseModel):
    "Uses Latin Hypercube Sampling (LHS) to sample variables"

    lhs: Literal[True] = DakotaField(
        default=True,
        description="Uses Latin Hypercube Sampling (LHS) to sample variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_LHS",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext1SbgoGPSurfpack(DakotaBaseModel):
    "Use the Surfpack version of Gaussian Process surrogates"

    surfpack: Literal[True] = DakotaField(
        default=True,
        description="Use the Surfpack version of Gaussian Process surrogates",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "KRIGING_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext1SbgoGPDakota(DakotaBaseModel):
    "Select the built in Gaussian Process surrogate"

    dakota: Literal[True] = DakotaField(
        default=True,
        description="Select the built in Gaussian Process surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "GP_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext1SbgoGPExperimentalConfig(DakotaBaseModel):
    "Use the experimental Gaussian Process surrogate"

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


class MethodGlobalSubProblemSolverContext1SbgoImportBuildPointsFileCustomAnnotatedConfig(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext1SbgoImportBuildPointsFileAnnotated(
    DakotaBaseModel
):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext1SbgoImportBuildPointsFileFreeform(
    DakotaBaseModel
):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext1SbgoExportApproxPointsFileCustomAnnotatedConfig(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext1SbgoExportApproxPointsFileAnnotated(
    DakotaBaseModel
):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext1SbgoExportApproxPointsFileFreeform(
    DakotaBaseModel
):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext1EgoGPSurfpackExportModelFormats(
    DakotaBaseModel
):
    "export_model not active"

    text_archive: Literal[True] | None = DakotaField(
        default=None,
        description="export_model not active",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TEXT_ARCHIVE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    binary_archive: Literal[True] | None = DakotaField(
        default=None,
        description="export_model not active",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "BINARY_ARCHIVE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext1EgoGPDakota(DakotaBaseModel):
    "Select the built in Gaussian Process surrogate"

    dakota: Literal[True] = DakotaField(
        default=True,
        description="Select the built in Gaussian Process surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "GP_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext1EgoGPExperimentalExportModelFormats(
    DakotaBaseModel
):
    "export_model not active"

    text_archive: Literal[True] | None = DakotaField(
        default=None,
        description="export_model not active",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TEXT_ARCHIVE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    binary_archive: Literal[True] | None = DakotaField(
        default=None,
        description="export_model not active",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "BINARY_ARCHIVE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext1EgoImportBuildPointsFileCustomAnnotatedConfig(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext1EgoImportBuildPointsFileAnnotated(
    DakotaBaseModel
):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext1EgoImportBuildPointsFileFreeform(
    DakotaBaseModel
):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext1EgoExportApproxPointsFileCustomAnnotatedConfig(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext1EgoExportApproxPointsFileAnnotated(
    DakotaBaseModel
):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext1EgoExportApproxPointsFileFreeform(
    DakotaBaseModel
):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext1Ea(DakotaBaseModel):
    "Use an evolutionary algorithm"

    ea: Literal[True] = DakotaField(
        default=True,
        description="Use an evolutionary algorithm",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_EA",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext1Lhs(DakotaBaseModel):
    "Uses Latin Hypercube Sampling (LHS) to sample variables"

    lhs: Literal[True] = DakotaField(
        default=True,
        description="Uses Latin Hypercube Sampling (LHS) to sample variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_LHS",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGpAlternativesNoExportGPSurfpack(DakotaBaseModel):
    "Use the Surfpack version of Gaussian Process surrogates"

    surfpack: Literal[True] = DakotaField(
        default=True,
        description="Use the Surfpack version of Gaussian Process surrogates",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "KRIGING_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodGpAlternativesNoExportGPDakota(DakotaBaseModel):
    "Select the built in Gaussian Process surrogate"

    dakota: Literal[True] = DakotaField(
        default=True,
        description="Select the built in Gaussian Process surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "GP_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodGpAlternativesNoExportGPExperimentalConfig(DakotaBaseModel):
    "Use the experimental Gaussian Process surrogate"

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


class MethodGpAlternativesWithExportContext2GPSurfpackExportModelFormats(
    DakotaBaseModel
):
    "Formats for surrogate model export"

    text_archive: Literal[True] | None = DakotaField(
        default=None,
        description="Surrogate model plain-text archive file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TEXT_ARCHIVE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    binary_archive: Literal[True] | None = DakotaField(
        default=None,
        description="Surrogate model binary archive file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "BINARY_ARCHIVE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGpAlternativesWithExportContext2GPDakota(DakotaBaseModel):
    "Select the built in Gaussian Process surrogate"

    dakota: Literal[True] = DakotaField(
        default=True,
        description="Select the built in Gaussian Process surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "GP_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodGpAlternativesWithExportContext1GPSurfpackExportModelFormats(
    DakotaBaseModel
):
    "export_model not active"

    text_archive: Literal[True] | None = DakotaField(
        default=None,
        description="export_model not active",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TEXT_ARCHIVE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    binary_archive: Literal[True] | None = DakotaField(
        default=None,
        description="export_model not active",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "BINARY_ARCHIVE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGpAlternativesWithExportContext1GPDakota(DakotaBaseModel):
    "Select the built in Gaussian Process surrogate"

    dakota: Literal[True] = DakotaField(
        default=True,
        description="Select the built in Gaussian Process surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "GP_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodGradientSubProblemSolverSqp(DakotaBaseModel):
    "Use a sequential quadratic programming method for solving an optimization sub-problem"

    sqp: Literal[True] = DakotaField(
        default=True,
        description="Use a sequential quadratic programming method for solving an optimization sub-problem",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_NPSOL",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGradientSubProblemSolverNip(DakotaBaseModel):
    "Use a nonlinear interior point method for solving an optimization sub-problem"

    nip: Literal[True] = DakotaField(
        default=True,
        description="Use a nonlinear interior point method for solving an optimization sub-problem",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_OPTPP",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodIteratorServerSchedulingDedicated(DakotaBaseModel):
    "Specify a dedicated scheduler partition for concurrent iterator executions"

    dedicated: Literal[True] = DakotaField(
        default=True,
        description="Specify a dedicated scheduler partition for concurrent iterator executions",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.iterator_scheduling",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DEDICATED_SCHEDULER_DYNAMIC",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodIteratorServerSchedulingPeer(DakotaBaseModel):
    "Specify a peer partition for parallel iterator scheduling"

    peer: Literal[True] = DakotaField(
        default=True,
        description="Specify a peer partition for parallel iterator scheduling",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.iterator_scheduling",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "PEER_SCHEDULING",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodJegaCommonOptsInitializationTypeSimpleRandom(DakotaBaseModel):
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


class MethodJegaCommonOptsInitializationTypeUniqueRandom(DakotaBaseModel):
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


class MethodJegaCommonOptsInitializationTypeFlatFile(DakotaBaseModel):
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


class MultiPointBinary(DakotaBaseModel):
    "Use bit switching for crossover events"

    multi_point_binary: int = DakotaField(
        default=2,
        gt=0,
        description="Use bit switching for crossover events",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.crossover_type",
                    "storage_type": "TYPE_DATA_COMBINED",
                    "stored_value": "multi_point_binary",
                    "secondary_ir_key": "method.jega.num_cross_points",
                    "secondary_ir_value_type": "size_t",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MultiPointParameterizedBinary(DakotaBaseModel):
    "Use bit switching to crossover each design variable"

    multi_point_parameterized_binary: int = DakotaField(
        default=2,
        gt=0,
        description="Use bit switching to crossover each design variable",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.crossover_type",
                    "storage_type": "TYPE_DATA_COMBINED",
                    "stored_value": "multi_point_parameterized_binary",
                    "secondary_ir_key": "method.jega.num_cross_points",
                    "secondary_ir_value_type": "size_t",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MultiPointReal(DakotaBaseModel):
    "Perform crossover in real valued genome"

    multi_point_real: int = DakotaField(
        default=2,
        gt=0,
        description="Perform crossover in real valued genome",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.crossover_type",
                    "storage_type": "TYPE_DATA_COMBINED",
                    "stored_value": "multi_point_real",
                    "secondary_ir_key": "method.jega.num_cross_points",
                    "secondary_ir_value_type": "size_t",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ShuffleRandomConfig(DakotaBaseModel):
    "Perform crossover by choosing design variable(s)"

    num_parents: int = DakotaField(
        default=2,
        gt=0,
        description="Number of parents in random shuffle crossover",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.jega.num_parents",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    num_offspring: int = DakotaField(
        default=2,
        gt=0,
        description="Number of offspring in random shuffle crossover",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.jega.num_offspring",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class BitRandom(DakotaBaseModel):
    "Mutate by flipping a random bit"

    bit_random: Literal[True] = DakotaField(
        default=True,
        description="Mutate by flipping a random bit",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.mutation_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "bit_random",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MethodJegaCommonOptsMutationTypeReplaceUniform(DakotaBaseModel):
    "Use uniformly distributed value over range of parameter"

    replace_uniform: Literal[True] = DakotaField(
        default=True,
        description="Use uniformly distributed value over range of parameter",
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


class OffsetNormalConfig(DakotaBaseModel):
    "Set mutation offset to use a normal distribution"

    mutation_scale: DakotaFloat = DakotaField(
        default=0.1,
        ge=0,
        le=1,
        description="Scales mutation across range of parameter",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.mutation_scale",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class MethodMlmfSolverMetricAverageEstimatorVariance(DakotaBaseModel):
    "Utilize the estimator variance averaged over the QoI as the solver metric for sampling-based multifidelity methods."

    average_estimator_variance: Literal[True] = DakotaField(
        default=True,
        description="Utilize the estimator variance averaged over the QoI as the solver metric for sampling-based multifidelity methods.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.estimator_variance_metric",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "AVG_ESTVAR_METRIC",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodMlmfSolverMetricNormEstimatorVarianceConfig(DakotaBaseModel):
    "Utilize a p-norm over the vector of QoI estimator variances as the solver metric for sampling-based multifidelity methods."

    norm_order: DakotaFloat = DakotaField(
        default=2.0,
        ge=1.0,
        description="Utilize the response covariance metric for guiding adaptive refinement during UQ.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.estimator_variance_metric_norm_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class MethodMlmfSolverMetricMaxEstimatorVariance(DakotaBaseModel):
    "Utilize the maximum estimator variance as the solver metric for sampling-based multifidelity methods."

    max_estimator_variance: Literal[True] = DakotaField(
        default=True,
        description="Utilize the maximum estimator variance as the solver metric for sampling-based multifidelity methods.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.estimator_variance_metric",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MAX_ESTVAR_METRIC",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodMlmfSubProblemSolverSqp(DakotaBaseModel):
    "Use a sequential quadratic programming method for solving an optimization sub-problem"

    sqp: Literal[True] = DakotaField(
        default=True,
        description="Use a sequential quadratic programming method for solving an optimization sub-problem",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_NPSOL",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodMlmfSubProblemSolverNip(DakotaBaseModel):
    "Use a nonlinear interior point method for solving an optimization sub-problem"

    nip: Literal[True] = DakotaField(
        default=True,
        description="Use a nonlinear interior point method for solving an optimization sub-problem",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_OPTPP",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodMlmfSubProblemSolverGlobalLocal(DakotaBaseModel):
    "Use a hybrid global-local scheme for solving an optimization sub-problem"

    global_local: Literal[True] = DakotaField(
        default=True,
        description="Use a hybrid global-local scheme for solving an optimization sub-problem",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_DIRECT_NPSOL_OPTPP",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodMlmfSubProblemSolverCompetedLocal(DakotaBaseModel):
    "Use a competed local solver scheme for solving an optimization sub-problem"

    competed_local: Literal[True] = DakotaField(
        default=True,
        description="Use a competed local solver scheme for solving an optimization sub-problem",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_NPSOL_OPTPP",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ValueBasedLineSearch(DakotaBaseModel):
    "Use only function values for line search"

    value_based_line_search: Literal[True] = DakotaField(
        default=True,
        description="Use only function values for line search",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.optpp.search_method",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "value_based_line_search",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class GradientBasedLineSearch(DakotaBaseModel):
    "Set the search method to use the gradient"

    gradient_based_line_search: Literal[True] = DakotaField(
        default=True,
        description="Set the search method to use the gradient",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.optpp.search_method",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "gradient_based_line_search",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class SearchMethodTrustRegion(DakotaBaseModel):
    "Use trust region as the globalization strategy."

    trust_region: Literal[True] = DakotaField(
        default=True,
        description="Use trust region as the globalization strategy.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.optpp.search_method",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "trust_region",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class TrPds(DakotaBaseModel):
    "Use direct search as the local search in a trust region method"

    tr_pds: Literal[True] = DakotaField(
        default=True,
        description="Use direct search as the local search in a trust region method",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.optpp.search_method",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "tr_pds",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ElBakry(DakotaBaseModel):
    "El-Bakry merit function"

    el_bakry: Literal[True] = DakotaField(
        default=True,
        description="El-Bakry merit function",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.optpp.merit_function",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NormFmu",
                    "enum_scope": "OPTPP",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ArgaezTapia(DakotaBaseModel):
    "The merit function by Tapia and Argaez"

    argaez_tapia: Literal[True] = DakotaField(
        default=True,
        description="The merit function by Tapia and Argaez",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.optpp.merit_function",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ArgaezTapia",
                    "enum_scope": "OPTPP",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class VanShanno(DakotaBaseModel):
    "The merit function by Vanderbei and Shanno"

    van_shanno: Literal[True] = DakotaField(
        default=True,
        description="The merit function by Vanderbei and Shanno",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.optpp.merit_function",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "VanShanno",
                    "enum_scope": "OPTPP",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class Blocking(DakotaBaseModel):
    "Complete concurrent evaluation of all evaluations within a batch"

    blocking: Literal[True] = DakotaField(
        default=True,
        description="Complete concurrent evaluation of all evaluations within a batch",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.synchronization",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BLOCKING_SYNCHRONIZATION",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class Nonblocking(DakotaBaseModel):
    "A batch of concurrent evaluations are queried for completions and a partial set is returned to the algorithm"

    nonblocking: Literal[True] = DakotaField(
        default=True,
        description="A batch of concurrent evaluations are queried for completions and a partial set is returned to the algorithm",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.synchronization",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NONBLOCKING_SYNCHRONIZATION",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class Debug(DakotaBaseModel):
    "Level 5 of 5 - maximum"

    debug: Literal[True] = DakotaField(
        default=True,
        description="Level 5 of 5 - maximum",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.output",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DEBUG_OUTPUT",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class Verbose(DakotaBaseModel):
    "Level 4 of 5 - more than normal"

    verbose: Literal[True] = DakotaField(
        default=True,
        description="Level 4 of 5 - more than normal",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.output",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "VERBOSE_OUTPUT",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class Normal(DakotaBaseModel):
    "Level 3 of 5 - default"

    normal: Literal[True] = DakotaField(
        default=True,
        description="Level 3 of 5 - default",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.output",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NORMAL_OUTPUT",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class Quiet(DakotaBaseModel):
    "Level 2 of 5 - less than normal"

    quiet: Literal[True] = DakotaField(
        default=True,
        description="Level 2 of 5 - less than normal",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.output",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "QUIET_OUTPUT",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class Silent(DakotaBaseModel):
    "Level 1 of 5 - minimum"

    silent: Literal[True] = DakotaField(
        default=True,
        description="Level 1 of 5 - minimum",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.output",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SILENT_OUTPUT",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfAllocControlContext2AllocationControlGreedy(DakotaBaseModel):
    "Sample allocation based on greedy refinement within multifidelity polynomial chaos"

    greedy: Literal[True] = DakotaField(
        default=True,
        description="Sample allocation based on greedy refinement within multifidelity polynomial chaos",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.multilevel_allocation_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "GREEDY_REFINEMENT",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfAllocControlContext1AllocationControlGreedy(DakotaBaseModel):
    "Sample allocation based on greedy refinement within multifidelity stochastic collocation"

    greedy: Literal[True] = DakotaField(
        default=True,
        description="Sample allocation based on greedy refinement within multifidelity stochastic collocation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.multilevel_allocation_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "GREEDY_REFINEMENT",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfAllocControlContext3AllocationControlGreedy(DakotaBaseModel):
    "Sample allocation based on greedy refinement within multifidelity function train"

    greedy: Literal[True] = DakotaField(
        default=True,
        description="Sample allocation based on greedy refinement within multifidelity function train",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.multilevel_allocation_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "GREEDY_REFINEMENT",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MlmfFinalStatsStatisticsEstimatorPerformance(DakotaBaseModel):
    "Return estimator performance as the final results of a UQ method"

    estimator_performance: Literal[True] = DakotaField(
        default=True,
        description="Return estimator performance as the final results of a UQ method",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.final_statistics",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ESTIMATOR_PERFORMANCE",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MlmfFinalStatsStatisticsQoiStatisticsFinalMomentsNoneKeyword(DakotaBaseModel):
    "Omit moments from the set of final statistics."

    none: Literal[True] = DakotaField(
        default=True,
        description="Omit moments from the set of final statistics.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.final_moments",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NO_MOMENTS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MlmfFinalStatsStatisticsQoiStatisticsFinalMomentsStandard(DakotaBaseModel):
    "Output standardized moments and include them within the set of final statistics."

    standard: Literal[True] = DakotaField(
        default=True,
        description="Output standardized moments and include them within the set of final statistics.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.final_moments",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "STANDARD_MOMENTS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MlmfFinalStatsStatisticsQoiStatisticsFinalMomentsCentral(DakotaBaseModel):
    "Output central moments and include them within the set of final statistics."

    central: Literal[True] = DakotaField(
        default=True,
        description="Output central moments and include them within the set of final statistics.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.final_moments",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "CENTRAL_MOMENTS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MlmfFinalStatsStatisticsQoiStatisticsDistributionCumulative(DakotaBaseModel):
    "Placeholder for future capabilities"

    cumulative: Literal[True] = DakotaField(
        default=True,
        description="Placeholder for future capabilities",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.distribution",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "CUMULATIVE",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MlmfFinalStatsStatisticsQoiStatisticsDistributionComplementary(DakotaBaseModel):
    "Placeholder for future capabilities"

    complementary: Literal[True] = DakotaField(
        default=True,
        description="Placeholder for future capabilities",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.distribution",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "COMPLEMENTARY",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfmcGroups(DakotaBaseModel):
    "Throttle multilevel BLUE to only search over MFMC groups"

    mfmc_groups: Literal[True] = DakotaField(
        default=True,
        description="Throttle multilevel BLUE to only search over MFMC groups",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.group_throttle_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MFMC_ESTIMATOR_GROUPS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class CommonGroups(DakotaBaseModel):
    "Throttle multilevel BLUE to only search over common groups"

    common_groups: Literal[True] = DakotaField(
        default=True,
        description="Throttle multilevel BLUE to only search over common groups",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.group_throttle_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "COMMON_ESTIMATOR_GROUPS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class GroupSize(DakotaBaseModel):
    "Throttle multilevel BLUE to only search over groups of a maximum size"

    group_size: int = DakotaField(
        gt=0,
        description="Throttle multilevel BLUE to only search over groups of a maximum size",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.group_size_throttle",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BestConditioned(DakotaBaseModel):
    "Throttle multilevel BLUE to only solve for allocations using the groups with the best conditioning in their group covariances"

    best_conditioned: int = DakotaField(
        gt=0,
        description="Throttle multilevel BLUE to only solve for allocations using the groups with the best conditioning in their group covariances",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.rcond_best_throttle",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class RcondTol(DakotaBaseModel):
    "Throttle multilevel BLUE to only solve for allocations using groups with prescribed conditioning in their group covariances"

    rcond_tolerance: DakotaFloat = DakotaField(
        ge=0,
        description="Throttle multilevel BLUE to only solve for allocations using groups with prescribed conditioning in their group covariances",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.rcond_tol_throttle",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class MlmfRelaxationFactorSequence(DakotaBaseModel):
    "For under-relaxation of shared sample increments, apply a sequence of factors, one per iteration"

    factor_sequence: list[DakotaFloat] = DakotaField(
        description="For under-relaxation of shared sample increments, apply a sequence of factors, one per iteration",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.relaxation.factor_sequence",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class MlmfRelaxationFixedFactor(DakotaBaseModel):
    "For under-relaxation of shared sample increments, apply a fixed factor that is invariant with iteration"

    fixed_factor: DakotaFloat = DakotaField(
        description="For under-relaxation of shared sample increments, apply a fixed factor that is invariant with iteration",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.relaxation.fixed_factor",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class MlmfRelaxationRecursiveFactor(DakotaBaseModel):
    "For under-relaxation of shared sample increments, apply a recursive factor on each iteration that advances the relaxation factor toward 1"

    recursive_factor: DakotaFloat = DakotaField(
        description="For under-relaxation of shared sample increments, apply a recursive factor on each iteration that advances the relaxation factor toward 1",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.relaxation.recursive_factor",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class OnlinePilotRelaxationFactorSequence(DakotaBaseModel):
    "For under-relaxation of shared sample increments, apply a sequence of factors, one per iteration"

    factor_sequence: list[DakotaFloat] = DakotaField(
        description="For under-relaxation of shared sample increments, apply a sequence of factors, one per iteration",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.relaxation.factor_sequence",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class OnlinePilotRelaxationFixedFactor(DakotaBaseModel):
    "For under-relaxation of shared sample increments, apply a fixed factor that is invariant with iteration"

    fixed_factor: DakotaFloat = DakotaField(
        description="For under-relaxation of shared sample increments, apply a fixed factor that is invariant with iteration",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.relaxation.fixed_factor",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class OnlinePilotRelaxationRecursiveFactor(DakotaBaseModel):
    "For under-relaxation of shared sample increments, apply a recursive factor on each iteration that advances the relaxation factor toward 1"

    recursive_factor: DakotaFloat = DakotaField(
        description="For under-relaxation of shared sample increments, apply a recursive factor on each iteration that advances the relaxation factor toward 1",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.relaxation.recursive_factor",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class OnlinePilotFinalStatisticsEstimatorPerformance(DakotaBaseModel):
    "Return estimator performance as the final results of a UQ method"

    estimator_performance: Literal[True] = DakotaField(
        default=True,
        description="Return estimator performance as the final results of a UQ method",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.final_statistics",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ESTIMATOR_PERFORMANCE",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class OnlinePilotFinalStatisticsQoiStatisticsFinalMomentsNone(DakotaBaseModel):
    "Omit moments from the set of final statistics."

    none: Literal[True] = DakotaField(
        default=True,
        description="Omit moments from the set of final statistics.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.final_moments",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NO_MOMENTS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class OnlinePilotFinalStatisticsQoiStatisticsFinalMomentsStandard(DakotaBaseModel):
    "Output standardized moments and include them within the set of final statistics."

    standard: Literal[True] = DakotaField(
        default=True,
        description="Output standardized moments and include them within the set of final statistics.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.final_moments",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "STANDARD_MOMENTS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class OnlinePilotFinalStatisticsQoiStatisticsFinalMomentsCentral(DakotaBaseModel):
    "Output central moments and include them within the set of final statistics."

    central: Literal[True] = DakotaField(
        default=True,
        description="Output central moments and include them within the set of final statistics.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.final_moments",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "CENTRAL_MOMENTS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class OnlinePilotFinalStatisticsQoiStatisticsDistributionCumulative(DakotaBaseModel):
    "Placeholder for future capabilities"

    cumulative: Literal[True] = DakotaField(
        default=True,
        description="Placeholder for future capabilities",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.distribution",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "CUMULATIVE",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class OnlinePilotFinalStatisticsQoiStatisticsDistributionComplementary(DakotaBaseModel):
    "Placeholder for future capabilities"

    complementary: Literal[True] = DakotaField(
        default=True,
        description="Placeholder for future capabilities",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.distribution",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "COMPLEMENTARY",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class OnlineProjection(DakotaBaseModel):
    "Specify a solution mode that estimates performance based on projecting initial correlation / covariance estimates from an online pilot sample"

    online_projection: Literal[True] = DakotaField(
        default=True,
        description="Specify a solution mode that estimates performance based on projecting initial correlation / covariance estimates from an online pilot sample",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.ensemble_pilot_solution_mode",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ONLINE_PILOT_PROJECTION",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class OfflineProjection(DakotaBaseModel):
    "Specify a solution mode that estimates performance based on projecting initial correlation/variance estimates from an offline pilot sample"

    offline_projection: Literal[True] = DakotaField(
        default=True,
        description="Specify a solution mode that estimates performance based on projecting initial correlation/variance estimates from an offline pilot sample",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.ensemble_pilot_solution_mode",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "OFFLINE_PILOT_PROJECTION",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExportApproxVarianceFileCustomAnnotatedConfig(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.export_approx_variance_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.export_approx_variance_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.export_approx_variance_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExportApproxVarianceFileAnnotated(DakotaBaseModel):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.export_approx_variance_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExportApproxVarianceFileFreeform(DakotaBaseModel):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.export_approx_variance_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ProbabilityLevelsContext1ProbabilityLevels(DakotaBaseModel):
    "Specify probability levels at which to compute credible and prediction intervals"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckProbabilityList(
            context="probabilitylevelscontext1probabilitylevels", list_field="values"
        ),
        CheckSumEqualsLength(
            context="probabilitylevelscontext1probabilitylevels",
            num_list_field="num_probability_levels",
            levels_list_field="values",
        ),
    ]

    values: list[DakotaFloat] = DakotaField(
        description="Specify probability levels at which to compute credible and prediction intervals"
    )
    num_probability_levels: list[int] | None = DakotaField(
        default=None, description="This child keyword is currently inactive"
    )


class ProbabilityLevelsContext2ProbabilityLevels(DakotaBaseModel):
    "Specify probability levels at which to estimate the corresponding response value"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckProbabilityList(
            context="probabilitylevelscontext2probabilitylevels", list_field="values"
        ),
        CheckSumEqualsLength(
            context="probabilitylevelscontext2probabilitylevels",
            num_list_field="num_probability_levels",
            levels_list_field="values",
        ),
    ]

    values: list[DakotaFloat] = DakotaField(
        description="Specify probability levels at which to estimate the corresponding response value"
    )
    num_probability_levels: list[int] | None = DakotaField(
        default=None,
        description="Specify which ``probability_levels`` correspond to which response",
    )


class PromotedModelSelectionContext2SearchModelGraphsNoRecursion(DakotaBaseModel):
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


class PromotedModelSelectionContext2SearchModelGraphsFullRecursion(DakotaBaseModel):
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


class PromotedModelSelectionContext1SearchModelGraphsNoRecursion(DakotaBaseModel):
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


class PromotedModelSelectionContext1SearchModelGraphsFullRecursion(DakotaBaseModel):
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


class GeneratingVectorInline(DakotaBaseModel):
    "Specify an inline generating vector"

    inline: list[int] = DakotaField(
        description="Specify an inline generating vector",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.generating_vector.inline",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )


class GeneratingVectorFile(DakotaBaseModel):
    "Specify a generating vector read from file"

    file: str = DakotaField(
        description="Specify a generating vector read from file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.generating_vector.file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Kuo(DakotaBaseModel):
    "A generating vector that provides up to 2\\ :sup:`20` points in up to 3600 dimensions"

    kuo: Literal[True] = DakotaField(
        default=True,
        description="A generating vector that provides up to 2\\ :sup:`20` points in up to 3600 dimensions",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.ld.rank1.generating_vector_scheme",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "GEN_VECTOR_KUO",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class CoolsKuoNuyens(DakotaBaseModel):
    "A generating vector that provides up to 2\\ :sup:`20` points in up to 250 dimensions"

    cools_kuo_nuyens: Literal[True] = DakotaField(
        default=True,
        description="A generating vector that provides up to 2\\ :sup:`20` points in up to 250 dimensions",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.ld.rank1.generating_vector_scheme",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "GEN_VECTOR_COOLS_KUO_NUYENS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class Rank1LatticeOptionsOrderingNatural(DakotaBaseModel):
    "Natural ordering of the points of this rank-1 lattice"

    natural: Literal[True] = DakotaField(
        default=True,
        description="Natural ordering of the points of this rank-1 lattice",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.ld.rank1.ordering",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RANK_1_LATTICE_NATURAL_ORDERING",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class RadicalInverse(DakotaBaseModel):
    "Radical inverse ordering of the points of this rank-1 lattice"

    radical_inverse: Literal[True] = DakotaField(
        default=True,
        description="Radical inverse ordering of the points of this rank-1 lattice",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.ld.rank1.ordering",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RANK_1_LATTICE_RADICAL_INVERSE_ORDERING",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ReliabilityLevelsReliabilityLevels(DakotaBaseModel):
    "Specify reliability levels at which the response values will be estimated"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckSumEqualsLength(
            context="reliabilitylevelsreliabilitylevels",
            num_list_field="num_reliability_levels",
            levels_list_field="values",
        ),
    ]

    values: list[DakotaFloat] = DakotaField(
        description="Specify reliability levels at which the response values will be estimated"
    )
    num_reliability_levels: list[int] | None = DakotaField(
        default=None,
        description="Specify which ``reliability_levels`` correspond to which response",
    )


class RngOptionsContext2Mt19937(DakotaBaseModel):
    "Generates random numbers using the Mersenne twister"

    mt19937: Literal[True] = DakotaField(
        default=True,
        description="Generates random numbers using the Mersenne twister",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.random_number_generator",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "mt19937",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class RngOptionsContext2Rnum2(DakotaBaseModel):
    "Generates pseudo-random numbers using the Pecos package"

    rnum2: Literal[True] = DakotaField(
        default=True,
        description="Generates pseudo-random numbers using the Pecos package",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.random_number_generator",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "rnum2",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class RngOptionsContext1Mt19937(DakotaBaseModel):
    "Undocumented: Recursive k-d (RKD) Darts is an experimental capability."

    mt19937: Literal[True] = DakotaField(
        default=True,
        description="Undocumented: Recursive k-d (RKD) Darts is an experimental capability.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.random_number_generator",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "mt19937",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class RngOptionsContext1Rnum2(DakotaBaseModel):
    "Undocumented: Recursive k-d (RKD) Darts is an experimental capability."

    rnum2: Literal[True] = DakotaField(
        default=True,
        description="Undocumented: Recursive k-d (RKD) Darts is an experimental capability.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.random_number_generator",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "rnum2",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class SbgoSubProblemSolverGPSurfpack(DakotaBaseModel):
    "Use the Surfpack version of Gaussian Process surrogates"

    surfpack: Literal[True] = DakotaField(
        default=True,
        description="Use the Surfpack version of Gaussian Process surrogates",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "KRIGING_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class SbgoSubProblemSolverGPDakota(DakotaBaseModel):
    "Select the built in Gaussian Process surrogate"

    dakota: Literal[True] = DakotaField(
        default=True,
        description="Select the built in Gaussian Process surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "GP_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class SbgoSubProblemSolverGPExperimentalConfig(DakotaBaseModel):
    "Use the experimental Gaussian Process surrogate"

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


class SbgoSubProblemSolverImportBuildPointsFileCustomAnnotatedConfig(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class SbgoSubProblemSolverImportBuildPointsFileAnnotated(DakotaBaseModel):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class SbgoSubProblemSolverImportBuildPointsFileFreeform(DakotaBaseModel):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class SbgoSubProblemSolverExportApproxPointsFileCustomAnnotatedConfig(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class SbgoSubProblemSolverExportApproxPointsFileAnnotated(DakotaBaseModel):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class SbgoSubProblemSolverExportApproxPointsFileFreeform(DakotaBaseModel):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class SbloSubProblemSolverSbgoExportApproxPointsFileCustomAnnotatedConfig(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class SbloSubProblemSolverSbgoExportApproxPointsFileAnnotated(DakotaBaseModel):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class SbloSubProblemSolverSbgoExportApproxPointsFileFreeform(DakotaBaseModel):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ScOptionsPiecewise(DakotaBaseModel):
    "Use piecewise local basis functions"

    piecewise: Literal[True] = DakotaField(
        default=True,
        description="Use piecewise local basis functions",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_type",
                    "storage_type": "METHOD_PIECEWISE",
                    "stored_value": "STD_UNIFORM_U",
                    "secondary_ir_key": "method.nond.piecewise_basis",
                    "secondary_ir_value_type": "bool",
                    "secondary_literal_value": True,
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ScOptionsAskey(DakotaBaseModel):
    "Select the standardized random variables (and associated basis polynomials) from the Askey family that best match the user-specified random variables."

    askey: Literal[True] = DakotaField(
        default=True,
        description="Select the standardized random variables (and associated basis polynomials) from the Askey family that best match the user-specified random variables.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ASKEY_U",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ScOptionsWiener(DakotaBaseModel):
    "Use standard normal random variables (along with Hermite orthogonal basis polynomials) when transforming to a standardized probability space."

    wiener: Literal[True] = DakotaField(
        default=True,
        description="Use standard normal random variables (along with Hermite orthogonal basis polynomials) when transforming to a standardized probability space.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "STD_NORMAL_U",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ScRefinementPRefinementUniform(DakotaBaseModel):
    "Refine an expansion uniformly in all dimensions."

    uniform: Literal[True] = DakotaField(
        default=True,
        description="Refine an expansion uniformly in all dimensions.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "UNIFORM_CONTROL",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ScRefinementPRefinementDimAdaptiveSobol(DakotaBaseModel):
    "Estimate dimension preference for automated refinement of stochastic expansion using total Sobol' sensitivity indices."

    sobol: Literal[True] = DakotaField(
        default=True,
        description="Estimate dimension preference for automated refinement of stochastic expansion using total Sobol' sensitivity indices.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DIMENSION_ADAPTIVE_CONTROL_SOBOL",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ScRefinementPRefinementDimAdaptiveGeneralized(DakotaBaseModel):
    "Use the generalized sparse grid dimension adaptive algorithm to refine a sparse grid approximation of stochastic expansion."

    generalized: Literal[True] = DakotaField(
        default=True,
        description="Use the generalized sparse grid dimension adaptive algorithm to refine a sparse grid approximation of stochastic expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DIMENSION_ADAPTIVE_CONTROL_GENERALIZED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class HRefinementUniform(DakotaBaseModel):
    "Refine an expansion uniformly in all dimensions."

    uniform: Literal[True] = DakotaField(
        default=True,
        description="Refine an expansion uniformly in all dimensions.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "UNIFORM_CONTROL",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class HRefinementDimAdaptiveSobol(DakotaBaseModel):
    "Estimate dimension preference for automated refinement of stochastic expansion using total Sobol' sensitivity indices."

    sobol: Literal[True] = DakotaField(
        default=True,
        description="Estimate dimension preference for automated refinement of stochastic expansion using total Sobol' sensitivity indices.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DIMENSION_ADAPTIVE_CONTROL_SOBOL",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class HRefinementDimAdaptiveGeneralized(DakotaBaseModel):
    "Use the generalized sparse grid dimension adaptive algorithm to refine a sparse grid approximation of stochastic expansion."

    generalized: Literal[True] = DakotaField(
        default=True,
        description="Use the generalized sparse grid dimension adaptive algorithm to refine a sparse grid approximation of stochastic expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DIMENSION_ADAPTIVE_CONTROL_GENERALIZED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ScRefinementHRefinementLocalAdaptive(DakotaBaseModel):
    "Planned future capability for local pointwise refinement within a generalized sparse grid."

    local_adaptive: Literal[True] = DakotaField(
        default=True,
        description="Planned future capability for local pointwise refinement within a generalized sparse grid.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LOCAL_ADAPTIVE_CONTROL",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ScSGLevelNodal(DakotaBaseModel):
    "Employ a nodal sparse grid construction in stochastic collocation"

    nodal: Literal[True] = DakotaField(
        default=True,
        description="Employ a nodal sparse grid construction in stochastic collocation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NODAL_INTERPOLANT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ScSGLevelHierarchical(DakotaBaseModel):
    "Employ a hierarchical sparse grid construction"

    hierarchical: Literal[True] = DakotaField(
        default=True,
        description="Employ a hierarchical sparse grid construction",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "HIERARCHICAL_INTERPOLANT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ScSGLevelRestricted(DakotaBaseModel):
    "Restrict the growth rates for nested and non-nested rules can be synchronized for consistency."

    restricted: Literal[True] = DakotaField(
        default=True,
        description="Restrict the growth rates for nested and non-nested rules can be synchronized for consistency.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.growth_override",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RESTRICTED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ScSGLevelUnrestricted(DakotaBaseModel):
    "Overide the default restriction of growth rates for nested and non-nested rules that are by defualt synchronized for consistency."

    unrestricted: Literal[True] = DakotaField(
        default=True,
        description="Overide the default restriction of growth rates for nested and non-nested rules that are by defualt synchronized for consistency.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.growth_override",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "UNRESTRICTED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ScSGLevelNested(DakotaBaseModel):
    "Enforce use of nested quadrature rules if available"

    nested: Literal[True] = DakotaField(
        default=True,
        description="Enforce use of nested quadrature rules if available",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.nesting_override",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NESTED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ScSGLevelNonNested(DakotaBaseModel):
    "Enforce use of non-nested quadrature rules"

    non_nested: Literal[True] = DakotaField(
        default=True,
        description="Enforce use of non-nested quadrature rules",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.nesting_override",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NON_NESTED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ScSGSequenceLevelSequenceNodal(DakotaBaseModel):
    "Employ a nodal sparse grid construction in stochastic collocation"

    nodal: Literal[True] = DakotaField(
        default=True,
        description="Employ a nodal sparse grid construction in stochastic collocation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NODAL_INTERPOLANT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ScSGSequenceLevelSequenceHierarchical(DakotaBaseModel):
    "Employ a hierarchical sparse grid construction"

    hierarchical: Literal[True] = DakotaField(
        default=True,
        description="Employ a hierarchical sparse grid construction",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "HIERARCHICAL_INTERPOLANT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ScSGSequenceLevelSequenceRestricted(DakotaBaseModel):
    "Restrict the growth rates for nested and non-nested rules can be synchronized for consistency."

    restricted: Literal[True] = DakotaField(
        default=True,
        description="Restrict the growth rates for nested and non-nested rules can be synchronized for consistency.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.growth_override",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RESTRICTED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ScSGSequenceLevelSequenceUnrestricted(DakotaBaseModel):
    "Overide the default restriction of growth rates for nested and non-nested rules that are by defualt synchronized for consistency."

    unrestricted: Literal[True] = DakotaField(
        default=True,
        description="Overide the default restriction of growth rates for nested and non-nested rules that are by defualt synchronized for consistency.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.growth_override",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "UNRESTRICTED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ScSGSequenceLevelSequenceNested(DakotaBaseModel):
    "Enforce use of nested quadrature rules if available"

    nested: Literal[True] = DakotaField(
        default=True,
        description="Enforce use of nested quadrature rules if available",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.nesting_override",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NESTED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ScSGSequenceLevelSequenceNonNested(DakotaBaseModel):
    "Enforce use of non-nested quadrature rules"

    non_nested: Literal[True] = DakotaField(
        default=True,
        description="Enforce use of non-nested quadrature rules",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.nesting_override",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NON_NESTED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ScSparseInterpolantNodal(DakotaBaseModel):
    "Employ a nodal sparse grid construction in stochastic collocation"

    nodal: Literal[True] = DakotaField(
        default=True,
        description="Employ a nodal sparse grid construction in stochastic collocation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NODAL_INTERPOLANT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ScSparseInterpolantHierarchical(DakotaBaseModel):
    "Employ a hierarchical sparse grid construction"

    hierarchical: Literal[True] = DakotaField(
        default=True,
        description="Employ a hierarchical sparse grid construction",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "HIERARCHICAL_INTERPOLANT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BinnedConfig(DakotaBaseModel):
    "Computes Sobol' main effects using a binned approach"

    num_bins: int = DakotaField(
        default=-1,
        description="Number of bins used to compute the variance-based decomposition",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.vbd_via_sampling_num_bins",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )


class PickAndFreeze(DakotaBaseModel):
    "Use the pick-and-freeze variance-based decomposition method"

    pick_and_freeze: Literal[True] = DakotaField(
        default=True,
        description="Use the pick-and-freeze variance-based decomposition method",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.vbd_via_sampling_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "VBD_PICK_AND_FREEZE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodDotCommonOptsMixin(
    DefaultConstraintTolMixin,
    DefaultScalingMixin,
    DefaultSpeculativeMixin,
    MethodConvergenceTolMixin,
    MethodMaxFunctionEvaluationsContext1Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
):
    "Generated model for MethodDotCommonOptsMixin"

    pass


class DefaultFinalMomentsMixin(DakotaBaseModel):
    "Generated model for DefaultFinalMomentsMixin"

    final_moments: Union[
        DefaultFinalMomentsNoneKeyword,
        DefaultFinalMomentsStandard,
        DefaultFinalMomentsCentral,
    ] = DakotaField(
        default_factory=DefaultFinalMomentsStandard,
        description="Output moments of the specified type and include them within the set of final statistics.",
        dakota={"union_pattern": 1, "model_default": "DefaultFinalMomentsStandard"},
    )


class DefaultTrustRegionContext1Mixin(DakotaBaseModel):
    "Generated model for DefaultTrustRegionContext1Mixin"

    trust_region: DefaultTrustRegionContext1TrustRegion | None = DakotaField(
        default=None,
        description="Specification group for trust region model management",
    )


class DefaultTrustRegionContext2Mixin(DakotaBaseModel):
    "Generated model for DefaultTrustRegionContext2Mixin"

    trust_region: DefaultTrustRegionContext2TrustRegion | None = DakotaField(
        default=None, description="Use trust region as the globalization strategy."
    )


class GeneratingMatricesPredefined(DakotaBaseModel):
    "Specify predefined generating matrices"

    predefined: Union[JoeKuo, SobolOrder2] = DakotaField(
        description="Specify predefined generating matrices"
    )


class DiscrepEmulationMixin(DakotaBaseModel):
    "Generated model for DiscrepEmulationMixin"

    discrepancy_emulation: (
        Union[
            DiscrepEmulationDiscrepancyEmulationDistinct,
            DiscrepEmulationDiscrepancyEmulationRecursive,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Formulation for emulation of model discrepancies.",
        dakota={"union_pattern": 2},
    )


class DistributionCumulComplContext2Mixin(DakotaBaseModel):
    "Generated model for DistributionCumulComplContext2Mixin"

    distribution: Union[
        DistributionCumulComplContext2Cumulative,
        DistributionCumulComplContext2Complementary,
    ] = DakotaField(
        default_factory=DistributionCumulComplContext2Cumulative,
        description="Placeholder for future capabilities",
        dakota={
            "union_pattern": 1,
            "model_default": "DistributionCumulComplContext2Cumulative",
        },
    )


class DistributionCumulComplContext1Mixin(DakotaBaseModel):
    "Generated model for DistributionCumulComplContext1Mixin"

    distribution: Union[
        DistributionCumulComplContext1Cumulative,
        DistributionCumulComplContext1Complementary,
    ] = DakotaField(
        default_factory=DistributionCumulComplContext1Cumulative,
        description="Selection of cumulative or complementary cumulative functions",
        dakota={
            "union_pattern": 1,
            "model_default": "DistributionCumulComplContext1Cumulative",
        },
    )


class EgoSubProblemSolverContext2GPSurfpackExportModel(DakotaBaseModel):
    "Exports surrogate model in user-specified format(s)"

    filename_prefix: str = DakotaField(
        default="exported_surrogate",
        description="User-customizable portion of exported/imported surrogate model filenames",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_prefix",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    formats: EgoSubProblemSolverContext2GPSurfpackExportModelFormats = DakotaField(
        description="Formats for surrogate model export"
    )


class EgoSubProblemSolverContext2GPExperimentalExportModel(DakotaBaseModel):
    "Exports surrogate model in user-specified format(s)"

    filename_prefix: str = DakotaField(
        default="exported_surrogate",
        description="User-customizable portion of exported/imported surrogate model filenames",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_prefix",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    formats: EgoSubProblemSolverContext2GPSurfpackExportModelFormats = DakotaField(
        description="Formats for surrogate model export"
    )


class EgoSubProblemSolverContext2ImportBuildPointsFileCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: EgoSubProblemSolverContext2ImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=EgoSubProblemSolverContext2ImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "EgoSubProblemSolverContext2ImportBuildPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class EgoSubProblemSolverContext2ExportApproxPointsFileCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: EgoSubProblemSolverContext2ExportApproxPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=EgoSubProblemSolverContext2ExportApproxPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "EgoSubProblemSolverContext2ExportApproxPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class EgoSubProblemSolverContext1GPSurfpackExportModel(DakotaBaseModel):
    "export_model not active"

    filename_prefix: str = DakotaField(
        default="exported_surrogate",
        description="export_model not active",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_prefix",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    formats: EgoSubProblemSolverContext1GPSurfpackExportModelFormats = DakotaField(
        description="export_model not active"
    )


class EgoSubProblemSolverContext1GPExperimentalExportModel(DakotaBaseModel):
    "export_model not active"

    filename_prefix: str = DakotaField(
        default="exported_surrogate",
        description="export_model not active",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_prefix",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    formats: EgoSubProblemSolverContext1GPSurfpackExportModelFormats = DakotaField(
        description="export_model not active"
    )


class EgoSubProblemSolverContext1ImportBuildPointsFileCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: EgoSubProblemSolverContext1ImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=EgoSubProblemSolverContext1ImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "EgoSubProblemSolverContext1ImportBuildPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class EgoSubProblemSolverContext1ExportApproxPointsFileCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: EgoSubProblemSolverContext1ExportApproxPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=EgoSubProblemSolverContext1ExportApproxPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "EgoSubProblemSolverContext1ExportApproxPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class GenReliabilityLevelsMixin(DakotaBaseModel):
    "Generated model for GenReliabilityLevelsMixin"

    gen_reliability_levels: GenReliabilityLevelsGenReliabilityLevels | None = (
        DakotaField(
            default=None,
            description="Specify generalized relability levels at which to estimate the corresponding response value",
            dakota={
                "argument": "values",
                "materialization": [
                    {
                        "ir_key": "method.nond.gen_reliability_levels",
                        "storage_type": "RESPONSE_LEVELS_ARRAY",
                        "ir_value_type": "RealVectorArray",
                    }
                ],
            },
        )
    )


class ImportBuildPointsFileCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: ImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=ImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "ImportBuildPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class MethodCovTypeMixin(DakotaBaseModel):
    "Generated model for MethodCovTypeMixin"

    covariance_type: Union[MethodCovTypeDiagCov, MethodCovTypeFullCov] | None = (
        DakotaField(
            default=None,
            description="Covariance Type",
            dakota={"anchor": True, "union_pattern": 2},
        )
    )


class MethodExportModelContext2ExportModel(DakotaBaseModel):
    "export_model not active"

    filename_prefix: str = DakotaField(
        default="exported_surrogate",
        description="export_model not active",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_prefix",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    formats: MethodExportModelContext2Formats = DakotaField(
        description="export_model not active"
    )


class MethodExportModelContext1ExportModel(DakotaBaseModel):
    "Exports surrogate model in user-specified format(s)"

    filename_prefix: str = DakotaField(
        default="exported_surrogate",
        description="User-customizable portion of exported/imported surrogate model filenames",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_prefix",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    formats: MethodExportModelContext1Formats = DakotaField(
        description="Formats for surrogate model export"
    )


class MethodGPGP(DakotaBaseModel):
    "gaussian_process"

    implementation: Union[MethodGPSurfpack, MethodGPDakota] = DakotaField(
        default=..., description="GP Implementation", dakota={"anchor": True}
    )


class MethodGlobalSubProblemSolverContext2SbgoGPExperimental(DakotaBaseModel):
    "Use the experimental Gaussian Process surrogate"

    experimental: MethodGlobalSubProblemSolverContext2SbgoGPExperimentalConfig = (
        DakotaField(
            description="Use the experimental Gaussian Process surrogate",
            dakota={
                "materialization": [
                    {
                        "ir_key": "method.nond.emulator",
                        "storage_type": "PRESENCE_ENUM",
                        "stored_value": "EXPGP_EMULATOR",
                        "ir_value_type": "short",
                    }
                ]
            },
        )
    )


class MethodGlobalSubProblemSolverContext2SbgoImportBuildPointsFileCustomAnnotated(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    custom_annotated: MethodGlobalSubProblemSolverContext2SbgoImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=MethodGlobalSubProblemSolverContext2SbgoImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "MethodGlobalSubProblemSolverContext2SbgoImportBuildPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class MethodGlobalSubProblemSolverContext2SbgoExportApproxPointsFileCustomAnnotated(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    custom_annotated: MethodGlobalSubProblemSolverContext2SbgoExportApproxPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=MethodGlobalSubProblemSolverContext2SbgoExportApproxPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "MethodGlobalSubProblemSolverContext2SbgoExportApproxPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class MethodGlobalSubProblemSolverContext2EgoGPSurfpackExportModel(DakotaBaseModel):
    "Exports surrogate model in user-specified format(s)"

    filename_prefix: str = DakotaField(
        default="exported_surrogate",
        description="User-customizable portion of exported/imported surrogate model filenames",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_prefix",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    formats: MethodGlobalSubProblemSolverContext2EgoGPSurfpackExportModelFormats = (
        DakotaField(description="Formats for surrogate model export")
    )


class MethodGlobalSubProblemSolverContext2EgoGPExperimentalExportModel(DakotaBaseModel):
    "Exports surrogate model in user-specified format(s)"

    filename_prefix: str = DakotaField(
        default="exported_surrogate",
        description="User-customizable portion of exported/imported surrogate model filenames",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_prefix",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    formats: MethodGlobalSubProblemSolverContext2EgoGPExperimentalExportModelFormats = (
        DakotaField(description="Formats for surrogate model export")
    )


class MethodGlobalSubProblemSolverContext2EgoImportBuildPointsFileCustomAnnotated(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    custom_annotated: MethodGlobalSubProblemSolverContext2EgoImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=MethodGlobalSubProblemSolverContext2EgoImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "MethodGlobalSubProblemSolverContext2EgoImportBuildPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class MethodGlobalSubProblemSolverContext2EgoExportApproxPointsFileCustomAnnotated(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    custom_annotated: MethodGlobalSubProblemSolverContext2EgoExportApproxPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=MethodGlobalSubProblemSolverContext2EgoExportApproxPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "MethodGlobalSubProblemSolverContext2EgoExportApproxPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class MethodGlobalSubProblemSolverContext1SbgoGPExperimental(DakotaBaseModel):
    "Use the experimental Gaussian Process surrogate"

    experimental: MethodGlobalSubProblemSolverContext1SbgoGPExperimentalConfig = (
        DakotaField(
            description="Use the experimental Gaussian Process surrogate",
            dakota={
                "materialization": [
                    {
                        "ir_key": "method.nond.emulator",
                        "storage_type": "PRESENCE_ENUM",
                        "stored_value": "EXPGP_EMULATOR",
                        "ir_value_type": "short",
                    }
                ]
            },
        )
    )


class MethodGlobalSubProblemSolverContext1SbgoImportBuildPointsFileCustomAnnotated(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    custom_annotated: MethodGlobalSubProblemSolverContext1SbgoImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=MethodGlobalSubProblemSolverContext1SbgoImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "MethodGlobalSubProblemSolverContext1SbgoImportBuildPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class MethodGlobalSubProblemSolverContext1SbgoExportApproxPointsFileCustomAnnotated(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    custom_annotated: MethodGlobalSubProblemSolverContext1SbgoExportApproxPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=MethodGlobalSubProblemSolverContext1SbgoExportApproxPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "MethodGlobalSubProblemSolverContext1SbgoExportApproxPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class MethodGlobalSubProblemSolverContext1EgoGPSurfpackExportModel(DakotaBaseModel):
    "export_model not active"

    filename_prefix: str = DakotaField(
        default="exported_surrogate",
        description="export_model not active",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_prefix",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    formats: MethodGlobalSubProblemSolverContext1EgoGPSurfpackExportModelFormats = (
        DakotaField(description="export_model not active")
    )


class MethodGlobalSubProblemSolverContext1EgoGPExperimentalExportModel(DakotaBaseModel):
    "export_model not active"

    filename_prefix: str = DakotaField(
        default="exported_surrogate",
        description="export_model not active",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_prefix",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    formats: MethodGlobalSubProblemSolverContext1EgoGPExperimentalExportModelFormats = (
        DakotaField(description="export_model not active")
    )


class MethodGlobalSubProblemSolverContext1EgoImportBuildPointsFileCustomAnnotated(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    custom_annotated: MethodGlobalSubProblemSolverContext1EgoImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=MethodGlobalSubProblemSolverContext1EgoImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "MethodGlobalSubProblemSolverContext1EgoImportBuildPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class MethodGlobalSubProblemSolverContext1EgoExportApproxPointsFileCustomAnnotated(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    custom_annotated: MethodGlobalSubProblemSolverContext1EgoExportApproxPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=MethodGlobalSubProblemSolverContext1EgoExportApproxPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "MethodGlobalSubProblemSolverContext1EgoExportApproxPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class MethodGpAlternativesNoExportGPExperimental(DakotaBaseModel):
    "Use the experimental Gaussian Process surrogate"

    experimental: MethodGpAlternativesNoExportGPExperimentalConfig = DakotaField(
        description="Use the experimental Gaussian Process surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "EXPGP_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodGpAlternativesWithExportContext2GPSurfpackExportModel(DakotaBaseModel):
    "Exports surrogate model in user-specified format(s)"

    filename_prefix: str = DakotaField(
        default="exported_surrogate",
        description="User-customizable portion of exported/imported surrogate model filenames",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_prefix",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    formats: MethodGpAlternativesWithExportContext2GPSurfpackExportModelFormats = (
        DakotaField(description="Formats for surrogate model export")
    )


class MethodGpAlternativesWithExportContext2GPExperimentalExportModel(DakotaBaseModel):
    "Exports surrogate model in user-specified format(s)"

    filename_prefix: str = DakotaField(
        default="exported_surrogate",
        description="User-customizable portion of exported/imported surrogate model filenames",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_prefix",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    formats: MethodGpAlternativesWithExportContext2GPSurfpackExportModelFormats = (
        DakotaField(description="Formats for surrogate model export")
    )


class MethodGpAlternativesWithExportContext1GPSurfpackExportModel(DakotaBaseModel):
    "export_model not active"

    filename_prefix: str = DakotaField(
        default="exported_surrogate",
        description="export_model not active",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_prefix",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    formats: MethodGpAlternativesWithExportContext1GPSurfpackExportModelFormats = (
        DakotaField(description="export_model not active")
    )


class MethodGpAlternativesWithExportContext1GPExperimentalExportModel(DakotaBaseModel):
    "export_model not active"

    filename_prefix: str = DakotaField(
        default="exported_surrogate",
        description="export_model not active",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_export_prefix",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    formats: MethodGpAlternativesWithExportContext1GPSurfpackExportModelFormats = (
        DakotaField(description="export_model not active")
    )


class MethodGradientSubProblemSolverMixin(DakotaBaseModel):
    "Generated model for MethodGradientSubProblemSolverMixin"

    optimization_solver: (
        Union[MethodGradientSubProblemSolverSqp, MethodGradientSubProblemSolverNip]
        | None
    ) = DakotaField(
        default=None,
        description="Optimization Solver",
        dakota={"anchor": True, "union_pattern": 2},
    )


class MethodIteratorServerSchedulingMixin(DakotaBaseModel):
    "Generated model for MethodIteratorServerSchedulingMixin"

    iterator_servers: int | None = DakotaField(
        default=None,
        gt=0,
        description="Specify the number of iterator servers when Dakota is run in parallel",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.iterator_servers",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    iterator_scheduling: (
        Union[
            MethodIteratorServerSchedulingDedicated, MethodIteratorServerSchedulingPeer
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Specify the scheduling of concurrent iterators when Dakota is run in parallel",
        dakota={"union_pattern": 2},
    )
    processors_per_iterator: int | None = DakotaField(
        default=None,
        gt=0,
        description="Specify the number of processors per iterator server when Dakota is run in parallel",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.processors_per_iterator",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )


class ShuffleRandom(DakotaBaseModel):
    "Perform crossover by choosing design variable(s)"

    shuffle_random: ShuffleRandomConfig = DakotaField(
        description="Perform crossover by choosing design variable(s)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.crossover_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "shuffle_random",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MethodJegaCommonOptsMutationTypeOffsetNormal(DakotaBaseModel):
    "Set mutation offset to use a normal distribution"

    offset_normal: OffsetNormalConfig = DakotaField(
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


class MethodJegaCommonOptsMutationTypeOffsetCauchy(DakotaBaseModel):
    "Use a Cauchy distribution for the mutation offset"

    offset_cauchy: OffsetNormalConfig = DakotaField(
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


class MethodJegaCommonOptsMutationTypeOffsetUniform(DakotaBaseModel):
    "Set mutation offset to use a uniform distribution"

    offset_uniform: OffsetNormalConfig = DakotaField(
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


class MethodMlmfSolverMetricNormEstimatorVariance(DakotaBaseModel):
    "Utilize a p-norm over the vector of QoI estimator variances as the solver metric for sampling-based multifidelity methods."

    norm_estimator_variance: MethodMlmfSolverMetricNormEstimatorVarianceConfig = DakotaField(
        description="Utilize a p-norm over the vector of QoI estimator variances as the solver metric for sampling-based multifidelity methods.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.estimator_variance_metric",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NORM_ESTVAR_METRIC",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodMlmfSubProblemSolverMixin(DakotaBaseModel):
    "Generated model for MethodMlmfSubProblemSolverMixin"

    optimization_solver: (
        Union[
            MethodMlmfSubProblemSolverSqp,
            MethodMlmfSubProblemSolverNip,
            MethodMlmfSubProblemSolverGlobalLocal,
            MethodMlmfSubProblemSolverCompetedLocal,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Optimization Solver",
        dakota={"anchor": True, "union_pattern": 2},
    )


class MethodOptppNewtonOptsMixin(DakotaBaseModel):
    "Generated model for MethodOptppNewtonOptsMixin"

    search_method: (
        Union[
            ValueBasedLineSearch,
            GradientBasedLineSearch,
            SearchMethodTrustRegion,
            TrPds,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Select a search method for Newton-based optimizers",
        dakota={"union_pattern": 2},
    )
    merit_function: Union[ElBakry, ArgaezTapia, VanShanno] = DakotaField(
        default_factory=ArgaezTapia,
        description="Balance goals of reducing objective function and satisfying constraints",
        dakota={"union_pattern": 1, "model_default": "ArgaezTapia"},
    )
    steplength_to_boundary: DakotaFloat = DakotaField(
        default=-1.0,
        description="Controls how close to the boundary of the feasible region the algorithm is allowed to move",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.optpp.steplength_to_boundary",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    centering_parameter: DakotaFloat = DakotaField(
        default=-1.0,
        description='Controls how closely the algorithm should follow the "central path"',
        dakota={
            "materialization": [
                {
                    "ir_key": "method.optpp.centering_parameter",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class MethodSynchronizationMixin(DakotaBaseModel):
    "Generated model for MethodSynchronizationMixin"

    synchronization: Union[Blocking, Nonblocking] | None = DakotaField(
        default=None,
        description="Select how Dakota schedules a batch of concurrent function evaluations in a parallel algorithm",
        dakota={"union_pattern": 2},
    )


class MethodThreeOptionalKeywordsMixin(DakotaBaseModel):
    "Generated model for MethodThreeOptionalKeywordsMixin"

    id_method: str | None = DakotaField(
        default=None,
        description="Name the method block; helpful when there are multiple",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.id",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    output: Union[Debug, Verbose, Normal, Quiet, Silent] = DakotaField(
        default_factory=Normal,
        description="Control how much method information is written to the screen and output file",
        dakota={"union_pattern": 1, "model_default": "Normal"},
    )
    final_solutions: int = DakotaField(
        default=0,
        ge=0,
        description="Number of designs returned as the best solutions",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.final_solutions",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class MfAllocControlContext2Mixin(DakotaBaseModel):
    "Generated model for MfAllocControlContext2Mixin"

    allocation_control: Union[MfAllocControlContext2AllocationControlGreedy] | None = (
        DakotaField(
            default=None,
            description="Sample allocation approach for multifidelity expansions",
            dakota={"union_pattern": 2},
        )
    )


class MfAllocControlContext1Mixin(DakotaBaseModel):
    "Generated model for MfAllocControlContext1Mixin"

    allocation_control: Union[MfAllocControlContext1AllocationControlGreedy] | None = (
        DakotaField(
            default=None,
            description="Sample allocation approach for multifidelity expansions",
            dakota={"union_pattern": 2},
        )
    )


class MfAllocControlContext3Mixin(DakotaBaseModel):
    "Generated model for MfAllocControlContext3Mixin"

    allocation_control: Union[MfAllocControlContext3AllocationControlGreedy] | None = (
        DakotaField(
            default=None,
            description="Sample allocation approach for multifidelity expansions",
            dakota={"union_pattern": 2},
        )
    )


class MlmfFinalStatsStatisticsQoiStatisticsConfig(DakotaBaseModel):
    "Return the quantity of interest (QoI) statistics as the final results of a UQ method"

    final_moments: Union[
        MlmfFinalStatsStatisticsQoiStatisticsFinalMomentsNoneKeyword,
        MlmfFinalStatsStatisticsQoiStatisticsFinalMomentsStandard,
        MlmfFinalStatsStatisticsQoiStatisticsFinalMomentsCentral,
    ] = DakotaField(
        default_factory=MlmfFinalStatsStatisticsQoiStatisticsFinalMomentsStandard,
        description="Output moments of the specified type and include them within the set of final statistics.",
        dakota={
            "union_pattern": 1,
            "model_default": "MlmfFinalStatsStatisticsQoiStatisticsFinalMomentsStandard",
        },
    )
    distribution: Union[
        MlmfFinalStatsStatisticsQoiStatisticsDistributionCumulative,
        MlmfFinalStatsStatisticsQoiStatisticsDistributionComplementary,
    ] = DakotaField(
        default_factory=MlmfFinalStatsStatisticsQoiStatisticsDistributionCumulative,
        description="Placeholder for future capabilities",
        dakota={
            "union_pattern": 1,
            "model_default": "MlmfFinalStatsStatisticsQoiStatisticsDistributionCumulative",
        },
    )


class MlmfGroupThrottleMixin(DakotaBaseModel):
    "Generated model for MlmfGroupThrottleMixin"

    group_throttle: (
        Union[MfmcGroups, CommonGroups, GroupSize, BestConditioned, RcondTol] | None
    ) = DakotaField(
        default=None,
        description="Reduce the number of groups in multilevel BLUE using a throttle",
        dakota={"union_pattern": 2},
    )


class MlmfRelaxationMixin(DakotaBaseModel):
    "Generated model for MlmfRelaxationMixin"

    relaxation: (
        Union[
            MlmfRelaxationFactorSequence,
            MlmfRelaxationFixedFactor,
            MlmfRelaxationRecursiveFactor,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="For an online pilot mode, apply under-relaxation to the shared sample increments",
        dakota={"union_pattern": 2},
    )


class OnlinePilotFinalStatisticsQoiStatisticsConfig(DakotaBaseModel):
    "Return the quantity of interest (QoI) statistics as the final results of a UQ method"

    final_moments: Union[
        OnlinePilotFinalStatisticsQoiStatisticsFinalMomentsNone,
        OnlinePilotFinalStatisticsQoiStatisticsFinalMomentsStandard,
        OnlinePilotFinalStatisticsQoiStatisticsFinalMomentsCentral,
    ] = DakotaField(
        default_factory=OnlinePilotFinalStatisticsQoiStatisticsFinalMomentsStandard,
        description="Output moments of the specified type and include them within the set of final statistics.",
        dakota={
            "union_pattern": 1,
            "model_default": "OnlinePilotFinalStatisticsQoiStatisticsFinalMomentsStandard",
        },
    )
    distribution: Union[
        OnlinePilotFinalStatisticsQoiStatisticsDistributionCumulative,
        OnlinePilotFinalStatisticsQoiStatisticsDistributionComplementary,
    ] = DakotaField(
        default_factory=OnlinePilotFinalStatisticsQoiStatisticsDistributionCumulative,
        description="Placeholder for future capabilities",
        dakota={
            "union_pattern": 1,
            "model_default": "OnlinePilotFinalStatisticsQoiStatisticsDistributionCumulative",
        },
    )


class OfflinePilotFinalStatisticsQoiStatisticsConfig(DakotaBaseModel):
    "Return the quantity of interest (QoI) statistics as the final results of a UQ method"

    final_moments: Union[
        OnlinePilotFinalStatisticsQoiStatisticsFinalMomentsNone,
        OnlinePilotFinalStatisticsQoiStatisticsFinalMomentsStandard,
        OnlinePilotFinalStatisticsQoiStatisticsFinalMomentsCentral,
    ] = DakotaField(
        default_factory=OnlinePilotFinalStatisticsQoiStatisticsFinalMomentsStandard,
        description="Output moments of the specified type and include them within the set of final statistics.",
        dakota={
            "union_pattern": 1,
            "model_default": "OnlinePilotFinalStatisticsQoiStatisticsFinalMomentsStandard",
        },
    )
    distribution: Union[
        OnlinePilotFinalStatisticsQoiStatisticsDistributionCumulative,
        OnlinePilotFinalStatisticsQoiStatisticsDistributionComplementary,
    ] = DakotaField(
        default_factory=OnlinePilotFinalStatisticsQoiStatisticsDistributionCumulative,
        description="Placeholder for future capabilities",
        dakota={
            "union_pattern": 1,
            "model_default": "OnlinePilotFinalStatisticsQoiStatisticsDistributionCumulative",
        },
    )


class ExportApproxVarianceFileCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: ExportApproxVarianceFileCustomAnnotatedConfig = DakotaField(
        default_factory=ExportApproxVarianceFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "ExportApproxVarianceFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "model.surrogate.export_approx_variance_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class ProbabilityLevelsContext1Mixin(DakotaBaseModel):
    "Generated model for ProbabilityLevelsContext1Mixin"

    probability_levels: ProbabilityLevelsContext1ProbabilityLevels | None = DakotaField(
        default=None,
        description="Specify probability levels at which to compute credible and prediction intervals",
        dakota={
            "argument": "values",
            "materialization": [
                {
                    "ir_key": "method.nond.probability_levels",
                    "storage_type": "RESPONSE_LEVELS_ARRAY",
                    "ir_value_type": "RealVectorArray",
                }
            ],
        },
    )


class ProbabilityLevelsContext2Mixin(DakotaBaseModel):
    "Generated model for ProbabilityLevelsContext2Mixin"

    probability_levels: ProbabilityLevelsContext2ProbabilityLevels | None = DakotaField(
        default=None,
        description="Specify probability levels at which to estimate the corresponding response value",
        dakota={
            "argument": "values",
            "materialization": [
                {
                    "ir_key": "method.nond.probability_levels",
                    "storage_type": "RESPONSE_LEVELS_ARRAY",
                    "ir_value_type": "RealVectorArray",
                }
            ],
        },
    )


class PromotedModelSelectionContext2SearchModelGraphs(DakotaBaseModel):
    "For weighted multilevel Monte Carlo, this option activates a search over possible hierarchical model graphs"

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
    recursion_option: Union[
        PromotedModelSelectionContext2SearchModelGraphsNoRecursion,
        PromotedModelSelectionContext2SearchModelGraphsFullRecursion,
    ] = DakotaField(
        description="DAG Ensemble Generation Option",
        dakota={"anchor": True, "union_pattern": 4},
    )


class PromotedModelSelectionContext1SearchModelGraphs(DakotaBaseModel):
    "Perform a search over admissible model relationships for a given model ensemble"

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
    recursion_option: Union[
        PromotedModelSelectionContext1SearchModelGraphsNoRecursion,
        PromotedModelSelectionContext1SearchModelGraphsFullRecursion,
    ] = DakotaField(
        description="DAG Ensemble Generation Option",
        dakota={"anchor": True, "union_pattern": 4},
    )


class GeneratingVectorPredefined(DakotaBaseModel):
    "Specify a predefined generating vector"

    predefined: Union[Kuo, CoolsKuoNuyens] = DakotaField(
        description="Specify a predefined generating vector"
    )


class ReliabilityLevelsMixin(DakotaBaseModel):
    "Generated model for ReliabilityLevelsMixin"

    reliability_levels: ReliabilityLevelsReliabilityLevels | None = DakotaField(
        default=None,
        description="Specify reliability levels at which the response values will be estimated",
        dakota={
            "argument": "values",
            "materialization": [
                {
                    "ir_key": "method.nond.reliability_levels",
                    "storage_type": "RESPONSE_LEVELS_ARRAY",
                    "ir_value_type": "RealVectorArray",
                }
            ],
        },
    )


class RngOptionsContext2Mixin(DakotaBaseModel):
    "Generated model for RngOptionsContext2Mixin"

    rng: Union[RngOptionsContext2Mt19937, RngOptionsContext2Rnum2] = DakotaField(
        default_factory=RngOptionsContext2Mt19937,
        description="Selection of a random number generator",
        dakota={"union_pattern": 1, "model_default": "RngOptionsContext2Mt19937"},
    )


class RngOptionsContext1Mixin(DakotaBaseModel):
    "Generated model for RngOptionsContext1Mixin"

    rng: Union[RngOptionsContext1Mt19937, RngOptionsContext1Rnum2] = DakotaField(
        default_factory=RngOptionsContext1Mt19937,
        description="Undocumented: Recursive k-d (RKD) Darts is an experimental capability.",
        dakota={"union_pattern": 1, "model_default": "RngOptionsContext1Mt19937"},
    )


class SbgoSubProblemSolverGPExperimental(DakotaBaseModel):
    "Use the experimental Gaussian Process surrogate"

    experimental: SbgoSubProblemSolverGPExperimentalConfig = DakotaField(
        description="Use the experimental Gaussian Process surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "EXPGP_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class SbgoSubProblemSolverImportBuildPointsFileCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: SbgoSubProblemSolverImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=SbgoSubProblemSolverImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "SbgoSubProblemSolverImportBuildPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class SbgoSubProblemSolverExportApproxPointsFileCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: SbgoSubProblemSolverExportApproxPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=SbgoSubProblemSolverExportApproxPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "SbgoSubProblemSolverExportApproxPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class SbloSubProblemSolverSbgoExportApproxPointsFileCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: SbloSubProblemSolverSbgoExportApproxPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=SbloSubProblemSolverSbgoExportApproxPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "SbloSubProblemSolverSbgoExportApproxPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class ScOptionsMixin(DakotaBaseModel):
    "Generated model for ScOptionsMixin"

    use_derivatives: Literal[True] | None = DakotaField(
        default=None,
        description="Use derivative data to construct surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.derivative_usage",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    basis_family: Union[ScOptionsPiecewise, ScOptionsAskey, ScOptionsWiener] | None = (
        DakotaField(
            default=None,
            description="Basis Polynomial Family",
            dakota={"anchor": True, "union_pattern": 2},
        )
    )


class ScRefinementPRefinementDimAdaptive(DakotaBaseModel):
    'Perform anisotropic expansion refinement by preferentially adapting in dimensions that are detected to have higher \\"importance\\".'

    dimension_adaptive: Union[
        ScRefinementPRefinementDimAdaptiveSobol,
        ScRefinementPRefinementDimAdaptiveGeneralized,
    ] = DakotaField(
        description='Perform anisotropic expansion refinement by preferentially adapting in dimensions that are detected to have higher "importance".'
    )


class ScRefinementHRefinementDimAdaptive(DakotaBaseModel):
    'Perform anisotropic expansion refinement by preferentially adapting in dimensions that are detected to have higher \\"importance\\".'

    dimension_adaptive: Union[
        HRefinementDimAdaptiveSobol, HRefinementDimAdaptiveGeneralized
    ] = DakotaField(
        description='Perform anisotropic expansion refinement by preferentially adapting in dimensions that are detected to have higher "importance".'
    )


class ScSGSGLevel(DakotaBaseModel):
    "Level to use in sparse grid integration or interpolation"

    level: int = DakotaField(
        default=USHRT_MAX,
        description="Level to use in sparse grid integration or interpolation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.sparse_grid_level",
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
    interpolant: Union[ScSGLevelNodal, ScSGLevelHierarchical] | None = DakotaField(
        default=None, dakota={"anchor": True, "union_pattern": 2}
    )
    growth_rule: Union[ScSGLevelRestricted, ScSGLevelUnrestricted] | None = DakotaField(
        default=None,
        description="Quadrature Rule Growth",
        dakota={"anchor": True, "union_pattern": 2},
    )
    nesting_rule: Union[ScSGLevelNested, ScSGLevelNonNested] | None = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class ScSGSequenceSGLevelSequence(DakotaBaseModel):
    "Sequence of sparse grid levels used in a multi-stage expansion"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="scsgsequencesglevelsequence", list_field="sequence"
        ),
    ]

    sequence: list[int] = DakotaField(
        description="Sequence of sparse grid levels used in a multi-stage expansion",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.sparse_grid_level_sequence",
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
    interpolant: (
        Union[ScSGSequenceLevelSequenceNodal, ScSGSequenceLevelSequenceHierarchical]
        | None
    ) = DakotaField(default=None, dakota={"anchor": True, "union_pattern": 2})
    growth_rule: (
        Union[
            ScSGSequenceLevelSequenceRestricted, ScSGSequenceLevelSequenceUnrestricted
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Growth",
        dakota={"anchor": True, "union_pattern": 2},
    )
    nesting_rule: (
        Union[ScSGSequenceLevelSequenceNested, ScSGSequenceLevelSequenceNonNested]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class ScSparseInterpolantMixin(DakotaBaseModel):
    "Generated model for ScSparseInterpolantMixin"

    interpolant: (
        Union[ScSparseInterpolantNodal, ScSparseInterpolantHierarchical] | None
    ) = DakotaField(default=None, dakota={"anchor": True, "union_pattern": 2})


class Binned(DakotaBaseModel):
    "Computes Sobol' main effects using a binned approach"

    binned: BinnedConfig = DakotaField(
        description="Computes Sobol' main effects using a binned approach",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.vbd_via_sampling_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "VBD_BINNED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class DigitalNetOptionsMixin(DakotaBaseModel):
    "Generated model for DigitalNetOptionsMixin"

    no_digital_shift: Literal[True] | None = DakotaField(
        default=None,
        description="Do not shift this digital net",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.no_digital_shift",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    no_scrambling: Literal[True] | None = DakotaField(
        default=None,
        description="Do not scramble this digital net",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.no_scrambling",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    integer_format: Union[MostSignificantBitFirst, LeastSignificantBitFirst] | None = (
        DakotaField(
            default=None,
            description="Specify format of integers in the generating matrices",
            dakota={"union_pattern": 2},
        )
    )
    m_max: int = DakotaField(
        default=0,
        ge=0,
        description="log2 of the maximum number of points in the digital net",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.m_max",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    t_max: int = DakotaField(
        default=0,
        ge=0,
        description="Bit depth of the generating matrices",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.t_max",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    t_scramble: int = DakotaField(
        default=64,
        ge=0,
        description="Number of rows in the affine scramble matrices",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.t_scramble",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    generating_matrices: (
        Union[
            GeneratingMatricesInline,
            GeneratingMatricesFile,
            GeneratingMatricesPredefined,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Specify the generating matrices of this digital net",
        dakota={"union_pattern": 2},
    )
    ordering: Union[DigitalNetOptionsOrderingNatural, GrayCode] | None = DakotaField(
        default=None,
        description="Ordering of the points of this digital net",
        dakota={"union_pattern": 2},
    )


class EgoSubProblemSolverContext2GPSurfpackConfig(DakotaBaseModel):
    "Use the Surfpack version of Gaussian Process surrogates"

    export_model: EgoSubProblemSolverContext2GPSurfpackExportModel | None = DakotaField(
        default=None,
        description="Exports surrogate model in user-specified format(s)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_surrogate",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class EgoSubProblemSolverContext2GPExperimentalConfig(DakotaBaseModel):
    "Use the experimental Gaussian Process surrogate"

    export_model: EgoSubProblemSolverContext2GPExperimentalExportModel | None = (
        DakotaField(
            default=None,
            description="Exports surrogate model in user-specified format(s)",
            dakota={
                "materialization": [
                    {
                        "ir_key": "method.export_surrogate",
                        "storage_type": "PRESENCE_TRUE",
                        "ir_value_type": "bool",
                    }
                ]
            },
        )
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


class EgoSubProblemSolverContext2ImportBuildPointsFile(DakotaBaseModel):
    "File containing points you wish to use to build a surrogate"

    filename: str = DakotaField(
        description="File containing points you wish to use to build a surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        EgoSubProblemSolverContext2ImportBuildPointsFileCustomAnnotated,
        EgoSubProblemSolverContext2ImportBuildPointsFileAnnotated,
        EgoSubProblemSolverContext2ImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=EgoSubProblemSolverContext2ImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "EgoSubProblemSolverContext2ImportBuildPointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class EgoSubProblemSolverContext2ExportApproxPointsFile(DakotaBaseModel):
    "Output file for surrogate model value evaluations"

    filename: str = DakotaField(
        description="Output file for surrogate model value evaluations",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        EgoSubProblemSolverContext2ExportApproxPointsFileCustomAnnotated,
        EgoSubProblemSolverContext2ExportApproxPointsFileAnnotated,
        EgoSubProblemSolverContext2ExportApproxPointsFileFreeform,
    ] = DakotaField(
        default_factory=EgoSubProblemSolverContext2ExportApproxPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "EgoSubProblemSolverContext2ExportApproxPointsFileAnnotated",
        },
    )


class EgoSubProblemSolverContext1GPSurfpackConfig(DakotaBaseModel):
    "Use the Surfpack version of Gaussian Process surrogates"

    export_model: EgoSubProblemSolverContext1GPSurfpackExportModel | None = DakotaField(
        default=None,
        description="export_model not active",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_surrogate",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class EgoSubProblemSolverContext1GPExperimentalConfig(DakotaBaseModel):
    "Use the experimental Gaussian Process surrogate"

    export_model: EgoSubProblemSolverContext1GPExperimentalExportModel | None = (
        DakotaField(
            default=None,
            description="export_model not active",
            dakota={
                "materialization": [
                    {
                        "ir_key": "method.export_surrogate",
                        "storage_type": "PRESENCE_TRUE",
                        "ir_value_type": "bool",
                    }
                ]
            },
        )
    )
    options_file: str | None = DakotaField(
        default=None,
        description="``options_file`` not active",
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


class EgoSubProblemSolverContext1ImportBuildPointsFile(DakotaBaseModel):
    "File containing points you wish to use to build a surrogate"

    filename: str = DakotaField(
        description="File containing points you wish to use to build a surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        EgoSubProblemSolverContext1ImportBuildPointsFileCustomAnnotated,
        EgoSubProblemSolverContext1ImportBuildPointsFileAnnotated,
        EgoSubProblemSolverContext1ImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=EgoSubProblemSolverContext1ImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "EgoSubProblemSolverContext1ImportBuildPointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class EgoSubProblemSolverContext1ExportApproxPointsFile(DakotaBaseModel):
    "Output file for surrogate model value evaluations"

    filename: str = DakotaField(
        description="Output file for surrogate model value evaluations",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        EgoSubProblemSolverContext1ExportApproxPointsFileCustomAnnotated,
        EgoSubProblemSolverContext1ExportApproxPointsFileAnnotated,
        EgoSubProblemSolverContext1ExportApproxPointsFileFreeform,
    ] = DakotaField(
        default_factory=EgoSubProblemSolverContext1ExportApproxPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "EgoSubProblemSolverContext1ExportApproxPointsFileAnnotated",
        },
    )


class ImportBuildImportBuildPointsFile(DakotaBaseModel):
    "File containing points you wish to use to build a surrogate"

    filename: str = DakotaField(
        description="File containing points you wish to use to build a surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        ImportBuildPointsFileCustomAnnotated,
        ImportBuildPointsFileAnnotated,
        ImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=ImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "ImportBuildPointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class MethodExportModelContext2Mixin(DakotaBaseModel):
    "Generated model for MethodExportModelContext2Mixin"

    export_model: MethodExportModelContext2ExportModel | None = DakotaField(
        default=None,
        description="export_model not active",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_surrogate",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class MethodExportModelContext1Mixin(DakotaBaseModel):
    "Generated model for MethodExportModelContext1Mixin"

    export_model: MethodExportModelContext1ExportModel | None = DakotaField(
        default=None,
        description="Exports surrogate model in user-specified format(s)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_surrogate",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class MethodGPMixin(DakotaBaseModel):
    "Generated model for MethodGPMixin"

    gaussian_process: MethodGPGP | None = DakotaField(
        default=None, description="gaussian_process", dakota={"aliases": ["kriging"]}
    )


class MethodGlobalSubProblemSolverContext2SbgoGP(DakotaBaseModel):
    "Gaussian Process surrogate model"

    implementation: Union[
        MethodGlobalSubProblemSolverContext2SbgoGPSurfpack,
        MethodGlobalSubProblemSolverContext2SbgoGPDakota,
        MethodGlobalSubProblemSolverContext2SbgoGPExperimental,
    ] = DakotaField(
        description="GP Implementation", dakota={"anchor": True, "union_pattern": 4}
    )


class MethodGlobalSubProblemSolverContext2SbgoImportBuildPointsFile(DakotaBaseModel):
    "File containing points you wish to use to build a surrogate"

    filename: str = DakotaField(
        description="File containing points you wish to use to build a surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        MethodGlobalSubProblemSolverContext2SbgoImportBuildPointsFileCustomAnnotated,
        MethodGlobalSubProblemSolverContext2SbgoImportBuildPointsFileAnnotated,
        MethodGlobalSubProblemSolverContext2SbgoImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=MethodGlobalSubProblemSolverContext2SbgoImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "MethodGlobalSubProblemSolverContext2SbgoImportBuildPointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext2SbgoExportApproxPointsFile(DakotaBaseModel):
    "Output file for surrogate model value evaluations"

    filename: str = DakotaField(
        description="Output file for surrogate model value evaluations",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        MethodGlobalSubProblemSolverContext2SbgoExportApproxPointsFileCustomAnnotated,
        MethodGlobalSubProblemSolverContext2SbgoExportApproxPointsFileAnnotated,
        MethodGlobalSubProblemSolverContext2SbgoExportApproxPointsFileFreeform,
    ] = DakotaField(
        default_factory=MethodGlobalSubProblemSolverContext2SbgoExportApproxPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "MethodGlobalSubProblemSolverContext2SbgoExportApproxPointsFileAnnotated",
        },
    )


class MethodGlobalSubProblemSolverContext2EgoGPSurfpackConfig(DakotaBaseModel):
    "Use the Surfpack version of Gaussian Process surrogates"

    export_model: (
        MethodGlobalSubProblemSolverContext2EgoGPSurfpackExportModel | None
    ) = DakotaField(
        default=None,
        description="Exports surrogate model in user-specified format(s)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_surrogate",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext2EgoGPExperimentalConfig(DakotaBaseModel):
    "Use the experimental Gaussian Process surrogate"

    export_model: (
        MethodGlobalSubProblemSolverContext2EgoGPExperimentalExportModel | None
    ) = DakotaField(
        default=None,
        description="Exports surrogate model in user-specified format(s)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_surrogate",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
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


class MethodGlobalSubProblemSolverContext2EgoImportBuildPointsFile(DakotaBaseModel):
    "File containing points you wish to use to build a surrogate"

    filename: str = DakotaField(
        description="File containing points you wish to use to build a surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        MethodGlobalSubProblemSolverContext2EgoImportBuildPointsFileCustomAnnotated,
        MethodGlobalSubProblemSolverContext2EgoImportBuildPointsFileAnnotated,
        MethodGlobalSubProblemSolverContext2EgoImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=MethodGlobalSubProblemSolverContext2EgoImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "MethodGlobalSubProblemSolverContext2EgoImportBuildPointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext2EgoExportApproxPointsFile(DakotaBaseModel):
    "Output file for surrogate model value evaluations"

    filename: str = DakotaField(
        description="Output file for surrogate model value evaluations",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        MethodGlobalSubProblemSolverContext2EgoExportApproxPointsFileCustomAnnotated,
        MethodGlobalSubProblemSolverContext2EgoExportApproxPointsFileAnnotated,
        MethodGlobalSubProblemSolverContext2EgoExportApproxPointsFileFreeform,
    ] = DakotaField(
        default_factory=MethodGlobalSubProblemSolverContext2EgoExportApproxPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "MethodGlobalSubProblemSolverContext2EgoExportApproxPointsFileAnnotated",
        },
    )


class MethodGlobalSubProblemSolverContext1SbgoGP(DakotaBaseModel):
    "Gaussian Process surrogate model"

    implementation: Union[
        MethodGlobalSubProblemSolverContext1SbgoGPSurfpack,
        MethodGlobalSubProblemSolverContext1SbgoGPDakota,
        MethodGlobalSubProblemSolverContext1SbgoGPExperimental,
    ] = DakotaField(
        description="GP Implementation", dakota={"anchor": True, "union_pattern": 4}
    )


class MethodGlobalSubProblemSolverContext1SbgoImportBuildPointsFile(DakotaBaseModel):
    "File containing points you wish to use to build a surrogate"

    filename: str = DakotaField(
        description="File containing points you wish to use to build a surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        MethodGlobalSubProblemSolverContext1SbgoImportBuildPointsFileCustomAnnotated,
        MethodGlobalSubProblemSolverContext1SbgoImportBuildPointsFileAnnotated,
        MethodGlobalSubProblemSolverContext1SbgoImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=MethodGlobalSubProblemSolverContext1SbgoImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "MethodGlobalSubProblemSolverContext1SbgoImportBuildPointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext1SbgoExportApproxPointsFile(DakotaBaseModel):
    "Output file for surrogate model value evaluations"

    filename: str = DakotaField(
        description="Output file for surrogate model value evaluations",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        MethodGlobalSubProblemSolverContext1SbgoExportApproxPointsFileCustomAnnotated,
        MethodGlobalSubProblemSolverContext1SbgoExportApproxPointsFileAnnotated,
        MethodGlobalSubProblemSolverContext1SbgoExportApproxPointsFileFreeform,
    ] = DakotaField(
        default_factory=MethodGlobalSubProblemSolverContext1SbgoExportApproxPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "MethodGlobalSubProblemSolverContext1SbgoExportApproxPointsFileAnnotated",
        },
    )


class MethodGlobalSubProblemSolverContext1EgoGPSurfpackConfig(DakotaBaseModel):
    "Use the Surfpack version of Gaussian Process surrogates"

    export_model: (
        MethodGlobalSubProblemSolverContext1EgoGPSurfpackExportModel | None
    ) = DakotaField(
        default=None,
        description="export_model not active",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_surrogate",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext1EgoGPExperimentalConfig(DakotaBaseModel):
    "Use the experimental Gaussian Process surrogate"

    export_model: (
        MethodGlobalSubProblemSolverContext1EgoGPExperimentalExportModel | None
    ) = DakotaField(
        default=None,
        description="export_model not active",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_surrogate",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    options_file: str | None = DakotaField(
        default=None,
        description="``options_file`` not active",
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


class MethodGlobalSubProblemSolverContext1EgoImportBuildPointsFile(DakotaBaseModel):
    "File containing points you wish to use to build a surrogate"

    filename: str = DakotaField(
        description="File containing points you wish to use to build a surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        MethodGlobalSubProblemSolverContext1EgoImportBuildPointsFileCustomAnnotated,
        MethodGlobalSubProblemSolverContext1EgoImportBuildPointsFileAnnotated,
        MethodGlobalSubProblemSolverContext1EgoImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=MethodGlobalSubProblemSolverContext1EgoImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "MethodGlobalSubProblemSolverContext1EgoImportBuildPointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext1EgoExportApproxPointsFile(DakotaBaseModel):
    "Output file for surrogate model value evaluations"

    filename: str = DakotaField(
        description="Output file for surrogate model value evaluations",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        MethodGlobalSubProblemSolverContext1EgoExportApproxPointsFileCustomAnnotated,
        MethodGlobalSubProblemSolverContext1EgoExportApproxPointsFileAnnotated,
        MethodGlobalSubProblemSolverContext1EgoExportApproxPointsFileFreeform,
    ] = DakotaField(
        default_factory=MethodGlobalSubProblemSolverContext1EgoExportApproxPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "MethodGlobalSubProblemSolverContext1EgoExportApproxPointsFileAnnotated",
        },
    )


class MethodGpAlternativesNoExportGP(DakotaBaseModel):
    "Gaussian Process surrogate model"

    implementation: Union[
        MethodGpAlternativesNoExportGPSurfpack,
        MethodGpAlternativesNoExportGPDakota,
        MethodGpAlternativesNoExportGPExperimental,
    ] = DakotaField(
        description="GP Implementation", dakota={"anchor": True, "union_pattern": 4}
    )


class MethodGpAlternativesWithExportContext2GPSurfpackConfig(DakotaBaseModel):
    "Use the Surfpack version of Gaussian Process surrogates"

    export_model: MethodGpAlternativesWithExportContext2GPSurfpackExportModel | None = (
        DakotaField(
            default=None,
            description="Exports surrogate model in user-specified format(s)",
            dakota={
                "materialization": [
                    {
                        "ir_key": "method.export_surrogate",
                        "storage_type": "PRESENCE_TRUE",
                        "ir_value_type": "bool",
                    }
                ]
            },
        )
    )


class MethodGpAlternativesWithExportContext2GPExperimentalConfig(DakotaBaseModel):
    "Use the experimental Gaussian Process surrogate"

    export_model: (
        MethodGpAlternativesWithExportContext2GPExperimentalExportModel | None
    ) = DakotaField(
        default=None,
        description="Exports surrogate model in user-specified format(s)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_surrogate",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
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


class MethodGpAlternativesWithExportContext1GPSurfpackConfig(DakotaBaseModel):
    "Use the Surfpack version of Gaussian Process surrogates"

    export_model: MethodGpAlternativesWithExportContext1GPSurfpackExportModel | None = (
        DakotaField(
            default=None,
            description="export_model not active",
            dakota={
                "materialization": [
                    {
                        "ir_key": "method.export_surrogate",
                        "storage_type": "PRESENCE_TRUE",
                        "ir_value_type": "bool",
                    }
                ]
            },
        )
    )


class MethodGpAlternativesWithExportContext1GPExperimentalConfig(DakotaBaseModel):
    "Use the experimental Gaussian Process surrogate"

    export_model: (
        MethodGpAlternativesWithExportContext1GPExperimentalExportModel | None
    ) = DakotaField(
        default=None,
        description="export_model not active",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_surrogate",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    options_file: str | None = DakotaField(
        default=None,
        description="``options_file`` not active",
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


class CrossoverType(DakotaBaseModel):
    "Select a crossover type for JEGA methods"

    strategy: Union[
        MultiPointBinary, MultiPointParameterizedBinary, MultiPointReal, ShuffleRandom
    ] = DakotaField(
        description="Crossover Type", dakota={"anchor": True, "union_pattern": 4}
    )
    crossover_rate: DakotaFloat | None = DakotaField(
        default=None,
        ge=0,
        description="Specify the probability of a crossover event",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.crossover_type",
                    "storage_type": "TYPE_DATA_COMBINED",
                    "stored_value": "null_crossover",
                    "secondary_ir_key": "method.crossover_rate",
                    "secondary_ir_value_type": "Real",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MethodJegaCommonOptsMutationType(DakotaBaseModel):
    "Select a mutation type for JEGA methods"

    strategy: Union[
        BitRandom,
        MethodJegaCommonOptsMutationTypeReplaceUniform,
        MethodJegaCommonOptsMutationTypeOffsetNormal,
        MethodJegaCommonOptsMutationTypeOffsetCauchy,
        MethodJegaCommonOptsMutationTypeOffsetUniform,
    ] = DakotaField(
        description="Mutation Type", dakota={"anchor": True, "union_pattern": 4}
    )
    mutation_rate: DakotaFloat | None = DakotaField(
        default=None,
        ge=0,
        description="Set probability of a mutation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.mutation_type",
                    "storage_type": "TYPE_DATA_COMBINED",
                    "stored_value": "null_mutation",
                    "secondary_ir_key": "method.mutation_rate",
                    "secondary_ir_value_type": "Real",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MethodMlmfSolverMetricMixin(DakotaBaseModel):
    "Generated model for MethodMlmfSolverMetricMixin"

    solver_metric: (
        Union[
            MethodMlmfSolverMetricAverageEstimatorVariance,
            MethodMlmfSolverMetricNormEstimatorVariance,
            MethodMlmfSolverMetricMaxEstimatorVariance,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Metric employed during numerical solutions in sampling-based multifidelity UQ methods.",
        dakota={"union_pattern": 2},
    )


class MlmfFinalStatsStatisticsQoiStatistics(DakotaBaseModel):
    "Return the quantity of interest (QoI) statistics as the final results of a UQ method"

    qoi_statistics: MlmfFinalStatsStatisticsQoiStatisticsConfig = DakotaField(
        description="Return the quantity of interest (QoI) statistics as the final results of a UQ method",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.final_statistics",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "QOI_STATISTICS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class OnlinePilotFinalStatisticsQoiStatistics(DakotaBaseModel):
    "Return the quantity of interest (QoI) statistics as the final results of a UQ method"

    qoi_statistics: OnlinePilotFinalStatisticsQoiStatisticsConfig = DakotaField(
        description="Return the quantity of interest (QoI) statistics as the final results of a UQ method",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.final_statistics",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "QOI_STATISTICS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class OfflinePilotFinalStatisticsQoiStatistics(DakotaBaseModel):
    "Return the quantity of interest (QoI) statistics as the final results of a UQ method"

    qoi_statistics: OfflinePilotFinalStatisticsQoiStatisticsConfig = DakotaField(
        description="Return the quantity of interest (QoI) statistics as the final results of a UQ method",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.final_statistics",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "QOI_STATISTICS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExportApproxVarianceFile(DakotaBaseModel):
    "Output file for surrogate model variance evaluations"

    filename: str = DakotaField(
        description="Output file for surrogate model variance evaluations",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.export_approx_variance_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        ExportApproxVarianceFileCustomAnnotated,
        ExportApproxVarianceFileAnnotated,
        ExportApproxVarianceFileFreeform,
    ] = DakotaField(
        default_factory=ExportApproxVarianceFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "ExportApproxVarianceFileAnnotated",
        },
    )


class LevelMappingsNoRelContext2Mixin(
    DistributionCumulComplContext1Mixin,
    GenReliabilityLevelsMixin,
    ProbabilityLevelsContext2Mixin,
    ResponseLevelsComputeProbGenContext2Mixin,
):
    "Generated model for LevelMappingsNoRelContext2Mixin"

    pass


class LevelMappingsNoRelContext1Mixin(
    DistributionCumulComplContext1Mixin,
    GenReliabilityLevelsMixin,
    ProbabilityLevelsContext2Mixin,
    ResponseLevelsComputeProbGenContext1Mixin,
):
    "Generated model for LevelMappingsNoRelContext1Mixin"

    pass


class PromotedModelSelectionContext2Mixin(DakotaBaseModel):
    "Generated model for PromotedModelSelectionContext2Mixin"

    search_model_graphs: PromotedModelSelectionContext2SearchModelGraphs | None = (
        DakotaField(
            default=None,
            description="For weighted multilevel Monte Carlo, this option activates a search over possible hierarchical model graphs",
        )
    )


class PromotedModelSelectionContext1Mixin(DakotaBaseModel):
    "Generated model for PromotedModelSelectionContext1Mixin"

    search_model_graphs: PromotedModelSelectionContext1SearchModelGraphs | None = (
        DakotaField(
            default=None,
            description="Perform a search over admissible model relationships for a given model ensemble",
        )
    )


class Rank1LatticeOptionsMixin(DakotaBaseModel):
    "Generated model for Rank1LatticeOptionsMixin"

    no_random_shift: Literal[True] | None = DakotaField(
        default=None,
        description="Do not shift this rank-1 lattice",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.no_random_shift",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    m_max: int = DakotaField(
        default=0,
        ge=0,
        description="log2 of the maximum number of points in the lattice",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.m_max",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    generating_vector: (
        Union[GeneratingVectorInline, GeneratingVectorFile, GeneratingVectorPredefined]
        | None
    ) = DakotaField(
        default=None,
        description="Specify the generating vector of this rank-1 lattice rule",
        dakota={"union_pattern": 2},
    )
    ordering: Union[Rank1LatticeOptionsOrderingNatural, RadicalInverse] | None = (
        DakotaField(
            default=None,
            description="Ordering of the points of this rank-1 lattice",
            dakota={"union_pattern": 2},
        )
    )


class LevelMappingsMixin(
    DistributionCumulComplContext1Mixin,
    GenReliabilityLevelsMixin,
    ProbabilityLevelsContext2Mixin,
    ReliabilityLevelsMixin,
    ResponseLevelsComputeProbRelGenMixin,
):
    "Generated model for LevelMappingsMixin"

    pass


class SbgoSubProblemSolverGP(DakotaBaseModel):
    "Gaussian Process surrogate model"

    implementation: Union[
        SbgoSubProblemSolverGPSurfpack,
        SbgoSubProblemSolverGPDakota,
        SbgoSubProblemSolverGPExperimental,
    ] = DakotaField(
        description="GP Implementation", dakota={"anchor": True, "union_pattern": 4}
    )


class SbgoSubProblemSolverImportBuildPointsFile(DakotaBaseModel):
    "File containing points you wish to use to build a surrogate"

    filename: str = DakotaField(
        description="File containing points you wish to use to build a surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        SbgoSubProblemSolverImportBuildPointsFileCustomAnnotated,
        SbgoSubProblemSolverImportBuildPointsFileAnnotated,
        SbgoSubProblemSolverImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=SbgoSubProblemSolverImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "SbgoSubProblemSolverImportBuildPointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class SbgoSubProblemSolverExportApproxPointsFile(DakotaBaseModel):
    "Output file for surrogate model value evaluations"

    filename: str = DakotaField(
        description="Output file for surrogate model value evaluations",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        SbgoSubProblemSolverExportApproxPointsFileCustomAnnotated,
        SbgoSubProblemSolverExportApproxPointsFileAnnotated,
        SbgoSubProblemSolverExportApproxPointsFileFreeform,
    ] = DakotaField(
        default_factory=SbgoSubProblemSolverExportApproxPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "SbgoSubProblemSolverExportApproxPointsFileAnnotated",
        },
    )


class SbloSubProblemSolverSbgoExportApproxPointsFile(DakotaBaseModel):
    "Output file for surrogate model value evaluations"

    filename: str = DakotaField(
        description="Output file for surrogate model value evaluations",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        SbloSubProblemSolverSbgoExportApproxPointsFileCustomAnnotated,
        SbloSubProblemSolverSbgoExportApproxPointsFileAnnotated,
        SbloSubProblemSolverSbgoExportApproxPointsFileFreeform,
    ] = DakotaField(
        default_factory=SbloSubProblemSolverSbgoExportApproxPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "SbloSubProblemSolverSbgoExportApproxPointsFileAnnotated",
        },
    )


class ScRefinementPRefinement(DakotaBaseModel):
    "Automatic polynomial order refinement"

    p_refinement: Union[
        ScRefinementPRefinementUniform, ScRefinementPRefinementDimAdaptive
    ] = DakotaField(
        description="Automatic polynomial order refinement",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "P_REFINEMENT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ScRefinementHRefinement(DakotaBaseModel):
    "Employ h-refinement to refine the grid"

    h_refinement: Union[
        HRefinementUniform,
        ScRefinementHRefinementDimAdaptive,
        ScRefinementHRefinementLocalAdaptive,
    ] = DakotaField(
        description="Employ h-refinement to refine the grid",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "H_REFINEMENT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ScSGMixin(DakotaBaseModel):
    "Generated model for ScSGMixin"

    sparse_grid_level: ScSGSGLevel = DakotaField(
        default=...,
        description="Level to use in sparse grid integration or interpolation",
        dakota={"argument": "level"},
    )


class ScSGSequenceMixin(DakotaBaseModel):
    "Generated model for ScSGSequenceMixin"

    sparse_grid_level_sequence: ScSGSequenceSGLevelSequence = DakotaField(
        default=...,
        description="Sequence of sparse grid levels used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class VbdSamplingVarianceBasedDecomp(DakotaBaseModel):
    "Activates global sensitivity analysis based on decomposition of response variance into contributions from variables"

    drop_tolerance: DakotaFloat = DakotaField(
        default=-1.0,
        description="Suppresses output of sensitivity indices with values lower than this tolerance",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.vbd_drop_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    vbd_sampling_method: Union[Binned, PickAndFreeze] | None = DakotaField(
        default=None,
        description="The method to use for variance-based decomposition",
        dakota={"union_pattern": 2},
    )


class EgoSubProblemSolverContext2GPSurfpack(DakotaBaseModel):
    "Use the Surfpack version of Gaussian Process surrogates"

    surfpack: EgoSubProblemSolverContext2GPSurfpackConfig = DakotaField(
        description="Use the Surfpack version of Gaussian Process surrogates",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "KRIGING_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class EgoSubProblemSolverContext2GPExperimental(DakotaBaseModel):
    "Use the experimental Gaussian Process surrogate"

    experimental: EgoSubProblemSolverContext2GPExperimentalConfig = DakotaField(
        description="Use the experimental Gaussian Process surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "EXPGP_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class EgoSubProblemSolverContext1GPSurfpack(DakotaBaseModel):
    "Use the Surfpack version of Gaussian Process surrogates"

    surfpack: EgoSubProblemSolverContext1GPSurfpackConfig = DakotaField(
        description="Use the Surfpack version of Gaussian Process surrogates",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "KRIGING_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class EgoSubProblemSolverContext1GPExperimental(DakotaBaseModel):
    "Use the experimental Gaussian Process surrogate"

    experimental: EgoSubProblemSolverContext1GPExperimentalConfig = DakotaField(
        description="Use the experimental Gaussian Process surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "EXPGP_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ImportBuildMixin(DakotaBaseModel):
    "Generated model for ImportBuildMixin"

    import_build_points_file: ImportBuildImportBuildPointsFile | None = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )


class MethodGlobalSubProblemSolverContext2SbgoConfig(DakotaBaseModel):
    "Use the surrogate based optimization method"

    gaussian_process: MethodGlobalSubProblemSolverContext2SbgoGP | None = DakotaField(
        default=None,
        description="Gaussian Process surrogate model",
        dakota={"aliases": ["kriging"]},
    )
    use_derivatives: Literal[True] | None = DakotaField(
        default=None,
        description="Use derivative data to construct surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.derivative_usage",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    import_build_points_file: (
        MethodGlobalSubProblemSolverContext2SbgoImportBuildPointsFile | None
    ) = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )
    export_approx_points_file: (
        MethodGlobalSubProblemSolverContext2SbgoExportApproxPointsFile | None
    ) = DakotaField(
        default=None,
        description="Output file for surrogate model value evaluations",
        dakota={"argument": "filename", "aliases": ["export_points_file"]},
    )


class MethodGlobalSubProblemSolverContext2EgoGPSurfpack(DakotaBaseModel):
    "Use the Surfpack version of Gaussian Process surrogates"

    surfpack: MethodGlobalSubProblemSolverContext2EgoGPSurfpackConfig = DakotaField(
        description="Use the Surfpack version of Gaussian Process surrogates",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "KRIGING_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext2EgoGPExperimental(DakotaBaseModel):
    "Use the experimental Gaussian Process surrogate"

    experimental: MethodGlobalSubProblemSolverContext2EgoGPExperimentalConfig = (
        DakotaField(
            description="Use the experimental Gaussian Process surrogate",
            dakota={
                "materialization": [
                    {
                        "ir_key": "method.nond.emulator",
                        "storage_type": "PRESENCE_ENUM",
                        "stored_value": "EXPGP_EMULATOR",
                        "ir_value_type": "short",
                    }
                ]
            },
        )
    )


class MethodGlobalSubProblemSolverContext1SbgoConfig(DakotaBaseModel):
    "Use the surrogate based optimization method"

    gaussian_process: MethodGlobalSubProblemSolverContext1SbgoGP | None = DakotaField(
        default=None,
        description="Gaussian Process surrogate model",
        dakota={"aliases": ["kriging"]},
    )
    use_derivatives: Literal[True] | None = DakotaField(
        default=None,
        description="Use derivative data to construct surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.derivative_usage",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    import_build_points_file: (
        MethodGlobalSubProblemSolverContext1SbgoImportBuildPointsFile | None
    ) = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )
    export_approx_points_file: (
        MethodGlobalSubProblemSolverContext1SbgoExportApproxPointsFile | None
    ) = DakotaField(
        default=None,
        description="Output file for surrogate model value evaluations",
        dakota={"argument": "filename", "aliases": ["export_points_file"]},
    )


class MethodGlobalSubProblemSolverContext1EgoGPSurfpack(DakotaBaseModel):
    "Use the Surfpack version of Gaussian Process surrogates"

    surfpack: MethodGlobalSubProblemSolverContext1EgoGPSurfpackConfig = DakotaField(
        description="Use the Surfpack version of Gaussian Process surrogates",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "KRIGING_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext1EgoGPExperimental(DakotaBaseModel):
    "Use the experimental Gaussian Process surrogate"

    experimental: MethodGlobalSubProblemSolverContext1EgoGPExperimentalConfig = (
        DakotaField(
            description="Use the experimental Gaussian Process surrogate",
            dakota={
                "materialization": [
                    {
                        "ir_key": "method.nond.emulator",
                        "storage_type": "PRESENCE_ENUM",
                        "stored_value": "EXPGP_EMULATOR",
                        "ir_value_type": "short",
                    }
                ]
            },
        )
    )


class MethodGpAlternativesNoExportMixin(DakotaBaseModel):
    "Generated model for MethodGpAlternativesNoExportMixin"

    gaussian_process: MethodGpAlternativesNoExportGP | None = DakotaField(
        default=None,
        description="Gaussian Process surrogate model",
        dakota={"aliases": ["kriging"]},
    )


class MethodGpAlternativesWithExportContext2GPSurfpack(DakotaBaseModel):
    "Use the Surfpack version of Gaussian Process surrogates"

    surfpack: MethodGpAlternativesWithExportContext2GPSurfpackConfig = DakotaField(
        description="Use the Surfpack version of Gaussian Process surrogates",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "KRIGING_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodGpAlternativesWithExportContext2GPExperimental(DakotaBaseModel):
    "Use the experimental Gaussian Process surrogate"

    experimental: MethodGpAlternativesWithExportContext2GPExperimentalConfig = (
        DakotaField(
            description="Use the experimental Gaussian Process surrogate",
            dakota={
                "materialization": [
                    {
                        "ir_key": "method.nond.emulator",
                        "storage_type": "PRESENCE_ENUM",
                        "stored_value": "EXPGP_EMULATOR",
                        "ir_value_type": "short",
                    }
                ]
            },
        )
    )


class MethodGpAlternativesWithExportContext1GPSurfpack(DakotaBaseModel):
    "Use the Surfpack version of Gaussian Process surrogates"

    surfpack: MethodGpAlternativesWithExportContext1GPSurfpackConfig = DakotaField(
        description="Use the Surfpack version of Gaussian Process surrogates",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "KRIGING_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodGpAlternativesWithExportContext1GPExperimental(DakotaBaseModel):
    "Use the experimental Gaussian Process surrogate"

    experimental: MethodGpAlternativesWithExportContext1GPExperimentalConfig = (
        DakotaField(
            description="Use the experimental Gaussian Process surrogate",
            dakota={
                "materialization": [
                    {
                        "ir_key": "method.nond.emulator",
                        "storage_type": "PRESENCE_ENUM",
                        "stored_value": "EXPGP_EMULATOR",
                        "ir_value_type": "short",
                    }
                ]
            },
        )
    )


class MethodJegaCommonOptsMixin(DakotaBaseModel):
    "Generated model for MethodJegaCommonOptsMixin"

    population_size: int = DakotaField(
        default=50,
        ge=0,
        description="Set the initial population size in JEGA methods",
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
    log_file: str = DakotaField(
        default="JEGAGlobal.log",
        description="Specify the name of a log file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.log_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    print_each_pop: Literal[True] | None = DakotaField(
        default=None,
        description="Print every population to a population file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.print_each_pop",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    initialization_type: Union[
        MethodJegaCommonOptsInitializationTypeSimpleRandom,
        MethodJegaCommonOptsInitializationTypeUniqueRandom,
        MethodJegaCommonOptsInitializationTypeFlatFile,
    ] = DakotaField(
        default_factory=MethodJegaCommonOptsInitializationTypeUniqueRandom,
        description="Specify how to initialize the population",
        dakota={
            "union_pattern": 1,
            "model_default": "MethodJegaCommonOptsInitializationTypeUniqueRandom",
        },
    )
    crossover_type: CrossoverType | None = DakotaField(
        default=None, description="Select a crossover type for JEGA methods"
    )
    mutation_type: MethodJegaCommonOptsMutationType | None = DakotaField(
        default=None, description="Select a mutation type for JEGA methods"
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
    convergence_tolerance: DakotaFloat = DakotaField(
        default=-DBL_MAX,
        description="Stopping criterion based on objective function or statistics convergence",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.convergence_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
                {
                    "ir_key": "method.jega.percent_change",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
            ]
        },
    )
    model_pointer: str | None = DakotaField(
        default=None,
        description="Identifier for model block to be used by a method",
        dakota={
            "block_pointer": "model",
            "materialization": [
                {
                    "ir_key": "method.model_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class MlmfFinalStatsMixin(DakotaBaseModel):
    "Generated model for MlmfFinalStatsMixin"

    final_statistics: (
        Union[
            MlmfFinalStatsStatisticsEstimatorPerformance,
            MlmfFinalStatsStatisticsQoiStatistics,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Indicate the type of final statistics to be returned by a UQ method",
        dakota={"union_pattern": 2},
    )


class OnlinePilotConfig(DakotaBaseModel):
    "Specify a solution mode that includes the pilot cost within the sample allocation logic"

    relaxation: (
        Union[
            OnlinePilotRelaxationFactorSequence,
            OnlinePilotRelaxationFixedFactor,
            OnlinePilotRelaxationRecursiveFactor,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="For an online pilot mode, apply under-relaxation to the shared sample increments",
        dakota={"union_pattern": 2},
    )
    final_statistics: (
        Union[
            OnlinePilotFinalStatisticsEstimatorPerformance,
            OnlinePilotFinalStatisticsQoiStatistics,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Indicate the type of final statistics to be returned by a UQ method",
        dakota={"union_pattern": 2},
    )


class OfflinePilotConfig(DakotaBaseModel):
    "Specify a solution mode that excludes the pilot cost from sample allocation logic"

    final_statistics: (
        Union[
            OnlinePilotFinalStatisticsEstimatorPerformance,
            OfflinePilotFinalStatisticsQoiStatistics,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Indicate the type of final statistics to be returned by a UQ method",
        dakota={"union_pattern": 2},
    )


class ModelVarianceExportMixin(DakotaBaseModel):
    "Generated model for ModelVarianceExportMixin"

    export_approx_variance_file: ExportApproxVarianceFile | None = DakotaField(
        default=None,
        description="Output file for surrogate model variance evaluations",
        dakota={"argument": "filename"},
    )


class SbgoSubProblemSolverSbgo(DakotaBaseModel):
    "Use the surrogate based optimization method"

    gaussian_process: SbgoSubProblemSolverGP | None = DakotaField(
        default=None,
        description="Gaussian Process surrogate model",
        dakota={"aliases": ["kriging"]},
    )
    use_derivatives: Literal[True] | None = DakotaField(
        default=None,
        description="Use derivative data to construct surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.derivative_usage",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    import_build_points_file: SbgoSubProblemSolverImportBuildPointsFile | None = (
        DakotaField(
            default=None,
            description="File containing points you wish to use to build a surrogate",
            dakota={"argument": "filename", "aliases": ["import_points_file"]},
        )
    )
    export_approx_points_file: SbgoSubProblemSolverExportApproxPointsFile | None = (
        DakotaField(
            default=None,
            description="Output file for surrogate model value evaluations",
            dakota={"argument": "filename", "aliases": ["export_points_file"]},
        )
    )


class SbloSubProblemSolverSbgo(DakotaBaseModel):
    "sblo"

    export_approx_points_file: SbloSubProblemSolverSbgoExportApproxPointsFile | None = (
        DakotaField(
            default=None,
            description="Output file for surrogate model value evaluations",
            dakota={"argument": "filename", "aliases": ["export_points_file"]},
        )
    )


class ScRefinementMixin(DakotaBaseModel):
    "Generated model for ScRefinementMixin"

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
    refinement_method: (
        Union[ScRefinementPRefinement, ScRefinementHRefinement] | None
    ) = DakotaField(
        default=None,
        description="Automated Refinement Type",
        dakota={"anchor": True, "union_pattern": 2},
    )


class VbdSamplingMixin(DakotaBaseModel):
    "Generated model for VbdSamplingMixin"

    variance_based_decomp: VbdSamplingVarianceBasedDecomp | None = DakotaField(
        default=None,
        description="Activates global sensitivity analysis based on decomposition of response variance into contributions from variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.variance_based_decomp",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class EgoSubProblemSolverContext2Ego(DakotaBaseModel):
    "Use the Efficient Global Optimization method"

    gaussian_process: (
        Union[
            EgoSubProblemSolverContext2GPSurfpack,
            EgoSubProblemSolverContext2GPDakota,
            EgoSubProblemSolverContext2GPExperimental,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Gaussian Process surrogate model",
        dakota={"union_pattern": 2, "aliases": ["kriging"]},
    )
    use_derivatives: Literal[True] | None = DakotaField(
        default=None,
        description="Use derivative data to construct surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.derivative_usage",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    import_build_points_file: (
        EgoSubProblemSolverContext2ImportBuildPointsFile | None
    ) = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )
    export_approx_points_file: (
        EgoSubProblemSolverContext2ExportApproxPointsFile | None
    ) = DakotaField(
        default=None,
        description="Output file for surrogate model value evaluations",
        dakota={"argument": "filename", "aliases": ["export_points_file"]},
    )


class EgoSubProblemSolverContext1Ego(DakotaBaseModel):
    "Use the Efficient Global Optimization method"

    gaussian_process: (
        Union[
            EgoSubProblemSolverContext1GPSurfpack,
            EgoSubProblemSolverContext1GPDakota,
            EgoSubProblemSolverContext1GPExperimental,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Gaussian Process surrogate model",
        dakota={"union_pattern": 2, "aliases": ["kriging"]},
    )
    use_derivatives: Literal[True] | None = DakotaField(
        default=None,
        description="Use derivative data to construct surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.derivative_usage",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    import_build_points_file: (
        EgoSubProblemSolverContext1ImportBuildPointsFile | None
    ) = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )
    export_approx_points_file: (
        EgoSubProblemSolverContext1ExportApproxPointsFile | None
    ) = DakotaField(
        default=None,
        description="Output file for surrogate model value evaluations",
        dakota={"argument": "filename", "aliases": ["export_points_file"]},
    )


class MethodGlobalSubProblemSolverContext2Sbgo(DakotaBaseModel):
    "Use the surrogate based optimization method"

    sbgo: MethodGlobalSubProblemSolverContext2SbgoConfig = DakotaField(
        description="Use the surrogate based optimization method",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_SBGO",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext2EgoConfig(DakotaBaseModel):
    "Use the Efficient Global Optimization method"

    gaussian_process: (
        Union[
            MethodGlobalSubProblemSolverContext2EgoGPSurfpack,
            MethodGlobalSubProblemSolverContext2EgoGPDakota,
            MethodGlobalSubProblemSolverContext2EgoGPExperimental,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Gaussian Process surrogate model",
        dakota={"union_pattern": 2, "aliases": ["kriging"]},
    )
    use_derivatives: Literal[True] | None = DakotaField(
        default=None,
        description="Use derivative data to construct surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.derivative_usage",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    import_build_points_file: (
        MethodGlobalSubProblemSolverContext2EgoImportBuildPointsFile | None
    ) = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )
    export_approx_points_file: (
        MethodGlobalSubProblemSolverContext2EgoExportApproxPointsFile | None
    ) = DakotaField(
        default=None,
        description="Output file for surrogate model value evaluations",
        dakota={"argument": "filename", "aliases": ["export_points_file"]},
    )


class MethodGlobalSubProblemSolverContext1Sbgo(DakotaBaseModel):
    "Use the surrogate based optimization method"

    sbgo: MethodGlobalSubProblemSolverContext1SbgoConfig = DakotaField(
        description="Use the surrogate based optimization method",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_SBGO",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext1EgoConfig(DakotaBaseModel):
    "Use the Efficient Global Optimization method"

    gaussian_process: (
        Union[
            MethodGlobalSubProblemSolverContext1EgoGPSurfpack,
            MethodGlobalSubProblemSolverContext1EgoGPDakota,
            MethodGlobalSubProblemSolverContext1EgoGPExperimental,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Gaussian Process surrogate model",
        dakota={"union_pattern": 2, "aliases": ["kriging"]},
    )
    use_derivatives: Literal[True] | None = DakotaField(
        default=None,
        description="Use derivative data to construct surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.derivative_usage",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    import_build_points_file: (
        MethodGlobalSubProblemSolverContext1EgoImportBuildPointsFile | None
    ) = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )
    export_approx_points_file: (
        MethodGlobalSubProblemSolverContext1EgoExportApproxPointsFile | None
    ) = DakotaField(
        default=None,
        description="Output file for surrogate model value evaluations",
        dakota={"argument": "filename", "aliases": ["export_points_file"]},
    )


class MethodGpAlternativesWithExportContext2Mixin(DakotaBaseModel):
    "Generated model for MethodGpAlternativesWithExportContext2Mixin"

    gaussian_process: (
        Union[
            MethodGpAlternativesWithExportContext2GPSurfpack,
            MethodGpAlternativesWithExportContext2GPDakota,
            MethodGpAlternativesWithExportContext2GPExperimental,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Gaussian Process surrogate model",
        dakota={"union_pattern": 2, "aliases": ["kriging"]},
    )


class MethodGpAlternativesWithExportContext1Mixin(DakotaBaseModel):
    "Generated model for MethodGpAlternativesWithExportContext1Mixin"

    gaussian_process: (
        Union[
            MethodGpAlternativesWithExportContext1GPSurfpack,
            MethodGpAlternativesWithExportContext1GPDakota,
            MethodGpAlternativesWithExportContext1GPExperimental,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Gaussian Process surrogate model",
        dakota={"union_pattern": 2, "aliases": ["kriging"]},
    )


class OnlinePilot(DakotaBaseModel):
    "Specify a solution mode that includes the pilot cost within the sample allocation logic"

    online_pilot: OnlinePilotConfig = DakotaField(
        default_factory=OnlinePilotConfig,
        description="Specify a solution mode that includes the pilot cost within the sample allocation logic",
        dakota={
            "model_default": "OnlinePilotConfig",
            "materialization": [
                {
                    "ir_key": "method.nond.ensemble_pilot_solution_mode",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ONLINE_PILOT",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class OfflinePilot(DakotaBaseModel):
    "Specify a solution mode that excludes the pilot cost from sample allocation logic"

    offline_pilot: OfflinePilotConfig = DakotaField(
        default_factory=OfflinePilotConfig,
        description="Specify a solution mode that excludes the pilot cost from sample allocation logic",
        dakota={
            "model_default": "OfflinePilotConfig",
            "materialization": [
                {
                    "ir_key": "method.nond.ensemble_pilot_solution_mode",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "OFFLINE_PILOT",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class SbgoSubProblemSolverMixin(DakotaBaseModel):
    "Generated model for SbgoSubProblemSolverMixin"

    sbgo: SbgoSubProblemSolverSbgo = DakotaField(
        description="Use the surrogate based optimization method",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_SBGO",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class SbloSubProblemSolverMixin(DakotaBaseModel):
    "Generated model for SbloSubProblemSolverMixin"

    sbgo: SbloSubProblemSolverSbgo = DakotaField(description="sblo")


class EgoSubProblemSolverContext2Mixin(DakotaBaseModel):
    "Generated model for EgoSubProblemSolverContext2Mixin"

    ego: EgoSubProblemSolverContext2Ego = DakotaField(
        description="Use the Efficient Global Optimization method",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_EGO",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class EgoSubProblemSolverContext1Mixin(DakotaBaseModel):
    "Generated model for EgoSubProblemSolverContext1Mixin"

    ego: EgoSubProblemSolverContext1Ego = DakotaField(
        description="Use the Efficient Global Optimization method",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_EGO",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext2Ego(DakotaBaseModel):
    "Use the Efficient Global Optimization method"

    ego: MethodGlobalSubProblemSolverContext2EgoConfig = DakotaField(
        description="Use the Efficient Global Optimization method",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_EGO",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MethodGlobalSubProblemSolverContext1Ego(DakotaBaseModel):
    "Use the Efficient Global Optimization method"

    ego: MethodGlobalSubProblemSolverContext1EgoConfig = DakotaField(
        description="Use the Efficient Global Optimization method",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_EGO",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MlmfSolutionModeMixin(DakotaBaseModel):
    "Generated model for MlmfSolutionModeMixin"

    solution_mode: Union[
        OnlinePilot, OfflinePilot, OnlineProjection, OfflineProjection
    ] = DakotaField(
        default_factory=OnlinePilot,
        description="Solution mode for multilevel/multifidelity methods",
        dakota={"union_pattern": 1, "model_default": "OnlinePilot"},
    )


class MethodGlobalSubProblemSolverContext2Mixin(DakotaBaseModel):
    "Generated model for MethodGlobalSubProblemSolverContext2Mixin"

    solution_approach: (
        Union[
            MethodGlobalSubProblemSolverContext2Sbgo,
            MethodGlobalSubProblemSolverContext2Ego,
            MethodGlobalSubProblemSolverContext2Ea,
            MethodGlobalSubProblemSolverContext2Lhs,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Solution Approach",
        dakota={"anchor": True, "union_pattern": 2},
    )


class MethodGlobalSubProblemSolverContext1Mixin(DakotaBaseModel):
    "Generated model for MethodGlobalSubProblemSolverContext1Mixin"

    solution_approach: (
        Union[
            MethodGlobalSubProblemSolverContext1Sbgo,
            MethodGlobalSubProblemSolverContext1Ego,
            MethodGlobalSubProblemSolverContext1Ea,
            MethodGlobalSubProblemSolverContext1Lhs,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Solution Approach",
        dakota={"anchor": True, "union_pattern": 2},
    )
