"""textbook_nips_s4: one study per process."""

from components import study_context, interface

study = study_context()
variables = study.variables(
    continuous_design={
        "count": 2,
        "initial_point": [0.9, 1.1],
        "lower_bounds": [0.5, -2.9],
        "upper_bounds": [5.8, 2.9],
        "descriptors": ["x1", "x2"],
    }
)
gradients = {"analytic_gradients": True}
responses = study.responses(
    variables,
    response_type={
        "objective_functions": {
            "count": 1,
            "nonlinear_inequality_constraints": {"count": 2},
        }
    },
    gradient_type=gradients,
    hessian_type={"analytic_hessians": True},
    **{}
)
model = study.model.simulation(
    variables, interface(study, named=not False, labeled=False), responses
)
options = {"max_iterations": 50, "convergence_tolerance": 1e-08}
options["merit_function"] = {"el_bakry": True}
study.run(study.method.optpp_newton(model, config=options))
