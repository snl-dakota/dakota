"""method_nlssol_sqp: one study per process."""

from components import study_context, response_interface as driver

study = study_context()
variables = study.variables(
    continuous_design={
        "count": 2,
        "initial_point": [0.9, 1.1],
        "descriptors": ["x1", "x2"],
    }
)
response = study.responses(
    variables,
    {
        "response_type": {"calibration_terms": {"count": 2}},
        "gradient_type": {"analytic_gradients": True},
        "hessian_type": {"no_hessians": True},
    },
)
model = study.model.simulation(variables, driver(study, "least_squares"), response)
study.run(
    study.method.nlssol_sqp(
        model, config={"max_iterations": 5, "max_function_evaluations": 20}
    )
)
