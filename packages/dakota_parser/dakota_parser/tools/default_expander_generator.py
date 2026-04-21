#!/usr/bin/env python3
"""
Dakota Default Expander Generator

Generates C++ code that expands parsed AST with schema defaults.
This handles:
1. Non-null defaults - always included in output
2. Required fields - error if missing when parent is specified
3. Union defaults (pattern 1 with x-model-default) - auto-instantiate
4. Recursive expansion - populate children of populated parents
"""

import json
from pathlib import Path
from typing import Dict, Any, List, Optional, Set, Tuple

import schema_utils as su


class FieldInfo:
    """Information about a field for default expansion."""
    def __init__(self, name, path, default_value=None, has_non_null_default=False, is_required=False, 
                 is_union=False, union_pattern=None, union_default_variant=None, 
                 union_default_child=None, field_type="unknown", ref_type=None, 
                 children=None, is_union_variant=False, is_discriminator=False, 
                 argument_field=None, is_anchor=False):
        self.name = name
        self.path = path
        self.default_value = default_value
        self.has_non_null_default = has_non_null_default
        self.is_required = is_required
        self.is_union = is_union
        self.union_pattern = union_pattern
        self.union_default_variant = union_default_variant
        self.union_default_child = union_default_child
        self.field_type = field_type
        self.ref_type = ref_type
        self.children = children if children is not None else []
        self.is_union_variant = is_union_variant
        self.is_discriminator = is_discriminator
        self.argument_field = argument_field
        self.is_anchor = is_anchor

