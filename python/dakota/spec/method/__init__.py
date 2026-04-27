"""Method configuration models"""

from .base import MethodSelection

from .adaptive_sampling import AdaptiveSamplingSelection, AdaptiveSamplingConfig
from .approximate_control_variate import (
    ApproximateControlVariateSelection,
    ApproximateControlVariateConfig,
)
from .asynch_pattern_search import (
    AsynchPatternSearchSelection,
    AsynchPatternSearchConfig,
)
from .bayes_calibration import BayesCalibrationSelection, BayesCalibrationConfig
from .branch_and_bound import BranchAndBoundSelection, BranchAndBoundConfig
from .centered_parameter_study import (
    CenteredParameterStudySelection,
    CenteredParameterStudyConfig,
)
from .coliny_beta import ColinyBetaSelection, ColinyBetaConfig
from .coliny_cobyla import ColinyCobylaSelection, ColinyCobylaConfig
from .coliny_direct import ColinyDirectSelection, ColinyDirectConfig
from .coliny_ea import ColinyEaSelection, ColinyEaConfig
from .coliny_pattern_search import (
    ColinyPatternSearchSelection,
    ColinyPatternSearchConfig,
)
from .coliny_solis_wets import ColinySolisWetsSelection, ColinySolisWetsConfig
from .conmin_frcg import ConminFrcgSelection, ConminFrcgConfig
from .conmin_mfd import ConminMfdSelection, ConminMfdConfig
from .dace import DaceSelection, DaceConfig
from .demo_tpl import DemoTplSelection, DemoTplConfig
from .dl_solver import DlSolverSelection, DlSolverConfig
from .dot_bfgs import DotBfgsSelection, DotBfgsConfig
from .dot_frcg import DotFrcgSelection, DotFrcgConfig
from .dot_mmfd import DotMmfdSelection, DotMmfdConfig
from .dot_slp import DotSlpSelection, DotSlpConfig
from .dot_sqp import DotSqpSelection, DotSqpConfig
from .efficient_global import EfficientGlobalSelection, EfficientGlobalConfig
from .external_python import ExternalPythonSelection, ExternalPythonConfig
from .fsu_cvt import FsuCvtSelection, FsuCvtConfig
from .fsu_quasi_mc import FsuQuasiMcSelection, FsuQuasiMcConfig
from .function_train import FtSelection, FtConfig
from .genie_direct import GenieDirectSelection, GenieDirectConfig
from .genie_opt_darts import GenieOptDartsSelection, GenieOptDartsConfig
from .global_evidence import GlobalEvidenceSelection, GlobalEvidenceConfig
from .global_interval_est import GlobalIntervalEstSelection, GlobalIntervalEstConfig
from .global_reliability import GlobalReliabilitySelection, GlobalReliabilityConfig
from .gpais import GpaisSelection, GpaisConfig
from .hybrid import HybridSelection, HybridConfig
from .import_points import ImportPointsSelection, ImportPointsConfig
from .importance_sampling import ImportanceSamplingSelection, ImportanceSamplingConfig
from .list_parameter_study import ListParameterStudySelection, ListParameterStudyConfig
from .local_evidence import LocalEvidenceSelection, LocalEvidenceConfig
from .local_interval_est import LocalIntervalEstSelection, LocalIntervalEstConfig
from .local_reliability import LocalReliabilitySelection, LocalReliabilityConfig
from .mesh_adaptive_search import MeshAdaptiveSearchSelection, MeshAdaptiveSearchConfig
from .moga import MogaSelection, MogaConfig
from .multi_start import MultiStartSelection, MultiStartConfig
from .multidim_parameter_study import (
    MultidimParameterStudySelection,
    MultidimParameterStudyConfig,
)
from .multifidelity_function_train import (
    MultifidelityFtSelection,
    MultifidelityFtConfig,
)
from .multifidelity_polynomial_chaos import MfPceSelection, MfPceConfig
from .multifidelity_sampling import (
    MultifidelitySamplingSelection,
    MultifidelitySamplingConfig,
)
from .multifidelity_stoch_collocation import (
    MultifidelityStochCollocSelection,
    MultifidelityStochCollocConfig,
)
from .multilevel_blue import MultilevelBlueSelection, MultilevelBlueConfig
from .multilevel_function_train import MultilevelFtSelection, MultilevelFtConfig
from .multilevel_multifidelity_sampling import (
    MultilevelMultifidelitySamplingSelection,
    MultilevelMultifidelitySamplingConfig,
)
from .multilevel_polynomial_chaos import MlPceSelection, MlPceConfig
from .multilevel_sampling import MultilevelSamplingSelection, MultilevelSamplingConfig
from .ncsu_direct import NcsuDirectSelection, NcsuDirectConfig
from .nl2sol import Nl2solSelection, Nl2solConfig
from .nlpql_sqp import NlpqlSqpSelection, NlpqlSqpConfig
from .nlssol_sqp import NlssolSqpSelection, NlssolSqpConfig
from .nonlinear_cg import NonlinearCgSelection, NonlinearCgConfig
from .nowpac import NowpacSelection, NowpacConfig
from .npsol_sqp import NpsolSqpSelection, NpsolSqpConfig
from .optpp_cg import OptppCgSelection, OptppCgConfig
from .optpp_fd_newton import OptppFdNewtonSelection, OptppFdNewtonConfig
from .optpp_g_newton import OptppGNewtonSelection, OptppGNewtonConfig
from .optpp_newton import OptppNewtonSelection, OptppNewtonConfig
from .optpp_pds import OptppPdsSelection, OptppPdsConfig
from .optpp_q_newton import OptppQNewtonSelection, OptppQNewtonConfig
from .pareto_set import ParetoSetSelection, ParetoSetConfig
from .pof_darts import PofDartsSelection, PofDartsConfig
from .polynomial_chaos import PceSelection, PceConfig
from .psuade_moat import PsuadeMoatSelection, PsuadeMoatConfig
from .richardson_extrap import RichardsonExtrapSelection, RichardsonExtrapConfig
from .rkd_darts import RkdDartsSelection, RkdDartsConfig
from .rol import RolSelection, RolConfig
from .sampling import SamplingSelection, SamplingConfig
from .snowpac import SnowpacSelection, SnowpacConfig
from .soga import SogaSelection, SogaConfig
from .stoch_collocation import StochCollocSelection, StochCollocConfig
from .surrogate_based_global import (
    SurrogateBasedGlobalSelection,
    SurrogateBasedGlobalConfig,
)
from .surrogate_based_local import (
    SurrogateBasedLocalSelection,
    SurrogateBasedLocalConfig,
)
from .surrogate_based_uq import SurrogateBasedUqSelection, SurrogateBasedUqConfig
from .vector_parameter_study import (
    VectorParameterStudySelection,
    VectorParameterStudyConfig,
)


