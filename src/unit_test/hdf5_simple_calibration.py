#!/usr/bin/env python
import unittest
import h5py
import h5py_console_extract as hce

_TEST_NAME = "simple_calibration"

class Structure(unittest.TestCase):
    def test_structure(self):
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            exec_group = h["/methods/NO_METHOD_ID/execution:1/"]
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
            params = h["/methods/NO_METHOD_ID/execution:1/best_parameters/continuous"]
            # Descriptors match
            h5_descriptors = set(params.dims[0][0][:])
            self.assertEqual(expected_descriptors, h5_descriptors)
            ## Verify the values
            for (label, c_val), h5_val in zip(console_params, params.value):
                self.assertAlmostEqual(c_val, h5_val)

    def test_residuals(self):
        self.assertEqual(len(self._residuals), 1)
        console_residuals = self._residuals[0]
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            h5_residuals = h["/methods/NO_METHOD_ID/execution:1/best_residuals"]
            self.assertEqual(len(console_residuals), h5_residuals.shape[0])
            for c_r, h_r in zip(console_residuals, h5_residuals.value):
                self.assertAlmostEqual(c_r, h_r)

    def test_constraints(self):
        self.assertEqual(len(self._constraints), 1)
        console_constraints = self._constraints[0]
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            h5_constraints = h["/methods/NO_METHOD_ID/execution:1/best_constraints"]
            self.assertEqual(len(console_constraints), h5_constraints.shape[0])
            for c_c, h_c in zip(console_constraints, h5_constraints.value):
                self.assertAlmostEqual(c_c, h_c)

    def test_norms(self):
        self.assertEqual(len(self._norms), 1)
        console_norm = self._norms[0]
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            self.assertAlmostEqual(console_norm,
                    h["/methods/NO_METHOD_ID/execution:1/best_norm"].value)

    def test_parameter_confidence_intervals(self):
        self.assertEqual(len(self._cis), 1)
        console_ci = self._cis[0]
        expected_descriptors = set(console_ci.keys())
        with h5py.File(_TEST_NAME + ".h5","r") as h:
            h5_ci = h["/methods/NO_METHOD_ID/execution:1/confidence_intervals"]
            self.assertEqual(expected_descriptors, set(h5_ci.dims[0][0][:]))
            self.assertEqual(len(console_ci), h5_ci.shape[0])
            for i, d in enumerate(h5_ci.dims[0][0]):
                for j in range(2):
                    self.assertAlmostEqual(console_ci[d][j], h5_ci[i,j])






if __name__ == '__main__':
    hce.run_dakota("dakota_hdf5_" + _TEST_NAME + ".in")
    unittest.main()

