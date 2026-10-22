"""interface_python_callback: direct Python interface with an injected callable."""

import os
import sys

from components import study_context

sys.path.insert(0, os.getcwd())  # driver_text_book.py is copied to the work dir
import driver_text_book  # noqa: E402

study = study_context()
variables = study.variables(
    continuous_design={
        "count": 3,
        "initial_point": [0.0, 0.0, 0.0],
        "upper_bounds": [10.0, 10.0, 10.0],
        "lower_bounds": [-10.0, -10.0, -10.0],
        "descriptors": ["x1", "x2", "x3"],
    },
    discrete_state_set={
        "integer": {
            "count": 3,
            "elements_per_variable": [2, 2, 2],
            "elements": [1, 2, 3, 4, 5, 6],
            "initial_state": [2, 4, 6],
            "descriptors": ["z1", "z2", "z3"],
        },
        "string": {
            "count": 1,
            "elements_per_variable": [4],
            "elements": ["four", "one", "three", "two"],
            "initial_state": ["two"],
            "descriptors": ["s1"],
        },
        "real": {
            "count": 2,
            "elements_per_variable": [2, 2],
            "elements": [1.2, 2.2, 3.2, 4.2],
            "initial_state": [1.2, 3.2],
            "descriptors": ["y1", "y2"],
        },
    },
)
python_driver = {
    "drivers": ["driver_text_book.text_book"],
    "interface_type": {"python": {}},
    "analysis_components": ["a", "b"],
}
interface = study.interface(
    analysis_drivers=python_driver, callback=driver_text_book.text_book
)
responses = study.responses(
    variables,
    {
        "response_type": {
            "objective_functions": {
                "count": 1,
                "nonlinear_inequality_constraints": {
                    "count": 2,
                    "upper_bounds": [0.0, 0.0],
                },
            },
        },
        "gradient_type": {"analytic_gradients": True},
        "hessian_type": {"analytic_hessians": True},
        "descriptors": ["f1", "c1", "c2"],
        "metadata": ["m1", "m2"],
    },
)
model = study.model.simulation(variables, interface, responses)
study.run(
    study.method.list_parameter_study(
        model, config={"source": {"list_of_points": [0.0, 0.0, 0.0]}}
    )
)
