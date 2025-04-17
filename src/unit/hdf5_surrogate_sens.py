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

_TEST_NAME = "surrogate_sens"

class Moments(unittest.TestCase):

    def setUp(self):
        # This method will be called once for each test, so cache the moments
        try:
            self._moments
        except AttributeError:
            self._moments = hce.extract_moments()
            self._cis = hce.extract_moment_confidence_intervals()

    def test_moments(self):
        # Extract the moments from Dakota console output and compare
        # them to the hdf5 output. Both the moments themselves and
        # the dimension scales are compared.
        console_moments = self._moments
        expected_scale_label = 'moments'
        expected_scale = ['mean', 'std_deviation', 'skewness', 'kurtosis']
        expected_descriptors = set(console_moments[0].keys())
        expected_num_incr = len(console_moments)

        with h5py.File(_TEST_NAME + ".h5","r") as h:
            # moments and scales
            for i in range(expected_num_incr):
                for r in expected_descriptors:
                    hdf5_moments = h["/methods/sampling/results/execution:1/moments/%s" % r]
                    for j in range(4):
                        self.assertAlmostEqual(console_moments[i][r][j], hdf5_moments[j])
                    scale_label = list(hdf5_moments.dims[0].keys())[0]
                    self.assertEqual(expected_scale_label, scale_label)
                    self.assertListEqual(expected_scale, hce.h5py_strings(hdf5_moments.dims[0][0])) 


    def test_moment_confidence_intervals(self):
        console_cis = self._cis
        expected_scale_labels = ['bounds', 'moments']
        expected_scales = [["lower", "upper"], ['mean', 'std_deviation']]
        expected_descriptors = set(console_cis[0].keys())
        expected_num_incr = len(console_cis)

        with h5py.File(_TEST_NAME + ".h5","r") as h:
            # CIs and scales
            for i, ci in enumerate(console_cis):
                hdf_cis = h["/methods/sampling/results/execution:1/moment_confidence_intervals/"]
                for r in expected_descriptors:
                    self.assertEqual(expected_scale_labels[0], list(hdf_cis[r].dims[0].keys())[0])
                    self.assertEqual(expected_scale_labels[1], list(hdf_cis[r].dims[1].keys())[0])
                    for j in range(2):
                        for k in range(2):
                            self.assertAlmostEqual(ci[r][j][k], hdf_cis[r][j,k])
                    self.assertListEqual(expected_scales[0], hce.h5py_strings(hdf_cis[r].dims[0][0]))
                    self.assertListEqual(expected_scales[1], hce.h5py_strings(hdf_cis[r].dims[1][0]))


class Correlations(unittest.TestCase):
    def setUp(self):
        try:
            self._simple
        except AttributeError:
            self._simple = hce.extract_simple_correlations()
            self._simple_rank = hce.extract_simple_rank_correlations()
            self._partial = hce.extract_partial_correlations()
            self._partial_rank = hce.extract_partial_rank_correlations()

    def correlation_helper(self, corr_type=None):
        # Verify the following:
        # 0. Expected number of increments are present
        # 1. dimenions of the matrices match
        # 2. factors match
        # 3. Elements match
        exec_linkname = "/methods/sampling/results/execution:1"
        if corr_type == "pearson":
            console_corrs = self._simple
            dataset_linkname = "/simple_correlations"
        else:
            console_corrs = self._simple_rank
            dataset_linkname = "/simple_rank_correlations"
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            # Check number and names of increment groups
            for i, corr in enumerate(console_corrs):
                # Dimensionality of datasets
                hdf_simple = h[exec_linkname+dataset_linkname]
                self.assertEqual(len(corr[1]), hdf_simple.shape[0])
                self.assertEqual(len(corr[1][0]), hdf_simple.shape[1])
                # factors
                self.assertListEqual(corr[0], hce.h5py_strings(hdf_simple.dims[0][0]))
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
        # 0. Expected number of increments are present
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
        expected_num_incr = len(console_corrs)
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            for i, corr in enumerate(console_corrs):
                # Dimensionality of datasets
                hdf_partial = h[exec_linkname+dataset_linkname]
                # verify same responses are present in both
                self.assertEqual(set(corr.keys()), set(hdf_partial.keys()))
                # for each response, verify data are the same length, factors are same,
                # and data are the same
                for resp in list(corr.keys()):
                    self.assertEqual(len(corr[resp][1]), hdf_partial[resp].shape[0])
                    self.assertListEqual(corr[resp][0], hce.h5py_strings(hdf_partial[resp].dims[0][0]))
                    for cd, hd in zip(corr[resp][1], hdf_partial[resp]):
                        self.assertAlmostEqual(cd, hd, 5)

    def test_partial_correlations(self):
        self.partial_correlation_helper("pearson")

    def test_partial_rank_correlations(self):
        self.partial_correlation_helper("spearman")

class EvaluationsStructure(unittest.TestCase):

    def test_interface_presence(self):
        expected_interfaces = {"NO_ID":["truth_m"],
                               "APPROX_INTERFACE_1":["surr"]}
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            self.assertListEqual(sorted(list(expected_interfaces.keys())), 
                    sorted(list(h["/interfaces"].keys())))
            for k, g in list(h["/interfaces"].items()):
                self.assertListEqual(expected_interfaces[k], list(g.keys()))

    def test_model_presence(self):
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            with self.assertRaises((KeyError,ValueError)):
                h["/models/"]

    def test_sources(self):
        with h5py.File(_TEST_NAME + ".h5", "r") as h:
            with self.assertRaises((KeyError,ValueError)):
                h["/methods/sampling/sources/"]
            with self.assertRaises((KeyError,ValueError)):
                h["/models/dace/sources/"]