# Auto-generated Union of all registered method selections
MethodUnion = MethodSelection.get_union()


__all__ = [
    "MethodConfig",
    "MethodSelection",
    "MethodUnion",
    "AdaptiveSamplingConfig",
    "AdaptiveSamplingSelection",
    "ApproximateControlVariateConfig",
    "ApproximateControlVariateSelection",
    "AsynchPatternSearchConfig",
    "AsynchPatternSearchSelection",
    "BayesCalibrationConfig",
    "BayesCalibrationSelection",
    "BranchAndBoundConfig",
    "BranchAndBoundSelection",
    "CenteredParameterStudyConfig",
    "CenteredParameterStudySelection",
    "ColinyBetaConfig",
    "ColinyBetaSelection",
    "ColinyCobylaConfig",
    "ColinyCobylaSelection",
    "ColinyDirectConfig",
    "ColinyDirectSelection",
    "ColinyEaConfig",
    "ColinyEaSelection",
    "ColinyPatternSearchConfig",
    "ColinyPatternSearchSelection",
    "ColinySolisWetsConfig",
    "ColinySolisWetsSelection",
    "ConminFrcgConfig",
    "ConminFrcgSelection",
    "ConminMfdConfig",
    "ConminMfdSelection",
    "DaceConfig",
    "DaceSelection",
    "DemoTplConfig",
    "DemoTplSelection",
    "DlSolverConfig",
    "DlSolverSelection",
    "DotBfgsConfig",
    "DotBfgsSelection",
    "DotFrcgConfig",
    "DotFrcgSelection",
    "DotMmfdConfig",
    "DotMmfdSelection",
    "DotSlpConfig",
    "DotSlpSelection",
    "DotSqpConfig",
    "DotSqpSelection",
    "EfficientGlobalConfig",
    "EfficientGlobalSelection",
    "ExternalPythonConfig",
    "ExternalPythonSelection",
    "FsuCvtConfig",
    "FsuCvtSelection",
    "FsuQuasiMcConfig",
    "FsuQuasiMcSelection",
    "FtConfig",
    "FtSelection",
    "GenieDirectConfig",
    "GenieDirectSelection",
    "GenieOptDartsConfig",
    "GenieOptDartsSelection",
    "GlobalEvidenceConfig",
    "GlobalEvidenceSelection",
    "GlobalIntervalEstConfig",
    "GlobalIntervalEstSelection",
    "GlobalReliabilityConfig",
    "GlobalReliabilitySelection",
    "GpaisConfig",
    "GpaisSelection",
    "HybridConfig",
    "HybridSelection",
    "ImportPointsConfig",
    "ImportPointsSelection",
    "ImportanceSamplingConfig",
    "ImportanceSamplingSelection",
    "ListParameterStudyConfig",
    "ListParameterStudySelection",
    "LocalEvidenceConfig",
    "LocalEvidenceSelection",
    "LocalIntervalEstConfig",
    "LocalIntervalEstSelection",
    "LocalReliabilityConfig",
    "LocalReliabilitySelection",
    "MeshAdaptiveSearchConfig",
    "MeshAdaptiveSearchSelection",
    "MfPceConfig",
    "MfPceSelection",
    "MlPceConfig",
    "MlPceSelection",
    "MogaConfig",
    "MogaSelection",
    "MultiStartConfig",
    "MultiStartSelection",
    "MultidimParameterStudyConfig",
    "MultidimParameterStudySelection",
    "MultifidelityFtConfig",
    "MultifidelityFtSelection",
    "MultifidelitySamplingConfig",
    "MultifidelitySamplingSelection",
    "MultifidelityStochCollocConfig",
    "MultifidelityStochCollocSelection",
    "MultilevelBlueConfig",
    "MultilevelBlueSelection",
    "MultilevelFtConfig",
    "MultilevelFtSelection",
    "MultilevelMultifidelitySamplingConfig",
    "MultilevelMultifidelitySamplingSelection",
    "MultilevelSamplingConfig",
    "MultilevelSamplingSelection",
    "NcsuDirectConfig",
    "NcsuDirectSelection",
    "Nl2solConfig",
    "Nl2solSelection",
    "NlpqlSqpConfig",
    "NlpqlSqpSelection",
    "NlssolSqpConfig",
    "NlssolSqpSelection",
    "NonlinearCgConfig",
    "NonlinearCgSelection",
    "NowpacConfig",
    "NowpacSelection",
    "NpsolSqpConfig",
    "NpsolSqpSelection",
    "OptppCgConfig",
    "OptppCgSelection",
    "OptppFdNewtonConfig",
    "OptppFdNewtonSelection",
    "OptppGNewtonConfig",
    "OptppGNewtonSelection",
    "OptppNewtonConfig",
    "OptppNewtonSelection",
    "OptppPdsConfig",
    "OptppPdsSelection",
    "OptppQNewtonConfig",
    "OptppQNewtonSelection",
    "ParetoSetConfig",
    "ParetoSetSelection",
    "PceConfig",
    "PceSelection",
    "PofDartsConfig",
    "PofDartsSelection",
    "PsuadeMoatConfig",
    "PsuadeMoatSelection",
    "RichardsonExtrapConfig",
    "RichardsonExtrapSelection",
    "RkdDartsConfig",
    "RkdDartsSelection",
    "RolConfig",
    "RolSelection",
    "SamplingConfig",
    "SamplingSelection",
    "SnowpacConfig",
    "SnowpacSelection",
    "SogaConfig",
    "SogaSelection",
    "StochCollocConfig",
    "StochCollocSelection",
    "SurrogateBasedGlobalConfig",
    "SurrogateBasedGlobalSelection",
    "SurrogateBasedLocalConfig",
    "SurrogateBasedLocalSelection",
    "SurrogateBasedUqConfig",
    "SurrogateBasedUqSelection",
    "VectorParameterStudyConfig",
    "VectorParameterStudySelection",
]
