"""Execute and compare fresh freeform/Python runs, without saved baselines."""

import argparse
import difflib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import time


def execute(command, directory, name, env=None, *, timeout=300):
    stdout_path = directory / (name + ".stdout")
    stderr_path = directory / (name + ".stderr")
    with stdout_path.open("w") as out, stderr_path.open("w") as err:
        subprocess.run(
            command,
            cwd=directory,
            env=env,
            stdout=out,
            stderr=err,
            check=True,
            timeout=timeout,
        )


_APPROX_INTERFACE_ID = re.compile(r"\bAPPROX_INTERFACE_\d+\b")


def _normalize_approx_ids(text):
    """Replace APPROX_INTERFACE_<N> with APPROX_INTERFACE_N throughout *text*."""
    return _APPROX_INTERFACE_ID.sub("APPROX_INTERFACE_N", text)


def result_structure(path):
    text = _normalize_approx_ids(path.read_text())
    lines = [line.split() for line in text.splitlines() if line.strip()]
    if len(lines) < 2:
        raise ValueError("Output comparison failed")
    # Keep labels and row layout exact; let dakota_diff apply numerical tolerances.
    number = re.compile(r"[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?")
    return [
        ["#" if number.fullmatch(token) else token for token in line] for line in lines
    ]


def _write_normalized(path):
    """Write a .norm.tst sibling of *path* with APPROX_INTERFACE_<N> normalized.

    Returns the path of the normalized file.
    """
    norm_path = path.with_suffix(".norm.tst")
    norm_path.write_text(_normalize_approx_ids(path.read_text()))
    return norm_path


