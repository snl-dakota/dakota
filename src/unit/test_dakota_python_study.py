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
    opt_responses = study.responses(opt_variables, {
        "response_type": {"objective_functions": {"count": 1}},
        "descriptors": ["f"],
        "gradient_type": {"analytic_gradients": True},
        "hessian_type": {"no_hessians": True},
    })
    opt_model = study.model.single({}, opt_variables, interface, opt_responses)
    low_fidelity_model = study.model.single(
        {}, opt_variables, interface, opt_responses
    )
    high_fidelity_model = study.model.single(
        {}, opt_variables, interface, opt_responses
    )
    ensemble_model = study.model.ensemble_surrogate({
        "ensemble": {
            "ordered_model_fidelities": {"pointers": ["LO", "HI"]},
        },
    }, [low_fidelity_model, high_fidelity_model], opt_variables, opt_responses)
    assert ensemble_model.__class__.__name__ == "EnsembleSurrModel"
    explicit_ensemble_model = study.model.ensemble_surrogate({
        "ensemble": {
            "truth_model_pointer": {"pointer": "HI"},
        },
    }, high_fidelity_model, [low_fidelity_model], opt_variables, opt_responses)
    assert explicit_ensemble_model.__class__.__name__ == "EnsembleSurrModel"

    local_model = study.model.local_surrogate(
        opt_model, opt_variables, opt_responses, taylor_series=True
    )
    assert local_model.__class__.__name__ == "DataFitSurrModel"

    multipoint_model = study.model.multipoint_surrogate(
        opt_model,
        opt_variables,
        opt_responses,
        type={"tana": True},
    )
    assert multipoint_model.__class__.__name__ == "DataFitSurrModel"

    global_model = study.model.global_surrogate(
        opt_variables,
        opt_responses,
        truth_model=opt_model,
        type={"polynomial": {"order": {"quadratic": True}}},
    )
    assert global_model.__class__.__name__ == "DataFitSurrModel"

    try:
        study.model.global_surrogate(
            opt_variables,
            opt_responses,
            truth_model=opt_model,
            type={"function_train": {}},
        )
    except RuntimeError as exc:
        assert "RecastModel refactor" in str(exc)
    else:
        raise AssertionError("global_function_train DI construction was accepted")

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
    ver_responses = study.responses(ver_variables, {
        "response_type": {"response_functions": {"count": 1}},
        "descriptors": ["f"],
        "gradient_type": {"no_gradients": True},
        "hessian_type": {"no_hessians": True},
    })
    ver_model = study.model.single({}, ver_variables, interface, ver_responses)
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
    interval_responses = study.responses(interval_variables, {
        "response_type": {"response_functions": {"count": 1}},
        "descriptors": ["f"],
        "gradient_type": {"no_gradients": True},
        "hessian_type": {"no_hessians": True},
    })
    interval_model = study.model.single({}, interval_variables, interface, interval_responses)
    global_interval = study.method.global_interval_est({
        "solution_approach": {"lhs": True},
        "samples": 4,
        "seed": 1234,
    }, interval_model)
    assert global_interval is not None


if __name__ == "__main__":
    main()
