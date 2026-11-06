"""Check that dakota-to-json preserves Dakota's IR for a freeform input case.

For one case from a manifest (python_parity/cases.json or gen_json/cases.json):
  1. extract the case's subtest with dakota_test.perl --file-extract
  2. convert it with dakota.tools.freeform_to_json
  3. dump the IR of both with ``dakota -check -dump_ir`` (no study is run)
  4. the two IR dumps must be identical
"""

import argparse
import json
import math
import os
from pathlib import Path
import shutil
import subprocess
import sys
import time


def execute(command, directory, name, env=None, *, timeout=300):
    stdout_path = directory / (name + ".stdout")
    stderr_path = directory / (name + ".stderr")
    with stdout_path.open("w") as out, stderr_path.open("w") as err:
        subprocess.run(command, cwd=directory, env=env, stdout=out, stderr=err,
                       check=True, timeout=timeout)


def copy_required_files(directory, source, case, driver, plugin):
    built = {"text_book": driver, "identity_map": plugin}
    for required in case.get("required_files", []):
        origin = Path(built[required]) if required in built else source / required
        destination = directory / origin.name
        if origin.is_dir():
            shutil.copytree(origin, destination, dirs_exist_ok=True)
        else:
            shutil.copy2(origin, destination)


# set_probabilities normalization is not idempotent, so a converted input can
# differ from the freeform IR by a few ulps (see DI_CLEANUP_TODO.md item 9).
FLOAT_REL_TOL = 1e-14


def json_differences(base, test, path="$"):
    if isinstance(base, float) and isinstance(test, float):
        if math.isclose(base, test, rel_tol=FLOAT_REL_TOL):
            return []
        return [f"{path}: {base!r} != {test!r}"]
    if type(base) is not type(test):
        return [f"{path}: {base!r} != {test!r}"]
    if isinstance(base, dict):
        diffs = []
        for key in sorted(set(base) | set(test)):
            if key not in test:
                diffs.append(f"{path}.{key}: missing from JSON-input IR")
            elif key not in base:
                diffs.append(f"{path}.{key}: only in JSON-input IR")
            else:
                diffs.extend(json_differences(base[key], test[key], f"{path}.{key}"))
        return diffs
    if isinstance(base, list):
        if len(base) != len(test):
            return [f"{path}: length {len(base)} != {len(test)}"]
        diffs = []
        for index, (b, t) in enumerate(zip(base, test)):
            diffs.extend(json_differences(b, t, f"{path}[{index}]"))
        return diffs
    return [] if base == test else [f"{path}: {base!r} != {test!r}"]


def write_report(work, case_id, subtest, duration, status, message):
    (work / "gen_json_report.out").write_text(
        f"gen_json_{case_id} (subtest {subtest})\n"
        f"{status} test {subtest}: {message}\n"
        f"duration_seconds: {duration:.3f}\n"
    )


def run_case(args, source, case, work, env, deadline):
    def remaining():
        return max(deadline - time.monotonic(), 30)

    copy_required_files(work, source, case, args.driver, args.plugin)
    env["PATH"] = str(work) + os.pathsep + os.environ.get("PATH", "")

    execute([args.perl, args.runner, "--file-extract=" + str(work / "dakota.in"),
             str(source / case["input"]), str(case["subtest"])], work, "extract")

    # Some cases import files produced by a setup study during construction.
    if "setup_input" in case:
        shutil.copy2(source / case["setup_input"], work / "setup.in")
        execute([args.dakota, "-i", "setup.in", "-o", "setup.out", "-e", "setup.err"],
                work, "setup", env, timeout=remaining())

    execute([args.python, "-m", "dakota.tools.freeform_to_json", "dakota.in", "dakota.json",
             "--dakota-parser", args.dakota_parser], work, "convert", env, timeout=remaining())

    execute([args.dakota, "-check", "-dump_ir", "freeform_ir.json", "-i", "dakota.in",
             "-o", "freeform_check.out", "-e", "freeform_check.err"],
            work, "freeform_check", env, timeout=remaining())
    execute([args.dakota, "-check", "-dump_ir", "json_ir.json", "-json", "dakota.json",
             "-o", "json_check.out", "-e", "json_check.err"],
            work, "json_check", env, timeout=remaining())

    base, test = (json.loads((work / name).read_text())["values"]
                  for name in ("freeform_ir.json", "json_ir.json"))
    differences = json_differences(base, test)
    (work / "ir_diffs.out").write_text(
        "".join(line + "\n" for line in differences) or "IR identical\n")
    return differences


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("case", "manifest", "source", "work", "dakota", "dakota-parser",
                 "driver", "plugin", "python", "python-path", "perl", "runner"):
        parser.add_argument("--" + name, required=True)
    args = parser.parse_args()

    source = Path(args.source).resolve()
    cases = json.loads((source / args.manifest).read_text())
    case = next(c for c in cases if c["id"] == args.case)
    work = Path(args.work).resolve()
    if work.exists():
        shutil.rmtree(work)
    work.mkdir(parents=True)

    started = time.monotonic()
    deadline = started + case.get("timeout", 1200) - 5
    env = os.environ.copy()
    env["PYTHONPATH"] = args.python_path + os.pathsep + env.get("PYTHONPATH", "")

    try:
        differences = run_case(args, source, case, work, env, deadline)
    except (OSError, StopIteration, subprocess.SubprocessError) as exc:
        write_report(work, case["id"], case["subtest"], time.monotonic() - started,
                     "FAIL", f"{type(exc).__name__}: {exc}")
        raise

    duration = time.monotonic() - started
    if differences:
        write_report(work, case["id"], case["subtest"], duration, "FAIL",
                     f"IR differs at {len(differences)} key(s); see ir_diffs.out")
        raise ValueError(f"IR differs at {len(differences)} key(s)")
    write_report(work, case["id"], case["subtest"], duration, "PASS",
                 "freeform and JSON inputs produce identical IR")
    print(f"{case['id']}: identical IR, {duration:.3f}s")


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, StopIteration, subprocess.SubprocessError) as exc:
        work = sys.argv[sys.argv.index("--work") + 1] if "--work" in sys.argv else "."
        print(f"gen_json regression failed ({type(exc).__name__}: {exc}); artifacts: {work}",
              file=sys.stderr)
        sys.exit(1)
