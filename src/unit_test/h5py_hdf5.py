from __future__ import print_function
import subprocess
import unittest
import h5py
import numpy as np

# This test must:
# 1. extract various results from console output
# 2. compare them to results in an hdf5 file


def extract_moments():
    """Extract the moments from the global __OUTPUT

    Returns: The moments structured as a list of dictionaries.
        The items in the list are for executions, and the 
        key, value pairs in the dictionary are the response
        descriptors and an numpy array of the moments
    """
    global __OUTPUT
    moments = []
    lines_iter = iter(__OUTPUT)
    for line in lines_iter:
        if line.startswith("Sample moment statistics"):
            next(lines_iter)
            moments.append({})
            moments_line = next(lines_iter)
            while moments_line != '':
                tokens = moments_line.split()
                resp_desc = tokens[0]
                moments_values = np.array([float(t) for t in tokens[1:]])
                moments[-1][resp_desc] = moments_values
                moments_line = next(lines_iter)
    return moments

def run_dakota(input_file):
    """Run Dakota on the input_file

    input_file: string containing a path to the input file

    returns: Dakota stdout as a list of strings with newlines
        stripped.
    """
    output = subprocess.check_output(["../dakota",input_file])
    output = output.split('\n')
    return output


## Capture the output here, once.
__OUTPUT = run_dakota("dakota_hdf5_test.in")


class Moments(unittest.TestCase):

    def setUp(self):
        try:
            self._moments
        except AttributeError:
            self._moments = extract_moments()

    def test_structure(self):
        # Verify the structure (execution id, descriptors)
        console_moments = self._moments
        expected_num_execs = len(console_moments)
        expected_descriptors = set(console_moments[0].keys())
        with h5py.File("for_h5py.h5","r") as h:
            ## Verify the presence of all the execution data
            # execution:N
            self.assertEqual(expected_num_execs, len(list(h["/methods/aleatory/"].keys())))
            for i, n in enumerate(h["/methods/aleatory/"].keys()):
                self.assertEqual("execution:%d" %(i+1,), n)
            # descriptors
            hdf5_descriptors = set(h["/methods/aleatory/execution:1/moments/"].keys())
            hdf5_descriptors.remove('_scales')
            self.assertEqual(expected_descriptors, hdf5_descriptors)

    def test_moments(self):
        # Extract the moments from Dakota console output and compare
        # them to the hdf5 output. Both the moments themselves and
        # the dimension scales are compared.
        console_moments = self._moments
        expected_scale_label = 'moments'
        expected_scale = ['mean', 'std_deviation', 'skewness', 'kurtosis']
        expected_descriptors = set(console_moments[0].keys())
        expected_num_execs = len(console_moments)

        with h5py.File("for_h5py.h5","r") as h:
            # moments and scales
            for i in range(expected_num_execs):
                for r in expected_descriptors:
                    hdf5_moments = h["/methods/aleatory/execution:%d/moments/%s" % (i+1, r)]
                    for j in range(4):
                        self.assertAlmostEqual(console_moments[i][r][j], hdf5_moments[j])
                    scale_label = list(hdf5_moments.dims[0].keys())[0]
                    self.assertEqual(expected_scale_label, scale_label)
                    for es, s in zip(expected_scale, hdf5_moments.dims[0][0]):
                        self.assertEqual(es,s)

if __name__ == '__main__':
    unittest.main()

