"""Generated Pydantic models for shared.expansion/options"""

from __future__ import annotations

from ....base import DakotaBaseModel, DakotaField, DakotaFloat
from typing import Literal, Union
from ....base import USHRT_MAX
from typing import ClassVar, List
from ....validation import ValidationRule
from ....validation.rules import (
    CheckNonnegativeList,
    CheckProbabilityList,
    CheckSumEqualsLength,
    CompareLength,
    CompareLengthOne,
)


class ExpansionDimPrefMixin(DakotaBaseModel):
    "Generated model for ExpansionDimPrefMixin"

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


class ExpansionOptionsSampleTypeLhs(DakotaBaseModel):
    "Uses Latin Hypercube Sampling (LHS) to sample variables"

    lhs: Literal[True] = DakotaField(
        default=True,
        description="Uses Latin Hypercube Sampling (LHS) to sample variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sample_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_LHS",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExpansionOptionsSampleTypeRandom(DakotaBaseModel):
    "Uses purely random Monte Carlo sampling to sample variables"

    random: Literal[True] = DakotaField(
        default=True,
        description="Uses purely random Monte Carlo sampling to sample variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sample_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_RANDOM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExpansionOptionsRngMt19937(DakotaBaseModel):
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


class ExpansionOptionsRngRnum2(DakotaBaseModel):
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


class ExpansionOptionsProbabilityRefinementImportance(DakotaBaseModel):
    "Importance sampling option for probability refinement"

    importance: Literal[True] = DakotaField(
        default=True,
        description="Importance sampling option for probability refinement",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.integration_refinement",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "IS",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExpansionOptionsProbabilityRefinementAdaptImport(DakotaBaseModel):
    "Importance sampling option for probability refinement"

    adapt_import: Literal[True] = DakotaField(
        default=True,
        description="Importance sampling option for probability refinement",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.integration_refinement",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "AIS",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExpansionOptionsProbabilityRefinementMmAdaptImport(DakotaBaseModel):
    "Importance sampling option for probability refinement"

    mm_adapt_import: Literal[True] = DakotaField(
        default=True,
        description="Importance sampling option for probability refinement",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.integration_refinement",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MMAIS",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExpansionOptionsFinalMomentsNoneKeyword(DakotaBaseModel):
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


class ExpansionOptionsFinalMomentsStandard(DakotaBaseModel):
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


class ExpansionOptionsFinalMomentsCentral(DakotaBaseModel):
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


class ExpansionOptionsResponseLevelsComputeProbabilities(DakotaBaseModel):
    "Computes probabilities associated with response levels"

    probabilities: Literal[True] = DakotaField(
        default=True,
        description="Computes probabilities associated with response levels",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_level_target",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "PROBABILITIES",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExpansionOptionsResponseLevelsComputeReliabilities(DakotaBaseModel):
    "Computes reliabilities associated with response levels"

    reliabilities: Literal[True] = DakotaField(
        default=True,
        description="Computes reliabilities associated with response levels",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_level_target",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RELIABILITIES",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExpansionOptionsResponseLevelsComputeGenReliabilities(DakotaBaseModel):
    "Computes generalized reliabilities associated with response levels"

    gen_reliabilities: Literal[True] = DakotaField(
        default=True,
        description="Computes generalized reliabilities associated with response levels",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_level_target",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "GEN_RELIABILITIES",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExpansionOptionsResponseLevelsComputeSystemSeries(DakotaBaseModel):
    "Aggregate response statistics assuming a series system"

    series: Literal[True] = DakotaField(
        default=True,
        description="Aggregate response statistics assuming a series system",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_level_target_reduce",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SYSTEM_SERIES",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExpansionOptionsResponseLevelsComputeSystemParallel(DakotaBaseModel):
    "Aggregate response statistics assuming a parallel system"

    parallel: Literal[True] = DakotaField(
        default=True,
        description="Aggregate response statistics assuming a parallel system",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_level_target_reduce",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SYSTEM_PARALLEL",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExpansionOptionsProbabilityLevels(DakotaBaseModel):
    "Specify probability levels at which to estimate the corresponding response value"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckProbabilityList(
            context="expansionoptionsprobabilitylevels", list_field="values"
        ),
        CheckSumEqualsLength(
            context="expansionoptionsprobabilitylevels",
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


class ExpansionOptionsReliabilityLevels(DakotaBaseModel):
    "Specify reliability levels at which the response values will be estimated"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckSumEqualsLength(
            context="expansionoptionsreliabilitylevels",
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


class ExpansionOptionsGenReliabilityLevels(DakotaBaseModel):
    "Specify generalized relability levels at which to estimate the corresponding response value"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckSumEqualsLength(
            context="expansionoptionsgenreliabilitylevels",
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


class ExpansionOptionsDistributionCumulative(DakotaBaseModel):
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


class ExpansionOptionsDistributionComplementary(DakotaBaseModel):
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


class ExpansionOptionsVarianceBasedDecomp(DakotaBaseModel):
    "Activates global sensitivity analysis based on decomposition of response variance into main, interaction, and total effects"

    interaction_order: int | None = DakotaField(
        default=None,
        gt=0,
        description="Specify the maximum number of variables allowed in an interaction when reporting interaction metrics.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.vbd_interaction_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
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


class ImportApproxPointsFileCustomAnnotatedConfig(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_approx_format",
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
                    "ir_key": "method.import_approx_format",
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
                    "ir_key": "method.import_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ImportApproxPointsFileAnnotated(DakotaBaseModel):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ImportApproxPointsFileFreeform(DakotaBaseModel):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExpansionOptionsExportApproxPointsFileCustomAnnotatedConfig(DakotaBaseModel):
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


class ExpansionOptionsExportApproxPointsFileAnnotated(DakotaBaseModel):
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


class ExpansionOptionsExportApproxPointsFileFreeform(DakotaBaseModel):
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


class ExpansionOptionsDiagCov(DakotaBaseModel):
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


class ExpansionOptionsFullCov(DakotaBaseModel):
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


class ExpansionQuadratureOrderNested(DakotaBaseModel):
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


class ExpansionQuadratureOrderNonNested(DakotaBaseModel):
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


class ExpansionQuadratureSequenceOrderSequenceNested(DakotaBaseModel):
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


class ExpansionQuadratureSequenceOrderSequenceNonNested(DakotaBaseModel):
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


class ExpansionRuleGrowthRestricted(DakotaBaseModel):
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


class ExpansionRuleGrowthUnrestricted(DakotaBaseModel):
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


class ExpansionRuleNestingNested(DakotaBaseModel):
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


class ExpansionRuleNestingNonNested(DakotaBaseModel):
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


class VbdExpansionVarianceBasedDecomp(DakotaBaseModel):
    "Activates global sensitivity analysis based on decomposition of response variance into main, interaction, and total effects"

    interaction_order: int | None = DakotaField(
        default=None,
        gt=0,
        description="Specify the maximum number of variables allowed in an interaction when reporting interaction metrics.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.vbd_interaction_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
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


class ExpansionOptionsProbabilityRefinement(DakotaBaseModel):
    "Allow refinement of probability and generalized reliability results using importance sampling"

    importance_sampling_approach: Union[
        ExpansionOptionsProbabilityRefinementImportance,
        ExpansionOptionsProbabilityRefinementAdaptImport,
        ExpansionOptionsProbabilityRefinementMmAdaptImport,
    ] = DakotaField(
        description="Importance Sampling Approach",
        dakota={"anchor": True, "union_pattern": 4},
    )
    refinement_samples: list[int] | None = DakotaField(
        default=None,
        description="Number of samples used to refine a probability estimate or sampling design.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.refinement_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )


class ExpansionOptionsResponseLevelsCompute(DakotaBaseModel):
    "Selection of statistics to compute at each response level"

    statistic: Union[
        ExpansionOptionsResponseLevelsComputeProbabilities,
        ExpansionOptionsResponseLevelsComputeReliabilities,
        ExpansionOptionsResponseLevelsComputeGenReliabilities,
    ] = DakotaField(
        description="Statistics to Compute", dakota={"anchor": True, "union_pattern": 4}
    )
    system: (
        Union[
            ExpansionOptionsResponseLevelsComputeSystemSeries,
            ExpansionOptionsResponseLevelsComputeSystemParallel,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Compute system reliability (series or parallel)",
        dakota={"union_pattern": 2},
    )


class ImportApproxPointsFileCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: ImportApproxPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=ImportApproxPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "ImportApproxPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class ExpansionOptionsExportApproxPointsFileCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: ExpansionOptionsExportApproxPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=ExpansionOptionsExportApproxPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "ExpansionOptionsExportApproxPointsFileCustomAnnotatedConfig",
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


class ExpansionQuadratureQuadratureOrder(DakotaBaseModel):
    "Order for tensor-products of Gaussian quadrature rules"

    order: int = DakotaField(
        default=USHRT_MAX,
        description="Order for tensor-products of Gaussian quadrature rules",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.quadrature_order",
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
    nesting_rule: (
        Union[ExpansionQuadratureOrderNested, ExpansionQuadratureOrderNonNested] | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class ExpansionQuadratureSequenceQuadratureOrderSequence(DakotaBaseModel):
    "Sequence of quadrature orders used in a multi-stage expansion"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="expansionquadraturesequencequadratureordersequence",
            list_field="sequence",
        ),
    ]

    sequence: list[int] = DakotaField(
        description="Sequence of quadrature orders used in a multi-stage expansion",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.quadrature_order_sequence",
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
    nesting_rule: (
        Union[
            ExpansionQuadratureSequenceOrderSequenceNested,
            ExpansionQuadratureSequenceOrderSequenceNonNested,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class ExpansionRuleGrowthMixin(DakotaBaseModel):
    "Generated model for ExpansionRuleGrowthMixin"

    growth_rule: (
        Union[ExpansionRuleGrowthRestricted, ExpansionRuleGrowthUnrestricted] | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Growth",
        dakota={"anchor": True, "union_pattern": 2},
    )


class ExpansionRuleNestingMixin(DakotaBaseModel):
    "Generated model for ExpansionRuleNestingMixin"

    nesting_rule: (
        Union[ExpansionRuleNestingNested, ExpansionRuleNestingNonNested] | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class VbdExpansionMixin(DakotaBaseModel):
    "Generated model for VbdExpansionMixin"

    variance_based_decomp: VbdExpansionVarianceBasedDecomp | None = DakotaField(
        default=None,
        description="Activates global sensitivity analysis based on decomposition of response variance into main, interaction, and total effects",
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


class ExpansionOptionsResponseLevels(DakotaBaseModel):
    "Values at which to estimate desired statistics for each response"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckSumEqualsLength(
            context="expansionoptionsresponselevels",
            num_list_field="num_response_levels",
            levels_list_field="values",
        ),
    ]

    values: list[DakotaFloat] = DakotaField(
        description="Values at which to estimate desired statistics for each response"
    )
    num_response_levels: list[int] | None = DakotaField(
        default=None,
        description="Number of values at which to estimate desired statistics for each response",
    )
    compute: ExpansionOptionsResponseLevelsCompute | None = DakotaField(
        default=None,
        description="Selection of statistics to compute at each response level",
    )


class ImportApproxPointsFile(DakotaBaseModel):
    "Filename for points at which to evaluate the PCE/SC surrogate"

    filename: str = DakotaField(
        description="Filename for points at which to evaluate the PCE/SC surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_approx_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        ImportApproxPointsFileCustomAnnotated,
        ImportApproxPointsFileAnnotated,
        ImportApproxPointsFileFreeform,
    ] = DakotaField(
        default_factory=ImportApproxPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "ImportApproxPointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_approx_active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class ExpansionOptionsExportApproxPointsFile(DakotaBaseModel):
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
        ExpansionOptionsExportApproxPointsFileCustomAnnotated,
        ExpansionOptionsExportApproxPointsFileAnnotated,
        ExpansionOptionsExportApproxPointsFileFreeform,
    ] = DakotaField(
        default_factory=ExpansionOptionsExportApproxPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "ExpansionOptionsExportApproxPointsFileAnnotated",
        },
    )


class ExpansionQuadratureMixin(DakotaBaseModel):
    "Generated model for ExpansionQuadratureMixin"

    quadrature_order: ExpansionQuadratureQuadratureOrder = DakotaField(
        default=...,
        description="Order for tensor-products of Gaussian quadrature rules",
        dakota={"argument": "order"},
    )


class ExpansionQuadratureSequenceMixin(DakotaBaseModel):
    "Generated model for ExpansionQuadratureSequenceMixin"

    quadrature_order_sequence: ExpansionQuadratureSequenceQuadratureOrderSequence = (
        DakotaField(
            default=...,
            description="Sequence of quadrature orders used in a multi-stage expansion",
            dakota={"argument": "sequence"},
        )
    )


class ExpansionOptionsMixin(DakotaBaseModel):
    "Generated model for ExpansionOptionsMixin"

    samples_on_emulator: int = DakotaField(
        default=0,
        description="Number of samples at which to evaluate an emulator (surrogate)",
        dakota={
            "aliases": ["samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.samples_on_emulator",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ],
        },
    )
    sample_type: (
        Union[ExpansionOptionsSampleTypeLhs, ExpansionOptionsSampleTypeRandom] | None
    ) = DakotaField(
        default=None,
        description="Selection of sampling strategy",
        dakota={"union_pattern": 2},
    )
    rng: Union[ExpansionOptionsRngMt19937, ExpansionOptionsRngRnum2] = DakotaField(
        default_factory=ExpansionOptionsRngMt19937,
        description="Selection of a random number generator",
        dakota={"union_pattern": 1, "model_default": "ExpansionOptionsRngMt19937"},
    )
    probability_refinement: ExpansionOptionsProbabilityRefinement | None = DakotaField(
        default=None,
        description="Allow refinement of probability and generalized reliability results using importance sampling",
        dakota={"aliases": ["sample_refinement"]},
    )
    final_moments: Union[
        ExpansionOptionsFinalMomentsNoneKeyword,
        ExpansionOptionsFinalMomentsStandard,
        ExpansionOptionsFinalMomentsCentral,
    ] = DakotaField(
        default_factory=ExpansionOptionsFinalMomentsStandard,
        description="Output moments of the specified type and include them within the set of final statistics.",
        dakota={
            "union_pattern": 1,
            "model_default": "ExpansionOptionsFinalMomentsStandard",
        },
    )
    response_levels: ExpansionOptionsResponseLevels | None = DakotaField(
        default=None,
        description="Values at which to estimate desired statistics for each response",
        dakota={
            "argument": "values",
            "materialization": [
                {
                    "ir_key": "method.nond.response_levels",
                    "storage_type": "RESPONSE_LEVELS_ARRAY",
                    "ir_value_type": "RealVectorArray",
                }
            ],
        },
    )
    probability_levels: ExpansionOptionsProbabilityLevels | None = DakotaField(
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
    reliability_levels: ExpansionOptionsReliabilityLevels | None = DakotaField(
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
    gen_reliability_levels: ExpansionOptionsGenReliabilityLevels | None = DakotaField(
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
    distribution: Union[
        ExpansionOptionsDistributionCumulative,
        ExpansionOptionsDistributionComplementary,
    ] = DakotaField(
        default_factory=ExpansionOptionsDistributionCumulative,
        description="Selection of cumulative or complementary cumulative functions",
        dakota={
            "union_pattern": 1,
            "model_default": "ExpansionOptionsDistributionCumulative",
        },
    )
    variance_based_decomp: ExpansionOptionsVarianceBasedDecomp | None = DakotaField(
        default=None,
        description="Activates global sensitivity analysis based on decomposition of response variance into main, interaction, and total effects",
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
    import_approx_points_file: ImportApproxPointsFile | None = DakotaField(
        default=None,
        description="Filename for points at which to evaluate the PCE/SC surrogate",
        dakota={"argument": "filename"},
    )
    export_approx_points_file: ExpansionOptionsExportApproxPointsFile | None = (
        DakotaField(
            default=None,
            description="Output file for surrogate model value evaluations",
            dakota={"argument": "filename", "aliases": ["export_points_file"]},
        )
    )
    covariance_type: Union[ExpansionOptionsDiagCov, ExpansionOptionsFullCov] | None = (
        DakotaField(
            default=None,
            description="Covariance Type",
            dakota={"anchor": True, "union_pattern": 2},
        )
    )
