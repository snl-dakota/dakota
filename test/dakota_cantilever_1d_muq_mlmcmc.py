#!/usr/bin/env python3
import dakota.interfacing as di
import math

params, results = di.read_parameters_file()
print("python params level = ", params['level'])
assert params['level'] <= 2

L, wx, wy, Fx, Fy, E = params["L"], params["wx"], params["wy"], params["Fx"], params["Fy"], params["E"]
term1 = Fx / (wx * wx)
term2 = Fy / (wy * wy)
prediction = (4. * L * L * L / (E * wx * wy)) * math.sqrt( term1 * term1 + term2 * term2 )

# mimic a case where we have different approximations levels of the truth
if params['level'] == 0:
    prediction += 0.00001
elif params['level'] == 1:
    prediction += 0.0

if results[0].asv.function:
    results[0].function = prediction
results.write()