class TabularData(unittest.TestCase):
    def test_tabular_data(self):
        # normally we'd compare to the top method's model, but models are turned off
        # for this test. Instead we'll compare to the approx interface. No recasts
        # occur in this test, so they should be the same.
        tdata = hce.read_tabular_data(_TEST_NAME + ".dat")
        metadata = ["%eval_id", "interface"]
        variables = ["x1", "x2","x3"]
        responses = ["f"]
        descriptors = metadata + variables + responses
        self.assertListEqual(descriptors, list(tdata.keys()))

        with h5py.File(_TEST_NAME + ".h5", "r") as h:
            # Variables
            hvars = h["/interfaces/APPROX_INTERFACE_1/surr/variables/continuous"]
            self.assertListEqual(variables, hce.h5py_strings(hvars.dims[1][0]))
            for i, v in enumerate(variables):
                for eid, tv, hv in zip(tdata["%eval_id"], tdata[v], hvars[:,i]):
                    self.assertAlmostEqual(tv, hv, msg="Bad comparison for variable '%s' for eval %d" % (v,eid), places=9)
            hresps = h["/interfaces/APPROX_INTERFACE_1/surr/responses/functions"]
            # Responses
            self.assertListEqual(responses, hce.h5py_strings(hresps.dims[1][0]))
            for i, r in enumerate(responses):
                for eid, tr, hr in zip(tdata["%eval_id"],tdata[r], hresps[:,i]):
                    self.assertAlmostEqual(tr, hr, msg="Bad comparison for response '%s'" % r, places=9)

class RestartData(unittest.TestCase):
    def test_restart_data(self):
        rdata = hce.read_restart_file(_TEST_NAME + ".rst")
        variables = ["x1", "x2","x3"]
        responses = ["f"]
        ac = ["component1","component2"]
        self.assertListEqual(variables, list(rdata["variables"]["continuous"].keys()))
        self.assertListEqual(responses, list(rdata["response"].keys()))
        
        with h5py.File(_TEST_NAME + ".h5", "r") as h:
            # Variables
            hvars = h["/interfaces/NO_ID/truth_m/variables/continuous"]
            self.assertListEqual(variables, hce.h5py_strings(hvars.dims[1][0]))
            for i, v in enumerate(variables):
                for eid, tv, hv in zip(rdata["eval_id"], rdata["variables"]["continuous"][v], hvars[:,i]):
                    self.assertAlmostEqual(tv, hv, msg="Bad comparison for variable '%s' for eval %d" % (v,eid), places=9)
            hresps_f = h["/interfaces/NO_ID/truth_m/responses/functions"]
            hresps_g = h["/interfaces/NO_ID/truth_m/responses/gradients"]
            hasv = h["/interfaces/NO_ID/truth_m/properties/active_set_vector"]
            hdvv = h["/interfaces/NO_ID/truth_m/properties/derivative_variables_vector"]
            hac = h["/interfaces/NO_ID/truth_m/properties/analysis_components"]
            # ASV
            for r_asv, h_asv in zip(rdata["asv"], h["/interfaces/NO_ID/truth_m/properties/active_set_vector"]):
                for r_a, h_a in zip(r_asv, h_asv):
                    self.assertEqual(r_a, h_a)
            #DVV
            dvv_lookup = hdvv.dims[1][1]
            for r_dvv, h_dvv in zip(rdata["dvv"],hdvv):
                h_dvv_ids = []
                for dvv_id, dvv_bool in zip(dvv_lookup, h_dvv):
                    if dvv_bool == 1:
                        h_dvv_ids.append(dvv_id)
                self.assertListEqual(r_dvv, h_dvv_ids)
            # Analysis Components
            self.assertEqual(len(ac), len(hac), "Unexpected number of analysis components")
            self.assertListEqual(ac, hce.h5py_strings(hac), "Analysis components in file not as expected.")
            # Responses
            # extract the portion of the gradient corresponding to the DVV
            def extract_gradient(full_grad, dvv):
                grad = []
                for d, g in zip(dvv, full_grad):
                    if d == 1:
                        grad.append(g)
                return grad
            self.assertListEqual(responses, hce.h5py_strings(hresps_f.dims[1][0]))
            for i, r in enumerate(responses):
                for eid, a, d, tr, hf, hg in zip(rdata["eval_id"], hasv, hdvv, rdata["response"][r], hresps_f[:,i], hresps_g[:,i,:]):
                    if a & 1:
                        self.assertAlmostEqual(tr["function"], hf, 
                                msg="Bad comparison for response '%s' for eval %d" % (r, eid), places=9)
                    if a & 2:
                        extracted_hg = extract_gradient(hg, d)
                        self.assertEqual(len(tr["gradient"]), len(extracted_hg), msg="Lengths of gradients " +
                                "for '%s' for eval %d are inconsistent" % (r, eid))
                        for c1, c2 in zip(tr["gradient"], extracted_hg):
                            self.assertAlmostEqual(c1, c2, places=9, msg="Gradients differ for " +
                                "'%s' in eval %d" %(r, eid))


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
