"""Native Study factory contract tests. CTest requires an importable module."""

import copy
import inspect
import os
import re
from pathlib import Path
import shutil
import shlex
import sys
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
    from dakota.spec.method import (
        CenteredParameterStudyConfig,
        DotBfgsConfig,
        MultiStartConfig,
        SamplingConfig,
    )
    from dakota.spec.model import SingleConfig, NestedConfig
    from dakota.spec.responses import ResponsesConfig
    from dakota.spec.variables import VariablesConfig
    from dakota.study.di_construction_demo import INTERFACE, METHOD, RESPONSES, VARIABLES


def _callable_signature_text(function):
    """Return a native or pybind11-embedded callable signature."""
    try:
        return str(inspect.signature(function))
    except (TypeError, ValueError):
        # Some Python/pybind11 combinations do not publish __text_signature__
        # for instance methods. Pybind11 still prepends the generated signature
        # to the docstring, which is also where Sphinx obtains it.
        text_signature = getattr(function, "__text_signature__", None)
        if text_signature:
            return text_signature
        prefix = f"{function.__name__}("
        for line in (inspect.getdoc(function) or "").splitlines():
            if line.startswith(prefix):
                return line
        raise AssertionError(f"No signature is available for {function!r}")


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

    def test_asynchronous_system_interface(self):
        interface = self.study.interface(
            analysis_drivers={"drivers": ["text_book"],
                              "interface_type": {"system": {}}},
            concurrency={"asynchronous": {"evaluation_concurrency": 5}})
        model = self.study.model.simulation(self.variables, interface, self.response)
        self.assertIsInstance(model, ds.SimulationModel)

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
            (s.method.centered_parameter_study, (self.model,),
             {"step_vector": [0.1, 0.1], "steps_per_variable": [2, 2]},
             CenteredParameterStudyConfig, ds.ParamStudy),
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

    def test_public_api_documentation_and_signatures(self):
        for public_type in (
            ds.Study,
            ds.StudyConfig,
            ds.StudyOutputConfig,
            ds.StudyRunConfig,
            ds.MethodFactory,
            ds.ModelFactory,
            ds.Variables,
            ds.Response,
            ds.Interface,
            ds.Model,
            ds.Iterator,
            ds.NonDLHSSampling,
        ):
            with self.subTest(public_type=public_type.__name__):
                self.assertTrue(inspect.getdoc(public_type))

        documented_callables = (
            (ds.Study.variables, ("config", "kwargs")),
            (ds.Study.responses, ("variables", "config", "kwargs")),
            (ds.Study.interface, ("config", "kwargs")),
            (ds.Study.run, ("method",)),
            (ds.MethodFactory.sampling, ("model", "config", "kwargs")),
            (ds.ModelFactory.simulation,
             ("variables", "interface", "response", "config", "kwargs")),
            (ds.ModelFactory.ensemble_surrogate,
             ("variables", "response", "ordered_models", "truth_model",
              "approximation_models", "config", "kwargs")),
        )
        for function, parameters in documented_callables:
            with self.subTest(function=function.__qualname__):
                self.assertTrue(inspect.getdoc(function))
                signature = _callable_signature_text(function)
                for parameter in parameters:
                    self.assertIn(parameter, signature)

        ensemble_signature = _callable_signature_text(
            ds.ModelFactory.ensemble_surrogate)
        self.assertRegex(ensemble_signature, r"\*,\s*ordered_models\b")
        keyword_only_marker = ensemble_signature.index("*")
        for name in ("ordered_models", "truth_model", "approximation_models",
                     "config"):
            self.assertGreater(ensemble_signature.index(name), keyword_only_marker)
        ensemble_doc = inspect.getdoc(ds.ModelFactory.ensemble_surrogate)
        self.assertEqual(ensemble_doc.count("ensemble_surrogate("), 1)
        self.assertIn(":param truth_model:", ensemble_doc)
        self.assertIn(":ref:`ensemble surrogate options", ensemble_doc)

        uniform_method_factories = (
            "vector_parameter_study",
            "list_parameter_study",
            "centered_parameter_study",
            "multidim_parameter_study",
            "richardson_extrap",
            "local_interval_est",
            "global_interval_est",
            "efficient_global",
            "npsol_sqp",
            "nl2sol",
        )
        for name in uniform_method_factories:
            factory = getattr(ds.MethodFactory, name)
            with self.subTest(factory=name):
                signature = _callable_signature_text(factory)
                self.assertIn("model", signature)
                self.assertIn("config", signature)
                self.assertIn("kwargs", signature)
                self.assertNotIn("method", signature)
                self.assertTrue(inspect.getdoc(factory))

    def test_uniform_method_factories_validate_before_construction(self):
        names = (
            "vector_parameter_study",
            "list_parameter_study",
            "centered_parameter_study",
            "multidim_parameter_study",
            "richardson_extrap",
            "local_interval_est",
            "global_interval_est",
            "efficient_global",
            "npsol_sqp",
            "nl2sol",
        )
        for name in names:
            factory = getattr(self.study.method, name)
            with self.subTest(factory=name):
                with self.assertRaises(TypeError):
                    factory(None)
                with self.assertRaisesRegex(TypeError, "pass config"):
                    factory(self.model, config={}, unexpected=True)
                with self.assertRaises(ValueError):
                    factory(self.model, not_a_dakota_option=True)

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
        data = {"hierarchical_tagging": True}
        factory = self.study.model.ensemble_surrogate
        for dependency_form in ("ordered", "explicit"):
            for style in ("dict", "pydantic", "kwargs"):
                with self.subTest(dependency_form=dependency_form, style=style):
                    # Fresh models keep native ensemble state independent.
                    truth = self.study.model.simulation(
                        self.variables, self.interface, self.response)
                    approx = self.study.model.simulation(
                        self.variables, self.interface, self.response)
                    dependencies = ({"ordered_models": [approx, truth]}
                                    if dependency_form == "ordered" else
                                    {"truth_model": truth,
                                     "approximation_models": [approx]})
                    if style == "kwargs":
                        result = factory(self.variables, self.response,
                                         **dependencies, **data)
                    else:
                        config = (EnsembleSurrogateConfig.model_validate(
                            copy.deepcopy(data))
                            if style == "pydantic" else data)
                        result = factory(self.variables, self.response,
                                         **dependencies, config=config)
                    self.assertIsInstance(result, ds.EnsembleSurrModel)
                    self.assertIsInstance(result, ds.Model)

        truth = self.study.model.simulation(self.variables, self.interface, self.response)
        with self.assertRaisesRegex(TypeError, "exactly one"):
            factory(self.variables, self.response, config=data)
        with self.assertRaisesRegex(TypeError, "exactly one"):
            factory(self.variables, self.response, ordered_models=[truth],
                    truth_model=truth, config=data)
        with self.assertRaisesRegex(ValueError, "must not be empty"):
            factory(self.variables, self.response, ordered_models=[], config=data)
        with self.assertRaisesRegex(TypeError, "non-null"):
            factory(self.variables, self.response, ordered_models=[None], config=data)
        with self.assertRaisesRegex(TypeError, "truth_model"):
            factory(self.variables, self.response, approximation_models=[], config=data)
        with self.assertRaisesRegex(TypeError, "non-null"):
            factory(self.variables, self.response, truth_model=truth,
                    approximation_models=[None], config=data)
        with self.assertRaisesRegex(TypeError, "pass config"):
            factory(self.variables, self.response, ordered_models=[truth],
                    config={}, hierarchical_tagging=True)

    def test_dot_feature(self):
        if not hasattr(self.study.method, "dot_bfgs"):
            self.skipTest("Dakota was built without DOT")
        self.assertTrue(hasattr(ds, "DOTOptimizer"))

    def test_initial_point_and_nonlinear_response_layout(self):
        driver = os.environ.get("DAKOTA_TEXT_BOOK") or shutil.which("text_book")
        if not driver:
            self.skipTest("text_book driver not available; set DAKOTA_TEXT_BOOK or PATH")
        for domain in ("mixed", "relaxed"):
            with self.subTest(domain=domain):
                config = ds.StudyConfig()
                config.output.output_file = str(Path(domain + ".out").resolve())
                config.output.error_file = str(Path(domain + ".err").resolve())
                config.output.precision = 10
                study = ds.Study(config)
                variables = study.variables(domain={domain: True}, continuous_design={
                    "count": 3, "initial_point": [1., 1., 1.]})
                interface = study.interface(analysis_drivers={
                    "drivers": [str(Path(driver).resolve())], "interface_type": {"fork": {}}})
                response = study.responses(variables, response_type={"objective_functions": {
                    "count": 1, "nonlinear_inequality_constraints": {"count": 2}}},
                    gradient_type={"no_gradients": True}, hessian_type={"no_hessians": True})
                model = study.model.simulation(variables, interface, response)
                method = study.method.vector_parameter_study(model,
                    step_control={"step_vector": [0.1, 0.1, 0.1]}, num_steps=1)
                study.run(method)
                output = Path(config.output.output_file).read_text()
                for title, expected in (("parameters", [1., 1., 1.]),
                                        ("objective function", [0.]), ("constraint values", [0.5, 0.5])):
                    match = re.search(r"^<<<<< Best " + title +
                                      r"\s*=\n((?:[ \t]+[^\n]+\n)+)", output, re.M)
                    self.assertIsNotNone(match, title)
                    self.assertEqual([float(line.split()[0]) for line in match[1].splitlines()], expected)

    def test_ensemble_does_not_resize_child_responses(self):
        driver = Path("one_response.py").resolve()
        driver.write_text("import sys\nfrom pathlib import Path\n"
                          "Path(sys.argv[2]).write_text('1.0\\n')\n")
        interface = self.study.interface(analysis_drivers={
            "drivers": [shlex.quote(sys.executable) + " " + shlex.quote(str(driver))],
            "interface_type": {"fork": {}}})
        children = [self.study.model.simulation(self.variables, interface, self.response)
                    for _ in range(2)]
        ensemble = self.study.model.ensemble_surrogate(
            self.variables, self.response, ordered_models=children)
        sampling = self.study.method.sampling(children[0], samples=2, seed=1234)
        self.study.run(sampling)
        self.assertEqual(sampling.num_responses(), 2)
        self.assertEqual(sampling.first_response_value(), 1.0)

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
