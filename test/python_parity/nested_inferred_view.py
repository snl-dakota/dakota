"""nested_inferred_view: inner LHS infers ALEATORY view without an explicit active= line.

Mirrors inputs/nested_inferred_view.in: a three-point outer vector parameter study
executes an inner seeded LHS sampler on a model with design + uniform_uncertain
variables.  The inner sampling iterator infers the ALEATORY view automatically
(matching legacy freeform behaviour) without needing active={"uncertain": True}.
"""

from components import study_context, interface, design, response

study = study_context()
outer_variables = design(study)
outer_response = response(study, outer_variables)
inner_variables = study.variables(
    continuous_design={
        "count": 2,
        "initial_point": [1.0, 1.0],
        "lower_bounds": [0.5, 0.5],
        "upper_bounds": [1.5, 1.5],
        "descriptors": ["x1", "x2"],
    },
    uniform_uncertain={"count": 1, "lower_bounds": [0.5], "upper_bounds": [1.5]},
)
inner_response = response(study, inner_variables, kind="response_functions")
inner_model = study.model.simulation(
    inner_variables, interface(study, id="driver"), inner_response
)
sampler = study.method.sampling(
    inner_model, samples=10, seed=1234, fixed_seed=True, sample_type={"lhs": True}
)
model = study.model.nested(
    sampler,
    outer_variables,
    outer_response,
    sub_method_pointer={
        "primary_variable_mapping": ["x1", "x2"],
        "primary_response_mapping": [1.0, 0.0],
    },
)
study.run(
    study.method.vector_parameter_study(
        model, step_control={"step_vector": [0.1, 0.1]}, num_steps=2
    )
)
