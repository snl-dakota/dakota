"""model_global: one study per process."""

from components import study_context, interface, design, response

study = study_context()
variables = design(study)
responses = response(study, variables)
model = study.model.global_surrogate(
    variables,
    responses,
    type={"polynomial": {"order": {"quadratic": True}}},
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
