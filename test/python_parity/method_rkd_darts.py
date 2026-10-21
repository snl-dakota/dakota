"""method_rkd_darts: one study per process."""

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
truth = study.model.simulation(variables, driver(study), response)
dace = study.method.sampling(truth, samples=12, seed=1234, sample_type={"lhs": True})
model = study.model.global_surrogate(
    variables,
    response,
    dace_iterator=dace,
    type={"gaussian_process": {"gp_implementation": {"surfpack": {}}}},
)
study.run(
    study.method.rkd_darts(
        model,
        config={
            "response_levels": {"values": [0.5]},
            "build_samples": 12,
            "seed": 1234,
            "samples_on_emulator": 32,
        },
    )
)
