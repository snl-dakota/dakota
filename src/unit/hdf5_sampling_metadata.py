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


_TEST_NAME = "sampling_metadata"


class RestartData(unittest.TestCase):
    def test_metadata(self):
        rdata = hce.read_restart_file(_TEST_NAME + ".rst")
        r_metadata = rdata["metadata"]
        r_metadata_labels = [label for label, data in r_metadata.items()]
        expected_metadata_labels = ["sum", "difference"]
        self.assertSequenceEqual(r_metadata_labels, expected_metadata_labels)

        r_metadata_results = [r for r in zip(r_metadata['sum'], r_metadata['difference'])]

        with h5py.File(_TEST_NAME + ".h5", "r") as h:
            h_intf_metadata = h["/interfaces/NO_ID/NO_MODEL_ID/metadata"]
            h_model_metadata = h["/models/simulation/NO_MODEL_ID/metadata"]
            
            self.assertSequenceEqual(hce.h5py_strings(h_intf_metadata.dims[1][0]), r_metadata_labels)
            self.assertSequenceEqual(hce.h5py_strings(h_model_metadata.dims[1][0]), r_metadata_labels)
            
            for intf, model, restart in zip(h_intf_metadata, h_model_metadata, r_metadata_results):
                self.assertAlmostEqual(intf[0], restart[0])
                self.assertAlmostEqual(intf[1], restart[1])
                self.assertAlmostEqual(model[0], restart[0])
                self.assertAlmostEqual(model[1], restart[1])


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
