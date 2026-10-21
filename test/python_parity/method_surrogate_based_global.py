"""method_surrogate_based_global: global SBO on a DACE global surrogate."""

from components import study_context, dace_global_surrogate

study = study_context()
model = dace_global_surrogate(study)
study.run(
    study.method.surrogate_based_global(
        model,
        id_method="SBGO",
        sub_method={"method_name": "coliny_pattern_search"},
        replace_points=True,
        max_iterations=2,
    )
)
