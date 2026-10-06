"""model_multipoint: one study per process."""

from components import study_context, interface, design, response

study = study_context()
variables = design(study)
responses = response(study, variables)
truth_response = response(study, variables, gradients="analytic_gradients")
truth = study.model.simulation(variables, interface(study, id="driver"), truth_response)
model = study.model.multipoint_surrogate(
    truth, variables, responses, type={"tana": True}
)
study.run(
    study.method.vector_parameter_study(
        model, step_control={"step_vector": [0.1, 0.1]}, num_steps=2
    )
)
