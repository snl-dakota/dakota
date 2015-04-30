#
# knapsack tests driven by test_knap.yml
#

import os
import sys
from os.path import abspath, dirname
currdir = dirname(abspath(__file__))+os.sep

import pyutilib.autotest
import pyutilib.th as  unittest

pyutilib.autotest.create_test_suites(filename=currdir+'test_knap.yml', _globals=globals())

if __name__ == "__main__":
    unittest.main()

