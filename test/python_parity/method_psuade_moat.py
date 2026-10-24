"""method_psuade_moat: one study per process.

Samples must be a multiple of (num_vars + 1). With 2 variables:
  samples = n_paths * 3 => use 4 paths => samples = 12.
"""

from components import study_context, interface

study = study_context()
variables = study.variables(
    continuous_design={
        "count": 2,
        "lower_bounds": [0.0, 0.0],
        "upper_bounds": [1.0, 1.0],
        "descriptors": ["x1", "x2"],
    }
)
responses = study.responses(
    variables,
    {
        "response_type": {"objective_functions": {"count": 1}},
        "gradient_type": {"no_gradients": True},
        "hessian_type": {"no_hessians": True},
    },
)
model = study.model.simulation(variables, interface(study), responses)
study.run(
    study.method.psuade_moat(
        model,
        config={
            "samples": 12,
            "partitions": [3, 3],
            "seed": 500,
        },
    )
)
