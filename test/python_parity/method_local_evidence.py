"""method_local_evidence: one study per process."""

from components import study_context
from components import response_interface as driver

study = study_context()
variables = study.variables(
    continuous_interval_uncertain={
        "count": 2,
        "num_intervals": [1, 1],
        "interval_probabilities": [1.0, 1.0],
        "lower_bounds": [-1.0, -1.0],
        "upper_bounds": [1.0, 1.0],
    }
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
    study.method.local_evidence(
        model,
        config={
            "response_levels": {"values": [0.5]},
            "optimization_solver": {"nip": True},
        },
    )
)
