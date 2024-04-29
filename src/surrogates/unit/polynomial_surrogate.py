#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2024
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

import numpy as np

#############################################################
#                   Demo Python Surrogate                   #
#############################################################

class Surrogate:

    def __init__(self, params=None):

        print("python Surrogate class constructer...")
        self.coeffs = None
        if params is not None:
            self.params = params


    def construct(self, var, resp):

        var2 = np.hstack((np.ones((var.shape[0], 1)), var))
        self.coeffs = np.zeros(var.shape[1])
        z = np.linalg.inv(np.dot(var2.T, var2))
        self.coeffs = np.dot(z, np.dot(var2.T, resp))

        return


    def predict(self, pts):

        return self.coeffs[0]+pts.dot(self.coeffs[1:])


    def gradient(self, pts):

        grad = self.coeffs[1:]
        return grad.T


# Simple test driver
if __name__ == "__main__":

    print("Hello from surrogates test module.")
