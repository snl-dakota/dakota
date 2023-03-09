#!/usr/bin/env python
#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014 Sandia Corporation.
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

import argparse
import sys
import unittest
import h5py
import h5py_console_extract as hce

_TEST_NAME = "mlmc"

class Moments(unittest.TestCase):

    def setUp(self):
        # This method will be called once for each test, so cache the moments
        try:
            self._moments
        except AttributeError:
            self._moments = hce.extract_moments()

    def test_structure(self):
        # Verify the structure (execution id, descriptors)
        console_moments = self._moments
        expected_descriptors = set(console_moments[0].keys())
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            # descriptors
            hdf5_descriptors = set(h["/methods/NO_METHOD_ID/results/execution:1/moments/"].keys())
            self.assertEqual(expected_descriptors, hdf5_descriptors)

    def test_moments(self):
        # Extract the moments from Dakota console output and compare
        # them to the hdf5 output. Both the moments themselves and
        # the dimension scales are compared.
        console_moments = self._moments
        expected_scale_label = 'moments'
        expected_scale = ['mean', 'std_deviation', 'skewness', 'kurtosis']
        expected_descriptors = set(console_moments[0].keys())

        with h5py.File(_TEST_NAME + ".h5","r") as h:
            # moments and scales
            for r in expected_descriptors:
                hdf5_moments = h["/methods/NO_METHOD_ID/results/execution:1/moments/%s" % r]
                for j in range(4):
                    self.assertAlmostEqual(console_moments[0][r][j], hdf5_moments[j])
                scale_label = list(hdf5_moments.dims[0].keys())[0]
                self.assertEqual(expected_scale_label, scale_label)
                self.assertListEqual(expected_scale, hce.h5py_strings(hdf5_moments.dims[0][0]))


class EquivHFAttribute(unittest.TestCase):

    def test_attribute(self):
        console_eq = hce.extract_equiv_num_hf_evals()
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            method_group = h["/methods/NO_METHOD_ID/results/execution:1/"]
            hdf5_eq = method_group.attrs["equiv_hf_evals"]
        self.assertAlmostEqual(console_eq, hdf5_eq)

#class EvaluationsStructure(unittest.TestCase):
#
#    def test_interface_presence(self):
#        with h5py.File(_TEST_NAME + ".h5","r") as h:
#            with self.assertRaises((KeyError,ValueError)):
#                h["/interfaces"]
#
#    def test_model_presence(self):
#        with h5py.File(_TEST_NAME + ".h5","r") as h:
#            h["/models/simulation/NO_MODEL_ID/"]
#
#    def test_sources(self):
#        with h5py.File(_TEST_NAME + ".h5", "r") as h:
#            method_sources = [k for k in h["/methods/sampling/sources/"]]
#            self.assertItemsEqual(method_sources,["NO_MODEL_ID"])
#            with self.assertRaises((KeyError,ValueError)):
#                h["/models/simulation/NO_MODEL_ID/sources/"]
#
#class TabularData(unittest.TestCase):
#    def test_tabular_data(self):
#        tdata = hce.read_tabular_data(_TEST_NAME + ".dat")
#        metadata = ["%eval_id", "interface"]
#        variables = ["x1", "x2", "x3"]
#        responses = ["f", "c"]
#        descriptors = metadata + variables + responses
#        self.assertItemsEqual(descriptors, tdata.keys())
#
#        with h5py.File(_TEST_NAME + ".h5", "r") as h:
#            # Variables
#            hvars = h["/models/simulation/NO_MODEL_ID/variables/continuous"]
#            self.assertItemsEqual(variables, hvars.dims[1][0][:])
#            for i, v in enumerate(variables):
#                for eid, tv, hv in zip(tdata["%eval_id"], tdata[v], hvars[:,i]):
#                    self.assertAlmostEqual(tv, hv, msg="Bad comparison for variable '%s' for eval %d" % (v,eid), places=9)
#            hresps = h["/models/simulation/NO_MODEL_ID/responses/functions"]
#            # Responses
#            self.assertItemsEqual(responses, hresps.dims[1][0][:])
#            for i, r in enumerate(responses):
#                for eid, tr, hr in zip(tdata["%eval_id"],tdata[r], hresps[:,i]):
#                    self.assertAlmostEqual(tr, hr, msg="Bad comparison for response '%s'" % r, places=9)


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
