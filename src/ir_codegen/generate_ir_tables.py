#!/usr/bin/env python3
"""
Generate block-specific C++ IR contract/instruction tables keyed by IR keys,
plus shared types and a central registry for block lookup.
"""

import argparse
import hashlib
import json
import re
from pathlib import Path
from typing import Any, Dict, List, Optional, Set, Tuple

BLOCKS = ("environment", "method", "model", "variables", "interface", "responses")

INT_TYPES = {
    "int",
    "short",
    "unsigned short",
    "size_t",
    "unsigned int",
    "long",
    "unsigned long",
    "long long",
    "unsigned long long",
}
FLOAT_TYPES = {"Real", "double", "float"}


class KeyContract:
    def __init__(
        self,
        full_ir_key,
        local_ir_key,
        member_variable_type,
        value_type_tag,
        default_kind,
        default_value,
        enum_scope,
        source,
    ):
        self.full_ir_key = full_ir_key
        self.local_ir_key = local_ir_key
        self.member_variable_type = member_variable_type
        self.value_type_tag = value_type_tag
        self.default_kind = default_kind
        self.default_value = default_value
        self.enum_scope = enum_scope
        self.source = source


def split_pdb_key(full_key: str) -> Tuple[str, str]:
    parts = full_key.split(".", 1)
    if len(parts) != 2:
        raise ValueError(f"Invalid IR key: {full_key}")
    block, local = parts
    if block not in BLOCKS:
        raise ValueError(f"Unknown block '{block}' in IR key '{full_key}'")
    return block, local


def type_tag(member_type: str) -> str:
    t = " ".join(member_type.split())
    if t == "bool":
        return "Bool"
    if t in INT_TYPES:
        return "Integral"
    if t in FLOAT_TYPES:
        return "Floating"
    if t in {"String", "std::string", "string"}:
        return "String"
    return "Object"


def policy_default_for_type(member_type: str) -> Any:
    t = " ".join(member_type.split())
    if t == "bool":
        return False
    if t in INT_TYPES:
        return 0
    if t in FLOAT_TYPES:
        return 0.0
    if t in {"String", "std::string", "string"}:
        return ""
    return [] if t.endswith("Array") or t.endswith("Vector") else {}


def _json_sig(v: Any) -> str:
    return json.dumps(v, sort_keys=True, separators=(",", ":"))


def load_schema_defaults(schema_defaults_path: Path) -> Dict[str, Dict[str, KeyContract]]:
    data = json.loads(schema_defaults_path.read_text(encoding="utf-8"))
    by_block = {b: {} for b in BLOCKS}  # type: Dict[str, Dict[str, KeyContract]]

    blocks = data.get("blocks")
    if isinstance(blocks, dict):
        for block, payload in blocks.items():
            if block not in BLOCKS or not isinstance(payload, dict):
                continue
            defaults = payload
            wrapper = payload.get("defaults_by_key")
            if isinstance(wrapper, dict):
                defaults = wrapper
            wrapper = payload.get("defaults_by_ir_key")
            if isinstance(wrapper, dict):
                defaults = wrapper
            if not isinstance(defaults, dict):
                # backward-compat
                defaults = payload.get("defaults_by_pdb_key")
            if not isinstance(defaults, dict):
                continue
            for local_key, info in defaults.items():
                if not isinstance(info, dict):
                    continue
                full = info.get("key") or info.get("ir_key") or info.get("pdb_key")
                if not isinstance(full, str):
                    full = f"{block}.{local_key}"
                cands = info.get("candidates")
                if not isinstance(cands, list) or not cands:
                    continue
                values = []
                member_types = []
                for c in cands:
                    if not isinstance(c, dict):
                        continue
                    values.append(c.get("value"))
                    mt = c.get("value_type") or c.get("ir_value_type") or c.get("member_variable_type")
                    if isinstance(mt, str) and mt:
                        member_types.append(mt)
                unique_values = {_json_sig(v): v for v in values}
                if len(unique_values) != 1:
                    raise ValueError(f"Schema defaults conflict for {full}: {sorted(unique_values.keys())}")
                unique_member_types = sorted(set(member_types))
                if len(unique_member_types) != 1:
                    raise ValueError(f"Member type conflict for {full}: {unique_member_types}")
                mt = unique_member_types[0]
                by_block[block][local_key] = KeyContract(
                    full_ir_key=full,
                    local_ir_key=local_key,
                    member_variable_type=mt,
                    value_type_tag=type_tag(mt),
                    default_kind="schema",
                    default_value=next(iter(unique_values.values())),
                    enum_scope=str(next((c.get("enum_scope", "") for c in cands if isinstance(c, dict) and c.get("enum_scope")), "")),
                    source="schema_default",
                )
        return by_block

    entries = data.get("defaults_by_key_type", {})
    if not isinstance(entries, dict):
        return by_block

    for payload in entries.values():
        if not isinstance(payload, dict):
            continue
        full = payload.get("key") or payload.get("ir_key") or payload.get("pdb_key")
        cands = payload.get("candidates")
        if not isinstance(full, str) or not isinstance(cands, list) or not cands:
            continue
        block, local = split_pdb_key(full)

        values = []
        member_types = []
        for c in cands:
            if not isinstance(c, dict):
                continue
            values.append(c.get("value"))
            mt = c.get("value_type") or c.get("ir_value_type") or c.get("member_variable_type")
            if isinstance(mt, str) and mt:
                member_types.append(mt)

        unique_values = {_json_sig(v): v for v in values}
        if len(unique_values) != 1:
            raise ValueError(f"Schema defaults conflict for {full}: {sorted(unique_values.keys())}")

        unique_member_types = sorted(set(member_types))
        if len(unique_member_types) != 1:
            raise ValueError(f"Member type conflict for {full}: {unique_member_types}")

        mt = unique_member_types[0]
        by_block[block][local] = KeyContract(
            full_ir_key=full,
            local_ir_key=local,
            member_variable_type=mt,
            value_type_tag=type_tag(mt),
            default_kind="schema",
            default_value=next(iter(unique_values.values())),
            enum_scope=str(next((c.get("enum_scope", "") for c in cands if isinstance(c, dict) and c.get("enum_scope")), "")),
            source="schema_default",
        )

    return by_block