def compare_outputs(command, paths, work, case_id, subtest):
    """Compare reduced results numerically (dakota_diff) and structurally.

    The report has two clearly labeled sections so that a numerical DIFF from
    dakota_diff.perl is not confused with a label/layout mismatch:
      1. dakota_diff.perl verdict (base = freeform executable, test = Python)
      2. structural check of labels and row layout, with a unified diff only
         when the structure differs
    """
    report = work / "dakota_diffs.out"
    structure_matches = result_structure(paths[0]) == result_structure(paths[1])
    # Build a normalized command so that APPROX_INTERFACE_<N> numbering differences
    # do not cause dakota_diff.perl to treat label lines as mismatches.
    norm_paths = [_write_normalized(p) for p in paths]
    norm_command = list(command[:-2]) + [str(p) for p in norm_paths]
    numeric_error = None
    try:
        execute(norm_command, work, "comparison", {**os.environ, "DAKDIFF_ALL_NUMERIC": "1"})
    except subprocess.CalledProcessError as exc:
        numeric_error = exc
    finally:
        rule = "=" * 72 + "\n"
        diagnostics = f"python_parity_{case_id} (subtest {subtest}): freeform executable vs Python\n"
        diagnostics += f"  base: {paths[0]}\n  test: {paths[1]}\n"
        diagnostics += rule
        diagnostics += "[1] Numerical comparison (dakota_diff.perl on APPROX_INTERFACE_N-normalized copies; base< freeform, test> Python)\n"
        diagnostics += rule
        for name in ("comparison.stdout", "comparison.stderr"):
            path = work / name
            if path.exists():
                diagnostics += path.read_text(errors="replace")
        diagnostics += rule
        diagnostics += "[2] Structural comparison (labels and row layout, numbers masked)\n"
        diagnostics += rule
        if structure_matches:
            diagnostics += "Structure matches\n"
        else:
            diagnostics += "Structure differs; full unified diff of results.tst:\n"
            diagnostics += "".join(
                difflib.unified_diff(
                    paths[0].read_text().splitlines(keepends=True),
                    paths[1].read_text().splitlines(keepends=True),
                    fromfile="freeform/results.tst",
                    tofile="python/results.tst",
                )
            )
        diagnostics += rule
        verdict = "PASS" if (numeric_error is None and structure_matches) else "FAIL"
        reasons = []
        if numeric_error is not None:
            reasons.append(f"numerical DIFF (dakota_diff exit {numeric_error.returncode})")
        if not structure_matches:
            reasons.append("structure mismatch")
        diagnostics += f"{verdict} test {subtest}" + (f": {', '.join(reasons)}" if reasons else "") + "\n"
        report.write_text(diagnostics)
    if numeric_error is not None or not structure_matches:
        raise ValueError("Output comparison failed: " + ", ".join(reasons))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in (
        "case",
        "source",
        "work",
        "dakota",
        "driver",
        "plugin",
        "python",
        "python-path",
        "perl",
        "runner",
        "diff",
    ):
        parser.add_argument("--" + name, required=True)
    parser.add_argument("--build-test", required=False, default=None)
    args = parser.parse_args()
    source = Path(args.source).resolve()
    build_test = Path(args.build_test).resolve() if args.build_test else None
    cases = json.loads((source / "python_parity/cases.json").read_text())
    case = next(c for c in cases if c["id"] == args.case)
    work = Path(args.work).resolve()
    work.mkdir(parents=True, exist_ok=True)
    for name in ("comparison.stdout", "comparison.stderr"):
        (work / name).unlink(missing_ok=True)
    # Replaced after comparison; remains a FAIL if execution is interrupted.
    (work / "dakota_diffs.out").write_text(
        f"python_parity_{case['id']}: executable vs Python\nFAIL test {case['subtest']} (pair incomplete)\n"
    )
    for kind in ("freeform", "python"):
        directory = work / kind
        if directory.exists():
            shutil.rmtree(directory)
        directory.mkdir()
    started = time.monotonic()
    case_timeout = case.get("timeout", 1200)
    deadline = started + case_timeout - 5  # 5s margin for overhead
    env = os.environ.copy()
    env["PYTHONPATH"] = args.python_path + os.pathsep + env.get("PYTHONPATH", "")
    for kind in ("freeform", "python"):
        directory = work / kind
        for required in case["required_files"]:
            # reserved names resolve to build products rather than source files
            built = {"text_book": args.driver, "identity_map": args.plugin}
            origin = Path(built[required]) if required in built else source / required
            dst = directory / origin.name
            if origin.is_dir():
                shutil.copytree(origin, dst, dirs_exist_ok=True)
            else:
                shutil.copy2(origin, dst)
        if build_test:
            for driver in (case.get("build_test_drivers") or []):
                src = build_test / driver
                dst = directory / driver
                if src.is_dir():
                    shutil.copytree(src, dst, dirs_exist_ok=True)
                else:
                    shutil.copy2(src, dst)
                    dst.chmod(0o755)
        env["PATH"] = str(directory) + os.pathsep + os.environ.get("PATH", "")
        # Optional freeform setup study (e.g. exporting a surrogate that the
        # pair then imports) run in each working directory before the pair.
        if "setup_input" in case:
            shutil.copy2(source / case["setup_input"], directory / "setup.in")
            execute(
                [args.dakota, "-i", "setup.in", "-o", "setup.out", "-e", "setup.err"],
                directory,
                "setup",
                env,
            )
        if kind == "freeform":
            execute(
                [
                    args.perl,
                    args.runner,
                    "--file-extract=" + str(directory / "dakota.in"),
                    str(source / case["input"]),
                    str(case["subtest"]),
                ],
                directory,
                "extract",
            )
            execute(
                [
                    args.dakota,
                    "-i",
                    "dakota.in",
                    "-o",
                    "dakota.out",
                    "-e",
                    "dakota.err",
                ],
                directory,
                "run",
                env,
                timeout=max(deadline - time.monotonic(), 30),
            )
        else:
            execute(
                [args.python, str(source / "python_parity" / case["script"])],
                directory,
                "run",
                env,
                timeout=max(deadline - time.monotonic(), 30),
            )
        execute(
            [
                args.perl,
                args.runner,
                "--reduce-output=" + str(directory / "dakota.out"),
                "--reduce-destination=" + str(directory / "results.tst"),
                str(case["subtest"]),
            ],
            directory,
            "reduce",
        )
    paths = [work / kind / "results.tst" for kind in ("freeform", "python")]
    compare_outputs(
        [args.perl, args.diff, case["input"], *map(str, paths)],
        paths,
        work,
        case["id"],
        case["subtest"],
    )
    print(f"{case['id']}: numerical parity, {time.monotonic() - started:.3f}s")


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, StopIteration, subprocess.SubprocessError) as exc:
        work = sys.argv[sys.argv.index("--work") + 1] if "--work" in sys.argv else "."
        print(f"Python regression failed ({type(exc).__name__}: {exc}); "
              f"artifacts: {work}", file=sys.stderr)
        sys.exit(1)
