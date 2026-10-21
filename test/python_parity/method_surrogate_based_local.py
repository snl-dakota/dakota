"""method_surrogate_based_local: trust-region SBO on a DACE global surrogate."""

from components import study_context, dace_global_surrogate

study = study_context()
model = dace_global_surrogate(study)
study.run(
    study.method.surrogate_based_local(
        model,
        id_method="SBLO",
        sub_method={"method_name": "coliny_pattern_search"},
        max_iterations=3,
        soft_convergence_limit=2,
        trust_region={
            "initial_size": [0.2],
            "contraction_factor": 0.5,
            "expansion_factor": 1.5,
        },
    )
)