def load_overrides(override_registry_path: Path) -> Dict[str, Dict[str, KeyContract]]:
    data = json.loads(override_registry_path.read_text(encoding="utf-8"))
    by_block = {b: {} for b in BLOCKS}  # type: Dict[str, Dict[str, KeyContract]]
    for block, payload in (data.get("blocks") or {}).items():
        if block not in BLOCKS or not isinstance(payload, dict):
            continue
        entries = payload
        wrapper = payload.get("overrides_by_key")
        if isinstance(wrapper, dict):
            entries = wrapper
        wrapper = payload.get("overrides_by_ir_key")
        if isinstance(wrapper, dict):
            entries = wrapper
        if not isinstance(entries, dict):
            # backward-compat
            entries = payload.get("overrides_by_pdb_key")
        if not isinstance(entries, dict):
            continue
        for local_key, info in entries.items():
            if not isinstance(info, dict):
                continue
            full = info.get("key") or info.get("ir_key") or info.get("pdb_key")
            mt = info.get("value_type") or info.get("ir_value_type") or info.get("member_variable_type")
            if not isinstance(full, str) or not isinstance(mt, str) or not mt:
                continue
            by_block[block][local_key] = KeyContract(
                full_ir_key=full,
                local_ir_key=local_key,
                member_variable_type=mt,
                value_type_tag=type_tag(mt),
                default_kind="override",
                default_value=info.get("value"),
                enum_scope=str(info.get("enum_scope", "")),
                source="override_registry",
            )
    return by_block


def load_policies(policy_registry_path: Path) -> Dict[str, Dict[str, KeyContract]]:
    data = json.loads(policy_registry_path.read_text(encoding="utf-8"))
    by_block = {b: {} for b in BLOCKS}  # type: Dict[str, Dict[str, KeyContract]]
    for block, payload in (data.get("blocks") or {}).items():
        if block not in BLOCKS or not isinstance(payload, dict):
            continue
        entries = payload
        wrapper = payload.get("policies_by_key")
        if isinstance(wrapper, dict):
            entries = wrapper
        wrapper = payload.get("policies_by_ir_key")
        if isinstance(wrapper, dict):
            entries = wrapper
        if not isinstance(entries, dict):
            # backward-compat
            entries = payload.get("policies_by_pdb_key")
        if not isinstance(entries, dict):
            continue
        for local_key, info in entries.items():
            if not isinstance(info, dict):
                continue
            full = info.get("key") or info.get("ir_key") or info.get("pdb_key")
            mt = info.get("value_type") or info.get("ir_value_type") or info.get("member_variable_type")
            if not isinstance(full, str) or not isinstance(mt, str) or not mt:
                continue
            by_block[block][local_key] = KeyContract(
                full_ir_key=full,
                local_ir_key=local_key,
                member_variable_type=mt,
                value_type_tag=type_tag(mt),
                default_kind="policy",
                default_value=policy_default_for_type(mt),
                enum_scope=str(info.get("enum_scope", "")),
                source="policy_registry",
            )
    return by_block


def merge_contracts(
    schema: Dict[str, Dict[str, KeyContract]],
    overrides: Dict[str, Dict[str, KeyContract]],
    policies: Dict[str, Dict[str, KeyContract]],
) -> Dict[str, Dict[str, KeyContract]]:
    out = {b: {} for b in BLOCKS}  # type: Dict[str, Dict[str, KeyContract]]

    for block in BLOCKS:
        schema_keys = set(schema[block].keys())
        override_keys = set(overrides[block].keys())
        policy_keys = set(policies[block].keys())

        c1 = sorted(schema_keys & override_keys)
        if c1:
            raise ValueError(f"Schema/override collisions in {block}: {', '.join(c1)}")
        c2 = sorted((schema_keys | override_keys) & policy_keys)
        if c2:
            raise ValueError(f"Schema/override/policy collisions in {block}: {', '.join(c2)}")

        out[block] = {**schema[block], **overrides[block], **policies[block]}

    return out


