"""method_hybrid_sequential: coliny EA then pattern search, one study per process."""

from components import study_context, hybrid_sub_methods

study = study_context()
ga, ps = hybrid_sub_methods(study)
study.run(
    study.method.hybrid_sequential(
        [ga, ps],
        id_method="HS",
    )
)
