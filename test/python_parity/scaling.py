"""scaling: CONMIN FRCG with user-specified variable and objective scales."""

from components import study_context, interface

study = study_context()
design = {"count": 2, "initial_point": [0.9, 1.1], "descriptors": ["x1", "x2"]}
design.update(lower_bounds=[0.5, 0.5], upper_bounds=[1.5, 1.5])
design.update(scale_types=["value"], scales=[4.0, 0.5])
variables = study.variables(continuous_design=design)
responses = study.responses(
    variables,
    {
        "response_type": {
            "objective_functions": {
                "count": 1,
                "primary_scale_types": ["value"],
                "primary_scales": [10.0],
            }
        },
        "gradient_type": {"analytic_gradients": True},
        "hessian_type": {"no_hessians": True},
    },
)
model = study.model.simulation(variables, interface(study), responses)
options = {"scaling": True, "max_iterations": 5, "convergence_tolerance": 0.0001}
study.run(study.method.conmin_frcg(model, config=options))
