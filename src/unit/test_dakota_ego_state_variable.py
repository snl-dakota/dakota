#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2025
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

# Regression test for an EGO bug where merit-function (augmented Lagrangian)
# updates driven by surrogate ("liar") data, plus a missing convergence gate,
# corrupted the final best-point selection for constrained problems with a
# mix of design and (fixed) state variables.
import math

import dakota.environment as dakenv

X0_FIXED = math.sqrt(0.5)


def evaluator(params):
    x0, x1 = params["cv"]
    n = 2
    f = -(x0 * x1) * (math.sqrt(n) ** n)
    g = x0 * x0 + x1 * x1 - 1.0
    return {"fns": [f, g]}


def ego_g3_state_input(seed):
    return f"""
environment
    top_method_pointer = 'EGO'

    method
        id_method = 'EGO'
        efficient_global
            seed = {seed}
            model_pointer = "TRUE_MODEL"

    model
        id_model = 'TRUE_MODEL'
        single
            interface_pointer = 'INTERFACE'
            variables_pointer = 'VARIABLES'
            responses_pointer = 'RESPONSES'

    variables
        id_variables = 'VARIABLES'
        continuous_design = 1
            descriptors       'X1'
            initial_point     0.0
            lower_bounds      0.0
            upper_bounds      1.0
        continuous_state = 1
            descriptors       'X0'
            initial_state     {X0_FIXED!r}
            lower_bounds      0.0
            upper_bounds      1.0

    interface
        id_interface = 'INTERFACE'
        python
            analysis_drivers
                'evaluator'

    responses
        id_responses = 'RESPONSES'
        objective_functions = 1
        nonlinear_equality_constraints = 1
        no_gradients
        no_hessians
"""


def test_ego_state_variable(seed):
    daklib = dakenv.study(callbacks={"evaluator": evaluator},
                           input_string=ego_g3_state_input(seed))
    daklib.execute()

    best_x1 = dakenv.get_variable_values(daklib)[0]

    target = X0_FIXED
    max_tol = 0.05 * X0_FIXED
    assert abs(best_x1 - target) < max_tol, \
        f"seed={seed}: best_x1={best_x1}, expected ~{target}"


if __name__ == "__main__":
    for seed in (123456, 111111, 222222, 333333):
        print(f"\n+++ Running EGO state-variable regression test, seed={seed}\n")
        test_ego_state_variable(seed)
    print("\n+++ Done.\n")
