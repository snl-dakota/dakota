"""method_hybrid_collaborative: coliny EA and pattern search, one study per process."""

from components import study_context, hybrid_sub_methods

study = study_context()
ga, ps = hybrid_sub_methods(study)
study.run(
    study.method.hybrid(
        [ga, ps],
        id_method="HC",
        strategy={"collaborative": {"sub_method": {"method_pointer_list": ["GA", "PS"]}}},
    )
)