def _literal_member_type_and_value(info: Dict[str, Any]) -> Optional[Tuple[str, Any]]:
    literal = info.get("literal_value")
    if literal is None:
        literal = info.get("stored_value")
    if literal is None:
        return None
    declared = info.get("ir_value_type")
    if isinstance(declared, str):
        if declared in {"String", "string", "std::string"}:
            return ("String", literal) if isinstance(literal, str) else None
        # Preserve declared integral/floating/bool type; enum values may arrive
        # as symbolic strings and should not be downcast to String.
        if declared in INT_TYPES or declared in FLOAT_TYPES or declared == "bool":
            return (declared, literal)
    if isinstance(literal, bool):
        return ("bool", literal)
    if isinstance(literal, int):
        return ("int", literal)
    if isinstance(literal, float):
        return ("double", literal)
    if isinstance(literal, str):
        return ("String", literal)
    return None


def _op_kind_uses_literal(op_kind: str) -> bool:
    return op_kind in {
        "LITERAL_ASSIGN",
        "PRESENCE_ENUM",
        "AUGMENT_ENUM",
    }


def _iter_materialization_entries(schema: Dict[str, Any]):
    def resolve_local_ref(ref: str) -> Any:
        if not ref.startswith("#/"):
            return None
        cur = schema  # type: Any
        for tok in ref[2:].split("/"):
            tok = tok.replace("~1", "/").replace("~0", "~")
            if not isinstance(cur, dict) or tok not in cur:
                return None
            cur = cur[tok]
        return cur

    seen = set()  # type: Set[Tuple[int, Tuple[str, ...]]]

    def walk(node: Any, path_tokens: List[str]):
        if not isinstance(node, dict):
            return
        marker = (id(node), tuple(path_tokens))
        if marker in seen:
            return
        seen.add(marker)

        ref = node.get("$ref")
        if isinstance(ref, str):
            target = resolve_local_ref(ref)
            if isinstance(target, dict):
                yield from walk(target, path_tokens)

        xmat = node.get("x-materialization")
        if isinstance(xmat, list):
            for info in xmat:
                if isinstance(info, dict):
                    yield "/".join(path_tokens), info

        props = node.get("properties")
        if isinstance(props, dict):
            for name, child in props.items():
                yield from walk(child, path_tokens + [name])

        computed = node.get("x-computed-fields")
        if isinstance(computed, dict):
            for name, child in computed.items():
                yield from walk(child, path_tokens + [name])

        items = node.get("items")
        if isinstance(items, dict):
            yield from walk(items, path_tokens)

        for comb in ("oneOf", "anyOf", "allOf"):
            arr = node.get(comb)
            if isinstance(arr, list):
                for child in arr:
                    yield from walk(child, path_tokens)

    yield from walk(schema, [])


