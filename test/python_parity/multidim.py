"""dakota_pstudy.in s5: all 60 grid evaluations."""

# s5 adds bounds to the common pilot variables.
from components import study_context, response

study = study_context()
variables = study.variables(
    continuous_design={
        "count": 3,
        "initial_point": [1.0, 1.0, 1.0],
        "lower_bounds": [-10.0, -10.0, -10.0],
        "upper_bounds": [10.0, 10.0, 10.0],
    }
)
interface = study.interface(
    analysis_drivers={"drivers": ["text_book"], "interface_type": {"system": {}}},
    concurrency={"asynchronous": {"evaluation_concurrency": 5}},
)
responses = response(study, variables, constraints=2)
model = study.model.simulation(variables, interface, responses)
study.run(study.method.multidim_parameter_study(model, partitions=[2, 3, 4]))
