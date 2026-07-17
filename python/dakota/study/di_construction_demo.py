"""DI construction demo using the :mod:`dakota.study` Python bindings.

This mirrors ``src/di_construction_demo.cpp``: it assembles a small sampling
study from keyword-wrapped JSON-shaped dictionaries, constructs Dakota
components through a ``Study`` context, and runs the resulting method.
"""

from __future__ import annotations

try:
    from . import Response, Study, StudyConfig, Variables
except ImportError:
    # Allow direct execution by path during development, e.g.
    # ``python python/dakota/study/di_construction_demo.py``.
    from dakota.study import Response, Study, StudyConfig, Variables


METHOD = {
    "sampling": {
        "sample_type": {"lhs": True},
        "samples": 10,
        "seed": 1234,
    }
}

VARIABLES = {
    "active": {"all": True},
    "uniform_uncertain": {
        "count": 2,
        "descriptors": ["x1", "x2"],
        "lower_bounds": [0.0, 0.0],
        "upper_bounds": [1.0, 1.0],
    },
}

RESPONSES = {
    "response_type": {"response_functions": {"count": 1}},
    "descriptors": ["f"],
    "gradient_type": {"no_gradients": True},
    "hessian_type": {"no_hessians": True},
}

INTERFACE = {
    "analysis_drivers": {
        "drivers": ["text_book"],
        "interface_type": {
            "fork": {
                "parameters_file": "params.in",
                "results_file": "results.out",
                "file_save": True,
            }
        },
        "deactivate": {"restart_file": True},
    }
}

MODEL = {}


def main() -> None:
    config = StudyConfig()
    config.output.precision = 12
    config.output.output_file = "di_construction_demo.out"
    config.output.error_file = "di_construction_demo.err"
    config.output.write_restart = "di_construction_demo.rst"
    config.output.results_output = True
    config.output.results_output_file = "di_construction_demo_results"

    study = Study(config)

    print("Constructing DI study components...")
    print(f"Configured output precision: {config.output.precision}")

    variables = Variables(VARIABLES)
    response = Response(RESPONSES, variables)
    interface = study.interface(INTERFACE)
    model = study.model.simulation(MODEL, variables, interface, response)
    sampling = study.method.sampling(METHOD, model)

    print("Running sampling study...")
    study.run(sampling)

    print("Completed DI study.")
    print(f"Samples evaluated: {sampling.num_responses()}")
    if sampling.num_responses() > 0:
        print(f"First response value: {sampling.first_response_value()}")


if __name__ == "__main__":
    main()