def load_instructions_by_block(schema_path: Path) -> Dict[str, Dict[str, List[Dict[str, Any]]]]:
    data = json.loads(schema_path.read_text(encoding="utf-8"))
    out = {b: {} for b in BLOCKS}  # type: Dict[str, Dict[str, List[Dict[str, Any]]]]

    for schema_path_str, info in _iter_materialization_entries(data):
        full = info.get("ir_key") or info.get("pdb_key")
        storage = info.get("storage_type")
        if not isinstance(full, str) or not isinstance(storage, str):
            continue
        try:
            block, local_key = split_pdb_key(full)
        except ValueError:
            continue

        local_path = schema_path_str
        prefix = f"{block}/"
        if local_path.startswith(prefix):
            local_path = local_path[len(prefix):]
        elif local_path == block:
            local_path = ""

        if storage == "TYPE_DATA_COMBINED":
            lit = _literal_member_type_and_value(info)
            out[block].setdefault(local_path, []).append(
                {
                    "op_kind": "LITERAL_ASSIGN",
                    "target_local_ir_key": local_key,
                    "literal_member_type": lit[0] if lit and _op_kind_uses_literal("LITERAL_ASSIGN") else None,
                    "literal_value": lit[1] if lit and _op_kind_uses_literal("LITERAL_ASSIGN") else None,
                    "literal_enum_scope": info.get("enum_scope"),
                }
            )
            arg_full = (
                info.get("secondary_ir_key")
                or info.get("secondary_pdb_key")
                or info.get("argument_ir_key")
                or info.get("argument_pdb_key")
            )
            arg_local_key = local_key
            if isinstance(arg_full, str):
                try:
                    arg_block, arg_local = split_pdb_key(arg_full)
                    if arg_block == block:
                        arg_local_key = arg_local
                except ValueError:
                    pass
            out[block].setdefault(local_path, []).append(
                {
                    "op_kind": "DIRECT_VALUE",
                    "target_local_ir_key": arg_local_key,
                }
            )
        elif storage == "METHOD_PIECEWISE":
            # Decompose method_piecewise custom handler into two primitive writes:
            #   expansion_type = STD_UNIFORM_U
            #   piecewise_basis = true
            lit = _literal_member_type_and_value(info)
            if lit:
                out[block].setdefault(local_path, []).append(
                    {
                        "op_kind": "LITERAL_ASSIGN",
                        "target_local_ir_key": local_key,
                        "literal_member_type": lit[0],
                        "literal_value": lit[1],
                        "literal_enum_scope": info.get("enum_scope"),
                    }
                )

            arg_full = (
                info.get("secondary_ir_key")
                or info.get("secondary_pdb_key")
                or info.get("argument_ir_key")
                or info.get("argument_pdb_key")
            )
            arg_local_key = None
            if isinstance(arg_full, str):
                try:
                    arg_block, arg_local = split_pdb_key(arg_full)
                    if arg_block == block:
                        arg_local_key = arg_local
                except ValueError:
                    pass
            if not arg_local_key:
                arg_local_key = "nond.piecewise_basis"

            arg_member_type = (
                info.get("secondary_ir_value_type")
                or info.get("argument_ir_value_type")
            )
            arg_literal = (
                info.get("secondary_literal_value")
                if "secondary_literal_value" in info
                else info.get("argument_literal_value")
            )
            if arg_member_type is None:
                arg_member_type = "bool"
            if arg_literal is None:
                arg_literal = True
            out[block].setdefault(local_path, []).append(
                {
                    "op_kind": "LITERAL_ASSIGN",
                    "target_local_ir_key": arg_local_key,
                    "literal_member_type": arg_member_type,
                    "literal_value": arg_literal,
                    "literal_enum_scope": (
                        info.get("secondary_enum_scope")
                        if "secondary_enum_scope" in info
                        else info.get("argument_enum_scope")
                    ),
                }
            )
        else:
            lit = _literal_member_type_and_value(info)
            op_kind = (
                "LITERAL_ASSIGN"
                if storage in {"PRESENCE_LITERAL", "TYPE_DATA"}
                else storage
            )
            uses_lit = _op_kind_uses_literal(op_kind)
            op = {
                "op_kind": op_kind,
                "target_local_ir_key": local_key,
                "ir_value_type": info.get("ir_value_type"),
                "literal_member_type": lit[0] if lit and uses_lit else None,
                "literal_value": lit[1] if lit and uses_lit else None,
                "literal_enum_scope": info.get("enum_scope") if lit and uses_lit else None,
            }
            out[block].setdefault(local_path, []).append(op)

    for block in BLOCKS:
        out[block] = {k: out[block][k] for k in sorted(out[block].keys())}
    return out


def _cpp_json_expr(value: Any) -> str:
    txt = json.dumps(value, sort_keys=True)
    txt = txt.replace("\\", "\\\\").replace('"', '\\"')
    return f'nlohmann::json::parse("{txt}")'


def synthesize_missing_contracts_from_instructions(
    contracts_by_block: Dict[str, Dict[str, KeyContract]],
    instructions_by_block: Dict[str, Dict[str, List[Dict[str, Any]]]],
) -> None:
    """Add contracts for write targets that are instruction-only IR keys.

    Some IR keys, especially derived variable count rollups like
    ``variables.continuous`` and ``variables.total``, do not correspond to a
    standalone schema field but are still legal write targets. When schema,
    override, and policy registries do not define them explicitly, synthesize a
    policy-style contract from the instruction materialization type.
    """

    for block in BLOCKS:
        contracts = contracts_by_block[block]
        for ops in instructions_by_block[block].values():
            for op in ops:
                local_key = op.get("target_local_ir_key")
                if not isinstance(local_key, str) or local_key in contracts:
                    continue

                member_type = op.get("ir_value_type") or op.get("literal_member_type")
                if not isinstance(member_type, str) or not member_type:
                    continue

                full_ir_key = f"{block}.{local_key}"
                contracts[local_key] = KeyContract(
                    full_ir_key=full_ir_key,
                    local_ir_key=local_key,
                    member_variable_type=member_type,
                    value_type_tag=type_tag(member_type),
                    default_kind="policy",
                    default_value=policy_default_for_type(member_type),
                    enum_scope="",
                    source="instruction_inferred",
                )


def _cpp_str(value: str) -> str:
    s = value.replace("\\", "\\\\").replace('"', '\\"')
    return f'"{s}"'


def _cpp_symbolic_expr(value: str) -> Optional[str]:
    text = value.strip()
    if not text:
        return None

    # C/C++ symbolic constants and scoped identifiers, optionally negated.
    if re.fullmatch(r"-?(?:[A-Za-z_][A-Za-z0-9_]*)(?:::[A-Za-z_][A-Za-z0-9_]*)*", text):
        return text

    # Common standard-library symbolic expressions used in defaults.
    if re.fullmatch(r"-?std::numeric_limits<[^>]+>::(?:max|min|lowest|infinity)\(\)", text):
        return text

    return None


