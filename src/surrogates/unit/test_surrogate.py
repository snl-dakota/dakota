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


# GaussianProcess:
# instantiate a Gaussian process surrogate using a
# Python dictionary for configOptions
nugget_opts = {"estimate nugget" : True}
config_opts = {"scaler name" : "none", "Nugget" : nugget_opts }
gp = daksurr.GaussianProcess(build_samples, build_response, config_opts)

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

# Save GP to binary and then reload
print("Saving GP")
# Free function for generic save (serialize via shared pointer to base)
daksurr.save(gp, "gp.bin", True)

# load using derived class constructor
print("Loading GP (derived class constructor)")
dctor_gpload = daksurr.GaussianProcess(filename = "gp.bin", binary = True)
print("Loaded GP is a: {0}".format(dctor_gpload.__class__.__name__))
assert(np.allclose(dctor_gpload.value(eval_samples), gp_eval_surr))

# free function load
print("Loading GP (free function)")
gen_gpload = daksurr.load("gp.bin", True)
print("Loaded GP is a: {0}".format(gen_gpload.__class__.__name__))
assert(np.allclose(gen_gpload.value(eval_samples), gp_eval_surr))
