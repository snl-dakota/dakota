"""Independent bound-object equivalent of dakota_pstudy list subtest."""

from common import simulation

study, model = simulation(False)
method = study.method.list_parameter_study(
    model,
    config={
        "source": {
            "list_of_points": [
                1.1,
                1.2,
                1.3,
                1.4,
                1.5,
                1.6,
                1.7,
                1.8,
                1.9,
                2.0,
                2.1,
                2.2,
            ]
        }
    },
)
study.run(method)
