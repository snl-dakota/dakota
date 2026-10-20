"""Independent bound-object equivalent of dakota_pstudy centered subtest."""

from common import simulation

study, model = simulation(False)
method = study.method.centered_parameter_study(
    model, config={"step_vector": [0.05, 0.05, 0.05], "steps_per_variable": [5]}
)
study.run(method)
