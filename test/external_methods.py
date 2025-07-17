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

import ext_method


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

class SimplePyOpt:

    def __init__(self, params=None):

        print("python SimplePyOpt class constructer...")
        if params is not None:
            self.params = params

    def core_run(self, executor):

        # Only treat continuous vars for now
        n_vars = executor.cv()
        n_fns  = executor.response_size()
        init_pts = executor.initial_values()
        l_bounds = executor.continuous_lower_bounds()
        u_bounds = executor.continuous_upper_bounds()

        if n_fns != 1:
            raise RuntimeError("SimplePyOpt only supports a single response")

        retval = {}

        # Hard-coded optimizer settings that could be configured by user
        max_evals = 100
        fn_tol = 1.e-4

        target = 0.0

        # Make test reproducible
        rnd.seed(12321)

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
#   Demo Python Method --> Numpy Optimizer  #
#############################################

class NumpyOpt:

    def __init__(self, params=None):

        print("python NumpyOpt class constructer...")
        if params is not None:
            self.params = params

    def core_run(self, executor):

        # Only treat continuous vars for now
        n_vars = executor.cv()
        n_fns  = executor.response_size()
        init_pts = np.array(executor.initial_values())
        l_bounds = np.array(executor.continuous_lower_bounds())
        u_bounds = np.array(executor.continuous_upper_bounds())

        if n_fns != 1:
            raise RuntimeError("NumpyOpt only supports a single response")

        # Hard-coded optimizer settings that could be configured by user
        max_evals = 100
        fn_tol = 1.e-4

        target = 0.0

        # Make test reproducible
        np.random.seed(12321)

        retval = {}

        # Crude "optimization" based on random sampling over parameter space
        i = 0
        best_x = init_pts
        best_f = executor.function_value(init_pts)[0]
        while  i<=max_evals and best_f>fn_tol:
            x = np.random.uniform(low=l_bounds, high=u_bounds, size=n_vars)
            f = executor.function_value(x)

            if abs(f[0]-target) < best_f:
                best_x = x.copy()
                best_f = abs(f[0]-target)
            i=i+1

        retval['fns_np']    = [best_f]
        retval['best_x_np'] = best_x

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

        # Make test reproducible
        rnd.seed(231)

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

        if False:
            print("fns = ", fns)
            print("x = ", xvals)

        return retval


    def post_run(self, executor):

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

        # Make test reproducible
        rnd.seed(123)

        # Crude random sampling over parameter space
        i = 1
        x = init_pts
        xvals = []
        mixed_vals = {}
        self.fns = []
        while  i<=max_evals:
            xvals.append(x)
            mixed_vals['cv']  = x
            mixed_vals['div'] = executor.discrete_int_variables()    # just use initial values for testing API
            mixed_vals['dsv'] = executor.discrete_string_variables() # just use initial values for testing API
            mixed_vals['drv'] = executor.discrete_real_variables()   # just use initial values for testing API
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

        # Output using Dakota formatting
        executor.output_central_moments(self.fns)



# Simple test driver for sanity checking
if __name__ == "__main__":
    print("Hello from external_methods module.")

    

