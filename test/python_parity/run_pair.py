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


def execute(command, directory, name, env=None, *, timeout=110):
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


def result_structure(path):
    lines = [line.split() for line in path.read_text().splitlines() if line.strip()]
    if len(lines) < 2:
        raise ValueError("Output comparison failed")
    # Keep labels and row layout exact; let dakota_diff apply numerical tolerances.
    number = re.compile(r"[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?")
    return [
        ["#" if number.fullmatch(token) else token for token in line] for line in lines
    ]


def compare_outputs(command, paths, work, case_id, subtest):
    report = work / "dakota_diffs.out"
    structure_matches = result_structure(paths[0]) == result_structure(paths[1])
    try:
        execute(command, work, "comparison", {**os.environ, "DAKDIFF_ALL_NUMERIC": "1"})
    finally:
        diagnostics = f"python_parity_{case_id}: executable vs Python\n"
        for name in ("comparison.stdout", "comparison.stderr"):
            path = work / name
            if path.exists():
                diagnostics += path.read_text(errors="replace")
        if not structure_matches:
            diagnostics += f"DIFF test {subtest}\n"
            diagnostics += "".join(
                difflib.unified_diff(
                    paths[0].read_text().splitlines(keepends=True),
                    paths[1].read_text().splitlines(keepends=True),
                    fromfile="executable/results.tst",
                    tofile="python/results.tst",
                )
            )
        report.write_text(diagnostics)
    if not structure_matches:
        raise ValueError("Output comparison failed")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in (
        "case",
        "source",
        "work",
        "dakota",
        "driver",
        "python",
        "python-path",
        "perl",
        "runner",
        "diff",
    ):
        parser.add_argument("--" + name, required=True)
    args = parser.parse_args()
    source = Path(args.source).resolve()
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
    env = os.environ.copy()
    env["PYTHONPATH"] = args.python_path + os.pathsep + env.get("PYTHONPATH", "")
    for kind in ("freeform", "python"):
        directory = work / kind
        for required in case["required_files"]:
            origin = Path(args.driver) if required == "text_book" else source / required
            shutil.copy2(origin, directory / Path(required).name)
        env["PATH"] = str(directory) + os.pathsep + os.environ.get("PATH", "")
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
            )
        else:
            execute(
                [args.python, str(source / "python_parity" / case["script"])],
                directory,
                "run",
                env,
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
        print(f"Python regression failed; artifacts: {work}", file=sys.stderr)
        sys.exit(1)