def _cpp_default_variant_expr(member_type: str, value: Any, enum_scope: str = "") -> str:
    t = " ".join(member_type.split())
    if value is None:
        return "IRValue{std::monostate{}}"
    if isinstance(value, bool):
        return "IRValue{bool(" + ("true" if value else "false") + ")}"
    if isinstance(value, str):
        symbolic_value = _cpp_symbolic_expr(value)
        # Symbolic literals/defaults are stored as strings in JSON registries,
        # but for non-string IR member types they represent C++ expressions.
        if t in INT_TYPES and symbolic_value:
            scoped_value = (
                f"{enum_scope}::{symbolic_value}"
                if enum_scope and "::" not in symbolic_value and not symbolic_value.startswith("-")
                else symbolic_value
            )
            if t == "short":
                return f"IRValue{{static_cast<short>({scoped_value})}}"
            if t == "unsigned short":
                return f"IRValue{{static_cast<unsigned short>({scoped_value})}}"
            if t == "size_t":
                return f"IRValue{{static_cast<size_t>({scoped_value})}}"
            if t == "unsigned int":
                return f"IRValue{{static_cast<unsigned int>({scoped_value})}}"
            if t == "long":
                return f"IRValue{{static_cast<long>({scoped_value})}}"
            if t == "unsigned long":
                return f"IRValue{{static_cast<unsigned long>({scoped_value})}}"
            if t == "long long":
                return f"IRValue{{static_cast<long long>({scoped_value})}}"
            if t == "unsigned long long":
                return f"IRValue{{static_cast<unsigned long long>({scoped_value})}}"
            return f"IRValue{{int({scoped_value})}}"
        if t in FLOAT_TYPES and symbolic_value:
            cast_type = "float" if t == "float" else "double"
            return f"IRValue{{{cast_type}({symbolic_value})}}"
        return f"IRValue{{std::string({_cpp_str(value)})}}"
    if isinstance(value, int):
        if t == "short":
            return f"IRValue{{short({value})}}"
        if t == "unsigned short":
            return f"IRValue{{static_cast<unsigned short>({value})}}"
        if t == "size_t":
            return f"IRValue{{static_cast<size_t>({value})}}"
        if t == "unsigned int":
            return f"IRValue{{static_cast<unsigned int>({value})}}"
        if t == "long":
            return f"IRValue{{static_cast<long>({value})}}"
        if t == "unsigned long":
            return f"IRValue{{static_cast<unsigned long>({value})}}"
        if t == "long long":
            return f"IRValue{{static_cast<long long>({value})}}"
        if t == "unsigned long long":
            return f"IRValue{{static_cast<unsigned long long>({value})}}"
        return f"IRValue{{int({value})}}"
    if isinstance(value, float):
        return f"IRValue{{double({repr(value)})}}"
    if (isinstance(value, list) and not value) or (isinstance(value, dict) and not value):
        # Policy defaults for Dakota container/matrix types should materialize
        # as their native C++ value types, not as nlohmann::json placeholders.
        if t not in INT_TYPES and t not in FLOAT_TYPES and t not in {"bool", "String", "std::string", "string"}:
            return f"IRValue{{{t}{{}}}}"
    return f"IRValue{{{_cpp_json_expr(value)}}}"


def _cpp_literal_expr(op: Dict[str, Any]) -> str:
    member_type = op.get("literal_member_type")
    value = op.get("literal_value")
    enum_scope = op.get("literal_enum_scope")
    if not isinstance(member_type, str) or value is None:
        return "OpLiteral{IrValueType::UnspecifiedType, IRValue{std::monostate{}}}"
    return (
        "OpLiteral{IrValueType::"
        + _enum_ident_from_member_type(member_type)
        + ", "
        + _cpp_default_variant_expr(
            member_type,
            value,
            enum_scope if isinstance(enum_scope, str) else "",
        )
        + "}"
    )


def _enum_ident_from_member_type(member_type: str) -> str:
    parts = []  # type: List[str]
    token = []  # type: List[str]
    for ch in member_type:
        if ch.isalnum():
            token.append(ch)
        else:
            if token:
                parts.append("".join(token))
                token = []
    if token:
        parts.append("".join(token))
    if not parts:
        return "Unknown"
    ident = "".join(p[:1].upper() + p[1:] for p in parts)
    if ident[0].isdigit():
        ident = "T" + ident
    return ident


def _enum_ident(text: str) -> str:
    parts = []  # type: List[str]
    token = []  # type: List[str]
    for ch in text:
        if ch.isalnum():
            token.append(ch)
        else:
            if token:
                parts.append("".join(token))
                token = []
    if token:
        parts.append("".join(token))
    if not parts:
        return "Unknown"
    norm_parts = []  # type: List[str]
    for p in parts:
        if p.isupper() or p.islower():
            norm_parts.append(p[:1].upper() + p[1:].lower())
        else:
            norm_parts.append(p[:1].upper() + p[1:])
    ident = "".join(norm_parts)
    if ident[0].isdigit():
        ident = "T" + ident
    return ident


