"""method_conmin_mfd: one study per process."""

from components import study_context, interface, response

study = study_context()
design = {"count": 2, "initial_point": [0.9, 1.1], "descriptors": ["x1", "x2"]}
design.update(lower_bounds=[0.5, 0.5], upper_bounds=[1.5, 1.5])
variables = study.variables(continuous_design=design)
responses = response(study, variables, gradients="analytic_gradients")
model = study.model.simulation(variables, interface(study), responses)
options = {"max_iterations": 5, "convergence_tolerance": 0.0001}
study.run(study.method.conmin_mfd(model, config=options))
