"""method_external_python: one study per process.

Uses ext_py_methods.SimplePyOpt from the Dakota test suite.
The ext_py_methods.py and ext_py_options.json files must be present.
"""

from components import study_context, interface, response

study = study_context()
design = {"count": 2, "initial_point": [0.5, 0.5], "descriptors": ["x1", "x2"]}
design.update(lower_bounds=[-2.0, -2.0], upper_bounds=[2.0, 2.0])
variables = study.variables(continuous_design=design)
responses = response(study, variables)
model = study.model.simulation(variables, interface(study), responses)
study.run(
    study.method.external_python(
        model,
        config={
            "class_path_and_name": "ext_py_methods.SimplePyOpt",
            "options_file": "ext_py_options.json",
        },
    )
)
