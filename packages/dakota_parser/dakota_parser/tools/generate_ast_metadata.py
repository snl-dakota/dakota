#!/usr/bin/env python3
"""
Generate AST-to-JSON metadata from JSON Schema.

This generates dakota_ast_metadata.hpp containing:
- Boolean fields
- Argument fields (field -> argument name mapping)
- Anchor fields (parent -> {anchor_name -> [children]})
- Array fields
- Number fields
- Discriminator config types
- Variable type metadata
"""

import json
import argparse
from pathlib import Path
from typing import Dict, Set, List, Tuple, Any, Optional

import schema_utils as su


class ASTMetadataGenerator:
    def __init__(self, schema_path: str):
        with open(schema_path, encoding='utf-8') as f:
            self.schema = json.load(f)
        self.defs = self.schema.get("$defs", {})
        
        # Collected metadata
        self.boolean_fields: Set[str] = set()
        self.argument_fields: Dict[str, str] = {}  # field_name -> argument_name (only for ALWAYS-have-argument fields)
        self.context_argument_fields: Dict[str, Dict[str, str]] = {}  # parent_context -> {field_name -> argument_name}
        self.field_argument_occurrences: Dict[str, Dict[str, int]] = {}  # field_name -> {argument_name -> count}
        self.field_total_occurrences: Dict[str, int] = {}  # field_name -> total occurrence count
        self.anchor_fields: Dict[str, Dict[str, Set[str]]] = {}  # parent -> {anchor -> [children]}
        self.array_fields: Set[str] = set()
        self.number_fields: Set[str] = set()  # Fields that are ALWAYS number type
        self.integer_fields: Set[str] = set()  # Fields that are sometimes integer type
        self.context_number_fields: Dict[str, Set[str]] = {}  # context -> [fields]
        self.discriminator_configs: Dict[str, str] = {}  # keyword -> ConfigType
        self.context_discriminator_configs: Dict[str, Dict[str, str]] = {}  # def_name -> {property -> ConfigType}
        self.variable_types: Set[str] = set()
        self.variable_prefixes: Dict[str, str] = {}  # var_type -> prefix
        self.variable_types_with_initial_point: Set[str] = set()  # var types that have initial_point
        self.scalar_argument_fields: Set[str] = set()  # argument field names that expect scalar values
        self.array_argument_keywords: Set[str] = set()  # keyword names where the argument target is an array
        self.definition_properties: Dict[str, Set[str]] = {}  # def_name -> set of all property names
        self.context_argument_fields_by_def: Dict[str, Dict[str, str]] = {}  # def_name -> {field_name -> argument_name}
        self.non_array_overrides: Dict[str, Set[str]] = {}  # def_name -> {fields that are NOT array here}
        
    # --- Schema utility delegations (thin wrappers for backward compat) ---
    def resolve_ref(self, ref: str) -> Optional[dict]:
        return su.resolve(self.defs, ref)
    
    def get_ref_name(self, ref: str) -> str:
        return su.ref_name(ref)
    
    def is_boolean_type(self, s: dict) -> bool:
        return su.is_boolean(s)
    
    def is_array_type(self, s: dict) -> bool:
        return su.is_array(s)
    
    def is_number_type(self, s: dict) -> bool:
        return su.is_number(s)
    
    def is_integer_type(self, s: dict) -> bool:
        return su.is_integer(s)
    
    def extract_anchor_children(self, anchor_schema: dict) -> Set[str]:
        return su.anchor_children(self.defs, anchor_schema)
    
    def process_definition(self, def_name: str, def_schema: dict):
        """Process a single definition to extract metadata."""
        properties = def_schema.get("properties", {})
        parent_context = self.get_parent_context(def_name)
        
        # Track all property names for this definition
        if properties:
            self.definition_properties[def_name] = set(properties.keys())
        
        for field_name, field_schema in properties.items():
            # Track total occurrences of each field
            self.field_total_occurrences[field_name] = self.field_total_occurrences.get(field_name, 0) + 1
            
            # Check for boolean fields
            if self.is_boolean_type(field_schema):
                self.boolean_fields.add(field_name)
            
            # Check for argument attribute - track occurrences per field
            if "argument" in field_schema:
                arg_name = field_schema["argument"]
                
                # Track how many times this field appears with this argument
                if field_name not in self.field_argument_occurrences:
                    self.field_argument_occurrences[field_name] = {}
                self.field_argument_occurrences[field_name][arg_name] = \
                    self.field_argument_occurrences[field_name].get(arg_name, 0) + 1
                
                # Also track context-specific arguments
                if parent_context:
                    if parent_context not in self.context_argument_fields:
                        self.context_argument_fields[parent_context] = {}
                    self.context_argument_fields[parent_context][field_name] = arg_name
                
                # Track by definition name (unambiguous, unlike keyword name)
                self.context_argument_fields_by_def.setdefault(def_name, {})[field_name] = arg_name
                
                # Check if this argument field expects a scalar value
                ref_type = su.first_ref(field_schema)
                if ref_type and ref_type in self.defs:
                    arg_prop = self.defs[ref_type].get("properties", {}).get(arg_name, {})
                    if arg_prop:
                        if not su.is_array(arg_prop):
                            self.scalar_argument_fields.add(arg_name)
                        else:
                            self.array_argument_keywords.add(field_name)
                        if su.is_number(arg_prop):
                            self.number_fields.add(field_name)
                        if su.is_integer(arg_prop):
                            self.integer_fields.add(field_name)
            
            # Check for anchor fields
            if su.is_anchor(field_schema):
                self.anchor_fields.setdefault(def_name, {})[field_name] = \
                    su.anchor_children(self.defs, field_schema)
            
            # Check for array fields
            if self.is_array_type(field_schema):
                self.array_fields.add(field_name)
            
            # Check for number fields (but also track if field is sometimes integer)
            if self.is_number_type(field_schema):
                self.number_fields.add(field_name)
            if self.is_integer_type(field_schema):
                self.integer_fields.add(field_name)
            
            # Check for $ref to build discriminator configs
            ref = su.first_ref(field_schema)
            if ref:
                self.discriminator_configs[field_name] = ref
                self.context_discriminator_configs.setdefault(def_name, {})[field_name] = ref
    
    def get_parent_context(self, def_name: str) -> Optional[str]:
        """Get the parent context (keyword name) for a definition.
        
        Derives this from the schema by finding where the definition is referenced.
        """
        # First check the cache
        if not hasattr(self, '_parent_context_cache'):
            self._parent_context_cache = self._build_parent_context_map()
        
        return self._parent_context_cache.get(def_name)
    
    def _build_parent_context_map(self) -> Dict[str, str]:
        """Build a map of definition names to their parent context keywords."""
        context_map = {}
        
        for def_name, def_schema in self.defs.items():
            for field_name, field_schema in def_schema.get("properties", {}).items():
                for ref in su.all_refs(field_schema):
                    if ref and ref not in context_map:
                        context_map[ref] = field_name
        
        # Derive context from definition name for any not found via refs
        for def_name in self.defs.keys():
            if def_name not in context_map:
                context_map[def_name] = self._derive_context_from_name(def_name)
        
        return context_map
    
    def _derive_context_from_name(self, def_name: str) -> str:
        """Derive parent context from definition name by converting to snake_case."""
        import re
        
        # Remove common suffixes
        base = def_name
        for suffix in ["Config", "Selection", "Settings"]:
            if base.endswith(suffix):
                base = base[:-len(suffix)]
                break
        
        # Convert CamelCase to snake_case
        snake = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', base)
        snake = re.sub('([a-z0-9])([A-Z])', r'\1_\2', snake).lower()
        return snake
    
    def find_variable_types(self):
        """Find all variable type keywords from VariablesConfig.
        
        Derives prefixes from field names and checks which types have initial_point.
        """
        var_config = self.defs.get("VariablesConfig", {})
        properties = var_config.get("properties", {})
        
        for field_name, field_schema in properties.items():
            # Check if this field has argument="count" - indicates a variable type
            if "argument" in field_schema and field_schema["argument"] == "count":
                self.variable_types.add(field_name)
                
                # Generate prefix from the field name
                prefix = self._generate_variable_prefix(field_name)
                self.variable_prefixes[field_name] = prefix
                
                # Check if this variable type has initial_point in its definition
                ref_type = None
                if "$ref" in field_schema:
                    ref_type = self.get_ref_name(field_schema["$ref"])
                elif "anyOf" in field_schema:
                    for opt in field_schema["anyOf"]:
                        if "$ref" in opt:
                            ref_type = self.get_ref_name(opt["$ref"])
                            break
                
                if ref_type and ref_type in self.defs:
                    ref_props = self.defs[ref_type].get("properties", {})
                    if "initial_point" in ref_props:
                        self.variable_types_with_initial_point.add(field_name)
    
    def _generate_variable_prefix(self, field_name: str) -> str:
        """Generate a variable descriptor prefix from the field name.
        
        This is an initial pass; collisions are resolved in find_variable_types().
        
        Examples:
            uniform_uncertain -> uuv
            continuous_design -> cdv
            discrete_state_set -> dssv
            histogram_bin_uncertain -> hbuv
        """
        parts = field_name.split("_")
        
        # Special handling for common patterns
        if len(parts) >= 2:
            # For *_uncertain types
            if parts[-1] == "uncertain":
                prefix_parts = parts[:-1]
                if len(prefix_parts) == 1:
                    # Single word before uncertain: use first letter + "uv"
                    prefix = prefix_parts[0][0] + "uv"
                else:
                    # Multiple words: use first letter of each + "uv"
                    prefix = "".join(p[0] for p in prefix_parts) + "uv"
                return prefix
            # For *_design types
            elif parts[-1] == "design":
                prefix = "".join(p[0] for p in parts[:-1]) + "dv"
                return prefix
            # For *_state types
            elif parts[-1] == "state":
                prefix = "".join(p[0] for p in parts[:-1]) + "sv"
                return prefix
            # For discrete_*_range or discrete_*_set
            elif parts[0] == "discrete" and len(parts) >= 3:
                # discrete_design_range -> ddrv, discrete_state_set -> dssv
                prefix = parts[0][0] + parts[1][0] + parts[2][0] + "v"
                return prefix
        
        # Default: first letter of each part + 'v'
        prefix = "".join(p[0] for p in parts) + "v"
        return prefix
    
    def _resolve_prefix_collisions(self):
        """Resolve collisions in variable prefixes by using more letters."""
        max_iterations = 5  # Prevent infinite loops
        
        for iteration in range(max_iterations):
            # Find collisions
            prefix_to_types = {}
            for var_type, prefix in self.variable_prefixes.items():
                if prefix not in prefix_to_types:
                    prefix_to_types[prefix] = []
                prefix_to_types[prefix].append(var_type)
            
            # Check if there are any collisions
            has_collisions = False
            for prefix, types in prefix_to_types.items():
                if len(types) > 1:
                    has_collisions = True
                    # Collision detected - use more letters
                    for var_type in types:
                        parts = var_type.split("_")
                        if len(parts) >= 2 and parts[-1] == "uncertain":
                            prefix_parts = parts[:-1]
                            if len(prefix_parts) >= 1:
                                # Use progressively more letters from first word
                                num_chars = min(2 + iteration, len(prefix_parts[0]))
                                new_prefix = prefix_parts[0][:num_chars] + "".join(p[0] for p in prefix_parts[1:]) + "uv"
                                self.variable_prefixes[var_type] = new_prefix
            
            if not has_collisions:
                break
    
    def _is_number_in_context(self, field_schema: dict) -> bool:
        """Check if a field produces number output, including via argument indirection."""
        if su.array_item_type(field_schema) == "number":
            return True
        if su.is_direct_number(field_schema):
            return True
        # Check argument target type via $ref indirection
        arg_name = field_schema.get("argument")
        if arg_name:
            ref_type = su.first_ref(field_schema)
            if ref_type:
                arg_prop = self.defs.get(ref_type, {}).get("properties", {}).get(arg_name, {})
                if arg_prop and su.is_number(arg_prop):
                    return True
        return False
    
    def find_context_dependent_number_fields(self):
        """Find fields that are number type in some contexts but not others."""
        for def_name, def_schema in self.defs.items():
            context = self.get_parent_context(def_name)
            if not context:
                continue
                
            for field_name, field_schema in def_schema.get("properties", {}).items():
                if self._is_number_in_context(field_schema):
                    self.context_number_fields.setdefault(context, set()).add(field_name)
                
                # Follow union wrappers: variant children appear directly under the
                # parent keyword at runtime, so register their number types under
                # the CURRENT context.
                if "anyOf" in field_schema:
                    for _, vchild_name, vchild_schema in su.iter_variant_children(self.defs, field_schema):
                        if self._is_number_in_context(vchild_schema):
                            self.context_number_fields.setdefault(context, set()).add(vchild_name)
    
    def propagate_anchor_context_fields(self):
        """Propagate context-dependent metadata through anchor/union wrappers.
        
        Anchors and unions are transparent in the DSL grammar — their children appear
        as direct children of the enclosing keyword. This propagates argument fields,
        $ref config types, and argument field definitions through variant wrappers
        to the runtime parent.
        """
        for def_name, def_schema in self.defs.items():
            context = self.get_parent_context(def_name)
            
            for field_name, field_schema in def_schema.get("properties", {}).items():
                if "anyOf" not in field_schema:
                    continue
                
                for _, vchild_name, vchild_schema in su.iter_variant_children(self.defs, field_schema):
                    # Propagate argument fields to runtime parent context
                    varg = vchild_schema.get("argument")
                    if varg and context:
                        self.context_argument_fields.setdefault(context, {}). \
                            setdefault(vchild_name, varg)
                        
                        # Check if argument target is an array
                        vref_type = su.first_ref(vchild_schema)
                        if vref_type:
                            varg_prop = self.defs.get(vref_type, {}).get("properties", {}).get(varg, {})
                            if varg_prop and su.is_array(varg_prop):
                                self.array_argument_keywords.add(vchild_name)
                    
                    # Propagate $ref config types to parent definition
                    child_ref = su.first_ref(vchild_schema)
                    if child_ref:
                        self.context_discriminator_configs.setdefault(def_name, {}). \
                            setdefault(vchild_name, child_ref)
                    
                    # Propagate argument fields by definition
                    if varg:
                        self.context_argument_fields_by_def.setdefault(def_name, {}). \
                            setdefault(vchild_name, varg)
    
    def _get_array_item_type(self, field_schema: dict) -> Optional[str]:
        return su.array_item_type(field_schema)
    
    def _is_direct_number_type(self, field_schema: dict) -> bool:
        return su.is_direct_number(field_schema)
    
    def process_all(self):
        """Process all definitions in the schema."""
        for def_name, def_schema in self.defs.items():
            self.process_definition(def_name, def_schema)
        
        self.find_variable_types()
        self._resolve_prefix_collisions()
        self.find_context_dependent_number_fields()
        self.propagate_anchor_context_fields()
        
        # Build non_array_overrides: definitions where a globally-array field is NOT an array
        for def_name, def_schema in self.defs.items():
            for field_name, field_schema in def_schema.get("properties", {}).items():
                if field_name in self.array_fields and not self.is_array_type(field_schema):
                    self.non_array_overrides.setdefault(def_name, set()).add(field_name)
        
        # Remove context-dependent fields from the global number_fields set.
        # A field that appears as both number and integer type in different contexts
        # should NOT be in the global set - it should only be in context_number_fields.
        context_dependent = self.number_fields & self.integer_fields
        self.number_fields -= context_dependent
        
        # Finalize argument_fields - only include fields that ALWAYS have the argument
        # (i.e., every occurrence of the field has the same argument attribute)
        for field_name, arg_counts in self.field_argument_occurrences.items():
            total = self.field_total_occurrences.get(field_name, 0)
            # If there's exactly one argument name and its count matches total occurrences,
            # this field ALWAYS has this argument
            if len(arg_counts) == 1:
                arg_name, count = list(arg_counts.items())[0]
                if count == total:
                    self.argument_fields[field_name] = arg_name
        
        # Also add aliases to argument_fields (only for fields that always have argument)
        for def_name, def_schema in self.defs.items():
            properties = def_schema.get("properties", {})
            for field_name, field_schema in properties.items():
                if "x-aliases" in field_schema and "argument" in field_schema:
                    # Only add alias if the main field is in argument_fields
                    if field_name in self.argument_fields:
                        arg_name = field_schema["argument"]
                        for alias in field_schema["x-aliases"]:
                            self.argument_fields[alias] = arg_name
    
    def generate_header(self) -> str:
        """Generate the C++ header file content."""
        lines = []
        lines.append("// Generated AST-to-JSON metadata from JSON Schema")
        lines.append("// DO NOT EDIT - Generated by generate_ast_metadata.py")
        lines.append("")
        lines.append("#ifndef DAKOTA_AST_METADATA_HPP")
        lines.append("#define DAKOTA_AST_METADATA_HPP")
        lines.append("")
        lines.append("#include <string>")
        lines.append("#include <set>")
        lines.append("#include <map>")
        lines.append("")
        lines.append("namespace dakota {")
        lines.append("namespace ast_metadata {")
        lines.append("")
        
        # Boolean fields
        lines.append("// Fields that are boolean type")
        lines.append("inline bool is_boolean_field(const std::string& field_name) {")
        lines.append("    static const std::set<std::string> boolean_fields = {")
        for field in sorted(self.boolean_fields):
            lines.append(f'        "{field}",')
        lines.append("    };")
        lines.append("    return boolean_fields.count(field_name) > 0;")
        lines.append("}")
        lines.append("")
        
        # Argument fields - global (fields that ALWAYS have this argument)
        lines.append("// Fields with argument attribute (field_name -> argument_name)")
        lines.append("// Only includes fields that ALWAYS have the argument, not context-dependent ones")
        lines.append("inline std::string get_argument_field(const std::string& field_name, const std::string& parent_context = \"\") {")
        lines.append("    // Context-specific argument fields (for fields that only have argument in some contexts)")
        lines.append("    static const std::map<std::string, std::map<std::string, std::string>> context_argument_fields = {")
        for context, fields in sorted(self.context_argument_fields.items()):
            # Only include context-specific ones (not in global set)
            context_specific = {f: a for f, a in fields.items() if f not in self.argument_fields}
            if context_specific:
                lines.append(f'        {{"{context}", {{')
                for field, arg in sorted(context_specific.items()):
                    lines.append(f'            {{"{field}", "{arg}"}},')
                lines.append("        }},")
        lines.append("    };")
        lines.append("    ")
        lines.append("    // Check context-specific first")
        lines.append("    if (!parent_context.empty()) {")
        lines.append("        auto ctx_it = context_argument_fields.find(parent_context);")
        lines.append("        if (ctx_it != context_argument_fields.end()) {")
        lines.append("            auto field_it = ctx_it->second.find(field_name);")
        lines.append("            if (field_it != ctx_it->second.end()) {")
        lines.append("                return field_it->second;")
        lines.append("            }")
        lines.append("        }")
        lines.append("    }")
        lines.append("    ")
        lines.append("    // Fall back to global argument fields")
        lines.append("    static const std::map<std::string, std::string> argument_fields = {")
        for field, arg in sorted(self.argument_fields.items()):
            lines.append(f'        {{"{field}", "{arg}"}},')
        lines.append("    };")
        lines.append("    auto it = argument_fields.find(field_name);")
        lines.append('    return (it != argument_fields.end()) ? it->second : "";')
        lines.append("}")
        lines.append("")
        
        # Definition-keyed argument fields — resolves context ambiguity
        lines.append("// Argument field lookup by parent definition name (unambiguous)")
        lines.append("// Returns the argument attribute for a property within a specific definition")
        lines.append('inline std::string get_argument_field_by_def(const std::string& parent_def, const std::string& field_name) {')
        lines.append("    static const std::map<std::string, std::map<std::string, std::string>> def_argument_fields = {")
        for def_name, fields in sorted(self.context_argument_fields_by_def.items()):
            lines.append(f'        {{"{def_name}", {{')
            for field, arg in sorted(fields.items()):
                lines.append(f'            {{"{field}", "{arg}"}},')
            lines.append("        }},")
        lines.append("    };")
        lines.append("    auto def_it = def_argument_fields.find(parent_def);")
        lines.append("    if (def_it != def_argument_fields.end()) {")
        lines.append("        auto field_it = def_it->second.find(field_name);")
        lines.append("        if (field_it != def_it->second.end()) {")
        lines.append("            return field_it->second;")
        lines.append("        }")
        lines.append("    }")
        lines.append('    return "";')
        lines.append("}")
        lines.append("")
        
        # Generate set of (def_name, prop_name) where prop is a plain scalar but has
        # argument fields in other contexts — the keyword-level fallback would be wrong
        props_with_args = set()
        for ctx, fields in self.context_argument_fields.items():
            props_with_args.update(fields.keys())
        for field in self.argument_fields:
            props_with_args.add(field)
        
        lines.append("// Returns true if parent_def explicitly owns kw_name as a plain scalar,")
        lines.append("// meaning any argument field from keyword-level context is a false match.")
        lines.append("inline bool is_scalar_in_definition(const std::string& parent_def, const std::string& kw_name) {")
        lines.append("    static const std::map<std::string, std::set<std::string>> scalar_conflicts = {")
        conflict_map = {}
        for def_name, all_props in sorted(self.definition_properties.items()):
            ref_props = set(self.context_discriminator_configs.get(def_name, {}).keys())
            scalar_props = all_props - ref_props
            conflicts = scalar_props & props_with_args
            if conflicts:
                conflict_map[def_name] = conflicts
        for def_name, conflicts in sorted(conflict_map.items()):
            children_str = ", ".join(f'"{c}"' for c in sorted(conflicts))
            lines.append(f'        {{"{def_name}", {{{children_str}}}}},')
        lines.append("    };")
        lines.append("    auto def_it = scalar_conflicts.find(parent_def);")
        lines.append("    if (def_it != scalar_conflicts.end()) {")
        lines.append("        return def_it->second.count(kw_name) > 0;")
        lines.append("    }")
        lines.append("    return false;")
        lines.append("}")
        lines.append("")
        
        # Anchor fields
        lines.append("// Anchor fields - returns anchor name if child belongs in an anchor under parent")
        lines.append("// Keyed by definition name (not keyword name) to avoid cross-context collisions")
        lines.append("inline std::string get_anchor_for_child(const std::string& parent_def, const std::string& child_name) {")
        lines.append("    // Map of definition_name -> anchor -> children")
        lines.append("    static const std::map<std::string, std::map<std::string, std::set<std::string>>> anchor_map = {")
        for parent, anchors in sorted(self.anchor_fields.items()):
            lines.append(f'        {{"{parent}", {{')
            for anchor, children in sorted(anchors.items()):
                children_str = ", ".join(f'"{c}"' for c in sorted(children))
                lines.append(f'            {{"{anchor}", {{{children_str}}}}},')
            lines.append("        }},")
        lines.append("    };")
        lines.append("    ")
        lines.append("    auto parent_it = anchor_map.find(parent_def);")
        lines.append("    if (parent_it != anchor_map.end()) {")
        lines.append("        for (const auto& [anchor_name, children] : parent_it->second) {")
        lines.append("            if (children.count(child_name) > 0) {")
        lines.append("                return anchor_name;")
        lines.append("            }")
        lines.append("        }")
        lines.append("    }")
        lines.append('    return "";')
        lines.append("}")
        lines.append("")
        
        # Anchor child → variant def: for each (parent_def, discriminator_key), 
        # return the variant def that owns that key inside an anchor field.
        # Used by ast_to_json to pick the correct parent_def when converting anchor children.
        lines.append("// For a discriminator key inside an anchor field, return the variant def that owns it.")
        lines.append("inline std::string get_anchor_variant_def(const std::string& parent_def, const std::string& disc_key) {")
        lines.append("    static const std::map<std::string, std::map<std::string, std::string>> m = {")
        # Build (parent_def -> disc_key -> variant_def) map
        import schema_utils as su_local
        acv = {}
        for dn, defn in self.defs.items():
            for fn, fs in defn.get("properties", {}).items():
                if not su_local.is_anchor(fs): continue
                for opt in fs.get("anyOf", []):
                    ref = opt.get("$ref", "")
                    if not ref.startswith("#/$defs/"): continue
                    vdef_name = ref[8:]
                    vdef = self.defs.get(vdef_name, {})
                    for disc_key_name in vdef.get("properties", {}):
                        acv.setdefault(dn, {})[disc_key_name] = vdef_name
        for parent_def_name, disc_map in sorted(acv.items()):
            inner = ", ".join(f'{{"{dk}", "{vd}"}}' for dk, vd in sorted(disc_map.items()))
            lines.append(f'        {{"{parent_def_name}", {{{inner}}}}},')
        lines.append("    };")
        lines.append("    auto it = m.find(parent_def);")
        lines.append("    if (it != m.end()) {")
        lines.append("        auto jt = it->second.find(disc_key);")
        lines.append("        if (jt != it->second.end()) return jt->second;")
        lines.append("    }")
        lines.append('    return "";')
        lines.append("}")
        lines.append("")
        
        # Array fields
        lines.append("// Fields that are array type")
        lines.append("inline bool is_array_field(const std::string& field_name) {")
        lines.append("    static const std::set<std::string> array_fields = {")
        for field in sorted(self.array_fields):
            lines.append(f'        "{field}",')
        lines.append("    };")
        lines.append("    return array_fields.count(field_name) > 0;")
        lines.append("}")
        lines.append("")
        
        # Non-array overrides: fields globally marked array but NOT array in specific definitions
        if self.non_array_overrides:
            lines.append("// Returns true if field_name is NOT an array in this specific parent definition,")
            lines.append("// despite being globally marked as an array field in other contexts.")
            lines.append("inline bool is_non_array_in_definition(const std::string& parent_def, const std::string& field_name) {")
            lines.append("    static const std::map<std::string, std::set<std::string>> overrides = {")
            for def_name, fields in sorted(self.non_array_overrides.items()):
                children_str = ", ".join(f'"{c}"' for c in sorted(fields))
                lines.append(f'        {{"{def_name}", {{{children_str}}}}},')
            lines.append("    };")
            lines.append("    auto it = overrides.find(parent_def);")
            lines.append("    return it != overrides.end() && it->second.count(field_name) > 0;")
            lines.append("}")
        else:
            lines.append("inline bool is_non_array_in_definition(const std::string&, const std::string&) { return false; }")
        lines.append("")
        
        # Number fields (always float)
        lines.append("// Fields that should always output as floating point")
        lines.append("inline bool is_number_field(const std::string& field_name) {")
        lines.append("    static const std::set<std::string> number_fields = {")
        for field in sorted(self.number_fields):
            lines.append(f'        "{field}",')
        lines.append("    };")
        lines.append("    return number_fields.count(field_name) > 0;")
        lines.append("}")
        lines.append("")
        
        # Context-dependent number fields
        lines.append("// Fields that are number type in specific contexts")
        lines.append("inline bool is_context_number_field(const std::string& field_name, const std::string& parent_context) {")
        lines.append("    static const std::map<std::string, std::set<std::string>> context_number_fields = {")
        for context, fields in sorted(self.context_number_fields.items()):
            fields_str = ", ".join(f'"{f}"' for f in sorted(fields))
            lines.append(f'        {{"{context}", {{{fields_str}}}}},')
        lines.append("    };")
        lines.append("    ")
        lines.append("    auto it = context_number_fields.find(parent_context);")
        lines.append("    if (it != context_number_fields.end()) {")
        lines.append("        return it->second.count(field_name) > 0;")
        lines.append("    }")
        lines.append("    return false;")
        lines.append("}")
        lines.append("")
        
        # Combined number field check
        lines.append("// Check if field should output as floating point (considering context)")
        lines.append("inline bool should_output_as_float(const std::string& field_name, const std::string& parent_context = \"\") {")
        lines.append("    return is_number_field(field_name) || is_context_number_field(field_name, parent_context);")
        lines.append("}")
        lines.append("")
        
        # Discriminator config types
        lines.append("// Discriminator keyword to config type mapping (global fallback)")
        lines.append("inline std::string get_discriminator_config_type(const std::string& kw_name) {")
        lines.append("    static const std::map<std::string, std::string> discriminator_configs = {")
        for kw, config in sorted(self.discriminator_configs.items()):
            lines.append(f'        {{"{kw}", "{config}"}},')
        lines.append("    };")
        lines.append("    auto it = discriminator_configs.find(kw_name);")
        lines.append('    return (it != discriminator_configs.end()) ? it->second : "";')
        lines.append("}")
        lines.append("")
        
        # Context-aware config type lookup
        lines.append("// Context-aware config type: (parent_def, property_name) -> config_type")
        lines.append("// Resolves the correct $ref config for a property within a specific definition")
        lines.append("inline std::string get_child_config_type(const std::string& parent_def, const std::string& child_name) {")
        lines.append("    static const std::map<std::string, std::map<std::string, std::string>> context_configs = {")
        for def_name, props in sorted(self.context_discriminator_configs.items()):
            lines.append(f'        {{"{def_name}", {{')
            for prop, config in sorted(props.items()):
                lines.append(f'            {{"{prop}", "{config}"}},')
            lines.append("        }},")
        lines.append("    };")
        lines.append("    auto def_it = context_configs.find(parent_def);")
        lines.append("    if (def_it != context_configs.end()) {")
        lines.append("        auto prop_it = def_it->second.find(child_name);")
        lines.append("        if (prop_it != def_it->second.end()) {")
        lines.append("            return prop_it->second;")
        lines.append("        }")
        lines.append("    }")
        lines.append('    return "";')
        lines.append("}")
        lines.append("")
        
        # Variable types
        lines.append("// Check if keyword is a variable type")
        lines.append("inline bool is_variable_type(const std::string& kw_name) {")
        lines.append("    static const std::set<std::string> variable_types = {")
        for vt in sorted(self.variable_types):
            lines.append(f'        "{vt}",')
        lines.append("    };")
        lines.append("    return variable_types.count(kw_name) > 0;")
        lines.append("}")
        lines.append("")
        
        # Variable prefixes
        lines.append("// Get descriptor prefix for variable type")
        lines.append("inline std::string get_variable_descriptor_prefix(const std::string& var_type) {")
        lines.append("    static const std::map<std::string, std::string> prefixes = {")
        for vt, prefix in sorted(self.variable_prefixes.items()):
            lines.append(f'        {{"{vt}", "{prefix}"}},')
        lines.append("    };")
        lines.append("    auto it = prefixes.find(var_type);")
        lines.append('    return (it != prefixes.end()) ? it->second : "var";')
        lines.append("}")
        lines.append("")
        
        # Variable types with initial_point
        lines.append("// Check if variable type has initial_point field")
        lines.append("inline bool variable_type_has_initial_point(const std::string& var_type) {")
        lines.append("    static const std::set<std::string> types_with_initial_point = {")
        for vt in sorted(self.variable_types_with_initial_point):
            lines.append(f'        "{vt}",')
        lines.append("    };")
        lines.append("    return types_with_initial_point.count(var_type) > 0;")
        lines.append("}")
        lines.append("")
        
        # Scalar argument fields - derive from schema
        # These are argument fields whose target type is not an array
        lines.append("// Argument fields that expect scalar values (not arrays)")
        lines.append("inline bool is_scalar_argument_field(const std::string& arg_field) {")
        lines.append("    static const std::set<std::string> scalar_args = {")
        for arg in sorted(self.scalar_argument_fields):
            lines.append(f'        "{arg}",')
        lines.append("    };")
        lines.append("    return scalar_args.count(arg_field) > 0;")
        lines.append("}")
        lines.append("")
        
        # Keywords whose argument target is an array type
        # Used to override is_scalar_argument_field when the argument field name
        # is ambiguous (e.g., 'sequence' is scalar for collocation_ratio but array
        # for sparse_grid_level_sequence)
        lines.append("// Keywords whose argument target property is an array type")
        lines.append("inline bool has_array_argument(const std::string& kw_name) {")
        lines.append("    static const std::set<std::string> array_arg_kws = {")
        for kw in sorted(self.array_argument_keywords):
            lines.append(f'        "{kw}",')
        lines.append("    };")
        lines.append("    return array_arg_kws.count(kw_name) > 0;")
        lines.append("}")
        
        lines.append("} // namespace ast_metadata")
        lines.append("} // namespace dakota")
        lines.append("")
        lines.append("#endif // DAKOTA_AST_METADATA_HPP")
        
        return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(description="Generate AST metadata from JSON Schema")
    parser.add_argument("schema", help="Path to JSON schema file")
    parser.add_argument("--output-dir", "-o", default=".", help="Output directory")
    args = parser.parse_args()
    
    generator = ASTMetadataGenerator(args.schema)
    generator.process_all()
    
    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    
    header_content = generator.generate_header()
    output_path = output_dir / "dakota_ast_metadata.hpp"
    with open(output_path, "w", encoding='utf-8') as f:
        f.write(header_content)
    
    print(f"Generated {output_path}")
    print(f"  - {len(generator.boolean_fields)} boolean fields")
    print(f"  - {len(generator.argument_fields)} argument fields")
    print(f"  - {len(generator.anchor_fields)} parent contexts with anchors")
    print(f"  - {len(generator.array_fields)} array fields")
    print(f"  - {len(generator.number_fields)} number fields")
    print(f"  - {len(generator.discriminator_configs)} discriminator configs")
    print(f"  - {len(generator.variable_types)} variable types")


if __name__ == "__main__":
    main()
