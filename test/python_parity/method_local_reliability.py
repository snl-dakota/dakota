"""method_local_reliability: one study per process."""

from components import study_context
from components import response_interface as driver

study = study_context()
variables = study.variables(
    normal_uncertain={"count": 2, "means": [0.0, 0.0], "std_deviations": [1.0, 1.0]}
)
response = study.responses(
    variables,
    {
        "response_type": {"response_functions": {"count": 1}},
        "gradient_type": {"analytic_gradients": True},
        "hessian_type": {"no_hessians": True},
    },
)
model = study.model.simulation(variables, driver(study, "high"), response)
study.run(
    study.method.local_reliability(model, config={"response_levels": {"values": [0.5]}})
)
