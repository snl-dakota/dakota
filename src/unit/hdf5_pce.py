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

_TEST_NAME = "pce"

class Moments(unittest.TestCase):

    def setUp(self):
        # This method will be called once for each test, so cache the moments
        try:
            self._moments
        except AttributeError:
            self._moments = hce.extract_expansion_moments()

    def test_structure(self):
        # Verify the structure (execution id, descriptors)
        console_moments = self._moments[0]
        expected_descriptors = set(console_moments.keys())
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            # descriptors
            hdf5_descriptors = set(h["/methods/pce/results/execution:1/expansion_moments/"].keys())
            self.assertEqual(expected_descriptors, hdf5_descriptors)

    def test_moments(self):
        # Extract the moments from Dakota console output and compare
        # them to the hdf5 output. Both the moments themselves and
        # the dimension scales are compared.
        console_moments = self._moments[0]
        expected_scale_label = 'moments'
        expected_scale = ['mean', 'std_deviation', 'skewness', 'kurtosis']
        expected_descriptors = set(console_moments.keys())

        with h5py.File(_TEST_NAME + ".h5","r") as h:
            # moments and scales
            for r in expected_descriptors:
                hdf5_moments = h["/methods/pce/results/execution:1/expansion_moments/%s" % r]
                for j in range(2):
                    self.assertAlmostEqual(console_moments[r][j], hdf5_moments[j])
                scale_label = list(hdf5_moments.dims[0].keys())[0]
                self.assertEqual(expected_scale_label, scale_label)
                for es, s in zip(expected_scale, hce.h5py_strings(hdf5_moments.dims[0][0])):
                    self.assertEqual(es,s)

class PDFs(unittest.TestCase):
    def setUp(self):
        try:
            self._pdfs
        except AttributeError:
            self._pdfs = hce.extract_pdfs()

    def test_pdfs(self):
        console_pdfs = self._pdfs[0]
        expected_scale_labels = set(('lower_bounds','upper_bounds'))
        expected_descriptors = set(console_pdfs.keys())

        with h5py.File(_TEST_NAME + ".h5","r") as h:
            # pdfs and scales
            for r in expected_descriptors:
                hdf5_pdf = h["/methods/pce/results/execution:1/probability_density/%s" % r]
                console_pdf = console_pdfs[r]
                # same number of bins
                self.assertEqual(len(console_pdf), len(hdf5_pdf))
                # labels
                self.assertEqual(expected_scale_labels, set(hdf5_pdf.dims[0].keys()))
                for j in range(len(hdf5_pdf)):
                    # Lower bound
                    self.assertAlmostEqual(console_pdf[j][0], hdf5_pdf.dims[0][0][j])
                    # Upper bound
                    self.assertAlmostEqual(console_pdf[j][1], hdf5_pdf.dims[0][1][j])
                    # Density
                    self.assertAlmostEqual(console_pdf[j][2], hdf5_pdf[j])

class LevelMappings(unittest.TestCase):
    def setUp(self):
        try:
            self._mappings
        except AttributeError:
            self._mappings = hce.extract_level_mappings()

    def test_level_mappings(self):
        console_mappings = self._mappings[0]
        expected_descriptors = set(console_mappings.keys())

        # This test is tricky because it is not clear from the table of level mappings in the
        # table of console output what was specified by the user and what Dakota calculated.
        # Nevertheless, we know that:
        # 1. Dakota writes mappings for any user-specified response_levels first
        # 2. For the user-specified response_levels, Dakota will calculate exactly one of:
        #    probability, reliability, or generalized reliability
        # 3. After results for any response_levels, are written, Dakota reports results for
        #    user specified probability_levels, reliability_levels, and then generalized_
        #    reliability_levels, in that order.
        #
        # So, the testing strategy is, for each execution and response:
        # 1. Verify that the total number of levels (rows in the table) from the console equals 
        #    the number in the hdf5 file.
        # 2. Verify that the first entries in the table match the hdf5 response_levels. This is
        #    tricky because computed results may be reported in any of the remaining three columns,
        #    depending on what the user requested using the 'compute' keyword.
        # 3. Verify that the following rows match any probability, reliability, and gen. rel. results
        #    from the hdf5 file. These are easier because we know which columns to find the data and
        #    scales in.
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            for r in expected_descriptors:
                # Create a set of the results present in the HDF5 file for convenience
                has_result = set()
                for result_type in ('response_levels', 'probability_levels', 
                        'reliability_levels', 'gen_reliability_levels'):
                    try:
                        h["/methods/pce/results/execution:1//%s/%s" % (result_type, r)]
                        has_result.add(result_type)
                    except (KeyError, ValueError):
                        pass
                # Verify the dimension scale label of the datasets
                for label in has_result:
                    self.assertEqual(label,
                        list(h["/methods/pce/results/execution:1/%s/%s" % (label, r)].dims[0].keys())[0])

                # compare the total number of results in the console and hdf5 file
                num_console_rows = len(console_mappings[r])
                num_hdf5_rows = 0
                for result_type in has_result:
                    num_hdf5_rows += len(h["/methods/pce/results/execution:1/%s/%s" % (result_type, r)])
                self.assertEqual(num_console_rows, num_hdf5_rows)
                # Verify any response_levels
                num_response_levels = 0
                if 'response_levels' in has_result:
                    # Determine the 'compute'd column for the response_levels
                    for j, c in enumerate(console_mappings[r][0][1:]):
                        if c is not None:
                            console_ri = j+1
                            break
                    # Loop over the response_levels and verify the user request and computed values
                    response_ds = h["/methods/pce/results/execution:1/response_levels/%s" % r]
                    num_response_levels = len(response_ds)
                    for j in range(num_response_levels):
                        self.assertAlmostEqual(console_mappings[r][j][console_ri], response_ds[j])
                        self.assertEqual(console_mappings[r][j][0], response_ds.dims[0][0][j])
                    # Remove response_levels from the results because it has been processed
                    has_result.remove('response_levels')
                # Verify any prob., rel., and gen. rel. levels
                begin = num_response_levels
                for j, result_type in enumerate(('probability_levels', 'reliability_levels', 'gen_reliability_levels')):
                    if result_type in has_result:
                        response_ds =  h["/methods/pce/results/execution:1/%s/%s" % (result_type, r)]
                        num_hdf5_rows = len(response_ds)
                        end = begin+num_hdf5_rows
                        for cr, hr, s in zip(console_mappings[r][begin:end], response_ds, response_ds.dims[0][0]):
                            # Results
                            self.assertAlmostEqual(cr[0], hr)
                            # Scales
                            self.assertEqual(cr[j+1], s)
                        begin += num_hdf5_rows


