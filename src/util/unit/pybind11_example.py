#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2023
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

# Test the Dakota tree CMake build of pybind11 wrapper

import pybind11_example as py_ex
import unittest


class TestAdd(unittest.TestCase):

    def test_add(self):
        self.assertEqual(py_ex.add(1, 2), 3, "Should be 3")

if __name__ == '__main__':
    unittest.main()
