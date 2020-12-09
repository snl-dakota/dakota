#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________


# Tests of top-level Dakota Python interface

import dakpy

# Probably don't want / need this...
def test_cmd():
    print("\n+++ Constructing CommandLine...\n")
    dakcmd = dakpy.CommandLine("test_dakota.in")
    print("\n+++ Running CommandLine...\n")
    dakcmd.execute()
    print("\n+++ Done CommandLine.\n")

# This should be a better path
def test_lib():
    text_book_input = """
method,
  output silent
  max_function_evaluations 500
  ncsu_direct
    min_boxsize_limit 1.e-20
    volume_boxsize_limit 1.e-20
variables,
  continuous_design = 3
    initial_point  -2.3    2.3   0.23
    upper_bounds  10.0   10.0  10.0
    lower_bounds     -10.0  -10.0 -10.0
    descriptors 'x1'  'x2'  'x3'
interface,
  direct
    analysis_driver = 'text_book'
responses,
  num_objective_functions = 1
  no_gradients
  no_hessians
"""
    print("\n+++ Constructing LibEnv...\n")
    daklib = dakpy.LibEnv(input = "", input_string = text_book_input)
    print("\n+++ Running LibEnv...\n")
    daklib.execute()
#    print("\n+++ Final Functions:\n")
#    print(daklib.response_results(daklib))
    print("\n+++ Done LibEnv.\n")


if __name__ == "__main__":

    print("\n+++ Dakota version:\n")
    dakpy.version()

    # TODO: these encapsulate the objects so timing info is printed
    # with each test case; better manage destructors.
    test_cmd()
    test_lib()
