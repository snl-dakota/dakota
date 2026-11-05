"""Execute parser -> strip -> dakota for a freeform input case.

This reuses the python_parity manifest so the same freeform inputs and required
files can be exercised through:
  1. dakota_parser <input> --json
  2. strip_inserted_context.py <validated.json> <stripped.json>
  3. dakota -input stripped.json
"""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import time


def execute(command, directory, name, env=None, *, timeout=300, stdin_path=None):
    stdout_path = directory / (name + ".stdout")
    stderr_path = directory / (name + ".stderr")
    with stdout_path.open("w") as out, stderr_path.open("w") as err:
        if stdin_path is None:
            subprocess.run(
                command,
                cwd=directory,
                env=env,
                stdout=out,
                stderr=err,
                check=True,
                timeout=timeout,
            )
        else:
            with Path(stdin_path).open("r", encoding="utf-8") as inp:
                subprocess.run(
                    command,
                    cwd=directory,
                    env=env,
                    stdin=inp,
                    stdout=out,
                    stderr=err,
                    check=True,
                    timeout=timeout,
                )


def load_case(source: Path, case_id: str) -> dict:
    cases = json.loads((source / "python_parity/cases.json").read_text())
    return next(c for c in cases if c["id"] == case_id)


def copy_required_files(directory: Path, source: Path, build_test: Path | None, case: dict,
                        driver: str, plugin: str):
    built = {"text_book": driver, "identity_map": plugin}
    for required in case["required_files"]:
        origin = Path(built[required]) if required in built else source / required
        destination = directory / origin.name
        if origin.is_dir():
            shutil.copytree(origin, destination, dirs_exist_ok=True)
        else:
            shutil.copy2(origin, destination)
    if build_test:
        for extra in (case.get("build_test_drivers") or []):
            src = build_test / extra
            dst = directory / extra
            if src.is_dir():
                shutil.copytree(src, dst, dirs_exist_ok=True)
            else:
                shutil.copy2(src, dst)
                dst.chmod(0o755)


def write_report(work: Path, case_id: str, subtest: int, duration: float, status: str, message: str):
    (work / "parser_strip_report.out").write_text(
        f"parser_strip_{case_id} (subtest {subtest})\n"
        f"{status} test {subtest}: {message}\n"
        f"duration_seconds: {duration:.3f}\n"
    )


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in (
        "case",
        "source",
        "work",
        "dakota",
        "dakota-parser",
        "driver",
        "plugin",
        "python",
        "python-path",
        "strip-script",
    ):
        parser.add_argument("--" + name, required=True)
    parser.add_argument("--build-test", required=False, default=None)
    args = parser.parse_args()

    source = Path(args.source).resolve()
    build_test = Path(args.build_test).resolve() if args.build_test else None
    work = Path(args.work).resolve()
    if work.exists():
        shutil.rmtree(work)
    work.mkdir(parents=True)

    case = load_case(source, args.case)
    started = time.monotonic()
    case_timeout = case.get("timeout", 1200)
    deadline = started + case_timeout - 5

    env = os.environ.copy()
    env["PYTHONPATH"] = args.python_path + os.pathsep + env.get("PYTHONPATH", "")

    try:
        copy_required_files(work, source, build_test, case, args.driver, args.plugin)
        env["PATH"] = str(work) + os.pathsep + os.environ.get("PATH", "")

        input_name = Path(case["input"]).name
        shutil.copy2(source / case["input"], work / input_name)

        if "setup_input" in case:
            setup_name = "setup.in"
            shutil.copy2(source / case["setup_input"], work / setup_name)
            execute(
                [args.dakota, "-i", setup_name, "-o", "setup.out", "-e", "setup.err"],
                work,
                "setup",
                env,
                timeout=max(deadline - time.monotonic(), 30),
            )

        validated_json = work / "validated.json"
        stripped_json = work / "stripped.json"

        execute(
            [args.dakota_parser, input_name, "--json"],
            work,
            "parser",
            env,
            timeout=max(deadline - time.monotonic(), 30),
        )
        validated_json.write_text((work / "parser.stdout").read_text(encoding="utf-8"), encoding="utf-8")

        execute(
            [args.python, "-I", args.strip_script, str(validated_json), str(stripped_json), "--verbose"],
            work,
            "strip",
            env,
            timeout=max(deadline - time.monotonic(), 30),
        )

        execute(
            [args.dakota, "-i", str(stripped_json.name), "-o", "dakota.out", "-e", "dakota.err"],
            work,
            "dakota",
            env,
            timeout=max(deadline - time.monotonic(), 30),
        )

    except (OSError, StopIteration, subprocess.SubprocessError) as exc:
        duration = time.monotonic() - started
        write_report(work, case["id"], case["subtest"], duration, "FAIL", f"{type(exc).__name__}: {exc}")
        print(f"Parser/strip regression failed ({type(exc).__name__}: {exc}); artifacts: {work}", file=sys.stderr)
        raise

    duration = time.monotonic() - started
    write_report(work, case["id"], case["subtest"], duration, "PASS", "parser -> strip -> dakota succeeded")
    print(f"{case['id']}: parser -> strip -> dakota, {duration:.3f}s")


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, StopIteration, subprocess.SubprocessError):
        sys.exit(1)
