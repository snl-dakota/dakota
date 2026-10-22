"""dakota_uq_adapted_basis.in s0: sampling over a level-1 adapted basis."""

from components import study_context

study = study_context()
variables = study.variables(
    normal_uncertain={"count": 15, "means": [0.0] * 15, "std_deviations": [1.0] * 15}
)
interface = study.interface(
    analysis_drivers={
        "drivers": ["steady_state_diffusion_1d"],
        "interface_type": {"direct": {}},
    }
)
responses = study.responses(
    variables,
    {
        "response_type": {"response_functions": {"count": 1}},
        "gradient_type": {"no_gradients": True},
        "hessian_type": {"no_hessians": True},
    },
)
truth = study.model.simulation(variables, interface, responses)
model = study.model.adapted_basis(
    truth,
    variables,
    responses,
    truth_model_pointer={"basis_control": {"sparse_grid_level": 1}},
)
study.run(
    study.method.sampling(
        model, samples=100, seed=1234567, output={"silent": True}
    )
)
