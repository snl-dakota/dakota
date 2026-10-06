"""dakota_multistart_ie.in s0: four ten-point vector studies."""

from components import study_context, interface, response

study = study_context()
variables = study.variables(continuous_design={"count": 1})
responses = response(study, variables)
model = study.model.simulation(variables, interface(study), responses)
inner = study.method.vector_parameter_study(
    model, id_method="PS", step_control={"step_vector": [0.01]}, num_steps=9
)
outer = study.method.multi_start(
    inner,
    id_method="MS",
    sub_method={"method_pointer": "PS"},
    starting_points=[-1.0, -0.5, 0.5, 1.0],
)
study.run(outer)
