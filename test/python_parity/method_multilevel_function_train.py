"""method_multilevel_function_train: one study per process."""

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
mode = "high"
children = []
for mode, cost in [("low", 0.1), ("high", 1.0)]:
    options = {"cost_model": {"solution_level_cost": [cost]}}
    children.append(
        study.model.simulation(
            variables, driver(study, mode, mode + "_driver"), response, **options
        )
    )
model = study.model.ensemble_surrogate(variables, response, ordered_models=children)
study.run(
    study.method.multilevel_function_train(
        model,
        config={
            "start_order_sequence": {"sequence": [3, 3]},
            "start_rank_sequence": [2, 2],
            "collocation_points_sequence": [12, 12],
            "seed_sequence": [1234, 5678],
            "max_iterations": 1,
        },
    )
)
