#!/usr/bin/env python
# -*- coding: utf-8 -*-
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
import math
import h5py
import h5py_console_extract as hce

_TEST_NAME = "dist_params"


# The baseline contains the expected values of the parameters. It
# was initially generated automatically from Dakota output, which
# is cheating, but I was careful to verify that all the values were
# as expected.
# There are a few more interesting cases. One is lognormal_uncetain
# variables. For these, means and standard deviations are specified
# in the input. Dakota computes lambdas, zetas, and error factors.
# Using the formulae in the Reference manual, I computed them myself
# to verify that the values being written are correct.
# For the "empirical" type distributions like discrete_uncertain_set
# and histogram_bin_uncertain variables, Dakota normalizes the
# probabilities, and these computed values are written, not the user
# input. Again, for these cases, I confirmed that the normalized values
# were consistent with the user input.

baseline = {
#    "beta_uncertain": {
#        "alpha": [
#            0.6,
#            1.2
#        ],
#        "beta": [
#            0.1,
#            0.2
#        ],
#        "lower_bound": [
#            0.0,
#            0.1
#        ],
#        "upper_bound": [
#            4.0,
#            5.0
#        ]
#    },
    "binomial_uncertain": {
        "num_trials": [
            3,
            6
        ],
        "probability_per_trial": [
            0.17,
            0.34
        ]
    },
    "continuous_design": {
        "lower_bound": [
            -4.0,
            -3.9
        ],
        "upper_bound": [
            4.0,
            3.9
        ]
    },
    "continuous_interval_uncertain": {
        "interval_probabilities": [
            [
                0.4,
                0.6,
                float('nan')
            ],
            [
                0.3,
                0.5,
                0.2
            ]
        ],
        "lower_bounds": [
            [
                1.0,
                1.1,
                float('nan')
            ],
            [
                2.0,
                2.1,
                2.2
            ]
        ],
        "num_elements": [
            2,
            3
        ],
        "upper_bounds": [
            [
                2.0,
                2.1,
                float('nan')
            ],
            [
                3.0,
                3.1,
                3.2
            ]
        ]
    },
    "continuous_state": {
        "lower_bound": [
            -0.6,
            -0.5
        ],
        "upper_bound": [
            0.3,
            0.4
        ]
    },
    "discrete_design_range": {
        "lower_bound": [
            -3,
            -2
        ],
        "upper_bound": [
            3,
            2
        ]
    },
    "discrete_design_set_int": {
        "elements": [
            [
                1,
                2,
                2147483647
            ],
            [
                3,
                4,
                5
            ]
        ],
        "num_elements": [
            2,
            3
        ]
    },
    "discrete_design_set_real": {
        "elements": [
            [
                -2.5,
                -1.5,
                float('nan')
            ],
            [
                0.5,
                1.5,
                2.5
            ]
        ],
        "num_elements": [
            2,
            3
        ]
    },
    "discrete_design_set_string": {
        "elements": [
            [
                "a",
                "b",
                ""
            ],
            [
                "c",
                "d",
                "e"
            ]
        ],
        "num_elements": [
            2,
            3
        ]
    },
    "discrete_interval_uncertain": {
        "interval_probabilities": [
            [
                0.6,
                0.4,
                float('nan')
            ],
            [
                0.2,
                0.3,
                0.5
            ]
        ],
        "lower_bounds": [
            [
                1,
                2,
                2147483647
            ],
            [
                3,
                4,
                5
            ]
        ],
        "num_elements": [
            2,
            3
        ],
        "upper_bounds": [
            [
                3,
                4,
                2147483647
            ],
            [
                5,
                6,
                7
            ]
        ]
    },
    "discrete_state_range": {
        "lower_bound": [
            -1,
            -4
        ],
        "upper_bound": [
            2,
            3
        ]
    },
    "discrete_state_set_int": {
        "elements": [
            [
                3,
                4,
                5,
                2147483647
            ],
            [
                1,
                2,
                3,
                4
            ]
        ],
        "num_elements": [
            3,
            4
        ]
    },
    "discrete_state_set_real": {
        "elements": [
            [
                0.9,
                1.8,
                2.7,
                float('nan')
            ],
            [
                2.1,
                4.2,
                8.4,
                16.8
            ]
        ],
        "num_elements": [
            3,
            4
        ]
    },
    "discrete_state_set_string": {
        "elements": [
            [
                "ab",
                "bb",
                "cb",
                ""
            ],
            [
                "db",
                "eb",
                "fb",
                "gb"
            ]
        ],
        "num_elements": [
            3,
            4
        ]
    },
    "discrete_uncertain_set_int": {
        "elements": [
            [
                2,
                3,
                4,
                2147483647
            ],
            [
                3,
                4,
                5,
                6
            ]
        ],
        "num_elements": [
            3,
            4
        ],
        "set_probabilities": [
            [
                0.1,
                0.7,
                0.2,
                float('nan')
            ],
            [
                0.35,
                0.3,
                0.25,
                0.1
            ]
        ]
    },
    "discrete_uncertain_set_real": {
        "elements": [
            [
                0.9,
                1.8,
                2.7,
                float('nan')
            ],
            [
                2.1,
                4.2,
                8.4,
                16.8
            ]
        ],
        "num_elements": [
            3,
            4
        ],
        "set_probabilities": [
            [
                0.35,
                0.25,
                0.4,
                float('nan')
            ],
            [
                0.1,
                0.2,
                0.3,
                0.4
            ]
        ]
    },
    "exponential_uncertain": {
        "beta": [
            0.75,
            1.25
        ]
    },
    "frechet_uncertain": {
        "alpha": [
            2.1,
            2.2
        ],
        "beta": [
            0.01,
            0.02
        ]
    },
    "gamma_uncertain": {
        "alpha": [
            1.1,
            1.3
        ],
        "beta": [
            0.25,
            0.35
        ]
    },
    "geometric_uncertain": {
        "probability_per_trial": [
            0.21,
            0.42
        ]
    },
    "gumbel_uncertain": {
        "alpha": [
            1.25,
            1.35
        ],
        "beta": [
            0.3,
            0.4
        ]
    },
    "histogram_bin_uncertain": {
        "abscissas": [
            [
                0.0,
                1.0,
                2.0,
                float('nan')
            ],
            [
                4.0,
                5.0,
                6.0,
                7.0
            ]
        ],
        "counts": [
            [
                0.3333333333333333,
                0.6666666666666666,
                0.0,
                float('nan')
            ],
            [
                0.5,
                0.3333333333333333,
                0.16666666666666666,
                0.0
            ]
        ],
        "num_elements": [
            3,
            4
        ]
    },
    "histogram_point_uncertain_int": {
        "abscissas": [
            [
                1,
                2,
                3,
                2147483647
            ],
            [
                2,
                3,
                4,
                5
            ]
        ],
        "counts": [
            [
                0.5,
                0.3333333333333333,
                0.16666666666666666,
                float('nan')
            ],
            [
                0.16666666666666666,
                0.3333333333333333,
                0.3333333333333333,
                0.16666666666666666
            ]
        ],
        "num_elements": [
            3,
            4
        ]
    },
    "histogram_point_uncertain_real": {
        "abscissas": [
            [
                1.5,
                2.0,
                2.5,
                float('nan')
            ],
            [
                1.0,
                1.75,
                2.5,
                3.25
            ]
        ],
        "counts": [
            [
                0.4,
                0.2,
                0.4,
                float('nan')
            ],
            [
                0.4,
                0.1,
                0.1,
                0.4
            ]
        ],
        "num_elements": [
            3,
            4
        ]
    },
    "hypergeometric_uncertain": {
        "num_drawn": [
            1,
            3
        ],
        "selected_population": [
            3,
            5
        ],
        "total_population": [
            8,
            12
        ]
    },
    "lognormal_uncertain": {
        "error_factor": [
            1.4992979627361507,
            1.7681044815401372
        ],
        "lambda": [
            -0.5411379346742083,
            -0.41669896508235027
        ],
        "lower_bound": [
            0.001,
            0.1
        ],
        "mean": [
            0.5999999999999999,
            0.7
        ],
        "std_deviation": [
            0.15,
            0.24999999999999994
        ],
        "upper_bound": [
            100000.0,
            1.5
        ],
        "zeta": [
            0.2462206770692398,
            0.3464794976434184
        ]
    },
    "loguniform_uncertain": {
        "lower_bound": [
            0.5,
            0.0001
        ],
        "upper_bound": [
            1.5,
            1.0
        ]
    },
    "negative_binomial_uncertain": {
        "num_trials": [
            2,
            4
        ],
        "probability_per_trial": [
            0.23,
            0.46
        ]
    },
    "normal_uncertain": {
        "lower_bound": [
            -0.7,
            float('-inf')
        ],
        "mean": [
            -0.4,
            0.4
        ],
        "std_deviation": [
            0.1,
            0.2
        ],
        "upper_bound": [
            float('inf'),
            0.6
        ]
    },
    "poisson_uncertain": {
        "lambda": [
            0.66,
            1.32
        ]
    },
    "triangular_uncertain": {
        "lower_bound": [
            0.1,
            0.2
        ],
        "mode": [
            1.5,
            2.5
        ],
        "upper_bound": [
            2.0,
            3.0
        ]
    },
    "uniform_uncertain": {
        "lower_bound": [
            -2.0,
            -1.5
        ],
        "upper_bound": [
            1.5,
            2.0
        ]
    },
    "weibull_uncertain": {
        "alpha": [
            0.06,
            0.07
        ],
        "beta": [
            0.015,
            0.025
        ]
    }
}

