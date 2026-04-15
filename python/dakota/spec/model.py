"""Generated Pydantic models for model"""

from __future__ import annotations

from .base import DakotaBaseModel, DakotaField, DakotaFloat
from typing import Literal, Union
from typing import ClassVar
from .base import SZ_MAX
from typing import List
from .validation import ValidationRule
from .validation.rules import CheckPositiveList

# Cross-module model imports
from dakota.spec.shared.expansion.function_train import (
    FtModelOrderMixin,
    FtModelRankMixin,
    FtModelRegressionOptsMixin,
    FtModelTolsMixin,
)
from dakota.spec.shared.formats import ModelFullSurfpackExportFormatMixin
from dakota.spec.shared.misc import (
    ModelFourOptionalKeywordsMixin,
    ModelVarianceExportMixin,
)
from dakota.spec.shared.surrogate import (
    ModelPartialSurrogateExportFormatMixin,
    ModelSurrogateImportMixin,
    SurrogateCorrectionMixin,
)


class ModelSelection(DakotaBaseModel):
    """Base class for model discriminator wrappers"""

    _registry: ClassVar[dict[str, type["ModelSelection"]]] = {}

    def __init_subclass__(cls, **kwargs):
        super().__init_subclass__(**kwargs)
        if cls.__name__ == "ModelSelection":
            return
        if not cls.__name__.endswith("Selection"):
            raise TypeError(f"{cls.__name__} must end with 'Selection'")

    @classmethod
    def get_registry(cls) -> dict[str, type["ModelSelection"]]:
        if not cls._registry:
            for subclass in cls.__subclasses__():
                if subclass.__name__ == "ModelSelection":
                    continue
                fields = list(subclass.model_fields.keys())
                if len(fields) == 1:
                    cls._registry[fields[0]] = subclass
        return cls._registry.copy()

    @classmethod
    def get_union(cls):
        cls.get_registry()
        from typing import Union

        return Union[tuple(cls._registry.values())]


class ModelConfig(DakotaBaseModel):
    """Generated model for ModelConfig"""

    model: ModelUnion | None = DakotaField(default=None, dakota={"union_pattern": 2})


