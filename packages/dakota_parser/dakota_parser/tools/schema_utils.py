"""
schema_utils.py — Shared utilities for Dakota JSON Schema processing.

Provides common schema traversal, $ref resolution, and type checking used
by all code generators (grammar, AST metadata, validation, default expansion).
"""

from typing import Any, Dict, Iterator, List, Optional, Set, Tuple


# =============================================================================
# $ref resolution
# =============================================================================

def ref_name(ref: str) -> str:
    """Extract definition name from a $ref string.  '#/$defs/Foo' → 'Foo'."""
    return ref[8:] if ref.startswith("#/$defs/") else ""


def first_ref(schema: dict) -> str:
    """Get the first $ref definition name from a schema (direct or anyOf).
    
    Returns '' if no $ref found.
    """
    if "$ref" in schema:
        return ref_name(schema["$ref"])
    for opt in schema.get("anyOf", []):
        if "$ref" in opt:
            return ref_name(opt["$ref"])
    return ""


def all_refs(schema: dict) -> List[str]:
    """Get all $ref definition names from a schema's anyOf (excluding null)."""
    if "$ref" in schema:
        name = ref_name(schema["$ref"])
        return [name] if name else []
    return [ref_name(opt["$ref"]) for opt in schema.get("anyOf", [])
            if "$ref" in opt and ref_name(opt["$ref"])]


def resolve(defs: dict, ref: str) -> Optional[dict]:
    """Resolve a $ref string to its definition dict.  Returns None if not found."""
    name = ref_name(ref) if ref.startswith("#") else ref
    return defs.get(name)


# =============================================================================
# Type checking
# =============================================================================

def _has_type_or_anyof(schema: dict, type_name: str) -> bool:
    """Check if schema has a given type directly or in anyOf."""
    if schema.get("type") == type_name:
        return True
    return any(opt.get("type") == type_name for opt in schema.get("anyOf", []))


def is_boolean(schema: dict) -> bool:
    """Check if field is boolean type (including const bool)."""
    if _has_type_or_anyof(schema, "boolean"):
        return True
    if "const" in schema and isinstance(schema["const"], bool):
        return True
    return False


def is_array(schema: dict) -> bool:
    """Check if field is array type."""
    return _has_type_or_anyof(schema, "array")


def is_anchor(schema: dict) -> bool:
    """Check if field is an anchor (transparent wrapper in the DSL)."""
    return schema.get("anchor") is True


def is_number(schema: dict) -> bool:
    """Check if field is number type, including array items of number type."""
    if _has_type_or_anyof(schema, "number"):
        return True
    # Check items type for arrays
    for s in [schema] + schema.get("anyOf", []):
        if s.get("type") == "array":
            if s.get("items", {}).get("type") == "number":
                return True
    return False


def is_integer(schema: dict) -> bool:
    """Check if field is integer type, including array items of integer type."""
    if _has_type_or_anyof(schema, "integer"):
        return True
    for s in [schema] + schema.get("anyOf", []):
        if s.get("type") == "array":
            if s.get("items", {}).get("type") == "integer":
                return True
    return False


def is_direct_number(schema: dict) -> bool:
    """Check if field is directly a number type (not via array items)."""
    return _has_type_or_anyof(schema, "number")


def array_item_type(schema: dict) -> Optional[str]:
    """Get the item type for an array field, or None if not an array."""
    if schema.get("type") == "array":
        return schema.get("items", {}).get("type")
    for opt in schema.get("anyOf", []):
        if opt.get("type") == "array":
            return opt.get("items", {}).get("type")
    return None


# =============================================================================
# Anchor / union variant traversal
# =============================================================================

def anchor_children(defs: dict, anchor_schema: dict) -> Set[str]:
    """Extract all child property names from an anchor's variant definitions."""
    children = set()
    for ref_str in all_refs(anchor_schema):
        defn = defs.get(ref_str, {})
        children.update(defn.get("properties", {}).keys())
    if not children and "$ref" in anchor_schema:
        defn = defs.get(ref_name(anchor_schema["$ref"]), {})
        children.update(defn.get("properties", {}).keys())
    return children


def iter_variant_children(
    defs: dict, schema: dict
) -> Iterator[Tuple[str, str, dict]]:
    """Iterate children of all anyOf variants in a schema.
    
    Yields (variant_def_name, child_prop_name, child_prop_schema) for each
    property in each variant definition referenced by the schema's anyOf.
    """
    for vref in all_refs(schema):
        vdef = defs.get(vref, {})
        for child_name, child_schema in vdef.get("properties", {}).items():
            yield vref, child_name, child_schema


# =============================================================================
# Block-level schema helpers
# =============================================================================

def collect_block_refs(block_schema: dict) -> Tuple[List[str], bool]:
    """Collect all $ref strings from a top-level block schema, handling items nesting.
    
    Returns (list_of_full_ref_strings, is_array).
    """
    refs: List[str] = []
    is_array = False
    
    def _collect_from_items(items):
        nonlocal is_array
        is_array = True
        if '$ref' in items:
            refs.append(items['$ref'])
        elif 'anyOf' in items:
            for opt in items['anyOf']:
                if '$ref' in opt:
                    refs.append(opt['$ref'])
    
    if 'anyOf' in block_schema:
        for option in block_schema['anyOf']:
            if '$ref' in option:
                refs.append(option['$ref'])
            elif 'items' in option:
                _collect_from_items(option['items'])
    elif 'items' in block_schema:
        _collect_from_items(block_schema['items'])
    elif '$ref' in block_schema:
        refs.append(block_schema['$ref'])
    
    return refs, is_array
