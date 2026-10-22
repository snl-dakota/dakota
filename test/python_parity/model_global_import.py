"""model_global_import: import a polynomial surrogate exported by the setup study."""

from components import study_context, design, response

study = study_context()
variables = design(study)
responses = response(study, variables)
model = study.model.global_surrogate(
    variables,
    responses,
    type={
        "polynomial": {
            "order": {"quadratic": True},
            "import_model": {
                "filename_prefix": "parity_poly",
                "import_format": {"text_archive": True},
            },
        }
    },
)
study.run(
    study.method.vector_parameter_study(
        model, step_control={"step_vector": [0.1, 0.1]}, num_steps=2
    )
)
