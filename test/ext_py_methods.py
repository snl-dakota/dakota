#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2025
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

import copy
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

    def __init__(self, executor, params_file=None):

        print("python SimplePyOpt class constructer...")
        self.executor = executor
        if params_file is not None:
            self.params_file = params_file

    def core_run(self):

        # Only treat continuous vars for now
        n_vars = self.executor.cv()
        n_fns  = self.executor.response_size()
        init_pts = self.executor.initial_values()
        l_bounds = self.executor.continuous_lower_bounds()
        u_bounds = self.executor.continuous_upper_bounds()

        if n_fns != 1:
            raise RuntimeError("SimplePyOpt only supports a single response")

        retval = {}

        # Hard-coded optimizer settings that could be configured by user
        max_evals = 100
        fn_tol = 1.e-4

        target = 0.0

        # Make test reproducible
        rnd.seed(12321)

        i = 0
        best_x = init_pts

        #print("Model supports gradients -->", self.executor.has_gradient())
        #print("Model supports hessians -->", self.executor.has_hessian())

        #asv = [3]
        #chk_f, chk_grad, chk_hess = self.executor.evaluate(init_pts, asv)
        #print(chk_f)
        #print(chk_grad)
        #print(chk_hess)
        ##raise RuntimeError("Stopping here ...")

        # Simple gradient-based optimizer with hard-coded step length
        if self.executor.has_gradient():

            best_f = self.executor.function_value(init_pts)[0]
            x_new = copy.deepcopy(best_x)
            while  i<=max_evals and best_f>fn_tol:
                x_old = copy.deepcopy(x_new)
                grad = self.executor.gradient_values(x_old)
                x_new[0] = x_old[0] - 0.30*grad[0][0]
                x_new[1] = x_old[1] - 0.30*grad[0][1]
                f = self.executor.function_value(x_new)

                if abs(f[0]-target) < best_f:
                    best_x = x_new
                    best_f = abs(f[0]-target)
                i=i+1

        # Crude "optimization" based on random sampling over parameter space
        else:

            i = 0
            best_x = init_pts
            best_f = self.executor.function_value(init_pts)[0]
            while  i<=max_evals and best_f>fn_tol:
                x = []
                for j in range(n_vars):
                    x.append(rnd.uniform(l_bounds[j], u_bounds[j]))
                f = self.executor.function_value(x)

                if abs(f[0]-target) < best_f:
                    best_x = x
                    best_f = abs(f[0]-target)
                i=i+1


        retval['fns']    = [best_f]
        retval['best_x'] = best_x

        self.executor.dak_print("<<<<< Best parameters          =")
        self.executor.dak_print(f"\t\t{best_x[0]} x1")
        self.executor.dak_print(f"\t\t{best_x[1]} x2")
        self.executor.dak_print("<<<<< Best objective function  =")
        self.executor.dak_print(f"\t\t{best_f}")

        if False:
            print("Found best_f = ", best_f)
            print("Using x = ", best_x)

        return retval


#############################################
#   Demo Python Method --> Numpy Optimizer  #
#############################################

class NumpyOpt:

    def __init__(self, executor, params_file=None):

        print("python NumpyOpt class constructer...")
        self.executor = executor
        if params_file is not None:
            self.params_file = params_file

    def core_run(self):

        # Only treat continuous vars for now
        n_vars = self.executor.cv()
        n_fns  = self.executor.response_size()
        init_pts = np.array(self.executor.initial_values())
        l_bounds = np.array(self.executor.continuous_lower_bounds())
        u_bounds = np.array(self.executor.continuous_upper_bounds())

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
        best_f = self.executor.function_value(init_pts)[0]
        while  i<=max_evals and best_f>fn_tol:
            x = np.random.uniform(low=l_bounds, high=u_bounds, size=n_vars)
            f = self.executor.function_value(x)

            if abs(f[0]-target) < best_f:
                best_x = x.copy()
                best_f = abs(f[0]-target)
            i=i+1

        retval['fns_np']    = [best_f]
        retval['best_x_np'] = best_x

        # Send output to Dakota in a format amenable to regression testing
        self.executor.dak_print("<<<<< Best parameters          =")
        self.executor.dak_print(f"\t\t{best_x[0]} x1")
        self.executor.dak_print(f"\t\t{best_x[1]} x2")
        self.executor.dak_print("<<<<< Best objective function  =")
        self.executor.dak_print(f"\t\t{best_f}")

        return retval


#############################################
#       Demo Python Method --> Sampling     #
#############################################

class RandomSample:

    def __init__(self, executor, params_file=None):

        print("python RandomSample class constructer...")
        self.executor = executor
        if params_file is not None:
            self.params_file = params_file

    def core_run(self):

        # Show the docstring for the executor
        #help(self.executor)

        n_vars = self.executor.tv()
        n_fns  = self.executor.response_size()
        init_pts = self.executor.initial_values()
        l_bounds = self.executor.continuous_lower_bounds()
        u_bounds = self.executor.continuous_upper_bounds()

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
            self.fns.append(self.executor.function_value(x))
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


    def post_run(self):

        # Output using Dakota formatting
        self.executor.output_central_moments(self.fns)



############################################################
#       Demo Python Method --> Mixed Variable Sampling     #
############################################################

class RandomSampleMixed:

    def __init__(self, executor, params_file=None):

        print("python RandomSampleMixed class constructer...")
        self.executor = executor
        if params_file is not None:
            self.params_file = params_file


    def core_run(self):

        print_executor(self.executor)

        n_vars = self.executor.cv()
        n_fns  = self.executor.response_size()
        init_pts = self.executor.initial_values()
        l_bounds = self.executor.continuous_lower_bounds()
        u_bounds = self.executor.continuous_upper_bounds()

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
            mixed_vals['div'] = self.executor.discrete_int_variables()    # just use initial values for testing API
            mixed_vals['dsv'] = self.executor.discrete_string_variables() # just use initial values for testing API
            mixed_vals['drv'] = self.executor.discrete_real_variables()   # just use initial values for testing API
            self.fns.append(self.executor.function_value(mixed_vals))
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

    def post_run(self):

        # Output using Dakota formatting
        self.executor.output_central_moments(self.fns)



# Simple test driver for sanity checking
if __name__ == "__main__":
    print("Hello from external_methods module.")

    

