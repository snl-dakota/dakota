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

_TEST_NAME = "discrete_vps"

class ResultsStructure(unittest.TestCase):
    def test_structure(self):
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            exec_group = h["/methods/NO_METHOD_ID/results/execution:1/parameter_sets"]
            contents = set(exec_group.keys())
            self.assertEqual(len(contents), 5) # correct number of groups
            self.assertTrue("continuous_variables" in contents)
            self.assertTrue("discrete_integer_variables" in contents)
            self.assertTrue("discrete_string_variables" in contents)
            self.assertTrue("discrete_real_variables" in contents)
            self.assertTrue("responses" in contents)



class Evaluations(unittest.TestCase):
    def setUp(self):
        try:
            self._variables
        except:
            self._tdata = hce.read_tabular_data(_TEST_NAME + ".dat")
            self._metadata = ["%eval_id", "interface"]
            self._variables = ["cdv_1",  #0 
                    "cdv_2",       #1
                    "ddsiv_1",     #2
                    "ddsiv_2",     #3
                    "ddssv_1",     #4
                    "ddssv_2",     #5
                    "ddsrv_1",     #6 
                    "ddsrv_2",     #7
                    "uuv_1",       #8
                    "uuv_2",       #9 
                    "dusiv_1",     #10
                    "dusiv_2",     #11
                    "dussv_1",     #12
                    "dussv_2",     #13
                    "dusrv_1",     #14 
                    "dusrv_2"]     #15
            self._responses = ["response_fn_1"]
            self._c_vars = self._variables[:2] + self._variables[8:10]
            self._di_vars = self._variables[2:4] + self._variables[10:12]
            self._ds_vars = self._variables[4:6] + self._variables[12:14]
            self._dr_vars = self._variables[6:8] + self._variables[14:]

            self._c_var_types = 2*["CONTINUOUS_DESIGN"] + \
                                2*["UNIFORM_UNCERTAIN"]
            self._di_var_types = 2*["DISCRETE_DESIGN_SET_INT"] + \
                                 2*["DISCRETE_UNCERTAIN_SET_INT"]
            self._ds_var_types = 2*["DISCRETE_DESIGN_SET_STRING"] + \
                                  2*["DISCRETE_UNCERTAIN_SET_STRING"]
            self._dr_var_types = 2*["DISCRETE_DESIGN_SET_REAL"] + \
                                  2*["DISCRETE_UNCERTAIN_SET_REAL"]

            self._c_var_ids = [1, 2, 9, 10]
            self._di_var_ids = [3, 4, 11, 12]
            self._ds_var_ids = [5, 6, 13, 14]
            self._dr_var_ids = [7, 8, 15, 16]

    def test_tabular_data(self):
        descriptors = self._metadata + self._variables + self._responses
        self.assertListEqual(descriptors, list(self._tdata.keys()))
        for ii in list(self._tdata.values()):
            self.assertEqual(len(ii), 3)

    def test_parameter_sets(self):
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            results = h["/methods/NO_METHOD_ID/results/execution:1/parameter_sets"]
        # responses
            for ri, rn in enumerate(self._responses):
                for t, h in zip(self._tdata[rn], results["responses"][:,ri]):
                    self.assertAlmostEqual(t,h, 
                            msg="parameter_sets doesn't match tabular data for response %s" % rn)
            # continuous variables
            for vi, vn in enumerate(self._c_vars):
                for t, h in zip(self._tdata[vn], results["continuous_variables"][:,vi]):
                    self.assertAlmostEqual(t,h, 
                            msg="parameter_sets doesn't match tabular data for variable %s" % vn)

            # discrete integer variables
            for vi, vn in enumerate(self._di_vars):
                for t, h in zip(self._tdata[vn], results["discrete_integer_variables"][:,vi]):
                    self.assertAlmostEqual(t,h, 
                            msg="parameter_sets doesn't match tabular data for variable %s" % vn)

            # discrete string variables
            for vi, vn in enumerate(self._ds_vars):
                for t, h in zip(self._tdata[vn], hce.numpy_strings(results["discrete_string_variables"][:,vi])):
                    self.assertEqual(t, h,
                                     msg="parameter_sets doesn't match tabular data for variable %s" % vn)

                           
           
            # discrete real variables
            for vi, vn in enumerate(self._dr_vars):
                for t, h in zip(self._tdata[vn], results["discrete_real_variables"][:,vi]):
                    self.assertAlmostEqual(t,h, 
                            msg="parameter_sets doesn't match tabular data for variable %s" % vn)

    def test_variable_labels(self):
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            var_group = h["/models/simulation/NO_MODEL_ID/variables/"]
            for var_space, var_labels in zip( 
                    ("continuous", "discrete_integer", "discrete_string", "discrete_real"),
                    (self._c_vars, self._di_vars, self._ds_vars, self._dr_vars)):
                self.assertListEqual(var_labels, hce.h5py_strings(var_group[var_space].dims[1][0]),
                            msg="Descriptors for variables of space %s are not correct." % var_space)
    
    def test_variable_ids(self):
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            var_group = h["/models/simulation/NO_MODEL_ID/variables/"]
            for var_space, var_ids in zip( 
                    ("continuous", "discrete_integer", "discrete_string", "discrete_real"),
                    (self._c_var_ids, self._di_var_ids, self._ds_var_ids, self._dr_var_ids)):
                for hl, el in zip(var_group[var_space].dims[1][1], var_ids):
                    self.assertEqual(hl, el, 
                            msg="Ids for variables of space %s are not correct." % var_space)
    
    def test_variable_types(self):
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            var_group = h["/models/simulation/NO_MODEL_ID/variables/"]
            for var_space, var_types in zip( 
                    ("continuous", "discrete_integer", "discrete_string", "discrete_real"),
                    (self._c_var_types, self._di_var_types, self._ds_var_types, self._dr_var_types)):
                self.assertListEqual(var_types, hce.h5py_strings(var_group[var_space].dims[1][2]),
                            msg="Types for variables of space %s are not correct." % var_space)




