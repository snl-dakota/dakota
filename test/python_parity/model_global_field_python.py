"""model_global_field_python: experimental Python surrogate over field responses.

Mirrors dakota_python_field_surrogate.in s1: an LHS DACE over a direct Python
interface builds one scalar surface per scalar response and one field
surface per field response.
"""

import os
import sys

from components import study_context

sys.path.insert(0, os.getcwd())  # surrogate_polynomial.py is copied to the work dir

study = study_context()
variables = study.variables(
    uniform_uncertain={
        "count": 2,
        "lower_bounds": [-2.0, -2.0],
        "upper_bounds": [2.0, 2.0],
        "descriptors": ["x1", "x2"],
    }
)
interface = study.interface(
    id_interface="I1",
    analysis_drivers={
        "drivers": ["surrogate_polynomial.funcs"],
        "interface_type": {"python": {}},
    },
)
responses = study.responses(
    variables,
    {
        "response_type": {
            "response_functions": {
                "count": 4,
                "scalar_responses": 2,
                "field_responses": {"count": 2, "lengths": [2, 3]},
            }
        },
        "gradient_type": {"no_gradients": True},
        "hessian_type": {"no_hessians": True},
    },
)
truth = study.model.simulation(variables, interface, responses)
dace = study.method.sampling(
    truth, id_method="DACE", sample_type={"lhs": True}, samples=100, seed=5034
)
model = study.model.global_surrogate(
    variables,
    responses,
    dace_iterator=dace,
    reuse_points={"reuse_domain": {"all": True}},
    type={
        "experimental_python": {
            "class_path_and_name": "surrogate_polynomial.Surrogate"
        }
    },
)
study.run(
    study.method.sampling(
        model,
        id_method="UQ",
        sample_type={"lhs": True},
        samples=100,
        seed=98765,
        output={"verbose": True},
    )
)
