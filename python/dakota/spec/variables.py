"""Generated Pydantic models for variables"""

from __future__ import annotations

from .base import DakotaBaseModel, DakotaField, DakotaFloat
from typing import Literal, Union
from typing import ClassVar, List
from .validation import ValidationRule
from .validation.rules import (
    CheckAdjacencyMatrix,
    CheckAllVariableDescriptorsUnique,
    CheckDescriptorsValid,
    CheckIntLowerBound,
    CheckLinearInequalityBoundsOrdering,
    CheckPermittedValues,
    CheckRealLowerBound,
    CheckRealUpperBound,
    CheckScalesRequired,
    CheckSumEqualsLength,
    CheckVariableBoundsOrdering,
    CheckVariableDescriptorsLength,
    CompareLength,
    CompareLengthOne,
    CompareListLengths,
    DefaultBoundsInt,
    DefaultBoundsReal,
    DefaultInitialPointInt,
    DefaultInitialPointReal,
    DefaultVariableDescriptors,
)
from .validation.variable_rules import (
    BetaUncertainInitial,
    BinomialUncertainInitial,
    CheckSetElementsOrdering,
    ContinuousIntervalUncertainInitial,
    DefaultSetProbabilities,
    DiscreteDesignSetIntInitial,
    DiscreteDesignSetRealInitial,
    DiscreteDesignSetStrInitial,
    DiscreteIntervalUncertainInitial,
    DiscreteStateSetIntInitial,
    DiscreteStateSetRealInitial,
    DiscreteStateSetStrInitial,
    DiscreteUncertainSetIntInitial,
    DiscreteUncertainSetRealInitial,
    DiscreteUncertainSetStrInitial,
    ExponentialUncertainInitial,
    FrechetUncertainInitial,
    GammaUncertainInitial,
    GeometricUncertainInitial,
    GumbelUncertainInitial,
    HistogramBinUncertainInitial,
    HistogramPointIntUncertainInitial,
    HistogramPointRealUncertainInitial,
    HistogramPointStrUncertainInitial,
    HypergeometricUncertainInitial,
    LognormalUncertainBounds,
    LognormalUncertainInitial,
    LoguniformUncertainInitial,
    NegativeBinomialUncertainInitial,
    NormalUncertainBounds,
    NormalUncertainInitial,
    PoissonUncertainInitial,
    TriangularUncertainInitial,
    UncertainCorrelationMatrixSize,
    UniformUncertainInitial,
    WeibullUncertainInitial,
)
from math import inf
from pydantic import computed_field
from .base import ComputedFieldSpec
from .validation.computed_fields import (
    BinomialBounds,
    ContinuousIntervalBounds,
    DiscreteIntervalBounds,
    DiscreteSetIntBounds,
    DiscreteSetRealBounds,
    DiscreteSetStrBounds,
    ExponentialBounds,
    FrechetBounds,
    GammaBounds,
    GeometricBounds,
    GumbelBounds,
    HistogramBinBounds,
    HistogramPointIntBounds,
    HistogramPointRealBounds,
    HistogramPointStrBounds,
    HypergeometricBounds,
    NegativeBinomialBounds,
    PoissonBounds,
    WeibullBounds,
)


