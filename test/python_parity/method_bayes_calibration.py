"""method_bayes_calibration: one study per process."""

from components import study_context
from components import response_interface as driver

study = study_context()
variables = study.variables(
    uniform_uncertain={
        "count": 2,
        "lower_bounds": [-1.0, -1.0],
        "upper_bounds": [1.0, 1.0],
    }
)
response = study.responses(
    variables,
    {
        "response_type": {"response_functions": {"count": 1}},
        "gradient_type": {"no_gradients": True},
        "hessian_type": {"no_hessians": True},
    },
)
model = study.model.simulation(variables, driver(study, "high"), response)
study.run(
    study.method.bayes_calibration(
        model,
        config={
            "sub_method": {
                "wasabi": {
                    "emulator": {
                        "gaussian_process": {
                            "implementation": {"surfpack": True},
                            "build_samples": 8,
                        }
                    },
                    "pushforward_samples": 16,
                    "seed": 1234,
                    "generate_posterior_samples": {
                        "posterior_samples_export_filename": "posterior_samples.dat"
                    },
                    "evaluate_posterior_density": {
                        "posterior_density_export_filename": "posterior_density.dat"
                    },
                    "data_distribution": {
                        "gaussian": {
                            "means": [0.0],
                            "covariance": {
                                "values": [0.25],
                                "type": {"diagonal": True},
                            },
                        }
                    },
                }
            }
        },
    )
)
