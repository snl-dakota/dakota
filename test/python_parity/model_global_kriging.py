"""model_global_kriging: kriging surrogate with non-default options."""

from components import study_context, design, response

study = study_context()
variables = design(study)
responses = response(study, variables)
model = study.model.global_surrogate(
    variables,
    responses,
    type={'gaussian_process': {'gp_implementation': {'surfpack': {'trend': {'linear': True}, 'max_trials': 50, 'nugget': {'nugget': 1e-06}}}}},
    import_build_points_file={
        "filename": "build_points.dat",
        "format": {"freeform": True},
    },
)
study.run(
    study.method.vector_parameter_study(
        model, step_control={"step_vector": [0.1, 0.1]}, num_steps=2
    )
)