def isposinf(x):
    """Helper that returns whether x is +inf"""
    return math.isinf(x) and x > 0

def isneginf(x):
    """Helper that returns whether x is -inf"""
    return math.isinf(x) and x < 0

class DistributionParameters(unittest.TestCase):

    def element_test_helper(self, b, r, name, field, i=None):
        """Compare baseline to result 

This helper is needed primarily because in Python (and in general)
float('nan') == float('nan') is False, so the vanilla assertEqual
or assertAlmostEqual can't be used.

b is the baseline expectation, r is result, name is the variable
type (e.g. "continuous_design"), field is a the paramter (e.g. "mean"),
and i is an optional index that is provided to enrichen error reporting
for the case that the field is a vector."""

        if i is None:
            message = "Error for %s/%s. Baseline is %%s and result is %%s" %(name, field)
        else:
            message = "Error for %s/%s at %d. Baseline is %%s and result is %%s" %(name, field, i)
        # if either are strings, do a normal assertEqual
        if isinstance(b, str) or isinstance(r, str):
            b = b.decode('utf-8') if isinstance(b, bytes) else b
            r = r.decode('utf-8') if isinstance(r, bytes) else r
            self.assertEqual(b,r, msg=message %(b,r))
        # The next three cases account are special handling of NaNs, +inf, and -inf.
        elif math.isnan(b) or math.isnan(r):
            self.assertTrue(math.isnan(b) and math.isnan(r), msg=message %(b,r))
        elif isposinf(b) or isposinf(r):
            self.assertTrue(isposinf(b) and isposinf(r), msg=message %(b,r))
        elif isneginf(b) or isneginf(r):
            self.assertTrue(isneginf(b) and isneginf(r), msg=message %(b,r))
        # floats. Although r is actually a numpy.float64, for the version of numpy
        # I checked, isintance(r,float) worked. Hopefully that's general..
        elif isinstance(b,float) or isinstance(r,float): 
            self.assertAlmostEqual(b,r, msg=message %(b,r))
        # everything else (should just be ints/np.int32/np.unint32)
        else:
            self.assertEqual(b,r , msg=message %(b,r))

    def test_params(self):
        global baseline
        with h5py.File("dist_params.h5","r") as h:
            # contents of md are datasets named for variable types (continuous_design, etc)
            md = h["models/simulation/NO_MODEL_ID/properties/variable_parameters"]
            for name, data in list(md.items()):
                # iterate the fields (mean, std_deviation)
                for f in data.dtype.names:
                    # iterate the individual variables of type name.
                    for i, v in enumerate(data):
                        # a field could be scalar or vector
                        if v[f].shape:  # vector case
                            for b, r in zip(baseline[name][f][i], v[f]):
                                self.element_test_helper(b, r, name, f, i)
                        else: # scalar case
                            self.element_test_helper(baseline[name][f][i], v[f], name, f)
                del baseline[name] # at the end, the baseline should be empty
        self.assertFalse(baseline,msg="Baseline dictionary is not empty!")


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
