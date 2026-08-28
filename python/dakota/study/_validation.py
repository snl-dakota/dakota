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
from dakota.spec.method import DotBfgsConfig, MultiStartConfig, SamplingConfig
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


def validate_dot_bfgs_fragment(value: Any) -> dict:
    return _dump_validated(DotBfgsConfig.model_validate(value))


def validate_multi_start_fragment(value: Any) -> dict:
    return _dump_validated(MultiStartConfig.model_validate(value))


def validate_simulation_model_fragment(value: Any) -> dict:
    return _dump_validated(SingleConfig.model_validate(value))


def validate_nested_model_fragment(value: Any) -> dict:
    return _dump_validated(NestedConfig.model_validate(value))
