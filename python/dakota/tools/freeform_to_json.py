"""Convert a Dakota freeform input file to clean JSON format.

The conversion pipeline:
  1. ``dakota_parser <input.in> --json`` -- validate and serialize to JSON
  2. Strip validator-inserted fields (see dakota.tools._strip)
  3. Write the clean JSON to the output path

The result is a JSON input file that Dakota can accept directly with
``dakota -json <file>.json``.

Usage (command line)::

    dakota-to-json study.in study.json
    dakota-to-json study.in              # writes study.json
    dakota-to-json --help

Usage (Python)::

    from dakota.tools.freeform_to_json import convert
    convert("study.in", "study.json")
    convert("study.in", "study.json", dakota_parser="/path/to/dakota_parser")
"""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path

from . import _strip


def convert(
    input_path: str | Path,
    output_path: str | Path,
    *,
    dakota_parser: str | Path | None = None,
) -> None:
    """Convert *input_path* (freeform Dakota ``.in``) to *output_path* (JSON).

    Args:
        input_path: Path to the freeform Dakota input file.
        output_path: Destination path for the clean JSON output.
        dakota_parser: Path to the ``dakota_parser`` executable.  If ``None``,
            the executable is looked up on ``PATH``.

    Raises:
        FileNotFoundError: If ``dakota_parser`` is not found.
        subprocess.CalledProcessError: If ``dakota_parser`` exits non-zero.
    """
    input_path = Path(input_path)
    output_path = Path(output_path)

    parser_cmd = str(dakota_parser) if dakota_parser is not None else "dakota_parser"

    result = subprocess.run(
        [parser_cmd, str(input_path), "--json"],
        capture_output=True,
        text=True,
        check=True,
    )

    validated = json.loads(result.stdout)
    rules = _strip._load_rules()
    _strip.apply(validated, rules)

    output_path.write_text(json.dumps(validated, indent=2) + "\n", encoding="utf-8")


def _build_arg_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        prog="dakota-to-json",
        description="Convert a Dakota freeform input file to clean JSON format.",
    )
    p.add_argument("input", type=Path, help="Freeform Dakota input file (.in)")
    p.add_argument(
        "output",
        type=Path,
        nargs="?",
        help="Output JSON path (default: input with .json extension)",
    )
    p.add_argument(
        "--dakota-parser",
        metavar="EXE",
        default=None,
        help="Path to dakota_parser executable (default: look up on PATH)",
    )
    return p


def main() -> int:
    args = _build_arg_parser().parse_args()

    output = args.output
    if output is None:
        output = args.input.with_suffix(".json")

    try:
        convert(args.input, output, dakota_parser=args.dakota_parser)
    except FileNotFoundError as exc:
        print(f"dakota-to-json: {exc}", file=sys.stderr)
        return 1
    except subprocess.CalledProcessError as exc:
        print(exc.stderr, file=sys.stderr, end="")
        return exc.returncode

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
