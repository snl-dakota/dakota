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

_TEST_NAME = "simple_calibration"

class Structure(unittest.TestCase):
    def test_structure(self):
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            exec_group = h["/methods/NO_METHOD_ID/results/execution:1/"]
            contents = set(exec_group.keys())
            self.assertEqual(len(contents), 5) # correct number of groups
            self.assertTrue("best_residuals" in contents)
            self.assertTrue("best_constraints" in contents)
            self.assertTrue("best_norm" in contents)
            self.assertTrue("confidence_intervals" in contents)

            contents = set(exec_group["best_parameters"].keys())
            # number of variable types
            self.assertEqual(len(contents), 1) # correct number of groups
            self.assertTrue("best_parameters/continuous" in exec_group)

class Results(unittest.TestCase):

    def setUp(self):
        # This method will be called once for each test, so cache the moments
        try:
            self._params
        except AttributeError:
            self._params = hce.extract_best_parameters()
            self._residuals = hce.extract_best_residuals()
            self._constraints = hce.extract_best_constraints()
            self._norms = hce.extract_best_residual_norms()
            self._cis = hce.extract_best_parameter_confidence_intervals()

    def test_parameters(self):
        self.assertEqual(len(self._params), 1)
        console_params = self._params[0]
        expected_descriptors = set(var[0] for var in console_params)
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            params = h["/methods/NO_METHOD_ID/results/execution:1/best_parameters/continuous"]
            # Descriptors match
            h5_descriptors = set(hce.h5py_strings(params.dims[0][0]))
            self.assertEqual(expected_descriptors, h5_descriptors)
            ## Verify the values
            for (label, c_val), h5_val in zip(console_params, params[()]):
                self.assertAlmostEqual(c_val, h5_val)

    def test_residuals(self):
        self.assertEqual(len(self._residuals), 1)
        console_residuals = self._residuals[0]
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            h5_residuals = h["/methods/NO_METHOD_ID/results/execution:1/best_residuals"]
            self.assertEqual(len(console_residuals), h5_residuals.shape[0])
            for c_r, h_r in zip(console_residuals, h5_residuals[()]):
                self.assertAlmostEqual(c_r, h_r)

    def test_constraints(self):
        self.assertEqual(len(self._constraints), 1)
        console_constraints = self._constraints[0]
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            h5_constraints = h["/methods/NO_METHOD_ID/results/execution:1/best_constraints"]
            self.assertEqual(len(console_constraints), h5_constraints.shape[0])
            for c_c, h_c in zip(console_constraints, h5_constraints[()]):
                self.assertAlmostEqual(c_c, h_c)

    def test_norms(self):
        self.assertEqual(len(self._norms), 1)
        console_norm = self._norms[0]
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            self.assertAlmostEqual(console_norm,
                    h["/methods/NO_METHOD_ID/results/execution:1/best_norm"][()])

    def test_parameter_confidence_intervals(self):
        self.assertEqual(len(self._cis), 1)
        console_ci = self._cis[0]
        expected_descriptors = set(console_ci.keys())
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            h5_ci = h["/methods/NO_METHOD_ID/results/execution:1/confidence_intervals"]
            self.assertEqual(expected_descriptors, set(hce.h5py_strings(h5_ci.dims[0][0])))
            self.assertEqual(len(console_ci), h5_ci.shape[0])
            for i, d in enumerate(hce.h5py_strings(h5_ci.dims[0][0])):
                for j in range(2):
                    self.assertAlmostEqual(console_ci[d][j], h5_ci[i,j])

class EvaluationsStructure(unittest.TestCase):

    def test_interface_presence(self):
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            h["/interfaces/NO_ID/NO_MODEL_ID"]

    def test_interface_members(self):
        expected = {"variables", "responses", "properties"}
        with h5py.File(_TEST_NAME + ".h5", "r") as f:
            h = f["/interfaces/NO_ID/NO_MODEL_ID"]
            actual = {item for item in h}
        self.assertEqual(expected, actual)

    def test_model_presence(self):
        expected_model_types = ["simulation"] #, "recast"]
        expected_sim_models = ["NO_MODEL_ID"]
        #expected_recast_models = ["RECAST_NO_MODEL_ID_DATA_TRANSFORM"]
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            model_types = [k for k in h["/models"]]
            self.assertListEqual(expected_model_types, model_types)
            sim_models = [k for k in h["/models/simulation"]]
            self.assertListEqual(expected_sim_models, sim_models)
            #recast_models = [k for k in h["/models/recast"]]
            #self.assertItemsEqual(expected_recast_models, recast_models)

    def test_model_members(self):
        expected = {"variables", "responses", "properties", "sources"}
        with h5py.File(_TEST_NAME + ".h5", "r") as f:
            h = f["/models/simulation/NO_MODEL_ID"]
            actual = {item for item in h}
        self.assertEqual(expected, actual)

    def test_sources(self):
        with h5py.File(_TEST_NAME + ".h5", "r") as h:
            # Methods
            method_sources = [k for k in h["/methods/NO_METHOD_ID/sources/"]]
            #self.assertItemsEqual(method_sources,["RECAST_NO_MODEL_ID_DATA_TRANSFORM"])
            self.assertListEqual(method_sources,["NO_MODEL_ID"])
            # Models
            #model_sources = [k for k in h["/models/recast/RECAST_NO_MODEL_ID_DATA_TRANSFORM/sources/"]]
            #self.assertItemsEqual(model_sources,["NO_MODEL_ID"])
            model_sources = [k for k in h["/models/simulation/NO_MODEL_ID/sources/"]]
            self.assertListEqual(model_sources,["NO_ID"])

    def test_gradient_presence(self):
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            h["/models/simulation/NO_MODEL_ID/responses/gradients"]