class DefaultExpansionSchemaExtractor:
    """Extract default expansion metadata from JSON Schema."""
    
    def __init__(self, schema: Dict[str, Any]):
        self.schema = schema
        self.defs = schema.get('$defs', {})
        
    def resolve_ref(self, ref: str) -> Dict[str, Any]:
        """Resolve a $ref to its definition."""
        return su.resolve(self.defs, ref) or {}
    
    def extract_block_metadata(self, block_name: str) -> Dict[str, FieldInfo]:
        """
        Extract default expansion metadata for a block.
        
        Returns a dict mapping field paths to FieldInfo objects.
        """
        fields: Dict[str, FieldInfo] = {}
        
        # Get block definition
        block_props = self.schema.get('properties', {})
        if block_name not in block_props:
            return fields
        
        block_schema = block_props[block_name]
        
        # Find the config type(s) for this block
        config_refs = self._extract_config_refs(block_schema)
        
        # Check if this block is a union of multiple types (like method with anyOf)
        # If so, the top-level variant names (adaptive_sampling, multifidelity_sampling, etc.)
        # should NOT be required at the block level (only one should be present),
        # but their children should be auto-populated when the parent variant is present.
        is_block_level_union = self._is_block_level_union(block_schema)
        
        for config_ref in config_refs:
            config = self.resolve_ref(config_ref)
            
            # For block-level unions, we need special handling:
            # - The direct children of the block (e.g., method.multifidelity_sampling) are union selectors
            # - They should be marked is_union_variant=True so they're not required
            # - But their children (e.g., method.multifidelity_sampling.max_iterations) should NOT be
            #   union variants so they can be auto-populated
            if is_block_level_union:
                # Process the Selection's properties (the selector fields)
                for prop_name, prop_schema in config.get('properties', {}).items():
                    selector_path = f"{block_name}.{prop_name}"
                    
                    # Create FieldInfo for the selector itself - mark as union variant
                    selector_info = self._create_field_info(prop_name, selector_path, prop_schema, 
                                                           set(config.get('required', [])))
                    selector_info.is_union_variant = True  # Key: mark as union variant
                    selector_info.is_required = False  # Not globally required
                    fields[selector_path] = selector_info
                    
                    # Now recursively process children with in_union_variant=False
                    # so they can be auto-populated
                    if selector_info.ref_type:
                        nested_config = self.defs.get(selector_info.ref_type, {})
                        if 'properties' in nested_config:
                            self._extract_fields_recursive(nested_config, selector_path, fields, 
                                                          in_union_variant=False)
                    
                    # For unions within the selector, also process those
                    if selector_info.is_union:
                        self._extract_union_variants(prop_schema, selector_path, fields)
            else:
                # Non-union blocks process normally
                self._extract_fields_recursive(config, block_name, fields, 
                                              in_union_variant=False)
        
        return fields
    
    def _is_block_level_union(self, block_schema: Dict[str, Any]) -> bool:
        """
        Check if a block schema represents a union of multiple types.
        
        A block is a union if:
        - It has anyOf with multiple $ref options (excluding null)
        - Or it's an array with items that have anyOf with multiple $ref options
        
        This is typical for blocks like 'method' which can be many different method types.
        """
        refs = []
        
        if 'anyOf' in block_schema:
            for option in block_schema['anyOf']:
                if '$ref' in option:
                    refs.append(option['$ref'])
                elif 'items' in option and '$ref' in option.get('items', {}):
                    refs.append(option['items']['$ref'])
                elif 'items' in option and 'anyOf' in option.get('items', {}):
                    # Array of union items - this is the 'method' pattern
                    for item_opt in option['items']['anyOf']:
                        if '$ref' in item_opt:
                            refs.append(item_opt['$ref'])
        elif 'items' in block_schema:
            items = block_schema['items']
            if 'anyOf' in items:
                for item_opt in items['anyOf']:
                    if '$ref' in item_opt:
                        refs.append(item_opt['$ref'])
        
        # If there are 2+ refs, this is a union of types
        return len(refs) > 1
    
    def _extract_config_refs(self, schema: Dict[str, Any]) -> List[str]:
        """Extract all $ref config types from a schema (handles arrays and anyOf)."""
        refs = []
        
        if 'anyOf' in schema:
            for option in schema['anyOf']:
                if '$ref' in option:
                    refs.append(option['$ref'])
                elif 'items' in option:
                    items = option['items']
                    if '$ref' in items:
                        refs.append(items['$ref'])
                    elif 'anyOf' in items:
                        for item_opt in items['anyOf']:
                            if '$ref' in item_opt:
                                refs.append(item_opt['$ref'])
        elif 'items' in schema:
            items = schema['items']
            if '$ref' in items:
                refs.append(items['$ref'])
            elif 'anyOf' in items:
                for item_opt in items['anyOf']:
                    if '$ref' in item_opt:
                        refs.append(item_opt['$ref'])
        elif '$ref' in schema:
            refs.append(schema['$ref'])
        
        return refs
    
    def _extract_fields_recursive(self, config: Dict[str, Any], parent_path: str, 
                                   fields: Dict[str, FieldInfo], 
                                   in_union_variant: bool = False):
        """
        Recursively extract field metadata from a config.
        
        Args:
            config: The config schema to process
            parent_path: Parent path for building field paths
            fields: Output dict to populate
            in_union_variant: If True, fields are inside a union variant (conditionally required)
        """
        properties = config.get('properties', {})
        required = set(config.get('required', []))
        
        for field_name, field_schema in properties.items():
            field_path = f"{parent_path}.{field_name}"
            
            # Create FieldInfo
            info = self._create_field_info(field_name, field_path, field_schema, required)
            
            # If we're inside a union variant, mark the field and clear is_required
            # (required within a variant only applies if that variant is selected)
            if in_union_variant:
                info.is_union_variant = True
                info.is_required = False
            
            fields[field_path] = info
            
            # Recursively process nested configs (still in_union_variant if we were)
            if info.ref_type:
                nested_config = self.defs.get(info.ref_type, {})
                if 'properties' in nested_config:
                    self._extract_fields_recursive(nested_config, field_path, fields, in_union_variant)
            
            # For unions, also process each variant
            if info.is_union:
                self._extract_union_variants(field_schema, field_path, fields)
    
    def _create_field_info(self, name: str, path: str, schema: Dict[str, Any], 
                           required: Set[str]) -> FieldInfo:
        """Create a FieldInfo from a field schema."""
        info = FieldInfo(name=name, path=path)
        info.is_required = name in required
        
        # Extract default
        if 'default' in schema:
            info.default_value = schema['default']
            info.has_non_null_default = schema['default'] is not None
        
        # Extract argument attribute (means param_values become this nested field)
        if 'argument' in schema:
            info.argument_field = schema['argument']
        
        # Check for anchor attribute (field exists only in JSON, not in DSL)
        if schema.get('anchor', False):
            info.is_anchor = True
            # Anchor fields are not required for DSL validation - they're synthesized
            info.is_required = False
        
        # Check for union pattern
        if 'x-union-pattern' in schema:
            info.is_union = True
            info.union_pattern = schema['x-union-pattern']
            info.field_type = 'union'
            
            if 'x-model-default' in schema:
                info.union_default_variant = schema['x-model-default']
                
                # Look up the variant definition to find the actual property name and value
                variant_def = self.defs.get(info.union_default_variant, {})
                variant_props = variant_def.get('properties', {})
                
                # Most union default variants have a single property with const: true
                # e.g., Abort has {"abort": {"const": true, "default": true, "type": "boolean"}}
                # Some have a $ref to a config object (e.g., method_source -> dakota -> DakotaConfig)
                for prop_name, prop_schema in variant_props.items():
                    # Use the property's default value, or True if it's a boolean const
                    if 'default' in prop_schema:
                        info.union_default_child = (prop_name, prop_schema['default'])
                        break
                    elif prop_schema.get('const') is True:
                        info.union_default_child = (prop_name, True)
                        break
                    elif prop_schema.get('type') == 'boolean':
                        # If it's a boolean without explicit default, use True
                        info.union_default_child = (prop_name, True)
                        break
                    elif '$ref' in prop_schema:
                        # Object variant (e.g., method_source -> dakota -> Config)
                        # Create the node; recursive default expansion fills children
                        info.union_default_child = (prop_name, True)
                        break
        
        # Determine field type and ref
        ref = su.first_ref(schema)
        if ref and not info.is_union:
            info.ref_type = ref
            info.field_type = 'object'
        elif not ref:
            if 'anyOf' in schema:
                for option in schema['anyOf']:
                    if 'type' in option and option['type'] != 'null':
                        info.field_type = self._map_json_type(option['type'])
                        break
            elif 'type' in schema:
                info.field_type = self._map_json_type(schema['type'])
        
        return info
    
    def _map_json_type(self, json_type: str) -> str:
        """Map JSON type to field type."""
        if json_type == 'array':
            return 'array'
        elif json_type in ('string', 'integer', 'number', 'boolean'):
            return 'primitive'
        elif json_type == 'object':
            return 'object'
        return 'unknown'
    
    def _extract_union_variants(self, union_schema: Dict[str, Any], parent_path: str,
                                 fields: Dict[str, FieldInfo]):
        """
        Extract fields from each union variant.
        
        These fields are conditionally required - only required if their variant is selected.
        We mark them with is_union_variant=True and is_required=False.
        """
        if 'anyOf' not in union_schema:
            return
        
        for variant_ref_name in su.all_refs(union_schema):
            variant_config = self.defs.get(variant_ref_name, {})
            if 'properties' not in variant_config:
                continue
            
            for disc_name, disc_schema in variant_config['properties'].items():
                disc_path = f"{parent_path}.{disc_name}"
                
                disc_info = FieldInfo(
                    name=disc_name,
                    path=disc_path,
                    is_union_variant=True,
                    is_discriminator=True,
                    is_required=False
                )
                
                # Classify discriminator type
                if '$ref' in disc_schema:
                    # Direct $ref — simple object
                    disc_info.ref_type = su.ref_name(disc_schema['$ref'])
                    disc_info.field_type = 'object'
                elif 'const' in disc_schema:
                    disc_info.field_type = 'primitive'
                    disc_info.has_non_null_default = True
                    disc_info.default_value = disc_schema.get('default', disc_schema['const'])
                elif 'anyOf' in disc_schema:
                    refs = su.all_refs(disc_schema)
                    if len(refs) > 1:
                        # Discriminator is itself a union (e.g., quasi_hessians → bfgs/sr1)
                        disc_info.is_union = True
                        disc_info.union_pattern = disc_schema.get('x-union-pattern', 0)
                        disc_info.field_type = 'union'
                        if 'x-model-default' in disc_schema:
                            disc_info.union_default_variant = disc_schema['x-model-default']
                
                fields[disc_path] = disc_info
                
                # Recurse into config children
                if disc_info.ref_type:
                    disc_config = self.defs.get(disc_info.ref_type, {})
                    if 'properties' in disc_config:
                        self._extract_fields_recursive(disc_config, disc_path, fields, in_union_variant=True)
                
                # If discriminator is a union, recurse into its variants
                if disc_info.is_union:
                    self._extract_union_variants(disc_schema, disc_path, fields)


