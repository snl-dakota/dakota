"""Report planned execution pairs against the actual binding declarations."""

import argparse
from collections import defaultdict
import json
from pathlib import Path
import re


def report(native=False):
    root = Path(__file__).resolve().parents[2]
    cases = json.loads((Path(__file__).with_name("cases.json")).read_text())
    study = (root / "src/python/study/BindStudy.cpp").read_text()
    iterator = (root / "src/python/study/BindIterator.cpp").read_text()
    methods = set(re.findall(r'bind_method\(factory, "(\w+)"', iterator))
    methods.update(re.findall(r'\.def\("(sampling|dot_bfgs|multi_start)"', iterator))
    models = set(
        re.findall(r'\.def\("(single|simulation|nested|\w+_surrogate)"', study)
    )
    factories = sorted(
        ["MethodFactory." + n for n in methods]
        + ["ModelFactory." + n for n in models if n != "single"]
    )
    if native:
        from dakota.study import MethodFactory, ModelFactory

        owners = {"MethodFactory": MethodFactory, "ModelFactory": ModelFactory}
        factories = [
            f for f in factories if hasattr(owners[f.split(".")[0]], f.split(".")[1])
        ]
    assignments = defaultdict(list)
    for case in cases:
        for factory in case["factories"]:
            assignments[factory].append(case["id"])
    paired = sum(bool(assignments[f]) for f in factories)
    lines = [
        "# Python paired execution inventory",
        "",
        f"{paired} of {len(factories)} factory entry points have paired cases.",
        "",
        "This lists authored execution pairs, not verified native passes. Optional",
        "factories are included unless `--native` filters to the installed extension.",
        "`ModelFactory.single` is an alias for `simulation`.",
        "",
        "| Factory | Paired cases |",
        "| --- | --- |",
    ]
    lines += [f"| `{f}` | {', '.join(assignments[f]) or 'TODO'} |" for f in factories]
    lines += [
        "",
        "## Native classes exercised by the paired scripts",
        "",
        ", ".join(
            "`" + c + "`"
            for c in sorted({c for case in cases for c in case["classes"]})
        ),
        "",
        "Multiple factory entry points can use the same native class; each factory",
        "still needs its own execution case. Timing claims require successful native runs.",
        "",
    ]
    return "\n".join(lines)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--native", action="store_true")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    result = report(args.native)
    if args.output:
        args.output.write_text(result)
    else:
        print(result)
