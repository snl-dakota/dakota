"""Generated Pydantic models for method.multilevel_polynomial_chaos"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField, DakotaFloat
from .base import MethodSelection
from typing import Literal, Union
from ..base import SZ_MAX, USHRT_MAX
from typing import ClassVar, List
from ..validation import ValidationRule
from ..validation.rules import CheckNonnegativeList

# Cross-module model imports
from dakota.spec.shared.core import MethodExpConvergenceToleranceMixin
from dakota.spec.shared.expansion.options import ExpansionOptionsMixin
from dakota.spec.shared.expansion.pce import MlpceAllocControlMixin, PceOptionsMixin
from dakota.spec.shared.misc import (
    DiscrepEmulationMixin,
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
    MethodSeedSequenceMixin,
    MethodThreeOptionalKeywordsMixin,
)


class MlPceExpansionOrderSequenceBasisTypeTensorProduct(DakotaBaseModel):
    "Use a tensor-product index set to construct a polynomial chaos expansion."

    tensor_product: Literal[True] = DakotaField(
        default=True,
        description="Use a tensor-product index set to construct a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TENSOR_PRODUCT_BASIS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MlPceExpansionOrderSequenceBasisTypeTotalOrder(DakotaBaseModel):
    "Use a total-order index set to construct a polynomial chaos expansion."

    total_order: Literal[True] = DakotaField(
        default=True,
        description="Use a total-order index set to construct a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TOTAL_ORDER_BASIS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MlPceExpansionOrderSequenceBasisTypeAdaptedConfig(DakotaBaseModel):
    "Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion."

    advancements: int = DakotaField(
        default=3,
        description="The maximum number of steps used to expand a basis step.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.adapted_basis.advancements",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    soft_convergence_limit: int = DakotaField(
        default=0,
        description="The maximum number of times no improvement in cross validation error is allowed before the algorithm is terminated.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.soft_convergence_limit",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MlPceExpansionOrderSequenceCollocRatioLeastSquaresSvd(DakotaBaseModel):
    "Calculate the coefficients of a polynomial chaos expansion using the singular value decomposition."

    svd: Literal[True] = DakotaField(
        default=True,
        description="Calculate the coefficients of a polynomial chaos expansion using the singular value decomposition.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SVD_LS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MlPceExpansionOrderSequenceCollocRatioLeastSquaresEqCon(DakotaBaseModel):
    "Calculate the coefficients of a polynomial chaos expansion using equality constrained least squares."

    equality_constrained: Literal[True] = DakotaField(
        default=True,
        description="Calculate the coefficients of a polynomial chaos expansion using equality constrained least squares.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "EQ_CON_LS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MlPceExpansionOrderSequenceCollocRatioOMPConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class MlPceExpansionOrderSequenceCollocRatioBP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit L1 -minimization problem using linear programming."

    basis_pursuit: Literal[True] = DakotaField(
        default=True,
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit L1 -minimization problem using linear programming.",
        dakota={
            "aliases": ["bp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BASIS_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class MlPceExpansionOrderSequenceCollocRatioBPDNConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization."

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class MlPceExpansionOrderSequenceCollocRatioLarsConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method."

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class MlPceExpansionOrderSequenceCollocRatioLassoConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    l2_penalty: DakotaFloat | None = DakotaField(
        default=None,
        description="The :math:`l_2`  pentalty used when performing compressed sensing with elastic net.",
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


class MlPceExpansionOrderSequenceCollocRatioCV(DakotaBaseModel):
    "Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion."

    noise_only: Literal[True] | None = DakotaField(
        default=None,
        description="Restrict the cross validation process to estimating only the best noise tolerance.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.noise_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
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


class MlPceExpansionOrderSequenceExpansionSamplesSequenceConfig(DakotaBaseModel):
    "Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="mlpceexpansionordersequenceexpansionsamplessequenceconfig",
            list_field="sequence",
        ),
    ]

    sequence: list[int] = DakotaField(
        description="Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_samples_sequence",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "SizetArray",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class MlPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig(
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


class MlPceExpansionOrderSequenceImportBuildPointsFileAnnotated(DakotaBaseModel):
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


class MlPceExpansionOrderSequenceImportBuildPointsFileFreeform(DakotaBaseModel):
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


class MlPceOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig(DakotaBaseModel):
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


class MlPceOrthogLeastInterpImportBuildPointsFileAnnotated(DakotaBaseModel):
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


class MlPceOrthogLeastInterpImportBuildPointsFileFreeform(DakotaBaseModel):
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


class MlPceExpansionOrderSequenceBasisTypeAdapted(DakotaBaseModel):
    "Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion."

    adapted: MlPceExpansionOrderSequenceBasisTypeAdaptedConfig = DakotaField(
        description="Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ADAPTED_BASIS_EXPANDING_FRONT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MlPceExpansionOrderSequenceCollocRatioLeastSquares(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using least squares"

    least_squares: Union[
        MlPceExpansionOrderSequenceCollocRatioLeastSquaresSvd,
        MlPceExpansionOrderSequenceCollocRatioLeastSquaresEqCon,
        dict,
    ] = DakotaField(
        description="Compute the coefficients of a polynomial expansion using least squares",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DEFAULT_LEAST_SQ_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MlPceExpansionOrderSequenceCollocRatioOMP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    orthogonal_matching_pursuit: MlPceExpansionOrderSequenceCollocRatioOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)",
        dakota={
            "aliases": ["omp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ORTHOG_MATCH_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class MlPceExpansionOrderSequenceCollocRatioBPDN(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization."

    basis_pursuit_denoising: MlPceExpansionOrderSequenceCollocRatioBPDNConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization.",
        dakota={
            "aliases": ["bpdn"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BASIS_PURSUIT_DENOISING",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class MlPceExpansionOrderSequenceCollocRatioLars(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method."

    least_angle_regression: MlPceExpansionOrderSequenceCollocRatioLarsConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method.",
        dakota={
            "aliases": ["lars"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LEAST_ANGLE_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class MlPceExpansionOrderSequenceCollocRatioLasso(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    least_absolute_shrinkage: MlPceExpansionOrderSequenceCollocRatioLassoConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the LASSO problem.",
        dakota={
            "aliases": ["lasso"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LASSO_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class MlPceExpansionOrderSequenceExpansionSamplesSequence(DakotaBaseModel):
    "Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`"

    expansion_samples_sequence: MlPceExpansionOrderSequenceExpansionSamplesSequenceConfig = DakotaField(
        default=...,
        description="Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`",
        dakota={"argument": "sequence"},
    )


class MlPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: MlPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=MlPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "MlPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig",
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


class MlPceOrthogLeastInterpImportBuildPointsFileCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: MlPceOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=MlPceOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "MlPceOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig",
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


class MlPceExpansionOrderSequenceCollocRatioConfig(DakotaBaseModel):
    "Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion."

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="mlpceexpansionordersequencecollocratioconfig",
            list_field="collocation_points_sequence",
        ),
    ]

    sequence: DakotaFloat = DakotaField(
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
    regression_method: (
        Union[
            MlPceExpansionOrderSequenceCollocRatioLeastSquares,
            MlPceExpansionOrderSequenceCollocRatioOMP,
            MlPceExpansionOrderSequenceCollocRatioBP,
            MlPceExpansionOrderSequenceCollocRatioBPDN,
            MlPceExpansionOrderSequenceCollocRatioLars,
            MlPceExpansionOrderSequenceCollocRatioLasso,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Regression Algorithm",
        dakota={"anchor": True, "union_pattern": 2},
    )
    cross_validation: MlPceExpansionOrderSequenceCollocRatioCV | None = DakotaField(
        default=None,
        description="Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    ratio_order: DakotaFloat = DakotaField(
        default=1.0,
        gt=0,
        description="Specify a non-linear the relationship between the expansion order of a polynomial chaos expansion and the number of samples that will be used to compute the PCE coefficients.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio_terms_order",
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
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
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


class MlPceExpansionOrderSequenceImportBuildPointsFile(DakotaBaseModel):
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
        MlPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotated,
        MlPceExpansionOrderSequenceImportBuildPointsFileAnnotated,
        MlPceExpansionOrderSequenceImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=MlPceExpansionOrderSequenceImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "MlPceExpansionOrderSequenceImportBuildPointsFileAnnotated",
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


class MlPceOrthogLeastInterpImportBuildPointsFile(DakotaBaseModel):
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
        MlPceOrthogLeastInterpImportBuildPointsFileCustomAnnotated,
        MlPceOrthogLeastInterpImportBuildPointsFileAnnotated,
        MlPceOrthogLeastInterpImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=MlPceOrthogLeastInterpImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "MlPceOrthogLeastInterpImportBuildPointsFileAnnotated",
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


class MlPceExpansionOrderSequenceCollocRatio(DakotaBaseModel):
    "Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion."

    collocation_ratio: MlPceExpansionOrderSequenceCollocRatioConfig = DakotaField(
        default=...,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={"argument": "sequence"},
    )


class MlPceOrthogLeastInterpConfig(DakotaBaseModel):
    "Build a polynomial chaos expansion from simulation samples using orthogonal least interpolation."

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="mlpceorthogleastinterpconfig",
            list_field="collocation_points_sequence",
        ),
        CheckNonnegativeList(
            context="mlpceorthogleastinterpconfig", list_field="tensor_grid"
        ),
    ]

    collocation_points_sequence: list[int] = DakotaField(
        description="Sequence of collocation point counts used in a multi-stage expansion",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_points_sequence",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "SizetArray",
                }
            ]
        },
    )
    tensor_grid: list[int] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.tensor_grid_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "UShortArray",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    import_build_points_file: MlPceOrthogLeastInterpImportBuildPointsFile | None = (
        DakotaField(
            default=None,
            description="File containing points you wish to use to build a surrogate",
            dakota={"argument": "filename", "aliases": ["import_points_file"]},
        )
    )


class MlPceExpansionOrderSequenceConfig(DakotaBaseModel):
    "Sequence of expansion orders used in a multi-stage expansion"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="mlpceexpansionordersequenceconfig", list_field="sequence"
        ),
    ]

    sequence: list[int] = DakotaField(
        description="Sequence of expansion orders used in a multi-stage expansion",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_order_sequence",
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
    basis_type: (
        Union[
            MlPceExpansionOrderSequenceBasisTypeTensorProduct,
            MlPceExpansionOrderSequenceBasisTypeTotalOrder,
            MlPceExpansionOrderSequenceBasisTypeAdapted,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Specify the type of basis truncation to be used for a Polynomial Chaos Expansion.",
        dakota={"union_pattern": 2},
    )
    point_sequence_selection: Union[
        MlPceExpansionOrderSequenceCollocRatio,
        MlPceExpansionOrderSequenceExpansionSamplesSequence,
    ] = DakotaField(dakota={"anchor": True, "union_pattern": 4})
    import_build_points_file: (
        MlPceExpansionOrderSequenceImportBuildPointsFile | None
    ) = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )


class MlPceOrthogLeastInterp(DakotaBaseModel):
    "Build a polynomial chaos expansion from simulation samples using orthogonal least interpolation."

    orthogonal_least_interpolation: MlPceOrthogLeastInterpConfig = DakotaField(
        description="Build a polynomial chaos expansion from simulation samples using orthogonal least interpolation.",
        dakota={
            "aliases": ["least_interpolation", "oli"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ORTHOG_LEAST_INTERPOLATION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class MlPceExpansionOrderSequence(DakotaBaseModel):
    "Sequence of expansion orders used in a multi-stage expansion"

    expansion_order_sequence: MlPceExpansionOrderSequenceConfig = DakotaField(
        default=...,
        description="Sequence of expansion orders used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class MlPceConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodMaxIterationsContext1Mixin,
    MlpceAllocControlMixin,
    MethodExpConvergenceToleranceMixin,
    DiscrepEmulationMixin,
    PceOptionsMixin,
    ExpansionOptionsMixin,
    MethodSeedSequenceMixin,
    MethodOptionalModelPointerMixin,
):
    "Multilevel uncertainty quantification using polynomial chaos expansions"

    coefficient_approach: Union[MlPceExpansionOrderSequence, MlPceOrthogLeastInterp] = (
        DakotaField(
            description="Coefficient Computation Approach",
            dakota={"anchor": True, "union_pattern": 4},
        )
    )


class MlPceSelection(MethodSelection):
    "Generated model for MlPceSelection"

    multilevel_polynomial_chaos: MlPceConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MULTILEVEL_POLYNOMIAL_CHAOS",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