def generate_default_metadata_header(block_name: str, fields: Dict[str, FieldInfo], 
                                      output_dir: Path) -> None:
    """Generate C++ header with default expansion metadata.
    
    IMPORTANT: All paths are converted to AST-level paths by stripping anchor segments.
    Anchor fields exist only in the JSON Schema (not in the Dakota DSL), so the default
    expander's populated_paths (built from the AST) never contain anchor segments.
    """
    
    header_path = output_dir / block_name / f"dakota_defaults_{block_name}.hpp"
    header_path.parent.mkdir(parents=True, exist_ok=True)
    
    guard = f"DAKOTA_DEFAULTS_{block_name.upper()}_HPP"
    
    # Build set of anchor paths for path stripping
    anchor_paths = set()
    for path, info in fields.items():
        if info.is_anchor:
            anchor_paths.add(path)
    
    def strip_anchor_segments(path: str) -> str:
        """Strip anchor path segments to produce an AST-level path.
        
        Example: 'interface.analysis_drivers.interface_type.system.file_tag'
        where 'interface.analysis_drivers.interface_type' is an anchor becomes:
        'interface.analysis_drivers.system.file_tag'
        """
        parts = path.split('.')
        result = []
        current = ''
        for part in parts:
            candidate = (current + '.' + part) if current else part
            if candidate in anchor_paths:
                # This segment is an anchor - skip it in the AST path
                current = candidate
            else:
                result.append(part)
                current = candidate
        return '.'.join(result)
    
    lines = []
    lines.append(f"#ifndef {guard}")
    lines.append(f"#define {guard}")
    lines.append("")
    lines.append('#include "dakota_default_types.hpp"')
    lines.append('#include <string>')
    lines.append('#include <map>')
    lines.append('#include <set>')
    lines.append('#include <vector>')
    lines.append("")
    lines.append("namespace dakota {")
    lines.append(f"namespace {block_name}_defaults {{")
    lines.append("")
    
    # Use common types
    lines.append("// Use common types from dakota_default_types.hpp")
    lines.append("using defaults::DefaultValue;")
    lines.append("using defaults::FieldMetadata;")
    lines.append("")
    
    # Generate metadata map (with AST-level paths)
    lines.append("// Get all field metadata for this block")
    lines.append("// NOTE: All paths are AST-level (anchor segments stripped)")
    lines.append("inline const std::map<std::string, FieldMetadata>& get_field_metadata() {")
    lines.append("    static const std::map<std::string, FieldMetadata> metadata = {")
    
    for path, info in sorted(fields.items()):
        # Skip anchor fields - they don't exist as AST nodes.
        # Their schema-level structure is handled during JSON conversion.
        if info.is_anchor:
            continue
        
        ast_path = strip_anchor_segments(path)
        
        # Format default value
        default_str = "std::nullopt"
        if info.has_non_null_default and info.default_value is not None:
            if isinstance(info.default_value, bool):
                default_str = f"DefaultValue{{{str(info.default_value).lower()}}}"
            elif isinstance(info.default_value, int):
                # Use LL suffix to ensure int64_t literal
                default_str = f"DefaultValue{{std::int64_t{{{info.default_value}LL}}}}"
            elif isinstance(info.default_value, float):
                default_str = f"DefaultValue{{{info.default_value}}}"
            elif isinstance(info.default_value, str):
                escaped = info.default_value.replace('\\', '\\\\').replace('"', '\\"')
                default_str = f'DefaultValue{{std::string("{escaped}")}}'
        
        # Use actual child property name (e.g., "abort") not definition name (e.g., "Abort")
        union_default = ""
        if info.union_default_child:
            union_default = info.union_default_child[0]
        elif info.union_default_variant:
            union_default = info.union_default_variant
        
        ref_type = info.ref_type or ""
        argument_field = info.argument_field or ""
        
        lines.append(f'        {{"{ast_path}", FieldMetadata{{')
        lines.append(f'            "{ast_path}",')
        lines.append(f'            {str(info.has_non_null_default).lower()},')
        lines.append(f'            {default_str},')
        lines.append(f'            {str(info.is_required).lower()},')
        lines.append(f'            {str(info.is_union).lower()},')
        lines.append(f'            {info.union_pattern or 0},')
        lines.append(f'            "{union_default}",')
        lines.append(f'            "{info.field_type}",')
        lines.append(f'            "{ref_type}",')
        lines.append(f'            "{argument_field}",')
        lines.append(f'            {str(info.is_anchor).lower()},')
        lines.append(f'            {str(info.is_union_variant).lower()}')
        lines.append(f'        }}}},')
    
    lines.append("    };")
    lines.append("    return metadata;")
    lines.append("}")
    lines.append("")
    
    # Generate helper to get fields with non-null defaults
    # Exclude union discriminator fields — they identify which variant is selected
    # and should never be auto-populated (only present when user selects that variant
    # or when added by the union_defaults pass for x-model-default).
    # Non-discriminator variant children (like system.file_save) ARE included,
    # gated by parent-is-populated check at runtime.
    lines.append("// Get paths of all fields with non-null defaults")
    lines.append("// Excludes union discriminators and anchor fields")
    lines.append("inline const std::set<std::string>& get_auto_populate_fields() {")
    lines.append("    static const std::set<std::string> fields = {")
    
    for path, info in sorted(fields.items()):
        if info.has_non_null_default:
            # Skip anchor fields — they don't exist in the AST
            if info.is_anchor:
                continue
            # Skip union discriminators — they must not be auto-populated
            if info.is_discriminator:
                continue
            ast_path = strip_anchor_segments(path)
            lines.append(f'        "{ast_path}",')
    
    lines.append("    };")
    lines.append("    return fields;")
    lines.append("}")
    lines.append("")
    
    # Generate helper for union defaults (pattern 1 with x-model-default)
    # NOTE: Paths are AST-level (anchor segments already stripped by shared function above)
    lines.append("// Get union fields that auto-instantiate their default variant")
    lines.append("// Returns: path -> (pattern, child_property_name)")
    lines.append("// The child_property_name is the actual property to create (e.g., 'abort' not 'Abort')")
    lines.append("// NOTE: Paths are AST-level (anchor segments stripped) to match populated_paths")
    lines.append("inline const std::map<std::string, std::pair<int, std::string>>& get_union_defaults() {")
    lines.append("    static const std::map<std::string, std::pair<int, std::string>> unions = {")
    
    for path, info in sorted(fields.items()):
        if info.is_union and info.union_pattern == 1 and info.union_default_variant:
            if info.is_anchor:
                continue  # Handled in get_anchor_union_defaults below
            if info.union_default_child:
                child_name, _ = info.union_default_child
                ast_path = strip_anchor_segments(path)
                lines.append(f'        {{"{ast_path}", {{{info.union_pattern}, "{child_name}"}}}},')
    
    lines.append("    };")
    lines.append("    return unions;")
    lines.append("}")
    lines.append("")
    
    # Generate helper for anchor-union defaults
    # When a union field itself is an anchor (doesn't exist in DSL), the default variant's
    # child must be added directly to the anchor's parent as a flat flag node.
    # E.g., schema path 'method.X.import_build_points_file.format' where 'format' is anchor+union:
    #   → AST parent: 'method.X.import_build_points_file'
    #   → Add child 'annotated' (flag=true) directly to import_build_points_file
    # But ONLY if no other variant (freeform, custom_annotated) is already present.
    lines.append("// Anchor-union defaults: the union field itself is an anchor (not in DSL)")
    lines.append("// The variant child is added as a flat flag directly to the parent node")
    lines.append("// Returns: parent_ast_path -> {default_child, all_variant_names}")
    lines.append("// If ANY variant is already populated, the default should NOT be added")
    lines.append("inline const std::map<std::string, std::pair<std::string, std::vector<std::string>>>& get_anchor_union_defaults() {")
    lines.append("    static const std::map<std::string, std::pair<std::string, std::vector<std::string>>> unions = {")
    
    for path, info in sorted(fields.items()):
        if info.is_union and info.union_pattern == 1 and info.union_default_variant and info.is_anchor:
            if info.union_default_child:
                child_name, _ = info.union_default_child
                # The path IS the anchor, so stripping it gives the parent
                ast_parent_path = strip_anchor_segments(path)
                # Collect all variant discriminator names from sibling fields
                variant_names = []
                for other_path, other_info in fields.items():
                    if other_info.is_discriminator and other_path.startswith(path + "."):
                        vname = other_path[len(path)+1:]
                        if "." not in vname:  # Only direct children
                            variant_names.append(vname)
                variant_list = ", ".join(f'"{v}"' for v in sorted(variant_names))
                lines.append(f'        {{"{ast_parent_path}", {{"{child_name}", {{{variant_list}}}}}}},')
    
    lines.append("    };")
    lines.append("    return unions;")
    lines.append("}")
    lines.append("")
    
    # Generate helper for required fields
    lines.append("// Get paths of all required fields")
    lines.append("inline const std::set<std::string>& get_required_fields() {")
    lines.append("    static const std::set<std::string> fields = {")
    
    for path, info in sorted(fields.items()):
        if info.is_required:
            ast_path = strip_anchor_segments(path)
            lines.append(f'        "{ast_path}",')
    
    lines.append("    };")
    lines.append("    return fields;")
    lines.append("}")
    lines.append("")
    
    lines.append(f"}} // namespace {block_name}_defaults")
    lines.append("} // namespace dakota")
    lines.append("")
    lines.append(f"#endif // {guard}")
    lines.append("")
    
    header_path.write_text('\n'.join(lines))
    print(f"  Generated {header_path.name}")


