"""This module provides a Dakota parameter file reader, results file writer, 
and objects for accessing the contents of the parameter file and for 
collecting response data. It interprets the active set vector and prevents 
(by raising exceptions) the user from returning response data to Dakota that 
was not requested.

Example Usage::

    import dakota.interfacing as di
    # Read and parse the parameters file and construct Parameters and Results 
    # objects
    params, results = di.read_parameters_file("params.in", "results.out")
    # Accessing variables
    x1 = params["x1"]
    x2 = params["x2"]
    # Run a fictitious Python-based simulation and store the result in the
    # function value of the 'f1' response.
    results["f1"].function =  sim_results = my_simulation(x1, x2)
    results.write()

Other members and methods of the Parameters and Results objects are described
below. A more full-featured example is available in 
``examples/script_interfaces/Python``.
"""
## The docstring for this "module" properly belongs to interfacing.py. Since
## the contents of that module are imported into this one, it's been placed
## here so that sphinx and other documentation tools produce sensible output

__author__ = 'J. Adam Stephens'
__copyright__ = 'Copyright 2014 Sandia Corporation'
__license__ = 'GNU Lesser General Public License'

from .interfacing import *

__all__=["parallel"]

