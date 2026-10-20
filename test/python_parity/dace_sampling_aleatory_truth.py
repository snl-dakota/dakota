"""dace_sampling_aleatory_truth: outer LHS on a global surrogate built by DACE LHS.

Mirrors inputs/dace_sampling_aleatory_truth.in: a global surrogate is built by a
DACE LHS over a truth model with design + uniform_uncertain variables.  The DACE
iterator infers ALEATORY for the truth model.  An outer LHS on the surrogate also
infers ALEATORY for the surrogate itself (DataFitSurrModel), but does NOT
re-assign the truth model because the truth model's view was already set by the
DACE iterator and the DataFitSurrModel recursion skips the truth when a
dace_iterator is present.
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
    uniform_uncertain={"count": 1, "lower_bounds": [0.5], "upper_bounds": [1.5]},
)
responses = response(study, variables, kind="response_functions")
truth = study.model.simulation(variables, interface(study), responses)
dace = study.method.sampling(
    truth, samples=16, seed=1234, sample_type={"lhs": True}
)
model = study.model.global_surrogate(
    variables,
    responses,
    dace_iterator=dace,
    type={"polynomial": {"order": {"quadratic": True}}},
)
study.run(
    study.method.sampling(model, samples=10, seed=5678, sample_type={"lhs": True})
)