def generate_default_expander_header(output_dir: Path) -> None:
    """Generate the main default expander C++ header."""
    
    header_path = output_dir / "dakota_default_expander.hpp"
    
    content = '''#ifndef DAKOTA_DEFAULT_EXPANDER_HPP
#define DAKOTA_DEFAULT_EXPANDER_HPP

#include "dakota_ast.hpp"
#include "dakota_default_types.hpp"
#include <string>
#include <map>
#include <set>
#include <vector>
#include <functional>
#include <stdexcept>

namespace dakota {

/**
 * Default Expander
 * 
 * Expands a parsed AST with schema defaults:
 * 1. Adds fields with non-null defaults
 * 2. Instantiates union default variants (pattern 1 with x-model-default)
 * 3. Validates required fields are present
 * 4. Recursively expands children of populated parents
 */
class DefaultExpander {
public:
    // Use common types from dakota_default_types.hpp
    using DefaultValue = defaults::DefaultValue;
    using FieldMetadata = defaults::FieldMetadata;
    
    // Expand a document with defaults
    static bool expand_document(Document& doc, std::vector<std::string>& errors);
    
    // Expand a single block with its defaults
    static bool expand_block(Block& block, 
                             const std::map<std::string, FieldMetadata>& metadata,
                             const std::set<std::string>& auto_populate,
                             const std::map<std::string, std::pair<int, std::string>>& union_defaults,
                             const std::map<std::string, std::pair<std::string, std::vector<std::string>>>& anchor_union_defaults,
                             const std::set<std::string>& required_fields,
                             std::vector<std::string>& errors);
    
private:
    // Expand a keyword node recursively
    static void expand_keyword(std::shared_ptr<KeywordNode>& node,
                               const std::string& current_path,
                               const std::map<std::string, FieldMetadata>& metadata,
                               const std::set<std::string>& auto_populate,
                               const std::map<std::string, std::pair<int, std::string>>& union_defaults,
                               std::set<std::string>& populated_paths);
    
    // Check required fields and report errors
    static bool validate_required(const std::set<std::string>& populated_paths,
                                  const std::set<std::string>& required_fields,
                                  const std::string& block_name,
                                  std::vector<std::string>& errors);
    
    // Create a keyword node with default value
    static std::shared_ptr<KeywordNode> create_default_node(
        const std::string& name,
        const FieldMetadata& metadata);
    
    // Create a union default variant node
    static std::shared_ptr<KeywordNode> create_union_default_node(
        const std::string& name,
        const std::string& variant_name,
        const std::string& union_path,
        const std::map<std::string, FieldMetadata>& all_metadata);
};

} // namespace dakota

#endif // DAKOTA_DEFAULT_EXPANDER_HPP
'''
    
    header_path.write_text(content)
    print(f"  Generated {header_path.name}")


