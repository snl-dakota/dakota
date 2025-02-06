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
#                   Demo Python Surrogates                  #
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


    def hessian(self, pts):

        num_vals = pts.shape[1]
        hess = np.zeros(shape=(num_vals, num_vals))
        return hess


def expanded_text_book_numpy(params):

    num_fns = params["functions"]
    x = params["cv"]
    ASV = params["asv"]

    retval = {}

    if (ASV[0] & 1):  # **** f:
        fn = sum([(val - 1.)**4 for val in x])
        retval["fns"] = np.array([fn, 1+x[0], 2+x[1], 3+x[0]*x[1], 4+x[0]*x[0], 5+x[1]*x[1], 1.5+x[0]*x[1]*x[1]])

    # Can add these later as needed ...
    #if (ASV[0] & 2):  # **** df/dx:
    #    g = [4. * (val - 1.)**3 for val in x]
    #    retval["fnGrads"] = np.array([np.array(g)])

    #if (ASV[0] & 4):  # **** d^2f/dx^2:
    #    h = np.diag([12. * (val - 1.)**2 for val in x])
    #    retval["fnHessians"] = np.array([h])

    return retval


def funcs(params):

    return expanded_text_book_numpy(params)


if __name__ == "__main__":

    print("Hello from surrogates test module.")
