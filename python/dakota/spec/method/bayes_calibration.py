"""Generated Pydantic models for method.bayes_calibration"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField, DakotaFloat
from .base import MethodSelection
from typing import Literal, Union

# Cross-module model imports
from dakota.spec.shared.bayesian import (
    BayesEmulatorMixin,
    BayesProposalCovMixin,
    BayesSeedRngMixin,
)
from dakota.spec.shared.core import MethodConvergenceTolMixin
from dakota.spec.shared.formats import MethodImportBuildFormatMixin
from dakota.spec.shared.misc import (
    DefaultScalingMixin,
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
    ProbabilityLevelsContext1Mixin,
)
from dakota.spec.shared.sampling import MethodExportSamplesFormatMixin


class QuesoExportChainPointsFile(MethodExportSamplesFormatMixin):
    """Generated model for QuesoExportChainPointsFile"""

    filename: str = DakotaField(
        description="Export the MCMC chain to the specified filename",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_mcmc_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class QuesoDram(DakotaBaseModel):
    """Generated model for QuesoDram"""

    dram: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.mcmc_type",
                    "stored_value": "dram",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class QuesoDelayedRejection(DakotaBaseModel):
    """Generated model for QuesoDelayedRejection"""

    delayed_rejection: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.mcmc_type",
                    "stored_value": "delayed_rejection",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class QuesoAdaptiveMetropolis(DakotaBaseModel):
    """Generated model for QuesoAdaptiveMetropolis"""

    adaptive_metropolis: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.mcmc_type",
                    "stored_value": "adaptive_metropolis",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class QuesoMetropolisHastings(DakotaBaseModel):
    """Generated model for QuesoMetropolisHastings"""

    metropolis_hastings: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.mcmc_type",
                    "stored_value": "metropolis_hastings",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Multilevel(DakotaBaseModel):
    """Generated model for Multilevel"""

    multilevel: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.mcmc_type",
                    "stored_value": "multilevel",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class QuesoPreSolveSqp(DakotaBaseModel):
    """Generated model for QuesoPreSolveSqp"""

    sqp: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "stored_value": "SUBMETHOD_NPSOL",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class QuesoPreSolveNip(DakotaBaseModel):
    """Generated model for QuesoPreSolveNip"""

    nip: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "stored_value": "SUBMETHOD_OPTPP",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class QuesoPreSolveNone(DakotaBaseModel):
    """Generated model for QuesoPreSolveNone"""

    none: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "stored_value": "SUBMETHOD_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class GpmsaImportBuildPointsFile(MethodImportBuildFormatMixin):
    """Generated model for GpmsaImportBuildPointsFile"""

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


class GpmsaExportChainPointsFile(MethodExportSamplesFormatMixin):
    """Generated model for GpmsaExportChainPointsFile"""

    filename: str = DakotaField(
        description="Export the MCMC chain to the specified filename",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_mcmc_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class GpmsaDram(DakotaBaseModel):
    """Generated model for GpmsaDram"""

    dram: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.mcmc_type",
                    "stored_value": "dram",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class GpmsaDelayedRejection(DakotaBaseModel):
    """Generated model for GpmsaDelayedRejection"""

    delayed_rejection: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.mcmc_type",
                    "stored_value": "delayed_rejection",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class GpmsaAdaptiveMetropolis(DakotaBaseModel):
    """Generated model for GpmsaAdaptiveMetropolis"""

    adaptive_metropolis: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.mcmc_type",
                    "stored_value": "adaptive_metropolis",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class GpmsaMetropolisHastings(DakotaBaseModel):
    """Generated model for GpmsaMetropolisHastings"""

    metropolis_hastings: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.mcmc_type",
                    "stored_value": "metropolis_hastings",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class CovDiag(DakotaBaseModel):
    """Generated model for CovDiag"""

    diagonal: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.data_dist_cov_type",
                    "stored_value": "diagonal",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class CovMatrix(DakotaBaseModel):
    """Generated model for CovMatrix"""

    matrix: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.data_dist_cov_type",
                    "stored_value": "matrix",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ObsDataFilename(DakotaBaseModel):
    """Generated model for ObsDataFilename"""

    obs_data_filename: str = DakotaField(
        description="(Experimental Capability) Filename from which to read experimental data",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.data_dist_filename",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class GeneratePosteriorSamples(DakotaBaseModel):
    """Generated model for GeneratePosteriorSamples"""

    posterior_samples_export_filename: str | None = DakotaField(
        default=None,
        description="(Experimental Capability) Filename for the exported posterior samples",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.posterior_samples_export_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class EvaluatePosteriorDensity(DakotaBaseModel):
    """Generated model for EvaluatePosteriorDensity"""

    posterior_density_export_filename: str | None = DakotaField(
        default=None,
        description="(Experimental Capability) Filename for the exported posterior density",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.posterior_density_export_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class DreamExportChainPointsFile(MethodExportSamplesFormatMixin):
    """Generated model for DreamExportChainPointsFile"""

    filename: str = DakotaField(
        description="Export the MCMC chain to the specified filename",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_mcmc_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MuqExportChainPointsFile(MethodExportSamplesFormatMixin):
    """Generated model for MuqExportChainPointsFile"""

    filename: str = DakotaField(
        description="Export the MCMC chain to the specified filename",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_mcmc_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class AdaptiveMetropolisConfig(DakotaBaseModel):
    """Generated model for AdaptiveMetropolisConfig"""

    period_num_steps: int = DakotaField(
        default=100,
        description="Number of steps between updates of the proposal covariance",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.am_period_num_steps",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    starting_step: int = DakotaField(
        default=100,
        description="Number of steps prior to start of proposal covariance adaptation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.am_starting_step",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    adapt_scale: DakotaFloat = DakotaField(
        default=1.0,
        description="Sample covariance scaling used to define proposal covariance",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.am_scale",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class DelayedRejectionConfig(DakotaBaseModel):
    """Generated model for DelayedRejectionConfig"""

    num_stages: int = DakotaField(
        default=3,
        description="Number of stages",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dr_num_stages",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    scale_type: str = DakotaField(
        default="Power",
        description="Type of scaling to use",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dr_scale_type",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    delay_scale: DakotaFloat = DakotaField(
        default=2.0,
        description="Scaling parameter",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dr_scale",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class DiliConfig(DakotaBaseModel):
    """Generated model for DiliConfig"""

    hessian_type: str = DakotaField(
        default="GaussNewton",
        description="Parameter for the DILI MCMC algorithm in MUQ",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dili_hessian_type",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    adapt_interval: int = DakotaField(
        default=-1,
        description="Parameter for the DILI MCMC algorithm in MUQ",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dili_adapt_interval",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    adapt_start: int = DakotaField(
        default=1,
        description="Parameter for the DILI MCMC algorithm in MUQ",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dili_adapt_start",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    adapt_end: int = DakotaField(
        default=-1,
        description="Parameter for the DILI MCMC algorithm in MUQ",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dili_adapt_end",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    initial_weight: int = DakotaField(
        default=100,
        description="Parameter for the DILI MCMC algorithm in MUQ",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dili_initial_weight",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    hess_tolerance: DakotaFloat = DakotaField(
        default=1.0e-4,
        description="Parameter for the DILI MCMC algorithm in MUQ",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dili_hess_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    lis_tolerance: DakotaFloat = DakotaField(
        default=0.1,
        description="Parameter for the DILI MCMC algorithm in MUQ",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dili_lis_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    ses_num_eigs: int = DakotaField(
        default=2,
        description="Parameter for the stochastic eigensolver used by the DILI MCMC algorithm in MUQ",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dili_ses_num_eigs",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    ses_rel_tol: DakotaFloat = DakotaField(
        default=0.001,
        description="Parameter for the stochastic eigensolver used by the DILI MCMC algorithm in MUQ",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dili_ses_rel_tol",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    ses_abs_tol: DakotaFloat = DakotaField(
        default=0.0,
        description="Parameter for the stochastic eigensolver used by the DILI MCMC algorithm in MUQ",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dili_ses_abs_tol",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    ses_exp_rank: int = DakotaField(
        default=2,
        description="Parameter for the stochastic eigensolver used by the DILI MCMC algorithm in MUQ",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dili_ses_exp_rank",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    ses_overs_factor: int = DakotaField(
        default=2,
        description="Parameter for the stochastic eigensolver used by the DILI MCMC algorithm in MUQ",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dili_ses_overs_factor",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    ses_block_size: int = DakotaField(
        default=2,
        description="Parameter for the stochastic eigensolver used by the DILI MCMC algorithm in MUQ",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dili_ses_block_size",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )


class DramConfig(DakotaBaseModel):
    """Generated model for DramConfig"""

    num_stages: int = DakotaField(
        default=3,
        description="Number of stages",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dr_num_stages",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    scale_type: str = DakotaField(
        default="Power",
        description="Type of scaling to use",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dr_scale_type",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    delay_scale: DakotaFloat = DakotaField(
        default=2.0,
        description="Scaling parameter",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dr_scale",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    period_num_steps: int = DakotaField(
        default=100,
        description="Number of steps between updates of the proposal covariance",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.am_period_num_steps",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    starting_step: int = DakotaField(
        default=100,
        description="Number of steps prior to start of proposal covariance adaptation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.am_starting_step",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    adapt_scale: DakotaFloat = DakotaField(
        default=1.0,
        description="Sample covariance scaling used to define proposal covariance",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.am_scale",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class MultilevelMcmcConfig(DakotaBaseModel):
    """Generated model for MultilevelMcmcConfig"""

    initial_chain_samples: int = DakotaField(
        default=1000,
        description="Number of initial samples for each level",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.mlmcmc_initial_chain_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    target_variance: DakotaFloat = DakotaField(
        default=0.05,
        description="Target variance for the greedy algorithm",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.mlmcmc_target_variance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    greedy_resampling_factor: DakotaFloat = DakotaField(
        default=0.5,
        description="Greedy resampling factor",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.mlmcmc_greedy_resampling_factor",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    subsampling_steps: list[int] = DakotaField(
        default=[],
        description="Number of samples at each level",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.mlmcmc_subsampling_steps",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )


class MalaConfig(DakotaBaseModel):
    """Generated model for MalaConfig"""

    step_size: DakotaFloat = DakotaField(
        default=1.0,
        description="Parameter for the MALA MCMC algorithm in MUQ",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.mala_step_size",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class MuqMetropolisHastings(DakotaBaseModel):
    """Generated model for MuqMetropolisHastings"""

    metropolis_hastings: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.mcmc_type",
                    "stored_value": "metropolis_hastings",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MuqPreSolveSqp(DakotaBaseModel):
    """Generated model for MuqPreSolveSqp"""

    sqp: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "stored_value": "SUBMETHOD_NPSOL",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MuqPreSolveNip(DakotaBaseModel):
    """Generated model for MuqPreSolveNip"""

    nip: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "stored_value": "SUBMETHOD_OPTPP",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MuqPreSolveNone(DakotaBaseModel):
    """Generated model for MuqPreSolveNone"""

    none: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.opt_subproblem_solver",
                    "stored_value": "SUBMETHOD_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ImportCandidatePointsFileCustomAnnotatedConfig(DakotaBaseModel):
    """Generated model for ImportCandidatePointsFileCustomAnnotatedConfig"""

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_candidate_format",
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
                    "ir_key": "method.import_candidate_format",
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
                    "ir_key": "method.import_candidate_format",
                    "stored_value": "TABULAR_IFACE_ID",
                    "storage_type": "AUGMENT_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ImportCandidatePointsFileAnnotated(DakotaBaseModel):
    """Generated model for ImportCandidatePointsFileAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_candidate_format",
                    "stored_value": "TABULAR_ANNOTATED",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ImportCandidatePointsFileFreeform(DakotaBaseModel):
    """Generated model for ImportCandidatePointsFileFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_candidate_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class One(DakotaBaseModel):
    """Generated model for One"""

    one: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.calibrate_error_mode",
                    "stored_value": "CALIBRATE_ONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class PerExperiment(DakotaBaseModel):
    """Generated model for PerExperiment"""

    per_experiment: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.calibrate_error_mode",
                    "stored_value": "CALIBRATE_PER_EXPER",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class PerResponse(DakotaBaseModel):
    """Generated model for PerResponse"""

    per_response: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.calibrate_error_mode",
                    "stored_value": "CALIBRATE_PER_RESP",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Both(DakotaBaseModel):
    """Generated model for Both"""

    both: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.calibrate_error_mode",
                    "stored_value": "CALIBRATE_BOTH",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class HyperpriorParameters(DakotaBaseModel):
    """Generated model for HyperpriorParameters"""

    alphas: list[DakotaFloat] = DakotaField(
        description="Shape (alpha) of the inverse gamma hyper-parameter prior",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.hyperprior_alphas",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    betas: list[DakotaFloat] = DakotaField(
        description="Scale (beta) of the inverse gamma hyper-parameter prior",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.hyperprior_betas",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class MutualInfo(DakotaBaseModel):
    """Generated model for MutualInfo"""

    ksg2: Literal[True] | None = DakotaField(
        default=None,
        description="Use second Kraskov algorithm to compute mutual information",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.mutual_info_ksg2",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class ChainDiagnostics(DakotaBaseModel):
    """Generated model for ChainDiagnostics"""

    confidence_intervals: Literal[True] | None = DakotaField(
        default=None,
        description="Calculate the confidence intervals on estimates of first and second moments",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.chain_diagnostics.confidence_intervals",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class ModelEvidence(DakotaBaseModel):
    """Generated model for ModelEvidence"""

    mc_approx: Literal[True] | None = DakotaField(
        default=None,
        description="Calculate model evidence using a Monte Carlo sampling approach",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.mc_approx",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    evidence_samples: int | None = DakotaField(
        default=None,
        description="The number of samples used in Monte Carlo approximation of the model evidence.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.evidence_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    laplace_approx: Literal[True] | None = DakotaField(
        default=None,
        description="Calculate model evidence using the Laplace approximation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.laplace_approx",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class DiscrepancyTypeGPConfig(DakotaBaseModel):
    """Generated model for DiscrepancyTypeGPConfig"""

    trend_order: int = DakotaField(
        default=2,
        ge=0,
        description="Trend function order of the model discrepancy",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.model_discrepancy.polynomial_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class DiscrepancyTypePolynomialConfig(DakotaBaseModel):
    """Generated model for DiscrepancyTypePolynomialConfig"""

    basis_order: int = DakotaField(
        default=2,
        ge=0,
        description="Basis function order of the model discrepancy",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.model_discrepancy.polynomial_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ImportPredictionConfigsCustomAnnotatedConfig(DakotaBaseModel):
    """Generated model for ImportPredictionConfigsCustomAnnotatedConfig"""

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_prediction_configs_format",
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
                    "ir_key": "method.import_prediction_configs_format",
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
                    "ir_key": "method.import_prediction_configs_format",
                    "stored_value": "TABULAR_IFACE_ID",
                    "storage_type": "AUGMENT_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ImportPredictionConfigsAnnotated(DakotaBaseModel):
    """Generated model for ImportPredictionConfigsAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_prediction_configs_format",
                    "stored_value": "TABULAR_ANNOTATED",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ImportPredictionConfigsFreeform(DakotaBaseModel):
    """Generated model for ImportPredictionConfigsFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_prediction_configs_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExportDiscrepancyFileCustomAnnotatedConfig(DakotaBaseModel):
    """Generated model for ExportDiscrepancyFileCustomAnnotatedConfig"""

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_discrep_format",
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
                    "ir_key": "method.nond.export_discrep_format",
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
                    "ir_key": "method.nond.export_discrep_format",
                    "stored_value": "TABULAR_IFACE_ID",
                    "storage_type": "AUGMENT_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExportDiscrepancyFileAnnotated(DakotaBaseModel):
    """Generated model for ExportDiscrepancyFileAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_discrep_format",
                    "stored_value": "TABULAR_ANNOTATED",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExportDiscrepancyFileFreeform(DakotaBaseModel):
    """Generated model for ExportDiscrepancyFileFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_discrep_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExportCorrectedModelFileCustomAnnotatedConfig(DakotaBaseModel):
    """Generated model for ExportCorrectedModelFileCustomAnnotatedConfig"""

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_corrected_model_format",
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
                    "ir_key": "method.nond.export_corrected_model_format",
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
                    "ir_key": "method.nond.export_corrected_model_format",
                    "stored_value": "TABULAR_IFACE_ID",
                    "storage_type": "AUGMENT_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExportCorrectedModelFileAnnotated(DakotaBaseModel):
    """Generated model for ExportCorrectedModelFileAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_corrected_model_format",
                    "stored_value": "TABULAR_ANNOTATED",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExportCorrectedModelFileFreeform(DakotaBaseModel):
    """Generated model for ExportCorrectedModelFileFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_corrected_model_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExportCorrectedVarianceFileCustomAnnotatedConfig(DakotaBaseModel):
    """Generated model for ExportCorrectedVarianceFileCustomAnnotatedConfig"""

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_corrected_variance_format",
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
                    "ir_key": "method.nond.export_corrected_variance_format",
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
                    "ir_key": "method.nond.export_corrected_variance_format",
                    "stored_value": "TABULAR_IFACE_ID",
                    "storage_type": "AUGMENT_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExportCorrectedVarianceFileAnnotated(DakotaBaseModel):
    """Generated model for ExportCorrectedVarianceFileAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_corrected_variance_format",
                    "stored_value": "TABULAR_ANNOTATED",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExportCorrectedVarianceFileFreeform(DakotaBaseModel):
    """Generated model for ExportCorrectedVarianceFileFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_corrected_variance_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class QuesoConfig(BayesSeedRngMixin, BayesEmulatorMixin, BayesProposalCovMixin):
    """Generated model for QuesoConfig"""

    chain_samples: int = DakotaField(
        description="Number of Markov Chain Monte Carlo posterior samples",
        dakota={
            "aliases": ["samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.chain_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ],
        },
    )
    standardized_space: Literal[True] | None = DakotaField(
        default=None,
        description="Perform Bayesian inference in standardized probability space",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.standardized_space",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    logit_transform: Literal[True] | None = DakotaField(
        default=None,
        description="Utilize the logit transformation to reduce sample rejection for bounded domains",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.logit_transform",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    export_chain_points_file: QuesoExportChainPointsFile | None = DakotaField(
        default=None,
        description="Export the MCMC chain to the specified filename",
        dakota={"argument": "filename"},
    )
    mcmc_algorithm: Union[
        QuesoDram,
        QuesoDelayedRejection,
        QuesoAdaptiveMetropolis,
        QuesoMetropolisHastings,
        Multilevel,
    ] = DakotaField(
        default_factory=QuesoDram,
        description="MCMC Algorithm",
        dakota={"anchor": True, "union_pattern": 1, "model_default": "QuesoDram"},
    )
    pre_solve: Union[QuesoPreSolveSqp, QuesoPreSolveNip, QuesoPreSolveNone] | None = (
        DakotaField(
            default=None,
            description="Perform deterministic optimization for MAP before Bayesian calibration",
            dakota={"union_pattern": 2},
        )
    )
    options_file: str | None = DakotaField(
        default=None,
        description="File containing advanced QUESO options",
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


class GpmsaConfig(BayesSeedRngMixin, BayesProposalCovMixin):
    """Generated model for GpmsaConfig"""

    chain_samples: int = DakotaField(
        description="Number of Markov Chain Monte Carlo posterior samples",
        dakota={
            "aliases": ["samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.chain_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ],
        },
    )
    build_samples: int = DakotaField(
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
    import_build_points_file: GpmsaImportBuildPointsFile | None = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )
    standardized_space: Literal[True] | None = DakotaField(
        default=None,
        description="Perform Bayesian inference in standardized probability space",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.standardized_space",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    logit_transform: Literal[True] | None = DakotaField(
        default=None,
        description="Utilize the logit transformation to reduce sample rejection for bounded domains",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.logit_transform",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    gpmsa_normalize: Literal[True] | None = DakotaField(
        default=None,
        description="Enable GPMSA-internal normalization",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.gpmsa_normalize",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    export_chain_points_file: GpmsaExportChainPointsFile | None = DakotaField(
        default=None,
        description="Export the MCMC chain to the specified filename",
        dakota={"argument": "filename"},
    )
    mcmc_algorithm: Union[
        GpmsaDram,
        GpmsaDelayedRejection,
        GpmsaAdaptiveMetropolis,
        GpmsaMetropolisHastings,
    ] = DakotaField(
        default_factory=GpmsaDram,
        description="MCMC Algorithm",
        dakota={"anchor": True, "union_pattern": 1, "model_default": "GpmsaDram"},
    )
    options_file: str | None = DakotaField(
        default=None,
        description="File containing advanced QUESO/GPMSA options",
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


class GaussianCov(DakotaBaseModel):
    """Generated model for GaussianCov"""

    values: list[DakotaFloat] = DakotaField(
        description="(Experimental Capability) Covariance of a Gaussian error distribution",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.data_dist_covariance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    type: Union[CovDiag, CovMatrix] = DakotaField(
        description="Values For", dakota={"anchor": True, "union_pattern": 4}
    )


class DreamConfig(BayesEmulatorMixin):
    """Generated model for DreamConfig"""

    chain_samples: int = DakotaField(
        description="Number of Markov Chain Monte Carlo posterior samples",
        dakota={
            "aliases": ["samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.chain_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
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
    chains: int = DakotaField(
        default=3,
        ge=3,
        description="Number of chains in DREAM",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.dream.num_chains",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    num_cr: int = DakotaField(
        default=3,
        ge=1,
        description="Number of candidate points for each crossover.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.dream.num_cr",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    crossover_chain_pairs: int = DakotaField(
        default=3,
        ge=0,
        description="Number of chains used in crossover.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.dream.crossover_chain_pairs",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    gr_threshold: DakotaFloat = DakotaField(
        default=1.2,
        gt=0.0,
        description="Convergence tolerance for the Gelman-Rubin statistic",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.dream.gr_threshold",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    jump_step: int = DakotaField(
        default=5,
        ge=0,
        description="Number of generations a long jump step is taken",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.dream.jump_step",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    standardized_space: Literal[True] | None = DakotaField(
        default=None,
        description="Perform Bayesian inference in standardized probability space",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.standardized_space",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    export_chain_points_file: DreamExportChainPointsFile | None = DakotaField(
        default=None,
        description="Export the MCMC chain to the specified filename",
        dakota={"argument": "filename"},
    )


class MuqAdaptiveMetropolis(DakotaBaseModel):
    """Generated model for MuqAdaptiveMetropolis"""

    adaptive_metropolis: AdaptiveMetropolisConfig = DakotaField(
        default_factory=AdaptiveMetropolisConfig,
        description="Use the Adaptive Metropolis MCMC algorithm",
        dakota={
            "model_default": "AdaptiveMetropolisConfig",
            "materialization": [
                {
                    "ir_key": "method.nond.mcmc_type",
                    "stored_value": "adaptive_metropolis",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class MuqDelayedRejection(DakotaBaseModel):
    """Generated model for MuqDelayedRejection"""

    delayed_rejection: DelayedRejectionConfig = DakotaField(
        default_factory=DelayedRejectionConfig,
        description="Use the Delayed Rejection MCMC algorithm",
        dakota={
            "model_default": "DelayedRejectionConfig",
            "materialization": [
                {
                    "ir_key": "method.nond.mcmc_type",
                    "stored_value": "delayed_rejection",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class Dili(DakotaBaseModel):
    """Generated model for Dili"""

    dili: DiliConfig = DakotaField(
        default_factory=DiliConfig,
        description="Dimension-independent likelihood-informed MCMC",
        dakota={
            "model_default": "DiliConfig",
            "materialization": [
                {
                    "ir_key": "method.nond.mcmc_type",
                    "stored_value": "dili",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class MuqDram(DakotaBaseModel):
    """Generated model for MuqDram"""

    dram: DramConfig = DakotaField(
        default_factory=DramConfig,
        description="Use the DRAM MCMC algorithm",
        dakota={
            "model_default": "DramConfig",
            "materialization": [
                {
                    "ir_key": "method.nond.mcmc_type",
                    "stored_value": "dram",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class MultilevelMcmc(DakotaBaseModel):
    """Generated model for MultilevelMcmc"""

    multilevel_mcmc: MultilevelMcmcConfig = DakotaField(
        default_factory=MultilevelMcmcConfig,
        description="Use the Multi-level MCMC algorithm",
        dakota={
            "model_default": "MultilevelMcmcConfig",
            "materialization": [
                {
                    "ir_key": "method.nond.mcmc_type",
                    "stored_value": "multilevel_mcmc",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class Mala(DakotaBaseModel):
    """Generated model for Mala"""

    mala: MalaConfig = DakotaField(
        default_factory=MalaConfig,
        description="Metropolis-adjusted Langevin algorithm",
        dakota={
            "model_default": "MalaConfig",
            "materialization": [
                {
                    "ir_key": "method.nond.mcmc_type",
                    "stored_value": "mala",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class ImportCandidatePointsFileCustomAnnotated(DakotaBaseModel):
    """Generated model for ImportCandidatePointsFileCustomAnnotated"""

    custom_annotated: ImportCandidatePointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=ImportCandidatePointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "ImportCandidatePointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_candidate_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class CalibrateErrorMultipliers(DakotaBaseModel):
    """Generated model for CalibrateErrorMultipliers"""

    selection: Union[One, PerExperiment, PerResponse, Both] = DakotaField(
        description="Calibrate Error Multipliers",
        dakota={"anchor": True, "union_pattern": 4},
    )
    hyperprior_parameters: HyperpriorParameters | None = DakotaField(
        default=None,
        description="Specify shape (alpha) and scale (beta) of the inverse gamma hyper-parameter prior",
    )


class PosteriorStats(DakotaBaseModel):
    """Generated model for PosteriorStats"""

    kl_divergence: Literal[True] | None = DakotaField(
        default=None,
        description="Calculate the Kullback-Leibler Divergence between prior and posterior",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.posterior_stats.kl_divergence",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    mutual_info: MutualInfo | None = DakotaField(
        default=None,
        description="Calculate the mutual information between prior and posterior",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.posterior_stats.mutual_info",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    kde: Literal[True] | None = DakotaField(
        default=None,
        description="Calculate the Kernel Density Estimate of the posterior distribution",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.posterior_stats.kde",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class DiscrepancyTypeGP(DakotaBaseModel):
    """Generated model for DiscrepancyTypeGP"""

    gaussian_process: DiscrepancyTypeGPConfig = DakotaField(
        default_factory=DiscrepancyTypeGPConfig,
        description="Use the Surfpack version of Gaussain process as the discrepancy model",
        dakota={
            "aliases": ["kriging"],
            "model_default": "DiscrepancyTypeGPConfig",
            "materialization": [
                {
                    "ir_key": "method.nond.discrepancy_type",
                    "stored_value": "global_kriging",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class DiscrepancyTypePolynomial(DakotaBaseModel):
    """Generated model for DiscrepancyTypePolynomial"""

    polynomial: DiscrepancyTypePolynomialConfig = DakotaField(
        default_factory=DiscrepancyTypePolynomialConfig,
        description="Use a polynomial surrogate as the discrepancy model",
        dakota={
            "model_default": "DiscrepancyTypePolynomialConfig",
            "materialization": [
                {
                    "ir_key": "method.nond.discrepancy_type",
                    "stored_value": "global_polynomial",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class ImportPredictionConfigsCustomAnnotated(DakotaBaseModel):
    """Generated model for ImportPredictionConfigsCustomAnnotated"""

    custom_annotated: ImportPredictionConfigsCustomAnnotatedConfig = DakotaField(
        default_factory=ImportPredictionConfigsCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "ImportPredictionConfigsCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_prediction_configs_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class ExportDiscrepancyFileCustomAnnotated(DakotaBaseModel):
    """Generated model for ExportDiscrepancyFileCustomAnnotated"""

    custom_annotated: ExportDiscrepancyFileCustomAnnotatedConfig = DakotaField(
        default_factory=ExportDiscrepancyFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "ExportDiscrepancyFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.nond.export_discrep_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class ExportCorrectedModelFileCustomAnnotated(DakotaBaseModel):
    """Generated model for ExportCorrectedModelFileCustomAnnotated"""

    custom_annotated: ExportCorrectedModelFileCustomAnnotatedConfig = DakotaField(
        default_factory=ExportCorrectedModelFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "ExportCorrectedModelFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.nond.export_corrected_model_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class ExportCorrectedVarianceFileCustomAnnotated(DakotaBaseModel):
    """Generated model for ExportCorrectedVarianceFileCustomAnnotated"""

    custom_annotated: ExportCorrectedVarianceFileCustomAnnotatedConfig = DakotaField(
        default_factory=ExportCorrectedVarianceFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "ExportCorrectedVarianceFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.nond.export_corrected_variance_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class Queso(DakotaBaseModel):
    """Generated model for Queso"""

    queso: QuesoConfig = DakotaField(
        description="Markov Chain Monte Carlo algorithms from the QUESO package",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "stored_value": "SUBMETHOD_QUESO",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class Gpmsa(DakotaBaseModel):
    """Generated model for Gpmsa"""

    gpmsa: GpmsaConfig = DakotaField(
        description="(Experimental) Gaussian Process Models for Simulation Analysis (GPMSA) Bayesian calibration",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "stored_value": "SUBMETHOD_GPMSA",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class GaussianConfig(DakotaBaseModel):
    """Generated model for GaussianConfig"""

    means: list[DakotaFloat] = DakotaField(
        description="(Experimental Capability) Means of Gaussian error distribution",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.data_dist_means",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    covariance: GaussianCov = DakotaField(
        default=...,
        description="(Experimental Capability) Covariance of a Gaussian error distribution",
        dakota={"argument": "values"},
    )


class Dream(DakotaBaseModel):
    """Generated model for Dream"""

    dream: DreamConfig = DakotaField(
        description="DREAM (DiffeRential Evolution Adaptive Metropolis)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "stored_value": "SUBMETHOD_DREAM",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MuqConfig(BayesSeedRngMixin, BayesProposalCovMixin):
    """Generated model for MuqConfig"""

    chain_samples: int = DakotaField(
        description="Number of Markov Chain Monte Carlo posterior samples",
        dakota={
            "aliases": ["samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.chain_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ],
        },
    )
    export_chain_points_file: MuqExportChainPointsFile | None = DakotaField(
        default=None,
        description="Export the MCMC chain to the specified filename",
        dakota={"argument": "filename"},
    )
    mcmc_algorithm: Union[
        MuqAdaptiveMetropolis,
        MuqDelayedRejection,
        Dili,
        MuqDram,
        MultilevelMcmc,
        Mala,
        MuqMetropolisHastings,
    ] = DakotaField(
        default_factory=MuqDram,
        description="MCMC Algorithm",
        dakota={"anchor": True, "union_pattern": 1, "model_default": "MuqDram"},
    )
    pre_solve: Union[MuqPreSolveSqp, MuqPreSolveNip, MuqPreSolveNone] | None = (
        DakotaField(
            default=None,
            description="Perform deterministic optimization for MAP before Bayesian calibration",
            dakota={"union_pattern": 2},
        )
    )


class ImportCandidatePointsFile(DakotaBaseModel):
    """Generated model for ImportCandidatePointsFile"""

    filename: str = DakotaField(
        description="Specify text file containing candidate design points",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_candidate_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        ImportCandidatePointsFileCustomAnnotated,
        ImportCandidatePointsFileAnnotated,
        ImportCandidatePointsFileFreeform,
    ] = DakotaField(
        default_factory=ImportCandidatePointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "ImportCandidatePointsFileAnnotated",
        },
    )


class ImportPredictionConfigs(DakotaBaseModel):
    """Generated model for ImportPredictionConfigs"""

    filename: str = DakotaField(
        description="Specify text file containing prediction configurations for model discrepancy",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_prediction_configs",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        ImportPredictionConfigsCustomAnnotated,
        ImportPredictionConfigsAnnotated,
        ImportPredictionConfigsFreeform,
    ] = DakotaField(
        default_factory=ImportPredictionConfigsAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "ImportPredictionConfigsAnnotated",
        },
    )


class ExportDiscrepancyFile(DakotaBaseModel):
    """Generated model for ExportDiscrepancyFile"""

    filename: str = DakotaField(
        description="Output file for prediction discrepancy calculations",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_discrepancy_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        ExportDiscrepancyFileCustomAnnotated,
        ExportDiscrepancyFileAnnotated,
        ExportDiscrepancyFileFreeform,
    ] = DakotaField(
        default_factory=ExportDiscrepancyFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "ExportDiscrepancyFileAnnotated",
        },
    )


class ExportCorrectedModelFile(DakotaBaseModel):
    """Generated model for ExportCorrectedModelFile"""

    filename: str = DakotaField(
        description="Output file for corrected model prediction calculations",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_corrected_model_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        ExportCorrectedModelFileCustomAnnotated,
        ExportCorrectedModelFileAnnotated,
        ExportCorrectedModelFileFreeform,
    ] = DakotaField(
        default_factory=ExportCorrectedModelFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "ExportCorrectedModelFileAnnotated",
        },
    )


class ExportCorrectedVarianceFile(DakotaBaseModel):
    """Generated model for ExportCorrectedVarianceFile"""

    filename: str = DakotaField(
        description="Output file for prediction variance calculations",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_corrected_variance_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        ExportCorrectedVarianceFileCustomAnnotated,
        ExportCorrectedVarianceFileAnnotated,
        ExportCorrectedVarianceFileFreeform,
    ] = DakotaField(
        default_factory=ExportCorrectedVarianceFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "ExportCorrectedVarianceFileAnnotated",
        },
    )


class Gaussian(DakotaBaseModel):
    """Generated model for Gaussian"""

    gaussian: GaussianConfig = DakotaField(
        description="(Experimental Capability) Gaussian error distribution"
    )


class Muq(DakotaBaseModel):
    """Generated model for Muq"""

    muq: MuqConfig = DakotaField(
        description="Markov Chain Monte Carlo algorithms from the MUQ package",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "stored_value": "SUBMETHOD_MUQ",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ExperimentalDesign(DakotaBaseModel):
    """Generated model for ExperimentalDesign"""

    initial_samples: int = DakotaField(
        description="Number of data points used during initial Bayesian calibration",
        dakota={
            "aliases": ["samples"],
            "materialization": [
                {
                    "ir_key": "method.adapt_exp_design_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ],
        },
    )
    num_candidates: int = DakotaField(
        gt=0,
        description="Number of candidate design points considered",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.num_candidates",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    max_hifi_evaluations: int | None = DakotaField(
        default=None,
        ge=0,
        description="Maximum number of high-fidelity model runs to be used",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.max_hifi_evaluations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    batch_size: int = DakotaField(
        default=1,
        ge=1,
        description="Number of optimal designs selected concurrently",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.batch_size",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    import_candidate_points_file: ImportCandidatePointsFile | None = DakotaField(
        default=None,
        description="Specify text file containing candidate design points",
        dakota={"argument": "filename"},
    )
    ksg2: Literal[True] | None = DakotaField(
        default=None,
        description="Use second Kraskov algorithm to compute mutual information",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.mutual_info_ksg2",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class ModelDiscrepancy(DakotaBaseModel):
    """Generated model for ModelDiscrepancy"""

    discrepancy_type: Union[DiscrepancyTypeGP, DiscrepancyTypePolynomial] = DakotaField(
        default_factory=DiscrepancyTypeGP,
        description="Specify the type of model discrepancy",
        dakota={"union_pattern": 1, "model_default": "DiscrepancyTypeGP"},
    )
    num_prediction_configs: int = DakotaField(
        default=0,
        ge=0,
        description="Specify number of prediction locations for model discrepancy",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.num_prediction_configs",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    prediction_configs: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="List prediction locations for model discrepancy",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.prediction_configs",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    import_prediction_configs: ImportPredictionConfigs | None = DakotaField(
        default=None,
        description="Specify text file containing prediction configurations for model discrepancy",
        dakota={"argument": "filename"},
    )
    export_discrepancy_file: ExportDiscrepancyFile | None = DakotaField(
        default=None,
        description="Output file for prediction discrepancy calculations",
        dakota={"argument": "filename"},
    )
    export_corrected_model_file: ExportCorrectedModelFile | None = DakotaField(
        default=None,
        description="Output file for corrected model prediction calculations",
        dakota={"argument": "filename"},
    )
    export_corrected_variance_file: ExportCorrectedVarianceFile | None = DakotaField(
        default=None,
        description="Output file for prediction variance calculations",
        dakota={"argument": "filename"},
    )


class WasabiConfig(BayesEmulatorMixin):
    """Generated model for WasabiConfig"""

    pushforward_samples: int = DakotaField(
        description="(Experimental Capability) Number of samples of the prior to push forward through the model",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.pushforward_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
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
    standardized_space: Literal[True] | None = DakotaField(
        default=None,
        description="Perform Bayesian inference in standardized probability space",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.standardized_space",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    data_distribution: Union[Gaussian, ObsDataFilename] = DakotaField(
        description="(Experimental Capability) Specify the distribution of the experimental data",
        dakota={"union_pattern": 4},
    )
    posterior_samples_import_filename: str | None = DakotaField(
        default=None,
        description="(Experimental Capability) Filename for samples at which the user would like the posterior density calculated",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.posterior_samples_import_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    generate_posterior_samples: GeneratePosteriorSamples | None = DakotaField(
        default=None,
        description="(Experimental Capability) Generate random samples from the posterior density",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.generate_posterior_samples",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    evaluate_posterior_density: EvaluatePosteriorDensity | None = DakotaField(
        default=None,
        description="(Experimental Capability) Evaluate the posterior density and output to the specified file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.evaluate_posterior_density",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class Wasabi(DakotaBaseModel):
    """Generated model for Wasabi"""

    wasabi: WasabiConfig = DakotaField(
        description="(Experimental Method) Non-MCMC Bayesian inference using interval analysis",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "stored_value": "SUBMETHOD_WASABI",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BayesCalibrationConfig(
    MethodThreeOptionalKeywordsMixin,
    ProbabilityLevelsContext1Mixin,
    MethodConvergenceTolMixin,
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
    DefaultScalingMixin,
):
    """Generated model for BayesCalibrationConfig"""

    sub_method: Union[Queso, Gpmsa, Wasabi, Dream, Muq] = DakotaField(
        description="Bayesian Calibration Method",
        dakota={"anchor": True, "union_pattern": 4},
    )
    experimental_design: ExperimentalDesign | None = DakotaField(
        default=None,
        description="(Experimental) Adaptively select experimental designs for iterative Bayesian updating",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.adapt_exp_design",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    calibrate_error_multipliers: CalibrateErrorMultipliers | None = DakotaField(
        default=None,
        description="Calibrate hyper-parameter multipliers on the observation error covariance",
    )
    burn_in_samples: int = DakotaField(
        default=0,
        description="Manually specify the burn in period for the MCMC chain.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.burn_in_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    posterior_stats: PosteriorStats | None = DakotaField(
        default=None,
        description="Compute information-theoretic metrics on posterior parameter distribution",
    )
    chain_diagnostics: ChainDiagnostics | None = DakotaField(
        default=None,
        description="Compute diagnostic metrics for Markov chain",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.chain_diagnostics",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    model_evidence: ModelEvidence | None = DakotaField(
        default=None,
        description="Calculate model evidence (marginal likelihood of model) when using Bayesian methods",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.model_evidence",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    model_discrepancy: ModelDiscrepancy | None = DakotaField(
        default=None,
        description="(Experimental) Post-calibration calculation of model discrepancy correction",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.model_discrepancy",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    sub_sampling_period: int = DakotaField(
        default=1,
        description="Specify a sub-sampling of the MCMC chain",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_sampling_period",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )


class BayesCalibrationSelection(MethodSelection):
    """Generated model for BayesCalibrationSelection"""

    bayes_calibration: BayesCalibrationConfig = DakotaField(
        dakota={
            "aliases": ["nond_bayes_calibration"],
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "BAYES_CALIBRATION",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        }
    )
