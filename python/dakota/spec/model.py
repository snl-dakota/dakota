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
    "Generated model for ModelConfig"

    model: ModelUnion | None = DakotaField(default=None, dakota={"union_pattern": 2})


class SolutionLevelCost(DakotaBaseModel):
    "Cost estimates associated with a set of solution control values."

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
    "Identify metadata by label for capturing online cost estimates"

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
    "No trend function"

    none: Literal[True] = DakotaField(
        default=True,
        description="No trend function",
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
    "Constant trend function"

    constant: Literal[True] = DakotaField(
        default=True,
        description="Constant trend function",
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
    "Use a linear polynomial or trend function"

    linear: Literal[True] = DakotaField(
        default=True,
        description="Use a linear polynomial or trend function",
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
    "Quadratic polynomials - main effects only"

    reduced_quadratic: Literal[True] = DakotaField(
        default=True,
        description="Quadratic polynomials - main effects only",
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
    "Use a quadratic polynomial or trend function"

    quadratic: Literal[True] = DakotaField(
        default=True,
        description="Use a quadratic polynomial or trend function",
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
    "Value for the fixed nugget parameter"

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
    "Use regression to estimate the nugget."

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
    "Constant trend function"

    constant: Literal[True] = DakotaField(
        default=True,
        description="Constant trend function",
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
    "Use a linear polynomial or trend function"

    linear: Literal[True] = DakotaField(
        default=True,
        description="Use a linear polynomial or trend function",
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
    "Quadratic polynomials - main effects only"

    reduced_quadratic: Literal[True] = DakotaField(
        default=True,
        description="Quadratic polynomials - main effects only",
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
    "Constant trend function"

    constant: Literal[True] = DakotaField(
        default=True,
        description="Constant trend function",
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
    "Use a linear polynomial or trend function"

    linear: Literal[True] = DakotaField(
        default=True,
        description="Use a linear polynomial or trend function",
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
    "Quadratic polynomials - main effects only"

    reduced_quadratic: Literal[True] = DakotaField(
        default=True,
        description="Quadratic polynomials - main effects only",
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
    "Use a quadratic polynomial or trend function"

    quadratic: Literal[True] = DakotaField(
        default=True,
        description="Use a quadratic polynomial or trend function",
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
    "Specify a nugget to handle ill-conditioning"

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
    "Have Surfpack compute a nugget to handle ill-conditioning"

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
    "Linear interpolation"

    linear: Literal[True] = DakotaField(
        default=True,
        description="Linear interpolation",
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
    "Cubic interpolation"

    cubic: Literal[True] = DakotaField(
        default=True,
        description="Cubic interpolation",
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
    "Moving Least Squares surrogate models"

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
    "Global surrogate model based on functional tensor train decomposition"

    pass


class NeuralNetworkConfig(
    ModelFullSurfpackExportFormatMixin, ModelSurrogateImportMixin
):
    "Artificial neural network model"

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
    "Radial basis function (RBF) model"

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
    "Polynomial order"

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
    "Use a linear polynomial or trend function"

    linear: Literal[True] = DakotaField(
        default=True,
        description="Use a linear polynomial or trend function",
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
    "Use a quadratic polynomial or trend function"

    quadratic: Literal[True] = DakotaField(
        default=True,
        description="Use a quadratic polynomial or trend function",
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
    "Use a cubic polynomial"

    cubic: Literal[True] = DakotaField(
        default=True,
        description="Use a cubic polynomial",
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
    "Use a deterministic polynomial surrogate"

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
    "Use the experimental python surrogates interface"

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
    "Gradient Threshold Parameter of the Optional Discontinuity Detection Capability for the Piecewise Decomposition Option of Global Surrogates"

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
    "Gradient Threshold Parameter of the Optional Discontinuity Detection Capability for the Piecewise Decomposition Option of Global Surrogates"

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
    "Specified number of training points"

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
    "Construct surrogate with minimum number of points"

    minimum_points: Literal[True] = DakotaField(
        default=True,
        description="Construct surrogate with minimum number of points",
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
    "Construct surrogate with recommended number of points"

    recommended_points: Literal[True] = DakotaField(
        default=True,
        description="Construct surrogate with recommended number of points",
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
    "Choice of error metric to satisfy"

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
    "A surrogate model pointer that guides a method to whether it should use a surrogate model or compute truth function evaluations"

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
    "Option for ``reuse_points``"

    all: Literal[True] = DakotaField(
        default=True,
        description="Option for ``reuse_points``",
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
    "Option for ``reuse_points``"

    region: Literal[True] = DakotaField(
        default=True,
        description="Option for ``reuse_points``",
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
    "Option for ``reuse_points``"

    none: Literal[True] = DakotaField(
        default=True,
        description="Option for ``reuse_points``",
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
    "Enable header row in custom-annotated tabular file"

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
    "Selects annotated tabular file format"

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
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
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
    "Selects custom-annotated tabular file format"

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
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
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
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
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
    "Number of cross validation folds"

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
    "Percent data per cross validation fold"

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
    "Enable header row in custom-annotated tabular file"

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
    "Selects annotated tabular file format"

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
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
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
    "Local multi-point model via two-point nonlinear approximation"

    tana: Literal[True] = DakotaField(
        default=True,
        description="Local multi-point model via two-point nonlinear approximation",
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
    "Multi-point surrogate approximation based on QMEA algorithm"

    qmea: Literal[True] = DakotaField(
        default=True,
        description="Multi-point surrogate approximation based on QMEA algorithm",
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
    "Build a locally accurate surrogate from data at a single point"

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
    "Specification of an hierarchy of model fidelities, ordered from low to high."

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
    'Pointer to specify a \\"truth\\" model, from which to construct a surrogate'

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
    "Pointer to interface that provides non-nested responses"

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
    "Specify a dedicated scheduler partition for concurrent iterator executions"

    dedicated: Literal[True] = DakotaField(
        default=True,
        description="Specify a dedicated scheduler partition for concurrent iterator executions",
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
    "Specify a peer partition for parallel iterator scheduling"

    peer: Literal[True] = DakotaField(
        default=True,
        description="Specify a peer partition for parallel iterator scheduling",
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
    "Uses Latin Hypercube Sampling (LHS) to sample variables"

    lhs: Literal[True] = DakotaField(
        default=True,
        description="Uses Latin Hypercube Sampling (LHS) to sample variables",
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
    "Uses purely random Monte Carlo sampling to sample variables"

    random: Literal[True] = DakotaField(
        default=True,
        description="Uses purely random Monte Carlo sampling to sample variables",
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
    "Truncate the subspace based on eigenvalue energy"

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
    "Select subspace to minimize cross-validation error"

    minimum: Literal[True] = DakotaField(
        default=True,
        description="Select subspace to minimize cross-validation error",
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
    "Choose subspace with cross-validation error less than tolerance"

    relative: Literal[True] = DakotaField(
        default=True,
        description="Choose subspace with cross-validation error less than tolerance",
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
    "Choose subspace where cross-validation error stabilizes"

    decrease: Literal[True] = DakotaField(
        default=True,
        description="Choose subspace where cross-validation error stabilizes",
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
    "Construct moving least squares surrogate over active subspace"

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
    "Normalize by sample mean of function values"

    mean_value: Literal[True] = DakotaField(
        default=True,
        description="Normalize by sample mean of function values",
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
    "Normalize by sample mean of gradient norms"

    mean_gradient: Literal[True] = DakotaField(
        default=True,
        description="Normalize by sample mean of gradient norms",
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
    "Normalize each gradient sample by its norm"

    local_gradient: Literal[True] = DakotaField(
        default=True,
        description="Normalize each gradient sample by its norm",
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
    "Level to use in sparse grid integration or interpolation"

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
    "The (initial) order of a polynomial expansion"

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
    "Use the unranked method to obtain the rotation matrix"

    unranked: Literal[True] = DakotaField(
        default=True,
        description="Use the unranked method to obtain the rotation matrix",
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
    "Use the ranked method to obtain the rotation matrix"

    ranked: Literal[True] = DakotaField(
        default=True,
        description="Use the ranked method to obtain the rotation matrix",
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
    "Specify that the random field will be built from a file of data. THIS IS AN EXPERIMENTAL CAPABILITY."

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
    "Pointer to a DACE method for purposes of generating an ensemble of field responses to be used in estimating a random field model. THIS IS AN EXPERIMENTAL CAPABILITY."

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
    "Specify a squared exponential covariance in the case where the random field is built from an analytic covariance functio. THIS IS AN EXPERIMENTAL CAPABILITY."

    squared_exponential: Literal[True] = DakotaField(
        default=True,
        description="Specify a squared exponential covariance in the case where the random field is built from an analytic covariance functio. THIS IS AN EXPERIMENTAL CAPABILITY.",
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
    "Specify an exponential covariance in the case where the random field is built from an analytic covariance functio. THIS IS AN EXPERIMENTAL CAPABILITY."

    exponential: Literal[True] = DakotaField(
        default=True,
        description="Specify an exponential covariance in the case where the random field is built from an analytic covariance functio. THIS IS AN EXPERIMENTAL CAPABILITY.",
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
    "Specify Karhunen-Loeve as the expansion form to be used in the random field representation. THIS IS AN EXPERIMENTAL CAPABILITY."

    karhunen_loeve: Literal[True] = DakotaField(
        default=True,
        description="Specify Karhunen-Loeve as the expansion form to be used in the random field representation. THIS IS AN EXPERIMENTAL CAPABILITY.",
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
    "Specify Principal Components as the form of the expansion to be used in the random field representation. THIS IS AN EXPERIMENTAL CAPABILITY."

    principal_components: Literal[True] = DakotaField(
        default=True,
        description="Specify Principal Components as the form of the expansion to be used in the random field representation. THIS IS AN EXPERIMENTAL CAPABILITY.",
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
    "A model with one of each block: variable, interface, and response"

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
    "Use the Gaussian process regression surrogate from the surrogates module"

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
    "Select the built in Gaussian Process surrogate"

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
    "Use the Surfpack version of Gaussian Process surrogates"

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
    "Multivariate Adaptive Regression Spline (MARS)"

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
    "Moving Least Squares surrogate models"

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
    "Global surrogate model based on functional tensor train decomposition"

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
    "Artificial neural network model"

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
    "Radial basis function (RBF) model"

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
    "Polynomial surrogate model"

    order: Union[BasisOrder, PolynomialLinear, PolynomialQuadratic, PolynomialCubic] = (
        DakotaField(
            description="Polynomial Order", dakota={"anchor": True, "union_pattern": 4}
        )
    )


class ExperimentalPolynomial(DakotaBaseModel):
    "Use a deterministic polynomial surrogate"

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
    "Use the experimental python surrogates interface"

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
    "Piecewise Domain Decomposition for Global Surrogate Models"

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
    "Experimental auto-refinement of surrogate model"

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
    "Surrogate model training data reuse control"

    reuse_domain: Union[ReusePointsAll, Region, ReusePointsNone] = DakotaField(
        description="Reuse Domain", dakota={"anchor": True, "union_pattern": 4}
    )


class GlobalApproxImportBuildPointsFileCustomAnnotatedConfig(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

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
    "Selects annotated tabular file format"

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
    "Selects custom-annotated tabular file format"

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
    "Compute surrogate quality metrics"

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
    "Selects custom-annotated tabular file format"

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
    "Selects annotated tabular file format"

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
    "Construct a surrogate from multiple existing training points"

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
    "Build a locally accurate surrogate from data at a single point"

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
    "Specification of an hierarchy of model fidelities, ordered from low to high."

    ordered_model_fidelities: OrderedModelFidelitiesConfig = DakotaField(
        default=...,
        description="Specification of an hierarchy of model fidelities, ordered from low to high.",
        dakota={"argument": "pointers", "aliases": ["model_fidelity_sequence"]},
    )


class EnsembleTruthModelPointer(DakotaBaseModel):
    'Pointer to specify a \\"truth\\" model, from which to construct a surrogate'

    truth_model_pointer: TruthModelPointerConfig = DakotaField(
        default=...,
        description='Pointer to specify a "truth" model, from which to construct a surrogate',
        dakota={"argument": "pointer", "aliases": ["actual_model_pointer"]},
    )


class SubMethodPointer(DakotaBaseModel):
    "The ``sub_method_pointer`` specifies the method block for the sub-iterator"

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
    "Truncate the subspace to minimize surrogate cross-validation error"

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
    "The (initial) order of a polynomial expansion"

    expansion_order: TruthModelPointerExpansionOrderConfig = DakotaField(
        default=...,
        description="The (initial) order of a polynomial expansion",
        dakota={"argument": "order"},
    )


class AnalyticCov(DakotaBaseModel):
    "Use an analytic covariance function for the purposes of generating a random field model. THIS IS AN EXPERIMENTAL CAPABILITY."

    analytic_covariance: Union[SquaredExponential, Exponential] = DakotaField(
        description="Use an analytic covariance function for the purposes of generating a random field model. THIS IS AN EXPERIMENTAL CAPABILITY."
    )


class SingleSelection(ModelSelection):
    "A model with one of each block: variable, interface, and response"

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
    "Use the Gaussian process regression surrogate from the surrogates module"

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
    "Select the built in Gaussian Process surrogate"

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
    "Use the Surfpack version of Gaussian Process surrogates"

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
    "Multivariate Adaptive Regression Spline (MARS)"

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
    "Polynomial surrogate model"

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
    "Specify a method to gather training data"

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
    "Selects custom-annotated tabular file format"

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
    "Output file for surrogate model value evaluations"

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
    "Selects custom-annotated tabular file format"

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
    "Construct a surrogate from multiple existing training points"

    multipoint: MultipointConfig = DakotaField(
        description="Construct a surrogate from multiple existing training points"
    )


class Ensemble(DakotaBaseModel):
    "Ensemble surrogates employ a collection of lower-fidelity models to approximate a truth reference model at reduced cost."

    ensemble: Union[OrderedModelFidelities, EnsembleTruthModelPointer] = DakotaField(
        description="Ensemble surrogates employ a collection of lower-fidelity models to approximate a truth reference model at reduced cost.",
        dakota={
            "materialization": [
                {
                    "ir_key": "model.surrogate.type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "ensemble",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class NestedConfig(ModelFourOptionalKeywordsMixin):
    "A model whose responses are computed through the use of a sub-iterator"

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
    "Metric that estimates active subspace size"

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
    'Pointer to specify a \\"truth\\" model, from which to construct a surrogate'

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
    "Experimental capability to generate a random field representation. from data, from simulation runs, or from a covariance matrix.  The representation may then be sampled for use as a random field input to another simulation.  THIS IS AN EXPERIMENTAL CAPABILITY."

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
    "Gaussian Process surrogate model"

    gp_implementation: Union[GlobalApproxGPDakota, GlobalApproxGPSurfpack] | None = (
        DakotaField(
            default=None,
            description="GP Implementation",
            dakota={"anchor": True, "union_pattern": 2},
        )
    )


class GlobalApproxDaceMethodPointer(DakotaBaseModel):
    "Specify a method to gather training data"

    dace_method_pointer: DaceMethodPointerConfig = DakotaField(
        default=...,
        description="Specify a method to gather training data",
        dakota={"argument": "pointer"},
    )


class GlobalApproxImportBuildPointsFile(DakotaBaseModel):
    "File containing points you wish to use to build a surrogate"

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
    "Datafile of points to assess surrogate quality"

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
    "A model whose responses are computed through the use of a sub-iterator"

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
    "Active (variable) subspace model"

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
    "Basis adaptation model"

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
    "Experimental capability to generate a random field representation. from data, from simulation runs, or from a covariance matrix.  The representation may then be sampled for use as a random field input to another simulation.  THIS IS AN EXPERIMENTAL CAPABILITY."

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
    "Gaussian Process surrogate model"

    gaussian_process: GlobalApproxGPConfig = DakotaField(
        description="Gaussian Process surrogate model", dakota={"aliases": ["kriging"]}
    )


class ActiveSubspaceSelection(ModelSelection):
    "Active (variable) subspace model"

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
    "Basis adaptation model"

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
    "Select a surrogate model with global support"

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
    "Select a surrogate model with global support"

    global_approx: GlobalApproxConfig = DakotaField(
        description="Select a surrogate model with global support"
    )


class SurrogateConfig(ModelFourOptionalKeywordsMixin):
    "An empirical model that is created from data or the results of a submodel"

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
    "An empirical model that is created from data or the results of a submodel"

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
