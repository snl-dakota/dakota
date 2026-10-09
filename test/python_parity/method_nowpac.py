"""method_nowpac: one study per process."""

from components import study_context

study = study_context()
variables = study.variables(continuous_design={
    "count": 2,
    "initial_point": [-1.2, 1.0],
    "lower_bounds": [-2.0, -2.0],
    "upper_bounds": [2.0, 2.0],
    "descriptors": ["x1", "x2"],
})
interface = study.interface(analysis_drivers={
    "drivers": ["rosenbrock"],
    "interface_type": {"direct": {}},
})
responses = study.responses(variables,
    response_type={"objective_functions": {"count": 1}},
    gradient_type={"no_gradients": True},
    hessian_type={"no_hessians": True})
model = study.model.simulation(variables, interface, responses)
options = {
    "max_iterations": 500,
    "trust_region": {
        "initial_size": [0.10],
        "minimum_size": 1.e-6,
        "contract_threshold": 0.25,
        "expand_threshold": 0.75,
        "contraction_factor": 0.50,
        "expansion_factor": 1.50,
    },
}
study.run(study.method.nowpac(model, config=options))
