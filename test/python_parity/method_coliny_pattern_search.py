"""method_coliny_pattern_search: one study per process."""

from components import study_context, interface, response

study = study_context()
design = {"count": 2, "initial_point": [0.9, 1.1], "descriptors": ["x1", "x2"]}
design.update(lower_bounds=[0.5, 0.5], upper_bounds=[1.5, 1.5])
variables = study.variables(continuous_design=design)
responses = response(study, variables, gradients="no_gradients")
model = study.model.simulation(variables, interface(study), responses)
options = {
    "max_iterations": 5,
    "max_function_evaluations": 25,
    "seed": 1234,
    "initial_delta": 0.1,
    "variable_tolerance": 0.0001,
}
study.run(study.method.coliny_pattern_search(model, config=options))
