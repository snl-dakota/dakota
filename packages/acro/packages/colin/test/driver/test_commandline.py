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
# Test the COLIN driver commandline
#
import os
import pyutilib.th
import unittest

cwd = os.path.dirname(os.path.abspath(__file__))+os.sep

def filter_time(line):
    return "Time" in line

class Test(pyutilib.th.TestCase): pass

#Test.add_baseline_test(cmd="./colintest", baseline="data/exec1.qa", name="colintest, no args")
#Test.add_baseline_test(cmd="./colintest --help", baseline="data/exec2.qa", name="colintest --help")
#Test.add_baseline_test(cmd="./colintest --help-solvers", baseline="data/exec3.qa", name="colintest --help-solvers")
#Test.add_baseline_test(cmd="ampl colintest5.mod", baseline="data/exec4_5.qa", name="colin through AMPL")

if __name__ == "__main__":
    unittest.main()

