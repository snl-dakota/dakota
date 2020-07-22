import dakmod
#import matplotlib.pyplot as plt
import numpy as np

np.random.seed(44)

def simple_func(x):
    return x[0]*x[0] + 2.0*x[0]*x[1] + 3.0*x[1]
    
build_samples = -1.0 + 2*np.random.random((10,2))

build_response = np.zeros(10)
for i, x in enumerate(build_samples):
    build_response[i] = simple_func(x)

# This doesn't work due to the way NumPy is passed to Eigen (by copy)
# Working now with return value function
pr = dakmod.PolynomialRegression(build_samples, build_response, {"max degree" : 2, "scaler type" : "none"})

eval_samples = -1.0 + 2*np.random.random((10,2))
eval_truth = np.zeros(10)
for i, x in enumerate(eval_samples):
    eval_truth[i] = simple_func(x)

eval_surr = pr.value(eval_samples)
print("Truth:\n{0}".format(eval_truth))
print("Surrogate value:\n{0}\n".format(eval_surr))

# deriv evaluation point
dpoint = np.atleast_2d(np.array([0.1, 0.4]))

eval_grad = pr.gradient(dpoint,0)
print("Surrogate gradient:\n{0}\n".format(eval_grad))

eval_hessian = pr.hessian(dpoint,0)
print("Surrogate Hessian:\n{0}\n".format(eval_hessian))

# This does work, and agrees
pypr = dakmod.PyPolyReg(build_samples, build_response, {"max degree" : 2, "scaler type" : "none"})
eval_pypr = pypr.value(eval_samples)
print("Truth:\n{0}".format(eval_truth))
print("Surrogate:\n{0}".format(eval_pypr))

# Try loading from file
# This reminds that we really need to load into pointer to get the type right
# Is this working correctly? Value output does not match

#pyprload = dakmod.PyPolyReg()
#dakmod.PyPolyReg.load("exported_poly.response_fn_1.txt", False, pyprload)

pyprload = dakmod.PolynomialRegression();
dakmod.load_poly("exported_poly.response_fn_1.txt", False, pyprload)

eval_loaded = pyprload.value(eval_samples)
print("Truth:\n{0}".format(eval_truth))
print("Loaded Surrogate:\n{0}".format(eval_loaded))