def _cpp_ir_type_token(member_type: str) -> str:
    t = " ".join(member_type.split())
    if t in {"bool", "int", "short", "unsigned short", "size_t", "double", "float"}:
        return t
    if t == "std::string":
        return t
    if t == "string":
        return "std::string"
    return f"Dakota::{t}"


def load_all_storage_types(schema_path: Path) -> Set[str]:
    data = json.loads(schema_path.read_text(encoding="utf-8"))
    out = set()  # type: Set[str]

    def walk(node: Any):
        if isinstance(node, dict):
            yield node
            for v in node.values():
                yield from walk(v)
        elif isinstance(node, list):
            for v in node:
                yield from walk(v)

    for obj in walk(data):
        if not isinstance(obj, dict):
            continue
        xmat = obj.get("x-materialization")
        if not isinstance(xmat, list):
            continue
        for info in xmat:
            if not isinstance(info, dict):
                continue
            st = info.get("storage_type")
            if isinstance(st, str) and st:
                out.add(st)

    # Composite storage types are decomposed into primitive ops.
    out.discard("TYPE_DATA")
    out.discard("TYPE_DATA_COMBINED")
    out.discard("METHOD_PIECEWISE")
    out.discard("PRESENCE_LITERAL")
    out.add("LITERAL_ASSIGN")
    return out


def render_types_hpp(member_types: List[str], op_kinds: List[str]) -> str:
    lines = []  # type: List[str]
    lines.append("// Auto-generated by update_pdb_keys/generate_ir_tables.py")
    sig_payload = {"member_types": member_types, "op_kinds": op_kinds}
    sig = hashlib.sha256(json.dumps(sig_payload, sort_keys=True).encode("utf-8")).hexdigest()
    lines.append(f"// type-signature: {sig}")
    lines.append("#pragma once")
    lines.append("")
    lines.append("#include <string>")
    lines.append("#include <unordered_map>")
    lines.append("#include <variant>")
    lines.append("#include <vector>")
    lines.append("#include <nlohmann/json.hpp>")
    lines.append('#include "dakota_data_types.hpp"')
    lines.append("")
    lines.append("namespace dakota::irgen {")
    lines.append("")
    lines.append("enum class IrValueType {")
    lines.append("  UnspecifiedType,")
    for mt in member_types:
        lines.append(f"  {_enum_ident_from_member_type(mt)},")
    lines.append("};")
    lines.append("")
    lines.append("enum class OpKind {")
    for op in op_kinds:
        lines.append(f"  {_enum_ident(op)},")
    lines.append("};")
    lines.append("")
    ir_alts = ["std::monostate"]
    for mt in member_types:
        tok = _cpp_ir_type_token(mt)
        if tok not in ir_alts:
            ir_alts.append(tok)
    if "nlohmann::json" not in ir_alts:
        ir_alts.append("nlohmann::json")
    lines.append("using IRValue = std::variant<" + ", ".join(ir_alts) + ">;")
    lines.append("")
    lines.append("} // namespace dakota::irgen")
    lines.append("")
    return "\n".join(lines)


def render_table_types_hpp() -> str:
    lines = []  # type: List[str]
    lines.append("// Stable table metadata types. Keep this header minimal and non-derived.")
    lines.append("#pragma once")
    lines.append("")
    lines.append("#include <string>")
    lines.append("#include <unordered_map>")
    lines.append("#include <vector>")
    lines.append('#include "generated_ir_types.hpp"')
    lines.append("")
    lines.append("namespace dakota::irgen {")
    lines.append("")
    lines.append("enum class BlockType { Environment, Method, Model, Variables, Interface, Responses };")
    lines.append("")
    lines.append("enum class DefaultSource { Policy, Schema, Override };")
    lines.append("")
    lines.append("struct KeyContract {")
    lines.append("  IrValueType ir_value_type;")
    lines.append("  DefaultSource default_source;")
    lines.append("  IRValue default_value;")
    lines.append("};")
    lines.append("")
    lines.append("struct OpLiteral {")
    lines.append("  IrValueType ir_value_type;")
    lines.append("  IRValue value;")
    lines.append("};")
    lines.append("")
    lines.append("struct WriteOp {")
    lines.append("  OpKind op_kind;")
    lines.append("  std::string target_local_ir_key;")
    lines.append("  OpLiteral literal;")
    lines.append("};")
    lines.append("")
    lines.append("struct BlockTables {")
    lines.append("  const std::unordered_map<std::string, KeyContract>& contracts;")
    lines.append("  const std::unordered_map<std::string, std::vector<WriteOp>>& instructions;")
    lines.append("};")
    lines.append("")
    lines.append("namespace environment {")
    lines.append("const std::unordered_map<std::string, KeyContract>& contracts();")
    lines.append("const std::unordered_map<std::string, std::vector<WriteOp>>& instructions();")
    lines.append("}")
    lines.append("namespace method {")
    lines.append("const std::unordered_map<std::string, KeyContract>& contracts();")
    lines.append("const std::unordered_map<std::string, std::vector<WriteOp>>& instructions();")
    lines.append("}")
    lines.append("namespace model {")
    lines.append("const std::unordered_map<std::string, KeyContract>& contracts();")
    lines.append("const std::unordered_map<std::string, std::vector<WriteOp>>& instructions();")
    lines.append("}")
    lines.append("namespace variables {")
    lines.append("const std::unordered_map<std::string, KeyContract>& contracts();")
    lines.append("const std::unordered_map<std::string, std::vector<WriteOp>>& instructions();")
    lines.append("}")
    lines.append("namespace interface {")
    lines.append("const std::unordered_map<std::string, KeyContract>& contracts();")
    lines.append("const std::unordered_map<std::string, std::vector<WriteOp>>& instructions();")
    lines.append("}")
    lines.append("namespace responses {")
    lines.append("const std::unordered_map<std::string, KeyContract>& contracts();")
    lines.append("const std::unordered_map<std::string, std::vector<WriteOp>>& instructions();")
    lines.append("}")
    lines.append("")
    lines.append("const BlockTables& tables_for_block(BlockType block);")
    lines.append("")
    lines.append("} // namespace dakota::irgen")
    lines.append("")
    return "\n".join(lines)


