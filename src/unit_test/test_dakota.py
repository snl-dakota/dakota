#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________


# Tests of top-level Dakota Python interface

import sys
#import numpy as np

# Optionally append a path to the python library, passed as argv[1]
if len(sys.argv) > 1:
    dakpy_lib_path = sys.argv[1]
    sys.path.append(dakpy_lib_path)


def text_book(**kwargs):

    num_fns = kwargs['functions']
    x = kwargs['cv']
    ASV = kwargs['asv']

    retval = dict([])

    if (ASV[0] & 1): # **** f:
        fn = 0.0
        for val in x:
            fn += pow(val-1.0, 4)
        f = [fn]
        retval['fns'] = f

    # The gradient format causes Dakota to complain that the matrix must have 1 row ...
    #   I think it does. ? 
    if (ASV[0] & 2): # **** df/dx:
        g = []
        for val in x:
            g.append( 4.0*pow(val-1.0, 3) )
        retval['fnGrads'] = [g]

    if (ASV[0] & 4): # **** d^2f/dx^2:
        raise("Hessians not currently supported for this driver.")

    return(retval)


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
        environment,
          results_output
            hdf5
            results_output_file = 'test.dakota'
        method,
          output silent
          max_function_evaluations 500
          ncsu_direct
            min_boxsize_limit 1.e-20
            volume_boxsize_limit 1.e-20
        variables,
          continuous_design = 3
            initial_point  -2.3    2.3   0.23
            upper_bounds   10.0   10.0   10.0
            lower_bounds  -10.0  -10.0  -10.0
            descriptors     'x1'  'x2'   'x3'
        interface,
          python
            analysis_driver = 'test_dakota:text_book'
        responses,
          num_objective_functions = 1
          no_gradients
          no_hessians
"""
    print("\n+++ Constructing LibEnv...\n")
    daklib = dakpy.LibEnv(input = "", input_string = text_book_input)
    print("\n+++ Running LibEnv...\n")
    daklib.execute()
    print("\n+++ Final Functions:\n")
    print("\tUsing free fn: "+str(dakpy.get_response_fn_val(daklib)))
    resp_res = daklib.response_results()
    print("\tUsing wrapped objs: "+str(resp_res.function_value(0)))
    assert(resp_res.function_value(0) < 1.e-20)
    vars_res = daklib.variables_results()
    #print("\tNumber active continuous variables: "+str(vars_res.num_active_cv()))
    assert(vars_res.num_active_cv() == 3)
    # --- Requires numpy
    #dak_vars = dakpy.get_variable_values_np(daklib)
    #print(dak_vars)
    # --- Default: uses python arrays
    dak_vars2 = dakpy.get_variable_values(daklib)
    #print(dak_vars2)
    target = 1.0;
    max_tol = 1.e-4;
    assert(abs((dak_vars2[0] - target)/target) < max_tol)
    assert(abs((dak_vars2[1] - target)/target) < max_tol)
    assert(abs((dak_vars2[2] - target)/target) < max_tol)
    print("\n+++ Done LibEnv.\n")

    # Conditionally test values written to the h5 file if h5py is available
    test_dakota_has_h5py = True
    try:
        import h5py
        # This variant did not seem to work ...
        #__import__(h5py)
    except ImportError:
        print("Module h5py not found. Skipping check of hdf5 file values.")
        test_dakota_has_h5py = False

    if test_dakota_has_h5py:
        with h5py.File("test.dakota.h5", "r") as h:
            hresps = h["/methods/NO_METHOD_ID/results/execution:1/best_objective_functions"]
            hvars =  h["/methods/NO_METHOD_ID/results/execution:1/best_parameters/continuous"]
            assert(hresps[0] < 1.e-20)
            assert(abs((hvars[0] - target)/target) < max_tol)
            assert(abs((hvars[1] - target)/target) < max_tol)
            assert(abs((hvars[2] - target)/target) < max_tol)



if __name__ == "__main__":

    print("\n+++ Dakota version:\n")
    dakpy.version()

    # TODO: these encapsulate the objects so timing info is printed
    # with each test case; better manage destructors.
    test_cmd()
    test_lib()
