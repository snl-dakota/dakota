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
from copy import deepcopy
from typing import Any

from pydantic import BaseModel

from dakota.spec import environment, interface, method, model, responses, variables

_FACTORY_CONFIG_CLASSES = {
    "Study.variables": variables.VariablesConfig,
    "Study.responses": responses.ResponsesConfig,
    "Study.interface": interface.InterfaceConfig,
    "ModelFactory.simulation": model.SingleConfig,
    "ModelFactory.nested": model.NestedConfig,
    "ModelFactory.ensemble_surrogate": model.EnsembleSurrogateConfig,
    "MethodFactory.sampling": method.SamplingConfig,
    "MethodFactory.dot_bfgs": method.DotBfgsConfig,
    "MethodFactory.multi_start": method.MultiStartConfig,
}


# Read actual selector field names: class names can be abbreviated (e.g.
# StochCollocConfig), so deriving a keyword from a class name is not reliable.
for _name, _selection in inspect.getmembers(method, inspect.isclass):
    if _selection is method.MethodSelection or not issubclass(_selection, method.MethodSelection):
        continue
    for _selector, _field in _selection.model_fields.items():
        _schema = _field.annotation
        if inspect.isclass(_schema) and issubclass(_schema, BaseModel):
            _FACTORY_CONFIG_CLASSES[f"MethodFactory.{_selector}"] = _schema


def _validated_config(value: Any, config_class: type[BaseModel], name: str) -> dict:
    if isinstance(value, config_class):
        validated = value
    elif isinstance(value, dict):
        # Some Dakota before-validators replace entries in their input dict.
        validated = config_class.model_validate(deepcopy(value))
    else:
        raise TypeError(
            f"{name}: config must be a dict, {config_class.__name__} instance, or None"
        )
    return _dump_validated(validated)


def normalize_factory_config(config: Any, kwargs: dict, factory_name: str) -> dict:
    """Return validated JSON data, trusting instances of the expected schema."""
    if config is not None and kwargs:
        raise TypeError(
            f"{factory_name}: pass config or configuration kwargs, not both"
        )
    return _validated_config(
        dict(kwargs) if config is None else config,
        _FACTORY_CONFIG_CLASSES[factory_name], factory_name,
    )


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
        model.EnsembleSurrogateConfig: "ensemble_surrogate",
    }
    if config_class in overrides:
        return overrides[config_class]
    return _snake_case(config_class.__name__.removesuffix("Config"))


def _make_validator(config_class: type[Any]) -> Callable[[Any], dict]:
    def validate_fragment(value: Any) -> dict:
        return _validated_config(value, config_class, _fragment_name(config_class))

    validate_fragment.__name__ = f"validate_{_fragment_name(config_class)}_fragment"
    return validate_fragment


def _register_validators() -> None:
    for module in (environment, interface, method, model, responses, variables):
        for _, config_class in inspect.getmembers(module, inspect.isclass):
            if not config_class.__name__.endswith("Config"):
                continue
            if not (config_class.__module__ == module.__name__ or
                    (module is method and
                     config_class.__module__.startswith(module.__name__ + "."))):
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
