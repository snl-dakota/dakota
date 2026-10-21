"""method_mesh_adaptive_search: one study per process."""

from components import study_context, interface

study = study_context()
design = {"count": 2, "initial_point": [0.9, 1.1], "descriptors": ["x1", "x2"]}
design.update(lower_bounds=[0.5, 0.5], upper_bounds=[1.5, 1.5])
variables = study.variables(continuous_design=design)
gradients = "no_gradients"
responses = study.responses(
    variables,
    {
        "response_type": {"objective_functions": {"count": 1}},
        "gradient_type": {gradients: True},
        "hessian_type": {"no_hessians": True},
    },
)
model = study.model.simulation(variables, interface(study), responses)
study.run(
    study.method.mesh_adaptive_search(
        model,
        config={
            "max_iterations": 5,
            "max_function_evaluations": 25,
            "seed": 1234,
            "initial_delta": 0.1,
            "variable_tolerance": 0.0001,
        },
    )
)
