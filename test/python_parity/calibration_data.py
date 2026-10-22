"""calibration_data: NL2SOL residuals against a scalar calibration data file."""

from components import study_context
from components import response_interface as driver

study = study_context()
variables = study.variables(
    continuous_design={"count": 2, "initial_point": [0.9, 1.1], "descriptors": ["x1", "x2"]}
)
response = study.responses(
    variables,
    {
        "response_type": {
            "calibration_terms": {
                "count": 2,
                "data_specification": {
                    "calibration_data_file": {
                        "filename": "least_squares_data.dat",
                        "format": {"freeform": True},
                    }
                },
            }
        },
        "gradient_type": {"analytic_gradients": True},
        "hessian_type": {"no_hessians": True},
    },
)
model = study.model.simulation(variables, driver(study, "least_squares"), response)
study.run(
    study.method.nl2sol(
        model, config={"max_iterations": 5, "max_function_evaluations": 20}
    )
)
