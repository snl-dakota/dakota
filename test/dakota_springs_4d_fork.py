#!/usr/bin/env python3
import dakota.interfacing as di
import math
 
params, results = di.read_parameters_file()
 
F1, k1, k2, k3, k4, k5, k6, k7, k8, k9, k10 = params["F1"], params["k1"], params["k2"], params["k3"], params["k4"], params["k5"], params["k6"], params["k7"], params["k8"], params["k9"], params["k10"]
 
if results[0].asv.function:
    results[0].function = F1 / k1 + F1 / k2 + F1 / k3 + F1 / k4 + F1 / k5 + F1 / k6 + F1 / k7 + F1 / k8 + F1 / k9 + F1 / k10
if results[0].asv.gradient:
    results[0].gradient = [- F1 / (k1 * k1), -F1 / (k2 * k2), -F1 / (k3 * k3), -F1 / (k4 * k4)]
results.write()
