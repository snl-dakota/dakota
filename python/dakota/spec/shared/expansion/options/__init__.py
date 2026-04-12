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
    """Generated model for ExpansionDimPrefMixin"""

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
    """Generated model for ExpansionOptionsSampleTypeLhs"""

    lhs: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sample_type",
                    "stored_value": "SUBMETHOD_LHS",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExpansionOptionsSampleTypeRandom(DakotaBaseModel):
    """Generated model for ExpansionOptionsSampleTypeRandom"""

    random: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sample_type",
                    "stored_value": "SUBMETHOD_RANDOM",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExpansionOptionsRngMt19937(DakotaBaseModel):
    """Generated model for ExpansionOptionsRngMt19937"""

    mt19937: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.random_number_generator",
                    "stored_value": "mt19937",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ExpansionOptionsRngRnum2(DakotaBaseModel):
    """Generated model for ExpansionOptionsRngRnum2"""

    rnum2: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.random_number_generator",
                    "stored_value": "rnum2",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ExpansionOptionsProbabilityRefinementImportance(DakotaBaseModel):
    """Generated model for ExpansionOptionsProbabilityRefinementImportance"""

    importance: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.integration_refinement",
                    "stored_value": "IS",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExpansionOptionsProbabilityRefinementAdaptImport(DakotaBaseModel):
    """Generated model for ExpansionOptionsProbabilityRefinementAdaptImport"""

    adapt_import: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.integration_refinement",
                    "stored_value": "AIS",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExpansionOptionsProbabilityRefinementMmAdaptImport(DakotaBaseModel):
    """Generated model for ExpansionOptionsProbabilityRefinementMmAdaptImport"""

    mm_adapt_import: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.integration_refinement",
                    "stored_value": "MMAIS",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExpansionOptionsFinalMomentsNoneKeyword(DakotaBaseModel):
    """Generated model for ExpansionOptionsFinalMomentsNoneKeyword"""

    none: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.final_moments",
                    "stored_value": "NO_MOMENTS",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExpansionOptionsFinalMomentsStandard(DakotaBaseModel):
    """Generated model for ExpansionOptionsFinalMomentsStandard"""

    standard: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.final_moments",
                    "stored_value": "STANDARD_MOMENTS",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExpansionOptionsFinalMomentsCentral(DakotaBaseModel):
    """Generated model for ExpansionOptionsFinalMomentsCentral"""

    central: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.final_moments",
                    "stored_value": "CENTRAL_MOMENTS",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExpansionOptionsResponseLevelsComputeProbabilities(DakotaBaseModel):
    """Generated model for ExpansionOptionsResponseLevelsComputeProbabilities"""

    probabilities: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_level_target",
                    "stored_value": "PROBABILITIES",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExpansionOptionsResponseLevelsComputeReliabilities(DakotaBaseModel):
    """Generated model for ExpansionOptionsResponseLevelsComputeReliabilities"""

    reliabilities: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_level_target",
                    "stored_value": "RELIABILITIES",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExpansionOptionsResponseLevelsComputeGenReliabilities(DakotaBaseModel):
    """Generated model for ExpansionOptionsResponseLevelsComputeGenReliabilities"""

    gen_reliabilities: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_level_target",
                    "stored_value": "GEN_RELIABILITIES",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExpansionOptionsResponseLevelsComputeSystemSeries(DakotaBaseModel):
    """Generated model for ExpansionOptionsResponseLevelsComputeSystemSeries"""

    series: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_level_target_reduce",
                    "stored_value": "SYSTEM_SERIES",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExpansionOptionsResponseLevelsComputeSystemParallel(DakotaBaseModel):
    """Generated model for ExpansionOptionsResponseLevelsComputeSystemParallel"""

    parallel: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_level_target_reduce",
                    "stored_value": "SYSTEM_PARALLEL",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExpansionOptionsProbabilityLevels(DakotaBaseModel):
    """Generated model for ExpansionOptionsProbabilityLevels"""

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
    """Generated model for ExpansionOptionsReliabilityLevels"""

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
    """Generated model for ExpansionOptionsGenReliabilityLevels"""

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
    """Generated model for ExpansionOptionsDistributionCumulative"""

    cumulative: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.distribution",
                    "stored_value": "CUMULATIVE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExpansionOptionsDistributionComplementary(DakotaBaseModel):
    """Generated model for ExpansionOptionsDistributionComplementary"""

    complementary: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.distribution",
                    "stored_value": "COMPLEMENTARY",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExpansionOptionsVarianceBasedDecomp(DakotaBaseModel):
    """Generated model for ExpansionOptionsVarianceBasedDecomp"""

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
    """Generated model for ImportApproxPointsFileCustomAnnotatedConfig"""

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_approx_format",
                    "stored_value": "TABULAR_HEADER",
                    "storage_type": "AUGMENT_ENUM",
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
                    "stored_value": "TABULAR_EVAL_ID",
                    "storage_type": "AUGMENT_ENUM",
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
                    "stored_value": "TABULAR_IFACE_ID",
                    "storage_type": "AUGMENT_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ImportApproxPointsFileAnnotated(DakotaBaseModel):
    """Generated model for ImportApproxPointsFileAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_approx_format",
                    "stored_value": "TABULAR_ANNOTATED",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ImportApproxPointsFileFreeform(DakotaBaseModel):
    """Generated model for ImportApproxPointsFileFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_approx_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExpansionOptionsExportApproxPointsFileCustomAnnotatedConfig(DakotaBaseModel):
    """Generated model for ExpansionOptionsExportApproxPointsFileCustomAnnotatedConfig"""

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "stored_value": "TABULAR_HEADER",
                    "storage_type": "AUGMENT_ENUM",
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
                    "stored_value": "TABULAR_EVAL_ID",
                    "storage_type": "AUGMENT_ENUM",
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
                    "stored_value": "TABULAR_IFACE_ID",
                    "storage_type": "AUGMENT_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExpansionOptionsExportApproxPointsFileAnnotated(DakotaBaseModel):
    """Generated model for ExpansionOptionsExportApproxPointsFileAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "stored_value": "TABULAR_ANNOTATED",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExpansionOptionsExportApproxPointsFileFreeform(DakotaBaseModel):
    """Generated model for ExpansionOptionsExportApproxPointsFileFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExpansionOptionsDiagCov(DakotaBaseModel):
    """Generated model for ExpansionOptionsDiagCov"""

    diagonal_covariance: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.covariance_control",
                    "stored_value": "DIAGONAL_COVARIANCE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExpansionOptionsFullCov(DakotaBaseModel):
    """Generated model for ExpansionOptionsFullCov"""

    full_covariance: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.covariance_control",
                    "stored_value": "FULL_COVARIANCE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExpansionQuadratureOrderNested(DakotaBaseModel):
    """Generated model for ExpansionQuadratureOrderNested"""

    nested: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.nesting_override",
                    "stored_value": "NESTED",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExpansionQuadratureOrderNonNested(DakotaBaseModel):
    """Generated model for ExpansionQuadratureOrderNonNested"""

    non_nested: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.nesting_override",
                    "stored_value": "NON_NESTED",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExpansionQuadratureSequenceOrderSequenceNested(DakotaBaseModel):
    """Generated model for ExpansionQuadratureSequenceOrderSequenceNested"""

    nested: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.nesting_override",
                    "stored_value": "NESTED",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExpansionQuadratureSequenceOrderSequenceNonNested(DakotaBaseModel):
    """Generated model for ExpansionQuadratureSequenceOrderSequenceNonNested"""

    non_nested: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.nesting_override",
                    "stored_value": "NON_NESTED",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExpansionRuleGrowthRestricted(DakotaBaseModel):
    """Generated model for ExpansionRuleGrowthRestricted"""

    restricted: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.growth_override",
                    "stored_value": "RESTRICTED",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExpansionRuleGrowthUnrestricted(DakotaBaseModel):
    """Generated model for ExpansionRuleGrowthUnrestricted"""

    unrestricted: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.growth_override",
                    "stored_value": "UNRESTRICTED",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExpansionRuleNestingNested(DakotaBaseModel):
    """Generated model for ExpansionRuleNestingNested"""

    nested: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.nesting_override",
                    "stored_value": "NESTED",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExpansionRuleNestingNonNested(DakotaBaseModel):
    """Generated model for ExpansionRuleNestingNonNested"""

    non_nested: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.nesting_override",
                    "stored_value": "NON_NESTED",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class VbdExpansionVarianceBasedDecomp(DakotaBaseModel):
    """Generated model for VbdExpansionVarianceBasedDecomp"""

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
    """Generated model for ExpansionOptionsProbabilityRefinement"""

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
    """Generated model for ExpansionOptionsResponseLevelsCompute"""

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
    """Generated model for ImportApproxPointsFileCustomAnnotated"""

    custom_annotated: ImportApproxPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=ImportApproxPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "ImportApproxPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_approx_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class ExpansionOptionsExportApproxPointsFileCustomAnnotated(DakotaBaseModel):
    """Generated model for ExpansionOptionsExportApproxPointsFileCustomAnnotated"""

    custom_annotated: ExpansionOptionsExportApproxPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=ExpansionOptionsExportApproxPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "ExpansionOptionsExportApproxPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.export_approx_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class ExpansionQuadratureQuadratureOrder(DakotaBaseModel):
    """Generated model for ExpansionQuadratureQuadratureOrder"""

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
    """Generated model for ExpansionQuadratureSequenceQuadratureOrderSequence"""

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
    """Generated model for ExpansionRuleGrowthMixin"""

    growth_rule: (
        Union[ExpansionRuleGrowthRestricted, ExpansionRuleGrowthUnrestricted] | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Growth",
        dakota={"anchor": True, "union_pattern": 2},
    )


