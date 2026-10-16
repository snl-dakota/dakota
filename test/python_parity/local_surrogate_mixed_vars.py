"""local_surrogate_optimizer: optimizer on a local surrogate with separate truth responses.

Mirrors inputs/local_surrogate_mixed_vars.in: a Taylor-series local surrogate is
built over a truth simulation model.  The outer optpp_pds optimizer infers DESIGN
for the local surrogate (objective functions → DESIGN).  Because the local
surrogate has no DACE iterator, DataFitSurrModel.assign_inferred_view recurses
into the truth model, which therefore also becomes DESIGN — verifying that the
recursion propagates the inferred view through the surrogate hierarchy.
"""

from components import study_context, interface, response

study = study_context()
variables = study.variables(
    continuous_design={
        "count": 2,
        "initial_point": [1.0, 1.0],
        "lower_bounds": [0.5, 0.5],
        "upper_bounds": [1.5, 1.5],
        "descriptors": ["x1", "x2"],
    },
)
responses = response(study, variables)
truth_response = response(study, variables, gradients="analytic_gradients")
truth = study.model.simulation(variables, interface(study, id="driver"), truth_response)
model = study.model.local_surrogate(truth, variables, responses, taylor_series=True)
study.run(
    study.method.optpp_pds(
        model, config={"max_iterations": 5, "max_function_evaluations": 25}
    )
)
