#!/usr/bin/env python

# Dakota will execute this script as
#   rosenbrock_bb_di.py params.in results.out
#  The command line arguments will be extracted by dakota.interfacing automatically.

# necessary python modules
import dakota.interfacing as di

# ----------------------------
# Parse Dakota parameters file
# ----------------------------

params, results = di.read_parameters_file()

# -------------------------------
# Convert and send to application
# -------------------------------

# set up the data structures the rosenbrock analysis code expects
# for this simple example, put all the variables into a single hardwired array
# The asv has to be mapped back into an integer
continuous_vars = [ params['x1'], params['x2'] ]
active_set_vector = 0
if results["obj_fn"].asv.function:
    active_set_vector += 1
if results["obj_fn"].asv.gradient:
    active_set_vector += 2
if results["obj_fn"].asv.hessian:
    active_set_vector += 4

# Alternatively, the ASV can be accessed by index in
# function, gradient, hessian order
#for i, bit in enumerate(results["obj_fn"].asv):
#    if bit:
#        active_set_vector += 1 << i

# set a dictionary for passing to rosenbrock via Python kwargs
rosen_params = {}
rosen_params['cv'] = continuous_vars
rosen_params['asv'] = [active_set_vector]
rosen_params['functions'] = 1

# execute the rosenbrock analysis as a separate Python module
print "Running rosenbrock..."
from rosenbrock import rosenbrock_list
rosen_results = rosenbrock_list(**rosen_params)
print "Rosenbrock complete."


# ----------------------------
# Return the results to Dakota
# ----------------------------

# Insert functions from rosen into results
# Results iterator provides an index, response name, and response
for i, n, r in results:
    if r.asv.function:
        r.function = rosen_results['fns'][i]
    if r.asv.gradient:
        r.gradient = rosen_results['fnGrads'][i]
    if r.asv.hessian:
        r.hessian = rosen_results['fnHessians'][i]

results.write()
