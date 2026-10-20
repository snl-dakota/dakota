"""Aggregate two executed simulation models in an ensemble."""

from components import study_context, interface, design, response

study = study_context()
variables = design(study)
responses = response(study, variables)
low = study.model.simulation(variables, interface(study, id="low_driver"), responses)
high = study.model.simulation(variables, interface(study, id="high_driver"), responses)
model = study.model.ensemble_surrogate(variables, responses, ordered_models=[low, high])
study.run(
    study.method.vector_parameter_study(
        model, step_control={"step_vector": [0.1, 0.1]}, num_steps=2
    )
)
