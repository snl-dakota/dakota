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

def extract_pdfs():
    """Extract the PDFs from the global __OUTPUT

    Returns: The PDFs with lower and upper bins structured
        as a list of dictionaries. The items in the list
        are for executions, and the key, value pairs in the
        dictionaries are the response descriptors and 2D lists
        of the lower and upper bounds and the densities
        with dimension (num_bins, 3)
    """
    global __OUTPUT
    pdfs = []
    lines_iter = iter(__OUTPUT)
    for line in lines_iter:
        if line.startswith("Probability Density Function (PDF)"):
            pdfs.append({})
            nline = next(lines_iter) # get either 'PDF for <descriptor>:" or a blank line
            while nline != '':  # Loop over the responses
                desc = nline.split()[-1][:-1]
                pdf_for_resp = []
                next(lines_iter)  # Skip heading "Bin Lower..."
                next(lines_iter)  # Skip heading "----..."
                nline = next(lines_iter) # Get the first line of data for this response
                while not nline.startswith("PDF for") and nline != '':  # loop over data
                    values = [float(t) for t in nline.split()]
                    pdf_for_resp.append(values)
                    nline = next(lines_iter)
                pdfs[-1][desc] = pdf_for_resp
    return pdfs

def extract_level_mapping_row(line):
    """Tokenize one row of a level mappings table.

    line: String containing a row

    returns: A list of length 4 containing the table entries. 
        Blank table entries contain None.
    """
    tokens = line.split(None, 1)
    tlen = len(tokens[0])
    precision = tlen - 7 if tokens[0] is '-' else tlen - 6
    width = precision + 7

    result = 4*[None]
    for i in range(0, 4):
        begin = i*(width + 2)
        try:
            result[i] = float(line[begin:begin + width+2])
        except (IndexError, ValueError):
            pass
    return result

def extract_level_mappings():
    """Extract the level mappings from the global __OUTPUT

    Returns: The level mappings are structured as a list of 
        dictionaries. The items in the list are for executions, 
        and the key, value pairs in the dictionaries are the 
        response descriptors and 2D lists of the mappings. The
        lists are (num_rows, 4), with None stored in empty
        elements.
    """

    global __OUTPUT
    mappings = []
    lines_iter = iter(__OUTPUT)
    for line in lines_iter:
        if line.startswith("Level mappings for each"):
            mappings.append({})
            nline = next(lines_iter) # get either 'PDF for <descriptor>:" or a blank line
            while not nline.startswith('--'):  # Loop over the responses
                desc = nline.split()[-1][:-1]
                mappings_for_resp = []
                next(lines_iter)  # Skip heading "Response Level.."
                next(lines_iter)  # Skip heading "----..."
                nline = next(lines_iter) # Get the first line of data for this response
                while not nline.startswith("Cumulative Distribution") and not nline.startswith('--'):  # loop over data
                    values = extract_level_mapping_row(nline)
                    mappings_for_resp.append(values)
                    nline = next(lines_iter)
                mappings[-1][desc] = mappings_for_resp
    return mappings


def run_dakota(input_file):
    """Run Dakota on the input_file

    input_file: string containing a path to the input file

    returns: Dakota stdout as a list of strings with newlines
        stripped.
    """
    output = subprocess.check_output(["../dakota",input_file], stderr=subprocess.STDOUT)
    output = output.split('\n')
    return output


## Capture the output here, once.
__OUTPUT = run_dakota("dakota_hdf5_test.in")


class Moments(unittest.TestCase):

    def setUp(self):
        # This method will be called once for each test, so cache the moments
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

class PDFs(unittest.TestCase):
    def setUp(self):
        try:
            self._pdfs
        except AttributeError:
            self._pdfs = extract_pdfs()

    def test_pdfs(self):
        console_pdfs = self._pdfs
        expected_scale_labels = set(('lower_bounds','upper_bounds'))
        expected_descriptors = set(console_pdfs[0].keys())
        expected_num_execs = len(console_pdfs)

        with h5py.File("for_h5py.h5","r") as h:
            # pdfs and scales
            for i in range(expected_num_execs):
                for r in expected_descriptors:
                    hdf5_pdf = h["/methods/aleatory/execution:%d/probability_density/%s" % (i+1, r)]
                    console_pdf = console_pdfs[i][r]
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
            self._mappings = extract_level_mappings()

    def test_level_mappings(self):
        console_mappings = self._mappings
        expected_descriptors = set(console_mappings[0].keys())
        expected_num_execs = len(console_mappings)

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
                
        with h5py.File("for_h5py.h5","r") as h:
            for i in range(expected_num_execs):
                for r in expected_descriptors:
                    # Create a set of the results present in the HDF5 file for convenience
                    has_result = set()
                    for result_type in ('response_levels', 'probability_levels', 
                            'reliability_levels', 'gen_reliability_levels'):
                        try:
                            h["/methods/aleatory/execution:%d/%s/%s" % (i+1, result_type, r)]
                            has_result.add(result_type)
                        except (KeyError, ValueError):
                            pass
                    # Verify the dimension scale label of the datasets
                    for label in has_result:
                        self.assertEqual(label,
                            h["/methods/aleatory/execution:%d/%s/%s" % (i+1, label, r)].dims[0].keys()[0])

                    # compare the total number of results in the console and hdf5 file
                    num_console_rows = len(console_mappings[i][r])
                    num_hdf5_rows = 0
                    for result_type in has_result:
                        num_hdf5_rows += len(h["/methods/aleatory/execution:%d/%s/%s" % (i+1, result_type, r)])
                    self.assertEqual(num_console_rows, num_hdf5_rows)
                    # Verify any response_levels
                    num_response_levels = 0
                    if 'response_levels' in has_result:
                        # Determine the 'compute'd column for the response_levels
                        for j, c in enumerate(console_mappings[i][r][0][1:]):
                            if c:
                                console_ri = j+1
                                break
                        # Loop over the response_levels and verify the user request and computed values
                        response_ds = h["/methods/aleatory/execution:%d/response_levels/%s" % (i+1, r)]
                        num_response_levels = len(response_ds)
                        for j in range(num_response_levels):
                            self.assertAlmostEqual(console_mappings[i][r][j][console_ri], response_ds[j])
                            self.assertEqual(console_mappings[i][r][j][0], response_ds.dims[0][0][j])
                        # Remove response_levels from the results because it has been processed
                        has_result.remove('response_levels')
                    # Verify any prob., rel., and gen. rel. levels
                    begin = num_response_levels
                    for j, result_type in enumerate(('probability_levels', 'reliability_levels', 'gen_reliability_levels')):
                        if result_type in has_result:
                            response_ds =  h["/methods/aleatory/execution:%d/%s/%s" % (i+1, result_type, r)]
                            num_hdf5_rows = len(response_ds)
                            end = begin+num_hdf5_rows
                            for cr, hr, s in zip(console_mappings[i][r][begin:end], response_ds, response_ds.dims[0][0]):
                                # Results
                                self.assertAlmostEqual(cr[0], hr)
                                # Scales
                                self.assertEqual(cr[j+1], s)
                            begin += num_hdf5_rows




                        




                        





if __name__ == '__main__':
    unittest.main()

