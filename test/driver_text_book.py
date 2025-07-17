#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2025
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

import numpy as np
import random as rnd

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
    expected_params["eval_id"]   = "1"

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
    expected_params["analysis_components"] = ["a", "b"]

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

#####################################
#       External Method Helper      #
#####################################

def print_executor(executor):

    print("executor.tv()                                ", executor.tv())
    print("executor.cv()                                ", executor.cv())
    print("executor.div()                               ", executor.div())
    print("executor.dsv()                               ", executor.dsv())
    print("executor.drv()                               ", executor.drv())
    print("executor.continuous_variables()              ", executor.continuous_variables())
    print("executor.discrete_int_variables()            ", executor.discrete_int_variables())
    print("executor.discrete_string_variables()         ", executor.discrete_string_variables())
    print("executor.discrete_real_variables()           ", executor.discrete_real_variables())
    print("executor.continuous_variable_labels()        ", executor.continuous_variable_labels())
    print("executor.discrete_int_variable_labels()      ", executor.discrete_int_variable_labels())
    print("executor.discrete_string_variable_labels()   ", executor.discrete_string_variable_labels())
    print("executor.discrete_real_variable_labels()     ", executor.discrete_real_variable_labels())
    print("executor.response_size()                     ", executor.response_size())
    print("executor.response_labels()                   ", executor.response_labels())
    print("executor.continuous_lower_bounds()           ", executor.continuous_lower_bounds())
    print("executor.continuous_upper_bounds()           ", executor.continuous_upper_bounds())
    print("executor.discrete_int_lower_bounds()         ", executor.discrete_int_lower_bounds())
    print("executor.discrete_int_upper_bounds()         ", executor.discrete_int_upper_bounds())
    print("executor.discrete_real_lower_bounds()        ", executor.discrete_real_lower_bounds())
    print("executor.discrete_real_upper_bounds()        ", executor.discrete_real_upper_bounds())



#############################################
#       Demo Python Method --> Optimizer    #
#############################################

import ext_method

class SimplePyOpt:

    def __init__(self, params=None):

        print("python SimplePyOpt class constructer...")
        if params is not None:
            self.params = params

    def core_run(self, executor):

        n_vars = executor.tv()
        n_fns  = executor.response_size()
        init_pts = executor.initial_values()
        l_bounds = executor.continuous_lower_bounds()
        u_bounds = executor.continuous_upper_bounds()

        if n_fns != 1:
            raise RuntimeError("SimplyPyOpt only supports a single response")

        retval = {}

        # Hard-coded optimizer settings that could be configured by user
        max_evals = 100
        fn_tol = 1.e-4

        target = 0.0

        # Crude "optimization" based on random sampling over parameter space
        i = 0
        best_x = init_pts
        best_f = executor.function_value(init_pts)[0]
        while  i<=max_evals and best_f>fn_tol:
            x = []
            for j in range(n_vars):
                x.append(rnd.uniform(l_bounds[j], u_bounds[j]))
            f = executor.function_value(x)

            if abs(f[0]-target) < best_f:
                best_x = x
                best_f = abs(f[0]-target)
            i=i+1

        retval['fns']    = [best_f]
        retval['best_x'] = best_x

        if False:
            print("Found best_f = ", best_f)
            print("Using x = ", best_x)

        return retval


#############################################
#       Demo Python Method --> Sampling     #
#############################################

class RandomSample:

    def __init__(self, params=None):

        print("python RandomSample class constructer...")
        if params is not None:
            self.params = params

    def core_run(self, executor):

        # Show the docstring for the executor
        #help(executor)

        n_vars = executor.tv()
        n_fns  = executor.response_size()
        init_pts = executor.initial_values()
        l_bounds = executor.continuous_lower_bounds()
        u_bounds = executor.continuous_upper_bounds()

        retval = {}

        # Hard-coded setting that could be configured by user
        max_evals = 25

        # Crude random sampling over parameter space
        i = 1
        x = init_pts
        xvals = []
        self.fns = []
        while  i<=max_evals:
            xvals.append(x)
            self.fns.append(executor.function_value(x))
            x = []
            for j in range(n_vars):
                x.append(rnd.uniform(l_bounds[j], u_bounds[j]))
            i=i+1

        retval['x']   = x
        retval['fns'] = self.fns
        #retval['fns_np'] = np.array(fns).astype(np.float64)

        #print("fns_np = ", retval['fns_np'])

        if False:
            print("fns = ", fns)
            print("x = ", xvals)

        return retval


    def post_run(self, executor):

        # Not sure how to use this yet
        #print(type(s))

        # Output using Dakota formatting
        executor.output_central_moments(self.fns)



############################################################
#       Demo Python Method --> Mixed Variable Sampling     #
############################################################

class RandomSampleMixed:

    def __init__(self, params=None):

        print("python RandomSampleMixed class constructer...")
        if params is not None:
            self.params = params


    def core_run(self, executor):

        print_executor(executor)

        n_vars = executor.cv()
        n_fns  = executor.response_size()
        init_pts = executor.initial_values()
        l_bounds = executor.continuous_lower_bounds()
        u_bounds = executor.continuous_upper_bounds()

        retval = {}

        # Hard-coded setting that could be configured by user
        max_evals = 5

        # Crude random sampling over parameter space
        i = 1
        x = init_pts
        xvals = []
        mixed_vals = {}
        self.fns = []
        while  i<=max_evals:
            xvals.append(x)
            mixed_vals['cv']  = x
            mixed_vals['div'] = executor.discrete_int_variables() # just use initial values for testing API
            mixed_vals['dsv'] = executor.discrete_string_variables() # just use initial values for testing API
            mixed_vals['drv'] = executor.discrete_real_variables() # just use initial values for testing API
            self.fns.append(executor.function_value(mixed_vals))
            x = []
            for j in range(n_vars):
                x.append(rnd.uniform(l_bounds[j], u_bounds[j]))
            i=i+1

        retval['x']   = x
        retval['fns'] = self.fns

        if False:
            print("fns = ", fns)
            print("x = ", xvals)

        return retval

    def post_run(self, executor):

        # Not sure how to use this yet
        #print(type(s))

        # Output using Dakota formatting
        executor.output_central_moments(self.fns)


    def test_np_array(self):

        arr = np.array([1.1, 2.2, 3.3])
        print(arr)
        return arr



# Simple test driver for sanity checking
if __name__ == "__main__":

    params = { 'variables' : 2   ,
               'functions' : 1   ,
               'asv'       : [1] ,
               'cv'        : [2.2, 1.3]
             }
    retval = text_book_list(params)

    print(retval)

    

