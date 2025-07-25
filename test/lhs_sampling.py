#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2025
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

from scipy.stats import qmc

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
#    Demo Python Method --> LHS Sampling    #
#############################################

class LHSSample:

    def __init__(self, params=None):

        print("python LHSSample class constructer...")
        if params is not None:
            self.params = params

    def pre_run(self, executor):

        n_vars = executor.tv()

        # Generate samples using LHS from scipy qmc
        sampler = qmc.LatinHypercube(d=n_vars)

        # Hard-coded setting that could be configured by user
        self.samples = sampler.random(n=25)
        print("Using scipy qmc LHS samples:",self.samples)

        #executor.output_central_moments(self.fns)

    def core_run(self, executor):

        # Show the docstring for the executor
        #help(executor)

        n_vars = executor.tv()
        n_fns  = executor.response_size()
        init_pts = executor.initial_values()
        l_bounds = executor.continuous_lower_bounds()
        u_bounds = executor.continuous_upper_bounds()

        retval = {}

        # Sample over parameter space
        x = init_pts
        self.fns = []
        for x in self.samples:
            self.fns.append(executor.function_value(x))

        retval['x']   = self.samples
        retval['fns'] = self.fns

        if False:
            print("fns = ", self.fns)
            print("x = ", self.samples)

        return retval


    def post_run(self, executor):

        # Output using Dakota formatting
        executor.output_central_moments(self.fns)



# Simple test driver for sanity checking
if __name__ == "__main__":
    print("Hello from lhs_samplig module.")
