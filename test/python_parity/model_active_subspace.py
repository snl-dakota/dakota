"""dakota_cv_subspace.in s0: LHS over a cross-validated active subspace."""

from dakota.study import Study, StudyConfig

config = StudyConfig()
config.output.precision = 16  # output_precision in the freeform input
config.output.output_file = "dakota.out"
config.output.error_file = "dakota.err"
study = Study(config)
variables = study.variables(
    normal_uncertain={"count": 20, "means": [0.5] * 20, "std_deviations": [0.2] * 20}
)
interface = study.interface(
    analysis_drivers={
        "drivers": ["aniso_quad_form"],
        "interface_type": {"direct": {}},
        "analysis_components": [
            "seed:61043",
            "eigenvals: 302.56 134.2 53.9 5.8 2.1 1e-1 5e-2 1e-3 4e-5 5e-6 1e-8",
        ],
    }
)
responses = study.responses(
    variables,
    {
        "response_type": {"response_functions": {"count": 1}},
        "gradient_type": {"analytic_gradients": True},
        "hessian_type": {"no_hessians": True},
    },
)
truth = study.model.simulation(variables, interface, responses)
model = study.model.active_subspace(
    truth,
    variables,
    responses,
    initial_samples=50,
    truncation_method={"cross_validation": {}},
)
study.run(
    study.method.sampling(
        model,
        seed=24620,
        samples=20,
        sample_type={"lhs": True},
        output={"debug": True},
    )
)
