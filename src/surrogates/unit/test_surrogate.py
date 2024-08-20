#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2024
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
import dak_surrogates as daksurr

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

# GaussianProcess:
# instantiate a Gaussian process surrogate using a
# Python dictionary for configOptions
nugget_opts = {"estimate nugget" : True}
config_opts = {"scaler name" : "none", "Nugget" : nugget_opts }
gp = daksurr.GaussianProcess(build_samples, build_response, config_opts)

# Set response labels, leaving vars unlabeled
gp.response_labels(["f"])

# evaluate the surrogate's value, variance, gradient, and hessian
gp_eval_surr = gp.value(eval_samples)
print("Truth:\n{0}".format(eval_truth))
print("GP value:\n{0}\n".format(gp_eval_surr))

eval_variance = gp.variance(eval_point)
print("GP variance:\n{0}\n".format(eval_variance))

eval_grad = gp.gradient(eval_point)
print("GP gradient:\n{0}\n".format(eval_grad))

eval_hessian = gp.hessian(eval_point)
print("GP Hessian:\n{0}\n".format(eval_hessian))

obj_fun_values = gp.objective_function_history()
print("GP MLE objective function value history:\n{0}\n".format(obj_fun_values))

obj_grad_values = gp.objective_gradient_history()
print("GP MLE objective gradient history:\n{0}\n".format(obj_grad_values))

theta_values = gp.theta_history()
print("GP MLE hyperparameter (theta) history:\n{0}\n".format(theta_values))

# Save GP to binary and then reload
print("Saving GP")
# Free function for generic save (serialize via shared pointer to base)
daksurr.save(gp, "gp.bin", True)

# load using derived class constructor
print("Loading GP (derived class constructor)")
dctor_gpload = daksurr.GaussianProcess(filename = "gp.bin", binary = True)
print("Loaded GP is a: {0}".format(dctor_gpload.__class__.__name__))
assert(np.allclose(dctor_gpload.value(eval_samples), gp_eval_surr))

assert(len(dctor_gpload.variable_labels()) == 0)
assert(dctor_gpload.response_labels() == ["f"])

# free function load
print("Loading GP (free function)")
gen_gpload = daksurr.load("gp.bin", True)
print("Loaded GP is a: {0}".format(gen_gpload.__class__.__name__))
assert(np.allclose(gen_gpload.value(eval_samples), gp_eval_surr))

# check that serialized MLE history matrices match original values
assert np.allclose(gen_gpload.objective_function_history(), obj_fun_values)
assert np.allclose(gen_gpload.objective_gradient_history(), obj_grad_values)
assert np.allclose(gen_gpload.theta_history(), theta_values)