class ActiveAll(DakotaBaseModel):
    """Generated model for ActiveAll"""

    all: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.view",
                    "stored_value": "ALL_VIEW",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class Design(DakotaBaseModel):
    """Generated model for Design"""

    design: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.view",
                    "stored_value": "DESIGN_VIEW",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class Uncertain(DakotaBaseModel):
    """Generated model for Uncertain"""

    uncertain: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.view",
                    "stored_value": "UNCERTAIN_VIEW",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class Aleatory(DakotaBaseModel):
    """Generated model for Aleatory"""

    aleatory: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.view",
                    "stored_value": "ALEATORY_UNCERTAIN_VIEW",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class Epistemic(DakotaBaseModel):
    """Generated model for Epistemic"""

    epistemic: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.view",
                    "stored_value": "EPISTEMIC_UNCERTAIN_VIEW",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class State(DakotaBaseModel):
    """Generated model for State"""

    state: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.view",
                    "stored_value": "STATE_VIEW",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class Mixed(DakotaBaseModel):
    """Generated model for Mixed"""

    mixed: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.domain",
                    "stored_value": "MIXED_DOMAIN",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class Relaxed(DakotaBaseModel):
    """Generated model for Relaxed"""

    relaxed: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.domain",
                    "stored_value": "RELAXED_DOMAIN",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ContinuousDesign(DakotaBaseModel):
    """Generated model for ContinuousDesign"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="continuous_design",
            list_field="initial_point",
            target_field="count",
        ),
        CompareLength(
            context="continuous_design", list_field="lower_bounds", target_field="count"
        ),
        CompareLength(
            context="continuous_design", list_field="upper_bounds", target_field="count"
        ),
        CompareLength(
            context="continuous_design", list_field="descriptors", target_field="count"
        ),
        CompareLengthOne(
            context="continuous_design", list_field="scale_types", target_field="count"
        ),
        CompareLengthOne(
            context="continuous_design", list_field="scales", target_field="count"
        ),
        CheckVariableDescriptorsLength(context="continuous_design"),
        CheckDescriptorsValid(context="continuous_design", check_uniqueness=False),
        DefaultVariableDescriptors(context="continuous_design", prefix="cdv_"),
        DefaultBoundsReal(context="continuous_design", literals=[-inf, inf]),
        DefaultInitialPointReal(
            context="continuous_design", target_field="initial_point"
        ),
        CheckVariableBoundsOrdering(context="continuous_design"),
        CheckPermittedValues(
            context="continuous_design",
            field_name="scale_types",
            permitted_values=["value", "auto", "log", "none"],
        ),
        CheckScalesRequired(
            context="continuous_design",
            scale_types_field="scale_types",
            scales_field="scales",
        ),
    ]

    count: int = DakotaField(
        gt=0,
        description="Design variable - continuous",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.continuous_design",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.continuous",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.design",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    initial_point: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "aliases": ["cdv_initial_point"],
            "materialization": [
                {
                    "ir_key": "variables.continuous_design.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    lower_bounds: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Specify minimum values",
        dakota={
            "aliases": ["cdv_lower_bounds"],
            "materialization": [
                {
                    "ir_key": "variables.continuous_design.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    upper_bounds: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Specify maximium values",
        dakota={
            "aliases": ["cdv_upper_bounds"],
            "materialization": [
                {
                    "ir_key": "variables.continuous_design.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    scale_types: list[str] | None = DakotaField(
        default=None,
        description="How to scale each continuous design variable",
        dakota={
            "aliases": ["cdv_scale_types"],
            "materialization": [
                {
                    "ir_key": "variables.continuous_design.scale_types",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )
    scales: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Characteristic values to scale continuous design variables",
        dakota={
            "aliases": ["cdv_scales"],
            "materialization": [
                {
                    "ir_key": "variables.continuous_design.scales",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "aliases": ["cdv_descriptors"],
            "materialization": [
                {
                    "ir_key": "variables.continuous_design.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )


class DiscreteDesignRange(DakotaBaseModel):
    """Generated model for DiscreteDesignRange"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="discrete_design_range",
            list_field="initial_point",
            target_field="count",
        ),
        CompareLength(
            context="discrete_design_range",
            list_field="lower_bounds",
            target_field="count",
        ),
        CompareLength(
            context="discrete_design_range",
            list_field="upper_bounds",
            target_field="count",
        ),
        CompareLength(
            context="discrete_design_range",
            list_field="descriptors",
            target_field="count",
        ),
        CheckVariableDescriptorsLength(context="discrete_design_range"),
        CheckDescriptorsValid(context="discrete_design_range", check_uniqueness=False),
        DefaultVariableDescriptors(context="discrete_design_range", prefix="ddriv_"),
        DefaultBoundsInt(
            context="discrete_design_range", literals=[-2147483648, 2147483647]
        ),
        DefaultInitialPointInt(
            context="discrete_design_range", target_field="initial_point"
        ),
        CheckVariableBoundsOrdering(context="discrete_design_range"),
    ]

    count: int = DakotaField(
        gt=0,
        description="Design variable - discrete range-valued",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_design_range",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.discrete",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.design",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    initial_point: list[int] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "aliases": ["ddv_initial_point"],
            "materialization": [
                {
                    "ir_key": "variables.discrete_design_range.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        },
    )
    lower_bounds: list[int] | None = DakotaField(
        default=None,
        description="Specify minimum values",
        dakota={
            "aliases": ["ddv_lower_bounds"],
            "materialization": [
                {
                    "ir_key": "variables.discrete_design_range.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        },
    )
    upper_bounds: list[int] | None = DakotaField(
        default=None,
        description="Specify maximium values",
        dakota={
            "aliases": ["ddv_upper_bounds"],
            "materialization": [
                {
                    "ir_key": "variables.discrete_design_range.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "aliases": ["ddv_descriptors"],
            "materialization": [
                {
                    "ir_key": "variables.discrete_design_range.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )


class IntegerCategorical(DakotaBaseModel):
    """Generated model for IntegerCategorical"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckPermittedValues(
            context="categorical",
            field_name="flags",
            permitted_values=["yes", "no", "true", "false", "y", "n", "t", "f"],
        ),
    ]

    flags: list[str] = DakotaField(
        description="Whether the set-valued variables are categorical or relaxable",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_design_set_int.categorical",
                    "storage_type": "CATEGORICAL",
                    "ir_value_type": "BitArray",
                }
            ]
        },
    )
    adjacency_matrix: list[int] | None = DakotaField(
        default=None,
        description="1-0 matrix defining which categorical variable levels are related.",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_design_set_int.adjacency_matrix",
                    "storage_type": "ADJACENCY_MATRIX",
                    "ir_value_type": "RealMatrixArray",
                }
            ]
        },
    )


class DiscreteDesignSetString(DakotaBaseModel):
    """Generated model for DiscreteDesignSetString"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="variables_discrete_design_set_string",
            list_field="initial_point",
            target_field="count",
        ),
        CompareLength(
            context="variables_discrete_design_set_string",
            list_field="descriptors",
            target_field="count",
        ),
        CompareLengthOne(
            context="variables_discrete_design_set_string",
            list_field="elements_per_variable",
            target_field="count",
        ),
        CheckVariableDescriptorsLength(context="string"),
        CheckDescriptorsValid(context="string", check_uniqueness=False),
        DefaultVariableDescriptors(context="string", prefix="ddssv_"),
        DiscreteDesignSetStrInitial(context="string"),
        CheckSumEqualsLength(
            context="string",
            num_list_field="elements_per_variable",
            levels_list_field="elements",
        ),
        CheckSetElementsOrdering(context="string"),
        CheckAdjacencyMatrix(
            context="string",
            adjacency_matrix_path="adjacency_matrix",
            flags_path="",
            elements_per_variable_path="elements_per_variable",
            elements_path="elements",
            count_path="count",
        ),
    ]

    count: int = DakotaField(
        gt=0,
        description="String-valued discrete design set variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_design_set_string",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.discrete",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.design",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    elements_per_variable: list[int] | None = DakotaField(
        default=None,
        description="Number of admissible elements for each set variable",
        dakota={"aliases": ["num_set_values"]},
    )
    elements: list[str] = DakotaField(
        description="The permissible values for each discrete variable",
        dakota={
            "aliases": ["set_values"],
            "materialization": [
                {
                    "ir_key": "variables.discrete_design_set_string.values",
                    "storage_type": "DISCRETE_SET_VALUES",
                    "ir_value_type": "StringSetArray",
                }
            ],
        },
    )
    adjacency_matrix: list[int] | None = DakotaField(
        default=None,
        description="1-0 matrix defining which categorical variable levels are related.",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_design_set_str.adjacency_matrix",
                    "storage_type": "ADJACENCY_MATRIX",
                    "ir_value_type": "RealMatrixArray",
                }
            ]
        },
    )
    initial_point: list[str] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_design_set_string.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_design_set_string.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )

    _COMPUTED_FIELDS: ClassVar[dict[str, ComputedFieldSpec]] = {
        "lower_bounds": ComputedFieldSpec(
            function="discrete_set_str_lower_bounds",
            description="Lower bounds: min element per variable",
            return_type=list[str],
            materialization=[
                {
                    "ir_key": "variables.discrete_design_set_string.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        ),
        "upper_bounds": ComputedFieldSpec(
            function="discrete_set_str_upper_bounds",
            description="Upper bounds: max element per variable",
            return_type=list[str],
            materialization=[
                {
                    "ir_key": "variables.discrete_design_set_string.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        ),
    }

    @computed_field
    @property
    def lower_bounds(self) -> List[str]:
        """Lower bounds: min element per variable"""
        return DiscreteSetStrBounds.lower_bounds(self)

    @computed_field
    @property
    def upper_bounds(self) -> List[str]:
        """Upper bounds: max element per variable"""
        return DiscreteSetStrBounds.upper_bounds(self)


class RealCategorical(DakotaBaseModel):
    """Generated model for RealCategorical"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckPermittedValues(
            context="categorical",
            field_name="flags",
            permitted_values=["yes", "no", "true", "false", "y", "n", "t", "f"],
        ),
    ]

    flags: list[str] = DakotaField(
        description="Whether the set-valued variables are categorical or relaxable",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_design_set_real.categorical",
                    "storage_type": "CATEGORICAL",
                    "ir_value_type": "BitArray",
                }
            ]
        },
    )
    adjacency_matrix: list[int] | None = DakotaField(
        default=None,
        description="1-0 matrix defining which categorical variable levels are related.",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_design_set_real.adjacency_matrix",
                    "storage_type": "ADJACENCY_MATRIX",
                    "ir_value_type": "RealMatrixArray",
                }
            ]
        },
    )


class NormalUncertain(DakotaBaseModel):
    """Generated model for NormalUncertain"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="normal_uncertain", list_field="means", target_field="count"
        ),
        CompareLength(
            context="normal_uncertain",
            list_field="std_deviations",
            target_field="count",
        ),
        CompareLength(
            context="normal_uncertain", list_field="lower_bounds", target_field="count"
        ),
        CompareLength(
            context="normal_uncertain", list_field="upper_bounds", target_field="count"
        ),
        CompareLength(
            context="normal_uncertain", list_field="initial_point", target_field="count"
        ),
        CompareLength(
            context="normal_uncertain", list_field="descriptors", target_field="count"
        ),
        CheckVariableDescriptorsLength(context="normal_uncertain"),
        CheckDescriptorsValid(context="normal_uncertain", check_uniqueness=False),
        DefaultVariableDescriptors(context="normal_uncertain", prefix="nuv_"),
        NormalUncertainBounds(context="normal_uncertain"),
        NormalUncertainInitial(context="normal_uncertain"),
        CheckVariableBoundsOrdering(context="normal_uncertain"),
        CheckRealLowerBound(
            context="normal_uncertain", list_field="std_deviations", lower_bound=0
        ),
    ]

    count: int = DakotaField(
        gt=0,
        description="Aleatory uncertain variable - normal (Gaussian)",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.normal_uncertain",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.continuous",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.aleatory_uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    means: list[DakotaFloat] = DakotaField(
        description="First parameter of the distribution",
        dakota={
            "aliases": ["nuv_means"],
            "materialization": [
                {
                    "ir_key": "variables.normal_uncertain.means",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    std_deviations: list[DakotaFloat] = DakotaField(
        description="Second parameter of the distribution",
        dakota={
            "aliases": ["nuv_std_deviations"],
            "materialization": [
                {
                    "ir_key": "variables.normal_uncertain.std_deviations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    lower_bounds: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Specify minimum values",
        dakota={
            "aliases": ["nuv_lower_bounds"],
            "materialization": [
                {
                    "ir_key": "variables.normal_uncertain.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    upper_bounds: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Specify maximium values",
        dakota={
            "aliases": ["nuv_upper_bounds"],
            "materialization": [
                {
                    "ir_key": "variables.normal_uncertain.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    initial_point: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.normal_uncertain.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "aliases": ["nuv_descriptors"],
            "materialization": [
                {
                    "ir_key": "variables.normal_uncertain.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )

    # Extra fields (not derived from XML grammar)
    initial_point_user_provided: bool = DakotaField(
        default=False,
        description="Internal-only flag indicating the user explicitly provided initial_point.",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.uncertain.initial_point_flag",
                    "storage_type": "UNCERTAIN_INIT_POINT_FLAG",
                    "ir_value_type": "bool",
                }
            ],
        },
    )
    inferred_lower_bounds: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Internal use only",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.normal_uncertain.inferred_lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    inferred_upper_bounds: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Internal use only",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.normal_uncertain.inferred_upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )


class Option1Config(DakotaBaseModel):
    """Generated model for Option1Config"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckRealLowerBound(context="option_1", list_field="means", lower_bound=0),
        CheckRealLowerBound(
            context="option_1", list_field="std_deviations", lower_bound=0
        ),
    ]

    means: list[DakotaFloat] = DakotaField(
        description="First parameter of the lognormal distribution (option 1)",
        dakota={
            "aliases": ["lnuv_means"],
            "materialization": [
                {
                    "ir_key": "variables.lognormal_uncertain.means",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    std_deviations: list[DakotaFloat] = DakotaField(
        description="Second parameter of the lognormal distribution (option 1)",
        dakota={
            "aliases": ["lnuv_std_deviations"],
            "materialization": [
                {
                    "ir_key": "variables.lognormal_uncertain.std_deviations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )


class Option2Config(DakotaBaseModel):
    """Generated model for Option2Config"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckRealLowerBound(context="option_2", list_field="means", lower_bound=0),
        CheckRealLowerBound(
            context="option_2", list_field="error_factors", lower_bound=0
        ),
    ]

    means: list[DakotaFloat] = DakotaField(
        description="First parameter of the lognormal distribution (option 1)",
        dakota={
            "aliases": ["lnuv_means"],
            "materialization": [
                {
                    "ir_key": "variables.lognormal_uncertain.means",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    error_factors: list[DakotaFloat] = DakotaField(
        description="Second parameter of the lognormal distribution (option 2)",
        dakota={
            "aliases": ["lnuv_error_factors"],
            "materialization": [
                {
                    "ir_key": "variables.lognormal_uncertain.error_factors",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )


class Option3Config(DakotaBaseModel):
    """Generated model for Option3Config"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckRealLowerBound(context="option_3", list_field="zetas", lower_bound=0),
    ]

    lambdas: list[DakotaFloat] = DakotaField(
        description="First parameter of the lognormal distribution (option 3)",
        dakota={
            "aliases": ["lnuv_lambdas"],
            "materialization": [
                {
                    "ir_key": "variables.lognormal_uncertain.lambdas",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    zetas: list[DakotaFloat] = DakotaField(
        description="Second parameter of the lognormal distribution (option 2)",
        dakota={
            "aliases": ["lnuv_zetas"],
            "materialization": [
                {
                    "ir_key": "variables.lognormal_uncertain.zetas",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )


class UniformUncertain(DakotaBaseModel):
    """Generated model for UniformUncertain"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="uniform_uncertain", list_field="lower_bounds", target_field="count"
        ),
        CompareLength(
            context="uniform_uncertain", list_field="upper_bounds", target_field="count"
        ),
        CompareLength(
            context="uniform_uncertain",
            list_field="initial_point",
            target_field="count",
        ),
        CompareLength(
            context="uniform_uncertain", list_field="descriptors", target_field="count"
        ),
        CheckVariableDescriptorsLength(context="uniform_uncertain"),
        CheckDescriptorsValid(context="uniform_uncertain", check_uniqueness=False),
        DefaultVariableDescriptors(context="uniform_uncertain", prefix="uuv_"),
        UniformUncertainInitial(context="uniform_uncertain"),
        CheckVariableBoundsOrdering(context="uniform_uncertain"),
        CheckRealLowerBound(
            context="uniform_uncertain", list_field="lower_bounds", lower_bound=-inf
        ),
        CheckRealUpperBound(
            context="uniform_uncertain", list_field="upper_bounds", upper_bound=inf
        ),
    ]

    count: int = DakotaField(
        gt=0,
        description="Aleatory uncertain variable - uniform",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.uniform_uncertain",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.continuous",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.aleatory_uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    lower_bounds: list[DakotaFloat] = DakotaField(
        description="Specify minimum values",
        dakota={
            "aliases": ["uuv_lower_bounds"],
            "materialization": [
                {
                    "ir_key": "variables.uniform_uncertain.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    upper_bounds: list[DakotaFloat] = DakotaField(
        description="Specify maximium values",
        dakota={
            "aliases": ["uuv_upper_bounds"],
            "materialization": [
                {
                    "ir_key": "variables.uniform_uncertain.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    initial_point: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.uniform_uncertain.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "aliases": ["uuv_descriptors"],
            "materialization": [
                {
                    "ir_key": "variables.uniform_uncertain.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )

    # Extra fields (not derived from XML grammar)
    initial_point_user_provided: bool = DakotaField(
        default=False,
        description="Internal-only flag indicating the user explicitly provided initial_point.",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.uncertain.initial_point_flag",
                    "storage_type": "UNCERTAIN_INIT_POINT_FLAG",
                    "ir_value_type": "bool",
                }
            ],
        },
    )


class LoguniformUncertain(DakotaBaseModel):
    """Generated model for LoguniformUncertain"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="loguniform_uncertain",
            list_field="lower_bounds",
            target_field="count",
        ),
        CompareLength(
            context="loguniform_uncertain",
            list_field="upper_bounds",
            target_field="count",
        ),
        CompareLength(
            context="loguniform_uncertain",
            list_field="initial_point",
            target_field="count",
        ),
        CompareLength(
            context="loguniform_uncertain",
            list_field="descriptors",
            target_field="count",
        ),
        CheckVariableDescriptorsLength(context="loguniform_uncertain"),
        CheckDescriptorsValid(context="loguniform_uncertain", check_uniqueness=False),
        DefaultVariableDescriptors(context="loguniform_uncertain", prefix="luuv_"),
        LoguniformUncertainInitial(context="loguniform_uncertain"),
        CheckVariableBoundsOrdering(context="loguniform_uncertain"),
        CheckRealLowerBound(
            context="loguniform_uncertain", list_field="lower_bounds", lower_bound=0
        ),
        CheckRealUpperBound(
            context="loguniform_uncertain", list_field="upper_bounds", upper_bound=inf
        ),
    ]

    count: int = DakotaField(
        gt=0,
        description="Aleatory uncertain variable - loguniform",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.loguniform_uncertain",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.continuous",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.aleatory_uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    lower_bounds: list[DakotaFloat] = DakotaField(
        description="Specify minimum values",
        dakota={
            "aliases": ["luuv_lower_bounds"],
            "materialization": [
                {
                    "ir_key": "variables.loguniform_uncertain.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    upper_bounds: list[DakotaFloat] = DakotaField(
        description="Specify maximium values",
        dakota={
            "aliases": ["luuv_upper_bounds"],
            "materialization": [
                {
                    "ir_key": "variables.loguniform_uncertain.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    initial_point: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.loguniform_uncertain.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "aliases": ["luuv_descriptors"],
            "materialization": [
                {
                    "ir_key": "variables.loguniform_uncertain.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )

    # Extra fields (not derived from XML grammar)
    initial_point_user_provided: bool = DakotaField(
        default=False,
        description="Internal-only flag indicating the user explicitly provided initial_point.",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.uncertain.initial_point_flag",
                    "storage_type": "UNCERTAIN_INIT_POINT_FLAG",
                    "ir_value_type": "bool",
                }
            ],
        },
    )


class TriangularUncertain(DakotaBaseModel):
    """Generated model for TriangularUncertain"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="triangular_uncertain", list_field="modes", target_field="count"
        ),
        CompareLength(
            context="triangular_uncertain",
            list_field="lower_bounds",
            target_field="count",
        ),
        CompareLength(
            context="triangular_uncertain",
            list_field="upper_bounds",
            target_field="count",
        ),
        CompareLength(
            context="triangular_uncertain",
            list_field="initial_point",
            target_field="count",
        ),
        CompareLength(
            context="triangular_uncertain",
            list_field="descriptors",
            target_field="count",
        ),
        CheckVariableDescriptorsLength(context="triangular_uncertain"),
        CheckDescriptorsValid(context="triangular_uncertain", check_uniqueness=False),
        DefaultVariableDescriptors(context="triangular_uncertain", prefix="tuv_"),
        TriangularUncertainInitial(context="triangular_uncertain"),
        CheckVariableBoundsOrdering(context="triangular_uncertain"),
        CheckRealLowerBound(
            context="triangular_uncertain", list_field="lower_bounds", lower_bound=-inf
        ),
        CheckRealUpperBound(
            context="triangular_uncertain", list_field="upper_bounds", upper_bound=inf
        ),
    ]

    count: int = DakotaField(
        gt=0,
        description="Aleatory uncertain variable - triangular",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.triangular_uncertain",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.continuous",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.aleatory_uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    modes: list[DakotaFloat] = DakotaField(
        description="Distribution parameter",
        dakota={
            "aliases": ["tuv_modes"],
            "materialization": [
                {
                    "ir_key": "variables.triangular_uncertain.modes",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    lower_bounds: list[DakotaFloat] = DakotaField(
        description="Specify minimum values",
        dakota={
            "aliases": ["tuv_lower_bounds"],
            "materialization": [
                {
                    "ir_key": "variables.triangular_uncertain.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    upper_bounds: list[DakotaFloat] = DakotaField(
        description="Specify maximium values",
        dakota={
            "aliases": ["tuv_upper_bounds"],
            "materialization": [
                {
                    "ir_key": "variables.triangular_uncertain.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    initial_point: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.triangular_uncertain.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "aliases": ["tuv_descriptors"],
            "materialization": [
                {
                    "ir_key": "variables.triangular_uncertain.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )

    # Extra fields (not derived from XML grammar)
    initial_point_user_provided: bool = DakotaField(
        default=False,
        description="Internal-only flag indicating the user explicitly provided initial_point.",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.uncertain.initial_point_flag",
                    "storage_type": "UNCERTAIN_INIT_POINT_FLAG",
                    "ir_value_type": "bool",
                }
            ],
        },
    )


class ExponentialUncertain(DakotaBaseModel):
    """Generated model for ExponentialUncertain"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="exponential_uncertain", list_field="betas", target_field="count"
        ),
        CompareLength(
            context="exponential_uncertain",
            list_field="initial_point",
            target_field="count",
        ),
        CompareLength(
            context="exponential_uncertain",
            list_field="descriptors",
            target_field="count",
        ),
        CheckVariableDescriptorsLength(context="exponential_uncertain"),
        CheckDescriptorsValid(context="exponential_uncertain", check_uniqueness=False),
        DefaultVariableDescriptors(context="exponential_uncertain", prefix="euv_"),
        ExponentialUncertainInitial(context="exponential_uncertain"),
        CheckRealLowerBound(
            context="exponential_uncertain", list_field="betas", lower_bound=0
        ),
    ]

    count: int = DakotaField(
        gt=0,
        description="Aleatory uncertain variable - exponential",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.exponential_uncertain",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.continuous",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.aleatory_uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    betas: list[DakotaFloat] = DakotaField(
        description="Parameter of the exponential distribution",
        dakota={
            "aliases": ["euv_betas"],
            "materialization": [
                {
                    "ir_key": "variables.exponential_uncertain.betas",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    initial_point: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.exponential_uncertain.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "aliases": ["euv_descriptors"],
            "materialization": [
                {
                    "ir_key": "variables.exponential_uncertain.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )

    # Extra fields (not derived from XML grammar)
    initial_point_user_provided: bool = DakotaField(
        default=False,
        description="Internal-only flag indicating the user explicitly provided initial_point.",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.uncertain.initial_point_flag",
                    "storage_type": "UNCERTAIN_INIT_POINT_FLAG",
                    "ir_value_type": "bool",
                }
            ],
        },
    )

    _COMPUTED_FIELDS: ClassVar[dict[str, ComputedFieldSpec]] = {
        "lower_bounds": ComputedFieldSpec(
            function="exponential_lower_bounds",
            description="Lower bounds (always 0 for exponential)",
            return_type=list[float],
            materialization=[
                {
                    "ir_key": "variables.exponential_uncertain.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        ),
        "upper_bounds": ComputedFieldSpec(
            function="exponential_upper_bounds",
            description="Upper bounds: mean + 3*stdev from beta",
            return_type=list[float],
            materialization=[
                {
                    "ir_key": "variables.exponential_uncertain.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        ),
    }

    @computed_field
    @property
    def lower_bounds(self) -> List[float]:
        """Lower bounds (always 0 for exponential)"""
        return ExponentialBounds.lower_bounds(self)

    @computed_field
    @property
    def upper_bounds(self) -> List[float]:
        """Upper bounds: mean + 3*stdev from beta"""
        return ExponentialBounds.upper_bounds(self)


class BetaUncertain(DakotaBaseModel):
    """Generated model for BetaUncertain"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="beta_uncertain", list_field="alphas", target_field="count"
        ),
        CompareLength(
            context="beta_uncertain", list_field="betas", target_field="count"
        ),
        CompareLength(
            context="beta_uncertain", list_field="lower_bounds", target_field="count"
        ),
        CompareLength(
            context="beta_uncertain", list_field="upper_bounds", target_field="count"
        ),
        CompareLength(
            context="beta_uncertain", list_field="initial_point", target_field="count"
        ),
        CompareLength(
            context="beta_uncertain", list_field="descriptors", target_field="count"
        ),
        CheckVariableDescriptorsLength(context="beta_uncertain"),
        CheckDescriptorsValid(context="beta_uncertain", check_uniqueness=False),
        DefaultVariableDescriptors(context="beta_uncertain", prefix="beuv_"),
        BetaUncertainInitial(context="beta_uncertain"),
        CheckVariableBoundsOrdering(context="beta_uncertain"),
        CheckRealLowerBound(
            context="beta_uncertain", list_field="alphas", lower_bound=0
        ),
        CheckRealLowerBound(
            context="beta_uncertain", list_field="betas", lower_bound=0
        ),
    ]

    count: int = DakotaField(
        gt=0,
        description="Aleatory uncertain variable - beta",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.beta_uncertain",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.continuous",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.aleatory_uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    alphas: list[DakotaFloat] = DakotaField(
        description="First parameter of the beta distribution",
        dakota={
            "aliases": ["buv_alphas"],
            "materialization": [
                {
                    "ir_key": "variables.beta_uncertain.alphas",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    betas: list[DakotaFloat] = DakotaField(
        description="Second parameter of the beta distribution",
        dakota={
            "aliases": ["buv_betas"],
            "materialization": [
                {
                    "ir_key": "variables.beta_uncertain.betas",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    lower_bounds: list[DakotaFloat] = DakotaField(
        description="Specify minimum values",
        dakota={
            "aliases": ["buv_lower_bounds"],
            "materialization": [
                {
                    "ir_key": "variables.beta_uncertain.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    upper_bounds: list[DakotaFloat] = DakotaField(
        description="Specify maximium values",
        dakota={
            "aliases": ["buv_upper_bounds"],
            "materialization": [
                {
                    "ir_key": "variables.beta_uncertain.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    initial_point: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.beta_uncertain.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "aliases": ["buv_descriptors"],
            "materialization": [
                {
                    "ir_key": "variables.beta_uncertain.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )

    # Extra fields (not derived from XML grammar)
    initial_point_user_provided: bool = DakotaField(
        default=False,
        description="Internal-only flag indicating the user explicitly provided initial_point.",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.uncertain.initial_point_flag",
                    "storage_type": "UNCERTAIN_INIT_POINT_FLAG",
                    "ir_value_type": "bool",
                }
            ],
        },
    )


class GammaUncertain(DakotaBaseModel):
    """Generated model for GammaUncertain"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="gamma_uncertain", list_field="alphas", target_field="count"
        ),
        CompareLength(
            context="gamma_uncertain", list_field="betas", target_field="count"
        ),
        CompareLength(
            context="gamma_uncertain", list_field="initial_point", target_field="count"
        ),
        CompareLength(
            context="gamma_uncertain", list_field="descriptors", target_field="count"
        ),
        CheckVariableDescriptorsLength(context="gamma_uncertain"),
        CheckDescriptorsValid(context="gamma_uncertain", check_uniqueness=False),
        DefaultVariableDescriptors(context="gamma_uncertain", prefix="gauv_"),
        GammaUncertainInitial(context="gamma_uncertain"),
        CheckRealLowerBound(
            context="gamma_uncertain", list_field="alphas", lower_bound=0
        ),
        CheckRealLowerBound(
            context="gamma_uncertain", list_field="betas", lower_bound=0
        ),
    ]

    count: int = DakotaField(
        gt=0,
        description="Aleatory uncertain variable - gamma",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.gamma_uncertain",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.continuous",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.aleatory_uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    alphas: list[DakotaFloat] = DakotaField(
        description="First parameter of the gamma distribution",
        dakota={
            "aliases": ["gauv_alphas"],
            "materialization": [
                {
                    "ir_key": "variables.gamma_uncertain.alphas",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    betas: list[DakotaFloat] = DakotaField(
        description="Second parameter of the gamma distribution",
        dakota={
            "aliases": ["gauv_betas"],
            "materialization": [
                {
                    "ir_key": "variables.gamma_uncertain.betas",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    initial_point: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.gamma_uncertain.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "aliases": ["gauv_descriptors"],
            "materialization": [
                {
                    "ir_key": "variables.gamma_uncertain.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )

    # Extra fields (not derived from XML grammar)
    initial_point_user_provided: bool = DakotaField(
        default=False,
        description="Internal-only flag indicating the user explicitly provided initial_point.",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.uncertain.initial_point_flag",
                    "storage_type": "UNCERTAIN_INIT_POINT_FLAG",
                    "ir_value_type": "bool",
                }
            ],
        },
    )

    _COMPUTED_FIELDS: ClassVar[dict[str, ComputedFieldSpec]] = {
        "lower_bounds": ComputedFieldSpec(
            function="gamma_lower_bounds",
            description="Lower bounds (always 0 for gamma)",
            return_type=list[float],
            materialization=[
                {
                    "ir_key": "variables.gamma_uncertain.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        ),
        "upper_bounds": ComputedFieldSpec(
            function="gamma_upper_bounds",
            description="Upper bounds: mean + 3*stdev from alpha, beta",
            return_type=list[float],
            materialization=[
                {
                    "ir_key": "variables.gamma_uncertain.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        ),
    }

    @computed_field
    @property
    def lower_bounds(self) -> List[float]:
        """Lower bounds (always 0 for gamma)"""
        return GammaBounds.lower_bounds(self)

    @computed_field
    @property
    def upper_bounds(self) -> List[float]:
        """Upper bounds: mean + 3*stdev from alpha, beta"""
        return GammaBounds.upper_bounds(self)


class GumbelUncertain(DakotaBaseModel):
    """Generated model for GumbelUncertain"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="gumbel_uncertain", list_field="alphas", target_field="count"
        ),
        CompareLength(
            context="gumbel_uncertain", list_field="betas", target_field="count"
        ),
        CompareLength(
            context="gumbel_uncertain", list_field="initial_point", target_field="count"
        ),
        CompareLength(
            context="gumbel_uncertain", list_field="descriptors", target_field="count"
        ),
        CheckVariableDescriptorsLength(context="gumbel_uncertain"),
        CheckDescriptorsValid(context="gumbel_uncertain", check_uniqueness=False),
        DefaultVariableDescriptors(context="gumbel_uncertain", prefix="guuv_"),
        GumbelUncertainInitial(context="gumbel_uncertain"),
        CheckRealLowerBound(
            context="gumbel_uncertain", list_field="alphas", lower_bound=0
        ),
    ]

    count: int = DakotaField(
        gt=0,
        description="Aleatory uncertain variable - gumbel",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.gumbel_uncertain",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.continuous",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.aleatory_uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    alphas: list[DakotaFloat] = DakotaField(
        description="First parameter of the gumbel distribution",
        dakota={
            "aliases": ["guuv_alphas"],
            "materialization": [
                {
                    "ir_key": "variables.gumbel_uncertain.alphas",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    betas: list[DakotaFloat] = DakotaField(
        description="Second parameter of the gumbel distribution",
        dakota={
            "aliases": ["guuv_betas"],
            "materialization": [
                {
                    "ir_key": "variables.gumbel_uncertain.betas",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    initial_point: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.gumbel_uncertain.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "aliases": ["guuv_descriptors"],
            "materialization": [
                {
                    "ir_key": "variables.gumbel_uncertain.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )

    # Extra fields (not derived from XML grammar)
    initial_point_user_provided: bool = DakotaField(
        default=False,
        description="Internal-only flag indicating the user explicitly provided initial_point.",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.uncertain.initial_point_flag",
                    "storage_type": "UNCERTAIN_INIT_POINT_FLAG",
                    "ir_value_type": "bool",
                }
            ],
        },
    )

    _COMPUTED_FIELDS: ClassVar[dict[str, ComputedFieldSpec]] = {
        "lower_bounds": ComputedFieldSpec(
            function="gumbel_lower_bounds",
            description="Lower bounds: mean - 3*stdev from alpha, beta",
            return_type=list[float],
            materialization=[
                {
                    "ir_key": "variables.gumbel_uncertain.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        ),
        "upper_bounds": ComputedFieldSpec(
            function="gumbel_upper_bounds",
            description="Upper bounds: mean + 3*stdev from alpha, beta",
            return_type=list[float],
            materialization=[
                {
                    "ir_key": "variables.gumbel_uncertain.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        ),
    }

    @computed_field
    @property
    def lower_bounds(self) -> List[float]:
        """Lower bounds: mean - 3*stdev from alpha, beta"""
        return GumbelBounds.lower_bounds(self)

    @computed_field
    @property
    def upper_bounds(self) -> List[float]:
        """Upper bounds: mean + 3*stdev from alpha, beta"""
        return GumbelBounds.upper_bounds(self)


class FrechetUncertain(DakotaBaseModel):
    """Generated model for FrechetUncertain"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="frechet_uncertain", list_field="alphas", target_field="count"
        ),
        CompareLength(
            context="frechet_uncertain", list_field="betas", target_field="count"
        ),
        CompareLength(
            context="frechet_uncertain",
            list_field="initial_point",
            target_field="count",
        ),
        CompareLength(
            context="frechet_uncertain", list_field="descriptors", target_field="count"
        ),
        CheckVariableDescriptorsLength(context="frechet_uncertain"),
        CheckDescriptorsValid(context="frechet_uncertain", check_uniqueness=False),
        DefaultVariableDescriptors(context="frechet_uncertain", prefix="fuv_"),
        FrechetUncertainInitial(context="frechet_uncertain"),
        CheckRealLowerBound(
            context="frechet_uncertain", list_field="alphas", lower_bound=2
        ),
    ]

    count: int = DakotaField(
        gt=0,
        description="Aleatory uncertain variable - Frechet",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.frechet_uncertain",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.continuous",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.aleatory_uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    alphas: list[DakotaFloat] = DakotaField(
        description="First parameter of the Frechet distribution",
        dakota={
            "aliases": ["fuv_alphas"],
            "materialization": [
                {
                    "ir_key": "variables.frechet_uncertain.alphas",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    betas: list[DakotaFloat] = DakotaField(
        description="Second parameter of the Frechet distribution",
        dakota={
            "aliases": ["fuv_betas"],
            "materialization": [
                {
                    "ir_key": "variables.frechet_uncertain.betas",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    initial_point: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.frechet_uncertain.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "aliases": ["fuv_descriptors"],
            "materialization": [
                {
                    "ir_key": "variables.frechet_uncertain.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )

    # Extra fields (not derived from XML grammar)
    initial_point_user_provided: bool = DakotaField(
        default=False,
        description="Internal-only flag indicating the user explicitly provided initial_point.",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.uncertain.initial_point_flag",
                    "storage_type": "UNCERTAIN_INIT_POINT_FLAG",
                    "ir_value_type": "bool",
                }
            ],
        },
    )

    _COMPUTED_FIELDS: ClassVar[dict[str, ComputedFieldSpec]] = {
        "lower_bounds": ComputedFieldSpec(
            function="frechet_lower_bounds",
            description="Lower bounds (always 0 for Frechet)",
            return_type=list[float],
            materialization=[
                {
                    "ir_key": "variables.frechet_uncertain.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        ),
        "upper_bounds": ComputedFieldSpec(
            function="frechet_upper_bounds",
            description="Upper bounds: mean + 3*stdev from alpha, beta",
            return_type=list[float],
            materialization=[
                {
                    "ir_key": "variables.frechet_uncertain.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        ),
    }

    @computed_field
    @property
    def lower_bounds(self) -> List[float]:
        """Lower bounds (always 0 for Frechet)"""
        return FrechetBounds.lower_bounds(self)

    @computed_field
    @property
    def upper_bounds(self) -> List[float]:
        """Upper bounds: mean + 3*stdev from alpha, beta"""
        return FrechetBounds.upper_bounds(self)


class WeibullUncertain(DakotaBaseModel):
    """Generated model for WeibullUncertain"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="weibull_uncertain", list_field="alphas", target_field="count"
        ),
        CompareLength(
            context="weibull_uncertain", list_field="betas", target_field="count"
        ),
        CompareLength(
            context="weibull_uncertain",
            list_field="initial_point",
            target_field="count",
        ),
        CompareLength(
            context="weibull_uncertain", list_field="descriptors", target_field="count"
        ),
        CheckVariableDescriptorsLength(context="weibull_uncertain"),
        CheckDescriptorsValid(context="weibull_uncertain", check_uniqueness=False),
        DefaultVariableDescriptors(context="weibull_uncertain", prefix="wuv_"),
        WeibullUncertainInitial(context="weibull_uncertain"),
        CheckRealLowerBound(
            context="weibull_uncertain", list_field="alphas", lower_bound=0
        ),
        CheckRealLowerBound(
            context="weibull_uncertain", list_field="betas", lower_bound=0
        ),
    ]

    count: int = DakotaField(
        gt=0,
        description="Aleatory uncertain variable - Weibull",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.weibull_uncertain",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.continuous",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.aleatory_uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    alphas: list[DakotaFloat] = DakotaField(
        description="First parameter of the Weibull distribution",
        dakota={
            "aliases": ["wuv_alphas"],
            "materialization": [
                {
                    "ir_key": "variables.weibull_uncertain.alphas",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    betas: list[DakotaFloat] = DakotaField(
        description="Second parameter of the Weibull distribution",
        dakota={
            "aliases": ["wuv_betas"],
            "materialization": [
                {
                    "ir_key": "variables.weibull_uncertain.betas",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    initial_point: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.weibull_uncertain.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "aliases": ["wuv_descriptors"],
            "materialization": [
                {
                    "ir_key": "variables.weibull_uncertain.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )

    # Extra fields (not derived from XML grammar)
    initial_point_user_provided: bool = DakotaField(
        default=False,
        description="Internal-only flag indicating the user explicitly provided initial_point.",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.uncertain.initial_point_flag",
                    "storage_type": "UNCERTAIN_INIT_POINT_FLAG",
                    "ir_value_type": "bool",
                }
            ],
        },
    )

    _COMPUTED_FIELDS: ClassVar[dict[str, ComputedFieldSpec]] = {
        "lower_bounds": ComputedFieldSpec(
            function="weibull_lower_bounds",
            description="Lower bounds (always 0 for Weibull)",
            return_type=list[float],
            materialization=[
                {
                    "ir_key": "variables.weibull_uncertain.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        ),
        "upper_bounds": ComputedFieldSpec(
            function="weibull_upper_bounds",
            description="Upper bounds: mean + 3*stdev from alpha, beta",
            return_type=list[float],
            materialization=[
                {
                    "ir_key": "variables.weibull_uncertain.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        ),
    }

    @computed_field
    @property
    def lower_bounds(self) -> List[float]:
        """Lower bounds (always 0 for Weibull)"""
        return WeibullBounds.lower_bounds(self)

    @computed_field
    @property
    def upper_bounds(self) -> List[float]:
        """Upper bounds: mean + 3*stdev from alpha, beta"""
        return WeibullBounds.upper_bounds(self)


class Ordinates(DakotaBaseModel):
    """Generated model for Ordinates"""

    ordinates: list[DakotaFloat] = DakotaField(
        description='Ordinates specifying a "skyline" probability density function',
        dakota={"aliases": ["huv_bin_ordinates"]},
    )


class Counts(DakotaBaseModel):
    """Generated model for Counts"""

    counts: list[DakotaFloat] = DakotaField(
        description="Frequency or relative probability of each bin",
        dakota={"aliases": ["huv_bin_counts"]},
    )


class PoissonUncertain(DakotaBaseModel):
    """Generated model for PoissonUncertain"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="poisson_uncertain", list_field="lambdas", target_field="count"
        ),
        CompareLength(
            context="poisson_uncertain",
            list_field="initial_point",
            target_field="count",
        ),
        CompareLength(
            context="poisson_uncertain", list_field="descriptors", target_field="count"
        ),
        CheckVariableDescriptorsLength(context="poisson_uncertain"),
        CheckDescriptorsValid(context="poisson_uncertain", check_uniqueness=False),
        DefaultVariableDescriptors(context="poisson_uncertain", prefix="puv_"),
        PoissonUncertainInitial(context="poisson_uncertain"),
        CheckRealLowerBound(
            context="poisson_uncertain", list_field="lambdas", lower_bound=0
        ),
    ]

    count: int = DakotaField(
        gt=0,
        description="Aleatory uncertain discrete variable - Poisson",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.poisson_uncertain",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.discrete",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.aleatory_uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    lambdas: list[DakotaFloat] = DakotaField(
        description="The parameter for the Poisson distribution, the expected number of events in the time interval of interest",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.poisson_uncertain.lambdas",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    initial_point: list[int] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.poisson_uncertain.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.poisson_uncertain.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )

    # Extra fields (not derived from XML grammar)
    initial_point_user_provided: bool = DakotaField(
        default=False,
        description="Internal-only flag indicating the user explicitly provided initial_point.",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.uncertain.initial_point_flag",
                    "storage_type": "UNCERTAIN_INIT_POINT_FLAG",
                    "ir_value_type": "bool",
                }
            ],
        },
    )

    _COMPUTED_FIELDS: ClassVar[dict[str, ComputedFieldSpec]] = {
        "lower_bounds": ComputedFieldSpec(
            function="poisson_lower_bounds",
            description="Lower bounds (always 0 for Poisson)",
            return_type=list[int],
            materialization=[
                {
                    "ir_key": "variables.poisson_uncertain.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        ),
        "upper_bounds": ComputedFieldSpec(
            function="poisson_upper_bounds",
            description="Upper bounds: ceil(lambda + 3*sqrt(lambda))",
            return_type=list[int],
            materialization=[
                {
                    "ir_key": "variables.poisson_uncertain.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        ),
    }

    @computed_field
    @property
    def lower_bounds(self) -> List[int]:
        """Lower bounds (always 0 for Poisson)"""
        return PoissonBounds.lower_bounds(self)

    @computed_field
    @property
    def upper_bounds(self) -> List[int]:
        """Upper bounds: ceil(lambda + 3*sqrt(lambda))"""
        return PoissonBounds.upper_bounds(self)


class BinomialUncertain(DakotaBaseModel):
    """Generated model for BinomialUncertain"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="binomial_uncertain",
            list_field="probability_per_trial",
            target_field="count",
        ),
        CompareLength(
            context="binomial_uncertain", list_field="num_trials", target_field="count"
        ),
        CompareLength(
            context="binomial_uncertain",
            list_field="initial_point",
            target_field="count",
        ),
        CompareLength(
            context="binomial_uncertain", list_field="descriptors", target_field="count"
        ),
        CheckVariableDescriptorsLength(context="binomial_uncertain"),
        CheckDescriptorsValid(context="binomial_uncertain", check_uniqueness=False),
        DefaultVariableDescriptors(context="binomial_uncertain", prefix="biuv_"),
        BinomialUncertainInitial(context="binomial_uncertain"),
        CheckIntLowerBound(
            context="binomial_uncertain", list_field="num_trials", lower_bound=0
        ),
    ]

    count: int = DakotaField(
        gt=0,
        description="Aleatory uncertain discrete variable - binomial",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.binomial_uncertain",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.discrete",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.aleatory_uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    probability_per_trial: list[DakotaFloat] = DakotaField(
        description="A distribution parameter for the binomial distribution",
        dakota={
            "aliases": ["prob_per_trial"],
            "materialization": [
                {
                    "ir_key": "variables.binomial_uncertain.prob_per_trial",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    num_trials: list[int] = DakotaField(
        description="A distribution parameter",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.binomial_uncertain.num_trials",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )
    initial_point: list[int] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.binomial_uncertain.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.binomial_uncertain.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )

    # Extra fields (not derived from XML grammar)
    initial_point_user_provided: bool = DakotaField(
        default=False,
        description="Internal-only flag indicating the user explicitly provided initial_point.",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.uncertain.initial_point_flag",
                    "storage_type": "UNCERTAIN_INIT_POINT_FLAG",
                    "ir_value_type": "bool",
                }
            ],
        },
    )

    _COMPUTED_FIELDS: ClassVar[dict[str, ComputedFieldSpec]] = {
        "lower_bounds": ComputedFieldSpec(
            function="binomial_lower_bounds",
            description="Lower bounds (always 0 for binomial)",
            return_type=list[int],
            materialization=[
                {
                    "ir_key": "variables.binomial_uncertain.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        ),
        "upper_bounds": ComputedFieldSpec(
            function="binomial_upper_bounds",
            description="Upper bounds (equals num_trials)",
            return_type=list[int],
            materialization=[
                {
                    "ir_key": "variables.binomial_uncertain.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        ),
    }

    @computed_field
    @property
    def lower_bounds(self) -> List[int]:
        """Lower bounds (always 0 for binomial)"""
        return BinomialBounds.lower_bounds(self)

    @computed_field
    @property
    def upper_bounds(self) -> List[int]:
        """Upper bounds (equals num_trials)"""
        return BinomialBounds.upper_bounds(self)


class NegativeBinomialUncertain(DakotaBaseModel):
    """Generated model for NegativeBinomialUncertain"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="negative_binomial_uncertain",
            list_field="probability_per_trial",
            target_field="count",
        ),
        CompareLength(
            context="negative_binomial_uncertain",
            list_field="num_trials",
            target_field="count",
        ),
        CompareLength(
            context="negative_binomial_uncertain",
            list_field="initial_point",
            target_field="count",
        ),
        CompareLength(
            context="negative_binomial_uncertain",
            list_field="descriptors",
            target_field="count",
        ),
        CheckVariableDescriptorsLength(context="negative_binomial_uncertain"),
        CheckDescriptorsValid(
            context="negative_binomial_uncertain", check_uniqueness=False
        ),
        DefaultVariableDescriptors(
            context="negative_binomial_uncertain", prefix="nbuv_"
        ),
        NegativeBinomialUncertainInitial(context="negative_binomial_uncertain"),
        CheckIntLowerBound(
            context="negative_binomial_uncertain",
            list_field="num_trials",
            lower_bound=0,
        ),
    ]

    count: int = DakotaField(
        gt=0,
        description="Aleatory uncertain discrete variable - negative binomial",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.negative_binomial_uncertain",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.discrete",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.aleatory_uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    probability_per_trial: list[DakotaFloat] = DakotaField(
        description="A negative binomial distribution parameter",
        dakota={
            "aliases": ["prob_per_trial"],
            "materialization": [
                {
                    "ir_key": "variables.negative_binomial_uncertain.prob_per_trial",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    num_trials: list[int] = DakotaField(
        description="A negative binomial distribution parameter",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.negative_binomial_uncertain.num_trials",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )
    initial_point: list[int] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.negative_binomial_uncertain.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.negative_binomial_uncertain.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )

    # Extra fields (not derived from XML grammar)
    initial_point_user_provided: bool = DakotaField(
        default=False,
        description="Internal-only flag indicating the user explicitly provided initial_point.",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.uncertain.initial_point_flag",
                    "storage_type": "UNCERTAIN_INIT_POINT_FLAG",
                    "ir_value_type": "bool",
                }
            ],
        },
    )

    _COMPUTED_FIELDS: ClassVar[dict[str, ComputedFieldSpec]] = {
        "lower_bounds": ComputedFieldSpec(
            function="negative_binomial_lower_bounds",
            description="Lower bounds (equals num_trials)",
            return_type=list[int],
            materialization=[
                {
                    "ir_key": "variables.negative_binomial_uncertain.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        ),
        "upper_bounds": ComputedFieldSpec(
            function="negative_binomial_upper_bounds",
            description="Upper bounds: ceil(mean + 3*stdev)",
            return_type=list[int],
            materialization=[
                {
                    "ir_key": "variables.negative_binomial_uncertain.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        ),
    }

    @computed_field
    @property
    def lower_bounds(self) -> List[int]:
        """Lower bounds (equals num_trials)"""
        return NegativeBinomialBounds.lower_bounds(self)

    @computed_field
    @property
    def upper_bounds(self) -> List[int]:
        """Upper bounds: ceil(mean + 3*stdev)"""
        return NegativeBinomialBounds.upper_bounds(self)


class GeometricUncertain(DakotaBaseModel):
    """Generated model for GeometricUncertain"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="geometric_uncertain",
            list_field="probability_per_trial",
            target_field="count",
        ),
        CompareLength(
            context="geometric_uncertain",
            list_field="initial_point",
            target_field="count",
        ),
        CompareLength(
            context="geometric_uncertain",
            list_field="descriptors",
            target_field="count",
        ),
        CheckVariableDescriptorsLength(context="geometric_uncertain"),
        CheckDescriptorsValid(context="geometric_uncertain", check_uniqueness=False),
        DefaultVariableDescriptors(context="geometric_uncertain", prefix="geuv_"),
        GeometricUncertainInitial(context="geometric_uncertain"),
    ]

    count: int = DakotaField(
        gt=0,
        description="Aleatory uncertain discrete variable - geometric",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.geometric_uncertain",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.discrete",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.aleatory_uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    probability_per_trial: list[DakotaFloat] = DakotaField(
        description="Geometric distribution parameter",
        dakota={
            "aliases": ["prob_per_trial"],
            "materialization": [
                {
                    "ir_key": "variables.geometric_uncertain.prob_per_trial",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    initial_point: list[int] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.geometric_uncertain.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.geometric_uncertain.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )

    # Extra fields (not derived from XML grammar)
    initial_point_user_provided: bool = DakotaField(
        default=False,
        description="Internal-only flag indicating the user explicitly provided initial_point.",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.uncertain.initial_point_flag",
                    "storage_type": "UNCERTAIN_INIT_POINT_FLAG",
                    "ir_value_type": "bool",
                }
            ],
        },
    )

    _COMPUTED_FIELDS: ClassVar[dict[str, ComputedFieldSpec]] = {
        "lower_bounds": ComputedFieldSpec(
            function="geometric_lower_bounds",
            description="Lower bounds (always 0 for geometric)",
            return_type=list[int],
            materialization=[
                {
                    "ir_key": "variables.geometric_uncertain.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        ),
        "upper_bounds": ComputedFieldSpec(
            function="geometric_upper_bounds",
            description="Upper bounds: ceil(mean + 3*stdev)",
            return_type=list[int],
            materialization=[
                {
                    "ir_key": "variables.geometric_uncertain.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        ),
    }

    @computed_field
    @property
    def lower_bounds(self) -> List[int]:
        """Lower bounds (always 0 for geometric)"""
        return GeometricBounds.lower_bounds(self)

    @computed_field
    @property
    def upper_bounds(self) -> List[int]:
        """Upper bounds: ceil(mean + 3*stdev)"""
        return GeometricBounds.upper_bounds(self)


class HypergeometricUncertain(DakotaBaseModel):
    """Generated model for HypergeometricUncertain"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="hypergeometric_uncertain",
            list_field="total_population",
            target_field="count",
        ),
        CompareLength(
            context="hypergeometric_uncertain",
            list_field="selected_population",
            target_field="count",
        ),
        CompareLength(
            context="hypergeometric_uncertain",
            list_field="num_drawn",
            target_field="count",
        ),
        CompareLength(
            context="hypergeometric_uncertain",
            list_field="initial_point",
            target_field="count",
        ),
        CompareLength(
            context="hypergeometric_uncertain",
            list_field="descriptors",
            target_field="count",
        ),
        CheckVariableDescriptorsLength(context="hypergeometric_uncertain"),
        CheckDescriptorsValid(
            context="hypergeometric_uncertain", check_uniqueness=False
        ),
        DefaultVariableDescriptors(context="hypergeometric_uncertain", prefix="hguv_"),
        HypergeometricUncertainInitial(context="hypergeometric_uncertain"),
        CheckIntLowerBound(
            context="hypergeometric_uncertain",
            list_field="total_population",
            lower_bound=0,
        ),
        CheckIntLowerBound(
            context="hypergeometric_uncertain",
            list_field="selected_population",
            lower_bound=0,
        ),
        CheckIntLowerBound(
            context="hypergeometric_uncertain", list_field="num_drawn", lower_bound=0
        ),
    ]

    count: int = DakotaField(
        gt=0,
        description="Aleatory uncertain discrete variable - hypergeometric",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.hypergeometric_uncertain",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.discrete",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.aleatory_uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    total_population: list[int] = DakotaField(
        description="Parameter for the hypergeometric probability distribution describing the size of the total population",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.hypergeometric_uncertain.total_population",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )
    selected_population: list[int] = DakotaField(
        description="Distribution parameter for the hypergeometric distribution describing the size of the population subset of interest",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.hypergeometric_uncertain.selected_population",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )
    num_drawn: list[int] = DakotaField(
        description="Distribution parameter for the hypergeometric distribution describing the number of draws from a combined population",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.hypergeometric_uncertain.num_drawn",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )
    initial_point: list[int] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.hypergeometric_uncertain.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.hypergeometric_uncertain.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )

    # Extra fields (not derived from XML grammar)
    initial_point_user_provided: bool = DakotaField(
        default=False,
        description="Internal-only flag indicating the user explicitly provided initial_point.",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.uncertain.initial_point_flag",
                    "storage_type": "UNCERTAIN_INIT_POINT_FLAG",
                    "ir_value_type": "bool",
                }
            ],
        },
    )

    _COMPUTED_FIELDS: ClassVar[dict[str, ComputedFieldSpec]] = {
        "lower_bounds": ComputedFieldSpec(
            function="hypergeometric_lower_bounds",
            description="Lower bounds (always 0 for hypergeometric)",
            return_type=list[int],
            materialization=[
                {
                    "ir_key": "variables.hypergeometric_uncertain.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        ),
        "upper_bounds": ComputedFieldSpec(
            function="hypergeometric_upper_bounds",
            description="Upper bounds: min(num_drawn, selected_population)",
            return_type=list[int],
            materialization=[
                {
                    "ir_key": "variables.hypergeometric_uncertain.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        ),
    }

    @computed_field
    @property
    def lower_bounds(self) -> List[int]:
        """Lower bounds (always 0 for hypergeometric)"""
        return HypergeometricBounds.lower_bounds(self)

    @computed_field
    @property
    def upper_bounds(self) -> List[int]:
        """Upper bounds: min(num_drawn, selected_population)"""
        return HypergeometricBounds.upper_bounds(self)


class HistogramPointUncertainInteger(DakotaBaseModel):
    """Generated model for HistogramPointUncertainInteger"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="variables_histogram_point_uncertain_integer",
            list_field="pairs_per_variable",
            target_field="count",
        ),
        CompareLength(
            context="variables_histogram_point_uncertain_integer",
            list_field="initial_point",
            target_field="count",
        ),
        CompareLength(
            context="variables_histogram_point_uncertain_integer",
            list_field="descriptors",
            target_field="count",
        ),
        CheckVariableDescriptorsLength(context="integer"),
        CheckDescriptorsValid(context="integer", check_uniqueness=False),
        DefaultVariableDescriptors(context="integer", prefix="hpiuv_"),
        CheckSumEqualsLength(
            context="integer",
            num_list_field="pairs_per_variable",
            levels_list_field="abscissas",
        ),
        CheckSetElementsOrdering(
            context="integer",
            elements_per_variable_field="pairs_per_variable",
            elements_field="abscissas",
        ),
        CompareListLengths(context="integer", list_a="counts", list_b="abscissas"),
        HistogramPointIntUncertainInitial(context="integer"),
    ]

    count: int = DakotaField(
        gt=0,
        description="Integer valued point histogram variable",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.histogram_uncertain.point_int",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.discrete",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.aleatory_uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    pairs_per_variable: list[int] | None = DakotaField(
        default=None,
        description="Number of pairs defining each histogram point integer variable",
        dakota={"aliases": ["num_pairs"]},
    )
    abscissas: list[int] = DakotaField(
        description="Integer abscissas for a point histogram"
    )
    counts: list[DakotaFloat] = DakotaField(
        description="Counts for integer-valued point histogram"
    )
    initial_point: list[int] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.histogram_uncertain.point_int.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.histogram_uncertain.point_int.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )

    # Extra fields (not derived from XML grammar)
    initial_point_user_provided: bool = DakotaField(
        default=False,
        description="Internal-only flag indicating the user explicitly provided initial_point.",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.uncertain.initial_point_flag",
                    "storage_type": "UNCERTAIN_INIT_POINT_FLAG",
                    "ir_value_type": "bool",
                }
            ],
        },
    )

    _COMPUTED_FIELDS: ClassVar[dict[str, ComputedFieldSpec]] = {
        "lower_bounds": ComputedFieldSpec(
            function="histogram_point_int_lower_bounds",
            description="Lower bounds: min abscissa per variable",
            return_type=list[int],
            materialization=[
                {
                    "ir_key": "variables.histogram_uncertain.point_int.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        ),
        "upper_bounds": ComputedFieldSpec(
            function="histogram_point_int_upper_bounds",
            description="Upper bounds: max abscissa per variable",
            return_type=list[int],
            materialization=[
                {
                    "ir_key": "variables.histogram_uncertain.point_int.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        ),
    }

    @computed_field
    @property
    def lower_bounds(self) -> List[int]:
        """Lower bounds: min abscissa per variable"""
        return HistogramPointIntBounds.lower_bounds(self)

    @computed_field
    @property
    def upper_bounds(self) -> List[int]:
        """Upper bounds: max abscissa per variable"""
        return HistogramPointIntBounds.upper_bounds(self)


class HistogramPointUncertainString(DakotaBaseModel):
    """Generated model for HistogramPointUncertainString"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="variables_histogram_point_uncertain_string",
            list_field="pairs_per_variable",
            target_field="count",
        ),
        CompareLength(
            context="variables_histogram_point_uncertain_string",
            list_field="initial_point",
            target_field="count",
        ),
        CompareLength(
            context="variables_histogram_point_uncertain_string",
            list_field="descriptors",
            target_field="count",
        ),
        CheckVariableDescriptorsLength(context="string"),
        CheckDescriptorsValid(context="string", check_uniqueness=False),
        DefaultVariableDescriptors(context="string", prefix="hpsuv_"),
        CheckSumEqualsLength(
            context="string",
            num_list_field="pairs_per_variable",
            levels_list_field="abscissas",
        ),
        CheckSetElementsOrdering(
            context="string",
            elements_per_variable_field="pairs_per_variable",
            elements_field="abscissas",
        ),
        CompareListLengths(context="string", list_a="counts", list_b="abscissas"),
        HistogramPointStrUncertainInitial(context="string"),
    ]

    count: int = DakotaField(
        gt=0,
        description="String (categorical) valued point histogram variable",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.histogram_uncertain.point_string",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.discrete",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.aleatory_uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    pairs_per_variable: list[int] | None = DakotaField(
        default=None,
        description="Number of pairs defining each histogram point string variable",
        dakota={"aliases": ["num_pairs"]},
    )
    abscissas: list[str] = DakotaField(
        description="String abscissas for a point histogram"
    )
    counts: list[DakotaFloat] = DakotaField(
        description="Counts for string-valued point histogram"
    )
    initial_point: list[str] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.histogram_uncertain.point_string.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.histogram_uncertain.point_string.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )

    # Extra fields (not derived from XML grammar)
    initial_point_user_provided: bool = DakotaField(
        default=False,
        description="Internal-only flag indicating the user explicitly provided initial_point.",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.uncertain.initial_point_flag",
                    "storage_type": "UNCERTAIN_INIT_POINT_FLAG",
                    "ir_value_type": "bool",
                }
            ],
        },
    )

    _COMPUTED_FIELDS: ClassVar[dict[str, ComputedFieldSpec]] = {
        "lower_bounds": ComputedFieldSpec(
            function="histogram_point_str_lower_bounds",
            description="Lower bounds: min abscissa per variable",
            return_type=list[str],
            materialization=[
                {
                    "ir_key": "variables.histogram_uncertain.point_string.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        ),
        "upper_bounds": ComputedFieldSpec(
            function="histogram_point_str_upper_bounds",
            description="Upper bounds: max abscissa per variable",
            return_type=list[str],
            materialization=[
                {
                    "ir_key": "variables.histogram_uncertain.point_string.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        ),
    }

    @computed_field
    @property
    def lower_bounds(self) -> List[str]:
        """Lower bounds: min abscissa per variable"""
        return HistogramPointStrBounds.lower_bounds(self)

    @computed_field
    @property
    def upper_bounds(self) -> List[str]:
        """Upper bounds: max abscissa per variable"""
        return HistogramPointStrBounds.upper_bounds(self)


class HistogramPointUncertainReal(DakotaBaseModel):
    """Generated model for HistogramPointUncertainReal"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="variables_histogram_point_uncertain_real",
            list_field="pairs_per_variable",
            target_field="count",
        ),
        CompareLength(
            context="variables_histogram_point_uncertain_real",
            list_field="initial_point",
            target_field="count",
        ),
        CompareLength(
            context="variables_histogram_point_uncertain_real",
            list_field="descriptors",
            target_field="count",
        ),
        CheckVariableDescriptorsLength(context="real"),
        CheckDescriptorsValid(context="real", check_uniqueness=False),
        DefaultVariableDescriptors(context="real", prefix="hpruv_"),
        CheckSumEqualsLength(
            context="real",
            num_list_field="pairs_per_variable",
            levels_list_field="abscissas",
        ),
        CheckSetElementsOrdering(
            context="real",
            elements_per_variable_field="pairs_per_variable",
            elements_field="abscissas",
        ),
        CompareListLengths(context="real", list_a="counts", list_b="abscissas"),
        HistogramPointRealUncertainInitial(context="real"),
    ]

    count: int = DakotaField(
        gt=0,
        description="Real valued point histogram variable",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.histogram_uncertain.point_real",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.discrete",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.aleatory_uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    pairs_per_variable: list[int] | None = DakotaField(
        default=None,
        description="Number of pairs defining each histogram point real variable",
        dakota={"aliases": ["num_pairs"]},
    )
    abscissas: list[DakotaFloat] = DakotaField(
        description="Real abscissas for a point histogram"
    )
    counts: list[DakotaFloat] = DakotaField(
        description="Counts for real-valued point histogram"
    )
    initial_point: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.histogram_uncertain.point_real.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.histogram_uncertain.point_real.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )

    # Extra fields (not derived from XML grammar)
    initial_point_user_provided: bool = DakotaField(
        default=False,
        description="Internal-only flag indicating the user explicitly provided initial_point.",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.uncertain.initial_point_flag",
                    "storage_type": "UNCERTAIN_INIT_POINT_FLAG",
                    "ir_value_type": "bool",
                }
            ],
        },
    )

    _COMPUTED_FIELDS: ClassVar[dict[str, ComputedFieldSpec]] = {
        "lower_bounds": ComputedFieldSpec(
            function="histogram_point_real_lower_bounds",
            description="Lower bounds: min abscissa per variable",
            return_type=list[float],
            materialization=[
                {
                    "ir_key": "variables.histogram_uncertain.point_real.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        ),
        "upper_bounds": ComputedFieldSpec(
            function="histogram_point_real_upper_bounds",
            description="Upper bounds: max abscissa per variable",
            return_type=list[float],
            materialization=[
                {
                    "ir_key": "variables.histogram_uncertain.point_real.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        ),
    }

    @computed_field
    @property
    def lower_bounds(self) -> List[float]:
        """Lower bounds: min abscissa per variable"""
        return HistogramPointRealBounds.lower_bounds(self)

    @computed_field
    @property
    def upper_bounds(self) -> List[float]:
        """Upper bounds: max abscissa per variable"""
        return HistogramPointRealBounds.upper_bounds(self)


class ContinuousIntervalUncertain(DakotaBaseModel):
    """Generated model for ContinuousIntervalUncertain"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="continuous_interval_uncertain",
            list_field="num_intervals",
            target_field="count",
        ),
        CompareLength(
            context="continuous_interval_uncertain",
            list_field="initial_point",
            target_field="count",
        ),
        CompareLength(
            context="continuous_interval_uncertain",
            list_field="descriptors",
            target_field="count",
        ),
        CheckVariableDescriptorsLength(context="continuous_interval_uncertain"),
        CheckDescriptorsValid(
            context="continuous_interval_uncertain", check_uniqueness=False
        ),
        DefaultVariableDescriptors(
            context="continuous_interval_uncertain", prefix="ciuv_"
        ),
        ContinuousIntervalUncertainInitial(context="continuous_interval_uncertain"),
    ]

    count: int = DakotaField(
        gt=0,
        description="Epistemic uncertain variable - values from one or more continuous intervals",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.continuous_interval_uncertain",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.continuous",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.epistemic_uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    num_intervals: list[int] | None = DakotaField(
        default=None,
        description="Specify the number of intervals for each variable",
        dakota={"aliases": ["iuv_num_intervals"]},
    )
    interval_probabilities: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Assign probability mass to each interval",
        dakota={"aliases": ["interval_probs", "iuv_interval_probs"]},
    )
    lower_bounds: list[DakotaFloat] = DakotaField(description="Specify minimum values")
    upper_bounds: list[DakotaFloat] = DakotaField(description="Specify maximium values")
    initial_point: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.continuous_interval_uncertain.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "aliases": ["iuv_descriptors"],
            "materialization": [
                {
                    "ir_key": "variables.continuous_interval_uncertain.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )

    # Extra fields (not derived from XML grammar)
    initial_point_user_provided: bool = DakotaField(
        default=False,
        description="Internal-only flag indicating the user explicitly provided initial_point.",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.uncertain.initial_point_flag",
                    "storage_type": "UNCERTAIN_INIT_POINT_FLAG",
                    "ir_value_type": "bool",
                }
            ],
        },
    )

    _COMPUTED_FIELDS: ClassVar[dict[str, ComputedFieldSpec]] = {
        "inferred_lower_bounds": ComputedFieldSpec(
            function="continuous_interval_inferred_lower_bounds",
            description="Inferred lower bounds: min lower_bound across intervals per variable",
            return_type=list[float],
            materialization=[
                {
                    "ir_key": "variables.continuous_interval_uncertain.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        ),
        "inferred_upper_bounds": ComputedFieldSpec(
            function="continuous_interval_inferred_upper_bounds",
            description="Inferred upper bounds: max upper_bound across intervals per variable",
            return_type=list[float],
            materialization=[
                {
                    "ir_key": "variables.continuous_interval_uncertain.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        ),
    }

    @computed_field
    @property
    def inferred_lower_bounds(self) -> List[float]:
        """Inferred lower bounds: min lower_bound across intervals per variable"""
        return ContinuousIntervalBounds.inferred_lower_bounds(self)

    @computed_field
    @property
    def inferred_upper_bounds(self) -> List[float]:
        """Inferred upper bounds: max upper_bound across intervals per variable"""
        return ContinuousIntervalBounds.inferred_upper_bounds(self)


class DiscreteIntervalUncertain(DakotaBaseModel):
    """Generated model for DiscreteIntervalUncertain"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="discrete_interval_uncertain",
            list_field="num_intervals",
            target_field="count",
        ),
        CompareLength(
            context="discrete_interval_uncertain",
            list_field="initial_point",
            target_field="count",
        ),
        CompareLength(
            context="discrete_interval_uncertain",
            list_field="descriptors",
            target_field="count",
        ),
        CheckVariableDescriptorsLength(context="discrete_interval_uncertain"),
        CheckDescriptorsValid(
            context="discrete_interval_uncertain", check_uniqueness=False
        ),
        DefaultVariableDescriptors(
            context="discrete_interval_uncertain", prefix="diuv_"
        ),
        DiscreteIntervalUncertainInitial(context="discrete_interval_uncertain"),
    ]

    count: int = DakotaField(
        gt=0,
        description="Epistemic uncertain variable - values from one or more discrete intervals",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_interval_uncertain",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.discrete",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.epistemic_uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    num_intervals: list[int] | None = DakotaField(
        default=None, description="Specify the number of intervals for each variable"
    )
    interval_probabilities: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Assign probability mass to each interval",
        dakota={"aliases": ["interval_probs", "range_probabilities", "range_probs"]},
    )
    lower_bounds: list[int] = DakotaField(description="Specify minimum values")
    upper_bounds: list[int] = DakotaField(description="Specify maximium values")
    initial_point: list[int] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_interval_uncertain.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_interval_uncertain.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )

    # Extra fields (not derived from XML grammar)
    initial_point_user_provided: bool = DakotaField(
        default=False,
        description="Internal-only flag indicating the user explicitly provided initial_point.",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.uncertain.initial_point_flag",
                    "storage_type": "UNCERTAIN_INIT_POINT_FLAG",
                    "ir_value_type": "bool",
                }
            ],
        },
    )

    _COMPUTED_FIELDS: ClassVar[dict[str, ComputedFieldSpec]] = {
        "inferred_lower_bounds": ComputedFieldSpec(
            function="discrete_interval_inferred_lower_bounds",
            description="Inferred lower bounds: min lower_bound across intervals per variable",
            return_type=list[int],
            materialization=[
                {
                    "ir_key": "variables.discrete_interval_uncertain.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        ),
        "inferred_upper_bounds": ComputedFieldSpec(
            function="discrete_interval_inferred_upper_bounds",
            description="Inferred upper bounds: max upper_bound across intervals per variable",
            return_type=list[int],
            materialization=[
                {
                    "ir_key": "variables.discrete_interval_uncertain.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        ),
    }

    @computed_field
    @property
    def inferred_lower_bounds(self) -> List[int]:
        """Inferred lower bounds: min lower_bound across intervals per variable"""
        return DiscreteIntervalBounds.inferred_lower_bounds(self)

    @computed_field
    @property
    def inferred_upper_bounds(self) -> List[int]:
        """Inferred upper bounds: max upper_bound across intervals per variable"""
        return DiscreteIntervalBounds.inferred_upper_bounds(self)


class DiscreteUncertainSetInteger(DakotaBaseModel):
    """Generated model for DiscreteUncertainSetInteger"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="variables_discrete_uncertain_set_integer",
            list_field="elements_per_variable",
            target_field="count",
        ),
        CompareLength(
            context="variables_discrete_uncertain_set_integer",
            list_field="categorical",
            target_field="count",
        ),
        CompareLength(
            context="variables_discrete_uncertain_set_integer",
            list_field="initial_point",
            target_field="count",
        ),
        CompareLength(
            context="variables_discrete_uncertain_set_integer",
            list_field="descriptors",
            target_field="count",
        ),
        CheckVariableDescriptorsLength(context="integer"),
        CheckDescriptorsValid(context="integer", check_uniqueness=False),
        DefaultVariableDescriptors(context="integer", prefix="dusiv_"),
        DiscreteUncertainSetIntInitial(context="integer"),
        CheckPermittedValues(
            context="integer",
            field_name="categorical",
            permitted_values=["yes", "no", "true", "false", "y", "n", "t", "f"],
        ),
        CheckSumEqualsLength(
            context="integer",
            num_list_field="elements_per_variable",
            levels_list_field="elements",
        ),
        CheckSetElementsOrdering(context="integer"),
        CompareListLengths(
            context="integer", list_a="set_probabilities", list_b="elements"
        ),
        DefaultSetProbabilities(context="integer"),
    ]

    count: int = DakotaField(
        gt=0,
        description="Discrete, epistemic uncertain variable - integers within a set",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_uncertain_set_int",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.discrete",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.epistemic_uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    elements_per_variable: list[int] | None = DakotaField(
        default=None,
        description="Number of admissible elements for each set variable",
        dakota={"aliases": ["num_set_values"]},
    )
    elements: list[int] = DakotaField(
        description="The permissible values for each discrete variable",
        dakota={"aliases": ["set_values"]},
    )
    set_probabilities: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="This keyword defines the probabilities for the various elements of discrete sets.",
        dakota={"aliases": ["set_probs"]},
    )
    categorical: list[str] | None = DakotaField(
        default=None,
        description="Whether the set-valued variables are categorical or relaxable",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_uncertain_set_int.categorical",
                    "storage_type": "CATEGORICAL",
                    "ir_value_type": "BitArray",
                }
            ]
        },
    )
    initial_point: list[int] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_uncertain_set_int.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_uncertain_set_int.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )

    # Extra fields (not derived from XML grammar)
    initial_point_user_provided: bool = DakotaField(
        default=False,
        description="Internal-only flag indicating the user explicitly provided initial_point.",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.uncertain.initial_point_flag",
                    "storage_type": "UNCERTAIN_INIT_POINT_FLAG",
                    "ir_value_type": "bool",
                }
            ],
        },
    )

    _COMPUTED_FIELDS: ClassVar[dict[str, ComputedFieldSpec]] = {
        "lower_bounds": ComputedFieldSpec(
            function="discrete_set_int_lower_bounds",
            description="Lower bounds: min element per variable",
            return_type=list[int],
            materialization=[
                {
                    "ir_key": "variables.discrete_uncertain_set_int.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        ),
        "upper_bounds": ComputedFieldSpec(
            function="discrete_set_int_upper_bounds",
            description="Upper bounds: max element per variable",
            return_type=list[int],
            materialization=[
                {
                    "ir_key": "variables.discrete_uncertain_set_int.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        ),
    }

    @computed_field
    @property
    def lower_bounds(self) -> List[int]:
        """Lower bounds: min element per variable"""
        return DiscreteSetIntBounds.lower_bounds(self)

    @computed_field
    @property
    def upper_bounds(self) -> List[int]:
        """Upper bounds: max element per variable"""
        return DiscreteSetIntBounds.upper_bounds(self)


class DiscreteUncertainSetString(DakotaBaseModel):
    """Generated model for DiscreteUncertainSetString"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="variables_discrete_uncertain_set_string",
            list_field="elements_per_variable",
            target_field="count",
        ),
        CompareLength(
            context="variables_discrete_uncertain_set_string",
            list_field="initial_point",
            target_field="count",
        ),
        CompareLength(
            context="variables_discrete_uncertain_set_string",
            list_field="descriptors",
            target_field="count",
        ),
        CheckVariableDescriptorsLength(context="string"),
        CheckDescriptorsValid(context="string", check_uniqueness=False),
        DefaultVariableDescriptors(context="string", prefix="dussv_"),
        DiscreteUncertainSetStrInitial(context="string"),
        CheckSumEqualsLength(
            context="string",
            num_list_field="elements_per_variable",
            levels_list_field="elements",
        ),
        CheckSetElementsOrdering(context="string"),
        CompareListLengths(
            context="string", list_a="set_probabilities", list_b="elements"
        ),
        DefaultSetProbabilities(context="string"),
    ]

    count: int = DakotaField(
        gt=0,
        description="Discrete, epistemic uncertain variable - strings within a set",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_uncertain_set_string",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.discrete",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.epistemic_uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    elements_per_variable: list[int] | None = DakotaField(
        default=None,
        description="Number of admissible elements for each set variable",
        dakota={"aliases": ["num_set_values"]},
    )
    elements: list[str] = DakotaField(
        description="The permissible values for each discrete variable",
        dakota={"aliases": ["set_values"]},
    )
    set_probabilities: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="This keyword defines the probabilities for the various elements of discrete sets.",
        dakota={"aliases": ["set_probs"]},
    )
    initial_point: list[str] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_uncertain_set_string.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_uncertain_set_string.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )

    # Extra fields (not derived from XML grammar)
    initial_point_user_provided: bool = DakotaField(
        default=False,
        description="Internal-only flag indicating the user explicitly provided initial_point.",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.uncertain.initial_point_flag",
                    "storage_type": "UNCERTAIN_INIT_POINT_FLAG",
                    "ir_value_type": "bool",
                }
            ],
        },
    )

    _COMPUTED_FIELDS: ClassVar[dict[str, ComputedFieldSpec]] = {
        "lower_bounds": ComputedFieldSpec(
            function="discrete_set_str_lower_bounds",
            description="Lower bounds: min element per variable",
            return_type=list[str],
            materialization=[
                {
                    "ir_key": "variables.discrete_uncertain_set_string.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        ),
        "upper_bounds": ComputedFieldSpec(
            function="discrete_set_str_upper_bounds",
            description="Upper bounds: max element per variable",
            return_type=list[str],
            materialization=[
                {
                    "ir_key": "variables.discrete_uncertain_set_string.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        ),
    }

    @computed_field
    @property
    def lower_bounds(self) -> List[str]:
        """Lower bounds: min element per variable"""
        return DiscreteSetStrBounds.lower_bounds(self)

    @computed_field
    @property
    def upper_bounds(self) -> List[str]:
        """Upper bounds: max element per variable"""
        return DiscreteSetStrBounds.upper_bounds(self)


class DiscreteUncertainSetReal(DakotaBaseModel):
    """Generated model for DiscreteUncertainSetReal"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="variables_discrete_uncertain_set_real",
            list_field="elements_per_variable",
            target_field="count",
        ),
        CompareLength(
            context="variables_discrete_uncertain_set_real",
            list_field="categorical",
            target_field="count",
        ),
        CompareLength(
            context="variables_discrete_uncertain_set_real",
            list_field="initial_point",
            target_field="count",
        ),
        CompareLength(
            context="variables_discrete_uncertain_set_real",
            list_field="descriptors",
            target_field="count",
        ),
        CheckVariableDescriptorsLength(context="real"),
        CheckDescriptorsValid(context="real", check_uniqueness=False),
        DefaultVariableDescriptors(context="real", prefix="dusrv_"),
        DiscreteUncertainSetRealInitial(context="real"),
        CheckPermittedValues(
            context="real",
            field_name="categorical",
            permitted_values=["yes", "no", "true", "false", "y", "n", "t", "f"],
        ),
        CheckSumEqualsLength(
            context="real",
            num_list_field="elements_per_variable",
            levels_list_field="elements",
        ),
        CheckSetElementsOrdering(context="real"),
        CompareListLengths(
            context="real", list_a="set_probabilities", list_b="elements"
        ),
        DefaultSetProbabilities(context="real"),
    ]

    count: int = DakotaField(
        gt=0,
        description="Discrete, epistemic uncertain variable - real numbers within a set",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_uncertain_set_real",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.discrete",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.epistemic_uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    elements_per_variable: list[int] | None = DakotaField(
        default=None,
        description="Number of admissible elements for each set variable",
        dakota={"aliases": ["num_set_values"]},
    )
    elements: list[DakotaFloat] = DakotaField(
        description="The permissible values for each discrete variable",
        dakota={"aliases": ["set_values"]},
    )
    set_probabilities: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="This keyword defines the probabilities for the various elements of discrete sets.",
        dakota={"aliases": ["set_probs"]},
    )
    categorical: list[str] | None = DakotaField(
        default=None,
        description="Whether the set-valued variables are categorical or relaxable",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_uncertain_set_real.categorical",
                    "storage_type": "CATEGORICAL",
                    "ir_value_type": "BitArray",
                }
            ]
        },
    )
    initial_point: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_uncertain_set_real.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_uncertain_set_real.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )

    # Extra fields (not derived from XML grammar)
    initial_point_user_provided: bool = DakotaField(
        default=False,
        description="Internal-only flag indicating the user explicitly provided initial_point.",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.uncertain.initial_point_flag",
                    "storage_type": "UNCERTAIN_INIT_POINT_FLAG",
                    "ir_value_type": "bool",
                }
            ],
        },
    )

    _COMPUTED_FIELDS: ClassVar[dict[str, ComputedFieldSpec]] = {
        "lower_bounds": ComputedFieldSpec(
            function="discrete_set_real_lower_bounds",
            description="Lower bounds: min element per variable",
            return_type=list[float],
            materialization=[
                {
                    "ir_key": "variables.discrete_uncertain_set_real.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        ),
        "upper_bounds": ComputedFieldSpec(
            function="discrete_set_real_upper_bounds",
            description="Upper bounds: max element per variable",
            return_type=list[float],
            materialization=[
                {
                    "ir_key": "variables.discrete_uncertain_set_real.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        ),
    }

    @computed_field
    @property
    def lower_bounds(self) -> List[float]:
        """Lower bounds: min element per variable"""
        return DiscreteSetRealBounds.lower_bounds(self)

    @computed_field
    @property
    def upper_bounds(self) -> List[float]:
        """Upper bounds: max element per variable"""
        return DiscreteSetRealBounds.upper_bounds(self)


class ContinuousState(DakotaBaseModel):
    """Generated model for ContinuousState"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="continuous_state", list_field="initial_state", target_field="count"
        ),
        CompareLength(
            context="continuous_state", list_field="lower_bounds", target_field="count"
        ),
        CompareLength(
            context="continuous_state", list_field="upper_bounds", target_field="count"
        ),
        CompareLength(
            context="continuous_state", list_field="descriptors", target_field="count"
        ),
        CheckVariableDescriptorsLength(context="continuous_state"),
        CheckDescriptorsValid(context="continuous_state", check_uniqueness=False),
        DefaultVariableDescriptors(context="continuous_state", prefix="csv_"),
        DefaultBoundsReal(context="continuous_state", literals=[-inf, inf]),
        DefaultInitialPointReal(
            context="continuous_state", target_field="initial_state"
        ),
        CheckVariableBoundsOrdering(context="continuous_state"),
    ]

    count: int = DakotaField(
        gt=0,
        description="State variable - continuous",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.continuous_state",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.continuous",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.state",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    initial_state: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "aliases": ["csv_initial_state"],
            "materialization": [
                {
                    "ir_key": "variables.continuous_state.initial_state",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    lower_bounds: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Specify minimum values",
        dakota={
            "aliases": ["csv_lower_bounds"],
            "materialization": [
                {
                    "ir_key": "variables.continuous_state.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    upper_bounds: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Specify maximium values",
        dakota={
            "aliases": ["csv_upper_bounds"],
            "materialization": [
                {
                    "ir_key": "variables.continuous_state.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "aliases": ["csv_descriptors"],
            "materialization": [
                {
                    "ir_key": "variables.continuous_state.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )


class DiscreteStateRange(DakotaBaseModel):
    """Generated model for DiscreteStateRange"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="discrete_state_range",
            list_field="initial_state",
            target_field="count",
        ),
        CompareLength(
            context="discrete_state_range",
            list_field="lower_bounds",
            target_field="count",
        ),
        CompareLength(
            context="discrete_state_range",
            list_field="upper_bounds",
            target_field="count",
        ),
        CompareLength(
            context="discrete_state_range",
            list_field="descriptors",
            target_field="count",
        ),
        CheckVariableDescriptorsLength(context="discrete_state_range"),
        CheckDescriptorsValid(context="discrete_state_range", check_uniqueness=False),
        DefaultVariableDescriptors(context="discrete_state_range", prefix="dsriv_"),
        DefaultBoundsInt(
            context="discrete_state_range", literals=[-2147483648, 2147483647]
        ),
        DefaultInitialPointInt(
            context="discrete_state_range", target_field="initial_state"
        ),
        CheckVariableBoundsOrdering(context="discrete_state_range"),
    ]

    count: int = DakotaField(
        gt=0,
        description="State variables - discrete range-valued",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_state_range",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.discrete",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.state",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    initial_state: list[int] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "aliases": ["dsv_initial_state"],
            "materialization": [
                {
                    "ir_key": "variables.discrete_state_range.initial_state",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        },
    )
    lower_bounds: list[int] | None = DakotaField(
        default=None,
        description="Specify minimum values",
        dakota={
            "aliases": ["dsv_lower_bounds"],
            "materialization": [
                {
                    "ir_key": "variables.discrete_state_range.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        },
    )
    upper_bounds: list[int] | None = DakotaField(
        default=None,
        description="Specify maximium values",
        dakota={
            "aliases": ["dsv_upper_bounds"],
            "materialization": [
                {
                    "ir_key": "variables.discrete_state_range.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "aliases": ["dsv_descriptors"],
            "materialization": [
                {
                    "ir_key": "variables.discrete_state_range.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )


class DiscreteStateSetInteger(DakotaBaseModel):
    """Generated model for DiscreteStateSetInteger"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="variables_discrete_state_set_integer",
            list_field="categorical",
            target_field="count",
        ),
        CompareLength(
            context="variables_discrete_state_set_integer",
            list_field="initial_state",
            target_field="count",
        ),
        CompareLength(
            context="variables_discrete_state_set_integer",
            list_field="descriptors",
            target_field="count",
        ),
        CompareLengthOne(
            context="variables_discrete_state_set_integer",
            list_field="elements_per_variable",
            target_field="count",
        ),
        CheckVariableDescriptorsLength(context="integer"),
        CheckDescriptorsValid(context="integer", check_uniqueness=False),
        DefaultVariableDescriptors(context="integer", prefix="dssiv_"),
        DiscreteStateSetIntInitial(context="integer"),
        CheckPermittedValues(
            context="integer",
            field_name="categorical",
            permitted_values=["yes", "no", "true", "false", "y", "n", "t", "f"],
        ),
        CheckSumEqualsLength(
            context="integer",
            num_list_field="elements_per_variable",
            levels_list_field="elements",
        ),
        CheckSetElementsOrdering(context="integer"),
    ]

    count: int = DakotaField(
        gt=0,
        description="Discrete state variables, each defined by a set of permissible integers",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_state_set_int",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.discrete",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.state",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    elements_per_variable: list[int] | None = DakotaField(
        default=None,
        description="Number of admissible elements for each set variable",
        dakota={"aliases": ["num_set_values"]},
    )
    elements: list[int] = DakotaField(
        description="The permissible values for each discrete variable",
        dakota={
            "aliases": ["set_values"],
            "materialization": [
                {
                    "ir_key": "variables.discrete_state_set_int.values",
                    "storage_type": "DISCRETE_SET_VALUES",
                    "ir_value_type": "IntSetArray",
                }
            ],
        },
    )
    categorical: list[str] | None = DakotaField(
        default=None,
        description="Whether the set-valued variables are categorical or relaxable",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_state_set_int.categorical",
                    "storage_type": "CATEGORICAL",
                    "ir_value_type": "BitArray",
                }
            ]
        },
    )
    initial_state: list[int] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_state_set_int.initial_state",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_state_set_int.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )

    _COMPUTED_FIELDS: ClassVar[dict[str, ComputedFieldSpec]] = {
        "lower_bounds": ComputedFieldSpec(
            function="discrete_set_int_lower_bounds",
            description="Lower bounds: min element per variable",
            return_type=list[int],
            materialization=[
                {
                    "ir_key": "variables.discrete_state_set_int.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        ),
        "upper_bounds": ComputedFieldSpec(
            function="discrete_set_int_upper_bounds",
            description="Upper bounds: max element per variable",
            return_type=list[int],
            materialization=[
                {
                    "ir_key": "variables.discrete_state_set_int.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        ),
    }

    @computed_field
    @property
    def lower_bounds(self) -> List[int]:
        """Lower bounds: min element per variable"""
        return DiscreteSetIntBounds.lower_bounds(self)

    @computed_field
    @property
    def upper_bounds(self) -> List[int]:
        """Upper bounds: max element per variable"""
        return DiscreteSetIntBounds.upper_bounds(self)


class DiscreteStateSetString(DakotaBaseModel):
    """Generated model for DiscreteStateSetString"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="variables_discrete_state_set_string",
            list_field="initial_state",
            target_field="count",
        ),
        CompareLength(
            context="variables_discrete_state_set_string",
            list_field="descriptors",
            target_field="count",
        ),
        CompareLengthOne(
            context="variables_discrete_state_set_string",
            list_field="elements_per_variable",
            target_field="count",
        ),
        CheckVariableDescriptorsLength(context="string"),
        CheckDescriptorsValid(context="string", check_uniqueness=False),
        DefaultVariableDescriptors(context="string", prefix="dsssv_"),
        DiscreteStateSetStrInitial(context="string"),
        CheckSumEqualsLength(
            context="string",
            num_list_field="elements_per_variable",
            levels_list_field="elements",
        ),
        CheckSetElementsOrdering(context="string"),
    ]

    count: int = DakotaField(
        gt=0,
        description="String-valued discrete state set variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_state_set_string",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.discrete",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.state",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    elements_per_variable: list[int] | None = DakotaField(
        default=None,
        description="Number of admissible elements for each set variable",
        dakota={"aliases": ["num_set_values"]},
    )
    elements: list[str] = DakotaField(
        description="The permissible values for each discrete variable",
        dakota={
            "aliases": ["set_values"],
            "materialization": [
                {
                    "ir_key": "variables.discrete_state_set_string.values",
                    "storage_type": "DISCRETE_SET_VALUES",
                    "ir_value_type": "StringSetArray",
                }
            ],
        },
    )
    initial_state: list[str] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_state_set_string.initial_state",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_state_set_string.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )

    _COMPUTED_FIELDS: ClassVar[dict[str, ComputedFieldSpec]] = {
        "lower_bounds": ComputedFieldSpec(
            function="discrete_set_str_lower_bounds",
            description="Lower bounds: min element per variable",
            return_type=list[str],
            materialization=[
                {
                    "ir_key": "variables.discrete_state_set_string.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        ),
        "upper_bounds": ComputedFieldSpec(
            function="discrete_set_str_upper_bounds",
            description="Upper bounds: max element per variable",
            return_type=list[str],
            materialization=[
                {
                    "ir_key": "variables.discrete_state_set_string.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        ),
    }

    @computed_field
    @property
    def lower_bounds(self) -> List[str]:
        """Lower bounds: min element per variable"""
        return DiscreteSetStrBounds.lower_bounds(self)

    @computed_field
    @property
    def upper_bounds(self) -> List[str]:
        """Upper bounds: max element per variable"""
        return DiscreteSetStrBounds.upper_bounds(self)


class DiscreteStateSetReal(DakotaBaseModel):
    """Generated model for DiscreteStateSetReal"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="variables_discrete_state_set_real",
            list_field="categorical",
            target_field="count",
        ),
        CompareLength(
            context="variables_discrete_state_set_real",
            list_field="initial_state",
            target_field="count",
        ),
        CompareLength(
            context="variables_discrete_state_set_real",
            list_field="descriptors",
            target_field="count",
        ),
        CompareLengthOne(
            context="variables_discrete_state_set_real",
            list_field="elements_per_variable",
            target_field="count",
        ),
        CheckVariableDescriptorsLength(context="real"),
        CheckDescriptorsValid(context="real", check_uniqueness=False),
        DefaultVariableDescriptors(context="real", prefix="dssrv_"),
        DiscreteStateSetRealInitial(context="real"),
        CheckPermittedValues(
            context="real",
            field_name="categorical",
            permitted_values=["yes", "no", "true", "false", "y", "n", "t", "f"],
        ),
        CheckSumEqualsLength(
            context="real",
            num_list_field="elements_per_variable",
            levels_list_field="elements",
        ),
        CheckSetElementsOrdering(context="real"),
    ]

    count: int = DakotaField(
        gt=0,
        description="Discrete state variables, each defined by a set of permissible real numbers",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_state_set_real",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.discrete",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.state",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    elements_per_variable: list[int] | None = DakotaField(
        default=None,
        description="Number of admissible elements for each set variable",
        dakota={"aliases": ["num_set_values"]},
    )
    elements: list[DakotaFloat] = DakotaField(
        description="The permissible values for each discrete variable",
        dakota={
            "aliases": ["set_values"],
            "materialization": [
                {
                    "ir_key": "variables.discrete_state_set_real.values",
                    "storage_type": "DISCRETE_SET_VALUES",
                    "ir_value_type": "RealSetArray",
                }
            ],
        },
    )
    categorical: list[str] | None = DakotaField(
        default=None,
        description="Whether the set-valued variables are categorical or relaxable",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_state_set_real.categorical",
                    "storage_type": "CATEGORICAL",
                    "ir_value_type": "BitArray",
                }
            ]
        },
    )
    initial_state: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_state_set_real.initial_state",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_state_set_real.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )

    _COMPUTED_FIELDS: ClassVar[dict[str, ComputedFieldSpec]] = {
        "lower_bounds": ComputedFieldSpec(
            function="discrete_set_real_lower_bounds",
            description="Lower bounds: min element per variable",
            return_type=list[float],
            materialization=[
                {
                    "ir_key": "variables.discrete_state_set_real.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        ),
        "upper_bounds": ComputedFieldSpec(
            function="discrete_set_real_upper_bounds",
            description="Upper bounds: max element per variable",
            return_type=list[float],
            materialization=[
                {
                    "ir_key": "variables.discrete_state_set_real.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        ),
    }

    @computed_field
    @property
    def lower_bounds(self) -> List[float]:
        """Lower bounds: min element per variable"""
        return DiscreteSetRealBounds.lower_bounds(self)

    @computed_field
    @property
    def upper_bounds(self) -> List[float]:
        """Upper bounds: max element per variable"""
        return DiscreteSetRealBounds.upper_bounds(self)


class DiscreteDesignSetInteger(DakotaBaseModel):
    """Generated model for DiscreteDesignSetInteger"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="variables_discrete_design_set_integer",
            list_field="initial_point",
            target_field="count",
        ),
        CompareLength(
            context="variables_discrete_design_set_integer",
            list_field="descriptors",
            target_field="count",
        ),
        CompareLengthOne(
            context="variables_discrete_design_set_integer",
            list_field="elements_per_variable",
            target_field="count",
        ),
        CompareLength(
            context="variables_discrete_design_set_integer",
            list_field="categorical.flags",
            target_field="count",
        ),
        CheckVariableDescriptorsLength(context="integer"),
        CheckDescriptorsValid(context="integer", check_uniqueness=False),
        DefaultVariableDescriptors(context="integer", prefix="ddsiv_"),
        DiscreteDesignSetIntInitial(context="integer"),
        CheckSumEqualsLength(
            context="integer",
            num_list_field="elements_per_variable",
            levels_list_field="elements",
        ),
        CheckSetElementsOrdering(context="integer"),
        CheckAdjacencyMatrix(
            context="integer",
            adjacency_matrix_path="categorical.adjacency_matrix",
            flags_path="categorical.flags",
            elements_per_variable_path="elements_per_variable",
            elements_path="elements",
            count_path="count",
        ),
    ]

    count: int = DakotaField(
        gt=0,
        description="Integer-valued discrete design variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_design_set_int",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.discrete",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.design",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    elements_per_variable: list[int] | None = DakotaField(
        default=None,
        description="Number of admissible elements for each set variable",
        dakota={"aliases": ["num_set_values"]},
    )
    elements: list[int] = DakotaField(
        description="The permissible values for each discrete variable",
        dakota={
            "aliases": ["set_values"],
            "materialization": [
                {
                    "ir_key": "variables.discrete_design_set_int.values",
                    "storage_type": "DISCRETE_SET_VALUES",
                    "ir_value_type": "IntSetArray",
                }
            ],
        },
    )
    categorical: IntegerCategorical | None = DakotaField(
        default=None,
        description="Whether the set-valued variables are categorical or relaxable",
        dakota={"argument": "flags"},
    )
    initial_point: list[int] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_design_set_int.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_design_set_int.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )

    _COMPUTED_FIELDS: ClassVar[dict[str, ComputedFieldSpec]] = {
        "lower_bounds": ComputedFieldSpec(
            function="discrete_set_int_lower_bounds",
            description="Lower bounds: min element per variable",
            return_type=list[int],
            materialization=[
                {
                    "ir_key": "variables.discrete_design_set_int.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        ),
        "upper_bounds": ComputedFieldSpec(
            function="discrete_set_int_upper_bounds",
            description="Upper bounds: max element per variable",
            return_type=list[int],
            materialization=[
                {
                    "ir_key": "variables.discrete_design_set_int.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        ),
    }

    @computed_field
    @property
    def lower_bounds(self) -> List[int]:
        """Lower bounds: min element per variable"""
        return DiscreteSetIntBounds.lower_bounds(self)

    @computed_field
    @property
    def upper_bounds(self) -> List[int]:
        """Upper bounds: max element per variable"""
        return DiscreteSetIntBounds.upper_bounds(self)


class DiscreteDesignSetReal(DakotaBaseModel):
    """Generated model for DiscreteDesignSetReal"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="variables_discrete_design_set_real",
            list_field="initial_point",
            target_field="count",
        ),
        CompareLength(
            context="variables_discrete_design_set_real",
            list_field="descriptors",
            target_field="count",
        ),
        CompareLengthOne(
            context="variables_discrete_design_set_real",
            list_field="elements_per_variable",
            target_field="count",
        ),
        CompareLength(
            context="variables_discrete_design_set_real",
            list_field="categorical.flags",
            target_field="count",
        ),
        CheckVariableDescriptorsLength(context="real"),
        CheckDescriptorsValid(context="real", check_uniqueness=False),
        DefaultVariableDescriptors(context="real", prefix="ddsrv_"),
        DiscreteDesignSetRealInitial(context="real"),
        CheckSumEqualsLength(
            context="real",
            num_list_field="elements_per_variable",
            levels_list_field="elements",
        ),
        CheckSetElementsOrdering(context="real"),
        CheckAdjacencyMatrix(
            context="real",
            adjacency_matrix_path="categorical.adjacency_matrix",
            flags_path="categorical.flags",
            elements_per_variable_path="elements_per_variable",
            elements_path="elements",
            count_path="count",
        ),
    ]

    count: int = DakotaField(
        gt=0,
        description="Real-valued discrete design variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_design_set_real",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.discrete",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.design",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    elements_per_variable: list[int] | None = DakotaField(
        default=None,
        description="Number of admissible elements for each set variable",
        dakota={"aliases": ["num_set_values"]},
    )
    elements: list[DakotaFloat] = DakotaField(
        description="The permissible values for each discrete variable",
        dakota={
            "aliases": ["set_values"],
            "materialization": [
                {
                    "ir_key": "variables.discrete_design_set_real.values",
                    "storage_type": "DISCRETE_SET_VALUES",
                    "ir_value_type": "RealSetArray",
                }
            ],
        },
    )
    categorical: RealCategorical | None = DakotaField(
        default=None,
        description="Whether the set-valued variables are categorical or relaxable",
        dakota={"argument": "flags"},
    )
    initial_point: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_design_set_real.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.discrete_design_set_real.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )

    _COMPUTED_FIELDS: ClassVar[dict[str, ComputedFieldSpec]] = {
        "lower_bounds": ComputedFieldSpec(
            function="discrete_set_real_lower_bounds",
            description="Lower bounds: min element per variable",
            return_type=list[float],
            materialization=[
                {
                    "ir_key": "variables.discrete_design_set_real.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        ),
        "upper_bounds": ComputedFieldSpec(
            function="discrete_set_real_upper_bounds",
            description="Upper bounds: max element per variable",
            return_type=list[float],
            materialization=[
                {
                    "ir_key": "variables.discrete_design_set_real.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        ),
    }

    @computed_field
    @property
    def lower_bounds(self) -> List[float]:
        """Lower bounds: min element per variable"""
        return DiscreteSetRealBounds.lower_bounds(self)

    @computed_field
    @property
    def upper_bounds(self) -> List[float]:
        """Upper bounds: max element per variable"""
        return DiscreteSetRealBounds.upper_bounds(self)


class Option1(DakotaBaseModel):
    """Generated model for Option1"""

    option_1: Option1Config = DakotaField(
        description="Specify means and standard deviations for the lognormal distributions"
    )


class Option2(DakotaBaseModel):
    """Generated model for Option2"""

    option_2: Option2Config = DakotaField(
        description="Specify means and error factors for the lognormal distributions"
    )


class Option3(DakotaBaseModel):
    """Generated model for Option3"""

    option_3: Option3Config = DakotaField(
        description="Specify lambdas and zetas for the lognormal distributions"
    )


class HistogramBinUncertain(DakotaBaseModel):
    """Generated model for HistogramBinUncertain"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="histogram_bin_uncertain",
            list_field="pairs_per_variable",
            target_field="count",
        ),
        CompareLength(
            context="histogram_bin_uncertain",
            list_field="initial_point",
            target_field="count",
        ),
        CompareLength(
            context="histogram_bin_uncertain",
            list_field="descriptors",
            target_field="count",
        ),
        CheckVariableDescriptorsLength(context="histogram_bin_uncertain"),
        CheckDescriptorsValid(
            context="histogram_bin_uncertain", check_uniqueness=False
        ),
        DefaultVariableDescriptors(context="histogram_bin_uncertain", prefix="hbuv_"),
        CheckSumEqualsLength(
            context="histogram_bin_uncertain",
            num_list_field="pairs_per_variable",
            levels_list_field="abscissas",
        ),
        CheckSetElementsOrdering(
            context="histogram_bin_uncertain",
            elements_per_variable_field="pairs_per_variable",
            elements_field="abscissas",
        ),
        CompareListLengths(
            context="histogram_bin_uncertain",
            list_a="density.ordinates",
            list_b="abscissas",
        ),
        CompareListLengths(
            context="histogram_bin_uncertain",
            list_a="density.counts",
            list_b="abscissas",
        ),
        HistogramBinUncertainInitial(context="histogram_bin_uncertain"),
    ]

    count: int = DakotaField(
        gt=0,
        description="Aleatory uncertain variable - continuous histogram",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.histogram_uncertain.bin",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.continuous",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.aleatory_uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    pairs_per_variable: list[int] | None = DakotaField(
        default=None,
        description="Number of pairs defining each histogram bin variable",
        dakota={"aliases": ["num_pairs"]},
    )
    abscissas: list[DakotaFloat] = DakotaField(
        description="Real abscissas for a bin histogram",
        dakota={"aliases": ["huv_bin_abscissas"]},
    )
    density: Union[Ordinates, Counts] = DakotaField(
        description="Density Values", dakota={"anchor": True, "union_pattern": 4}
    )
    initial_point: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.histogram_bin_uncertain.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "aliases": ["huv_bin_descriptors"],
            "materialization": [
                {
                    "ir_key": "variables.histogram_bin_uncertain.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )

    # Extra fields (not derived from XML grammar)
    initial_point_user_provided: bool = DakotaField(
        default=False,
        description="Internal-only flag indicating the user explicitly provided initial_point.",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.uncertain.initial_point_flag",
                    "storage_type": "UNCERTAIN_INIT_POINT_FLAG",
                    "ir_value_type": "bool",
                }
            ],
        },
    )

    _COMPUTED_FIELDS: ClassVar[dict[str, ComputedFieldSpec]] = {
        "lower_bounds": ComputedFieldSpec(
            function="histogram_bin_lower_bounds",
            description="Lower bounds: first abscissa per variable",
            return_type=list[float],
            materialization=[
                {
                    "ir_key": "variables.histogram_bin_uncertain.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        ),
        "upper_bounds": ComputedFieldSpec(
            function="histogram_bin_upper_bounds",
            description="Upper bounds: last abscissa per variable",
            return_type=list[float],
            materialization=[
                {
                    "ir_key": "variables.histogram_bin_uncertain.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        ),
    }

    @computed_field
    @property
    def lower_bounds(self) -> List[float]:
        """Lower bounds: first abscissa per variable"""
        return HistogramBinBounds.lower_bounds(self)

    @computed_field
    @property
    def upper_bounds(self) -> List[float]:
        """Upper bounds: last abscissa per variable"""
        return HistogramBinBounds.upper_bounds(self)


class HistogramPointUncertain(DakotaBaseModel):
    """Generated model for HistogramPointUncertain"""

    integer: HistogramPointUncertainInteger | None = DakotaField(
        default=None,
        description="Integer valued point histogram variable",
        dakota={
            "argument": "count",
            "materialization": [
                {
                    "ir_key": "variables.histogram_uncertain.point_int_pairs",
                    "storage_type": "HISTOGRAM_POINT_UNCERTAIN",
                    "ir_value_type": "IntRealMapArray",
                }
            ],
        },
    )
    string: HistogramPointUncertainString | None = DakotaField(
        default=None,
        description="String (categorical) valued point histogram variable",
        dakota={
            "argument": "count",
            "materialization": [
                {
                    "ir_key": "variables.histogram_uncertain.point_string_pairs",
                    "storage_type": "HISTOGRAM_POINT_UNCERTAIN",
                    "ir_value_type": "StringRealMapArray",
                }
            ],
        },
    )
    real: HistogramPointUncertainReal | None = DakotaField(
        default=None,
        description="Real valued point histogram variable",
        dakota={
            "argument": "count",
            "materialization": [
                {
                    "ir_key": "variables.histogram_uncertain.point_real_pairs",
                    "storage_type": "HISTOGRAM_POINT_UNCERTAIN",
                    "ir_value_type": "RealRealMapArray",
                }
            ],
        },
    )


class DiscreteUncertainSet(DakotaBaseModel):
    """Generated model for DiscreteUncertainSet"""

    integer: DiscreteUncertainSetInteger | None = DakotaField(
        default=None,
        description="Discrete, epistemic uncertain variable - integers within a set",
        dakota={
            "argument": "count",
            "materialization": [
                {
                    "ir_key": "variables.discrete_uncertain_set_int.values_probs",
                    "storage_type": "DISCRETE_UNCERTAIN_SET_VALUES_PROBS",
                    "ir_value_type": "IntRealMapArray",
                }
            ],
        },
    )
    string: DiscreteUncertainSetString | None = DakotaField(
        default=None,
        description="Discrete, epistemic uncertain variable - strings within a set",
        dakota={
            "argument": "count",
            "materialization": [
                {
                    "ir_key": "variables.discrete_uncertain_set_string.values_probs",
                    "storage_type": "DISCRETE_UNCERTAIN_SET_VALUES_PROBS",
                    "ir_value_type": "StringRealMapArray",
                }
            ],
        },
    )
    real: DiscreteUncertainSetReal | None = DakotaField(
        default=None,
        description="Discrete, epistemic uncertain variable - real numbers within a set",
        dakota={
            "argument": "count",
            "materialization": [
                {
                    "ir_key": "variables.discrete_uncertain_set_real.values_probs",
                    "storage_type": "DISCRETE_UNCERTAIN_SET_VALUES_PROBS",
                    "ir_value_type": "RealRealMapArray",
                }
            ],
        },
    )


class DiscreteStateSet(DakotaBaseModel):
    """Generated model for DiscreteStateSet"""

    integer: DiscreteStateSetInteger | None = DakotaField(
        default=None,
        description="Discrete state variables, each defined by a set of permissible integers",
        dakota={
            "argument": "count",
            "materialization": [
                {
                    "ir_key": "variables.discrete_state_set_int.values",
                    "storage_type": "DISCRETE_SET_VALUES",
                    "ir_value_type": "IntSetArray",
                }
            ],
        },
    )
    string: DiscreteStateSetString | None = DakotaField(
        default=None,
        description="String-valued discrete state set variables",
        dakota={
            "argument": "count",
            "materialization": [
                {
                    "ir_key": "variables.discrete_state_set_string.values",
                    "storage_type": "DISCRETE_SET_VALUES",
                    "ir_value_type": "StringSetArray",
                }
            ],
        },
    )
    real: DiscreteStateSetReal | None = DakotaField(
        default=None,
        description="Discrete state variables, each defined by a set of permissible real numbers",
        dakota={
            "argument": "count",
            "materialization": [
                {
                    "ir_key": "variables.discrete_state_set_real.values",
                    "storage_type": "DISCRETE_SET_VALUES",
                    "ir_value_type": "RealSetArray",
                }
            ],
        },
    )


class DiscreteDesignSet(DakotaBaseModel):
    """Generated model for DiscreteDesignSet"""

    integer: DiscreteDesignSetInteger | None = DakotaField(
        default=None,
        description="Integer-valued discrete design variables",
        dakota={
            "argument": "count",
            "materialization": [
                {
                    "ir_key": "variables.discrete_design_set_int.values",
                    "storage_type": "DISCRETE_SET_VALUES",
                    "ir_value_type": "IntSetArray",
                }
            ],
        },
    )
    string: DiscreteDesignSetString | None = DakotaField(
        default=None,
        description="String-valued discrete design set variables",
        dakota={
            "argument": "count",
            "materialization": [
                {
                    "ir_key": "variables.discrete_design_set_string.values",
                    "storage_type": "DISCRETE_SET_VALUES",
                    "ir_value_type": "StringSetArray",
                }
            ],
        },
    )
    real: DiscreteDesignSetReal | None = DakotaField(
        default=None,
        description="Real-valued discrete design variables",
        dakota={
            "argument": "count",
            "materialization": [
                {
                    "ir_key": "variables.discrete_design_set_real.values",
                    "storage_type": "DISCRETE_SET_VALUES",
                    "ir_value_type": "RealSetArray",
                }
            ],
        },
    )


class LognormalUncertain(DakotaBaseModel):
    """Generated model for LognormalUncertain"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CompareLength(
            context="lognormal_uncertain",
            list_field="lower_bounds",
            target_field="count",
        ),
        CompareLength(
            context="lognormal_uncertain",
            list_field="upper_bounds",
            target_field="count",
        ),
        CompareLength(
            context="lognormal_uncertain",
            list_field="initial_point",
            target_field="count",
        ),
        CompareLength(
            context="lognormal_uncertain",
            list_field="descriptors",
            target_field="count",
        ),
        CompareLength(
            context="lognormal_uncertain",
            list_field="parameters.option_1.means",
            target_field="count",
        ),
        CompareLength(
            context="lognormal_uncertain",
            list_field="parameters.option_1.std_deviations",
            target_field="count",
        ),
        CompareLength(
            context="lognormal_uncertain",
            list_field="parameters.option_2.means",
            target_field="count",
        ),
        CompareLength(
            context="lognormal_uncertain",
            list_field="parameters.option_2.error_factors",
            target_field="count",
        ),
        CompareLength(
            context="lognormal_uncertain",
            list_field="parameters.option_3.lambdas",
            target_field="count",
        ),
        CompareLength(
            context="lognormal_uncertain",
            list_field="parameters.option_3.zetas",
            target_field="count",
        ),
        CheckVariableDescriptorsLength(context="lognormal_uncertain"),
        CheckDescriptorsValid(context="lognormal_uncertain", check_uniqueness=False),
        DefaultVariableDescriptors(context="lognormal_uncertain", prefix="lnuv_"),
        LognormalUncertainBounds(context="lognormal_uncertain"),
        LognormalUncertainInitial(context="lognormal_uncertain"),
        CheckVariableBoundsOrdering(context="lognormal_uncertain"),
        CheckRealLowerBound(
            context="lognormal_uncertain",
            list_field="lower_bounds",
            lower_bound=0,
            inclusive=True,
        ),
        CheckRealUpperBound(
            context="lognormal_uncertain",
            list_field="upper_bounds",
            upper_bound=inf,
            inclusive=True,
        ),
    ]

    count: int = DakotaField(
        gt=0,
        description="Aleatory uncertain variable - lognormal",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.lognormal_uncertain",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.continuous",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.aleatory_uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.uncertain",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
                {
                    "ir_key": "variables.total",
                    "storage_type": "ADD_TO_VALUE",
                    "ir_value_type": "size_t",
                },
            ]
        },
    )
    parameters: Union[Option1, Option2, Option3] = DakotaField(
        description="Select a parmaterization of the lognormal distribution",
        dakota={"union_pattern": 4},
    )
    lower_bounds: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Specify minimum values",
        dakota={
            "aliases": ["lnuv_lower_bounds"],
            "materialization": [
                {
                    "ir_key": "variables.lognormal_uncertain.lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    upper_bounds: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Specify maximium values",
        dakota={
            "aliases": ["lnuv_upper_bounds"],
            "materialization": [
                {
                    "ir_key": "variables.lognormal_uncertain.upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )
    initial_point: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Initial values for variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.lognormal_uncertain.initial_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    descriptors: list[str] | None = DakotaField(
        default=None,
        description="Labels for the variables",
        dakota={
            "aliases": ["lnuv_descriptors"],
            "materialization": [
                {
                    "ir_key": "variables.lognormal_uncertain.labels",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )

    # Extra fields (not derived from XML grammar)
    initial_point_user_provided: bool = DakotaField(
        default=False,
        description="Internal-only flag indicating the user explicitly provided initial_point.",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.uncertain.initial_point_flag",
                    "storage_type": "UNCERTAIN_INIT_POINT_FLAG",
                    "ir_value_type": "bool",
                }
            ],
        },
    )
    inferred_upper_bounds: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Internal use only",
        dakota={
            "internal_only": True,
            "materialization": [
                {
                    "ir_key": "variables.lognormal_uncertain.inferred_upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )


class VariablesConfig(DakotaBaseModel):
    """Generated model for VariablesConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckAllVariableDescriptorsUnique(context="variables"),
        CheckPermittedValues(
            context="variables",
            field_name="linear_inequality_scale_types",
            permitted_values=["value", "auto", "none"],
        ),
        CheckScalesRequired(
            context="variables",
            scale_types_field="linear_inequality_scale_types",
            scales_field="linear_inequality_scales",
        ),
        CheckPermittedValues(
            context="variables",
            field_name="linear_equality_scale_types",
            permitted_values=["value", "auto", "none"],
        ),
        CheckScalesRequired(
            context="variables",
            scale_types_field="linear_equality_scale_types",
            scales_field="linear_equality_scales",
        ),
        CheckLinearInequalityBoundsOrdering(context="variables"),
        UncertainCorrelationMatrixSize(context="variables"),
    ]

    id_variables: str | None = DakotaField(
        default=None,
        description="Name the variables block; helpful when there are multiple",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.id",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    active: Union[ActiveAll, Design, Uncertain, Aleatory, Epistemic, State] | None = (
        DakotaField(
            default=None,
            description="Set the active variables view a method will see",
            dakota={"union_pattern": 2},
        )
    )
    domain: Union[Mixed, Relaxed] | None = DakotaField(
        default=None,
        description="Variable Domain",
        dakota={"anchor": True, "union_pattern": 2},
    )
    continuous_design: ContinuousDesign | None = DakotaField(
        default=None,
        description="Design variable - continuous",
        dakota={"argument": "count"},
    )
    discrete_design_range: DiscreteDesignRange | None = DakotaField(
        default=None,
        description="Design variable - discrete range-valued",
        dakota={"argument": "count"},
    )
    discrete_design_set: DiscreteDesignSet | None = DakotaField(
        default=None, description="Design variable - discrete set-valued"
    )
    normal_uncertain: NormalUncertain | None = DakotaField(
        default=None,
        description="Aleatory uncertain variable - normal (Gaussian)",
        dakota={"argument": "count"},
    )
    lognormal_uncertain: LognormalUncertain | None = DakotaField(
        default=None,
        description="Aleatory uncertain variable - lognormal",
        dakota={"argument": "count"},
    )
    uniform_uncertain: UniformUncertain | None = DakotaField(
        default=None,
        description="Aleatory uncertain variable - uniform",
        dakota={"argument": "count"},
    )
    loguniform_uncertain: LoguniformUncertain | None = DakotaField(
        default=None,
        description="Aleatory uncertain variable - loguniform",
        dakota={"argument": "count"},
    )
    triangular_uncertain: TriangularUncertain | None = DakotaField(
        default=None,
        description="Aleatory uncertain variable - triangular",
        dakota={"argument": "count"},
    )
    exponential_uncertain: ExponentialUncertain | None = DakotaField(
        default=None,
        description="Aleatory uncertain variable - exponential",
        dakota={"argument": "count"},
    )
    beta_uncertain: BetaUncertain | None = DakotaField(
        default=None,
        description="Aleatory uncertain variable - beta",
        dakota={"argument": "count"},
    )
    gamma_uncertain: GammaUncertain | None = DakotaField(
        default=None,
        description="Aleatory uncertain variable - gamma",
        dakota={"argument": "count"},
    )
    gumbel_uncertain: GumbelUncertain | None = DakotaField(
        default=None,
        description="Aleatory uncertain variable - gumbel",
        dakota={"argument": "count"},
    )
    frechet_uncertain: FrechetUncertain | None = DakotaField(
        default=None,
        description="Aleatory uncertain variable - Frechet",
        dakota={"argument": "count"},
    )
    weibull_uncertain: WeibullUncertain | None = DakotaField(
        default=None,
        description="Aleatory uncertain variable - Weibull",
        dakota={"argument": "count"},
    )
    histogram_bin_uncertain: HistogramBinUncertain | None = DakotaField(
        default=None,
        description="Aleatory uncertain variable - continuous histogram",
        dakota={
            "argument": "count",
            "materialization": [
                {
                    "ir_key": "variables.histogram_uncertain.bin_pairs",
                    "storage_type": "HISTOGRAM_BIN_UNCERTAIN",
                    "ir_value_type": "RealRealMapArray",
                }
            ],
        },
    )
    poisson_uncertain: PoissonUncertain | None = DakotaField(
        default=None,
        description="Aleatory uncertain discrete variable - Poisson",
        dakota={"argument": "count"},
    )
    binomial_uncertain: BinomialUncertain | None = DakotaField(
        default=None,
        description="Aleatory uncertain discrete variable - binomial",
        dakota={"argument": "count"},
    )
    negative_binomial_uncertain: NegativeBinomialUncertain | None = DakotaField(
        default=None,
        description="Aleatory uncertain discrete variable - negative binomial",
        dakota={"argument": "count"},
    )
    geometric_uncertain: GeometricUncertain | None = DakotaField(
        default=None,
        description="Aleatory uncertain discrete variable - geometric",
        dakota={"argument": "count"},
    )
    hypergeometric_uncertain: HypergeometricUncertain | None = DakotaField(
        default=None,
        description="Aleatory uncertain discrete variable - hypergeometric",
        dakota={"argument": "count"},
    )
    histogram_point_uncertain: HistogramPointUncertain | None = DakotaField(
        default=None, description="Aleatory uncertain variable - discrete histogram"
    )
    uncertain_correlation_matrix: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Correlation among aleatory uncertain variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.uncertain.correlation_matrix",
                    "storage_type": "UNCERTAIN_CORRELATION_MATRIX",
                    "ir_value_type": "RealSymMatrix",
                }
            ]
        },
    )
    continuous_interval_uncertain: ContinuousIntervalUncertain | None = DakotaField(
        default=None,
        description="Epistemic uncertain variable - values from one or more continuous intervals",
        dakota={
            "argument": "count",
            "aliases": ["interval_uncertain"],
            "materialization": [
                {
                    "ir_key": "variables.continuous_interval_uncertain.basic_probs",
                    "storage_type": "CONTINUOUS_INTERVAL_UNCERTAIN",
                    "ir_value_type": "RealRealPairRealMapArray",
                }
            ],
        },
    )
    discrete_interval_uncertain: DiscreteIntervalUncertain | None = DakotaField(
        default=None,
        description="Epistemic uncertain variable - values from one or more discrete intervals",
        dakota={
            "argument": "count",
            "materialization": [
                {
                    "ir_key": "variables.discrete_interval_uncertain.basic_probs",
                    "storage_type": "DISCRETE_INTERVAL_UNCERTAIN",
                    "ir_value_type": "IntIntPairRealMapArray",
                }
            ],
        },
    )
    discrete_uncertain_set: DiscreteUncertainSet | None = DakotaField(
        default=None, description="Epistemic uncertain variable - discrete set-valued"
    )
    continuous_state: ContinuousState | None = DakotaField(
        default=None,
        description="State variable - continuous",
        dakota={"argument": "count"},
    )
    discrete_state_range: DiscreteStateRange | None = DakotaField(
        default=None,
        description="State variables - discrete range-valued",
        dakota={"argument": "count"},
    )
    discrete_state_set: DiscreteStateSet | None = DakotaField(
        default=None, description="State variable - discrete set-valued"
    )
    linear_inequality_constraint_matrix: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Define coefficients of the linear inequality constraints",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.linear_inequality_constraints",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    linear_inequality_lower_bounds: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Define lower bounds for the linear inequality constraint",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.linear_inequality_lower_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    linear_inequality_upper_bounds: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Define upper bounds for the linear inequality constraint",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.linear_inequality_upper_bounds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    linear_inequality_scale_types: list[str] | None = DakotaField(
        default=None,
        description="How to scale each linear inequality constraint",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.linear_inequality_scale_types",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )
    linear_inequality_scales: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Characteristic values to scale linear inequalities",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.linear_inequality_scales",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    linear_equality_constraint_matrix: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Define coefficients of the linear equalities",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.linear_equality_constraints",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    linear_equality_targets: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Define target values for the linear equality constraints",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.linear_equality_targets",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    linear_equality_scale_types: list[str] | None = DakotaField(
        default=None,
        description="How to scale each linear equality constraint",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.linear_equality_scale_types",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )
    linear_equality_scales: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Characteristic values to scale linear equalities",
        dakota={
            "materialization": [
                {
                    "ir_key": "variables.linear_equality_scales",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