def generate_default_types_header(output_dir: Path) -> None:
    """Generate common types header used by all blocks and the expander."""
    
    header_path = output_dir / "dakota_default_types.hpp"
    
    content = '''#ifndef DAKOTA_DEFAULT_TYPES_HPP
#define DAKOTA_DEFAULT_TYPES_HPP

#include <string>
#include <variant>
#include <optional>
#include <cstdint>

namespace dakota {
namespace defaults {

// Default value can be bool, int64_t, double, or string
using DefaultValue = std::variant<bool, std::int64_t, double, std::string>;

// Metadata for a single field
struct FieldMetadata {
    std::string path;
    bool has_non_null_default = false;
    std::optional<DefaultValue> default_value;
    bool is_required = false;
    bool is_union = false;
    int union_pattern = 0;
    std::string union_default_variant;  // x-model-default
    std::string field_type;  // primitive, object, array, union
    std::string ref_type;  // $def name if object ref
    std::string argument_field;  // If set, parent's param_values provide this nested field's value
    bool is_anchor = false;  // True if field exists only in JSON (not in DSL)
    bool is_union_variant = false;  // True if this field is a variant child of a union (should not be auto-populated)
};

} // namespace defaults
} // namespace dakota

#endif // DAKOTA_DEFAULT_TYPES_HPP
'''
    
    header_path.write_text(content)
    print(f"  Generated {header_path.name}")


