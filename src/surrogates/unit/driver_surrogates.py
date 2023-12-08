#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2023
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
    print(model.summary())
    return


def predict(pts):

    global model
    print(pts)
    pts2 = sm.add_constant(pts)
    return model.predict(pts2)


# Simple test driver
if __name__ == "__main__":

    print("Hello from surrogates test module.")