class SolutionLevelCost(DakotaBaseModel):
    """Generated model for SolutionLevelCost"""

    solution_level_cost: list[DakotaFloat] = DakotaField(
        description="Cost estimates associated with a set of solution control values.",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.simulation.solution_level_cost",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class CostRecoveryMetadata(DakotaBaseModel):
    """Generated model for CostRecoveryMetadata"""

    cost_recovery_metadata: str = DakotaField(
        description="Identify metadata by label for capturing online cost estimates",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.simulation.cost_recovery_metadata",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class TrendNone(DakotaBaseModel):
    """Generated model for TrendNone"""

    none: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.trend_order",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "none",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ExperimentalGPTrendConstant(DakotaBaseModel):
    """Generated model for ExperimentalGPTrendConstant"""

    constant: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.trend_order",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "constant",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ExperimentalGPTrendLinear(DakotaBaseModel):
    """Generated model for ExperimentalGPTrendLinear"""

    linear: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.trend_order",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "linear",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ExperimentalGPTrendReducedQuadratic(DakotaBaseModel):
    """Generated model for ExperimentalGPTrendReducedQuadratic"""

    reduced_quadratic: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.trend_order",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "reduced_quadratic",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ExperimentalGPTrendQuadratic(DakotaBaseModel):
    """Generated model for ExperimentalGPTrendQuadratic"""

    quadratic: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.trend_order",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "quadratic",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ExperimentalGPNugget(DakotaBaseModel):
    """Generated model for ExperimentalGPNugget"""

    nugget: DakotaFloat = DakotaField(
        gt=0,
        description="Value for the fixed nugget parameter",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.nugget",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class ExperimentalGPFindNugget(DakotaBaseModel):
    """Generated model for ExperimentalGPFindNugget"""

    find_nugget: int = DakotaField(
        description="Use regression to estimate the nugget.",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.find_nugget",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class DakotaTrendConstant(DakotaBaseModel):
    """Generated model for DakotaTrendConstant"""

    constant: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.trend_order",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "constant",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class DakotaTrendLinear(DakotaBaseModel):
    """Generated model for DakotaTrendLinear"""

    linear: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.trend_order",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "linear",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class DakotaTrendReducedQuadratic(DakotaBaseModel):
    """Generated model for DakotaTrendReducedQuadratic"""

    reduced_quadratic: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.trend_order",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "reduced_quadratic",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class SurfpackTrendConstant(DakotaBaseModel):
    """Generated model for SurfpackTrendConstant"""

    constant: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.trend_order",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "constant",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class SurfpackTrendLinear(DakotaBaseModel):
    """Generated model for SurfpackTrendLinear"""

    linear: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.trend_order",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "linear",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class SurfpackTrendReducedQuadratic(DakotaBaseModel):
    """Generated model for SurfpackTrendReducedQuadratic"""

    reduced_quadratic: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.trend_order",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "reduced_quadratic",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class SurfpackTrendQuadratic(DakotaBaseModel):
    """Generated model for SurfpackTrendQuadratic"""

    quadratic: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.trend_order",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "quadratic",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class SurfpackNugget(DakotaBaseModel):
    """Generated model for SurfpackNugget"""

    nugget: DakotaFloat = DakotaField(
        gt=0,
        description="Specify a nugget to handle ill-conditioning",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.nugget",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class SurfpackFindNugget(DakotaBaseModel):
    """Generated model for SurfpackFindNugget"""

    find_nugget: int = DakotaField(
        description="Have Surfpack compute a nugget to handle ill-conditioning",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.find_nugget",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class InterpLinear(DakotaBaseModel):
    """Generated model for InterpLinear"""

    linear: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.mars_interpolation",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "linear",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class InterpCubic(DakotaBaseModel):
    """Generated model for InterpCubic"""

    cubic: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.mars_interpolation",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "cubic",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MovingLeastSquaresConfig(
    ModelPartialSurrogateExportFormatMixin, ModelSurrogateImportMixin
):
    """Generated model for MovingLeastSquaresConfig"""

    basis_order: int = DakotaField(
        default=2,
        ge=0,
        description="Polynomial order for the MLS bases",
        dakota={
            "aliases": ["poly_order"],
            "materialization": [
                {
                    "ir_key": "model.surrogate.polynomial_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "short",
                }
            ],
        },
    )
    weight_function: int = DakotaField(
        default=0,
        description="Selects the weight function for the MLS model",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.mls_weight_function",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ModelSurrogateGlobalApproxFtConfig(
    FtModelRegressionOptsMixin, FtModelTolsMixin, FtModelOrderMixin, FtModelRankMixin
):
    """Generated model for ModelSurrogateGlobalApproxFtConfig"""

    pass


class NeuralNetworkConfig(
    ModelFullSurfpackExportFormatMixin, ModelSurrogateImportMixin
):
    """Generated model for NeuralNetworkConfig"""

    max_nodes: int = DakotaField(
        default=0,
        description="Maximum number of hidden layer nodes",
        dakota={
            "aliases": ["nodes"],
            "materialization": [
                {
                    "ir_key": "model.surrogate.neural_network_nodes",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "short",
                }
            ],
        },
    )
    range: DakotaFloat = DakotaField(
        default=0.0,
        description="Range for neural network random weights",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.neural_network_range",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    random_weight: int = DakotaField(
        default=0,
        description="(Inactive) Random weight control",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.neural_network_random_weight",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class RadialBasisConfig(ModelFullSurfpackExportFormatMixin, ModelSurrogateImportMixin):
    """Generated model for RadialBasisConfig"""

    bases: int = DakotaField(
        default=0,
        description="Initial number of radial basis functions",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.rbf_bases",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "short",
                }
            ]
        },
    )
    max_pts: int = DakotaField(
        default=0,
        description="Maximum number of RBF CVT points",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.rbf_max_pts",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "short",
                }
            ]
        },
    )
    min_partition: int = DakotaField(
        default=0,
        description="(Inactive) Minimum RBF partition",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.rbf_min_partition",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "short",
                }
            ]
        },
    )
    max_subsets: int = DakotaField(
        default=0,
        description="Number of trial RBF subsets",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.rbf_max_subsets",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BasisOrder(DakotaBaseModel):
    """Generated model for BasisOrder"""

    basis_order: int = DakotaField(
        ge=0,
        description="Polynomial order",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.polynomial_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PolynomialLinear(DakotaBaseModel):
    """Generated model for PolynomialLinear"""

    linear: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.polynomial_order",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": 1,
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PolynomialQuadratic(DakotaBaseModel):
    """Generated model for PolynomialQuadratic"""

    quadratic: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.polynomial_order",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": 2,
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PolynomialCubic(DakotaBaseModel):
    """Generated model for PolynomialCubic"""

    cubic: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.polynomial_order",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": 3,
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ExperimentalPolynomialConfig(
    ModelPartialSurrogateExportFormatMixin, ModelSurrogateImportMixin
):
    """Generated model for ExperimentalPolynomialConfig"""

    basis_order: int = DakotaField(
        default=2,
        ge=0,
        description="Total degree of the polynomial basis",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.polynomial_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "short",
                }
            ]
        },
    )
    options_file: str | None = DakotaField(
        default=None,
        description="Filename for a YAML file that specifies polynomial surrogate options",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.advanced_options_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ExperimentalPythonConfig(DakotaBaseModel):
    """Generated model for ExperimentalPythonConfig"""

    class_path_and_name: str = DakotaField(
        description="Specify the module and class names of the python surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.class_path_and_name",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class JumpThreshold(DakotaBaseModel):
    """Generated model for JumpThreshold"""

    jump_threshold: DakotaFloat = DakotaField(
        description="Gradient Threshold Parameter of the Optional Discontinuity Detection Capability for the Piecewise Decomposition Option of Global Surrogates",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.discont_jump_thresh",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class GradientThreshold(DakotaBaseModel):
    """Generated model for GradientThreshold"""

    gradient_threshold: DakotaFloat = DakotaField(
        description="Gradient Threshold Parameter of the Optional Discontinuity Detection Capability for the Piecewise Decomposition Option of Global Surrogates",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.discont_grad_thresh",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class TotalPoints(DakotaBaseModel):
    """Generated model for TotalPoints"""

    total_points: int = DakotaField(
        description="Specified number of training points",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.points_total",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )


class MinimumPoints(DakotaBaseModel):
    """Generated model for MinimumPoints"""

    minimum_points: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.points_management",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MINIMUM_POINTS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class RecommendedPoints(DakotaBaseModel):
    """Generated model for RecommendedPoints"""

    recommended_points: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.points_management",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RECOMMENDED_POINTS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class CVMetric(DakotaBaseModel):
    """Generated model for CVMetric"""

    metric: str = DakotaField(
        default="root_mean_squared",
        description="Choice of error metric to satisfy",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.refine_cv_metric",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    folds: int = DakotaField(
        default=10,
        gt=0,
        description="Number of cross validation folds",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.refine_cv_folds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )


class GlobalApproxTruthModelPointer(DakotaBaseModel):
    """Generated model for GlobalApproxTruthModelPointer"""

    truth_model_pointer: str = DakotaField(
        description="A surrogate model pointer that guides a method to whether it should use a surrogate model or compute truth function evaluations",
        dakota={
            "block_pointer": "model",
            "aliases": ["actual_model_pointer"],
            "materialization": [
                {
                    "ir_key": "model.surrogate.truth_model_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class ReusePointsAll(DakotaBaseModel):
    """Generated model for ReusePointsAll"""

    all: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Region(DakotaBaseModel):
    """Generated model for Region"""

    region: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "region",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ReusePointsNone(DakotaBaseModel):
    """Generated model for ReusePointsNone"""

    none: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "none",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ImportBuildPointsFileCustomAnnotatedHeader(DakotaBaseModel):
    """Generated model for ImportBuildPointsFileCustomAnnotatedHeader"""

    use_variable_labels: Literal[True] | None = DakotaField(
        default=None,
        description="Validate/use variable labels from tabular file header",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.import_use_variable_labels",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class ImportBuildPointsFileAnnotatedConfig(DakotaBaseModel):
    """Generated model for ImportBuildPointsFileAnnotatedConfig"""

    use_variable_labels: Literal[True] | None = DakotaField(
        default=None,
        description="Validate/use variable labels from tabular file header",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.import_use_variable_labels",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class GlobalApproxImportBuildPointsFileFreeform(DakotaBaseModel):
    """Generated model for GlobalApproxImportBuildPointsFileFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class GlobalApproxExportApproxPointsFileCustomAnnotatedConfig(DakotaBaseModel):
    """Generated model for GlobalApproxExportApproxPointsFileCustomAnnotatedConfig"""

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.export_approx_format",
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
                    "ir_key": "model.surrogate.export_approx_format",
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
                    "ir_key": "model.surrogate.export_approx_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class GlobalApproxExportApproxPointsFileAnnotated(DakotaBaseModel):
    """Generated model for GlobalApproxExportApproxPointsFileAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class GlobalApproxExportApproxPointsFileFreeform(DakotaBaseModel):
    """Generated model for GlobalApproxExportApproxPointsFileFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Folds(DakotaBaseModel):
    """Generated model for Folds"""

    folds: int = DakotaField(
        description="Number of cross validation folds",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.folds",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )


class Percent(DakotaBaseModel):
    """Generated model for Percent"""

    percent: DakotaFloat = DakotaField(
        description="Percent data per cross validation fold",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.percent",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class ImportChallengePointsFileCustomAnnotatedHeader(DakotaBaseModel):
    """Generated model for ImportChallengePointsFileCustomAnnotatedHeader"""

    use_variable_labels: Literal[True] | None = DakotaField(
        default=None,
        description="Validate/use variable labels from tabular file header",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.challenge_use_variable_labels",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class ImportChallengePointsFileAnnotatedConfig(DakotaBaseModel):
    """Generated model for ImportChallengePointsFileAnnotatedConfig"""

    use_variable_labels: Literal[True] | None = DakotaField(
        default=None,
        description="Validate/use variable labels from tabular file header",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.challenge_use_variable_labels",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class ImportChallengePointsFileFreeform(DakotaBaseModel):
    """Generated model for ImportChallengePointsFileFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.challenge_points_file_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Tana(DakotaBaseModel):
    """Generated model for Tana"""

    tana: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "multipoint_tana",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Qmea(DakotaBaseModel):
    """Generated model for Qmea"""

    qmea: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "multipoint_qmea",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class LocalConfig(DakotaBaseModel):
    """Generated model for LocalConfig"""

    taylor_series: bool = DakotaField(
        description="Construct a Taylor Series expansion around a point"
    )
    truth_model_pointer: str = DakotaField(
        description='Pointer to specify a "truth" model, from which to construct a surrogate',
        dakota={
            "block_pointer": "model",
            "aliases": ["actual_model_pointer"],
            "materialization": [
                {
                    "ir_key": "model.surrogate.truth_model_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class OrderedModelFidelitiesConfig(SurrogateCorrectionMixin):
    """Generated model for OrderedModelFidelitiesConfig"""

    pointers: list[str] = DakotaField(
        description="Specification of an hierarchy of model fidelities, ordered from low to high.",
        dakota={
            "block_pointer": "model",
            "materialization": [
                {
                    "ir_key": "model.surrogate.ensemble_model_pointers",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )


class TruthModelPointerConfig(DakotaBaseModel):
    """Generated model for TruthModelPointerConfig"""

    pointer: str = DakotaField(
        description='Pointer to specify a "truth" model, from which to construct a surrogate',
        dakota={
            "block_pointer": "model",
            "materialization": [
                {
                    "ir_key": "model.surrogate.truth_model_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    approximation_models: list[str] | None = DakotaField(
        default=None,
        description="Specification of an unordered ensemble of low-fidelity approximations",
        dakota={
            "block_pointer": "model",
            "aliases": ["unordered_model_fidelities"],
            "materialization": [
                {
                    "ir_key": "model.surrogate.ensemble_model_pointers",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )


class OptionalInterfacePointer(DakotaBaseModel):
    """Generated model for OptionalInterfacePointer"""

    pointer: str = DakotaField(
        description="Pointer to interface that provides non-nested responses",
        dakota={
            "block_pointer": "interface",
            "materialization": [
                {
                    "ir_key": "model.interface_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    optional_interface_responses_pointer: str | None = DakotaField(
        default=None,
        description="Pointer to responses block that defines non-nested responses",
        dakota={
            "block_pointer": "responses",
            "materialization": [
                {
                    "ir_key": "model.optional_interface_responses_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class SubMethodPointerIteratorSchedulingDedicated(DakotaBaseModel):
    """Generated model for SubMethodPointerIteratorSchedulingDedicated"""

    dedicated: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.nested.iterator_scheduling",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DEDICATED_SCHEDULER_DYNAMIC",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class SubMethodPointerIteratorSchedulingPeer(DakotaBaseModel):
    """Generated model for SubMethodPointerIteratorSchedulingPeer"""

    peer: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.nested.iterator_scheduling",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "PEER_SCHEDULING",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ActiveSubspaceSampleTypeLhs(DakotaBaseModel):
    """Generated model for ActiveSubspaceSampleTypeLhs"""

    lhs: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.active_subspace.sample_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_LHS",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ActiveSubspaceSampleTypeRandom(DakotaBaseModel):
    """Generated model for ActiveSubspaceSampleTypeRandom"""

    random: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.active_subspace.sample_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_RANDOM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Energy(DakotaBaseModel):
    """Generated model for Energy"""

    truncation_tolerance: DakotaFloat = DakotaField(
        default=1.0e-6,
        description="Specify the maximum percentage (as a decimal) of the eigenvalue energy not captured by the active subspace representation.",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.active_subspace.truncation_method.energy.truncation_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
                {
                    "ir_key": "model.truncation_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
            ]
        },
    )


class Minimum(DakotaBaseModel):
    """Generated model for Minimum"""

    minimum: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.active_subspace.cv.id_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MINIMUM_METRIC",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class CVRelative(DakotaBaseModel):
    """Generated model for CVRelative"""

    relative: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.active_subspace.cv.id_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RELATIVE_TOLERANCE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Decrease(DakotaBaseModel):
    """Generated model for Decrease"""

    decrease: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.active_subspace.cv.id_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DECREASE_TOLERANCE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BuildSurrogate(DakotaBaseModel):
    """Generated model for BuildSurrogate"""

    refinement_samples: list[int] | None = DakotaField(
        default=None,
        description="Number of supplementary surrogate build samples",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.refinement_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )


class MeanValue(DakotaBaseModel):
    """Generated model for MeanValue"""

    mean_value: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.active_subspace.normalization",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBSPACE_NORM_MEAN_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MeanGradient(DakotaBaseModel):
    """Generated model for MeanGradient"""

    mean_gradient: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.active_subspace.normalization",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBSPACE_NORM_MEAN_GRAD",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class LocalGradient(DakotaBaseModel):
    """Generated model for LocalGradient"""

    local_gradient: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.active_subspace.normalization",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBSPACE_NORM_LOCAL_GRAD",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class TruthModelPointerSGLevel(DakotaBaseModel):
    """Generated model for TruthModelPointerSGLevel"""

    sparse_grid_level: int = DakotaField(
        description="Level to use in sparse grid integration or interpolation",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.adapted_basis.sparse_grid_level",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class TruthModelPointerExpansionOrderConfig(DakotaBaseModel):
    """Generated model for TruthModelPointerExpansionOrderConfig"""

    order: int = DakotaField(
        description="The (initial) order of a polynomial expansion",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.adapted_basis.expansion_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    collocation_ratio: DakotaFloat = DakotaField(
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.adapted_basis.collocation_ratio",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class Unranked(DakotaBaseModel):
    """Generated model for Unranked"""

    unranked: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.adapted_basis.rotation_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ROTATION_METHOD_UNRANKED",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class Ranked(DakotaBaseModel):
    """Generated model for Ranked"""

    ranked: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.adapted_basis.rotation_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ROTATION_METHOD_RANKED",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class RfDataFile(DakotaBaseModel):
    """Generated model for RfDataFile"""

    rf_data_file: str = DakotaField(
        description="Specify that the random field will be built from a file of data. THIS IS AN EXPERIMENTAL CAPABILITY.",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.rf_data_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class BuildSourceDaceMethodPointer(DakotaBaseModel):
    """Generated model for BuildSourceDaceMethodPointer"""

    dace_method_pointer: str = DakotaField(
        description="Pointer to a DACE method for purposes of generating an ensemble of field responses to be used in estimating a random field model. THIS IS AN EXPERIMENTAL CAPABILITY.",
        dakota={
            "block_pointer": "method",
            "materialization": [
                {
                    "ir_key": "model.dace_method_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                },
                {
                    "ir_key": "model.nested.sub_method_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                },
            ],
        },
    )


class SquaredExponential(DakotaBaseModel):
    """Generated model for SquaredExponential"""

    squared_exponential: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.rf.analytic_covariance",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "EXP_L2",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Exponential(DakotaBaseModel):
    """Generated model for Exponential"""

    exponential: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.rf.analytic_covariance",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "EXP_L1",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class KarhunenLoeve(DakotaBaseModel):
    """Generated model for KarhunenLoeve"""

    karhunen_loeve: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.rf.expansion_form",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RF_KARHUNEN_LOEVE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExpansionFormPrincipalComponents(DakotaBaseModel):
    """Generated model for ExpansionFormPrincipalComponents"""

    principal_components: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "model.rf.expansion_form",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RF_PCA_GP",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class SingleConfig(ModelFourOptionalKeywordsMixin):
    """Generated model for SingleConfig"""

    interface_pointer: str | None = DakotaField(
        default=None,
        description="Interface block pointer for the single model type",
        dakota={
            "block_pointer": "interface",
            "materialization": [
                {
                    "ir_key": "model.interface_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    solution_level_control: str | None = DakotaField(
        default=None,
        description="Cost estimates associated with a set of solution control values.",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.simulation.solution_level_control",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    cost_model: Union[SolutionLevelCost, CostRecoveryMetadata] | None = DakotaField(
        default=None,
        description="Cost model for 1 or more resolution levels",
        dakota={"anchor": True, "union_pattern": 2},
    )


class ExperimentalGPConfig(
    ModelVarianceExportMixin,
    ModelPartialSurrogateExportFormatMixin,
    ModelSurrogateImportMixin,
):
    """Generated model for ExperimentalGPConfig"""

    trend: Union[
        TrendNone,
        ExperimentalGPTrendConstant,
        ExperimentalGPTrendLinear,
        ExperimentalGPTrendReducedQuadratic,
        ExperimentalGPTrendQuadratic,
    ] = DakotaField(
        default_factory=ExperimentalGPTrendReducedQuadratic,
        description="This keyword enables the use of deterministic polynomial trend function",
        dakota={
            "union_pattern": 1,
            "model_default": "ExperimentalGPTrendReducedQuadratic",
        },
    )
    num_restarts: int = DakotaField(
        default=10,
        gt=1,
        description="Number of optimization restarts for L-BFGS-B",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.num_restarts",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    nugget: Union[ExperimentalGPNugget, ExperimentalGPFindNugget] | None = DakotaField(
        default=None, description="Nugget", dakota={"anchor": True, "union_pattern": 2}
    )
    options_file: str | None = DakotaField(
        default=None,
        description="Filename for a YAML file that specifies Gaussian process options",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.advanced_options_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class GPDakotaConfig(DakotaBaseModel):
    """Generated model for GPDakotaConfig"""

    point_selection: Literal[True] | None = DakotaField(
        default=None,
        description="Enable greedy selection of well-spaced build points",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.point_selection",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    trend: Union[
        DakotaTrendConstant, DakotaTrendLinear, DakotaTrendReducedQuadratic
    ] = DakotaField(
        default_factory=DakotaTrendReducedQuadratic,
        description="Choose a trend function for a Gaussian process surrogate",
        dakota={"union_pattern": 1, "model_default": "DakotaTrendReducedQuadratic"},
    )


class GlobalApproxGPSurfpackConfig(
    ModelFullSurfpackExportFormatMixin, ModelSurrogateImportMixin
):
    """Generated model for GlobalApproxGPSurfpackConfig"""

    trend: Union[
        SurfpackTrendConstant,
        SurfpackTrendLinear,
        SurfpackTrendReducedQuadratic,
        SurfpackTrendQuadratic,
    ] = DakotaField(
        default_factory=SurfpackTrendReducedQuadratic,
        description="Choose a trend function for a Gaussian process surrogate",
        dakota={"union_pattern": 1, "model_default": "SurfpackTrendReducedQuadratic"},
    )
    optimization_method: str | None = DakotaField(
        default=None,
        description="Change the optimization method used to compute hyperparameters",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.kriging_opt_method",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    max_trials: int | None = DakotaField(
        default=None,
        gt=0,
        description="Max number of likelihood function evaluations",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.kriging_max_trials",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "short",
                }
            ]
        },
    )
    nugget: Union[SurfpackNugget, SurfpackFindNugget] | None = DakotaField(
        default=None, description="Nugget", dakota={"anchor": True, "union_pattern": 2}
    )
    correlation_lengths: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Specify the correlation lengths for the Gaussian process",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.kriging_correlations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class MarsConfig(ModelPartialSurrogateExportFormatMixin, ModelSurrogateImportMixin):
    """Generated model for MarsConfig"""

    max_bases: int = DakotaField(
        default=0,
        description="Maximum number of MARS bases",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.mars_max_bases",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "short",
                }
            ]
        },
    )
    interpolation: Union[InterpLinear, InterpCubic] | None = DakotaField(
        default=None,
        description="MARS model interpolation type",
        dakota={"union_pattern": 2},
    )


class MovingLeastSquares(DakotaBaseModel):
    """Generated model for MovingLeastSquares"""

    moving_least_squares: MovingLeastSquaresConfig = DakotaField(
        description="Moving Least Squares surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "global_moving_least_squares",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Ft(DakotaBaseModel):
    """Generated model for Ft"""

    function_train: ModelSurrogateGlobalApproxFtConfig = DakotaField(
        description="Global surrogate model based on functional tensor train decomposition",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "global_function_train",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class NeuralNetwork(DakotaBaseModel):
    """Generated model for NeuralNetwork"""

    neural_network: NeuralNetworkConfig = DakotaField(
        description="Artificial neural network model",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "global_neural_network",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class RadialBasis(DakotaBaseModel):
    """Generated model for RadialBasis"""

    radial_basis: RadialBasisConfig = DakotaField(
        description="Radial basis function (RBF) model",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "global_radial_basis",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class GlobalApproxPolynomialConfig(
    ModelFullSurfpackExportFormatMixin, ModelSurrogateImportMixin
):
    """Generated model for GlobalApproxPolynomialConfig"""

    order: Union[BasisOrder, PolynomialLinear, PolynomialQuadratic, PolynomialCubic] = (
        DakotaField(
            description="Polynomial Order", dakota={"anchor": True, "union_pattern": 4}
        )
    )


class ExperimentalPolynomial(DakotaBaseModel):
    """Generated model for ExperimentalPolynomial"""

    experimental_polynomial: ExperimentalPolynomialConfig = DakotaField(
        description="Use a deterministic polynomial surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "global_exp_poly",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ExperimentalPython(DakotaBaseModel):
    """Generated model for ExperimentalPython"""

    experimental_python: ExperimentalPythonConfig = DakotaField(
        description="Use the experimental python surrogates interface",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "global_exp_python",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class DomainDecomposition(DakotaBaseModel):
    """Generated model for DomainDecomposition"""

    cell_type: str = DakotaField(
        default="voronoi",
        description="Type of the Geometric Cells Used for the Piecewise Decomposition Option of Global Surrogates",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.decomp_cell_type",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    support_layers: int = DakotaField(
        default=0,
        description="Optional Number of Support Layers for the Piecewise Decomposition Option of Global Surrogates",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.decomp_support_layers",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    discontinuity_detection: Union[JumpThreshold, GradientThreshold] | None = (
        DakotaField(
            default=None,
            description="Optional Discontinuity Detection Capability for the Piecewise Decomposition Option of Global Surrogates",
            dakota={
                "union_pattern": 2,
                "materialization": [
                    {
                        "ir_key": "model.surrogate.decomp_discont_detect",
                        "storage_type": "PRESENCE_TRUE",
                        "ir_value_type": "bool",
                    }
                ],
            },
        )
    )


class AutoRefinement(DakotaBaseModel):
    """Generated model for AutoRefinement"""

    max_iterations: int = DakotaField(
        default=SZ_MAX,
        gt=0,
        description="Number of iterations allowed for optimizers and adaptive UQ methods",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.max_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    max_function_evaluations: int = DakotaField(
        default=SZ_MAX,
        gt=0,
        description="Number of function evaluations allowed for optimizers",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.max_function_evals",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    convergence_tolerance: DakotaFloat = DakotaField(
        default=1.0e-4,
        description="Cross-validation threshold for surrogate convergence",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.convergence_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    soft_convergence_limit: int = DakotaField(
        default=0,
        ge=0,
        description="Maximum number of iterations without improvement in cross-validation",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.soft_convergence_limit",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    cross_validation_metric: CVMetric | None = DakotaField(
        default=None,
        description="Choice of error metric to satisfy",
        dakota={"argument": "metric"},
    )


class ReusePoints(DakotaBaseModel):
    """Generated model for ReusePoints"""

    reuse_domain: Union[ReusePointsAll, Region, ReusePointsNone] = DakotaField(
        description="Reuse Domain", dakota={"anchor": True, "union_pattern": 4}
    )


class GlobalApproxImportBuildPointsFileCustomAnnotatedConfig(DakotaBaseModel):
    """Generated model for GlobalApproxImportBuildPointsFileCustomAnnotatedConfig"""

    header: ImportBuildPointsFileCustomAnnotatedHeader | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.import_build_format",
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
                    "ir_key": "model.surrogate.import_build_format",
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
                    "ir_key": "model.surrogate.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class GlobalApproxImportBuildPointsFileAnnotated(DakotaBaseModel):
    """Generated model for GlobalApproxImportBuildPointsFileAnnotated"""

    annotated: ImportBuildPointsFileAnnotatedConfig = DakotaField(
        default_factory=ImportBuildPointsFileAnnotatedConfig,
        description="Selects annotated tabular file format",
        dakota={
            "model_default": "ImportBuildPointsFileAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "model.surrogate.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class GlobalApproxExportApproxPointsFileCustomAnnotated(DakotaBaseModel):
    """Generated model for GlobalApproxExportApproxPointsFileCustomAnnotated"""

    custom_annotated: GlobalApproxExportApproxPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=GlobalApproxExportApproxPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "GlobalApproxExportApproxPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "model.surrogate.export_approx_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class Metrics(DakotaBaseModel):
    """Generated model for Metrics"""

    metrics: list[str] = DakotaField(
        description="Compute surrogate quality metrics",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.diagnostics",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                },
                {
                    "ir_key": "model.metrics",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                },
            ]
        },
    )
    cross_validation: Union[Folds, Percent, dict] | None = DakotaField(
        default=None,
        description="Perform k-fold cross validation",
        dakota={
            "union_pattern": 5,
            "materialization": [
                {
                    "ir_key": "model.surrogate.cross_validate",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ],
        },
    )
    press: Literal[True] | None = DakotaField(
        default=None,
        description="Leave-one-out cross validation",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.press",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class ImportChallengePointsFileCustomAnnotatedConfig(DakotaBaseModel):
    """Generated model for ImportChallengePointsFileCustomAnnotatedConfig"""

    header: ImportChallengePointsFileCustomAnnotatedHeader | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.challenge_points_file_format",
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
                    "ir_key": "model.surrogate.challenge_points_file_format",
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
                    "ir_key": "model.surrogate.challenge_points_file_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ImportChallengePointsFileAnnotated(DakotaBaseModel):
    """Generated model for ImportChallengePointsFileAnnotated"""

    annotated: ImportChallengePointsFileAnnotatedConfig = DakotaField(
        default_factory=ImportChallengePointsFileAnnotatedConfig,
        description="Selects annotated tabular file format",
        dakota={
            "model_default": "ImportChallengePointsFileAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "model.surrogate.challenge_points_file_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class MultipointConfig(DakotaBaseModel):
    """Generated model for MultipointConfig"""

    type: Union[Tana, Qmea] = DakotaField(
        description="Multipoint Surrogate", dakota={"anchor": True, "union_pattern": 4}
    )
    truth_model_pointer: str = DakotaField(
        description='Pointer to specify a "truth" model, from which to construct a surrogate',
        dakota={
            "block_pointer": "model",
            "aliases": ["actual_model_pointer"],
            "materialization": [
                {
                    "ir_key": "model.surrogate.truth_model_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class Local(DakotaBaseModel):
    """Generated model for Local"""

    local: LocalConfig = DakotaField(
        description="Build a locally accurate surrogate from data at a single point",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "local_taylor",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class OrderedModelFidelities(DakotaBaseModel):
    """Generated model for OrderedModelFidelities"""

    ordered_model_fidelities: OrderedModelFidelitiesConfig = DakotaField(
        default=...,
        description="Specification of an hierarchy of model fidelities, ordered from low to high.",
        dakota={"argument": "pointers", "aliases": ["model_fidelity_sequence"]},
    )


class EnsembleTruthModelPointer(DakotaBaseModel):
    """Generated model for EnsembleTruthModelPointer"""

    truth_model_pointer: TruthModelPointerConfig = DakotaField(
        default=...,
        description='Pointer to specify a "truth" model, from which to construct a surrogate',
        dakota={"argument": "pointer", "aliases": ["actual_model_pointer"]},
    )


class SubMethodPointer(DakotaBaseModel):
    """Generated model for SubMethodPointer"""

    pointer: str = DakotaField(
        description="The ``sub_method_pointer`` specifies the method block for the sub-iterator",
        dakota={
            "block_pointer": "method",
            "materialization": [
                {
                    "ir_key": "model.dace_method_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                },
                {
                    "ir_key": "model.nested.sub_method_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                },
            ],
        },
    )
    iterator_servers: int | None = DakotaField(
        default=None,
        gt=0,
        description="Specify the number of iterator servers when Dakota is run in parallel",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.nested.iterator_servers",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    iterator_scheduling: (
        Union[
            SubMethodPointerIteratorSchedulingDedicated,
            SubMethodPointerIteratorSchedulingPeer,
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
                    "ir_key": "model.nested.processors_per_iterator",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    primary_variable_mapping: list[str] | None = DakotaField(
        default=None,
        description="Primary mappning of top-level variables to sub-model variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.nested.primary_variable_mapping",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )
    secondary_variable_mapping: list[str] | None = DakotaField(
        default=None,
        description="Secondary mappning of top-level variables to sub-model variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.nested.secondary_variable_mapping",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )
    primary_response_mapping: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Mapping of sub-method results to top-level primary responses",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.nested.primary_response_mapping",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    secondary_response_mapping: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="Mapping of sub-method results to top-level secondary responses",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.nested.secondary_response_mapping",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    identity_response_mapping: Literal[True] | None = DakotaField(
        default=None,
        description="Identity mapping of sub-method results to top-level responses",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.nested.identity_resp_map",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class TruncationMethodCV(DakotaBaseModel):
    """Generated model for TruncationMethodCV"""

    criterion: Union[Minimum, CVRelative, Decrease] | None = DakotaField(
        default=None,
        description="CV Selection Criterion",
        dakota={"anchor": True, "union_pattern": 2},
    )
    relative_tolerance: DakotaFloat = DakotaField(
        default=1.0e-6,
        description="Tolerance for cross-validation error value",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.active_subspace.cv.relative_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    decrease_tolerance: DakotaFloat = DakotaField(
        default=1.0e-6,
        description="Tolerance for cross-validation error stabilization",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.active_subspace.cv.decrease_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    max_rank: int = DakotaField(
        default=-1,
        description="Maximum subspace dimension to consider",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.active_subspace.cv.max_rank",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    exhaustive: Literal[True] | None = DakotaField(
        default=None,
        description="Assess all admissible subspace dimensions",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.active_subspace.cv.incremental",
                    "storage_type": "PRESENCE_FALSE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class TruthModelPointerExpansionOrder(DakotaBaseModel):
    """Generated model for TruthModelPointerExpansionOrder"""

    expansion_order: TruthModelPointerExpansionOrderConfig = DakotaField(
        default=...,
        description="The (initial) order of a polynomial expansion",
        dakota={"argument": "order"},
    )


class AnalyticCov(DakotaBaseModel):
    """Generated model for AnalyticCov"""

    analytic_covariance: Union[SquaredExponential, Exponential]


class SingleSelection(ModelSelection):
    """Generated model for SingleSelection"""

    single: SingleConfig = DakotaField(
        description="A model with one of each block: variable, interface, and response",
        dakota={
            "aliases": ["simulation"],
            "materialization": [
                {
                    "ir_key": "model.type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "simulation",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class ExperimentalGP(DakotaBaseModel):
    """Generated model for ExperimentalGP"""

    experimental_gaussian_process: ExperimentalGPConfig = DakotaField(
        description="Use the Gaussian process regression surrogate from the surrogates module",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "global_exp_gauss_proc",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class GlobalApproxGPDakota(DakotaBaseModel):
    """Generated model for GlobalApproxGPDakota"""

    dakota: GPDakotaConfig = DakotaField(
        description="Select the built in Gaussian Process surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "global_gaussian",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class GlobalApproxGPSurfpack(DakotaBaseModel):
    """Generated model for GlobalApproxGPSurfpack"""

    surfpack: GlobalApproxGPSurfpackConfig = DakotaField(
        description="Use the Surfpack version of Gaussian Process surrogates",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "global_kriging",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Mars(DakotaBaseModel):
    """Generated model for Mars"""

    mars: MarsConfig = DakotaField(
        description="Multivariate Adaptive Regression Spline (MARS)",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "global_mars",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class GlobalApproxPolynomial(DakotaBaseModel):
    """Generated model for GlobalApproxPolynomial"""

    polynomial: GlobalApproxPolynomialConfig = DakotaField(
        description="Polynomial surrogate model",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "global_polynomial",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class DaceMethodPointerConfig(DakotaBaseModel):
    """Generated model for DaceMethodPointerConfig"""

    pointer: str = DakotaField(
        description="Specify a method to gather training data",
        dakota={
            "block_pointer": "method",
            "materialization": [
                {
                    "ir_key": "model.dace_method_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                },
                {
                    "ir_key": "model.nested.sub_method_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                },
            ],
        },
    )
    auto_refinement: AutoRefinement | None = DakotaField(
        default=None,
        description="Experimental auto-refinement of surrogate model",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.auto_refine",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class GlobalApproxImportBuildPointsFileCustomAnnotated(DakotaBaseModel):
    """Generated model for GlobalApproxImportBuildPointsFileCustomAnnotated"""

    custom_annotated: GlobalApproxImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=GlobalApproxImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "GlobalApproxImportBuildPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "model.surrogate.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class GlobalApproxExportApproxPointsFile(DakotaBaseModel):
    """Generated model for GlobalApproxExportApproxPointsFile"""

    filename: str = DakotaField(
        description="Output file for surrogate model value evaluations",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.export_approx_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        GlobalApproxExportApproxPointsFileCustomAnnotated,
        GlobalApproxExportApproxPointsFileAnnotated,
        GlobalApproxExportApproxPointsFileFreeform,
    ] = DakotaField(
        default_factory=GlobalApproxExportApproxPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "GlobalApproxExportApproxPointsFileAnnotated",
        },
    )


class ImportChallengePointsFileCustomAnnotated(DakotaBaseModel):
    """Generated model for ImportChallengePointsFileCustomAnnotated"""

    custom_annotated: ImportChallengePointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=ImportChallengePointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "ImportChallengePointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "model.surrogate.challenge_points_file_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class Multipoint(DakotaBaseModel):
    """Generated model for Multipoint"""

    multipoint: MultipointConfig = DakotaField(
        description="Construct a surrogate from multiple existing training points"
    )


class Ensemble(DakotaBaseModel):
    """Generated model for Ensemble"""

    ensemble: Union[OrderedModelFidelities, EnsembleTruthModelPointer] = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "ensemble",
                    "ir_value_type": "String",
                }
            ]
        }
    )


class NestedConfig(ModelFourOptionalKeywordsMixin):
    """Generated model for NestedConfig"""

    optional_interface_pointer: OptionalInterfacePointer | None = DakotaField(
        default=None,
        description="Pointer to interface that provides non-nested responses",
        dakota={"argument": "pointer"},
    )
    sub_method_pointer: SubMethodPointer = DakotaField(
        default=...,
        description="The ``sub_method_pointer`` specifies the method block for the sub-iterator",
        dakota={"argument": "pointer"},
    )


class TruncationMethod(DakotaBaseModel):
    """Generated model for TruncationMethod"""

    bing_li: Literal[True] | None = DakotaField(
        default=None,
        description='Use the Bing Li "ladle" diagnostic to truncate subspace',
        dakota={
            "materialization": [
                {
                    "ir_key": "model.active_subspace.truncation_method.bing_li",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    constantine: Literal[True] | None = DakotaField(
        default=None,
        description="Use the Constantine diagnostic to truncate subspace",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.active_subspace.truncation_method.constantine",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    energy: Energy | None = DakotaField(
        default=None,
        description="Truncate the subspace based on eigenvalue energy",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.active_subspace.truncation_method.energy",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    cross_validation: TruncationMethodCV | None = DakotaField(
        default=None,
        description="Truncate the subspace to minimize surrogate cross-validation error",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.active_subspace.truncation_method.cv",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class AdaptedBasisTruthModelPointer(DakotaBaseModel):
    """Generated model for AdaptedBasisTruthModelPointer"""

    pointer: str = DakotaField(
        description='Pointer to specify a "truth" model, from which to construct a surrogate',
        dakota={
            "block_pointer": "model",
            "materialization": [
                {
                    "ir_key": "model.surrogate.truth_model_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    basis_control: Union[TruthModelPointerSGLevel, TruthModelPointerExpansionOrder] = (
        DakotaField(
            description="Expansion Basis Control",
            dakota={"anchor": True, "union_pattern": 4},
        )
    )
    dimension: int = DakotaField(
        default=0,
        description="Prescribe the number of active dimensions",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.rf.expansion_bases",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                },
                {
                    "ir_key": "model.subspace.dimension",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                },
            ]
        },
    )
    rotation_method: Union[Unranked, Ranked] = DakotaField(
        default_factory=Ranked,
        description="Method used to build the rotation matrix",
        dakota={"union_pattern": 1, "model_default": "Ranked"},
    )


class RandomFieldConfig(ModelFourOptionalKeywordsMixin):
    """Generated model for RandomFieldConfig"""

    build_source: (
        Union[RfDataFile, BuildSourceDaceMethodPointer, AnalyticCov] | None
    ) = DakotaField(
        default=None,
        description="Specify how the random field will be built: from a data file, from simulation runs, or from a covariance matrix. THIS IS AN EXPERIMENTAL CAPABILITY.",
        dakota={"union_pattern": 2},
    )
    expansion_form: Union[KarhunenLoeve, ExpansionFormPrincipalComponents] | None = (
        DakotaField(
            default=None,
            description="Specify the form of the expansion to be used in the random field representation. THIS IS AN EXPERIMENTAL CAPABILITY.",
            dakota={"union_pattern": 2},
        )
    )
    expansion_bases: int = DakotaField(
        default=0,
        description="Specify the number of basis functions to be used in the random field representation. THIS IS AN EXPERIMENTAL CAPABILITY.",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.rf.expansion_bases",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                },
                {
                    "ir_key": "model.subspace.dimension",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                },
            ]
        },
    )
    truncation_tolerance: DakotaFloat = DakotaField(
        default=1.0e-6,
        description="Specify a percent of the response variance that should be captured with the random field representation. THIS IS AN EXPERIMENTAL CAPABILITY.",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.active_subspace.truncation_method.energy.truncation_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
                {
                    "ir_key": "model.truncation_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
            ]
        },
    )
    propagation_model_pointer: str = DakotaField(
        description="Pointer to the model that will accept realizations of the random field and use them for subsequent analysis. Typcially, this model will take the random field as inputs, e.g. a random field defining a pressure boundary or temperature boundary condition over a structure. THIS IS AN EXPERIMENTAL CAPABILITY.",
        dakota={
            "block_pointer": "model",
            "materialization": [
                {
                    "ir_key": "model.rf.propagation_model_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class GlobalApproxGPConfig(ModelVarianceExportMixin):
    """Generated model for GlobalApproxGPConfig"""

    gp_implementation: Union[GlobalApproxGPDakota, GlobalApproxGPSurfpack] | None = (
        DakotaField(
            default=None,
            description="GP Implementation",
            dakota={"anchor": True, "union_pattern": 2},
        )
    )


class GlobalApproxDaceMethodPointer(DakotaBaseModel):
    """Generated model for GlobalApproxDaceMethodPointer"""

    dace_method_pointer: DaceMethodPointerConfig = DakotaField(
        default=...,
        description="Specify a method to gather training data",
        dakota={"argument": "pointer"},
    )


class GlobalApproxImportBuildPointsFile(DakotaBaseModel):
    """Generated model for GlobalApproxImportBuildPointsFile"""

    filename: str = DakotaField(
        description="File containing points you wish to use to build a surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.import_build_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        GlobalApproxImportBuildPointsFileCustomAnnotated,
        GlobalApproxImportBuildPointsFileAnnotated,
        GlobalApproxImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=GlobalApproxImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "GlobalApproxImportBuildPointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.import_build_active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class ImportChallengePointsFile(DakotaBaseModel):
    """Generated model for ImportChallengePointsFile"""

    filename: str = DakotaField(
        description="Datafile of points to assess surrogate quality",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.challenge_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        ImportChallengePointsFileCustomAnnotated,
        ImportChallengePointsFileAnnotated,
        ImportChallengePointsFileFreeform,
    ] = DakotaField(
        default_factory=ImportChallengePointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "ImportChallengePointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.challenge_points_file_active",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class NestedSelection(ModelSelection):
    """Generated model for NestedSelection"""

    nested: NestedConfig = DakotaField(
        description="A model whose responses are computed through the use of a sub-iterator",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "nested",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ActiveSubspaceConfig(ModelFourOptionalKeywordsMixin):
    """Generated model for ActiveSubspaceConfig"""

    truth_model_pointer: str = DakotaField(
        description="Pointer to specify a full-space model, from which to construct a lower dimensional surrogate",
        dakota={
            "block_pointer": "model",
            "aliases": ["actual_model_pointer"],
            "materialization": [
                {
                    "ir_key": "model.surrogate.truth_model_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    initial_samples: int = DakotaField(
        default=0,
        description="Initial number of samples for sampling-based methods",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.initial_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    sample_type: (
        Union[ActiveSubspaceSampleTypeLhs, ActiveSubspaceSampleTypeRandom] | None
    ) = DakotaField(
        default=None,
        description="Selection of sampling strategy",
        dakota={"union_pattern": 2},
    )
    truncation_method: TruncationMethod | None = DakotaField(
        default=None, description="Metric that estimates active subspace size"
    )
    dimension: int = DakotaField(
        default=0,
        description="Explicitly specify the desired subspace size",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.rf.expansion_bases",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                },
                {
                    "ir_key": "model.subspace.dimension",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                },
            ]
        },
    )
    bootstrap_samples: int = DakotaField(
        default=100,
        description="Number of bootstrap replicates used in truncation metrics",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.active_subspace.bootstrap_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    build_surrogate: BuildSurrogate | None = DakotaField(
        default=None,
        description="Construct moving least squares surrogate over active subspace",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.active_subspace.build_surrogate",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    normalization: Union[MeanValue, MeanGradient, LocalGradient] | None = DakotaField(
        default=None,
        description="Normalize gradient samples",
        dakota={"union_pattern": 2},
    )


class AdaptedBasisConfig(ModelFourOptionalKeywordsMixin):
    """Generated model for AdaptedBasisConfig"""

    truth_model_pointer: AdaptedBasisTruthModelPointer = DakotaField(
        default=...,
        description='Pointer to specify a "truth" model, from which to construct a surrogate',
        dakota={"argument": "pointer", "aliases": ["actual_model_pointer"]},
    )
    truncation_tolerance: DakotaFloat = DakotaField(
        default=0.9,
        description="Convergence tolerance used to identify the (reduced) dimension of the rotation matrix",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.adapted_basis.truncation_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class RandomFieldSelection(ModelSelection):
    """Generated model for RandomFieldSelection"""

    random_field: RandomFieldConfig = DakotaField(
        description="Experimental capability to generate a random field representation. from data, from simulation runs, or from a covariance matrix.  The representation may then be sampled for use as a random field input to another simulation.  THIS IS AN EXPERIMENTAL CAPABILITY.",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "random_field",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class GlobalApproxGP(DakotaBaseModel):
    """Generated model for GlobalApproxGP"""

    gaussian_process: GlobalApproxGPConfig = DakotaField(
        description="Gaussian Process surrogate model", dakota={"aliases": ["kriging"]}
    )


class ActiveSubspaceSelection(ModelSelection):
    """Generated model for ActiveSubspaceSelection"""

    active_subspace: ActiveSubspaceConfig = DakotaField(
        description="Active (variable) subspace model",
        dakota={
            "aliases": ["subspace"],
            "materialization": [
                {
                    "ir_key": "model.type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "active_subspace",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class AdaptedBasisSelection(ModelSelection):
    """Generated model for AdaptedBasisSelection"""

    adapted_basis: AdaptedBasisConfig = DakotaField(
        description="Basis adaptation model",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "adapted_basis",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class GlobalApproxConfig(SurrogateCorrectionMixin):
    """Generated model for GlobalApproxConfig"""

    type: Union[
        ExperimentalGP,
        GlobalApproxGP,
        Mars,
        MovingLeastSquares,
        Ft,
        NeuralNetwork,
        RadialBasis,
        GlobalApproxPolynomial,
        ExperimentalPolynomial,
        ExperimentalPython,
    ] = DakotaField(
        description="Global Surrogate Type", dakota={"anchor": True, "union_pattern": 4}
    )
    domain_decomposition: DomainDecomposition | None = DakotaField(
        default=None,
        description="Piecewise Domain Decomposition for Global Surrogate Models",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.domain_decomp",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    build_points: Union[TotalPoints, MinimumPoints, RecommendedPoints] | None = (
        DakotaField(
            default=None,
            description="Number of Build Points",
            dakota={"anchor": True, "union_pattern": 2},
        )
    )
    build_data: (
        Union[GlobalApproxDaceMethodPointer, GlobalApproxTruthModelPointer] | None
    ) = DakotaField(
        default=None,
        description="Build Data Source",
        dakota={"anchor": True, "union_pattern": 2},
    )
    reuse_points: ReusePoints | None = DakotaField(
        default=None,
        description="Surrogate model training data reuse control",
        dakota={"aliases": ["reuse_samples"]},
    )
    import_build_points_file: GlobalApproxImportBuildPointsFile | None = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={
            "argument": "filename",
            "aliases": ["import_points_file", "samples_file"],
        },
    )
    export_approx_points_file: GlobalApproxExportApproxPointsFile | None = DakotaField(
        default=None,
        description="Output file for surrogate model value evaluations",
        dakota={"argument": "filename", "aliases": ["export_points_file"]},
    )
    use_derivatives: Literal[True] | None = DakotaField(
        default=None,
        description="Use derivative data to construct surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.derivative_usage",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    metrics: Metrics | None = DakotaField(
        default=None,
        description="Compute surrogate quality metrics",
        dakota={"argument": "metrics", "aliases": ["diagnostics"]},
    )
    import_challenge_points_file: ImportChallengePointsFile | None = DakotaField(
        default=None,
        description="Datafile of points to assess surrogate quality",
        dakota={"argument": "filename", "aliases": ["challenge_points_file"]},
    )


class GlobalApprox(DakotaBaseModel):
    """Generated model for GlobalApprox"""

    global_approx: GlobalApproxConfig = DakotaField(
        description="Select a surrogate model with global support"
    )


class SurrogateConfig(ModelFourOptionalKeywordsMixin):
    """Generated model for SurrogateConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckPositiveList(context="surrogateconfig", list_field="id_surrogates"),
    ]

    id_surrogates: list[int] | None = DakotaField(
        default=None,
        description="Identifies the subset of the response functions by number that are to be approximated (the default is all functions).",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.function_indices",
                    "storage_type": "ID_TO_INDEX_SET",
                    "ir_value_type": "SizetSet",
                }
            ]
        },
    )
    category: Union[GlobalApprox, Multipoint, Local, Ensemble] = DakotaField(
        description="Surrogate Category", dakota={"anchor": True, "union_pattern": 4}
    )


class SurrogateSelection(ModelSelection):
    """Generated model for SurrogateSelection"""

    surrogate: SurrogateConfig = DakotaField(
        description="An empirical model that is created from data or the results of a submodel",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "surrogate",
                    "ir_value_type": "String",
                }
            ]
        },
    )


# Auto-generated Union of all registered model selections
# Must be at end of module after all ModelSelection subclasses are defined
ModelUnion = ModelSelection.get_union()