class ExpansionRuleNestingMixin(DakotaBaseModel):
    """Generated model for ExpansionRuleNestingMixin"""

    nesting_rule: (
        Union[ExpansionRuleNestingNested, ExpansionRuleNestingNonNested] | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class VbdExpansionMixin(DakotaBaseModel):
    """Generated model for VbdExpansionMixin"""

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
    """Generated model for ExpansionOptionsResponseLevels"""

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
    """Generated model for ImportApproxPointsFile"""

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
    """Generated model for ExpansionOptionsExportApproxPointsFile"""

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
    """Generated model for ExpansionQuadratureMixin"""

    quadrature_order: ExpansionQuadratureQuadratureOrder = DakotaField(
        default=...,
        description="Order for tensor-products of Gaussian quadrature rules",
        dakota={"argument": "order"},
    )


class ExpansionQuadratureSequenceMixin(DakotaBaseModel):
    """Generated model for ExpansionQuadratureSequenceMixin"""

    quadrature_order_sequence: ExpansionQuadratureSequenceQuadratureOrderSequence = (
        DakotaField(
            default=...,
            description="Sequence of quadrature orders used in a multi-stage expansion",
            dakota={"argument": "sequence"},
        )
    )


class ExpansionOptionsMixin(DakotaBaseModel):
    """Generated model for ExpansionOptionsMixin"""

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
