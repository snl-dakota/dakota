"""method_multilevel_multifidelity_sampling: one study per process."""

from components import study_context
from components import response_interface as driver

study = study_context()
variables = study.variables(
    uniform_uncertain={
        "count": 2,
        "lower_bounds": [-1.0, -1.0],
        "upper_bounds": [1.0, 1.0],
    },
    active={"aleatory": True},
    discrete_state_set={
        "integer": {
            "count": 1,
            "elements_per_variable": [2],
            "elements": [1, 2],
            "initial_state": [1],
            "descriptors": ["level"],
        }
    },
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
    options.update(
        solution_level_control="level",
        cost_model={"solution_level_cost": [cost, 2 * cost]},
    )
    children.append(
        study.model.simulation(
            variables, driver(study, mode, mode + "_driver"), response, **options
        )
    )
model = study.model.ensemble_surrogate(variables, response, ordered_models=children)
study.run(
    study.method.multilevel_multifidelity_sampling(
        model,
        config={
            "pilot_samples": [8],
            "max_iterations": 1,
            "max_function_evaluations": 24,
            "seed_sequence": [1234],
        },
    )
)
