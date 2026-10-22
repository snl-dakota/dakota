"""interface_python: direct Python interface importing driver module.function."""

from components import study_context

study = study_context()
variables = study.variables(continuous_design={"count": 2, "descriptors": ["x1", "x2"]})
python_driver = {
    "drivers": ["driver_text_book.text_book"],
    "interface_type": {"python": {}},
}
interface = study.interface(analysis_drivers=python_driver)
responses = study.responses(
    variables,
    {
        "response_type": {"objective_functions": {"count": 1}},
        "gradient_type": {"analytic_gradients": True},
        "hessian_type": {"no_hessians": True},
    },
)
model = study.model.simulation(variables, interface, responses)
study.run(
    study.method.list_parameter_study(
        model, list_of_points=[0.0, 0.0, 0.5, 1.5, 2.0, -1.0]
    )
)
