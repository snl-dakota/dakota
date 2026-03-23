#!/usr/bin/env python3
"""
Extract candidate IR defaults from generated JSON Schema.

This script focuses on two default sources:
1) Field-level JSON Schema `default` values on nodes that carry x-materialization.
2) Union-level `x-model-default` selections, resolved through the chosen model's
   x-materialization entries (important for enum/presence patterns).

Output is keyed by key and records one or more candidate defaults with source
metadata.
"""

import argparse
import json
from collections import defaultdict
from pathlib import Path
from typing import Any, Dict, List, Tuple


class DefaultCandidate:
    def __init__(
        self,
        value,
        source,
        schema_path,
        storage_type,
        handler_type,
        keyword_name,
        member_variable_type,
        enum_scope="",
        chosen_model="",
    ):
        self.value = value
        self.source = source
        self.schema_path = schema_path
        self.storage_type = storage_type
        self.handler_type = handler_type
        self.keyword_name = keyword_name
        self.member_variable_type = member_variable_type
        self.enum_scope = enum_scope
        self.chosen_model = chosen_model


def _walk(node: Any, path: str = ""):
    if isinstance(node, dict):
        yield path, node
        for k, v in node.items():
            child = f"{path}/{k}" if path else k
            yield from _walk(v, child)
    elif isinstance(node, list):
        for i, v in enumerate(node):
            child = f"{path}[{i}]"
            yield from _walk(v, child)


def _to_ref_name(ref: str) -> str:
    # "#/$defs/Normal" -> "Normal"
    if "/$defs/" in ref:
        return ref.rsplit("/", 1)[-1]
    return ref


def _find_materializations(node: Any):
    mats = []
    for schema_path, obj in _walk(node):
        if not isinstance(obj, dict):
            continue
        xmat = obj.get("x-materialization")
        if not isinstance(xmat, list):
            continue
        for m in xmat:
            if isinstance(m, dict) and isinstance((m.get("ir_key") or m.get("pdb_key")), str):
                mats.append((schema_path, obj, m))
    return mats


def _infer_presence_value(m: dict) -> Any:
    storage = m.get("storage_type")
    if storage in ("PRESENCE_ENUM", "PRESENCE_LITERAL", "AUGMENT_ENUM"):
        return m.get("stored_value")
    if storage == "PRESENCE_BOOL":
        # Presence-bool semantics are keyword-presence driven in this schema.
        # If present in a chosen default branch, treat as true.
        return True
    return None


def _schema_default_targets(m: dict, default_value: Any) -> List[Tuple[str, Any, str]]:
    """Resolve schema-field defaults into IR default candidates.

    Most materialization entries map field defaults directly to their primary
    IR key, but a few storage types write multiple targets or route the field
    value to a secondary key instead.
    """
    storage_type = str(m.get("storage_type", ""))
    if storage_type == "TYPE_DATA_COMBINED":
        ir_key = m.get("secondary_ir_key")
        member_variable_type = str(
            m.get("secondary_ir_value_type")
            or m.get("secondary_member_variable_type")
            or m.get("secondary_value_type")
            or ""
        )
        if isinstance(ir_key, str):
            return [(ir_key, default_value, member_variable_type)]
        return []

    if storage_type == "METHOD_PIECEWISE":
        # Wrapper defaults like Literal[True] on the "piecewise" selector do not
        # mean Dakota's effective IR default is piecewise-selected. The actual
        # defaults come from the DataMethod constructor (e.g. expansionType and
        # piecewiseBasis initializers), so do not synthesize schema defaults here.
        return []

    ir_key = m.get("ir_key") or m.get("pdb_key")
    member_variable_type = str(
        m.get("ir_value_type")
        or m.get("member_variable_type")
        or ""
    )
    if isinstance(ir_key, str):
        return [(ir_key, default_value, member_variable_type)]
    return []


def _xmodel_default_targets(m: dict) -> List[Tuple[str, Any, str]]:
    """Resolve x-model-default branch selections into IR default candidates."""
    storage = str(m.get("storage_type", ""))
    if storage == "METHOD_PIECEWISE":
        return []

    ir_key = m.get("ir_key") or m.get("pdb_key")
    if not isinstance(ir_key, str):
        return []
    value = _infer_presence_value(m)
    if value is None:
        return []
    member_variable_type = str(
        m.get("ir_value_type")
        or m.get("member_variable_type")
        or ""
    )
    return [(ir_key, value, member_variable_type)]


