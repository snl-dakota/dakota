#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2025
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

import numpy as np
import statsmodels.api as sm

##########################################################.##
#       Demo Surrogate --> Algorithmic callback             #
#############################################################

model = None

def construct(var, resp):

    global model
    var2 = sm.add_constant(var)
    model = sm.OLS(resp, var2).fit()
    #print(model.summary())
    return


def predict(pts):

    global model
    # This is a workaround for trying to add a constant to a single
    # parameter point which ends up being a no-op
    if pts.shape[0] == 1:
        pad_vals = np.zeros_like(pts)
        pts = np.append(pts, pad_vals, axis=0)
    pts2 = sm.add_constant(pts)
    return model.predict(pts2)


def gradient(pts):

    global model
    # Need to check that pts are the same as were used with predict;
    # else need to update model via predict
    grad = model.params
    return np.array([grad])

# Simple test driver
if __name__ == "__main__":

    print("Hello from surrogates test module.")
