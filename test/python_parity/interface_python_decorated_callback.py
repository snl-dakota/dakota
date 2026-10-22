"""interface_python_decorated_callback: inject a @python_interface callback."""

import os
import sys

from components import study_context

sys.path.insert(0, os.getcwd())  # decorated_driver.py is copied to the work dir
import decorated_driver  # noqa: E402

study = study_context()
variables = study.variables(continuous_design={"count": 2, "descriptors": ["x1", "x2"]})
python_driver = {
    "drivers": ["decorated_driver.rosenbrock"],
    "interface_type": {"python": {}},
}
interface = study.interface(
    analysis_drivers=python_driver, callback=decorated_driver.rosenbrock
)
responses = study.responses(
    variables,
    {
        "response_type": {"objective_functions": {"count": 1}},
        "gradient_type": {"analytic_gradients": True},
        "hessian_type": {"no_hessians": True},
        "descriptors": ["f"],
    },
)
model = study.model.simulation(variables, interface, responses)
study.run(
    study.method.list_parameter_study(
        model,
        config={"source": {"list_of_points": [-1.2, 1.0, 0.0, 0.0, 1.0, 1.0]}},
    )
)
