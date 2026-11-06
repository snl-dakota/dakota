"""Apply strip_paths.json rules to remove validator-inserted fields from Dakota JSON.

This is an internal module; the public interface is dakota.tools.freeform_to_json.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

_STRIP_PATHS = Path(__file__).parent / "strip_paths.json"


def _load_rules(paths_file: Path = _STRIP_PATHS) -> list[dict[str, Any]]:
    return json.loads(paths_file.read_text(encoding="utf-8"))["block_rules"]


def _navigate(data: Any, nav: list[str]) -> list[dict[str, Any]]:
    """Return all dict nodes reached by following nav from data.

    A list at any step is automatically iterated; a missing key is skipped.
    """
    current: list[Any] = [data]
    for key in nav:
        next_level: list[Any] = []
        for node in current:
            if isinstance(node, list):
                for item in node:
                    if isinstance(item, dict) and key in item:
                        next_level.append(item[key])
            elif isinstance(node, dict) and key in node:
                next_level.append(node[key])
        current = next_level
    return [n for n in current if isinstance(n, dict)]


def apply(data: Any, rules: list[dict[str, Any]], removed: list[str] | None = None) -> Any:
    """Strip validator-inserted fields from *data* in-place and return it.

    *removed* is an optional list that receives dot-path strings for each
    removed field (useful for ``--verbose`` output).
    """
    for rule in rules:
        nav: list[str] = rule["nav"]
        always: list[str] = rule["always_strip"]
        flag_rules: list[dict[str, str]] = rule["strip_if_flag_false"]
        inferred_rules: list[dict[str, str]] = rule["strip_if_inferred_differs"]

        for block in _navigate(data, nav):
            path_prefix = ".".join(nav)

            # Save flag and inferred values before always_strip removes them.
            saved_flags: dict[str, Any] = {
                e["flag"]: block.get(e["flag"]) for e in flag_rules
            }
            saved_inferred: dict[str, Any] = {
                e["inferred"]: block.get(e["inferred"]) for e in inferred_rules
            }

            for field in always:
                if field in block:
                    if removed is not None:
                        removed.append(f"{path_prefix}.{field}")
                    del block[field]

            for entry in flag_rules:
                field, flag = entry["field"], entry["flag"]
                if field in block and saved_flags.get(flag) is False:
                    if removed is not None:
                        removed.append(f"{path_prefix}.{field}")
                    del block[field]

            for entry in inferred_rules:
                ordinary, inferred = entry["ordinary"], entry["inferred"]
                if ordinary in block and inferred in saved_inferred:
                    if saved_inferred[inferred] != block.get(ordinary):
                        # ordinary was defaulted (differs from the computed inferred value)
                        if removed is not None:
                            removed.append(f"{path_prefix}.{ordinary}")
                        del block[ordinary]

    return data


def strip_file(
    input_path: Path,
    output_path: Path,
    *,
    paths_file: Path = _STRIP_PATHS,
    verbose: bool = False,
) -> list[str]:
    """Strip *input_path* and write cleaned JSON to *output_path*.

    Returns the list of removed field paths.
    """
    rules = _load_rules(paths_file)
    data = json.loads(input_path.read_text(encoding="utf-8"))
    removed: list[str] = []
    apply(data, rules, removed)
    output_path.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")
    return removed


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Remove Dakota validation-inserted fields from validated JSON."
    )
    parser.add_argument("input", type=Path, help="Validated JSON input file")
    parser.add_argument("output", type=Path, nargs="?", help="Output JSON file")
    parser.add_argument(
        "--paths",
        type=Path,
        default=_STRIP_PATHS,
        help=f"strip_paths.json file (default: {_STRIP_PATHS})",
    )
    parser.add_argument("--in-place", action="store_true", help="Rewrite the input file")
    parser.add_argument("--verbose", action="store_true", help="Print removed paths to stderr")
    return parser.parse_args()


def main() -> int:
    import sys

    args = parse_args()
    if args.in_place:
        output_path = args.input
    else:
        if args.output is None:
            raise SystemExit("output path required unless --in-place is used")
        output_path = args.output

    removed = strip_file(args.input, output_path, paths_file=args.paths, verbose=args.verbose)

    if args.verbose:
        for path in removed:
            print(path, file=sys.stderr)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