class Sobol(unittest.TestCase):
    def setUp(self):
        try:
            self._sobol
        except AttributeError:
            self._sobol = hce.extract_sobol_indices()

    def test_structure(self):
        with h5py.File(_TEST_NAME + ".h5", "r") as h:
            results = h["/methods/pce/results/execution:1/"]
            # main effects, total effects, and all interactions are present in hdf5
            for console_order in self._sobol:
                self.assertTrue(console_order in results)
            # responses are present
            for console_order, response_data in self._sobol.items():
                self.assertEqual(len(response_data), len(results[console_order]))
                for r in response_data:
                    self.assertTrue(r in results[console_order])
            # variables
            for console_order, responses_data in self._sobol.items():
                for response_label, response_data  in responses_data.items():
                    if "order" in console_order:
                        for c_vars, h_vars in zip(response_data["variables"], results[console_order][response_label].dims[0][0]):
                            c_vars_set = set(c_vars)
                            h_vars_set = set([h.decode('utf-8') if isinstance(h, bytes) else h for h in h_vars])
                            self.assertEqual(c_vars_set, h_vars_set)
                    else:
                        for c_var, h_var in zip(response_data["variables"], results[console_order][response_label].dims[0][0]):
                            h_var_test = h_var.decode('utf-8') if isinstance(h_var, bytes) else h_var
                            self.assertEqual(c_var, h_var_test)
 
    def test_indices(self):
        with h5py.File(_TEST_NAME + ".h5", "r") as h:
            results = h["/methods/pce/results/execution:1/"]
            for console_order, responses_data in self._sobol.items():
                for response_label, response_data  in responses_data.items():
                    for c_val, h_val in zip(response_data["indices"], results[console_order][response_label]):
                        self.assertAlmostEqual(c_val, h_val, places=6)
                       
class EvaluationsStructure(unittest.TestCase):

    def test_interface_presence(self):
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            with self.assertRaises((KeyError,ValueError)):
                h["/interfaces"]

    def test_model_presence(self):
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            h["/models/simulation/NO_MODEL_ID/"]

    def test_sources(self):
        with h5py.File(_TEST_NAME + ".h5", "r") as h:
            method_sources = [k for k in h["/methods/pce/sources/"]]
            self.assertListEqual(method_sources,["NO_MODEL_ID"])
            with self.assertRaises((KeyError,ValueError)):
                h["/models/simulation/NO_MODEL_ID/sources/"]

class TabularData(unittest.TestCase):
    def test_tabular_data(self):
        tdata = hce.read_tabular_data(_TEST_NAME + ".dat")
        metadata = ["%eval_id", "interface"]
        variables = ["x1", "x2", "x3"]
        responses = ["f", "c"]
        descriptors = metadata + variables + responses
        self.assertListEqual(descriptors, list(tdata.keys()))

        with h5py.File(_TEST_NAME + ".h5", "r") as h:
            # Variables
            hvars = h["/models/simulation/NO_MODEL_ID/variables/continuous"]
            self.assertListEqual(variables, hce.h5py_strings(hvars.dims[1][0]))
            for i, v in enumerate(variables):
                for eid, tv, hv in zip(tdata["%eval_id"], tdata[v], hvars[:,i]):
                    self.assertAlmostEqual(tv, hv, msg="Bad comparison for variable '%s' for eval %d" % (v,eid), places=9)
            hresps = h["/models/simulation/NO_MODEL_ID/responses/functions"]
            # Responses
            self.assertListEqual(responses, hce.h5py_strings(hresps.dims[1][0]))
            for i, r in enumerate(responses):
                for eid, tr, hr in zip(tdata["%eval_id"],tdata[r], hresps[:,i]):
                    self.assertAlmostEqual(tr, hr, msg="Bad comparison for response '%s'" % r, places=9)


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
