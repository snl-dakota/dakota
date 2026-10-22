"""Rosenbrock analysis driver written with the @python_interface decorator."""

import dakota.interfacing as di


@di.python_interface
def rosenbrock(params, results):
    x1, x2 = params["x1"], params["x2"]
    response = results["f"]
    if response.asv.function:
        response.function = 100.0 * (x2 - x1 * x1) ** 2 + (1.0 - x1) ** 2
    if response.asv.gradient:
        response.gradient = [
            -400.0 * x1 * (x2 - x1 * x1) - 2.0 * (1.0 - x1),
            200.0 * (x2 - x1 * x1),
        ]
    return results
