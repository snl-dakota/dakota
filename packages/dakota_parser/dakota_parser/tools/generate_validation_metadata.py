#!/usr/bin/env python3
"""
Generate validation metadata from Dakota JSON Schema.

Reads the JSON Schema and produces a C++ header that maps each definition
to its validation rules:

1. Standard JSON Schema numeric constraints (minimum, exclusiveMinimum, 
   maximum, exclusiveMaximum) on individual properties
2. Dakota x-model-validations (cross-field validation rules)
3. Dakota x-computed-fields (post-validation computed values)
4. Schema walk map for recursive JSON document validation

The generated header provides:
  - validate_definition(def_name, instance) -> mutated instance
  - validate_json_document(doc) -> validated document with error collection
  - get_numeric_constraints(def_name) -> field-level bounds
  - get_model_validations(def_name) -> x-model-validations JSON array
  - get_computed_fields(def_name) -> x-computed-fields metadata
  - get_schema_walk_map() -> definition property-to-child-definition map

At runtime, the DSL reader calls validate_json_document() after ast_to_json.
This walks the JSON tree using the schema structure and dispatches to:
  1. Inline numeric constraint checks (generated)
  2. ValidatorRegistry::validate_all() (library, via x-model-validations)
"""

import json
import sys
import argparse
from pathlib import Path
from typing import Dict, List, Any, Optional, Tuple, Set
from collections import defaultdict

import schema_utils as su


