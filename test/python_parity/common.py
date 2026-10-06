"""Shared, hand-authored simulation components for the pilot studies."""

from dakota.study import Study, StudyConfig


def simulation(derivatives):
    config = StudyConfig()
    config.output.precision = 10
    config.output.output_file = "dakota.out"
    config.output.error_file = "dakota.err"
    study = Study(config)
    variables = study.variables(
        continuous_design={"count": 3, "initial_point": [1.0, 1.0, 1.0]}
    )
    interface = study.interface(
        analysis_drivers={"drivers": ["text_book"], "interface_type": {"system": {}}},
        concurrency={"asynchronous": {"evaluation_concurrency": 5}},
    )
    responses = study.responses(
        variables,
        response_type={
            "objective_functions": {
                "count": 1,
                "nonlinear_inequality_constraints": {"count": 2},
            }
        },
        gradient_type={"analytic_gradients" if derivatives else "no_gradients": True},
        hessian_type={"analytic_hessians" if derivatives else "no_hessians": True},
    )
    model = study.model.simulation(variables, interface, responses)
    return study, model
