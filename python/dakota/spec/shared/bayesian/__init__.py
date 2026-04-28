"""Generated Pydantic models for shared.bayesian"""

from __future__ import annotations

from ...base import DakotaBaseModel, DakotaField, DakotaFloat
from typing import Literal, Union
from ...base import INT_MAX, SZ_MAX, USHRT_MAX
from typing import ClassVar, List
from ...validation import ValidationRule
from ...validation.rules import CheckNonnegativeList, CompareLength, CompareLengthOne


class EmulatorGPSurfpack(DakotaBaseModel):
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


class EmulatorGPDakota(DakotaBaseModel):
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


class GPImportBuildPointsFileCustomAnnotatedConfig(DakotaBaseModel):
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


class GPImportBuildPointsFileAnnotated(DakotaBaseModel):
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


class GPImportBuildPointsFileFreeform(DakotaBaseModel):
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


class EmulatorPcePRefinementUniform(DakotaBaseModel):
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


class EmulatorPcePRefinementDimAdaptiveSobol(DakotaBaseModel):
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


class EmulatorPcePRefinementDimAdaptiveDecay(DakotaBaseModel):
    "Estimate spectral coefficient decay rates to guide dimension-adaptive refinement."

    decay: Literal[True] = DakotaField(
        default=True,
        description="Estimate spectral coefficient decay rates to guide dimension-adaptive refinement.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DIMENSION_ADAPTIVE_CONTROL_DECAY",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class EmulatorPcePRefinementDimAdaptiveGeneralized(DakotaBaseModel):
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


class EmulatorPceQuadratureOrderNested(DakotaBaseModel):
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


class EmulatorPceQuadratureOrderNonNested(DakotaBaseModel):
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


class EmulatorPceSGLevelRestricted(DakotaBaseModel):
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


class EmulatorPceSGLevelUnrestricted(DakotaBaseModel):
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


class EmulatorPceCubatureIntegrand(DakotaBaseModel):
    "Cubature using Stroud rules and their extensions"

    cubature_integrand: int = DakotaField(
        description="Cubature using Stroud rules and their extensions",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cubature_integrand",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class EmulatorPceExpansionOrderBasisTypeTensorProduct(DakotaBaseModel):
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


class EmulatorPceExpansionOrderBasisTypeTotalOrder(DakotaBaseModel):
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


class EmulatorPceExpansionOrderBasisTypeAdaptedConfig(DakotaBaseModel):
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


class EmulatorPceExpansionOrderCollocPointsLeastSquaresSvd(DakotaBaseModel):
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


class EmulatorPceExpansionOrderCollocPointsLeastSquaresEqCon(DakotaBaseModel):
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


class EmulatorPceExpansionOrderCollocPointsOMPConfig(DakotaBaseModel):
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


class EmulatorPceExpansionOrderCollocPointsBP(DakotaBaseModel):
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


class EmulatorPceExpansionOrderCollocPointsLassoConfig(DakotaBaseModel):
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


class EmulatorPceExpansionOrderCollocPointsCV(DakotaBaseModel):
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


class EmulatorPceExpansionOrderExpansionSamplesConfig(DakotaBaseModel):
    "Number of simulation samples used to estimate the expected value of a set of PCE coefficients"

    value: int = DakotaField(
        default=SZ_MAX,
        description="Number of simulation samples used to estimate the expected value of a set of PCE coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
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


class EmulatorPceImportExpansionFile(DakotaBaseModel):
    "Build a Polynomial Chaos Expansion (PCE) by importing expansion coefficients and a corresponding multi-index from a file"

    import_expansion_file: str = DakotaField(
        description="Build a Polynomial Chaos Expansion (PCE) by importing expansion coefficients and a corresponding multi-index from a file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.import_expansion_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class EmulatorPceAskey(DakotaBaseModel):
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


class EmulatorPceWiener(DakotaBaseModel):
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


class EmulatorPceDiagCov(DakotaBaseModel):
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


class EmulatorPceFullCov(DakotaBaseModel):
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


class EmulatorMlPceAllocationControlEstimatorVarianceConfig(DakotaBaseModel):
    "Variance of mean estimator within multilevel polynomial chaos"

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


class EmulatorMlPceAllocationControlRipSampling(DakotaBaseModel):
    "Sample allocation based on restricted isometry property (RIP) within multilevel polynomial chaos"

    rip_sampling: Literal[True] = DakotaField(
        default=True,
        description="Sample allocation based on restricted isometry property (RIP) within multilevel polynomial chaos",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.multilevel_allocation_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RIP_SAMPLING",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class EmulatorMlPceDiscrepancyEmulationDistinct(DakotaBaseModel):
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


class EmulatorMlPceDiscrepancyEmulationRecursive(DakotaBaseModel):
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


class EmulatorMlPceExpansionOrderSequenceExpansionSamplesSequenceConfig(
    DakotaBaseModel
):
    "Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesemulatormlpceexpansionordersequenceexpansionsamplessequenceconfig",
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


class EmulatorMfPceAllocationControlGreedy(DakotaBaseModel):
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


class EmulatorScHRefinementLocalAdaptive(DakotaBaseModel):
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


class EmulatorScSGLevelNodal(DakotaBaseModel):
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


class EmulatorScSGLevelHierarchical(DakotaBaseModel):
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


class EmulatorScPiecewise(DakotaBaseModel):
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


class MfScPRefinementUniform(DakotaBaseModel):
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


class MfScPRefinementDimAdaptiveSobol(DakotaBaseModel):
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


class MfScPRefinementDimAdaptiveGeneralized(DakotaBaseModel):
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


class MfScHRefinementLocalAdaptive(DakotaBaseModel):
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


class MfScAllocationControlGreedy(DakotaBaseModel):
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


class MfScDiscrepancyEmulationDistinct(DakotaBaseModel):
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


class MfScDiscrepancyEmulationRecursive(DakotaBaseModel):
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


class MfScQuadratureOrderSequenceNested(DakotaBaseModel):
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


class MfScQuadratureOrderSequenceNonNested(DakotaBaseModel):
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


class MfScSGLevelSequenceNodal(DakotaBaseModel):
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


class MfScSGLevelSequenceHierarchical(DakotaBaseModel):
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


class MfScSGLevelSequenceRestricted(DakotaBaseModel):
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


class MfScSGLevelSequenceUnrestricted(DakotaBaseModel):
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


class MfScPiecewise(DakotaBaseModel):
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


class MfScAskey(DakotaBaseModel):
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


class MfScWiener(DakotaBaseModel):
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


class MfScDiagCov(DakotaBaseModel):
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


class MfScFullCov(DakotaBaseModel):
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


class PriorConfig(DakotaBaseModel):
    "Uses the covariance of the prior distributions to define the MCMC proposal covariance."

    multiplier: DakotaFloat = DakotaField(
        default=1.0,
        gt=0.0,
        description="Multiplier to scale prior variance",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.prior_prop_cov_mult",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class DerivativesConfig(DakotaBaseModel):
    "Use derivatives to inform the MCMC proposal covariance."

    update_period: int = DakotaField(
        default=INT_MAX,
        description="Period at which to update derivative-based proposal covariance",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.prop_cov_update_period",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )


class ValuesDiag(DakotaBaseModel):
    "Specifies the diagonal matrix format when specifying a user-specified proposal covariance."

    diagonal: Literal[True] = DakotaField(
        default=True,
        description="Specifies the diagonal matrix format when specifying a user-specified proposal covariance.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.proposal_covariance_input_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "diagonal",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ValuesMatrix(DakotaBaseModel):
    "Specifies the full matrix format when specifying a user-specified proposal covariance."

    matrix: Literal[True] = DakotaField(
        default=True,
        description="Specifies the full matrix format when specifying a user-specified proposal covariance.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.proposal_covariance_input_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "matrix",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ScPRefinementUniform(DakotaBaseModel):
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


class ScPRefinementDimAdaptiveSobol(DakotaBaseModel):
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


class ScPRefinementDimAdaptiveGeneralized(DakotaBaseModel):
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


class BayesScHRefinementLocalAdaptive(DakotaBaseModel):
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


class ScQuadratureOrderNested(DakotaBaseModel):
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


class ScQuadratureOrderNonNested(DakotaBaseModel):
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


class BayesScSGLevelNodal(DakotaBaseModel):
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


class BayesScSGLevelHierarchical(DakotaBaseModel):
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


class BayesScSGLevelRestricted(DakotaBaseModel):
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


class BayesScSGLevelUnrestricted(DakotaBaseModel):
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


class BayesScPiecewise(DakotaBaseModel):
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


class ScAskey(DakotaBaseModel):
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


class ScWiener(DakotaBaseModel):
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


class ScDiagCov(DakotaBaseModel):
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


class ScFullCov(DakotaBaseModel):
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


class BayesSeedRngMt19937(DakotaBaseModel):
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


class BayesSeedRngRnum2(DakotaBaseModel):
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


class GPImportBuildPointsFileCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: GPImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=GPImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "GPImportBuildPointsFileCustomAnnotatedConfig",
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


class EmulatorPceExpansionOrderImportBuildPointsFileCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: GPImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=GPImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "GPImportBuildPointsFileCustomAnnotatedConfig",
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


class EmulatorPcePRefinementDimAdaptive(DakotaBaseModel):
    'Perform anisotropic expansion refinement by preferentially adapting in dimensions that are detected to have higher \\"importance\\".'

    dimension_adaptive: Union[
        EmulatorPcePRefinementDimAdaptiveSobol,
        EmulatorPcePRefinementDimAdaptiveDecay,
        EmulatorPcePRefinementDimAdaptiveGeneralized,
    ] = DakotaField(
        description='Perform anisotropic expansion refinement by preferentially adapting in dimensions that are detected to have higher "importance".'
    )


class EmulatorMfPcePRefinementDimAdaptive(DakotaBaseModel):
    'Perform anisotropic expansion refinement by preferentially adapting in dimensions that are detected to have higher \\"importance\\".'

    dimension_adaptive: Union[
        EmulatorPcePRefinementDimAdaptiveSobol,
        EmulatorPcePRefinementDimAdaptiveDecay,
        EmulatorPcePRefinementDimAdaptiveGeneralized,
    ] = DakotaField(
        description='Perform anisotropic expansion refinement by preferentially adapting in dimensions that are detected to have higher "importance".'
    )


class EmulatorScPRefinementDimAdaptive(DakotaBaseModel):
    'Perform anisotropic expansion refinement by preferentially adapting in dimensions that are detected to have higher \\"importance\\".'

    dimension_adaptive: Union[
        EmulatorPcePRefinementDimAdaptiveSobol,
        EmulatorPcePRefinementDimAdaptiveGeneralized,
    ] = DakotaField(
        description='Perform anisotropic expansion refinement by preferentially adapting in dimensions that are detected to have higher "importance".'
    )


class EmulatorPceQuadratureOrderConfig(DakotaBaseModel):
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
        Union[EmulatorPceQuadratureOrderNested, EmulatorPceQuadratureOrderNonNested]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class EmulatorMfPceQuadratureOrderSequenceConfig(DakotaBaseModel):
    "Sequence of quadrature orders used in a multi-stage expansion"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesemulatormfpcequadratureordersequenceconfig",
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
        Union[EmulatorPceQuadratureOrderNested, EmulatorPceQuadratureOrderNonNested]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class EmulatorScQuadratureOrderConfig(DakotaBaseModel):
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
        Union[EmulatorPceQuadratureOrderNested, EmulatorPceQuadratureOrderNonNested]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class EmulatorPceSGLevelConfig(DakotaBaseModel):
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
    growth_rule: (
        Union[EmulatorPceSGLevelRestricted, EmulatorPceSGLevelUnrestricted] | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Growth",
        dakota={"anchor": True, "union_pattern": 2},
    )
    nesting_rule: (
        Union[EmulatorPceQuadratureOrderNested, EmulatorPceQuadratureOrderNonNested]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class EmulatorMfPceSGLevelSequenceConfig(DakotaBaseModel):
    "Sequence of sparse grid levels used in a multi-stage expansion"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesemulatormfpcesglevelsequenceconfig", list_field="sequence"
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
    growth_rule: (
        Union[EmulatorPceSGLevelRestricted, EmulatorPceSGLevelUnrestricted] | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Growth",
        dakota={"anchor": True, "union_pattern": 2},
    )
    nesting_rule: (
        Union[EmulatorPceQuadratureOrderNested, EmulatorPceQuadratureOrderNonNested]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class EmulatorPceExpansionOrderBasisTypeAdapted(DakotaBaseModel):
    "Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion."

    adapted: EmulatorPceExpansionOrderBasisTypeAdaptedConfig = DakotaField(
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


class EmulatorMlPceExpansionOrderSequenceBasisTypeAdapted(DakotaBaseModel):
    "Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion."

    adapted: EmulatorPceExpansionOrderBasisTypeAdaptedConfig = DakotaField(
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


class EmulatorPceExpansionOrderCollocPointsLeastSquares(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using least squares"

    least_squares: Union[
        EmulatorPceExpansionOrderCollocPointsLeastSquaresSvd,
        EmulatorPceExpansionOrderCollocPointsLeastSquaresEqCon,
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


class EmulatorPceExpansionOrderCollocRatioLeastSquares(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using least squares"

    least_squares: Union[
        EmulatorPceExpansionOrderCollocPointsLeastSquaresSvd,
        EmulatorPceExpansionOrderCollocPointsLeastSquaresEqCon,
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


class EmulatorPceExpansionOrderCollocPointsOMP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    orthogonal_matching_pursuit: EmulatorPceExpansionOrderCollocPointsOMPConfig = DakotaField(
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


class EmulatorPceExpansionOrderCollocPointsBPDN(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization."

    basis_pursuit_denoising: EmulatorPceExpansionOrderCollocPointsOMPConfig = DakotaField(
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


class EmulatorPceExpansionOrderCollocPointsLars(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method."

    least_angle_regression: EmulatorPceExpansionOrderCollocPointsOMPConfig = DakotaField(
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


class EmulatorPceExpansionOrderCollocRatioOMP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    orthogonal_matching_pursuit: EmulatorPceExpansionOrderCollocPointsOMPConfig = DakotaField(
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


class EmulatorPceExpansionOrderCollocPointsLasso(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    least_absolute_shrinkage: EmulatorPceExpansionOrderCollocPointsLassoConfig = DakotaField(
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


class EmulatorPceExpansionOrderCollocRatioLasso(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    least_absolute_shrinkage: EmulatorPceExpansionOrderCollocPointsLassoConfig = DakotaField(
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


class EmulatorPceExpansionOrderExpansionSamples(DakotaBaseModel):
    "Number of simulation samples used to estimate the expected value of a set of PCE coefficients"

    expansion_samples: EmulatorPceExpansionOrderExpansionSamplesConfig = DakotaField(
        default=...,
        description="Number of simulation samples used to estimate the expected value of a set of PCE coefficients",
        dakota={"argument": "value"},
    )


class EmulatorMlPceAllocationControlEstimatorVariance(DakotaBaseModel):
    "Variance of mean estimator within multilevel polynomial chaos"

    estimator_variance: EmulatorMlPceAllocationControlEstimatorVarianceConfig = (
        DakotaField(
            description="Variance of mean estimator within multilevel polynomial chaos",
            dakota={
                "materialization": [
                    {
                        "ir_key": "method.nond.multilevel_allocation_control",
                        "storage_type": "PRESENCE_ENUM",
                        "stored_value": "ESTIMATOR_VARIANCE",
                        "ir_value_type": "short",
                    }
                ]
            },
        )
    )


class EmulatorMlPceExpansionOrderSequenceExpansionSamplesSequence(DakotaBaseModel):
    "Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`"

    expansion_samples_sequence: EmulatorMlPceExpansionOrderSequenceExpansionSamplesSequenceConfig = DakotaField(
        default=...,
        description="Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`",
        dakota={"argument": "sequence"},
    )


class EmulatorMfPceExpansionOrderSequenceExpansionSamplesSequence(DakotaBaseModel):
    "Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`"

    expansion_samples_sequence: EmulatorMlPceExpansionOrderSequenceExpansionSamplesSequenceConfig = DakotaField(
        default=...,
        description="Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`",
        dakota={"argument": "sequence"},
    )


class EmulatorScSGLevelConfig(DakotaBaseModel):
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
    interpolant: Union[EmulatorScSGLevelNodal, EmulatorScSGLevelHierarchical] | None = (
        DakotaField(default=None, dakota={"anchor": True, "union_pattern": 2})
    )
    growth_rule: (
        Union[EmulatorPceSGLevelRestricted, EmulatorPceSGLevelUnrestricted] | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Growth",
        dakota={"anchor": True, "union_pattern": 2},
    )
    nesting_rule: (
        Union[EmulatorPceQuadratureOrderNested, EmulatorPceQuadratureOrderNonNested]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class EmulatorMfScSGLevelSequenceConfig(DakotaBaseModel):
    "Sequence of sparse grid levels used in a multi-stage expansion"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesemulatormfscsglevelsequenceconfig", list_field="sequence"
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
    interpolant: Union[EmulatorScSGLevelNodal, EmulatorScSGLevelHierarchical] | None = (
        DakotaField(default=None, dakota={"anchor": True, "union_pattern": 2})
    )
    growth_rule: (
        Union[EmulatorPceSGLevelRestricted, EmulatorPceSGLevelUnrestricted] | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Growth",
        dakota={"anchor": True, "union_pattern": 2},
    )
    nesting_rule: (
        Union[EmulatorPceQuadratureOrderNested, EmulatorPceQuadratureOrderNonNested]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class MfScPRefinementDimAdaptive(DakotaBaseModel):
    'Perform anisotropic expansion refinement by preferentially adapting in dimensions that are detected to have higher \\"importance\\".'

    dimension_adaptive: Union[
        MfScPRefinementDimAdaptiveSobol, MfScPRefinementDimAdaptiveGeneralized
    ] = DakotaField(
        description='Perform anisotropic expansion refinement by preferentially adapting in dimensions that are detected to have higher "importance".'
    )


class MfScHRefinementDimAdaptive(DakotaBaseModel):
    'Perform anisotropic expansion refinement by preferentially adapting in dimensions that are detected to have higher \\"importance\\".'

    dimension_adaptive: Union[
        MfScPRefinementDimAdaptiveSobol, MfScPRefinementDimAdaptiveGeneralized
    ] = DakotaField(
        description='Perform anisotropic expansion refinement by preferentially adapting in dimensions that are detected to have higher "importance".'
    )


class MfScQuadratureOrderSequenceConfig(DakotaBaseModel):
    "Sequence of quadrature orders used in a multi-stage expansion"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesmfscquadratureordersequenceconfig", list_field="sequence"
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
        Union[MfScQuadratureOrderSequenceNested, MfScQuadratureOrderSequenceNonNested]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class BayesMfScSGLevelSequenceConfig(DakotaBaseModel):
    "Sequence of sparse grid levels used in a multi-stage expansion"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesmfscsglevelsequenceconfig", list_field="sequence"
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
        Union[MfScSGLevelSequenceNodal, MfScSGLevelSequenceHierarchical] | None
    ) = DakotaField(default=None, dakota={"anchor": True, "union_pattern": 2})
    growth_rule: (
        Union[MfScSGLevelSequenceRestricted, MfScSGLevelSequenceUnrestricted] | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Growth",
        dakota={"anchor": True, "union_pattern": 2},
    )
    nesting_rule: (
        Union[MfScQuadratureOrderSequenceNested, MfScQuadratureOrderSequenceNonNested]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class Prior(DakotaBaseModel):
    "Uses the covariance of the prior distributions to define the MCMC proposal covariance."

    prior: PriorConfig = DakotaField(
        description="Uses the covariance of the prior distributions to define the MCMC proposal covariance.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.proposal_covariance_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "prior",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Derivatives(DakotaBaseModel):
    "Use derivatives to inform the MCMC proposal covariance."

    derivatives: DerivativesConfig = DakotaField(
        description="Use derivatives to inform the MCMC proposal covariance.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.proposal_covariance_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "derivatives",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ValuesConfig(DakotaBaseModel):
    "Specifies matrix values to use as the MCMC proposal covariance."

    elements: list[DakotaFloat] = DakotaField(
        description="Specifies matrix values to use as the MCMC proposal covariance.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.proposal_covariance_data",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    covariance_form: Union[ValuesDiag, ValuesMatrix] = DakotaField(
        description="Values For", dakota={"anchor": True, "union_pattern": 4}
    )


class FileConfig(DakotaBaseModel):
    "Uses a file to import a user-specified MCMC proposal covariance."

    filename: str = DakotaField(
        description="Uses a file to import a user-specified MCMC proposal covariance.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.proposal_covariance_filename",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    covariance_form: Union[ValuesDiag, ValuesMatrix] = DakotaField(
        description="Values For", dakota={"anchor": True, "union_pattern": 4}
    )


class BayesScPRefinementDimAdaptive(DakotaBaseModel):
    'Perform anisotropic expansion refinement by preferentially adapting in dimensions that are detected to have higher \\"importance\\".'

    dimension_adaptive: Union[
        ScPRefinementDimAdaptiveSobol, ScPRefinementDimAdaptiveGeneralized
    ] = DakotaField(
        description='Perform anisotropic expansion refinement by preferentially adapting in dimensions that are detected to have higher "importance".'
    )


class ScHRefinementDimAdaptive(DakotaBaseModel):
    'Perform anisotropic expansion refinement by preferentially adapting in dimensions that are detected to have higher \\"importance\\".'

    dimension_adaptive: Union[
        ScPRefinementDimAdaptiveSobol, ScPRefinementDimAdaptiveGeneralized
    ] = DakotaField(
        description='Perform anisotropic expansion refinement by preferentially adapting in dimensions that are detected to have higher "importance".'
    )


class BayesScQuadratureOrderConfig(DakotaBaseModel):
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
    nesting_rule: Union[ScQuadratureOrderNested, ScQuadratureOrderNonNested] | None = (
        DakotaField(
            default=None,
            description="Quadrature Rule Nesting",
            dakota={"anchor": True, "union_pattern": 2},
        )
    )


class BayesScSGLevelConfig(DakotaBaseModel):
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
    interpolant: Union[BayesScSGLevelNodal, BayesScSGLevelHierarchical] | None = (
        DakotaField(default=None, dakota={"anchor": True, "union_pattern": 2})
    )
    growth_rule: Union[BayesScSGLevelRestricted, BayesScSGLevelUnrestricted] | None = (
        DakotaField(
            default=None,
            description="Quadrature Rule Growth",
            dakota={"anchor": True, "union_pattern": 2},
        )
    )
    nesting_rule: Union[ScQuadratureOrderNested, ScQuadratureOrderNonNested] | None = (
        DakotaField(
            default=None,
            description="Quadrature Rule Nesting",
            dakota={"anchor": True, "union_pattern": 2},
        )
    )


class BayesSeedRngMixin(DakotaBaseModel):
    "Generated model for BayesSeedRngMixin"

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
    rng: Union[BayesSeedRngMt19937, BayesSeedRngRnum2] = DakotaField(
        default_factory=BayesSeedRngMt19937,
        description="Selection of a random number generator",
        dakota={"union_pattern": 1, "model_default": "BayesSeedRngMt19937"},
    )


class GPImportBuildPointsFile(DakotaBaseModel):
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
        GPImportBuildPointsFileCustomAnnotated,
        GPImportBuildPointsFileAnnotated,
        GPImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=GPImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "GPImportBuildPointsFileAnnotated",
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


class EmulatorPceExpansionOrderImportBuildPointsFile(DakotaBaseModel):
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
        EmulatorPceExpansionOrderImportBuildPointsFileCustomAnnotated,
        GPImportBuildPointsFileAnnotated,
        GPImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=GPImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "GPImportBuildPointsFileAnnotated",
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


class EmulatorPceOrthogLeastInterpImportBuildPointsFile(DakotaBaseModel):
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
        EmulatorPceExpansionOrderImportBuildPointsFileCustomAnnotated,
        GPImportBuildPointsFileAnnotated,
        GPImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=GPImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "GPImportBuildPointsFileAnnotated",
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


class EmulatorScPRefinement(DakotaBaseModel):
    "Automatic polynomial order refinement"

    p_refinement: Union[
        EmulatorPcePRefinementUniform, EmulatorScPRefinementDimAdaptive
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


class EmulatorScHRefinement(DakotaBaseModel):
    "Employ h-refinement to refine the grid"

    h_refinement: Union[
        EmulatorPcePRefinementUniform,
        EmulatorScPRefinementDimAdaptive,
        EmulatorScHRefinementLocalAdaptive,
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


class EmulatorMfScPRefinement(DakotaBaseModel):
    "Automatic polynomial order refinement"

    p_refinement: Union[
        EmulatorPcePRefinementUniform, EmulatorScPRefinementDimAdaptive
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


class EmulatorMfScHRefinement(DakotaBaseModel):
    "Employ h-refinement to refine the grid"

    h_refinement: Union[
        EmulatorPcePRefinementUniform,
        EmulatorScPRefinementDimAdaptive,
        EmulatorScHRefinementLocalAdaptive,
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


class EmulatorPceQuadratureOrder(DakotaBaseModel):
    "Order for tensor-products of Gaussian quadrature rules"

    quadrature_order: EmulatorPceQuadratureOrderConfig = DakotaField(
        default=...,
        description="Order for tensor-products of Gaussian quadrature rules",
        dakota={"argument": "order"},
    )


class EmulatorMfPceQuadratureOrderSequence(DakotaBaseModel):
    "Sequence of quadrature orders used in a multi-stage expansion"

    quadrature_order_sequence: EmulatorMfPceQuadratureOrderSequenceConfig = DakotaField(
        default=...,
        description="Sequence of quadrature orders used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class EmulatorMfScQuadratureOrderSequence(DakotaBaseModel):
    "Sequence of quadrature orders used in a multi-stage expansion"

    quadrature_order_sequence: EmulatorMfPceQuadratureOrderSequenceConfig = DakotaField(
        default=...,
        description="Sequence of quadrature orders used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class EmulatorScQuadratureOrder(DakotaBaseModel):
    "Order for tensor-products of Gaussian quadrature rules"

    quadrature_order: EmulatorScQuadratureOrderConfig = DakotaField(
        default=...,
        description="Order for tensor-products of Gaussian quadrature rules",
        dakota={"argument": "order"},
    )


class EmulatorPceSGLevel(DakotaBaseModel):
    "Level to use in sparse grid integration or interpolation"

    sparse_grid_level: EmulatorPceSGLevelConfig = DakotaField(
        default=...,
        description="Level to use in sparse grid integration or interpolation",
        dakota={"argument": "level"},
    )


class EmulatorMfPceSGLevelSequence(DakotaBaseModel):
    "Sequence of sparse grid levels used in a multi-stage expansion"

    sparse_grid_level_sequence: EmulatorMfPceSGLevelSequenceConfig = DakotaField(
        default=...,
        description="Sequence of sparse grid levels used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class EmulatorPceExpansionOrderCollocPointsConfig(DakotaBaseModel):
    "Number of collocation points used to estimate expansion coefficients"

    points: int = DakotaField(
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
    regression_method: (
        Union[
            EmulatorPceExpansionOrderCollocPointsLeastSquares,
            EmulatorPceExpansionOrderCollocPointsOMP,
            EmulatorPceExpansionOrderCollocPointsBP,
            EmulatorPceExpansionOrderCollocPointsBPDN,
            EmulatorPceExpansionOrderCollocPointsLars,
            EmulatorPceExpansionOrderCollocPointsLasso,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Regression Algorithm",
        dakota={"anchor": True, "union_pattern": 2},
    )
    cross_validation: EmulatorPceExpansionOrderCollocPointsCV | None = DakotaField(
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


class EmulatorPceExpansionOrderCollocRatioConfig(DakotaBaseModel):
    "Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion."

    value: DakotaFloat = DakotaField(
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
    regression_method: (
        Union[
            EmulatorPceExpansionOrderCollocRatioLeastSquares,
            EmulatorPceExpansionOrderCollocRatioOMP,
            EmulatorPceExpansionOrderCollocPointsBP,
            EmulatorPceExpansionOrderCollocPointsBPDN,
            EmulatorPceExpansionOrderCollocPointsLars,
            EmulatorPceExpansionOrderCollocRatioLasso,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Regression Algorithm",
        dakota={"anchor": True, "union_pattern": 2},
    )
    cross_validation: EmulatorPceExpansionOrderCollocPointsCV | None = DakotaField(
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


class EmulatorMlPceExpansionOrderSequenceCollocRatioConfig(DakotaBaseModel):
    "Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion."

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesemulatormlpceexpansionordersequencecollocratioconfig",
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
            EmulatorPceExpansionOrderCollocRatioLeastSquares,
            EmulatorPceExpansionOrderCollocRatioOMP,
            EmulatorPceExpansionOrderCollocPointsBP,
            EmulatorPceExpansionOrderCollocPointsBPDN,
            EmulatorPceExpansionOrderCollocPointsLars,
            EmulatorPceExpansionOrderCollocRatioLasso,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Regression Algorithm",
        dakota={"anchor": True, "union_pattern": 2},
    )
    cross_validation: EmulatorPceExpansionOrderCollocPointsCV | None = DakotaField(
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


class EmulatorMfPceExpansionOrderSequenceCollocRatioConfig(DakotaBaseModel):
    "Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion."

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesemulatormfpceexpansionordersequencecollocratioconfig",
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
            EmulatorPceExpansionOrderCollocRatioLeastSquares,
            EmulatorPceExpansionOrderCollocRatioOMP,
            EmulatorPceExpansionOrderCollocPointsBP,
            EmulatorPceExpansionOrderCollocPointsBPDN,
            EmulatorPceExpansionOrderCollocPointsLars,
            EmulatorPceExpansionOrderCollocRatioLasso,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Regression Algorithm",
        dakota={"anchor": True, "union_pattern": 2},
    )
    cross_validation: EmulatorPceExpansionOrderCollocPointsCV | None = DakotaField(
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


class EmulatorScSGLevel(DakotaBaseModel):
    "Level to use in sparse grid integration or interpolation"

    sparse_grid_level: EmulatorScSGLevelConfig = DakotaField(
        default=...,
        description="Level to use in sparse grid integration or interpolation",
        dakota={"argument": "level"},
    )


class EmulatorMfScSGLevelSequence(DakotaBaseModel):
    "Sequence of sparse grid levels used in a multi-stage expansion"

    sparse_grid_level_sequence: EmulatorMfScSGLevelSequenceConfig = DakotaField(
        default=...,
        description="Sequence of sparse grid levels used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class BayesMfScPRefinement(DakotaBaseModel):
    "Automatic polynomial order refinement"

    p_refinement: Union[MfScPRefinementUniform, MfScPRefinementDimAdaptive] = (
        DakotaField(
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
    )


class BayesMfScHRefinement(DakotaBaseModel):
    "Employ h-refinement to refine the grid"

    h_refinement: Union[
        MfScPRefinementUniform, MfScHRefinementDimAdaptive, MfScHRefinementLocalAdaptive
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


class BayesMfScQuadratureOrderSequence(DakotaBaseModel):
    "Sequence of quadrature orders used in a multi-stage expansion"

    quadrature_order_sequence: MfScQuadratureOrderSequenceConfig = DakotaField(
        default=...,
        description="Sequence of quadrature orders used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class BayesMfScSGLevelSequence(DakotaBaseModel):
    "Sequence of sparse grid levels used in a multi-stage expansion"

    sparse_grid_level_sequence: BayesMfScSGLevelSequenceConfig = DakotaField(
        default=...,
        description="Sequence of sparse grid levels used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class Values(DakotaBaseModel):
    "Specifies matrix values to use as the MCMC proposal covariance."

    values: ValuesConfig = DakotaField(
        default=...,
        description="Specifies matrix values to use as the MCMC proposal covariance.",
        dakota={"argument": "elements"},
    )


class ProposalCovFile(DakotaBaseModel):
    "Uses a file to import a user-specified MCMC proposal covariance."

    file: FileConfig = DakotaField(
        default=...,
        description="Uses a file to import a user-specified MCMC proposal covariance.",
        dakota={"argument": "filename"},
    )


class BayesScPRefinement(DakotaBaseModel):
    "Automatic polynomial order refinement"

    p_refinement: Union[ScPRefinementUniform, BayesScPRefinementDimAdaptive] = (
        DakotaField(
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
    )


class BayesScHRefinement(DakotaBaseModel):
    "Employ h-refinement to refine the grid"

    h_refinement: Union[
        ScPRefinementUniform, ScHRefinementDimAdaptive, BayesScHRefinementLocalAdaptive
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


class BayesScQuadratureOrder(DakotaBaseModel):
    "Order for tensor-products of Gaussian quadrature rules"

    quadrature_order: BayesScQuadratureOrderConfig = DakotaField(
        default=...,
        description="Order for tensor-products of Gaussian quadrature rules",
        dakota={"argument": "order"},
    )


class BayesScSGLevel(DakotaBaseModel):
    "Level to use in sparse grid integration or interpolation"

    sparse_grid_level: BayesScSGLevelConfig = DakotaField(
        default=...,
        description="Level to use in sparse grid integration or interpolation",
        dakota={"argument": "level"},
    )


class EmulatorGPConfig(DakotaBaseModel):
    "Gaussian Process surrogate model"

    implementation: Union[EmulatorGPSurfpack, EmulatorGPDakota] = DakotaField(
        description="GP Implementation", dakota={"anchor": True, "union_pattern": 4}
    )
    build_samples: int = DakotaField(
        default=0,
        description="Number of initial model evaluations used in build phase",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.build_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    posterior_adaptive: Literal[True] | None = DakotaField(
        default=None,
        description="Adapt emulator model to increase accuracy in high posterior probability regions",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.adaptive_posterior_refinement",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    import_build_points_file: GPImportBuildPointsFile | None = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )


class EmulatorPceOrthogLeastInterpConfig(DakotaBaseModel):
    "Build a polynomial chaos expansion from simulation samples using orthogonal least interpolation."

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesemulatorpceorthogleastinterpconfig", list_field="tensor_grid"
        ),
    ]

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
    import_build_points_file: (
        EmulatorPceOrthogLeastInterpImportBuildPointsFile | None
    ) = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )
    posterior_adaptive: Literal[True] | None = DakotaField(
        default=None,
        description="Adapt emulator model to increase accuracy in high posterior probability regions",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.adaptive_posterior_refinement",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class EmulatorMlPceOrthogLeastInterpConfig(DakotaBaseModel):
    "Build a polynomial chaos expansion from simulation samples using orthogonal least interpolation."

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesemulatormlpceorthogleastinterpconfig",
            list_field="collocation_points_sequence",
        ),
        CheckNonnegativeList(
            context="bayesemulatormlpceorthogleastinterpconfig",
            list_field="tensor_grid",
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
    import_build_points_file: (
        EmulatorPceOrthogLeastInterpImportBuildPointsFile | None
    ) = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )


class EmulatorPceExpansionOrderCollocPoints(DakotaBaseModel):
    "Number of collocation points used to estimate expansion coefficients"

    collocation_points: EmulatorPceExpansionOrderCollocPointsConfig = DakotaField(
        default=...,
        description="Number of collocation points used to estimate expansion coefficients",
        dakota={"argument": "points"},
    )


class EmulatorPceExpansionOrderCollocRatio(DakotaBaseModel):
    "Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion."

    collocation_ratio: EmulatorPceExpansionOrderCollocRatioConfig = DakotaField(
        default=...,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={"argument": "value"},
    )


class EmulatorMlPceExpansionOrderSequenceCollocRatio(DakotaBaseModel):
    "Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion."

    collocation_ratio: EmulatorMlPceExpansionOrderSequenceCollocRatioConfig = DakotaField(
        default=...,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={"argument": "sequence"},
    )


class EmulatorMfPceExpansionOrderSequenceCollocRatio(DakotaBaseModel):
    "Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion."

    collocation_ratio: EmulatorMfPceExpansionOrderSequenceCollocRatioConfig = DakotaField(
        default=...,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={"argument": "sequence"},
    )


class ScConfig(DakotaBaseModel):
    "Stochastic Collocation as an emulator model."

    refinement_method: Union[EmulatorScPRefinement, EmulatorScHRefinement] | None = (
        DakotaField(
            default=None,
            description="Automated Refinement Type",
            dakota={"anchor": True, "union_pattern": 2},
        )
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
    grid_type: Union[EmulatorScQuadratureOrder, EmulatorScSGLevel] = DakotaField(
        description="Interpolation Grid Type",
        dakota={"anchor": True, "union_pattern": 4},
    )
    basis_family: (
        Union[EmulatorScPiecewise, EmulatorPceAskey, EmulatorPceWiener] | None
    ) = DakotaField(
        default=None,
        description="Basis Polynomial Family",
        dakota={"anchor": True, "union_pattern": 2},
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
    covariance_type: Union[EmulatorPceDiagCov, EmulatorPceFullCov] | None = DakotaField(
        default=None,
        description="Covariance Type",
        dakota={"anchor": True, "union_pattern": 2},
    )


class MfScConfig(DakotaBaseModel):
    "Multifidelity Stochastic Collocation as an emulator model."

    refinement_method: (
        Union[EmulatorMfScPRefinement, EmulatorMfScHRefinement] | None
    ) = DakotaField(
        default=None,
        description="Automated Refinement Type",
        dakota={"anchor": True, "union_pattern": 2},
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
    allocation_control: Union[EmulatorMfPceAllocationControlGreedy] | None = (
        DakotaField(
            default=None,
            description="Sample allocation approach for multifidelity expansions",
            dakota={"union_pattern": 2},
        )
    )
    discrepancy_emulation: (
        Union[
            EmulatorMlPceDiscrepancyEmulationDistinct,
            EmulatorMlPceDiscrepancyEmulationRecursive,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Formulation for emulation of model discrepancies.",
        dakota={"union_pattern": 2},
    )
    grid_type: Union[
        EmulatorMfScQuadratureOrderSequence, EmulatorMfScSGLevelSequence
    ] = DakotaField(
        description="Interpolation Grid Type",
        dakota={"anchor": True, "union_pattern": 4},
    )
    basis_family: (
        Union[EmulatorScPiecewise, EmulatorPceAskey, EmulatorPceWiener] | None
    ) = DakotaField(
        default=None,
        description="Basis Polynomial Family",
        dakota={"anchor": True, "union_pattern": 2},
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
    covariance_type: Union[EmulatorPceDiagCov, EmulatorPceFullCov] | None = DakotaField(
        default=None,
        description="Covariance Type",
        dakota={"anchor": True, "union_pattern": 2},
    )


class BayesMfScMfSc(DakotaBaseModel):
    "Multifidelity Stochastic Collocation as an emulator model."

    refinement_method: Union[BayesMfScPRefinement, BayesMfScHRefinement] | None = (
        DakotaField(
            default=None,
            description="Automated Refinement Type",
            dakota={"anchor": True, "union_pattern": 2},
        )
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
    allocation_control: Union[MfScAllocationControlGreedy] | None = DakotaField(
        default=None,
        description="Sample allocation approach for multifidelity expansions",
        dakota={"union_pattern": 2},
    )
    discrepancy_emulation: (
        Union[MfScDiscrepancyEmulationDistinct, MfScDiscrepancyEmulationRecursive]
        | None
    ) = DakotaField(
        default=None,
        description="Formulation for emulation of model discrepancies.",
        dakota={"union_pattern": 2},
    )
    grid_type: Union[BayesMfScQuadratureOrderSequence, BayesMfScSGLevelSequence] = (
        DakotaField(
            description="Interpolation Grid Type",
            dakota={"anchor": True, "union_pattern": 4},
        )
    )
    basis_family: Union[MfScPiecewise, MfScAskey, MfScWiener] | None = DakotaField(
        default=None,
        description="Basis Polynomial Family",
        dakota={"anchor": True, "union_pattern": 2},
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
    covariance_type: Union[MfScDiagCov, MfScFullCov] | None = DakotaField(
        default=None,
        description="Covariance Type",
        dakota={"anchor": True, "union_pattern": 2},
    )


class BayesProposalCovMixin(DakotaBaseModel):
    "Generated model for BayesProposalCovMixin"

    proposal_covariance: Union[Prior, Derivatives, Values, ProposalCovFile] | None = (
        DakotaField(
            default=None,
            description="Defines the technique used to generate the MCMC proposal covariance.",
            dakota={
                "union_pattern": 2,
                "materialization": [
                    {
                        "ir_key": "method.nond.proposal_covariance_type",
                        "storage_type": "PRESENCE_LITERAL",
                        "stored_value": "user",
                        "ir_value_type": "String",
                    }
                ],
            },
        )
    )


class BayesScSc(DakotaBaseModel):
    "Stochastic Collocation as an emulator model."

    refinement_method: Union[BayesScPRefinement, BayesScHRefinement] | None = (
        DakotaField(
            default=None,
            description="Automated Refinement Type",
            dakota={"anchor": True, "union_pattern": 2},
        )
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
    grid_type: Union[BayesScQuadratureOrder, BayesScSGLevel] = DakotaField(
        description="Interpolation Grid Type",
        dakota={"anchor": True, "union_pattern": 4},
    )
    basis_family: Union[BayesScPiecewise, ScAskey, ScWiener] | None = DakotaField(
        default=None,
        description="Basis Polynomial Family",
        dakota={"anchor": True, "union_pattern": 2},
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
    covariance_type: Union[ScDiagCov, ScFullCov] | None = DakotaField(
        default=None,
        description="Covariance Type",
        dakota={"anchor": True, "union_pattern": 2},
    )


class EmulatorGP(DakotaBaseModel):
    "Gaussian Process surrogate model"

    gaussian_process: EmulatorGPConfig = DakotaField(
        description="Gaussian Process surrogate model", dakota={"aliases": ["kriging"]}
    )


class EmulatorPceOrthogLeastInterp(DakotaBaseModel):
    "Build a polynomial chaos expansion from simulation samples using orthogonal least interpolation."

    orthogonal_least_interpolation: EmulatorPceOrthogLeastInterpConfig = DakotaField(
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


class EmulatorMlPceOrthogLeastInterp(DakotaBaseModel):
    "Build a polynomial chaos expansion from simulation samples using orthogonal least interpolation."

    orthogonal_least_interpolation: EmulatorMlPceOrthogLeastInterpConfig = DakotaField(
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


class EmulatorMfPceOrthogLeastInterp(DakotaBaseModel):
    "Build a polynomial chaos expansion from simulation samples using orthogonal least interpolation."

    orthogonal_least_interpolation: EmulatorMlPceOrthogLeastInterpConfig = DakotaField(
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


class EmulatorPceExpansionOrderConfig(DakotaBaseModel):
    "The (initial) order of a polynomial expansion"

    order: int = DakotaField(
        default=USHRT_MAX,
        description="The (initial) order of a polynomial expansion",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_order",
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
    basis_type: (
        Union[
            EmulatorPceExpansionOrderBasisTypeTensorProduct,
            EmulatorPceExpansionOrderBasisTypeTotalOrder,
            EmulatorPceExpansionOrderBasisTypeAdapted,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Specify the type of basis truncation to be used for a Polynomial Chaos Expansion.",
        dakota={"union_pattern": 2},
    )
    point_selection: Union[
        EmulatorPceExpansionOrderCollocPoints,
        EmulatorPceExpansionOrderCollocRatio,
        EmulatorPceExpansionOrderExpansionSamples,
    ] = DakotaField(dakota={"anchor": True, "union_pattern": 4})
    import_build_points_file: EmulatorPceExpansionOrderImportBuildPointsFile | None = (
        DakotaField(
            default=None,
            description="File containing points you wish to use to build a surrogate",
            dakota={"argument": "filename", "aliases": ["import_points_file"]},
        )
    )
    posterior_adaptive: Literal[True] | None = DakotaField(
        default=None,
        description="Adapt emulator model to increase accuracy in high posterior probability regions",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.adaptive_posterior_refinement",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class EmulatorMlPceExpansionOrderSequenceConfig(DakotaBaseModel):
    "Sequence of expansion orders used in a multi-stage expansion"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesemulatormlpceexpansionordersequenceconfig",
            list_field="sequence",
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
            EmulatorPceExpansionOrderBasisTypeTensorProduct,
            EmulatorPceExpansionOrderBasisTypeTotalOrder,
            EmulatorMlPceExpansionOrderSequenceBasisTypeAdapted,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Specify the type of basis truncation to be used for a Polynomial Chaos Expansion.",
        dakota={"union_pattern": 2},
    )
    point_sequence_selection: Union[
        EmulatorMlPceExpansionOrderSequenceCollocRatio,
        EmulatorMlPceExpansionOrderSequenceExpansionSamplesSequence,
    ] = DakotaField(dakota={"anchor": True, "union_pattern": 4})
    import_build_points_file: (
        EmulatorPceOrthogLeastInterpImportBuildPointsFile | None
    ) = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )


class EmulatorMfPceExpansionOrderSequenceConfig(DakotaBaseModel):
    "Sequence of expansion orders used in a multi-stage expansion"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesemulatormfpceexpansionordersequenceconfig",
            list_field="sequence",
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
            EmulatorPceExpansionOrderBasisTypeTensorProduct,
            EmulatorPceExpansionOrderBasisTypeTotalOrder,
            EmulatorMlPceExpansionOrderSequenceBasisTypeAdapted,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Specify the type of basis truncation to be used for a Polynomial Chaos Expansion.",
        dakota={"union_pattern": 2},
    )
    point_sequence_selection: Union[
        EmulatorMfPceExpansionOrderSequenceCollocRatio,
        EmulatorMfPceExpansionOrderSequenceExpansionSamplesSequence,
    ] = DakotaField(dakota={"anchor": True, "union_pattern": 4})
    import_build_points_file: (
        EmulatorPceOrthogLeastInterpImportBuildPointsFile | None
    ) = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )


class EmulatorSc(DakotaBaseModel):
    "Stochastic Collocation as an emulator model."

    sc: ScConfig = DakotaField(
        description="Stochastic Collocation as an emulator model.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SC_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class EmulatorMfSc(DakotaBaseModel):
    "Multifidelity Stochastic Collocation as an emulator model."

    mf_sc: MfScConfig = DakotaField(
        description="Multifidelity Stochastic Collocation as an emulator model.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MF_SC_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesMfScMixin(DakotaBaseModel):
    "Generated model for BayesMfScMixin"

    mf_sc: BayesMfScMfSc = DakotaField(
        description="Multifidelity Stochastic Collocation as an emulator model.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MF_SC_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesScMixin(DakotaBaseModel):
    "Generated model for BayesScMixin"

    sc: BayesScSc = DakotaField(
        description="Stochastic Collocation as an emulator model.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SC_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class EmulatorPceExpansionOrder(DakotaBaseModel):
    "The (initial) order of a polynomial expansion"

    expansion_order: EmulatorPceExpansionOrderConfig = DakotaField(
        default=...,
        description="The (initial) order of a polynomial expansion",
        dakota={"argument": "order"},
    )


class EmulatorMlPceExpansionOrderSequence(DakotaBaseModel):
    "Sequence of expansion orders used in a multi-stage expansion"

    expansion_order_sequence: EmulatorMlPceExpansionOrderSequenceConfig = DakotaField(
        default=...,
        description="Sequence of expansion orders used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class EmulatorMfPceExpansionOrderSequence(DakotaBaseModel):
    "Sequence of expansion orders used in a multi-stage expansion"

    expansion_order_sequence: EmulatorMfPceExpansionOrderSequenceConfig = DakotaField(
        default=...,
        description="Sequence of expansion orders used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class BayesEmulatorPceConfig(DakotaBaseModel):
    "Polynomial Chaos Expansion surrogate model"

    p_refinement: (
        Union[EmulatorPcePRefinementUniform, EmulatorPcePRefinementDimAdaptive] | None
    ) = DakotaField(
        default=None,
        description="Automatic polynomial order refinement",
        dakota={
            "union_pattern": 2,
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "P_REFINEMENT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
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
    coefficient_approach: Union[
        EmulatorPceQuadratureOrder,
        EmulatorPceSGLevel,
        EmulatorPceCubatureIntegrand,
        EmulatorPceExpansionOrder,
        EmulatorPceOrthogLeastInterp,
        EmulatorPceImportExpansionFile,
    ] = DakotaField(dakota={"anchor": True, "union_pattern": 4})
    basis_family: Union[EmulatorPceAskey, EmulatorPceWiener] | None = DakotaField(
        default=None,
        description="Basis Polynomial Family",
        dakota={"anchor": True, "union_pattern": 2},
    )
    normalized: Literal[True] | None = DakotaField(
        default=None,
        description="The normalized specification requests output of PCE coefficients that correspond to normalized orthogonal basis polynomials",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.normalized",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    export_expansion_file: str | None = DakotaField(
        default=None,
        description="Export the coefficients and multi-index of a Polynomial Chaos Expansion (PCE) to a file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_expansion_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    covariance_type: Union[EmulatorPceDiagCov, EmulatorPceFullCov] | None = DakotaField(
        default=None,
        description="Covariance Type",
        dakota={"anchor": True, "union_pattern": 2},
    )


class BayesEmulatorMlPceConfig(DakotaBaseModel):
    "Multilevel Polynomial Chaos Expansion as an emulator model."

    allocation_control: (
        Union[
            EmulatorMlPceAllocationControlEstimatorVariance,
            EmulatorMlPceAllocationControlRipSampling,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Sample allocation approach for multilevel expansions",
        dakota={"union_pattern": 2},
    )
    discrepancy_emulation: (
        Union[
            EmulatorMlPceDiscrepancyEmulationDistinct,
            EmulatorMlPceDiscrepancyEmulationRecursive,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Formulation for emulation of model discrepancies.",
        dakota={"union_pattern": 2},
    )
    coefficient_approach: Union[
        EmulatorMlPceExpansionOrderSequence, EmulatorMlPceOrthogLeastInterp
    ] = DakotaField(dakota={"anchor": True, "union_pattern": 4})
    basis_family: Union[EmulatorPceAskey, EmulatorPceWiener] | None = DakotaField(
        default=None,
        description="Basis Polynomial Family",
        dakota={"anchor": True, "union_pattern": 2},
    )
    normalized: Literal[True] | None = DakotaField(
        default=None,
        description="The normalized specification requests output of PCE coefficients that correspond to normalized orthogonal basis polynomials",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.normalized",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    export_expansion_file: str | None = DakotaField(
        default=None,
        description="Export the coefficients and multi-index of a Polynomial Chaos Expansion (PCE) to a file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_expansion_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    covariance_type: Union[EmulatorPceDiagCov, EmulatorPceFullCov] | None = DakotaField(
        default=None,
        description="Covariance Type",
        dakota={"anchor": True, "union_pattern": 2},
    )


class BayesEmulatorMfPceConfig(DakotaBaseModel):
    "Multifidelity Polynomial Chaos Expansion as an emulator model."

    p_refinement: (
        Union[EmulatorPcePRefinementUniform, EmulatorMfPcePRefinementDimAdaptive] | None
    ) = DakotaField(
        default=None,
        description="Automatic polynomial order refinement",
        dakota={
            "union_pattern": 2,
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "P_REFINEMENT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
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
    allocation_control: Union[EmulatorMfPceAllocationControlGreedy] | None = (
        DakotaField(
            default=None,
            description="Sample allocation approach for multifidelity expansions",
            dakota={"union_pattern": 2},
        )
    )
    discrepancy_emulation: (
        Union[
            EmulatorMlPceDiscrepancyEmulationDistinct,
            EmulatorMlPceDiscrepancyEmulationRecursive,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Formulation for emulation of model discrepancies.",
        dakota={"union_pattern": 2},
    )
    coefficient_approach: Union[
        EmulatorMfPceQuadratureOrderSequence,
        EmulatorMfPceSGLevelSequence,
        EmulatorMfPceExpansionOrderSequence,
        EmulatorMfPceOrthogLeastInterp,
    ] = DakotaField(dakota={"anchor": True, "union_pattern": 4})
    basis_family: Union[EmulatorPceAskey, EmulatorPceWiener] | None = DakotaField(
        default=None,
        description="Basis Polynomial Family",
        dakota={"anchor": True, "union_pattern": 2},
    )
    normalized: Literal[True] | None = DakotaField(
        default=None,
        description="The normalized specification requests output of PCE coefficients that correspond to normalized orthogonal basis polynomials",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.normalized",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    export_expansion_file: str | None = DakotaField(
        default=None,
        description="Export the coefficients and multi-index of a Polynomial Chaos Expansion (PCE) to a file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_expansion_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    covariance_type: Union[EmulatorPceDiagCov, EmulatorPceFullCov] | None = DakotaField(
        default=None,
        description="Covariance Type",
        dakota={"anchor": True, "union_pattern": 2},
    )


class EmulatorPce(DakotaBaseModel):
    "Polynomial Chaos Expansion surrogate model"

    pce: BayesEmulatorPceConfig = DakotaField(
        description="Polynomial Chaos Expansion surrogate model",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "PCE_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class EmulatorMlPce(DakotaBaseModel):
    "Multilevel Polynomial Chaos Expansion as an emulator model."

    ml_pce: BayesEmulatorMlPceConfig = DakotaField(
        description="Multilevel Polynomial Chaos Expansion as an emulator model.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ML_PCE_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class EmulatorMfPce(DakotaBaseModel):
    "Multifidelity Polynomial Chaos Expansion as an emulator model."

    mf_pce: BayesEmulatorMfPceConfig = DakotaField(
        description="Multifidelity Polynomial Chaos Expansion as an emulator model.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MF_PCE_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesEmulatorMixin(DakotaBaseModel):
    "Generated model for BayesEmulatorMixin"

    emulator: (
        Union[
            EmulatorGP,
            EmulatorPce,
            EmulatorMlPce,
            EmulatorMfPce,
            EmulatorSc,
            EmulatorMfSc,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Use an emulator or surrogate model to evaluate the likelihood function",
        dakota={"union_pattern": 2},
    )
