#  _______________________________________________________________________
#
#    Dakota: Explore and predict with confidence.
#    Copyright 2014-2025
#    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#    This software is distributed under the GNU Lesser General Public License.
#    For more information, see the README file in the top Dakota directory.
#    _______________________________________________________________________

from __future__ import annotations

import inspect
import re
from collections.abc import Callable
from typing import Any

from dakota.spec import environment, interface, method, model, responses, variables


def _dump_validated(model: Any) -> dict:
    return model.model_dump(mode="json", exclude_none=True)


def _snake_case(name: str) -> str:
    first_pass = re.sub(r"(.)([A-Z][a-z]+)", r"\1_\2", name)
    return re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", first_pass).lower()


def _fragment_name(config_class: type[Any]) -> str:
    overrides = {
        environment.EnvironmentConfig: "environment",
        interface.InterfaceConfig: "interface",
        responses.ResponsesConfig: "responses",
        variables.VariablesConfig: "variables",
        model.SingleConfig: "single",
        model.NestedConfig: "nested",
    }
    if config_class in overrides:
        return overrides[config_class]
    return _snake_case(config_class.__name__.removesuffix("Config"))


def _make_validator(config_class: type[Any]) -> Callable[[Any], dict]:
    def validate_fragment(value: Any) -> dict:
        return _dump_validated(config_class.model_validate(value))

    validate_fragment.__name__ = f"validate_{_fragment_name(config_class)}_fragment"
    return validate_fragment


def _register_validators() -> None:
    for module in (environment, interface, method, model, responses, variables):
        for _, config_class in inspect.getmembers(module, inspect.isclass):
            if not config_class.__name__.endswith("Config"):
                continue
            if config_class.__module__ != module.__name__:
                continue
            globals()[f"validate_{_fragment_name(config_class)}_fragment"] = (
                _make_validator(config_class)
            )


_register_validators()

__all__ = sorted(
    name
    for name in globals()
    if name.startswith("validate_") and name.endswith("_fragment")
)
