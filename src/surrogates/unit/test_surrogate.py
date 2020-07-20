import dakmod
import matplotlib.pyplot as plt
import numpy as np

def simple_func(x):
    return x[0]*x[0] + 2.0*x[0]*x[1] + 3*x[1]

build_samples = -1.0 + 2*np.random.random((10,2))

build_response = np.zeros(10)
for i, x in enumerate(build_samples):
    build_response[i] = simple_func(x)

pr = dakmod.PolynomialRegression(build_samples, build_response, {})

eval_samples = -1.0 + 2*np.random.random((10,2))
eval_truth = np.zeros(10)
for i, x in enumerate(eval_samples):
    eval_truth[i] = simple_func(x)

eval_surr = np.zeros(10)
pr.value(eval_samples, eval_surr)

print("Truth:\n{0}".format(eval_truth))
print("Surrogate:\n{0}".format(eval_surr))


pypr = dakmod.PyPolyReg(build_samples, build_response, {"max degree" : 2})
eval_pypr = pypr.value(eval_samples)

print("Truth:\n{0}".format(eval_truth))
print("Surrogate PyPR:\n{0}".format(eval_pypr))
