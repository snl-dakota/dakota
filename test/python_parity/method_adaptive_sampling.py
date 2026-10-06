"""method_adaptive_sampling: one study per process."""

from components import study_context
from components import response_interface as driver

study = study_context()
variables = study.variables(
    uniform_uncertain={
        "count": 2,
        "lower_bounds": [-1.0, -1.0],
        "upper_bounds": [1.0, 1.0],
    }
)
response = study.responses(
    variables,
    {
        "response_type": {"response_functions": {"count": 1}},
        "gradient_type": {"no_gradients": True},
        "hessian_type": {"no_hessians": True},
    },
)
model = study.model.simulation(variables, driver(study, "high"), response)
study.run(
    study.method.adaptive_sampling(
        model,
        config={
            "initial_samples": 8,
            "seed": 1234,
            "max_iterations": 1,
            "samples_on_emulator": 16,
            "refinement_samples": [1],
        },
    )
)