def _extract_type_signature(header_text: str) -> str:
    for ln in header_text.splitlines():
        if ln.startswith("// type-signature: "):
            return ln.split(": ", 1)[1].strip()
    return ""


def render_registry_cpp() -> str:
    lines = []  # type: List[str]
    lines.append("// Auto-generated by update_pdb_keys/generate_ir_tables.py")
    lines.append('#include "generated_ir_table_types.hpp"')
    lines.append("")
    lines.append("namespace dakota::irgen {")
    lines.append("")
    lines.append("const BlockTables& tables_for_block(BlockType block) {")
    lines.append("  switch (block) {")
    lines.append("    case BlockType::Environment: {")
    lines.append("      static const BlockTables t{environment::contracts(), environment::instructions()};")
    lines.append("      return t;")
    lines.append("    }")
    lines.append("    case BlockType::Method: {")
    lines.append("      static const BlockTables t{method::contracts(), method::instructions()};")
    lines.append("      return t;")
    lines.append("    }")
    lines.append("    case BlockType::Model: {")
    lines.append("      static const BlockTables t{model::contracts(), model::instructions()};")
    lines.append("      return t;")
    lines.append("    }")
    lines.append("    case BlockType::Variables: {")
    lines.append("      static const BlockTables t{variables::contracts(), variables::instructions()};")
    lines.append("      return t;")
    lines.append("    }")
    lines.append("    case BlockType::Interface: {")
    lines.append("      static const BlockTables t{interface::contracts(), interface::instructions()};")
    lines.append("      return t;")
    lines.append("    }")
    lines.append("    case BlockType::Responses: {")
    lines.append("      static const BlockTables t{responses::contracts(), responses::instructions()};")
    lines.append("      return t;")
    lines.append("    }")
    lines.append("  }")
    lines.append("  static const BlockTables fallback{environment::contracts(), environment::instructions()};")
    lines.append("  return fallback;")
    lines.append("}")
    lines.append("")
    lines.append("} // namespace dakota::irgen")
    lines.append("")
    return "\n".join(lines)


