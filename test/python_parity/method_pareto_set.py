"""method_pareto_set: one study per process.

Wraps an optpp_q_newton sub-optimizer in a Pareto-set meta-iterator with
three weight combinations for a two-objective problem.
"""

from components import study_context, interface

study = study_context()
design = {"count": 2, "initial_point": [0.9, 1.1], "descriptors": ["x1", "x2"]}
design.update(lower_bounds=[0.5, 0.5], upper_bounds=[1.5, 1.5])
variables = study.variables(continuous_design=design)
responses = study.responses(
    variables,
    {
        "response_type": {"objective_functions": {"count": 2}},
        "gradient_type": {"analytic_gradients": True},
        "hessian_type": {"no_hessians": True},
    },
)
model = study.model.simulation(variables, interface(study), responses)
inner = study.method.optpp_q_newton(
    model,
    id_method="NLP",
    max_iterations=10,
    max_function_evaluations=50,
)
outer = study.method.pareto_set(
    inner,
    id_method="PS",
    sub_method={"method_pointer": "NLP"},
    weight_sets=[1.0, 0.0, 0.0, 1.0, 0.5, 0.5],
)
study.run(outer)
