"""Hand-authored components shared by the additional paired studies."""

from dakota.study import Study, StudyConfig


def study_context():
    config = StudyConfig()
    config.output.precision = 10
    config.output.output_file = "dakota.out"
    config.output.error_file = "dakota.err"
    return Study(config)


def interface(study, named=False, labeled=False, id=None):
    fork = (
        {
            "parameters_file": "text_book.in",
            "results_file": "text_book.out",
            "file_tag": True,
        }
        if named
        else {}
    )
    if labeled:
        fork["results_format"] = {"standard": {"labeled": True}}
    options = {"id_interface": id} if id else {}
    return study.interface(
        **options,
        analysis_drivers={"drivers": ["text_book"], "interface_type": {"fork": fork}}
    )


def design(study):
    return study.variables(
        continuous_design={
            "count": 2,
            "initial_point": [1.0, 1.0],
            "lower_bounds": [0.5, 0.5],
            "upper_bounds": [1.5, 1.5],
            "descriptors": ["x1", "x2"],
        }
    )


def response(
    study,
    variables,
    kind="objective_functions",
    gradients="no_gradients",
    hessians="no_hessians",
    constraints=0,
    labels=None,
):
    primary = {"count": 1}
    if constraints:
        primary["nonlinear_inequality_constraints"] = {"count": constraints}
    config = {
        "response_type": {kind: primary},
        "gradient_type": {gradients: True},
        "hessian_type": {hessians: True},
    }
    if labels:
        config["descriptors"] = labels
    return study.responses(variables, config)


def response_interface(study, mode="high", id=None):
    options = {"id_interface": id} if id else {}
    return study.interface(
        **options,
        analysis_drivers={
            "drivers": ["python3 response_driver.py " + mode],
            "interface_type": {"fork": {}},
        }
    )


def hybrid_sub_methods(study):
    """Coliny EA and pattern search sub-methods sharing one text_book model."""
    design = {"count": 2, "initial_point": [0.9, 1.1], "descriptors": ["x1", "x2"]}
    design.update(lower_bounds=[0.5, 0.5], upper_bounds=[1.5, 1.5])
    variables = study.variables(continuous_design=design)
    model = study.model.simulation(variables, interface(study), response(study, variables))
    ga = study.method.coliny_ea(
        model,
        id_method="GA",
        max_iterations=2,
        max_function_evaluations=24,
        seed=1234,
        population_size=8,
    )
    ps = study.method.coliny_pattern_search(
        model,
        id_method="PS",
        max_iterations=5,
        max_function_evaluations=25,
        seed=1234,
        initial_delta=0.1,
        variable_tolerance=0.0001,
    )
    return ga, ps


def dace_global_surrogate(study):
    """Quadratic polynomial surrogate of text_book built by a ten-point LHS."""
    design = {"count": 2, "initial_point": [0.9, 1.1], "descriptors": ["x1", "x2"]}
    design.update(lower_bounds=[0.5, 0.5], upper_bounds=[1.5, 1.5])
    variables = study.variables(continuous_design=design)
    responses = response(study, variables)
    truth = study.model.simulation(variables, interface(study), responses)
    dace = study.method.sampling(truth, samples=10, seed=1234, sample_type={"lhs": True})
    return study.model.global_surrogate(
        variables,
        responses,
        dace_iterator=dace,
        type={"polynomial": {"order": {"quadratic": True}}},
    )
