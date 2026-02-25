#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2025
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

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


def print_opt_results(executor, xvals, fval):

    c_labels = executor.continuous_variable_labels()

    # Output to Dakota output srteam using expected format, e.g. for regression testing
    executor.dak_print("<<<<< Best parameters          =")
    for i,x in enumerate(xvals):
        executor.dak_print(f"\t\t{xvals[i]} {c_labels[i]}")
    executor.dak_print("<<<<< Best objective function  =")
    executor.dak_print(f"\t\t{fval}")


# Simple test driver for sanity checking
if __name__ == "__main__":
    print("Hello from ext_py_helper module.")
