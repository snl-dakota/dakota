"""textbook_s0: one study per process."""

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
gradients = {
    "numerical_gradients": {
        "method_source": {"dakota": {}},
        "interval_type": {"central": True},
        "fd_step_size": [0.0001],
    }
}
responses = study.responses(
    variables,
    response_type={
        "objective_functions": {
            "count": 1,
            "nonlinear_inequality_constraints": {"count": 2},
        }
    },
    gradient_type=gradients,
    hessian_type={"no_hessians": True},
    **{"descriptors": ["f", "c1", "c2"]}
)
model = study.model.simulation(
    variables, interface(study, named=not True, labeled=True), responses
)
options = {"max_iterations": 50, "convergence_tolerance": 0.0001}
study.run(study.method.dot_mmfd(model, config=options))
