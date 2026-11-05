#!/usr/bin/env python3
"""Strip validator-inserted/internal context from Dakota validated JSON.

This tool removes fields that are not valid user input and are introduced by
Dakota validation/materialization, based on schema metadata in ``src/dakota.json``.

It removes, recursively:
- properties marked ``x-internal-only: true``
- properties listed in an object's ``x-computed-fields``

Usage:
    python -I tools/strip_inserted_context.py input.json output.json
    python -I tools/strip_inserted_context.py --in-place input.json
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any


class SchemaStripper:
    def __init__(self, full_schema: dict[str, Any]):
        self.full_schema = full_schema
        self.defs = full_schema.get("$defs", {})
        self.removed_paths: list[str] = []

    def resolve(self, schema_node: Any) -> dict[str, Any]:
        """Resolve local $ref chains and merge simple wrappers."""
        if not isinstance(schema_node, dict):
            return {}

        node = schema_node
        seen: set[str] = set()

        while "$ref" in node:
            ref = node["$ref"]
            if ref in seen:
                raise ValueError(f"Cyclic $ref encountered: {ref}")
            seen.add(ref)
            if not ref.startswith("#/$defs/"):
                raise ValueError(f"Unsupported $ref: {ref}")
            ref_name = ref.split("/", 2)[2]
            target = self.defs.get(ref_name)
            if target is None:
                raise KeyError(f"Missing schema definition: {ref_name}")

            merged = dict(target)
            for key, value in node.items():
                if key == "$ref":
                    continue
                merged[key] = value
            node = merged

        return node

    def strip(self, data: Any, schema_node: Any, path: str = "$") -> Any:
        schema_node = self.resolve(schema_node)

        if isinstance(data, dict):
            object_schema = self._select_object_schema(data, schema_node)
            props = object_schema.get("properties", {}) if object_schema else {}
            computed = set(object_schema.get("x-computed-fields", {}).keys()) if object_schema else set()
            internal = {
                name
                for name, prop_schema in props.items()
                if self.resolve(prop_schema).get("x-internal-only", False)
            }

            result: dict[str, Any] = {}
            for key, value in data.items():
                if key in computed or key in internal:
                    self.removed_paths.append(f"{path}.{key}")
                    continue
                child_schema = props.get(key, {})
                result[key] = self.strip(value, child_schema, f"{path}.{key}")
            return result

        if isinstance(data, list):
            item_schema = self._select_array_item_schema(data, schema_node)
            return [self.strip(item, item_schema, f"{path}[{idx}]") for idx, item in enumerate(data)]

        return data

    def _select_object_schema(self, obj: dict[str, Any], schema_node: dict[str, Any]) -> dict[str, Any]:
        schema_node = self.resolve(schema_node)

        if "properties" in schema_node or schema_node.get("type") == "object":
            return schema_node

        for key in ("oneOf", "anyOf", "allOf"):
            options = schema_node.get(key, [])
            for option in options:
                candidate = self.resolve(option)
                if self._object_schema_matches(obj, candidate):
                    return candidate

        return schema_node

    def _select_array_item_schema(self, arr: list[Any], schema_node: dict[str, Any]) -> Any:
        schema_node = self.resolve(schema_node)
        if "items" in schema_node:
            return schema_node["items"]

        for key in ("oneOf", "anyOf", "allOf"):
            options = schema_node.get(key, [])
            for option in options:
                candidate = self.resolve(option)
                if "items" in candidate:
                    return candidate["items"]

        return {}

    @staticmethod
    def _object_schema_matches(obj: dict[str, Any], candidate: dict[str, Any]) -> bool:
        props = candidate.get("properties", {})
        if not props:
            return False
        obj_keys = set(obj.keys())
        prop_keys = set(props.keys())
        required = set(candidate.get("required", []))
        return required.issubset(obj_keys) and bool(obj_keys & prop_keys)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Remove Dakota validation-inserted/internal fields from JSON input."
    )
    parser.add_argument("input", type=Path, help="Validated JSON input file")
    parser.add_argument("output", type=Path, nargs="?", help="Output JSON file")
    parser.add_argument(
        "--schema",
        type=Path,
        default=Path("/projects/src/dakota.json"),
        help="Dakota JSON schema file (default: /projects/src/dakota.json)",
    )
    parser.add_argument(
        "--root-def",
        default=None,
        help="Optional root schema definition under $defs; defaults to the schema root",
    )
    parser.add_argument(
        "--in-place",
        action="store_true",
        help="Rewrite the input file instead of writing a separate output file",
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Print removed JSON paths to stderr",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    if args.in_place:
        output_path = args.input
    else:
        if args.output is None:
            raise SystemExit("output path required unless --in-place is used")
        output_path = args.output

    with args.schema.open("r", encoding="utf-8") as f:
        schema = json.load(f)

    with args.input.open("r", encoding="utf-8") as f:
        data = json.load(f)

    if args.root_def is None:
        root_schema = schema
    else:
        root_schema = schema.get("$defs", {}).get(args.root_def)
        if root_schema is None:
            raise SystemExit(f"root definition '{args.root_def}' not found in schema")

    stripper = SchemaStripper(schema)
    cleaned = stripper.strip(data, root_schema)

    with output_path.open("w", encoding="utf-8") as f:
        json.dump(cleaned, f, indent=2)
        f.write("\n")

    if args.verbose:
        import sys

        for removed in stripper.removed_paths:
            print(removed, file=sys.stderr)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
