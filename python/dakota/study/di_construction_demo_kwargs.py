#  _______________________________________________________________________
#
#    Dakota: Explore and predict with confidence.
#    Copyright 2014-2025
#    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#    This software is distributed under the GNU Lesser General Public License.
#    For more information, see the README file in the top Dakota directory.
#    _______________________________________________________________________

"""DI construction demo using the :mod:`dakota.study` Python bindings.

This mirrors ``src/di_construction_demo.cpp``: it assembles a small sampling
study from keyword arguments, constructs Dakota components through a ``Study``
context, and runs the resulting method.

This is a refactored version of ``di_construction_demo.py`` that passes
configuration as keyword arguments directly at each call site instead of
using pre-defined module-level dictionaries.
"""

from __future__ import annotations

try:
    from . import Response, Study, StudyConfig, Variables
except ImportError:
    # Allow direct execution by path during development, e.g.
    # ``python python/dakota/study/di_construction_demo_kwargs.py``.
    from dakota.study import Response, Study, StudyConfig, Variables


def main() -> None:
    # [docs-library-api-start]
    config = StudyConfig()
    config.output.precision = 12
    config.output.output_file = "di_construction_demo.out"
    config.output.error_file = "di_construction_demo.err"
    config.output.write_restart = "di_construction_demo.rst"
    config.output.results_output = True
    config.output.results_output_file = "di_construction_demo_results"

    study = Study(config)

    variables = study.variables(
        active={"all": True},
        uniform_uncertain={
            "count": 2,
            "descriptors": ["x1", "x2"],
            "lower_bounds": [0.0, 0.0],
            "upper_bounds": [1.0, 1.0],
        },
    )

    response = study.responses(
        variables,
        response_type={"response_functions": {"count": 1}},
        descriptors=["f"],
        gradient_type={"no_gradients": True},
        hessian_type={"no_hessians": True},
    )

    interface = study.interface(
        analysis_drivers={
            "drivers": ["text_book"],
            "interface_type": {
                "fork": {
                    "parameters_file": "params.in",
                    "results_file": "results.out",
                    "file_save": True,
                }
            }
        },
        deactivate={"restart_file": True}
    )

    model = study.model.simulation(
        variables,
        interface,
        response,
    )

    sampling = study.method.sampling(
        model,
        sample_type={"lhs": True},
        samples=10,
        seed=1234,
    )

    print("Running sampling study...")
    study.run(sampling)

    print("Completed DI study.")
    print(f"Samples evaluated: {sampling.num_responses()}")
    if sampling.num_responses() > 0:
        print(f"First response value: {sampling.first_response_value()}")
    # [docs-library-api-end]


if __name__ == "__main__":
    main()
