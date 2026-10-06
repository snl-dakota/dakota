"""Import precomputed responses without invoking the interface."""

from components import study_context, interface, response

study = study_context()
variables = study.variables(
    uniform_uncertain={
        "count": 2,
        "lower_bounds": [0.0, 0.0],
        "upper_bounds": [1.0, 1.0],
    }
)
responses = response(study, variables, kind="response_functions")
model = study.model.simulation(variables, interface(study), responses)
study.run(
    study.method.import_points(
        model, import_points_file={"filename": "dakota_import_points.dat"}
    )
)
