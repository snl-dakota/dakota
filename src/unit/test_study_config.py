"""Configuration normalization tests; runnable without the native extension."""

import copy
import importlib.util
from pathlib import Path
import unittest

from pydantic import BaseModel, RootModel, ValidationError
from dakota.spec.method import SamplingConfig, MultiStartConfig
from dakota.spec.model import NestedConfig, SingleConfig
from dakota.spec.variables import VariablesConfig

# Load this pure Python helper directly, avoiding dakota.study.__init__'s native
# imports. Production imports still use the ordinary package path.
_spec = importlib.util.spec_from_file_location(
    "study_validation_under_test",
    Path(__file__).resolve().parents[2] / "python/dakota/study/_validation.py",
)
_validation = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_validation)
normalize = _validation.normalize_factory_config


class ConfigNormalizationTests(unittest.TestCase):
    def test_dict_kwargs_validate_once_and_models_are_not_revalidated(self):
        from unittest.mock import patch

        data = {"samples": 10, "seed": 1234}
        before = copy.deepcopy(data)
        config = SamplingConfig.model_validate(data)
        expected = config.model_dump(mode="json", exclude_none=True)
        with patch.object(SamplingConfig, "model_validate",
                          wraps=SamplingConfig.model_validate) as validate:
            self.assertEqual(normalize(data, {}, "MethodFactory.sampling"), expected)
            self.assertEqual(validate.call_count, 1)
            self.assertEqual(normalize(None, data, "MethodFactory.sampling"), expected)
            self.assertEqual(validate.call_count, 2)
            self.assertEqual(normalize(config, {}, "MethodFactory.sampling"), expected)
            self.assertEqual(validate.call_count, 2)
        self.assertEqual(data, before)

    def test_conflicts_and_wrong_types(self):
        for config in ({}, SamplingConfig(samples=10)):
            with self.assertRaisesRegex(TypeError, "pass config"):
                normalize(config, {"samples": 10}, "MethodFactory.sampling")
        for config in ([], False, "bad", SingleConfig(),
                       RootModel[list[int]]([1])):
            with self.subTest(config=config), self.assertRaisesRegex(TypeError, "SamplingConfig"):
                normalize(config, {}, "MethodFactory.sampling")

    def test_schema_errors_and_defaults(self):
        for data in ({"unknown": True}, {"samples": "not a number"}):
            with self.assertRaises(ValidationError):
                normalize(data, {}, "MethodFactory.sampling")
        # Injected dependencies replace input-file pointers in API mode, so
        # pointer-only selections are populated with the API sentinel.
        self.assertIn("sub_method_pointer",
                      normalize(None, {}, "ModelFactory.nested"))
        self.assertIn("sub_method",
                      normalize(None, {}, "MethodFactory.multi_start"))
        self.assertEqual(normalize(None, {}, "ModelFactory.simulation"),
                         SingleConfig().model_dump(mode="json", exclude_none=True))

    def test_every_factory_schema(self):
        from dakota.spec.interface import InterfaceConfig
        from dakota.spec.responses import ResponsesConfig
        from dakota.spec.method import DotBfgsConfig

        fixtures = [
            ("Study.variables", VariablesConfig, {"uniform_uncertain": {
                "count": 2, "lower_bounds": [0., 0.], "upper_bounds": [1., 1.]}}),
            ("Study.responses", ResponsesConfig, {
                "response_type": {"response_functions": {"count": 1}},
                "gradient_type": {"no_gradients": True},
                "hessian_type": {"no_hessians": True}}),
            ("Study.interface", InterfaceConfig, {"analysis_drivers": {
                "drivers": ["text_book"], "interface_type": {"fork": {}}}}),
            ("MethodFactory.sampling", SamplingConfig, {"samples": 10}),
            ("MethodFactory.dot_bfgs", DotBfgsConfig, {}),
            ("ModelFactory.simulation", SingleConfig, {}),
            ("ModelFactory.nested", NestedConfig, {"sub_method_pointer": {"pointer": "inner"}}),
            ("MethodFactory.multi_start", MultiStartConfig, {
                "sub_method": {"method_pointer": "inner"}, "starting_points": [0.5, 0.5]}),
        ]
        for name, schema, data in fixtures:
            with self.subTest(factory=name):
                config = schema.model_validate(data)
                expected = config.model_dump(mode="json", exclude_none=True)
                self.assertEqual(normalize(data, {}, name), expected)
                self.assertEqual(normalize(None, data, name), expected)
                self.assertEqual(normalize(config, {}, name), expected)

    def test_variable_internal_and_derived_values_are_preserved(self):
        from unittest.mock import patch

        for kind, fragment in [
            ("uniform_uncertain", {"count": 2, "lower_bounds": [0., 0.], "upper_bounds": [1., 1.]}),
            ("normal_uncertain", {"count": 2, "means": [0., 1.], "std_deviations": [1., 2.]}),
        ]:
            for initial in (None, [0.25, 0.75]):
                data = {kind: dict(fragment, initial_point=initial)}
                config = VariablesConfig.model_validate(data)
                before = config.model_dump(mode="json", exclude_none=True)
                with self.subTest(kind=kind, initial=initial):
                    with patch.object(VariablesConfig, "model_validate",
                                      side_effect=AssertionError("must not revalidate")):
                        result = normalize(config, {}, "Study.variables")
                    self.assertEqual(result, before)
                    self.assertEqual(result[kind]["initial_point_user_provided"], initial is not None)
                    self.assertIn("initial_point", result[kind])
                    if kind == "normal_uncertain":
                        self.assertIn("inferred_lower_bounds", result[kind])
                    self.assertEqual(normalize(data, {}, "Study.variables"), before)
                    self.assertEqual(config.model_dump(mode="json", exclude_none=True), before)
        invalid = {"uniform_uncertain": {"count": 1, "lower_bounds": [0.0],
                    "upper_bounds": [1.0], "initial_point_user_provided": False}}
        with self.assertRaises(ValidationError):
            normalize(invalid, {}, "Study.variables")

    def test_legacy_method_validators_are_registered_and_trust_instances(self):
        from unittest.mock import patch

        config = SamplingConfig(samples=10)
        with patch.object(SamplingConfig, "model_validate",
                          side_effect=AssertionError("must not revalidate")):
            self.assertEqual(_validation.validate_sampling_fragment(config),
                             config.model_dump(mode="json", exclude_none=True))
        self.assertTrue(callable(_validation.validate_vector_parameter_study_fragment))


if __name__ == "__main__":
    unittest.main()
