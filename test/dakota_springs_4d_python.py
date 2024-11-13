import dakota.interfacing as di
import math
 
@di.python_interface
def decorated_driver(paramsFromDak, results):
    F1, k1, k2, k3, k4, k5, k6, k7, k8, k9, k10 = paramsFromDak["F1"], paramsFromDak["k1"], paramsFromDak["k2"], paramsFromDak["k3"], paramsFromDak["k4"], paramsFromDak["k5"], paramsFromDak["k6"], paramsFromDak["k7"], paramsFromDak["k8"], paramsFromDak["k9"], paramsFromDak["k10"]

    if results[0].asv.function:
        results[0].function = F1 / k1 + F1 / k2 + F1 / k3 + F1 / k4 + F1 / k5 + F1 / k6 + F1 / k7 + F1 / k8 + F1 / k9 + F1 / k10
    if results[0].asv.gradient:
        results[0].gradient = [- F1 / (k1 * k1), -F1 / (k2 * k2), -F1 / (k3 * k3), -F1 / (k4 * k4)]

    return results
