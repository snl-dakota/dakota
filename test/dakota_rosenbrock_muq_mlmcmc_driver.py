#!/usr/bin/env python3
import dakota.interfacing as di
import math

params, results = di.read_parameters_file()
print("python: level = ", params['level'])

x1 = params['x1']
x2 = params['x2']

f1, f2 = 0., 0.
# this is inspired by lf_rosenbrock.cpp
# we mimic having two approximations
if params['level'] == 0:
    f1 = x2 - x1*x1 + 0.001
    f2 = 0.999 - x1
elif params['level'] == 1:
    f1 = x2 - x1*x1
    f2 = 1. - x1

results['f1'].function = 10.*f1
results['f2'].function = f2
results.write()
