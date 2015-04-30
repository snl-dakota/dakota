#  _________________________________________________________________________
#
#  Acro: A Common Repository for Optimizers
#  Copyright (c) 2008 Sandia Corporation.
#  This software is distributed under the BSD License.
#  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
#  the U.S. Government retains certain rights in this software.
#  For more information, see the README.txt file in the top Acro directory.
#  _________________________________________________________________________
#

#
# Test the Coliny driver commandline
#
import pyutilib
import pyutilib_th
import unittest
import os
import os.path
import re


#
# The PyUnit testing class
#
class SolverOptions(pyutilib_th.TestCase): pass

#
# This function is called to test that solver options can be printed.
#
def solver_test(name):
    pyutilib.run_command("coliny --help-options "+name, outfile=currdir+"coliny_"+name+".out")
    INPUT = open(currdir+"coliny_"+name+".out","r")
    found_options=False
    lineno=0
    for line in INPUT:
        lineno=lineno+1
        tokens = re.split("[ \t]+",line.strip())
        if tokens[0] == "ERROR":
            return "Unexpected error at line "+str(lineno)+": "+line.strip()
        if tokens[0] == "Options":
            found_options=True
    if found_options:
        os.remove(currdir+"coliny_"+name+".out")
        return ""
    return "Failed to find options for solver '"+name+"'"

#
# Final all registered solvers
# 
currdir=os.path.dirname(os.path.abspath(__file__))+os.sep
pyutilib.run_command("coliny --help-solvers", outfile=currdir+"coliny.out")
solvers = []
INPUT=open(currdir+"coliny.out","r")
for line in INPUT:
    tokens = re.split("[ \t]+",line.strip())
    if ":" in line and len(tokens) == 1:
        solvers.append(tokens[0])
INPUT.close()
os.remove("coliny.out")
#
# Iterate through all registered solvers, adding tests to the 'SolverOptions'
# tesing class.
#
for solver in solvers:
    SolverOptions.add_fn_test(fn=solver_test, name=solver)

    
if __name__ == "__main__":
   unittest.main()

