"""method_branch_and_bound_method_pointer: injected sub-problem minimizer."""

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
    },
    domain={"relaxed": True},
    discrete_design_range={
        "count": 1,
        "initial_point": [1],
        "lower_bounds": [0],
        "upper_bounds": [2],
    },
)
response = study.responses(
    variables,
    {
        "response_type": {"objective_functions": {"count": 1}},
        "gradient_type": {"analytic_gradients": True},
        "hessian_type": {"no_hessians": True},
    },
)
model = study.model.simulation(variables, driver(study, "optimization"), response)
method = study.method.optpp_q_newton(
    model, id_method="NLP", max_iterations=10, max_function_evaluations=50
)
study.run(
    study.method.branch_and_bound(
        method=method, config={"sub_method": {"method_pointer": "NLP"}}
    )
)