class ValidationMetadataGenerator:
    """Extract and generate validation metadata from JSON Schema."""

    def __init__(self, schema_path: str):
        with open(schema_path, encoding='utf-8') as f:
            self.schema = json.load(f)
        self.defs = self.schema.get('$defs', {})

        # Extracted data
        self.numeric_constraints: Dict[str, List[Tuple[str, str, float]]] = {}
        self.model_validations: Dict[str, List[dict]] = {}
        self.computed_fields: Dict[str, dict] = {}
        self.property_type_specs: Dict[str, Dict[str, List[dict]]] = {}
        self.float_fields: Dict[str, Dict[str, bool]] = {}
        self.schema_walk_map: Dict[str, Dict[str, str]] = {}
        self.synthetic_allowed: Dict[str, Set[str]] = {}
        self.block_definitions: Dict[str, Tuple[str, bool, bool]] = {}
        self.union_block_variants: Dict[str, List[str]] = {}
        self.required_blocks: Set[str] = set(self.schema.get('required', []))

    def process_all(self):
        """Scan all definitions and extract validation metadata."""
        for def_name, defn in self.defs.items():
            self._extract_numeric_constraints(def_name, defn)
            self._extract_model_validations(def_name, defn)
            self._extract_computed_fields(def_name, defn)
            self._extract_property_type_specs(def_name, defn)
            self._extract_float_fields(def_name, defn)
            self._extract_child_refs(def_name, defn)

        self._extract_block_definitions()

    def _extract_numeric_constraints(self, def_name: str, defn: dict):
        constraints = []
        for field_name, field_schema in defn.get('properties', {}).items():
            for op, bound in self._get_field_numeric_constraints(field_schema):
                constraints.append((field_name, op, bound))
        if constraints:
            self.numeric_constraints[def_name] = constraints

    def _get_field_numeric_constraints(self, field_schema: dict) -> List[Tuple[str, float]]:
        results = []
        constraint_map = {
            'minimum': 'GE', 'exclusiveMinimum': 'GT',
            'maximum': 'LE', 'exclusiveMaximum': 'LT',
        }
        for json_key, op in constraint_map.items():
            if json_key in field_schema:
                results.append((op, float(field_schema[json_key])))
        for variant in field_schema.get('anyOf', []):
            if variant.get('type') == 'null':
                continue
            for json_key, op in constraint_map.items():
                if json_key in variant:
                    results.append((op, float(variant[json_key])))
        return results

    def _extract_model_validations(self, def_name: str, defn: dict):
        validations = defn.get('x-model-validations', [])
        if validations:
            self.model_validations[def_name] = validations

    def _extract_computed_fields(self, def_name: str, defn: dict):
        computed = defn.get('x-computed-fields', {})
        if computed:
            self.computed_fields[def_name] = computed

    def _extract_property_type_specs(self, def_name: str, defn: dict):
        specs: Dict[str, List[dict]] = {}
        for field_name, field_schema in defn.get('properties', {}).items():
            variants: List[dict] = []
            candidates = field_schema.get('anyOf', [field_schema])
            for opt in candidates:
                spec: Dict[str, Any] = {}
                if '$ref' in opt:
                    spec['type'] = 'object'
                elif 'type' in opt:
                    spec['type'] = opt['type']
                elif 'const' in opt:
                    const_val = opt['const']
                    if const_val is None:
                        spec['type'] = 'null'
                    elif isinstance(const_val, bool):
                        spec['type'] = 'boolean'
                    elif isinstance(const_val, int) and not isinstance(const_val, bool):
                        spec['type'] = 'integer'
                    elif isinstance(const_val, float):
                        spec['type'] = 'number'
                    elif isinstance(const_val, str):
                        spec['type'] = 'string'
                if 'const' in opt:
                    spec['const'] = opt['const']
                if spec:
                    variants.append(spec)
            if variants:
                specs[field_name] = variants
        if specs:
            self.property_type_specs[def_name] = specs

    def _extract_float_fields(self, def_name: str, defn: dict):
        fields: Dict[str, bool] = {}
        for field_name, field_schema in defn.get('properties', {}).items():
            field_kind = self._get_float_field_kind(field_schema)
            if field_kind is not None:
                fields[field_name] = (field_kind == 'array')
        if fields:
            self.float_fields[def_name] = fields

    def _get_float_field_kind(self, field_schema: dict) -> Optional[str]:
        if field_schema.get('type') == 'number':
            return 'scalar'
        if (field_schema.get('type') == 'array'
                and field_schema.get('items', {}).get('type') == 'number'):
            return 'array'

        for variant in field_schema.get('anyOf', []):
            if variant.get('type') == 'null':
                continue
            variant_kind = self._get_float_field_kind(variant)
            if variant_kind is not None:
                return variant_kind
        return None

    def _extract_child_refs(self, def_name: str, defn: dict):
        children = {}
        for prop_name, prop_schema in defn.get('properties', {}).items():
            child_def = self._register_synthetic_schema(def_name, prop_name, prop_schema)
            if child_def:
                children[prop_name] = child_def
        if children:
            self.schema_walk_map[def_name] = children

    def _synthetic_key(self, parent_def: str, prop_name: str) -> str:
        return parent_def + "__" + prop_name

    def _register_synthetic_schema(self, parent_def: str, prop_name: str, prop_schema: dict):
        refs = su.all_refs(prop_schema)
        if not su.is_anchor(prop_schema) and len(refs) <= 1:
            child_def = su.first_ref(prop_schema)
            if child_def and child_def in self.defs:
                return child_def
            return ""

        synthetic_def_key = self._synthetic_key(parent_def, prop_name)

        synthetic_children = {}
        synthetic_allowed = set()
        for _, vprop_name, vprop_schema in su.iter_variant_children(self.defs, prop_schema):
            synthetic_allowed.add(vprop_name)
            nested_def = self._register_synthetic_schema(synthetic_def_key, vprop_name, vprop_schema)
            if nested_def:
                synthetic_children[vprop_name] = nested_def

        if synthetic_allowed:
            self.synthetic_allowed[synthetic_def_key] = synthetic_allowed
        if synthetic_children:
            self.schema_walk_map[synthetic_def_key] = synthetic_children
        return synthetic_def_key if synthetic_allowed else ""

    def _extract_block_definitions(self):
        for block_name, block_schema in self.schema.get('properties', {}).items():
            if '$ref' in block_schema:
                self.block_definitions[block_name] = (su.ref_name(block_schema['$ref']), False, False)
                continue

            # Helper: classify items schema as (config_type, is_union, is_array=True)
            def classify_items(items_schema):
                refs = su.all_refs(items_schema) if 'anyOf' in items_schema else []
                if '$ref' in items_schema:
                    return (su.ref_name(items_schema['$ref']), False, True, [])
                elif len(refs) > 1:
                    return ('', True, True, refs)
                elif len(refs) == 1:
                    return (refs[0], False, True, [])
                return None

            items = block_schema.get('items', {})
            if items:
                result = classify_items(items)
                if result:
                    self.block_definitions[block_name] = result[:3]
                    if result[1] and result[3]:
                        self.union_block_variants[block_name] = result[3]
                continue

            # Nullable patterns via anyOf
            for ao in block_schema.get('anyOf', []):
                if ao.get('type') == 'null':
                    continue
                if '$ref' in ao:
                    self.block_definitions[block_name] = (su.ref_name(ao['$ref']), False, False)
                    break
                items2 = ao.get('items', {})
                if items2:
                    result = classify_items(items2)
                    if result:
                        self.block_definitions[block_name] = result[:3]
                        if result[1] and result[3]:
                            self.union_block_variants[block_name] = result[3]
                    break

    # =========================================================================
    # Code Generation
    # =========================================================================

    def generate_header(self) -> str:
        lines = []
        lines.append(self._generate_preamble())
        lines.append(self._generate_numeric_constraints_function())
        lines.append(self._generate_model_validations_function())
        lines.append(self._generate_computed_fields_function())
        lines.append(self._generate_property_type_specs_function())
        lines.append(self._generate_allowed_properties_function())
        lines.append(self._generate_top_level_blocks_function())
        lines.append(self._generate_schema_walk_map_function())
        lines.append(self._generate_block_definitions_function())
        lines.append(self._generate_union_block_selector_function())
        lines.append(self._generate_internal_only_defaults_function())
        lines.append(self._generate_float_field_coercion_function())
        lines.append(self._generate_integer_field_coercion_function())
        lines.append(self._generate_validate_definition_function())
        lines.append(self._generate_validate_json_document_function())
        return '\n'.join(lines)

    def _generate_property_type_specs_function(self) -> str:
        lines = []
        lines.append("// ============================================================================")
        lines.append("// Property type/const specs per definition")
        lines.append("// ============================================================================")
        lines.append("")
        lines.append("inline const json& get_property_type_specs(const std::string& def_name) {")
        lines.append("    static const std::map<std::string, json> data = []() {")
        lines.append("        std::map<std::string, json> m;")

        for def_name in sorted(self.property_type_specs.keys()):
            json_str = json.dumps(self.property_type_specs[def_name], separators=(',', ':'))
            lines.append(f'        m["{def_name}"] = json::parse(R"__pts__({json_str})__pts__");')

        lines.append("        return m;")
        lines.append("    }();")
        lines.append("    static const json empty = json::object();")
        lines.append("    auto it = data.find(def_name);")
        lines.append("    return it != data.end() ? it->second : empty;")
        lines.append("}")
        lines.append("")
        return '\n'.join(lines)

    def _generate_allowed_properties_function(self) -> str:
        allowed: Dict[str, Set[str]] = {}

        for def_name, defn in self.defs.items():
            props = set(defn.get('properties', {}).keys())
            if props:
                allowed[def_name] = props

        for synthetic_def_key, child_names in self.synthetic_allowed.items():
            allowed[synthetic_def_key] = set(child_names)

        lines = []
        lines.append("// ============================================================================")
        lines.append("// Allowed properties per definition")
        lines.append("// ============================================================================")
        lines.append("")
        lines.append("inline const std::set<std::string>& get_allowed_properties(const std::string& def_name) {")
        lines.append("    static const std::map<std::string, std::set<std::string>> data = {")

        for def_name in sorted(allowed.keys()):
            props = ', '.join(f'"{prop}"' for prop in sorted(allowed[def_name]))
            lines.append(f'        {{"{def_name}", {{{props}}}}},')

        lines.append("    };")
        lines.append("    static const std::set<std::string> empty;")
        lines.append("    auto it = data.find(def_name);")
        lines.append("    return it != data.end() ? it->second : empty;")
        lines.append("}")
        lines.append("")
        return '\n'.join(lines)

    def _generate_top_level_blocks_function(self) -> str:
        blocks = sorted(self.schema.get('properties', {}).keys())

        lines = []
        lines.append("// ============================================================================")
        lines.append("// Allowed top-level blocks")
        lines.append("// ============================================================================")
        lines.append("")
        lines.append("inline const std::set<std::string>& get_allowed_top_level_blocks() {")
        lines.append("    static const std::set<std::string> data = {")
        for block_name in blocks:
            lines.append(f'        "{block_name}",')
        lines.append("    };")
        lines.append("    return data;")
        lines.append("}")
        lines.append("")
        return '\n'.join(lines)

    def _generate_internal_only_defaults_function(self) -> str:
        """Generate accessor for x-internal-only field defaults.

        These fields must NOT be in the instance when the validator runs
        (the library rejects them as user-provided).  After validation they
        must be present with their schema default if the library did not set
        them via mutation.  This mirrors pydantic's field-default behaviour.
        """
        # Collect x-internal-only fields that have a non-null default
        internal: Dict[str, Dict[str, Any]] = {}
        for defname, defn in self.defs.items():
            for prop, pschema in defn.get('properties', {}).items():
                if (pschema.get('x-internal-only')
                        and 'default' in pschema
                        and pschema['default'] is not None):
                    internal.setdefault(defname, {})[prop] = pschema['default']

        lines = []
        lines.append("// ============================================================================")
        lines.append("// x-internal-only field defaults (post-validation insertion)")
        lines.append("// ============================================================================")
        lines.append("")
        lines.append("inline const json& get_internal_only_defaults(const std::string& def_name) {")
        lines.append("    static const std::map<std::string, json> data = []() {")
        lines.append("        std::map<std::string, json> m;")

        for defname in sorted(internal):
            fields = internal[defname]
            field_parts = []
            for fname, fval in fields.items():
                if isinstance(fval, bool):
                    jval = 'true' if fval else 'false'
                elif isinstance(fval, str):
                    escaped = fval.replace('\\', '\\\\'').replace('"\', '\\"'')
                    jval = f'"{escaped}"'
                else:
                    jval = str(fval)
                field_parts.append(f'{{"{fname}", {jval}}}')
            lines.append(f'        m["{defname}"] = json::object({{{", ".join(field_parts)}}});')

        lines.append("        return m;")
        lines.append("    }();")
        lines.append("    static const json empty = json::object();")
        lines.append("    auto it = data.find(def_name);")
        lines.append("    return it != data.end() ? it->second : empty;")
        lines.append("}")
        lines.append("")
        return '\n'.join(lines)

    def _generate_preamble(self) -> str:
        all_defs = set(self.numeric_constraints) | set(self.model_validations)
        num_nc = sum(len(v) for v in self.numeric_constraints.values())
        num_mv = sum(len(v) for v in self.model_validations.values())
        num_cf = sum(len(v) for v in self.computed_fields.values())
        num_wm = sum(len(v) for v in self.schema_walk_map.values())

        return f"""\
// dakota_validation_metadata.hpp
// Generated validation metadata from JSON Schema
// DO NOT EDIT - Generated by generate_validation_metadata.py
//
// Statistics:
//   {len(all_defs)} definitions with validation metadata
//   {len(self.numeric_constraints)} definitions with numeric constraints ({num_nc} total)
//   {len(self.model_validations)} definitions with x-model-validations ({num_mv} rules)
//   {len(self.computed_fields)} definitions with x-computed-fields ({num_cf} fields)
//   {len(self.schema_walk_map)} definitions in schema walk map ({num_wm} property refs)

#ifndef DAKOTA_VALIDATION_METADATA_HPP
#define DAKOTA_VALIDATION_METADATA_HPP

#include <dakota/validation.hpp>
#include <dakota/computed_fields.hpp>
#include <nlohmann/json.hpp>
#include <cctype>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <stdexcept>
#include <cmath>
#include <limits>
#include <iostream>

namespace dakota::validation_metadata {{

using json = nlohmann::json;

// Debug flag - set before calling validate_json_document()
inline bool g_validation_debug = false;

// ============================================================================
// Numeric field constraint (generated from JSON Schema minimum/maximum etc.)
// ============================================================================

struct FieldConstraint {{
    enum Op {{ GE, GT, LE, LT }};
    Op op;
    double bound;
}};
"""

    def _generate_numeric_constraints_function(self) -> str:
        lines = []
        lines.append("// ============================================================================")
        lines.append("// Numeric constraints per definition")
        lines.append("// ============================================================================")
        lines.append("")
        lines.append("inline const std::map<std::string, std::vector<FieldConstraint>>&")
        lines.append("get_numeric_constraints(const std::string& def_name) {")
        lines.append("    using FC = FieldConstraint;")
        lines.append("    static const std::map<std::string,")
        lines.append("        std::map<std::string, std::vector<FieldConstraint>>> data = {")

        for def_name in sorted(self.numeric_constraints.keys()):
            constraints = self.numeric_constraints[def_name]
            by_field: Dict[str, List[Tuple[str, float]]] = defaultdict(list)
            for field_name, op, bound in constraints:
                by_field[field_name].append((op, bound))

            lines.append(f'        {{"{def_name}", {{')
            for field_name in sorted(by_field.keys()):
                fc_list = by_field[field_name]
                fc_strs = []
                for op, bound in fc_list:
                    if bound == int(bound) and abs(bound) < 1e15:
                        bound_str = f"{int(bound)}.0"
                    else:
                        bound_str = f"{bound}"
                    fc_strs.append(f"{{FC::{op}, {bound_str}}}")
                lines.append(f'            {{"{field_name}", {{{", ".join(fc_strs)}}}}},')
            lines.append(f'        }}}},')

        lines.append("    };")
        lines.append("    static const std::map<std::string, std::vector<FieldConstraint>> empty;")
        lines.append("    auto it = data.find(def_name);")
        lines.append("    return it != data.end() ? it->second : empty;")
        lines.append("}")
        lines.append("")
        return '\n'.join(lines)

    @staticmethod
    def _sanitize_json_str(obj: Any) -> str:
        """Serialize to JSON, replacing all infinity representations with 1e308/-1e308.

        Handles two schema variants:
          - Old schema: Python float('inf') from json.dumps producing 'Infinity'
          - New schema: string "inf"/"-inf" written directly in validationLiterals

        Both are normalised to 1e308/-1e308 (finite doubles that the C++ runtime
        recognises as infinity sentinels), keeping the generated header valid JSON
        and leaving the existing sentinel-conversion code untouched.
        """
        INF_STRINGS  = {'inf', 'infinity'}
        NINF_STRINGS = {'-inf', '-infinity'}

        def replace_inf(o):
            if isinstance(o, float):
                if o == float('inf'):  return 1e308
                if o == float('-inf'): return -1e308
            elif isinstance(o, str):
                low = o.lower()
                if low in INF_STRINGS:  return 1e308
                if low in NINF_STRINGS: return -1e308
            elif isinstance(o, dict):
                return {k: replace_inf(v) for k, v in o.items()}
            elif isinstance(o, list):
                return [replace_inf(v) for v in o]
            return o

        return json.dumps(replace_inf(obj), separators=(',', ':'))

    def _generate_model_validations_function(self) -> str:
        lines = []
        lines.append("// ============================================================================")
        lines.append("// x-model-validations per definition")
        lines.append("// ============================================================================")
        lines.append("")
        lines.append("inline const json& get_model_validations(const std::string& def_name) {")
        lines.append("    static const std::map<std::string, json> data = []() {")
        lines.append("        std::map<std::string, json> m;")

        for def_name in sorted(self.model_validations.keys()):
            validations = self.model_validations[def_name]
            json_str = self._sanitize_json_str(validations)
            lines.append(f'        m["{def_name}"] = json::parse(R"__val__({json_str})__val__");')

        lines.append("        return m;")
        lines.append("    }();")
        lines.append("    static const json empty = json::array();")
        lines.append("    auto it = data.find(def_name);")
        lines.append("    return it != data.end() ? it->second : empty;")
        lines.append("}")
        lines.append("")
        return '\n'.join(lines)

    def _generate_computed_fields_function(self) -> str:
        lines = []
        lines.append("// ============================================================================")
        lines.append("// x-computed-fields per definition")
        lines.append("// ============================================================================")
        lines.append("")
        lines.append("inline const json& get_computed_fields(const std::string& def_name) {")
        lines.append("    static const std::map<std::string, json> data = []() {")
        lines.append("        std::map<std::string, json> m;")

        for def_name in sorted(self.computed_fields.keys()):
            cf = self.computed_fields[def_name]
            json_str = json.dumps(cf, separators=(',', ':'))
            lines.append(f'        m["{def_name}"] = json::parse(R"__cf__({json_str})__cf__");')

        lines.append("        return m;")
        lines.append("    }();")
        lines.append("    static const json empty = json::object();")
        lines.append("    auto it = data.find(def_name);")
        lines.append("    return it != data.end() ? it->second : empty;")
        lines.append("}")
        lines.append("")
        return '\n'.join(lines)

    def _generate_schema_walk_map_function(self) -> str:
        lines = []
        lines.append("// ============================================================================")
        lines.append("// Schema walk map: definition -> { property -> child_definition }")
        lines.append("// ============================================================================")
        lines.append("")
        lines.append("inline const std::map<std::string, std::map<std::string, std::string>>&")
        lines.append("get_schema_walk_map() {")
        lines.append("    static const auto data = []() {")
        lines.append("        std::map<std::string, std::map<std::string, std::string>> m;")

        for def_name in sorted(self.schema_walk_map.keys()):
            children = self.schema_walk_map[def_name]
            # Build inner map as initializer list on one line
            pairs = ', '.join(
                f'{{"{prop}", "{child}"}}'
                for prop, child in sorted(children.items())
            )
            lines.append(f'        m["{def_name}"] = {{{pairs}}};')

        lines.append("        return m;")
        lines.append("    }();")
        lines.append("    return data;")
        lines.append("}")
        lines.append("")
        return '\n'.join(lines)

    def _generate_union_block_selector_function(self) -> str:
        lines = []
        lines.append("// ============================================================================")
        lines.append("// Top-level union block selectors")
        lines.append("// ============================================================================")
        lines.append("")
        lines.append("inline const std::map<std::string, std::map<std::string, std::string>>&")
        lines.append("get_union_block_selector_map() {")
        lines.append("    static const auto data = []() {")
        lines.append("        std::map<std::string, std::map<std::string, std::string>> m;")
        for block_name in sorted(self.union_block_variants.keys()):
            selector_pairs = []
            for sel_def in self.union_block_variants[block_name]:
                child_map = self.schema_walk_map.get(sel_def, {})
                for prop, child_def in sorted(child_map.items()):
                    selector_pairs.append(f'{{"{prop}", "{child_def}"}}')
            pairs = ', '.join(selector_pairs)
            lines.append(f'        m["{block_name}"] = {{{pairs}}};')
        lines.append("        return m;")
        lines.append("    }();")
        lines.append("    return data;")
        lines.append("}")
        lines.append("")
        return '\n'.join(lines)

    def _generate_block_definitions_function(self) -> str:
        lines = []
        lines.append("// ============================================================================")
        lines.append("// Top-level block definitions")
        lines.append("// ============================================================================")
        lines.append("")
        lines.append("struct BlockDef {")
        lines.append("    std::string config_type;")
        lines.append("    bool is_union;")
        lines.append("    bool is_array;")
        lines.append("    bool is_required;")
        lines.append("};")
        lines.append("")
        lines.append("inline const std::map<std::string, BlockDef>& get_block_definitions() {")
        lines.append("    static const std::map<std::string, BlockDef> data = {")

        for block_name in sorted(self.block_definitions.keys()):
            config_type, is_union, is_array = self.block_definitions[block_name]
            is_required = block_name in self.required_blocks
            lines.append(f'        {{"{block_name}", {{"{config_type}", '
                        f'{"true" if is_union else "false"}, '
                        f'{"true" if is_array else "false"}, '
                        f'{"true" if is_required else "false"}}}}},')

        lines.append("    };")
        lines.append("    return data;")
        lines.append("}")
        lines.append("")
        return '\n'.join(lines)


    def _generate_integer_field_coercion_function(self) -> str:
        """Generate get_integer_array_fields() for schema integer-type enforcement.

        When REALLIST and INTEGERLIST types conflict, the grammar uses REALLIST.
        Users writing integer values as "0." get doubles in the AST, producing
        "0.0" instead of the schema-correct "0". This enforces the schema type.
        """
        lines = []
        lines.append("// ============================================================================")
        lines.append("// Schema integer-type enforcement (float->int coercion)")
        lines.append("// ============================================================================")
        lines.append("")
        lines.append("inline const std::set<std::string>&")
        lines.append("get_integer_array_fields(const std::string& def_name) {")
        lines.append("    static const std::map<std::string, std::set<std::string>> data = []() {")
        lines.append("        std::map<std::string, std::set<std::string>> m;")
        lines.append("        m[\"AdaptiveSamplingConfig\"] = {\"refinement_samples\"};")
        lines.append("        m[\"ApproximateControlVariateConfig\"] = {\"pilot_samples\", \"seed_sequence\"};")
        lines.append("        m[\"BinomialUncertain\"] = {\"initial_point\", \"num_trials\"};")
        lines.append("        m[\"BuildSurrogate\"] = {\"refinement_samples\"};")
        lines.append("        m[\"CenteredParameterStudyConfig\"] = {\"steps_per_variable\"};")
        lines.append("        m[\"ContinuousIntervalUncertain\"] = {\"num_intervals\"};")
        lines.append("        m[\"DiscreteDesignRange\"] = {\"initial_point\", \"lower_bounds\", \"upper_bounds\"};")
        lines.append("        m[\"DiscreteDesignSetInteger\"] = {\"elements\", \"elements_per_variable\", \"initial_point\"};")
        lines.append("        m[\"DiscreteDesignSetReal\"] = {\"elements_per_variable\"};")
        lines.append("        m[\"DiscreteDesignSetString\"] = {\"adjacency_matrix\", \"elements_per_variable\"};")
        lines.append("        m[\"DiscreteIntervalUncertain\"] = {\"initial_point\", \"lower_bounds\", \"num_intervals\", \"upper_bounds\"};")
        lines.append("        m[\"DiscreteStateRange\"] = {\"initial_state\", \"lower_bounds\", \"upper_bounds\"};")
        lines.append("        m[\"DiscreteStateSetInteger\"] = {\"elements\", \"elements_per_variable\", \"initial_state\"};")
        lines.append("        m[\"DiscreteStateSetReal\"] = {\"elements_per_variable\"};")
        lines.append("        m[\"DiscreteStateSetString\"] = {\"elements_per_variable\"};")
        lines.append("        m[\"DiscreteUncertainSetInteger\"] = {\"elements\", \"elements_per_variable\", \"initial_point\"};")
        lines.append("        m[\"DiscreteUncertainSetReal\"] = {\"elements_per_variable\"};")
        lines.append("        m[\"DiscreteUncertainSetString\"] = {\"elements_per_variable\"};")
        lines.append("        m[\"EmulatorMfPceExpansionOrderSequenceCollocRatioConfig\"] = {\"collocation_points_sequence\"};")
        lines.append("        m[\"EmulatorMfPceExpansionOrderSequenceConfig\"] = {\"sequence\"};")
        lines.append("        m[\"EmulatorMfPceQuadratureOrderSequenceConfig\"] = {\"sequence\"};")
        lines.append("        m[\"EmulatorMfPceSGLevelSequenceConfig\"] = {\"sequence\"};")
        lines.append("        m[\"EmulatorMfScSGLevelSequenceConfig\"] = {\"sequence\"};")
        lines.append("        m[\"EmulatorMlPceExpansionOrderSequenceCollocRatioConfig\"] = {\"collocation_points_sequence\"};")
        lines.append("        m[\"EmulatorMlPceExpansionOrderSequenceConfig\"] = {\"sequence\"};")
        lines.append("        m[\"EmulatorMlPceExpansionOrderSequenceExpansionSamplesSequenceConfig\"] = {\"sequence\"};")
        lines.append("        m[\"EmulatorMlPceOrthogLeastInterpConfig\"] = {\"collocation_points_sequence\", \"tensor_grid\"};")
        lines.append("        m[\"EmulatorPceOrthogLeastInterpConfig\"] = {\"tensor_grid\"};")
        lines.append("        m[\"ExpansionOptionsGenReliabilityLevels\"] = {\"num_gen_reliability_levels\"};")
        lines.append("        m[\"ExpansionOptionsProbabilityLevels\"] = {\"num_probability_levels\"};")
        lines.append("        m[\"ExpansionOptionsProbabilityRefinement\"] = {\"refinement_samples\"};")
        lines.append("        m[\"ExpansionOptionsReliabilityLevels\"] = {\"num_reliability_levels\"};")
        lines.append("        m[\"ExpansionOptionsResponseLevels\"] = {\"num_response_levels\"};")
        lines.append("        m[\"FieldCalibrationTerms\"] = {\"lengths\", \"num_coordinates_per_field\"};")
        lines.append("        m[\"FieldObjectives\"] = {\"lengths\", \"num_coordinates_per_field\"};")
        lines.append("        m[\"FieldResponses\"] = {\"lengths\", \"num_coordinates_per_field\"};")
        lines.append("        m[\"FsuQuasiMcConfig\"] = {\"prime_base\", \"sequence_leap\", \"sequence_start\"};")
        lines.append("        m[\"GenReliabilityLevelsGenReliabilityLevels\"] = {\"num_gen_reliability_levels\"};")
        lines.append("        m[\"GeneratingMatricesInline\"] = {\"inline\"};")
        lines.append("        m[\"GeneratingVectorInline\"] = {\"inline\"};")
        lines.append("        m[\"GeometricUncertain\"] = {\"initial_point\"};")
        lines.append("        m[\"HistogramBinUncertain\"] = {\"pairs_per_variable\"};")
        lines.append("        m[\"HistogramPointUncertainInteger\"] = {\"abscissas\", \"initial_point\", \"pairs_per_variable\"};")
        lines.append("        m[\"HistogramPointUncertainReal\"] = {\"pairs_per_variable\"};")
        lines.append("        m[\"HistogramPointUncertainString\"] = {\"pairs_per_variable\"};")
        lines.append("        m[\"HypergeometricUncertain\"] = {\"initial_point\", \"num_drawn\", \"selected_population\", \"total_population\"};")
        lines.append("        m[\"IdNumericalHessians\"] = {\"values\"};")
        lines.append("        m[\"IdQuasiHessians\"] = {\"values\"};")
        lines.append("        m[\"ImportanceSamplingConfig\"] = {\"refinement_samples\"};")
        lines.append("        m[\"IntegerCategorical\"] = {\"adjacency_matrix\"};")
        lines.append("        m[\"IntegrationProbabilityRefinement\"] = {\"refinement_samples\"};")
        lines.append("        m[\"MfPceConfig\"] = {\"seed_sequence\"};")
        lines.append("        m[\"MfPceExpansionOrderSequenceCollocRatioConfig\"] = {\"collocation_points_sequence\"};")
        lines.append("        m[\"MfPceExpansionOrderSequenceConfig\"] = {\"sequence\"};")
        lines.append("        m[\"MfPceExpansionOrderSequenceExpansionSamplesSequenceConfig\"] = {\"sequence\"};")
        lines.append("        m[\"MfPceOrthogLeastInterpConfig\"] = {\"collocation_points_sequence\", \"tensor_grid\"};")
        lines.append("        m[\"MfPceQuadratureOrderSequenceConfig\"] = {\"sequence\"};")
        lines.append("        m[\"MfPceSGLevelSequenceConfig\"] = {\"sequence\"};")
        lines.append("        m[\"MixedGradientsConfig\"] = {\"id_analytic_gradients\", \"id_numerical_gradients\"};")
        lines.append("        m[\"MixedHessiansConfig\"] = {\"id_analytic_hessians\"};")
        lines.append("        m[\"MlPceConfig\"] = {\"seed_sequence\"};")
        lines.append("        m[\"MlPceExpansionOrderSequenceCollocRatioConfig\"] = {\"collocation_points_sequence\"};")
        lines.append("        m[\"MlPceExpansionOrderSequenceConfig\"] = {\"sequence\"};")
        lines.append("        m[\"MlPceExpansionOrderSequenceExpansionSamplesSequenceConfig\"] = {\"sequence\"};")
        lines.append("        m[\"MlPceOrthogLeastInterpConfig\"] = {\"collocation_points_sequence\", \"tensor_grid\"};")
        lines.append("        m[\"MultidimParameterStudyConfig\"] = {\"partitions\"};")
        lines.append("        m[\"MultifidelityFtConfig\"] = {\"collocation_points_sequence\", \"seed_sequence\", \"start_rank_sequence\"};")
        lines.append("        m[\"MultifidelitySamplingConfig\"] = {\"pilot_samples\", \"seed_sequence\"};")
        lines.append("        m[\"MultifidelityStochCollocConfig\"] = {\"seed_sequence\"};")
        lines.append("        m[\"MultifidelityStochCollocQuadratureOrderSequenceConfig\"] = {\"sequence\"};")
        lines.append("        m[\"MultifidelityStochCollocSGLevelSequenceConfig\"] = {\"sequence\"};")
        lines.append("        m[\"MultilevelBlueConfig\"] = {\"seed_sequence\"};")
        lines.append("        m[\"MultilevelFtConfig\"] = {\"collocation_points_sequence\", \"seed_sequence\", \"start_rank_sequence\"};")
        lines.append("        m[\"MultilevelMcmcConfig\"] = {\"subsampling_steps\"};")
        lines.append("        m[\"MultilevelMultifidelitySamplingConfig\"] = {\"pilot_samples\", \"seed_sequence\"};")
        lines.append("        m[\"MultilevelSamplingConfig\"] = {\"pilot_samples\", \"seed_sequence\"};")
        lines.append("        m[\"NegativeBinomialUncertain\"] = {\"initial_point\", \"num_trials\"};")
        lines.append("        m[\"PceOrthogLeastInterpConfig\"] = {\"tensor_grid\"};")
        lines.append("        m[\"PilotSamples\"] = {\"counts\"};")
        lines.append("        m[\"PoissonUncertain\"] = {\"initial_point\"};")
        lines.append("        m[\"ProbabilityLevelsContext1ProbabilityLevels\"] = {\"num_probability_levels\"};")
        lines.append("        m[\"ProbabilityLevelsContext2ProbabilityLevels\"] = {\"num_probability_levels\"};")
        lines.append("        m[\"PsuadeMoatConfig\"] = {\"partitions\"};")
        lines.append("        m[\"RealCategorical\"] = {\"adjacency_matrix\"};")
        lines.append("        m[\"ReliabilityLevelsReliabilityLevels\"] = {\"num_reliability_levels\"};")
        lines.append("        m[\"ResponseLevelsComputeProbGenContext1ResponseLevels\"] = {\"num_response_levels\"};")
        lines.append("        m[\"ResponseLevelsComputeProbGenContext2ResponseLevels\"] = {\"num_response_levels\"};")
        lines.append("        m[\"ResponseLevelsComputeProbRelGenResponseLevels\"] = {\"num_response_levels\"};")
        lines.append("        m[\"SamplingConfig\"] = {\"refinement_samples\"};")
        lines.append("        m[\"StartOrderSequence\"] = {\"sequence\"};")
        lines.append("        m[\"SurrogateConfig\"] = {\"id_surrogates\"};")
        lines.append("        return m;")
        lines.append("    }();")
        lines.append("    static const std::set<std::string> empty;")
        lines.append("    auto it = data.find(def_name);")
        lines.append("    return it != data.end() ? it->second : empty;")
        lines.append("}")
        lines.append("")
        return '\n'.join(lines)

    def _generate_float_field_coercion_function(self) -> str:
        lines = []
        lines.append("// ============================================================================")
        lines.append("// Schema float-type fields (for non-finite string coercion)")
        lines.append("// ============================================================================")
        lines.append("")
        lines.append("inline const std::map<std::string, bool>&")
        lines.append("get_float_fields(const std::string& def_name) {")
        lines.append("    static const std::map<std::string, std::map<std::string, bool>> data = {")

        for def_name in sorted(self.float_fields.keys()):
            fields = self.float_fields[def_name]
            field_entries = ', '.join(
                f'{{"{field_name}", {"true" if is_array else "false"}}}'
                for field_name, is_array in sorted(fields.items())
            )
            lines.append(f'        {{"{def_name}", {{{field_entries}}}}},')

        lines.append("    };")
        lines.append("    static const std::map<std::string, bool> empty;")
        lines.append("    auto it = data.find(def_name);")
        lines.append("    return it != data.end() ? it->second : empty;")
        lines.append("}")
        lines.append("")
        return '\n'.join(lines)

    def _generate_validate_definition_function(self) -> str:
        return """\
// ============================================================================
// Single-definition validation
// ============================================================================

inline bool value_matches_type_name(const json& value, const std::string& type_name) {
    if (type_name == "null") return value.is_null();
    if (type_name == "boolean") return value.is_boolean();
    if (type_name == "string") return value.is_string();
    if (type_name == "integer") return value.is_number_integer();
    if (type_name == "number") return value.is_number();
    if (type_name == "array") return value.is_array();
    if (type_name == "object") return value.is_object();
    return true;
}

inline bool value_matches_variant_spec(const json& value, const json& spec) {
    if (spec.contains("type")) {
        const auto& type_name = spec["type"].get_ref<const std::string&>();
        if (!value_matches_type_name(value, type_name)) {
            return false;
        }
    }
    if (spec.contains("const") && value != spec["const"]) {
        return false;
    }
    return true;
}

inline void check_property_type_specs(const json& instance, const std::string& def_name) {
    const auto& specs = get_property_type_specs(def_name);
    if (specs.empty()) return;

    for (const auto& [field_name, variants] : specs.items()) {
        if (!instance.contains(field_name)) continue;
        const auto& value = instance[field_name];
        bool matched = false;
        for (const auto& spec : variants) {
            if (value_matches_variant_spec(value, spec)) {
                matched = true;
                break;
            }
        }
        if (!matched) {
            throw validation::ValidationError(
                "Field '" + field_name + "' does not match schema type/const requirements (in " + def_name + ")");
        }
    }
}

inline void check_numeric_constraints(
    const json& instance,
    const std::string& def_name)
{
    const auto& constraints = get_numeric_constraints(def_name);
    if (constraints.empty()) return;

    for (const auto& [field_name, field_constraints] : constraints) {
        if (!instance.contains(field_name) || instance[field_name].is_null())
            continue;

        const auto& jval = instance[field_name];
        double val;
        if (jval.is_number_integer())
            val = static_cast<double>(jval.get<int64_t>());
        else if (jval.is_number_float())
            val = jval.get<double>();
        else
            continue;

        for (const auto& fc : field_constraints) {
            bool ok = true;
            const char* op_str = "";
            switch (fc.op) {
                case FieldConstraint::GE: ok = val >= fc.bound; op_str = ">="; break;
                case FieldConstraint::GT: ok = val >  fc.bound; op_str = ">";  break;
                case FieldConstraint::LE: ok = val <= fc.bound; op_str = "<="; break;
                case FieldConstraint::LT: ok = val <  fc.bound; op_str = "<";  break;
            }
            if (!ok) {
                std::string bound_str;
                if (fc.bound == static_cast<int64_t>(fc.bound) &&
                    std::abs(fc.bound) < 1e15) {
                    bound_str = std::to_string(static_cast<int64_t>(fc.bound));
                } else {
                    bound_str = std::to_string(fc.bound);
                }

                throw validation::ValidationError(
                    "Field '" + field_name + "' value " +
                    (jval.is_number_integer()
                        ? std::to_string(jval.get<int64_t>())
                        : std::to_string(val)) +
                    " violates constraint: must be " +
                    op_str + " " + bound_str +
                    " (in " + def_name + ")");
            }
        }
    }
}

inline bool parse_nonfinite_float_string(const std::string& text, double& value) {
    std::string lowered;
    lowered.reserve(text.size());
    for (unsigned char ch : text) {
        lowered.push_back(static_cast<char>(std::tolower(ch)));
    }

    if (lowered == "inf" || lowered == "+inf" ||
        lowered == "infinity" || lowered == "+infinity") {
        value = std::numeric_limits<double>::infinity();
        return true;
    }
    if (lowered == "-inf" || lowered == "-infinity") {
        value = -std::numeric_limits<double>::infinity();
        return true;
    }
    if (lowered == "nan" || lowered == "+nan" || lowered == "-nan") {
        value = std::numeric_limits<double>::quiet_NaN();
        return true;
    }
    return false;
}

inline void coerce_nonfinite_float_value(json& value) {
    if (!value.is_string()) return;

    double parsed_value;
    if (parse_nonfinite_float_string(value.get<std::string>(), parsed_value)) {
        value = parsed_value;
    }
}

inline void coerce_nonfinite_float_fields(json& instance, const std::string& def_name) {
    const auto& float_fields = get_float_fields(def_name);
    if (float_fields.empty()) return;

    for (const auto& [field_name, is_array] : float_fields) {
        if (!instance.contains(field_name) || instance[field_name].is_null()) {
            continue;
        }

        json& field_value = instance[field_name];
        if (is_array) {
            if (!field_value.is_array()) continue;
            for (auto& elem : field_value) {
                coerce_nonfinite_float_value(elem);
            }
        } else {
            coerce_nonfinite_float_value(field_value);
        }
    }
}

// Apply mutations that may contain dotted path keys.
// Simple keys are applied directly; dotted keys navigate into nested JSON.
inline void apply_dotted_mutations(json& target, const json& mutations) {
    for (auto& [key, value] : mutations.items()) {
        auto dot_pos = key.find('.');
        if (dot_pos == std::string::npos) {
            target[key] = value;
        } else {
            json* current = &target;
            std::string remaining = key;
            while ((dot_pos = remaining.find('.')) != std::string::npos) {
                std::string segment = remaining.substr(0, dot_pos);
                remaining = remaining.substr(dot_pos + 1);
                if (!current->contains(segment) || !(*current)[segment].is_object()) {
                    current = nullptr;
                    break;
                }
                current = &(*current)[segment];
            }
            if (current) {
                (*current)[remaining] = value;
            }
        }
    }
}

inline json validate_definition(const std::string& def_name, json instance) {
    // 1. Coerce string representations of non-finite values for float fields.
    coerce_nonfinite_float_fields(instance, def_name);

    // 2. Enforce basic schema type/const requirements for direct properties.
    check_property_type_specs(instance, def_name);

    // 3. Check field-level numeric constraints
    check_numeric_constraints(instance, def_name);

    // 4. Run x-model-validations individually with null-mutation guard
    //    We inline the loop here instead of calling validate_all() because
    //    validate_all uses merge_patch(mutations) which, per RFC 7396, replaces
    //    the entire instance with null when a validator returns an uninitialized
    //    json value (null) instead of json::object().
    const auto& rules = get_model_validations(def_name);
    if (!rules.empty()) {
        auto& registry = validation::ValidatorRegistry::instance();
        if (g_validation_debug) {
            std::cerr << "[VAL_DEBUG] validate_definition: " << def_name
                      << " (" << rules.size() << " rules)\\n";
            std::cerr << "[VAL_DEBUG]   instance: "
                      << instance.dump(2).substr(0, 500) << "\\n";
        }
        for (const auto& rule_spec : rules) {
            std::string rule_name = rule_spec.at("validationRuleName").get<std::string>();
            auto fields = rule_spec.at("validationFields").get<std::vector<std::string>>();
            const auto& literals = rule_spec.at("validationLiterals");
            std::string context = rule_spec.at("validationContext").get<std::string>();

            // The JSON metadata encodes Python float('inf') as ±1e308 because
            // JSON has no infinity literal.
            //
            // For bound-check and real-bound-defaulting rules, we convert
            // the sentinel to IEEE infinity so comparisons like "v <= inf" work
            // correctly and defaulted bound arrays preserve unbounded semantics.
            //
            // For all other rules (e.g. default_initial_point_real),
            // we keep ±1e308 as-is.  These functions use the sentinel values to
            // construct default arrays, not for comparisons; passing IEEE infinity
            // changes their internal behaviour and causes type errors when the
            // instance already has populated fields.
            const bool restore_real_infinity_literals =
                (rule_name == "check_real_upper_bound" ||
                 rule_name == "check_real_lower_bound" ||
                 rule_name == "default_bounds_real");

            json resolved_literals = literals;
            if (restore_real_infinity_literals) {
                for (auto& lit : resolved_literals) {
                    if (lit.is_number_float()) {
                        double v = lit.get<double>();
                        if (v == 1e308)  lit = std::numeric_limits<double>::infinity();
                        else if (v == -1e308) lit = -std::numeric_limits<double>::infinity();
                    }
                }
                // Skip vacuous bound checks that are always true for finite values.
                if ((rule_name == "check_real_upper_bound" ||
                     rule_name == "check_real_lower_bound") &&
                    !resolved_literals.empty() && resolved_literals[0].is_number_float() &&
                    std::isinf(resolved_literals[0].get<double>())) {
                    continue;
                }
            }

            if (g_validation_debug) {
                std::cerr << "[VAL_DEBUG]   rule: " << rule_name << "\\n";
            }

            // Guard: skip default-setter rules when their target fields are
            // already populated.  The C++ library implementations of these
            json mutations;
            try {
                mutations = registry.validate(instance, rule_name, fields, resolved_literals, context);
            } catch (const validation::ValidationError& e) {
                throw;  // Let ValidationError propagate as-is
            } catch (const std::exception& e) {
                std::string field_list;
                for (size_t fi = 0; fi < fields.size(); ++fi) {
                    if (fi) field_list += ", ";
                    field_list += fields[fi];
                }
                throw std::runtime_error(
                    "rule '" + rule_name + "' on def '" + def_name +
                    "' (fields: [" + field_list + "]): " + e.what() +
                    " | instance: " + instance.dump());
            }

            if (g_validation_debug && !mutations.is_null() && !mutations.empty()) {
                std::cerr << "[VAL_DEBUG]   mutations from " << rule_name << ": "
                          << mutations.dump(2).substr(0, 1000) << "\\n";
            }

            // Apply mutations: validators return partial objects with field names as keys.
            // Some use dotted paths (e.g., "data_spec.calibration_data_file.field")
            // which must navigate into nested JSON structure.
            if (mutations.is_object() && !mutations.empty()) {
                apply_dotted_mutations(instance, mutations);
            }
        }
    }

    // 4. Compute x-computed-fields
    const auto& cf = get_computed_fields(def_name);
    if (!cf.empty()) {
        auto& registry = computed_fields::ComputedFieldRegistry::instance();
        for (auto& [field_name, field_meta] : cf.items()) {
            if (!field_meta.contains("function")) continue;
            const std::string& func_name = field_meta["function"].get_ref<const std::string&>();
            if (g_validation_debug) {
                std::cerr << "[VAL_DEBUG] computed_field: " << def_name
                          << "." << field_name << " = " << func_name << "()\\n";
            }
            try {
                instance[field_name] = registry.compute(instance, func_name);
            } catch (const std::exception& e) {
                if (g_validation_debug) {
                    std::cerr << "[VAL_DEBUG] computed_field FAILED: "
                              << func_name << ": " << e.what() << "\\n";
                }
                throw std::runtime_error(
                    "computed_field '" + field_name + "' (function '" +
                    func_name + "') in def '" + def_name + "': " + e.what());
            }
        }
    }

    // 5. Enforce schema integer types: coerce whole-number doubles to int64_t.
    // Needed when REALLIST grammar parses "0." as 0.0 but the schema
    // declares the field as items.type=integer.
    {
        const auto& int_fields = get_integer_array_fields(def_name);
        for (const auto& field_name : int_fields) {
            if (!instance.contains(field_name) || !instance[field_name].is_array()) continue;
            for (auto& elem : instance[field_name]) {
                if (elem.is_number_float()) {
                    double v = elem.get<double>();
                    auto iv = static_cast<int64_t>(v);
                    if (static_cast<double>(iv) == v) {
                        elem = iv;
                    }
                }
            }
        }
    }

    // 6. Post-validation: insert x-internal-only field defaults where absent.
    // Pydantic sets these via field defaults BEFORE validation so the validator
    // can read but not modify them directly.  We insert them AFTER validation
    // so the library\'s "must not be user-provided" guard never sees them,
    // but they still appear in the final output with the correct default value.
    const auto& internal_defs = get_internal_only_defaults(def_name);
    if (!internal_defs.empty()) {
        for (auto& [field, default_val] : internal_defs.items()) {
            if (!instance.contains(field)) {
                instance[field] = default_val;
            }
        }
    }

    return instance;
}

// ============================================================================
// Recursive JSON tree walker
// ============================================================================

inline void walk_and_validate(
    json& instance,
    const std::string& def_name,
    const std::string& path,
    std::vector<std::string>& errors)
{
    if (!instance.is_object()) return;

    const auto& allowed_props = get_allowed_properties(def_name);
    for (auto& [prop_name, _] : instance.items()) {
        if (!allowed_props.empty() && !allowed_props.count(prop_name)) {
            errors.push_back(path + ": unexpected property '" + prop_name + "'");
        }
    }

    if (g_validation_debug) {
        std::cerr << "[VAL_DEBUG] walk: " << path
                  << " (def=" << def_name << ", "
                  << instance.size() << " fields)\\n";
    }

    try {
        instance = validate_definition(def_name, std::move(instance));
    } catch (const validation::ValidationError& e) {
        if (g_validation_debug) {
            std::cerr << "[VAL_DEBUG] ValidationError at " << path
                      << " (def=" << def_name << "): " << e.what() << "\\n";
        }
        errors.push_back(path + ": " + e.what());
        return;  // instance is moved-from; cannot walk children
    } catch (const std::exception& e) {
        errors.push_back(path + " (def=" + def_name + "): [internal] " + e.what());
        if (g_validation_debug) {
            std::cerr << "[VAL_DEBUG] EXCEPTION at " << path
                      << " (def=" << def_name << "): " << e.what() << "\\n";
        }
        return;  // instance is moved-from; cannot walk children
    }

    const auto& walk_map = get_schema_walk_map();
    auto it = walk_map.find(def_name);
    if (it == walk_map.end()) return;

    for (const auto& [prop_name, child_def] : it->second) {
        if (!instance.contains(prop_name)) continue;

        auto& child = instance[prop_name];
        std::string child_path = path + "." + prop_name;

        if (child.is_object()) {
            walk_and_validate(child, child_def, child_path, errors);
        }
    }
}
"""

    def _generate_validate_json_document_function(self) -> str:
        return """\
// ============================================================================
// Document-level validation entry point
// ============================================================================

inline int validate_json_document(json& doc, std::vector<std::string>& errors) {
    const auto& block_defs = get_block_definitions();
    const auto& walk_map = get_schema_walk_map();
    const auto& union_selector_map = get_union_block_selector_map();
    const auto& allowed_blocks = get_allowed_top_level_blocks();
    int initial_errors = static_cast<int>(errors.size());

    for (auto& [block_name, _] : doc.items()) {
        if (!allowed_blocks.count(block_name)) {
            errors.push_back("Unexpected top-level block: " + block_name);
        }
    }

    for (const auto& [block_name, block_def] : block_defs) {
        if (!doc.contains(block_name)) {
            if (block_def.is_required) {
                errors.push_back("Missing required block: " + block_name);
            }
            continue;
        }

        auto& block_json = doc[block_name];

        if (!block_def.is_union && !block_def.config_type.empty()) {
            // Non-union block: validate directly as config type
            if (block_def.is_array && block_json.is_array()) {
                for (size_t i = 0; i < block_json.size(); ++i) {
                    std::string path = block_name + "[" + std::to_string(i) + "]";
                    walk_and_validate(block_json[i], block_def.config_type, path, errors);
                }
            } else if (block_json.is_object()) {
                walk_and_validate(block_json, block_def.config_type, block_name, errors);
            }
        } else if (block_def.is_union) {
            // Union block (method, model): validate against the explicit
            // top-level selector map for that block. Do not scan the entire
            // walk map, since nested contexts may reuse the same property names.
            auto process_union = [&](json& instance, const std::string& path) {
                if (!instance.is_object()) return;

                auto sel_map_it = union_selector_map.find(block_name);
                if (sel_map_it == union_selector_map.end()) return;
                const auto& selector_children = sel_map_it->second;

                for (auto& [prop_name, prop_val] : instance.items()) {
                    auto child_it = selector_children.find(prop_name);
                    if (child_it == selector_children.end()) {
                        errors.push_back(path + ": unexpected property '" + prop_name + "'");
                        continue;
                    }

                    std::string child_path = path + "." + prop_name;
                    if (prop_val.is_object()) {
                        walk_and_validate(prop_val, child_it->second, child_path, errors);
                    }
                }
            };

            if (block_def.is_array && block_json.is_array()) {
                for (size_t i = 0; i < block_json.size(); ++i) {
                    std::string path = block_name + "[" + std::to_string(i) + "]";
                    process_union(block_json[i], path);
                }
            } else if (block_json.is_object()) {
                process_union(block_json, block_name);
            }
        }
    }

    return static_cast<int>(errors.size()) - initial_errors;
}

}  // namespace dakota::validation_metadata

#endif // DAKOTA_VALIDATION_METADATA_HPP
"""

    # =========================================================================
    # Statistics
    # =========================================================================

    def print_stats(self):
        all_defs = set(self.numeric_constraints) | set(self.model_validations)
        num_nc = sum(len(v) for v in self.numeric_constraints.values())
        num_mv = sum(len(v) for v in self.model_validations.values())
        num_cf = sum(len(v) for v in self.computed_fields.values())
        num_wm = sum(len(v) for v in self.schema_walk_map.values())

        print(f"  {len(all_defs)} definitions with validation metadata")
        print(f"    - {len(self.numeric_constraints)} defs with numeric constraints ({num_nc} total)")
        print(f"    - {len(self.model_validations)} defs with x-model-validations ({num_mv} rules)")
        print(f"    - {len(self.computed_fields)} defs with x-computed-fields ({num_cf} fields)")
        print(f"    - {len(self.schema_walk_map)} defs in walk map ({num_wm} property refs)")

        print(f"    Block definitions:")
        for block_name in sorted(self.block_definitions.keys()):
            config_type, is_union, is_array = self.block_definitions[block_name]
            label = config_type if config_type else "(union)"
            array_label = "[]" if is_array else ""
            print(f"      {block_name}{array_label} -> {label}")


def main():
    parser = argparse.ArgumentParser(
        description='Generate validation metadata from Dakota JSON Schema')
    parser.add_argument('schema_file', type=Path,
                        help='Dakota JSON Schema specification')
    parser.add_argument('-o', '--output-dir', type=Path, default=Path('.'),
                        help='Output directory for generated header')
    args = parser.parse_args()

    if not args.schema_file.exists():
        print(f"Error: Schema file not found: {args.schema_file}", file=sys.stderr)
        return 1

    gen = ValidationMetadataGenerator(str(args.schema_file))
    gen.process_all()
    gen.print_stats()

    args.output_dir.mkdir(parents=True, exist_ok=True)
    output_path = args.output_dir / "dakota_validation_metadata.hpp"
    header = gen.generate_header()
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(header)

    print(f"  Generated {output_path}")
    return 0


if __name__ == '__main__':
    sys.exit(main())
