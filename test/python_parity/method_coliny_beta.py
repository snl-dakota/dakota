"""method_coliny_beta: one study per process."""

from components import study_context
from components import response_interface as driver

study = study_context()
variables = study.variables(
    continuous_design={
        "count": 2,
        "initial_point": [0.9, 1.1],
        "descriptors": ["x1", "x2"],
        "lower_bounds": [0.5, 0.5],
        "upper_bounds": [1.5, 1.5],
    }
)
response = study.responses(
    variables,
    {
        "response_type": {"objective_functions": {"count": 1}},
        "gradient_type": {"no_gradients": True},
        "hessian_type": {"no_hessians": True},
    },
)
model = study.model.simulation(variables, driver(study, "optimization"), response)
study.run(
    study.method.coliny_beta(
        model,
        config={
            "beta_solver_name": "sco:PatternSearch",
            "max_iterations": 3,
            "max_function_evaluations": 16,
            "seed": 1234,
        },
    )
)
