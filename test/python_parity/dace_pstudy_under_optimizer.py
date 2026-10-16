"""dace_pstudy_under_optimizer: optimizer on a global surrogate built by DACE LHS.

Mirrors inputs/dace_pstudy_under_optimizer.in: a polynomial quadratic global
surrogate is built by a DACE LHS over a truth model with continuous_design and
uniform_uncertain variables.  Both the DACE sampler and the outer optpp_pds
optimizer use objective functions, so Variables.infer_view_spec returns DESIGN
at level 2 (response-type check) before the method-name check is ever reached.
Both truth and surrogate therefore acquire view DESIGN.  DataFitSurrModel does
NOT recurse into the truth model (dace_iterator is present), and the surrogate/
truth view compatibility check passes as Case 1 (same view, matching counts).
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
    uniform_uncertain={
        "count": 1,
        "lower_bounds": [0.5],
        "upper_bounds": [1.5],
    },
)
responses = response(study, variables)
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
    study.method.optpp_pds(
        model, config={"max_iterations": 5, "max_function_evaluations": 25}
    )
)
