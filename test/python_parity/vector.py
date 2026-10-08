"""Independent bound-object equivalent of dakota_pstudy vector subtest."""

from common import simulation

study, model = simulation(True)
method = study.method.vector_parameter_study(
    model, config={"step_control": {"step_vector": [0.1, 0.1, 0.1]}, "num_steps": 4}
)
study.run(method)
