"""Cheap smooth responses shared by the remaining execution pairs."""

import sys
from pathlib import Path


def evaluate(mode, values):
    """Return values, gradients and Hessians in Dakota response order."""
    if mode == "richardson":
        n = values[0]
        return [(2.0 + n**-2, [-2.0 * n**-3], [[6.0 * n**-4]])]
    x, y = values[:2]
    if mode == "least_squares":
        return [
            (x - 0.8, [1.0, 0.0], [[0.0, 0.0], [0.0, 0.0]]),
            (y - 1.2, [0.0, 1.0], [[0.0, 0.0], [0.0, 0.0]]),
        ]
    if mode == "optimization":
        targets = [0.8, 1.2, 1.2][: len(values)]
        gradient = [2 * (v - target) for v, target in zip(values, targets, strict=True)]
        hessian = [
            [2.0 if i == j else 0.0 for j in range(len(values))]
            for i in range(len(values))
        ]
        return [
            (
                sum(
                    (v - target) ** 2 for v, target in zip(values, targets, strict=True)
                ),
                gradient,
                hessian,
            )
        ]
    cross = 0.1 if mode == "low" else 0.2
    level = values[2] if len(values) > 2 else 1.0
    error = (0.1 * x * x + 0.05 * y) / level**2 if len(values) > 2 else 0.0
    value = x + 0.5 * y + cross * x * y + 0.1 * y * y + error
    gradient = [1 + cross * y, 0.5 + cross * x + 0.2 * y]
    hessian = [[0.0, cross], [cross, 0.2]]
    if len(values) > 2:
        gradient[0] += 0.2 * x / level**2
        gradient[1] += 0.05 / level**2
        hessian[0][0] += 0.2 / level**2
        # The solution level is a discrete state, excluded from derivatives.
    return [(value, gradient, hessian)]


def run(mode, parameters, results):
    lines = iter(Path(parameters).read_text().splitlines())
    count = int(next(lines).split()[0])
    values = [float(next(lines).split()[0]) for _ in range(count)]
    count = int(next(lines).split()[0])
    requests = [int(next(lines).split()[0]) for _ in range(count)]
    count = int(next(lines).split()[0])
    derivative_ids = [int(next(lines).split()[0]) - 1 for _ in range(count)]
    responses = evaluate(mode, values)
    output = []
    pairs = list(zip(requests, responses, strict=True))
    for request, (value, _, _) in pairs:
        if request & 1:
            output.append(f"{value:.17g}")
    for request, (_, gradient, _) in pairs:
        if request & 2:
            output.append(
                "[ " + " ".join(f"{gradient[i]:.17g}" for i in derivative_ids) + " ]"
            )
    for request, (_, _, hessian) in pairs:
        if request & 4:
            output.append(
                "[[ "
                + "\n".join(
                    " ".join(f"{hessian[i][j]:.17g}" for j in derivative_ids)
                    for i in derivative_ids
                )
                + " ]]"
            )
    Path(results).write_text("\n".join(output) + "\n")


if __name__ == "__main__":
    run(*sys.argv[1:])
