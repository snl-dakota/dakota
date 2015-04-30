import os
import sys
from os.path import abspath, dirname
currdir = dirname(abspath(__file__))+os.sep

import pyutilib.th as unittest

class Tester(unittest.TestCase):

    def test_pass(self):
        pass

    @unittest.expectedFailure
    def test_fail(self):
        self.fail("test_fail will always fail")

    @unittest.skip("demonstrating skipping")
    def test_skip(self):
        self.fail("test_skip will always be skipped")

