#!/usr/bin/env python3

import dakota.interfacing as di

p, r = di.read_parameters_file()

x1 = p["x1"]

if x1 == 0.0:
    r["f"].function = float("-Inf")
elif x1 == 1.0:
    r["f"].function = 0.0
else:
    r["f"].function = float("NaN")

r["f"].gradient = [float("Inf"), 0.0]
r["f"].hessian = [
    [1.0, float("NaN")],
    [float("NaN"), float("Inf")]
    ]
r.metadata["seconds"] = float("NaN")

r.write(json=True)

