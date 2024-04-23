#!/usr/bin/env python3
import dakota.interfacing as di
import math
 
params, results = di.read_parameters_file()
 
L, wx, wy, Fx, Fy, E = params["L"], params["wx"], params["wy"], params["Fx"], params["Fy"], params["E"]
 
term1 = Fx / (wx * wx)
term2 = Fy / (wy * wy)

if results[0].asv.function:
    results[0].function = (4. * L * L * L / (E * wx * wy)) * math.sqrt( term1 * term1 + term2 * term2 )
if results[0].asv.gradient:
    results[0].gradient = [-(4. * L * L * L / (E * E * wx * wy)) * math.sqrt( term1 * term1 + term2 * term2 )]
results.write()