def generate_default_expander_cpp(output_dir: Path) -> None:
    """Generate the default expander C++ implementation."""
    
    cpp_path = output_dir / "dakota_default_expander.cpp"
    
    content = '''#include "dakota_default_expander.hpp"

// Include block-specific default metadata
#include "environment/dakota_defaults_environment.hpp"
#include "method/dakota_defaults_method.hpp"
#include "model/dakota_defaults_model.hpp"
#include "variables/dakota_defaults_variables.hpp"
#include "interface/dakota_defaults_interface.hpp"
#include "responses/dakota_defaults_responses.hpp"

namespace dakota {

bool DefaultExpander::expand_document(Document& doc, std::vector<std::string>& errors) {
    bool success = true;
    
    for (auto& block : doc.blocks) {
        // Get metadata for this block type
        const std::map<std::string, FieldMetadata>* metadata = nullptr;
        const std::set<std::string>* auto_populate = nullptr;
        const std::map<std::string, std::pair<int, std::string>>* union_defaults = nullptr;
        const std::map<std::string, std::pair<std::string, std::vector<std::string>>>* anchor_union_defaults = nullptr;
        const std::set<std::string>* required_fields = nullptr;
        
        if (block.name == "environment") {
            metadata = &environment_defaults::get_field_metadata();
            auto_populate = &environment_defaults::get_auto_populate_fields();
            union_defaults = &environment_defaults::get_union_defaults();
            anchor_union_defaults = &environment_defaults::get_anchor_union_defaults();
            required_fields = &environment_defaults::get_required_fields();
        } else if (block.name == "method") {
            metadata = &method_defaults::get_field_metadata();
            auto_populate = &method_defaults::get_auto_populate_fields();
            union_defaults = &method_defaults::get_union_defaults();
            anchor_union_defaults = &method_defaults::get_anchor_union_defaults();
            required_fields = &method_defaults::get_required_fields();
        } else if (block.name == "model") {
            metadata = &model_defaults::get_field_metadata();
            auto_populate = &model_defaults::get_auto_populate_fields();
            union_defaults = &model_defaults::get_union_defaults();
            anchor_union_defaults = &model_defaults::get_anchor_union_defaults();
            required_fields = &model_defaults::get_required_fields();
        } else if (block.name == "variables") {
            metadata = &variables_defaults::get_field_metadata();
            auto_populate = &variables_defaults::get_auto_populate_fields();
            union_defaults = &variables_defaults::get_union_defaults();
            anchor_union_defaults = &variables_defaults::get_anchor_union_defaults();
            required_fields = &variables_defaults::get_required_fields();
        } else if (block.name == "interface") {
            metadata = &interface_defaults::get_field_metadata();
            auto_populate = &interface_defaults::get_auto_populate_fields();
            union_defaults = &interface_defaults::get_union_defaults();
            anchor_union_defaults = &interface_defaults::get_anchor_union_defaults();
            required_fields = &interface_defaults::get_required_fields();
        } else if (block.name == "responses") {
            metadata = &responses_defaults::get_field_metadata();
            auto_populate = &responses_defaults::get_auto_populate_fields();
            union_defaults = &responses_defaults::get_union_defaults();
            anchor_union_defaults = &responses_defaults::get_anchor_union_defaults();
            required_fields = &responses_defaults::get_required_fields();
        }
        
        if (metadata) {
            if (!expand_block(block, *metadata, *auto_populate, *union_defaults, 
                              *anchor_union_defaults, *required_fields, errors)) {
                success = false;
            }
        }
    }
    
    return success;
}

bool DefaultExpander::expand_block(
    Block& block,
    const std::map<std::string, FieldMetadata>& metadata,
    const std::set<std::string>& auto_populate,
    const std::map<std::string, std::pair<int, std::string>>& union_defaults,
    const std::map<std::string, std::pair<std::string, std::vector<std::string>>>& anchor_union_defaults,
    const std::set<std::string>& required_fields,
    std::vector<std::string>& errors) 
{
    std::set<std::string> populated_paths;
    populated_paths.insert(block.name);  // The block itself is populated
    
    // First pass: mark all existing paths as populated
    for (const auto& [kw_name, kw_nodes] : block.keywords) {
        for (const auto& node : kw_nodes) {
            std::string path = block.name + "." + node->effective_name();
            populated_paths.insert(path);
            
            // Check if this node has param_values and an argument field in metadata
            // If so, the argument field is satisfied by the param_values
            auto meta_it = metadata.find(path);
            if (meta_it != metadata.end() && !meta_it->second.argument_field.empty()) {
                if (!node->param_values.empty()) {
                    // The argument field is satisfied
                    std::string arg_path = path + "." + meta_it->second.argument_field;
                    populated_paths.insert(arg_path);
                }
            }
            
            // Recursively mark children (and check for argument fields)
            std::function<void(const std::shared_ptr<KeywordNode>&, const std::string&)> 
            mark_children = [&](const auto& n, const std::string& p) {
                for (const auto& [child_name, child_nodes] : n->children) {
                    for (const auto& child : child_nodes) {
                        std::string child_path = p + "." + child->effective_name();
                        populated_paths.insert(child_path);
                        
                        // Check for argument field on this child
                        auto child_meta_it = metadata.find(child_path);
                        if (child_meta_it != metadata.end() && !child_meta_it->second.argument_field.empty()) {
                            if (!child->param_values.empty()) {
                                std::string arg_path = child_path + "." + child_meta_it->second.argument_field;
                                populated_paths.insert(arg_path);
                            }
                        }
                        
                        mark_children(child, child_path);
                    }
                }
            };
            mark_children(node, path);
        }
    }
    
    // Second pass: add non-null defaults for block-level fields
    for (const auto& field_path : auto_populate) {
        // Check if this is a direct child of the block
        size_t dot_pos = field_path.find('.', block.name.length() + 1);
        if (dot_pos == std::string::npos) {
            // Direct child of block
            if (populated_paths.find(field_path) == populated_paths.end()) {
                // Not yet populated - add it
                auto it = metadata.find(field_path);
                if (it != metadata.end()) {
                    auto node = create_default_node(it->second.path.substr(block.name.length() + 1), 
                                                    it->second);
                    if (node) {
                        block.keywords[node->name].push_back(node);
                        populated_paths.insert(field_path);
                    }
                }
            }
        }
    }
    
    // Reusable: add nested defaults when parent is populated
    // e.g., when "deactivate" is populated, add "active_set_vector" with its default
    // Called after initial population, and again after union/anchor-union defaults add new nodes
    auto populate_nested_defaults = [&]() {
        for (const auto& field_path : auto_populate) {
            // Skip if already populated
            if (populated_paths.find(field_path) != populated_paths.end()) {
                continue;
            }
            
            // Find the parent path
            size_t last_dot = field_path.rfind('.');
            if (last_dot == std::string::npos) continue;
            
            std::string parent_path = field_path.substr(0, last_dot);
            
            // Check if parent is populated
            if (populated_paths.find(parent_path) == populated_paths.end()) {
                continue;
            }
            
            // Parent is populated - add this default to the parent
            auto meta_it = metadata.find(field_path);
            if (meta_it == metadata.end()) continue;
            
            std::string field_name = field_path.substr(last_dot + 1);
            
            // Find the parent node and add default child
            std::function<bool(std::shared_ptr<KeywordNode>&, const std::string&, const std::string&)>
            add_to_parent = [&](auto& node, const std::string& current_path, const std::string& target_parent) -> bool {
                if (current_path == target_parent) {
                    // Found the parent - add the default child
                    auto child = create_default_node(field_name, meta_it->second);
                    if (child) {
                        node->add_child(field_name, child);
                        populated_paths.insert(field_path);
                        return true;
                    }
                }
                // Search children
                for (auto& [child_name, child_list] : node->children) {
                    for (auto& child : child_list) {
                        std::string child_path = current_path + "." + child->effective_name();
                        if (add_to_parent(child, child_path, target_parent)) {
                            return true;
                        }
                    }
                }
                return false;
            };
            
            // Search block keywords
            for (auto& [kw_name, kw_list] : block.keywords) {
                for (auto& kw : kw_list) {
                    std::string kw_path = block.name + "." + kw->effective_name();
                    if (add_to_parent(kw, kw_path, parent_path)) {
                        break;
                    }
                }
            }
        }
    };
    
    // Second pass (part 2): add nested defaults when parent is populated
    populate_nested_defaults();
    
    // Third pass: add union defaults (pattern 1)
    // Union defaults should only be added when:
    // 1. The union's parent is populated (user specified something at that level)
    // 2. The union itself is NOT populated (user didn't specify this particular union)
    for (const auto& union_entry : union_defaults) {
        const std::string& union_path = union_entry.first;
        const auto& pattern_and_variant = union_entry.second;
        
        size_t last_dot = union_path.rfind('.');
        if (last_dot == std::string::npos) continue;
        
        std::string parent_path = union_path.substr(0, last_dot);
        std::string union_name = union_path.substr(last_dot + 1);
        
        // Check if parent is populated but union is not
        if (populated_paths.find(parent_path) == populated_paths.end()) continue;
        if (populated_paths.find(union_path) != populated_paths.end()) continue;
        
        // Create the default variant node
        auto default_node = create_union_default_node(union_name, pattern_and_variant.second, union_path, metadata);
        if (!default_node) continue;
        
        // Copy union_path for lambda capture (C++17 doesn't allow capturing structured bindings)
        std::string union_path_copy = union_path;
        std::string variant_name_copy = pattern_and_variant.second;
        
        // Find the parent and add this default child
        if (parent_path == block.name) {
            // Direct child of block
            block.keywords[union_name].push_back(default_node);
            populated_paths.insert(union_path_copy);
            // Also mark the variant child as populated so its children get defaults
            populated_paths.insert(union_path_copy + "." + variant_name_copy);
        } else {
            // Nested union - find parent node and add to it
            std::function<bool(std::shared_ptr<KeywordNode>&, const std::string&, const std::string&)>
            add_to_nested_parent = [&, union_path_copy, variant_name_copy](auto& node, const std::string& current_path, const std::string& target_parent) -> bool {
                if (current_path == target_parent) {
                    node->add_child(union_name, default_node);
                    populated_paths.insert(union_path_copy);
                    populated_paths.insert(union_path_copy + "." + variant_name_copy);
                    return true;
                }
                for (auto& child_entry : node->children) {
                    for (auto& child : child_entry.second) {
                        std::string child_path = current_path + "." + child->effective_name();
                        if (add_to_nested_parent(child, child_path, target_parent)) {
                            return true;
                        }
                    }
                }
                return false;
            };
            
            for (auto& kw_entry : block.keywords) {
                for (auto& kw : kw_entry.second) {
                    std::string kw_path = block.name + "." + kw->effective_name();
                    if (add_to_nested_parent(kw, kw_path, parent_path)) {
                        goto next_union;  // Found and added, move to next union
                    }
                }
            }
            next_union:;
        }
    }
    
    // Fourth pass: add anchor-union defaults
    // For union fields that are themselves anchors (don't exist in DSL),
    // add the default variant child directly as a flat flag to the parent node.
    // But ONLY if no other variant from the same union is already present.
    for (const auto& [parent_path, variant_info] : anchor_union_defaults) {
        const auto& child_name = variant_info.first;
        const auto& all_variants = variant_info.second;
        
        // Check if parent is populated
        if (populated_paths.find(parent_path) == populated_paths.end()) continue;
        
        // Check if ANY variant is already populated (not just the default)
        bool any_variant_present = false;
        for (const auto& variant : all_variants) {
            std::string variant_path = parent_path + "." + variant;
            if (populated_paths.find(variant_path) != populated_paths.end()) {
                any_variant_present = true;
                break;
            }
        }
        if (any_variant_present) continue;
        
        // Create a flat flag node for the variant child
        std::string child_path = parent_path + "." + child_name;
        auto variant_node = std::make_shared<KeywordNode>();
        variant_node->name = child_name;
        variant_node->is_flag = true;
        variant_node->param_values.push_back(true);
        
        // Find the parent and add this child
        if (parent_path == block.name) {
            block.keywords[child_name].push_back(variant_node);
            populated_paths.insert(child_path);
        } else {
            std::function<bool(std::shared_ptr<KeywordNode>&, const std::string&, const std::string&)>
            find_parent = [&](auto& node, const std::string& current_path, const std::string& target) -> bool {
                if (current_path == target) {
                    node->add_child(child_name, variant_node);
                    populated_paths.insert(child_path);
                    return true;
                }
                for (auto& child_entry : node->children) {
                    for (auto& child : child_entry.second) {
                        std::string cp = current_path + "." + child->effective_name();
                        if (find_parent(child, cp, target)) return true;
                    }
                }
                return false;
            };
            
            for (auto& kw_entry : block.keywords) {
                for (auto& kw : kw_entry.second) {
                    std::string kw_path = block.name + "." + kw->effective_name();
                    if (find_parent(kw, kw_path, parent_path)) {
                        goto next_anchor_union;
                    }
                }
            }
            next_anchor_union:;
        }
    }
    
    // Fifth pass: re-run nested defaults for nodes created by passes 3+4
    // e.g., pass 3 added method_source.dakota, now fill dakota.ignore_bounds etc.
    populate_nested_defaults();
    
    // Validate required fields
    return validate_required(populated_paths, required_fields, block.name, errors);
}

bool DefaultExpander::validate_required(
    const std::set<std::string>& populated_paths,
    const std::set<std::string>& required_fields,
    const std::string& block_name,
    std::vector<std::string>& errors)
{
    (void)block_name;  // Reserved for future use in error messages
    bool success = true;
    
    for (const auto& req_path : required_fields) {
        // Only check if the parent is populated
        size_t last_dot = req_path.rfind('.');
        if (last_dot != std::string::npos) {
            std::string parent_path = req_path.substr(0, last_dot);
            
            // If parent is populated but required field is not, that's an error
            if (populated_paths.find(parent_path) != populated_paths.end() &&
                populated_paths.find(req_path) == populated_paths.end()) {
                errors.push_back("Missing required field: " + req_path);
                success = false;
            }
        }
    }
    
    return success;
}

std::shared_ptr<KeywordNode> DefaultExpander::create_default_node(
    const std::string& name,
    const FieldMetadata& metadata)
{
    auto node = std::make_shared<KeywordNode>();
    node->name = name;
    
    if (metadata.default_value.has_value()) {
        const auto& val = metadata.default_value.value();
        
        if (std::holds_alternative<bool>(val)) {
            node->is_flag = true;
            // Store bool directly - Value variant now supports bool
            node->param_values.push_back(std::get<bool>(val));
        } else if (std::holds_alternative<std::int64_t>(val)) {
            node->param_values.push_back(std::get<std::int64_t>(val));
        } else if (std::holds_alternative<double>(val)) {
            node->param_values.push_back(std::get<double>(val));
        } else if (std::holds_alternative<std::string>(val)) {
            node->param_values.push_back(std::get<std::string>(val));
        }
    } else if (metadata.field_type == "primitive") {
        node->is_flag = true;
    }
    
    return node;
}

std::shared_ptr<KeywordNode> DefaultExpander::create_union_default_node(
    const std::string& name,
    const std::string& variant_name,
    const std::string& union_path,
    const std::map<std::string, FieldMetadata>& all_metadata)
{
    auto node = std::make_shared<KeywordNode>();
    node->name = name;
    
    // Create the variant child
    auto variant = std::make_shared<KeywordNode>();
    variant->name = variant_name;
    
    // Check if the variant child is a $ref object or a simple flag
    std::string variant_path = union_path + "." + variant_name;
    auto meta_it = all_metadata.find(variant_path);
    if (meta_it != all_metadata.end() && meta_it->second.field_type == "object") {
        // $ref variant: create as empty parent node (children filled by auto-populate re-run)
        variant->is_flag = false;
    } else {
        // Simple variant: boolean flag with value true
        variant->is_flag = true;
        variant->param_values.push_back(true);
    }
    
    node->add_child(variant_name, variant);
    
    return node;
}

} // namespace dakota
'''
    
    cpp_path.write_text(content)
    print(f"  Generated {cpp_path.name}")


