"""method_hybrid_sequential: coliny EA then pattern search, one study per process."""

from components import study_context, hybrid_sub_methods

study = study_context()
ga, ps = hybrid_sub_methods(study)
study.run(
    study.method.hybrid(
        [ga, ps],
        id_method="HS",
        strategy={"sequential": {"sub_method": {"method_pointer_list": ["GA", "PS"]}}},
    )
)
