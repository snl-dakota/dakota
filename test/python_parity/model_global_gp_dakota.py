"""model_global_gp_dakota: gp dakota surrogate with non-default options."""

from components import study_context, design, response

study = study_context()
variables = design(study)
responses = response(study, variables)
model = study.model.global_surrogate(
    variables,
    responses,
    type={'gaussian_process': {'gp_implementation': {'dakota': {'trend': {'constant': True}}}}},
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
