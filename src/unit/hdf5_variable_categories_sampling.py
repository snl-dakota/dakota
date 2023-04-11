#!/usr/bin/env python
#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014 Sandia Corporation.
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

import argparse
import math
import sys
import unittest
import h5py
import h5py_console_extract as hce

# This test serves two purposes.
# First, it ensures that correlation matrices and labels are printed
# and stored in a consistent order when variables that belong to
# two different "categories" (here, aleatory uncertain and design)
# are present in a sampling study. (The expected ordering is "input spec":
# first the design, then the aleatory uncertain).
# Second, it serves as a tiny verification test of sampling results. I
# determined the "correct" pearson and spearman coefficients using
# a separate tool (numpy functions) and large number of samples.
# The partial spearson coefficients are "obvious". The partial spearman
# are not, and they are not trivial to calculate, so they aren't verified.


_TEST_NAME = "variable_categories_sampling"

class Correlations(unittest.TestCase):
    def setUp(self):
        self._simple = hce.extract_simple_correlations()
        self._simple_rank = hce.extract_simple_rank_correlations()
        self._partial = hce.extract_partial_correlations()
        self._partial_rank = hce.extract_partial_rank_correlations()

    def correlation_helper(self, corr_type=None):
        # Verify the following:
        # 1. dimensions of the matrices match
        # 2. factors match
        # 3. Elements match
        exec_linkname = "/methods/sampling/results/execution:1"
        if corr_type == "pearson":
            console_corrs = self._simple
            dataset_linkname = "/simple_correlations"
        else:
            console_corrs = self._simple_rank
            dataset_linkname = "/simple_rank_correlations"
        with h5py.File(_TEST_NAME + ".h5", "r") as h:
            for i, corr in enumerate(console_corrs):
                # Dimensionality of datasets
                hdf_simple = h[exec_linkname + dataset_linkname]
                self.assertEqual(len(corr[1]), hdf_simple.shape[0])
                self.assertEqual(len(corr[1][0]), hdf_simple.shape[1])
                # factors
                hdf_factors = hce.h5py_strings(hdf_simple.dims[0][0])
                for cf, hf in zip(corr[0], hdf_factors):
                    self.assertEqual(cf, hf)
                # elements
                n = len(corr[1])
                for i in range(n):
                    for j in range(n):
                        self.assertAlmostEqual(corr[1][i][j], hdf_simple[i,j], 5)
        
    def test_simple_correlations(self):
        self.correlation_helper("pearson")

    def test_simple_rank_correlations(self):
        self.correlation_helper("spearman")

    def partial_correlation_helper(self, corr_type=None):
        # Verify the following:
        # 1. dimenions of the matrices match
        # 2. factors match
        # 3. Elements match
        exec_linkname = "/methods/sampling/results/execution:1"
        if corr_type == "pearson":
            console_corrs = self._partial
            dataset_linkname = "/partial_correlations"
        else:
            console_corrs = self._partial_rank
            dataset_linkname = "/partial_rank_correlations"
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            # Check number and names of increment groups
            for i, corr in enumerate(console_corrs):
                # Dimensionality of datasets
                hdf_partial = h[exec_linkname + dataset_linkname]
                # verify same responses are present in both
                self.assertEqual(set(corr.keys()), set(hdf_partial.keys()))
                # for each response, verify data are the same length, factors are same,
                # and data are the same
                for resp in list(corr.keys()):
                    self.assertEqual(len(corr[resp][1]), hdf_partial[resp].shape[0])
                    for cf, hf in zip(corr[resp][0], hce.h5py_strings(hdf_partial[resp].dims[0][0])):
                        self.assertEqual(cf, hf)
                    for cd, hd in zip(corr[resp][1], hdf_partial[resp]):
                        if math.isnan(cd) or math.isnan(hd):
                            self.assertTrue(math.isnan(cd) and math.isnan(hd))
                        else:
                            self.assertAlmostEqual(cd, hd, 5)

    def test_partial_correlations(self):
        self.partial_correlation_helper("pearson")

    def test_partial_rank_correlations(self):
        self.partial_correlation_helper("spearman")

    def test_verify_simple_correlations(self):
        x3_f, x1_f, x2_f, _ = self._simple[0][1][3]
        self.assertAlmostEqual(x3_f, 0.0, 2)
        self.assertAlmostEqual(x1_f, 0.894, 2) 
        self.assertAlmostEqual(x2_f, 0.447, 2)

    def test_verify_simple_rank_correlations(self):
        x3_f, x1_f, x2_f, _ = self._simple_rank[0][1][3]
        self.assertAlmostEqual(x3_f, 0.0, 2)
        self.assertAlmostEqual(x1_f, 0.885, 2) 
        self.assertAlmostEqual(x2_f, 0.431, 2)

    def test_verify_partial_correlations(self):
        x3, x1, x2 = self._partial[0]["f"][1]
        self.assertTrue(math.isnan(x3))
        self.assertAlmostEqual(x1, 1.0, 5)
        self.assertAlmostEqual(x2, 1.0, 5)

    def test_verify_labels(self):
        label_1, label_2, label_3, label_4 = self._simple[0][0]
        self.assertEqual(label_1, "x3")    
        self.assertEqual(label_2, "x1")    
        self.assertEqual(label_3, "x2")    
        self.assertEqual(label_4, "f")    

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
