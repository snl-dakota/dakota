"""method_hybrid_embedded: global coliny EA with local pattern search, one study per process."""

from components import study_context, hybrid_sub_methods

study = study_context()
ga, ps = hybrid_sub_methods(study)
study.run(
    study.method.hybrid(
        [ga, ps],
        id_method="HE",
        strategy={
            "embedded": {
                "sub_method": {"global_method_pointer": "GA"},
                "local_sub_method": {"local_method_pointer": "PS"},
                "local_search_probability": 0.5,
            }
        },
    )
)
