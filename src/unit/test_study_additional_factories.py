"""Contracts for the additional concrete DI iterators and ensemble models."""

import copy
import importlib.util
from pathlib import Path
import unittest
from unittest.mock import patch

from pydantic import ValidationError
from dakota.spec.model import EnsembleSurrogateConfig
from test_study_config import normalize, _validation

_spec = importlib.util.spec_from_file_location(
    "iterator_bindings_manifest",
    Path(__file__).resolve().parents[2] /
    "python/dakota/study/_iterator_bindings.py",
)
_manifest = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_manifest)


class AdditionalConfigTests(unittest.TestCase):
    def test_all_factory_selectors_have_correct_schemas(self):
        # Real selection schemas are the source of truth, including names whose
        # spelling differs from their config class (StochCollocConfig, FtConfig).
        from dakota.spec import method
        for name in _manifest.FACTORY_NAMES:
            with self.subTest(method=name):
                schema = _validation._FACTORY_CONFIG_CLASSES[f"MethodFactory.{name}"]
                selections = [cls for cls in vars(method).values()
                              if isinstance(cls, type) and
                              issubclass(cls, method.MethodSelection) and
                              name in cls.model_fields]
                self.assertTrue(selections)
                self.assertIs(schema, selections[0].model_fields[name].annotation)
                with self.assertRaises(ValidationError):
                    normalize({"not_a_dakota_option": True}, {}, f"MethodFactory.{name}")
                with self.assertRaisesRegex(TypeError, "pass config"):
                    normalize({}, {"not_a_dakota_option": True}, f"MethodFactory.{name}")

    def test_ensemble_serialization_and_single_validation(self):
        name = "ModelFactory.ensemble_surrogate"
        data = {"ensemble": {"truth_model_pointer": {
            "pointer": "truth", "approximation_models": ["approx"]}}}
        config = EnsembleSurrogateConfig.model_validate(copy.deepcopy(data))
        expected = config.model_dump(mode="json", exclude_none=True)
        with patch.object(EnsembleSurrogateConfig, "model_validate",
                          wraps=EnsembleSurrogateConfig.model_validate) as validate:
            self.assertEqual(normalize(data, {}, name), expected)
            self.assertEqual(normalize(None, data, name), expected)
            self.assertEqual(validate.call_count, 2)
            self.assertEqual(normalize(config, {}, name), expected)
            self.assertEqual(validate.call_count, 2)
        # In API mode the injected models replace input-file pointers, so the
        # pointer union is populated with a sentinel when omitted.
        self.assertIn("ensemble", normalize({}, {}, name))
        correction = {
            "ensemble": {"ordered_model_fidelities": {
                "correction": {
                    "correction_order": {"zeroth_order": True},
                    "correction_type": {"additive": True},
                }
            }}
        }
        normalized = normalize(correction, {}, name)
        ordered = normalized["ensemble"]["ordered_model_fidelities"]
        self.assertIn("pointers", ordered)
        self.assertEqual(ordered["correction"], correction["ensemble"]
                         ["ordered_model_fidelities"]["correction"])
        with self.assertRaises(ValidationError):
            normalize(dict(data, ordered_model_fidelities={"pointers": ["approx", "truth"]}), {}, name)


if __name__ == "__main__":
    unittest.main()
