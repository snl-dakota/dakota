#!/usr/bin/env python3
"""Generate strip_paths.json from src/dakota.json.

Walks the JSON schema and produces a structured path list that
dakota.tools._strip uses to remove
validator-inserted fields from validated JSON before passing it
back to dakota as input.

Usage:
    python -I tools/generate_strip_paths.py \\
        --schema src/dakota.json \\
        --output python/dakota/tools/strip_paths.json
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from typing import Any


def _resolve_ref(ref: str, defs: dict[str, Any]) -> dict[str, Any]:
    if not ref.startswith("#/$defs/"):
        return {}
    name = ref.split("/", 2)[2]
    return defs.get(name, {})


def _rules_for_def(def_schema: dict[str, Any]) -> dict[str, Any] | None:
    """Return stripping rules for a resolved schema def, or None if nothing to strip."""
    props: dict[str, Any] = def_schema.get("properties", {})
    computed: set[str] = set(def_schema.get("x-computed-fields", {}).keys())

    always: list[str] = list(computed)  # computed fields are always stripped
    flag_rules: list[dict[str, str]] = []
    inferred_rules: list[dict[str, str]] = []

    for field_name, field_schema in props.items():
        if not isinstance(field_schema, dict):
            continue
        if not field_schema.get("x-internal-only"):
            continue

        always.append(field_name)

        if field_name == "initial_point_user_provided":
            if "initial_point" in props and not props["initial_point"].get("x-internal-only"):
                flag_rules.append({"field": "initial_point", "flag": "initial_point_user_provided"})

        if field_name.startswith("inferred_"):
            ordinary = field_name[len("inferred_"):]
            if (
                ordinary in props
                and not props[ordinary].get("x-internal-only")
                and ordinary not in computed
            ):
                inferred_rules.append({"ordinary": ordinary, "inferred": field_name})

    if not always and not flag_rules and not inferred_rules:
        return None

    return {
        "always_strip": sorted(always),
        "strip_if_flag_false": flag_rules,
        "strip_if_inferred_differs": inferred_rules,
    }


def _def_name_from_property(prop_schema: dict[str, Any]) -> str | None:
    """Return the $defs name referenced by a schema property, if any."""
    if "$ref" in prop_schema:
        return prop_schema["$ref"].split("/")[-1]
    # handle anyOf with a single concrete ref
    for entry in prop_schema.get("anyOf", []):
        if "$ref" in entry:
            return entry["$ref"].split("/")[-1]
    return None


def build_block_rules(schema: dict[str, Any]) -> list[dict[str, Any]]:
    defs = schema.get("$defs", {})
    variables_prop = schema.get("properties", {}).get("variables", {})
    items_ref = variables_prop.get("items", {}).get("$ref")
    if not items_ref:
        return []

    vars_config_def = _resolve_ref(items_ref, defs)
    vars_props = vars_config_def.get("properties", {})

    block_rules = []

    for block_key, block_prop in vars_props.items():
        def_name = _def_name_from_property(block_prop)
        if not def_name:
            continue
        block_def = defs.get(def_name, {})
        block_props = block_def.get("properties", {})

        # Check for nested sub-keys (e.g. discrete_design_set -> integer/string/real)
        sub_keys_with_defs = {
            sub_key: _def_name_from_property(sub_prop)
            for sub_key, sub_prop in block_props.items()
            if _def_name_from_property(sub_prop) is not None
        }

        if sub_keys_with_defs and not block_def.get("x-computed-fields") and not any(
            v.get("x-internal-only") for v in block_props.values() if isinstance(v, dict)
        ):
            # Compound block — recurse into sub-keys
            for sub_key, sub_def_name in sub_keys_with_defs.items():
                sub_def = defs.get(sub_def_name, {})
                rules = _rules_for_def(sub_def)
                if rules:
                    block_rules.append({"nav": ["variables", block_key, sub_key], **rules})
        else:
            rules = _rules_for_def(block_def)
            if rules:
                block_rules.append({"nav": ["variables", block_key], **rules})

    return block_rules


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--schema",
        type=Path,
        required=True,
        help="Path to src/dakota.json",
    )
    parser.add_argument(
        "--output",
        type=Path,
        required=True,
        help="Path to write strip_paths.json",
    )
    args = parser.parse_args()

    schema_text = args.schema.read_text(encoding="utf-8")
    schema = json.loads(schema_text)
    schema_hash = hashlib.sha256(schema_text.encode()).hexdigest()

    block_rules = build_block_rules(schema)

    output = {
        "generated_from_schema": schema_hash,
        "block_rules": block_rules,
    }

    args.output.write_text(json.dumps(output, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote {len(block_rules)} block rules to {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
