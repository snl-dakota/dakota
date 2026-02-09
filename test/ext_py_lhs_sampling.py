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
import ext_py_helper


#############################################
#    Demo Python Method --> LHS Sampling    #
#############################################

class LHSSample:

    def __init__(self, executor, params_file=None):

        print("python LHSSample class constructer...")
        self.executor = executor
        if params_file is not None:
            self.params_file = params_file

    def pre_run(self):

        n_vars = self.executor.tv()
        l_bounds = self.executor.continuous_lower_bounds()
        u_bounds = self.executor.continuous_upper_bounds()

        # Generate samples using LHS from scipy qmc
        sampler = qmc.LatinHypercube(d=n_vars)

        # Hard-coded setting that could be configured by user
        samples = sampler.random(n=25)
        self.samples = qmc.scale(samples, l_bounds, u_bounds)
        print("Using scipy qmc LHS samples:",self.samples)

    def core_run(self):

        # Show the docstring for the executor
        #help(self.executor)

        retval = {}

        # Sample over parameter space
        self.fns = []
        for x in self.samples:
            self.fns.append(self.executor.function_value(x))

        retval['x']   = self.samples
        retval['fns'] = self.fns

        if False:
            print("fns = ", self.fns)
            print("x = ", self.samples)

        return retval


    def post_run(self):

        # Output using Dakota formatting
        self.executor.output_central_moments(self.fns)



# Simple test driver for sanity checking
if __name__ == "__main__":
    print("Hello from lhs_samplig module.")
