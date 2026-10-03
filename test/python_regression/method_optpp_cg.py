"""method_optpp_cg: one study per process."""

from components import study_context, interface

study = study_context()
design = {"count": 2, "initial_point": [0.9, 1.1], "descriptors": ["x1", "x2"]}
variables = study.variables(continuous_design=design)
gradients = "analytic_gradients"
responses = study.responses(
    variables,
    {
        "response_type": {"objective_functions": {"count": 1}},
        "gradient_type": {gradients: True},
        "hessian_type": {"no_hessians": True},
    },
)
model = study.model.simulation(variables, interface(study), responses)
study.run(
    study.method.optpp_cg(
        model,
        config={
            "max_iterations": 5,
            "max_function_evaluations": 25,
            "convergence_tolerance": 0.0001,
        },
    )
)
