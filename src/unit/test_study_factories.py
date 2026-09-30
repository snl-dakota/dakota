"""Native Study factory contract tests. CTest requires an importable module."""

import copy
import os
from pathlib import Path
import shutil
import tempfile
import unittest

from pydantic import BaseModel

try:
    import dakota.study as ds
except ModuleNotFoundError as error:
    if error.name != "dakota.study._study" or os.environ.get("DAKOTA_REQUIRE_STUDY"):
        raise
    ds = None

if ds is not None:
    from dakota.spec.interface import InterfaceConfig
    from dakota.spec.method import SamplingConfig, DotBfgsConfig, MultiStartConfig
    from dakota.spec.model import SingleConfig, NestedConfig
    from dakota.spec.responses import ResponsesConfig
    from dakota.spec.variables import VariablesConfig
    from dakota.study.di_construction_demo import INTERFACE, METHOD, RESPONSES, VARIABLES


@unittest.skipIf(ds is None, "native dakota.study extension is not built for this interpreter")
class FactoryTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        previous = Path.cwd()
        os.chdir(self.directory.name)
        self.addCleanup(os.chdir, previous)
        self.study = ds.Study()
        self.variables = self.study.variables(VARIABLES)
        self.response = self.study.responses(self.variables, RESPONSES)
        self.interface = self.study.interface(INTERFACE)
        self.model = self.study.model.simulation(self.variables, self.interface, self.response)
        self.iterator = self.study.method.sampling(self.model, METHOD)

    def cases(self):
        s = self.study
        cases = [
            (s.variables, (), VARIABLES, VariablesConfig, ds.Variables),
            (s.interface, (), INTERFACE, InterfaceConfig, ds.Interface),
            (s.responses, (self.variables,), RESPONSES, ResponsesConfig, ds.Response),
            (s.model.simulation, (self.variables, self.interface, self.response),
             {}, SingleConfig, ds.SimulationModel),
            (s.model.nested, (self.iterator, self.variables, self.response),
             {"sub_method_pointer": {"pointer": "inner"}}, NestedConfig, ds.NestedModel),
            (s.method.sampling, (self.model,), METHOD, SamplingConfig, ds.NonDLHSSampling),
            (s.method.multi_start, (self.iterator,),
             {"sub_method": {"method_pointer": "inner"}, "starting_points": [0.5, 0.5]},
             MultiStartConfig, ds.ConcurrentMetaIterator),
        ]
        if hasattr(s.method, "dot_bfgs"):
            # A gradient-based optimizer needs continuous design variables and
            # objective responses; don't use the sampling model for this case.
            v = s.variables(continuous_design={"count": 2, "initial_point": [0.5, 0.5],
                                               "lower_bounds": [0.0, 0.0],
                                               "upper_bounds": [1.0, 1.0]})
            r = s.responses(v, response_type={"objective_functions": {"count": 1}},
                            gradient_type={"numerical_gradients": {}},
                            hessian_type={"no_hessians": True})
            model = s.model.simulation(v, self.interface, r)
            cases.append((s.method.dot_bfgs, (model,), {}, DotBfgsConfig, ds.DOTOptimizer))
        return cases

    def test_all_configuration_forms(self):
        for factory, dependencies, data, schema, result_type in self.cases():
            for style in ("dict", "pydantic", "kwargs", "none_kwargs", "positional"):
                with self.subTest(factory=factory.__name__, style=style):
                    before = copy.deepcopy(data)
                    if style == "dict":
                        result = factory(*dependencies, config=data)
                    elif style == "pydantic":
                        result = factory(
                            *dependencies,
                            config=schema.model_validate(copy.deepcopy(data)))
                    elif style == "kwargs":
                        result = factory(*dependencies, **data)
                    elif style == "positional":
                        args = dependencies + ((None,) if factory.__name__ == "nested" else ())
                        result = factory(*args, data)
                    else:
                        result = factory(*dependencies, config=None, **data)
                    self.assertIsInstance(result, result_type)
                    self.assertEqual(data, before)

    def test_conflicts_and_invalid_config(self):
        class WrongConfig(BaseModel):
            nonexistent_configuration_field: bool = True

        for factory, dependencies, data, schema, _ in self.cases():
            for config in ({}, data, schema.model_validate(copy.deepcopy(data))):
                with self.subTest(factory=factory.__name__, config=config):
                    with self.assertRaisesRegex(TypeError, "pass config"):
                        factory(*dependencies, config=config, unexpected=True)
            for config in ([], False, "bad"):
                with self.subTest(factory=factory.__name__, config=config):
                    with self.assertRaisesRegex(TypeError, "config must"):
                        factory(*dependencies, config=config)
            with self.subTest(factory=factory.__name__, invalid_field=True):
                with self.assertRaises((ValueError, RuntimeError)):
                    factory(*dependencies, nonexistent_configuration_field=True)
                with self.assertRaises(TypeError):
                    factory(*dependencies, config=WrongConfig())

    def test_required_dependencies(self):
        for factory, dependencies, data, _, _ in self.cases():
            if not dependencies:
                continue
            with self.subTest(factory=factory.__name__, missing=True):
                with self.assertRaises(TypeError):
                    factory(config=data)
            for index in range(len(dependencies)):
                for invalid in (None, {}):
                    args = list(dependencies)
                    args[index] = invalid
                    with self.subTest(factory=factory.__name__, index=index, invalid=invalid):
                        with self.assertRaises(TypeError):
                            factory(*args, config=data)

    def test_keyword_dependencies_and_positional_config(self):
        s = self.study
        r = s.responses(variables=self.variables, config=RESPONSES)
        m = s.model.simulation(variables=self.variables, interface=self.interface, response=r)
        self.assertIsInstance(s.method.sampling(model=m, samples=10), ds.NonDLHSSampling)
        self.assertIsInstance(s.method.sampling(m, METHOD), ds.NonDLHSSampling)
        self.assertIsInstance(s.model.single({}, self.variables, self.interface, r), ds.SimulationModel)

    def test_nested_optional_interface_and_required_schema(self):
        nested_config = {"sub_method_pointer": {"pointer": "inner"}}
        for interface in (None, self.interface):
            with self.subTest(interface=interface):
                result = self.study.model.nested(
                    sub_iterator=self.iterator, variables=self.variables, response=self.response,
                    optional_interface=interface, config=nested_config)
                self.assertIsInstance(result, ds.NestedModel)
        with self.assertRaises(TypeError):
            self.study.model.nested(self.iterator, self.variables, self.response, nested_config)
        # The injected iterator replaces the corresponding input-file pointer;
        # API-mode validation supplies a sentinel for materialization.
        self.assertIsInstance(
            self.study.model.nested(
                self.iterator, self.variables, self.response),
            ds.NestedModel)
        self.assertIsInstance(
            self.study.method.multi_start(
                self.iterator, starting_points=[0.5, 0.5]),
            ds.ConcurrentMetaIterator)

    def test_additional_factories_validate_before_construction(self):
        from dakota.study._iterator_bindings import FACTORY_NAMES, ITERATOR_TYPES
        from dakota.study import _study

        for name in ITERATOR_TYPES:
            if hasattr(_study, name):
                self.assertIs(getattr(ds, name), getattr(_study, name))
        for name in FACTORY_NAMES:
            if not hasattr(self.study.method, name):
                continue  # Optional libraries control availability.
            factory = getattr(self.study.method, name)
            with self.subTest(factory=name):
                with self.assertRaises(TypeError):
                    factory(None)
                with self.assertRaisesRegex(TypeError, "pass config"):
                    factory(self.model, config={}, unexpected=True)
                with self.assertRaises(ValueError):
                    factory(self.model, not_a_dakota_option=True)

    def test_import_points_construction(self):
        from dakota.spec.method import ImportPointsConfig
        data = {"import_points_file": {"filename": "points.dat"}}
        Path("points.dat").write_text("%eval_id x1 x2 f\n1 0.25 0.75 1.0\n")
        factory = self.study.method.import_points
        for style in ("dict", "pydantic", "kwargs"):
            with self.subTest(style=style):
                if style == "kwargs":
                    result = factory(self.model, **data)
                else:
                    config = (ImportPointsConfig.model_validate(copy.deepcopy(data))
                              if style == "pydantic" else data)
                    result = factory(self.model, config=config)
                self.assertIsInstance(result, ds.NonDImportPoints)
                self.assertIsInstance(result, ds.Iterator)

    def test_ensemble_construction(self):
        from dakota.spec.model import EnsembleSurrogateConfig
        data = {"ensemble": {"truth_model_pointer": {
            "pointer": "truth", "approximation_models": ["approx"]}}}
        factory = self.study.model.ensemble_surrogate
        for style in ("dict", "pydantic", "kwargs", "legacy"):
            with self.subTest(style=style):
                # Fresh component models keep native ensemble state independent.
                truth = self.study.model.simulation(self.variables, self.interface, self.response)
                approx = self.study.model.simulation(self.variables, self.interface, self.response)
                dependencies = (truth, [approx], self.variables, self.response)
                if style == "legacy":
                    result = factory(data, *dependencies)
                elif style == "kwargs":
                    result = factory(*dependencies, **data)
                else:
                    config = (EnsembleSurrogateConfig.model_validate(
                        copy.deepcopy(data)) if style == "pydantic" else data)
                    result = factory(*dependencies, config=config)
                self.assertIsInstance(result, ds.EnsembleSurrModel)
                self.assertIsInstance(result, ds.Model)
        with self.assertRaises(TypeError):
            factory(None, [], self.variables, self.response, config=data)
        with self.assertRaises(TypeError):
            factory(self.model, [None], self.variables, self.response, config=data)

    def test_dot_feature(self):
        if not hasattr(self.study.method, "dot_bfgs"):
            self.skipTest("Dakota was built without DOT")
        self.assertTrue(hasattr(ds, "DOTOptimizer"))

    def test_seeded_sampling_equivalence(self):
        driver = os.environ.get("DAKOTA_TEXT_BOOK") or shutil.which("text_book")
        if not driver:
            self.skipTest("text_book driver not available; set DAKOTA_TEXT_BOOK or PATH")
        interface_data = copy.deepcopy(INTERFACE)
        interface_data["analysis_drivers"]["drivers"] = [str(Path(driver).resolve())]
        results = []
        for style in ("dict", "pydantic", "kwargs"):
            def create(factory, dependencies, data, schema):
                if style == "kwargs":
                    return factory(*dependencies, **data)
                config = (schema.model_validate(copy.deepcopy(data))
                          if style == "pydantic" else data)
                return factory(*dependencies, config=config)

            v = create(self.study.variables, (), VARIABLES, VariablesConfig)
            r = create(self.study.responses, (v,), RESPONSES, ResponsesConfig)
            i = create(self.study.interface, (), interface_data, InterfaceConfig)
            m = create(self.study.model.simulation, (v, i, r), {}, SingleConfig)
            sampling = create(self.study.method.sampling, (m,), METHOD, SamplingConfig)
            self.study.run(sampling)
            self.assertEqual(sampling.num_responses(), 10)
            results.append(sampling.first_response_value())
        for value in results[1:]:
            self.assertAlmostEqual(value, results[0], places=12)


if __name__ == "__main__":
    unittest.main()
