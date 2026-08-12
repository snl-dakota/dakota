"""Smoke tests for the dakota.study DI/library-mode bindings."""

from dakota.study import Study, StudyConfig


def main():
    config = StudyConfig()
    config.output.precision = 12
    config.output.output_file = "dakota_python_study.out"
    config.output.error_file = "dakota_python_study.err"

    study = Study(config)
    assert study.method is not None
    assert study.model is not None

    try:
        study.variables({"bogus": 2})
    except Exception as exc:
        assert (
            "permit" in str(exc).lower()
            or "extra" in str(exc).lower()
            or "unexpected property" in str(exc).lower()
        )
    else:
        raise AssertionError("study.variables() accepted an invalid fragment")

    interface = study.interface({
        "analysis_drivers": {
            "drivers": ["text_book"],
            "interface_type": {"fork": {}},
        }
    })

    opt_variables = study.variables({
        "continuous_design": {
            "count": 2,
            "descriptors": ["x1", "x2"],
            "initial_point": [0.9, 1.1],
            "lower_bounds": [0.5, 0.5],
            "upper_bounds": [5.8, 2.9],
        }
    })
    opt_responses = study.responses({
        "response_type": {"objective_functions": {"count": 1}},
        "descriptors": ["f"],
        "gradient_type": {"analytic_gradients": True},
        "hessian_type": {"no_hessians": True},
    }, opt_variables)
    opt_model = study.model.simulation({}, opt_variables, interface, opt_responses)
    param_study = study.method.vector_parameter_study({
        "step_control": {"final_point": [1.1, 1.3]},
        "num_steps": 2,
    }, opt_model)
    assert param_study.__class__.__name__ == "ParamStudy"

    ver_variables = study.variables({
        "continuous_state": {
            "count": 2,
            "descriptors": ["h1", "h2"],
            "initial_state": [0.25, 0.125],
        }
    })
    ver_responses = study.responses({
        "response_type": {"response_functions": {"count": 1}},
        "descriptors": ["f"],
        "gradient_type": {"no_gradients": True},
        "hessian_type": {"no_hessians": True},
    }, ver_variables)
    ver_model = study.model.simulation({}, ver_variables, interface, ver_responses)
    verification = study.method.richardson_extrap({
        "mode": {"estimate_order": True},
        "refinement_rate": 2.0,
        "convergence_tolerance": 1.0e-4,
        "max_iterations": 4,
    }, ver_model)
    assert verification.__class__.__name__ == "RichExtrapVerification"

    interval_variables = study.variables({
        "continuous_interval_uncertain": {
            "count": 2,
            "descriptors": ["x1", "x2"],
            "lower_bounds": [0.0, 0.0],
            "upper_bounds": [1.0, 1.0],
            "interval_probabilities": [1.0, 1.0],
        }
    })
    interval_responses = study.responses({
        "response_type": {"response_functions": {"count": 1}},
        "descriptors": ["f"],
        "gradient_type": {"no_gradients": True},
        "hessian_type": {"no_hessians": True},
    }, interval_variables)
    interval_model = study.model.simulation({}, interval_variables, interface, interval_responses)
    global_interval = study.method.global_interval_est({
        "solution_approach": {"lhs": True},
        "samples": 4,
        "seed": 1234,
    }, interval_model)
    assert global_interval is not None


if __name__ == "__main__":
    main()
