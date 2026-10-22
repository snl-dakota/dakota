"""interface_plugin: identity-map plugin library behind a centered study."""

from components import study_context

study = study_context()
variables = study.variables(continuous_design={"count": 2, "descriptors": ["x1", "x2"]})
interface = study.interface(
    analysis_drivers={
        "drivers": ["f_of_x_equals_x"],
        "interface_type": {"plugin": {"library_path": "libidentity_map.so"}},
    }
)
responses = study.responses(
    variables,
    {
        "response_type": {"response_functions": {"count": 2}},
        "descriptors": ["f0", "f1"],
        "gradient_type": {"analytic_gradients": True},
        "hessian_type": {"analytic_hessians": True},
    },
)
model = study.model.simulation(variables, interface, responses)
study.run(
    study.method.centered_parameter_study(
        model, step_vector=[0.5, 1.7], steps_per_variable=[1]
    )
)