def render_block_cpp(
    block: str,
    contracts: Dict[str, KeyContract],
    instructions: Dict[str, List[Dict[str, Any]]],
) -> str:
    ns = f"dakota::irgen::{block}"
    def _default_source_enum(v: str) -> str:
        m = {"policy": "Policy", "schema": "Schema", "override": "Override"}
        return m.get(v, "Policy")

    lines = []  # type: List[str]
    lines.append("// Auto-generated by update_pdb_keys/generate_ir_tables.py")
    lines.append(f"// Block: {block}")
    lines.append('#include "generated_ir_table_types.hpp"')
    # Enum symbols referenced by defaults/literals come from these headers.
    block_enum_headers = {
        "environment": ["DakotaGlobalEnums.hpp"],
        "method": ["DakotaMethodEnums.hpp", "DakotaModelEnums.hpp", "globals.h"],
        "model": ["DakotaModelEnums.hpp", "DakotaMethodEnums.hpp"],
        "variables": ["DakotaVariablesEnums.hpp", "DakotaGlobalEnums.hpp"],
        # interface tables also use scheduling enums from method enums.
        "interface": ["DakotaInterfaceEnums.hpp", "DakotaMethodEnums.hpp"],
        "responses": ["DakotaGlobalEnums.hpp"],
    }
    for hdr in block_enum_headers.get(block, []):
        lines.append(f'#include "{hdr}"')
    lines.append("")
    lines.append(f"namespace {ns} {{")
    lines.append("using namespace Dakota;")
    lines.append("")
    lines.append("const std::unordered_map<std::string, KeyContract>& contracts() {")
    lines.append("  static const auto kContracts = []() {")
    lines.append("    std::unordered_map<std::string, KeyContract> m;")
    lines.append(f"    m.reserve({len(contracts)});")
    for local in sorted(contracts.keys()):
        c = contracts[local]
        lines.append(
            "    m.emplace("
            + _cpp_str(local)
            + ", KeyContract{"
            + "IrValueType::"
            + _enum_ident_from_member_type(c.member_variable_type)
            + ", "
            + "DefaultSource::"
            + _default_source_enum(c.default_kind)
            + ", "
            + _cpp_default_variant_expr(c.member_variable_type, c.default_value, c.enum_scope)
            + "});"
        )
    lines.append("    return m;")
    lines.append("  }();")
    lines.append("  return kContracts;")
    lines.append("}")
    lines.append("")
    lines.append("const std::unordered_map<std::string, std::vector<WriteOp>>& instructions() {")
    lines.append("  static const auto kInstructions = []() {")
    lines.append("    std::unordered_map<std::string, std::vector<WriteOp>> m;")
    lines.append(f"    m.reserve({len(instructions)});")
    for path, ops in instructions.items():
        lines.append(f"    m.emplace({_cpp_str(path)}, std::vector<WriteOp>{{")
        for op in ops:
            lines.append(
                "      WriteOp{"
                + "OpKind::"
                + _enum_ident(op["op_kind"])
                + ", "
                + _cpp_str(op["target_local_ir_key"])
                + ", "
                + _cpp_literal_expr(op)
                + "},"
            )
        lines.append("    });")
    lines.append("    return m;")
    lines.append("  }();")
    lines.append("  return kInstructions;")
    lines.append("}")
    lines.append("")
    lines.append(f"}} // namespace {ns}")
    lines.append("")
    return "\n".join(lines)


def main() -> int:
    ap = argparse.ArgumentParser(description="Generate block-specific IR tables")
    ap.add_argument("--schema-defaults", required=True, help="schema defaults JSON")
    ap.add_argument("--override-registry", required=True, help="default_overrides_registry.json")
    ap.add_argument("--policy-registry", required=True, help="default_policy_registry.json")
    ap.add_argument("--schema", required=True, help="dakota_study_schema.json")
    ap.add_argument(
        "--output-dir",
        default="update_pdb_keys/generated_ir_tables",
        help="directory for generated artifacts",
    )
    args = ap.parse_args()

    schema = load_schema_defaults(Path(args.schema_defaults))
    overrides = load_overrides(Path(args.override_registry))
    policies = load_policies(Path(args.policy_registry))
    merged = merge_contracts(schema, overrides, policies)
    instructions = load_instructions_by_block(Path(args.schema))
    synthesize_missing_contracts_from_instructions(merged, instructions)
    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    all_member_types = sorted(
        {
            c.member_variable_type
            for b in BLOCKS
            for c in merged[b].values()
            if isinstance(c.member_variable_type, str) and c.member_variable_type
        }
    )
    all_storage_types = load_all_storage_types(Path(args.schema))
    all_op_kinds = sorted(
        set(all_storage_types)
        | {
            op["op_kind"]
            for b in BLOCKS
            for _path, ops in instructions[b].items()
            for op in ops
            if isinstance(op.get("op_kind"), str) and op.get("op_kind")
        }
    )
    types_header_path = out_dir / "generated_ir_types.hpp"
    new_types_header = render_types_hpp(all_member_types, all_op_kinds)
    should_write_types_header = True
    if types_header_path.exists():
        old_text = types_header_path.read_text(encoding="utf-8")
        old_sig = _extract_type_signature(old_text)
        new_sig = _extract_type_signature(new_types_header)
        if old_sig and new_sig and old_sig == new_sig and old_text == new_types_header:
            should_write_types_header = False
            print(f"Unchanged types header signature; keeping existing: {types_header_path}")
    if should_write_types_header:
        types_header_path.write_text(new_types_header, encoding="utf-8")
        print(f"Wrote: {types_header_path}")
    (out_dir / "generated_ir_registry.cpp").write_text(render_registry_cpp(), encoding="utf-8")
    print(f"Wrote: {out_dir / 'generated_ir_registry.cpp'}")

    for block in BLOCKS:
        cpp = render_block_cpp(block, merged[block], instructions[block])
        cpp_path = out_dir / f"generated_ir_{block}.cpp"
        cpp_path.write_text(cpp, encoding="utf-8")
        print(f"Wrote: {cpp_path} (contracts={len(merged[block])}, instruction_paths={len(instructions[block])})")

    total_contracts = sum(len(merged[b]) for b in BLOCKS)
    total_paths = sum(len(instructions[b]) for b in BLOCKS)
    print(f"Total contracts: {total_contracts}")
    print(f"Total instruction paths: {total_paths}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