class EvaluationsStructure(unittest.TestCase):

    def test_interface_presence(self):
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            h["/interfaces/NO_ID/NO_MODEL_ID"]

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
            #self.assertListEqual(expected_recast_models, recast_models)

    def test_sources(self):
        with h5py.File(_TEST_NAME + ".h5", "r") as h:
            # Methods
            method_sources = [k for k in h["/methods/NO_METHOD_ID/sources/"]]
            #self.assertListEqual(method_sources,["RECAST_NO_MODEL_ID_DATA_TRANSFORM"])
            self.assertListEqual(method_sources,["NO_MODEL_ID"])
            # Models
            #model_sources = [k for k in h["/models/recast/RECAST_NO_MODEL_ID_DATA_TRANSFORM/sources/"]]
            #self.assertListEqual(model_sources,["NO_MODEL_ID"])
            model_sources = [k for k in h["/models/simulation/NO_MODEL_ID/sources/"]]
            self.assertListEqual(model_sources,["NO_ID"])

    def test_variables_presence(self):
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            model_group = h["/models/simulation/NO_MODEL_ID/"]
            contents = set(model_group.keys())
            self.assertTrue("variables" in contents)
            self.assertTrue("responses" in contents)
            self.assertTrue("properties" in contents)
            self.assertTrue("sources" in contents)

            variables_group = model_group["variables"]
            contents = set(variables_group.keys())
            self.assertTrue("continuous" in contents)
            self.assertTrue("discrete_integer" in contents)
            self.assertTrue("discrete_string" in contents)
            self.assertTrue("discrete_real" in contents)

            interface_group = h["/interfaces/NO_ID/NO_MODEL_ID/"]
            contents = set(interface_group.keys())
            self.assertTrue("variables" in contents)
            self.assertTrue("responses" in contents)
            self.assertTrue("properties" in contents)

            variables_group = interface_group["variables"]
            contents = set(variables_group.keys())
            self.assertTrue("continuous" in contents)
            self.assertTrue("discrete_integer" in contents)
            self.assertTrue("discrete_string" in contents)
            self.assertTrue("discrete_real" in contents)


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

