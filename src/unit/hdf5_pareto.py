#!/usr/bin/env python
# _______________________________________________________________________
#
#    Dakota: Explore and predict with confidence.
#    Copyright 2014-2025
#    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#    This software is distributed under the GNU Lesser General Public License.
#    For more information, see the README file in the top Dakota directory.
# _______________________________________________________________________

import argparse
import sys
import unittest
import h5py
import h5py_console_extract as hce

_TEST_NAME = "pareto"

class Results(unittest.TestCase):

    def setUp(self):
        # This method will be called once for each test, so cache the moments
        try:
            self._results
        except AttributeError:
            self._results = hce.extract_pareto_set_results()
            self._best_labels = ["x1", "x2","x3"]
            self._function_labels = ["f1", "f2"]
            self._num_sets = 5

    def test_weights(self):
        # Compare the weights from Dakota console output to
        # the hdf5 output.
        console_num_sets = self._num_sets
        # Make sure console output is as expected
        self.assertEqual(console_num_sets, len(self._results["weights"]),
                msg="Number of sets in console output aren't as expected.")
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            hdf5_weights = h["/methods/pareto/results/execution:1/weights/"]
            hdf5_num_sets, hdf5_num_weights = hdf5_weights.shape
            self.assertEqual(hdf5_num_sets, console_num_sets, 
                    msg="Number of sets in hdf5 output not as expected.")
            self.assertEqual(hdf5_num_weights, len(self._function_labels),
                    msg="Number of weights in hdf5 output not as expected.")
            for cs, hs in zip(self._results["weights"], hdf5_weights):
                for cv, hv in zip(cs, hs):
                    self.assertAlmostEqual(cv, hv, msg="Difference in weights between console and hdf5")

    def test_best_parameters(self):
        # Compare the best points from Dakota console output to
        # the hdf5 output.
        console_labels = self._best_labels
        console_num_sets = self._num_sets
        # Make sure console output is as expected
        self.assertEqual(self._results["best_labels"], console_labels,
                msg="Best labels in console output aren't as expected.")
        self.assertEqual(console_num_sets, len(self._results["best"]),
                msg="Number of sets in console output aren't as expected.")
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            hdf5_best = h["/methods/pareto/results/execution:1/best_parameters/continuous"]
            hdf5_num_sets, hdf5_num_vars = hdf5_best.shape
            self.assertEqual(hdf5_num_sets, console_num_sets, 
                    msg="Number of sets in hdf5 output not as expected.")
            self.assertEqual(hdf5_num_vars, len(console_labels),
                    msg="Number of best variables in hdf5 output not as expected.")
            self.assertEqual(console_labels, hce.h5py_strings(hdf5_best.dims[1][0]),
                             msg="Unexpected label in best variables")                            
            for e_id, h_id in zip(list(range(1,console_num_sets+1)), hdf5_best.dims[0][0][:]):
                self.assertEqual(e_id, h_id, msg="Unexpected set_id in best variables")
            for cs, hs in zip(self._results["best"], hdf5_best):
                for cv, hv in zip(cs, hs):
                    self.assertAlmostEqual(cv, hv, msg="Difference in best points between console and hdf5")

    def test_best_responses(self):
        # Compare the best responses from Dakota console output to
        # the hdf5 output.
        console_labels = self._function_labels
        console_num_sets = self._num_sets
        # Make sure console output is as expected
        self.assertEqual(self._results["function_labels"], console_labels,
                msg="Function labels in console output aren't as expected.")
        self.assertEqual(console_num_sets, len(self._results["functions"]),
                msg="Number of sets in console output aren't as expected.")
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            hdf5_best = h["/methods/pareto/results/execution:1/best_responses"]
            hdf5_num_sets, hdf5_num_resps = hdf5_best.shape
            self.assertEqual(hdf5_num_sets, console_num_sets, 
                    msg="Number of sets in hdf5 output not as expected.")
            self.assertEqual(hdf5_num_resps, len(console_labels),
                    msg="Number of best resposnses in hdf5 output not as expected.")
            self.assertListEqual(console_labels, hce.h5py_strings(hdf5_best.dims[1][0]),
                                 msg="Unexpected label in responses")
            for e_id, h_id in zip(list(range(1,console_num_sets+1)), hdf5_best.dims[0][0][:]):
                self.assertEqual(e_id, h_id, msg="Unexpected set_id in responses")
            for cs, hs in zip(self._results["functions"], hdf5_best):
                for cv, hv in zip(cs, hs):
                    self.assertAlmostEqual(cv, hv, msg="Difference in best responses between console and hdf5")


class ResultsStructure(unittest.TestCase):

    def test_sources(self):
        expected_sources = set()
        expected_sources.add("local")
        with h5py.File(_TEST_NAME + ".h5", "r") as h:
            hdf5_sources = set(h["/methods/pareto/sources/"].keys())
            self.assertEqual(expected_sources, hdf5_sources,
                    msg="pareto sources not as expected.")

    def multi_start_results_objects(self):
        top_level = set(["best_parameters", "best_resposnes"])
        params = set(["continuous"])    

        with h5py.File(_TEST_NAME + ".h5","r") as h:
            hdf5_results = h["/methods/pareto/results/execution:1/"]
            h_top_level = set(hdf5_results.keys())
            self.assertEqual(h_top_level, top_level, msg="Unexpected top-level results in hdf5")
            h_best = set(hdf5_results["best_parameters"].keys())
            self.assertEqual(h_best, params, msg="Unexpected best parameter results in hdf5")

if __name__ == '__main__':
    # do some gyrations to extract the --bindir option from the comamnd line
    # while leaving the unittest options intact for it to parse
    parser = argparse.ArgumentParser()
    parser.add_argument('--bindir', dest="bindir")
    parser.add_argument('unittest_args', nargs='*')
    args = parser.parse_args()
    hce.set_executable_dir(args.bindir)
    hce.run_dakota("dakota_hdf5_" + _TEST_NAME + ".in")
    # Now set the sys.argv to the unittest_args (leaving sys.argv[0] alone)
    sys.argv[1:] = args.unittest_args
    unittest.main()