class TabularData(unittest.TestCase):
    def test_tabular_data(self):
        tdata = hce.read_tabular_data(_TEST_NAME + ".dat")
        metadata = ["%eval_id", "interface"]
        variables = ["x1", "x2", "Y"]
        responses = ['displacement01', 'displacement02', 'g']        
        descriptors = metadata + variables + responses
        self.assertListEqual(descriptors, list(tdata.keys()))

        with h5py.File(_TEST_NAME + ".h5", "r") as h:
            # Variables
            hvars = h["/models/simulation/NO_MODEL_ID/variables/continuous"]
            hasv = h["/models/simulation/NO_MODEL_ID/properties/active_set_vector"]
            self.assertListEqual(variables, hce.h5py_strings(hvars.dims[1][0]))
            hindex = 0
            for i, eid in enumerate(tdata["%eval_id"]):
                while not all((a & 1 for a in hasv[hindex,:])):
                    hindex += 1
                for vi, v in enumerate(variables):
                    tv = tdata[v][i]
                    hv = hvars[hindex,vi]
                    self.assertAlmostEqual(tv, hv, msg="Bad comparison for variable '%s' for eval %d" % (v,eid), places=9)
                hindex += 1
                    
            # Responses
            hresps = h["/models/simulation/NO_MODEL_ID/responses/functions"]
            self.assertListEqual(responses, hce.h5py_strings(hresps.dims[1][0]))
            hindex = 0
            for i, eid in enumerate(tdata["%eval_id"]):
                while not all((a & 1 for a in hasv[hindex,:])):
                    hindex += 1
                for ri, r in enumerate(responses):
                    tr = tdata[r][i]
                    hr = hresps[hindex,ri]
                    self.assertAlmostEqual(tr, hr, msg="Bad comparison for response '%s' for eval %d" % (r, eid), places=9)
                hindex += 1
 
class RestartData(unittest.TestCase):
    def test_restart_data(self):
        rdata = hce.read_restart_file(_TEST_NAME + ".rst")
        variables = ["x1", "x2", "Y"]
        responses = ['displacement01', 'displacement02', 'g']        
        self.assertListEqual(variables, list(rdata["variables"]["continuous"].keys()))
        self.assertListEqual(responses, list(rdata["response"].keys()))
        
        with h5py.File(_TEST_NAME + ".h5", "r") as h:
            # Variables
            hvars = h["/interfaces/NO_ID/NO_MODEL_ID/variables/continuous"]
            self.assertListEqual(variables, hce.h5py_strings(hvars.dims[1][0]))
            for i, v in enumerate(variables):
                for eid, tv, hv in zip(rdata["eval_id"], rdata["variables"]["continuous"][v], hvars[:,i]):
                    self.assertAlmostEqual(tv, hv, msg="Bad comparison for variable '%s' for eval %d" % (v,eid), places=9)
            hresps = h["/interfaces/NO_ID/NO_MODEL_ID/responses/functions"]
            # Responses
            self.assertListEqual(responses, hce.h5py_strings(hresps.dims[1][0]))
            for i, r in enumerate(responses):
                for eid, tr, hr in zip(rdata["eval_id"], rdata["response"][r], hresps[:,i]):
                    self.assertAlmostEqual(tr["function"], hr, msg="Bad comparison for response '%s' for eval %d" % (r, eid), places=9)
            # ASV
            for r_asv, h_asv in zip(rdata["asv"], h["/interfaces/NO_ID/NO_MODEL_ID/properties/active_set_vector"]):
                for r_a, h_a in zip(r_asv, h_asv):
                    self.assertEqual(r_a, h_a)


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
