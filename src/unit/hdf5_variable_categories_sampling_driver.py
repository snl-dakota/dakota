#!/usr/bin/env python3
import sys
from typing import Tuple, List


def driver(x1: List[float], x2: List[float]) -> List[float]:
    y = [0.0] * len(x1)
    for i in range(len(x1)):
        y[i] = x1[i] + 0.5*x2[i]
    return y 


def get_variables(params: str) -> Tuple[float, float]:
    x1 = []
    x2 = []
    with open(params, "r") as f:
        next(f)
        next(f)
        try:
            while True:
                x1.append(float(next(f).split()[0]))
                x2.append(float(next(f).split()[0]))
                for i in range(10):
                    next(f)
        except StopIteration:
            pass
    return x1, x2


def write_results(results_file: str, y: List[float]) -> None:
    with open(results_file, "w") as f:
        for y_i in y:        
            f.write(f"{y_i}\n#\n")
            

if __name__ == '__main__':
    params = sys.argv[1]
    results = sys.argv[2]
    x1, x2 = get_variables(params)
    y = driver(x1, x2)
    write_results(results, y)
