"""method_snowpac: one study per process.

Mirrors subtest 0 of test/dakota_snowpac.in: SNOWPAC optimizes over a
NestedModel whose MC sampling sub-iterator supplies the error estimates
SNOWPAC requires (a plain simulation model does not provide them).
"""

from components import study_context

study = study_context()

# Inner UQ study: MC sampling over the cantilever direct driver.
# Freeform infers the aleatory view from the sampling method; the DI path
# defaults to the ALL view, so the active view must be stated explicitly or
# the sampler will also sample (and overwrite) the design variables w, t.
uq_variables = study.variables(
    active={"aleatory": True},
    continuous_design={"count": 2},
    normal_uncertain={
        "count": 4,
        "means": [40000.0, 29.0e6, 500.0, 1000.0],
        "std_deviations": [2000.0, 1.45e6, 100.0, 100.0],
        "descriptors": ["R", "E", "X", "Y"],
    },
)
uq_interface = study.interface(
    id_interface="UQ_I",
    analysis_drivers={
        "drivers": ["cantilever"],
        "interface_type": {"direct": {}},
    },
    deactivate={"evaluation_cache": True, "restart_file": True},
)
uq_responses = study.responses(uq_variables,
    response_type={"response_functions": {"count": 3}},
    gradient_type={"no_gradients": True},
    hessian_type={"no_hessians": True})
uq_model = study.model.simulation(uq_variables, uq_interface, uq_responses)
uq_method = study.method.sampling(
    uq_model,
    samples=50,
    seed=12347,
    sample_type={"random": True},  # MC error estimates
    output={"silent": True},
)

# Outer optimization over a nested model
opt_variables = study.variables(continuous_design={
    "count": 2,
    "initial_point": [8.0, 8.0],
    "upper_bounds": [10.0, 10.0],
    "lower_bounds": [1.0, 1.0],
    "descriptors": ["w", "t"],
})
opt_responses = study.responses(opt_variables,
    response_type={"objective_functions": {
        "count": 1,
        "nonlinear_inequality_constraints": {"count": 2},
    }},
    gradient_type={"no_gradients": True},
    hessian_type={"no_hessians": True})
opt_model = study.model.nested(
    uq_method,
    opt_variables,
    opt_responses,
    sub_method_pointer={
        "primary_response_mapping": [1.0, 0.0, 0.0, 0.0, 0.0, 0.0],
        "secondary_response_mapping": [
            0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 1.0, 0.0,
        ],
    },
)
options = {
    "seed": 25041981,
    "max_iterations": 500,
    "max_function_evaluations": 250,
    "trust_region": {
        "initial_size": [0.10],
        "minimum_size": 1.e-6,
        "contract_threshold": 0.25,
        "expand_threshold": 0.75,
        "contraction_factor": 0.50,
        "expansion_factor": 1.50,
    },
}
study.run(study.method.snowpac(opt_model, config=options))
