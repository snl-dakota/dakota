#!/usr/bin/env python3
import sys
from typing import Tuple


def driver(x1: float, x2:float) -> Tuple[float, float, float]:
    return x1 * x2, x1 + x2, x1 - x2


def get_variables(params):
    with open(params, "r") as f:
        r = f.readlines()
    x1 = float(r[1].split()[0])
    x2 = float(r[2].split()[0])
    return x1, x2


def write_results(results_file, *args):
    with open(results_file, "w") as f:
        for a in args:
            f.write(f"{a}\n")
            

if __name__ == '__main__':
    params = sys.argv[1]
    results = sys.argv[2]
    x1, x2 = get_variables(params)
    vproduct, vsum, vdifference = driver(x1, x2)
    write_results(results, vproduct, vsum, vdifference)