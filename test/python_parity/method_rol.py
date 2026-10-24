"""method_rol: one study per process."""

from components import study_context, interface, response

study = study_context()
design = {"count": 2, "initial_point": [0.9, 1.1], "descriptors": ["x1", "x2"]}
design.update(lower_bounds=[0.5, 0.5], upper_bounds=[1.5, 1.5])
variables = study.variables(continuous_design=design)
responses = response(study, variables, gradients="analytic_gradients")
model = study.model.simulation(variables, interface(study), responses)
study.run(
    study.method.rol(
        model,
        config={
            "gradient_tolerance": 1.0e-3,
            "constraint_tolerance": 1.0e-3,
            "variable_tolerance": 1.0e-3,
            "max_iterations": 10,
        },
    )
)
