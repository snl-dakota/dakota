#  _______________________________________________________________________
#
#    Dakota: Explore and predict with confidence.
#    Copyright 2014-2025
#    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#    This software is distributed under the GNU Lesser General Public License.
#    For more information, see the README file in the top Dakota directory.
#    _______________________________________________________________________

from __future__ import annotations

from typing import Any

from dakota.spec.environment import EnvironmentConfig
from dakota.spec.interface import InterfaceConfig
from dakota.spec.method import (
    CenteredParameterStudyConfig,
    DotBfgsConfig,
    EfficientGlobalConfig,
    GlobalIntervalEstConfig,
    ListParameterStudyConfig,
    LocalIntervalEstConfig,
    MultiStartConfig,
    MultidimParameterStudyConfig,
    Nl2solConfig,
    NpsolSqpConfig,
    RichardsonExtrapConfig,
    SamplingConfig,
    VectorParameterStudyConfig,
)
from dakota.spec.model import NestedConfig, SingleConfig
from dakota.spec.responses import ResponsesConfig
from dakota.spec.variables import VariablesConfig


def _dump_validated(model: Any) -> dict:
    return model.model_dump(mode="json", exclude_none=True)


def validate_variables_fragment(value: Any) -> dict:
    return _dump_validated(VariablesConfig.model_validate(value))


def validate_responses_fragment(value: Any) -> dict:
    return _dump_validated(ResponsesConfig.model_validate(value))


def validate_interface_fragment(value: Any) -> dict:
    return _dump_validated(InterfaceConfig.model_validate(value))


def validate_environment_fragment(value: Any) -> dict:
    return _dump_validated(EnvironmentConfig.model_validate(value))


def validate_sampling_fragment(value: Any) -> dict:
    return _dump_validated(SamplingConfig.model_validate(value))


def validate_vector_parameter_study_fragment(value: Any) -> dict:
    return _dump_validated(VectorParameterStudyConfig.model_validate(value))


def validate_list_parameter_study_fragment(value: Any) -> dict:
    return _dump_validated(ListParameterStudyConfig.model_validate(value))


def validate_centered_parameter_study_fragment(value: Any) -> dict:
    return _dump_validated(CenteredParameterStudyConfig.model_validate(value))


def validate_multidim_parameter_study_fragment(value: Any) -> dict:
    return _dump_validated(MultidimParameterStudyConfig.model_validate(value))


def validate_richardson_extrap_fragment(value: Any) -> dict:
    return _dump_validated(RichardsonExtrapConfig.model_validate(value))


def validate_local_interval_est_fragment(value: Any) -> dict:
    return _dump_validated(LocalIntervalEstConfig.model_validate(value))


def validate_global_interval_est_fragment(value: Any) -> dict:
    return _dump_validated(GlobalIntervalEstConfig.model_validate(value))


def validate_efficient_global_fragment(value: Any) -> dict:
    return _dump_validated(EfficientGlobalConfig.model_validate(value))


def validate_npsol_sqp_fragment(value: Any) -> dict:
    return _dump_validated(NpsolSqpConfig.model_validate(value))


def validate_nl2sol_fragment(value: Any) -> dict:
    return _dump_validated(Nl2solConfig.model_validate(value))


def validate_dot_bfgs_fragment(value: Any) -> dict:
    return _dump_validated(DotBfgsConfig.model_validate(value))


def validate_multi_start_fragment(value: Any) -> dict:
    return _dump_validated(MultiStartConfig.model_validate(value))


def validate_simulation_model_fragment(value: Any) -> dict:
    return _dump_validated(SingleConfig.model_validate(value))


def validate_nested_model_fragment(value: Any) -> dict:
    return _dump_validated(NestedConfig.model_validate(value))
