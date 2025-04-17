#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2025
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

import numpy as np
import sys

# Optionally append a path to the python library, passed as argv[1]
if len(sys.argv) > 1:
    surr_lib_path = sys.argv[1]
    sys.path.append(surr_lib_path)
import surrogates as daksurr

np.random.seed(44)

# generate training and test data
def simple_func(x):
    return x[0]*x[0] + 2.0*x[0]*x[1] + 3.0*x[1]
    
build_samples = -1.0 + 2*np.random.random((10,2))
build_response = np.zeros(10)
for i, x in enumerate(build_samples):
  build_response[i] = simple_func(x)

eval_samples = -1.0 + 2*np.random.random((10,2))
eval_truth = np.zeros(10)
for i, x in enumerate(eval_samples):
  eval_truth[i] = simple_func(x)

# single evaluation point
eval_point = np.atleast_2d(np.array([0.1, 0.4]))

# PolynomialRegression:
# instantiate a polynomial regression surrogate using a
# Python dictionary for configOptions
pr = daksurr.PolynomialRegression(build_samples,
     build_response, {"max degree" : 2, "scaler type" : "none", "reduced basis" : False})

# Set variable labels, leaving responses unlabeled
pr.variable_labels(["x0", "x1"])

# evaluate the surrogate's value, gradient, and hessian
eval_surr = pr.value(eval_samples)
print("Truth:\n{0}".format(eval_truth))
print("Surrogate value:\n{0}\n".format(eval_surr))

eval_grad = pr.gradient(eval_point)
print("Surrogate gradient:\n{0}\n".format(eval_grad))

eval_hessian = pr.hessian(eval_point)
print("Surrogate Hessian:\n{0}\n".format(eval_hessian))


# save polynomial surrogate to text
print("Saving Polynomial")
# Free function for generic save (serialize via shared pointer to base)
daksurr.save(pr, "poly.txt", False)

# load using derived class constructor
print("Loading Polynomial (derived class constructor)")
dctor_prload = daksurr.PolynomialRegression(filename = "poly.txt", binary = False)
print("Loaded Poly is a: {0}".format(dctor_prload.__class__.__name__))
assert(np.allclose(pr.value(eval_point), dctor_prload.value(eval_point)))

# free function load
print("Loading Polynomial (free function)")
gen_prload = daksurr.load("poly.txt", False)
print("Loaded Poly is a: {0}".format(gen_prload.__class__.__name__))
assert(np.allclose(pr.value(eval_point), gen_prload.value(eval_point)))
assert(gen_prload.variable_labels() == ["x0", "x1"])
assert(len(gen_prload.response_labels()) == 0)
