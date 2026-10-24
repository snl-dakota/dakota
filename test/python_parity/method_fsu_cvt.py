"""method_fsu_cvt: one study per process."""

from components import study_context, interface, response

study = study_context()
design = {"count": 2, "initial_point": [0.9, 1.1], "descriptors": ["x1", "x2"]}
design.update(lower_bounds=[0.5, 0.5], upper_bounds=[1.5, 1.5])
variables = study.variables(continuous_design=design)
responses = response(study, variables)
model = study.model.simulation(variables, interface(study), responses)
study.run(
    study.method.fsu_cvt(
        model,
        config={
            "samples": 25,
            "num_trials": 1000,
            "max_iterations": 50,
            "seed": 50374,
        },
    )
)
