#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2023
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

import numpy as np

##########################################################.##
#       Demo Surrogate --> Algorithmic callback             #
#############################################################

coeffs = None

def construct(var, resp):

    print("HERE... python surrogate construct:")

    global coeffs
    var2 = np.hstack((np.ones((var.shape[0], 1)), var))
    coeffs = np.zeros(var.shape[1])
    z = np.linalg.inv(np.dot(var2.T, var2))
    coeffs = np.dot(z, np.dot(var2.T, resp))
    return


def predict(pts):

    global coeffs
    return coeffs[0]+pts.dot(coeffs[1:])


def gradient(pts):

    global coeffs
    grad = coeffs[1:]
    return grad.T


# Simple test driver
if __name__ == "__main__":

    print("Hello from surrogates test module.")
