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
import coopr.opt.testing
import os
import unittest

class Test(coopr.opt.testing.TestCase): pass

#Test.add_coopr_results_test(name="exec4_1", baseline="data/exec4_1.qa", cmd="ampl colintest1.mod")
#Test.add_coopr_results_test(name="exec4_1", baseline="data/exec4_2.qa", cmd="ampl colintest2.mod")
#Test.add_coopr_results_test(name="exec4_1", baseline="data/exec4_3.qa", cmd="ampl colintest3.mod")
#Test.add_coopr_results_test(name="exec4_1", baseline="data/exec4_4.qa", cmd="ampl colintest4.mod")


if __name__ == "__main__":
    unittest.main()

