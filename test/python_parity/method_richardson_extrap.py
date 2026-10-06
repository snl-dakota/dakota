"""method_richardson_extrap: one study per process."""

from components import study_context
from components import response_interface as driver

study = study_context()
variables = study.variables(
    continuous_state={"count": 1, "initial_state": [8.0], "descriptors": ["resolution"]}
)
response = study.responses(
    variables,
    {
        "response_type": {"response_functions": {"count": 1}},
        "gradient_type": {"no_gradients": True},
        "hessian_type": {"no_hessians": True},
    },
)
model = study.model.simulation(variables, driver(study, "richardson"), response)
study.run(
    study.method.richardson_extrap(
        model, config={"mode": {"estimate_order": True}, "refinement_rate": 2.0}
    )
)
