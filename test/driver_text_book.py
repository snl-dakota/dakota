#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2023
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

import numpy as np

use_list_return_type = True


def text_book_numpy(params):

    num_fns = params["functions"]
    x = params["cv"]
    ASV = params["asv"]

    retval = {}

    retval["metadata"] = np.array([5., 10.])

    if (ASV[0] & 1):  # **** f:
        fn = sum([(val - 1.)**4 for val in x])
        retval["fns"] = np.array([fn])

    if (ASV[0] & 2):  # **** df/dx:
        g = [4. * (val - 1.)**3 for val in x]
        retval["fnGrads"] = np.array([np.array(g)])

    if (ASV[0] & 4):  # **** d^2f/dx^2:
        h = np.diag([12. * (val - 1.)**2 for val in x])
        retval["fnHessians"] = np.array([h])

    if num_fns == 1:
        return retval

    # We have nonlinear constraints to provide
    if num_fns != 3:
        raise("text_book problem supports exactly 2 nonlinear inequality constraints.")

    if (ASV[1] & 1):  # **** first nonlinear constraint objective:
        retval['fns'] = np.append(retval['fns'], (x[0] * x[0] - x[1] / 2.0))

    if (ASV[1] & 2):  # **** d/dx of first nonlinear constraint objective:
        g = [2.0 * x[0], -0.5]
        for i in range(len(x) - 2):
            g.append(0.0)
        retval['fnGrads'] = np.append(
            retval['fnGrads'], np.array([np.array(g)]), axis=0)

    if (ASV[1] & 4):  # **** d2/dx2 of first nonlinear constraint objective:
        h = np.zeros_like(retval['fnHessians'][0])
        h[0, 0] = 2.0
        retval['fnHessians'] = np.append(retval['fnHessians'], [h], axis=0)

    if (ASV[2] & 1):  # **** second nonlinear constraint objective:
        retval['fns'] = np.append(retval['fns'], (x[1] * x[1] - x[0] / 2.0))

    if (ASV[2] & 2):  # **** d/dx of second nonlinear constraint objective:
        g = [-0.5, 2.0 * x[1]]
        for i in range(len(x) - 2):
            g.append(0.0)
        retval['fnGrads'] = np.append(
            retval['fnGrads'], np.array([np.array(g)]), axis=0)

    if (ASV[2] & 4):  # **** d2/dx2 of second nonlinear constraint objective:
        h = np.zeros_like(retval['fnHessians'][0])
        h[1, 1] = 2.0
        retval['fnHessians'] = np.append(retval['fnHessians'], [h], axis=0)

    return retval


def text_book_list(params):

    num_fns = params['functions']
    x = params['cv']
    ASV = params['asv']

    retval = {}

    retval["metadata"] = [5., 10.]

    if (ASV[0] & 1):  # **** f:
        fn = 0.0
        for val in x:
            fn += pow(val - 1.0, 4)
        retval['fns'] = [fn]

    if (ASV[0] & 2):  # **** df/dx:
        g = []
        for val in x:
            g.append(4.0 * pow(val - 1.0, 3))
        retval['fnGrads'] = [g]

    if (ASV[0] & 4):  # **** d^2f/dx^2:
        h = []
        for i in range(len(x)):
            h.append([0 * _ for _ in x])
        for i, val in enumerate(x):
            h[i][i] = 12.0 * pow(val - 1.0, 2)
        retval['fnHessians'] = [h]

    if num_fns == 1:
        return retval

    # We have nonlinear constraints to provide
    if num_fns != 3:
        raise("text_book problem supports exactly 2 nonlinear inequality constraints.")

    if (ASV[1] & 1):  # **** first nonlinear constraint objective:
        retval['fns'].append(x[0] * x[0] - x[1] / 2.0)

    if (ASV[1] & 2):  # **** d/dx of first nonlinear constraint objective:
        g = [2.0 * x[0], -0.5]
        for i in range(len(x) - 2):
            g.append(0.0)
        retval['fnGrads'].append(g)

    if (ASV[1] & 4):  # **** d2/dx2 of first nonlinear constraint objective:
        h = []
        for i, val in enumerate(x):
            row = [0.0 * _ for _ in x]
            h.append(row)
        h[0][0] = 2.0
        retval['fnHessians'].append(h)

    if (ASV[2] & 1):  # **** second nonlinear constraint objective:
        retval['fns'].append(x[1] * x[1] - x[0] / 2.0)

    if (ASV[2] & 2):  # **** d/dx of second nonlinear constraint objective:
        g = [-0.5, 2.0 * x[1]]
        for i in range(len(x) - 2):
            g.append(0.0)
        retval['fnGrads'].append(g)

    if (ASV[2] & 4):  # **** d2/dx2 of second nonlinear constraint objective:
        h = []
        for i, val in enumerate(x):
            row = [0.0 * _ for _ in x]
            h.append(row)
        h[1][1] = 2.0
        retval['fnHessians'].append(h)

    return retval


def check_expected_params(params):

    expected_params = {}
    expected_params["variables"] = 9
    expected_params["functions"] = 3
    expected_params["eval_id"]   = 1

    expected_params["cv_labels"] = ["x1", "x2", "x3"]
    expected_params["div_labels"] = ["z1", "z2", "z3"]
    expected_params["dsv_labels"] = ["s1"]
    expected_params["drv_labels"] = ["y1", "y2"]
    expected_params["variable_labels"] = expected_params["cv_labels"] \
        + expected_params["div_labels"] \
        + expected_params["dsv_labels"] \
        + expected_params["drv_labels"]

    expected_params["function_labels"] = ["f1", "c1", "c2"]
    expected_params["metadata_labels"] = ["m1", "m2"]

    expected_params["cv"] = [0., 0., 0.]
    expected_params["div"] = [2, 4, 6]
    expected_params["dsv"] = ["two"]
    expected_params["drv"] = [1.2, 3.2]
    expected_params["asv"] = [7, 7, 7]
    expected_params["dvv"] = [1, 2, 3]

    single_value_type_keys = ["variables", "functions", "eval_id"]
    array_type_keys = ["variable_labels", "cv", "cv_labels", "div", "div_labels",
                       "dsv", "dsv_labels", "drv", "drv_labels", "asv", "dvv"]

    for key in single_value_type_keys:
        dakota_value = params[key]
        expected_value = expected_params[key]
        assert(expected_value == dakota_value)

    for key in array_type_keys:
        dakota_values = params[key]
        expected_values = expected_params[key]
        for idx, expected_value in enumerate(expected_values):
            assert(expected_value == dakota_values[idx])

    assert(len(params["analysis_components"]) == 0)


def text_book(params):
    check_expected_params(params)

    if use_list_return_type:
        return text_book_list(params)
    else:
        return text_book_numpy(params)


def text_book_batch(list_of_params):
    retvals = []
    for param_dict in list_of_params:
        if use_list_return_type:
            retvals.append(text_book_list(param_dict))
        else:
            retvals.append(text_book_numpy(param_dict))
    return retvals
