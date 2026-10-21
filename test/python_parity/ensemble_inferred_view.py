"""ensemble_inferred_view: multilevel_sampling on an ensemble with design + aleatory variables.

Mirrors inputs/ensemble_inferred_view.in: two simulation models sharing design +
uniform_uncertain variables are gathered into an ensemble surrogate and run under
multilevel_sampling.  No active= keyword is used anywhere.  The multilevel_sampling
iterator infers ALEATORY for the ensemble and both child simulation models
automatically (matching legacy freeform behaviour).
"""

from components import study_context
from components import response_interface as driver

study = study_context()
variables = study.variables(
    continuous_design={
        "count": 2,
        "initial_point": [1.0, 1.0],
        "lower_bounds": [0.5, 0.5],
        "upper_bounds": [1.5, 1.5],
        "descriptors": ["x1", "x2"],
    },
    uniform_uncertain={
        "count": 2,
        "lower_bounds": [-1.0, -1.0],
        "upper_bounds": [1.0, 1.0],
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
    study.method.multilevel_sampling(
        model,
        config={
            "pilot_samples": [8],
            "max_iterations": 1,
            "max_function_evaluations": 24,
            "seed_sequence": [1234],
        },
    )
)