def main():
    """Test the generator."""
    import argparse
    
    parser = argparse.ArgumentParser(description='Generate default expansion metadata')
    parser.add_argument('schema_file', type=Path, help='Dakota JSON Schema')
    parser.add_argument('--output-dir', type=Path, required=True, help='Output directory')
    parser.add_argument('--block', type=str, help='Single block to generate')
    
    args = parser.parse_args()
    
    with open(args.schema_file) as f:
        schema = json.load(f)
    
    extractor = DefaultExpansionSchemaExtractor(schema)
    
    blocks = [args.block] if args.block else [
        'environment', 'method', 'model', 'variables', 'interface', 'responses'
    ]
    
    print("Generating default expansion metadata...")
    
    for block_name in blocks:
        print(f"\n  Processing {block_name}...")
        fields = extractor.extract_block_metadata(block_name)
        print(f"    Found {len(fields)} fields")
        
        # Count stats
        non_null_defaults = sum(1 for f in fields.values() if f.has_non_null_default)
        required = sum(1 for f in fields.values() if f.is_required)
        union_variants = sum(1 for f in fields.values() if f.is_union_variant)
        union_defaults = sum(1 for f in fields.values() 
                            if f.is_union and f.union_pattern == 1 and f.union_default_variant)
        
        print(f"    Non-null defaults: {non_null_defaults}")
        print(f"    Required fields: {required}")
        print(f"    Union variant fields (conditionally required): {union_variants}")
        print(f"    Union defaults (pattern 1): {union_defaults}")
        
        generate_default_metadata_header(block_name, fields, args.output_dir)
    
    # Generate common headers
    print("\n  Generating common files...")
    generate_default_types_header(args.output_dir)
    generate_default_expander_header(args.output_dir)
    generate_default_expander_cpp(args.output_dir)
    
    print("\nDone!")


if __name__ == '__main__':
    main()