def extract_defaults(schema: dict) -> Dict[str, List[DefaultCandidate]]:
    defs = schema.get("$defs", {})
    by_key = defaultdict(list)  # type: Dict[str, List[DefaultCandidate]]

    for schema_path, node in _walk(schema):
        if not isinstance(node, dict):
            continue

        # Source 1: direct field defaults on nodes with x-materialization
        if "default" in node and isinstance(node.get("x-materialization"), list):
            default_value = node["default"]
            if default_value is None:
                # Optional field default=None is not a useful IR initialization value.
                continue
            for m in node["x-materialization"]:
                if not isinstance(m, dict):
                    continue
                storage_type = str(m.get("storage_type", ""))
                # Presence/augment handlers are keyword-presence semantics.
                # The schema field default (often const true/false on a keyword model)
                # is usually NOT the IR default for the destination pdb_key.
                # For these, prefer x-model-default resolution.
                if storage_type in {
                    "PRESENCE_ENUM",
                    "PRESENCE_LITERAL",
                    "AUGMENT_ENUM",
                    "PRESENCE_BOOL",
                    "METHOD_PIECEWISE",
                }:
                    continue
                for ir_key, candidate_value, member_variable_type in _schema_default_targets(m, default_value):
                    if not isinstance(ir_key, str):
                        continue
                    by_key[ir_key].append(
                        DefaultCandidate(
                            value=candidate_value,
                            source="schema_default",
                            schema_path=schema_path,
                            storage_type=storage_type,
                            handler_type=str(m.get("handler_type", "")),
                            keyword_name=str(m.get("keyword_name", "")),
                            member_variable_type=member_variable_type,
                            enum_scope=str(m.get("enum_scope", "")),
                        )
                    )

        # Source 2: union x-model-default (important enum case)
        x_model_default = node.get("x-model-default")
        any_of = node.get("anyOf")
        if not isinstance(x_model_default, str) or not isinstance(any_of, list):
            continue

        refs = [
            _to_ref_name(entry.get("$ref", ""))
            for entry in any_of
            if isinstance(entry, dict) and isinstance(entry.get("$ref"), str)
        ]
        if x_model_default not in refs:
            # Sometimes x-model-default may be absent from refs in malformed schemas.
            continue

        chosen_def = defs.get(x_model_default)
        if not isinstance(chosen_def, dict):
            continue

        for chosen_schema_path, _obj, m in _find_materializations(chosen_def):
            storage_type = str(m.get("storage_type", ""))
            targets = _xmodel_default_targets(m)
            if not targets:
                # x-model-default is most useful for presence/enum-like storage.
                continue
            for ir_key, value, member_variable_type in targets:
                if not isinstance(ir_key, str):
                    continue
                by_key[ir_key].append(
                    DefaultCandidate(
                        value=value,
                        source="x_model_default",
                        schema_path=f"{schema_path} -> $defs/{x_model_default}:{chosen_schema_path}",
                        storage_type=storage_type,
                        handler_type=str(m.get("handler_type", "")),
                        keyword_name=str(m.get("keyword_name", "")),
                        member_variable_type=member_variable_type,
                        enum_scope=str(m.get("enum_scope", "")),
                        chosen_model=x_model_default,
                    )
                )

    # Deduplicate exact candidate records per key
    deduped = {}  # type: Dict[str, List[DefaultCandidate]]
    for ir_key, candidates in by_key.items():
        seen = set()
        uniq = []
        for c in candidates:
            sig = json.dumps(
                {
                    "value": c.value,
                    "source": c.source,
                    "schema_path": c.schema_path,
                    "storage_type": c.storage_type,
                    "handler_type": c.handler_type,
                    "keyword_name": c.keyword_name,
                    "member_variable_type": c.member_variable_type,
                    "enum_scope": c.enum_scope,
                    "chosen_model": c.chosen_model,
                },
                sort_keys=True,
                default=str,
            )
            if sig in seen:
                continue
            seen.add(sig)
            uniq.append(c)
        deduped[ir_key] = uniq
    return deduped


def summarize(cands: Dict[str, List[DefaultCandidate]]) -> Dict[str, Any]:
    num_keys = len(cands)
    num_candidates = sum(len(v) for v in cands.values())
    enum_like = 0
    mixed = 0
    conflicting = 0
    for vals in cands.values():
        values = {json.dumps(v.value, sort_keys=True) for v in vals}
        if len(values) > 1:
            conflicting += 1
        storage_types = {v.storage_type for v in vals}
        if storage_types & {"PRESENCE_ENUM", "PRESENCE_LITERAL", "AUGMENT_ENUM", "PRESENCE_BOOL"}:
            enum_like += 1
        if len({v.source for v in vals}) > 1:
            mixed += 1
    return {
        "keys_with_candidates": num_keys,
        "total_candidates": num_candidates,
        "keys_with_presence_or_enum_like_defaults": enum_like,
        "keys_with_mixed_sources": mixed,
        "keys_with_conflicting_values": conflicting,
    }


def main() -> int:
    ap = argparse.ArgumentParser(description="Extract candidate IR defaults from schema")
    ap.add_argument("--schema", required=True, help="Path to dakota_study_schema.json")
    ap.add_argument(
        "--output-json",
        default="update_pdb_keys/schema_defaults_extracted.json",
        help="Output JSON path",
    )
    args = ap.parse_args()

    schema_path = Path(args.schema)
    out_path = Path(args.output_json)

    schema = json.loads(schema_path.read_text(encoding="utf-8"))
    cands = extract_defaults(schema)
    summary = summarize(cands)

    blocks = ("environment", "method", "model", "variables", "interface", "responses")
    defaults_by_block = {b: {} for b in blocks}  # type: Dict[str, Dict[str, Any]]
    skipped_invalid_key = 0
    for ir_key in sorted(cands.keys()):
        parts = ir_key.split(".", 1)
        if len(parts) != 2 or parts[0] not in defaults_by_block:
            skipped_invalid_key += 1
            continue
        block, local_key = parts
        minimal_candidates = []
        for cand in cands[ir_key]:
            minimal_candidates.append({
                "value": cand.value,
                "value_type": cand.member_variable_type,
                "enum_scope": cand.enum_scope,
            })
        defaults_by_block[block][local_key] = {
            "key": ir_key,
            "candidates": minimal_candidates,
        }

    out = {"blocks": {block: {k: defaults_by_block[block][k] for k in sorted(defaults_by_block[block].keys())}
                      for block in blocks}}
    out_path.write_text(json.dumps(out, indent=2) + "\n", encoding="utf-8")

    print(f"Wrote: {out_path}")
    for k, v in summary.items():
        print(f"{k}: {v}")
    print(f"skipped_invalid_keys: {skipped_invalid_key}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
