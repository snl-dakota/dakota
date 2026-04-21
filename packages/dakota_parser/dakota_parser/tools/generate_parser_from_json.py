#!/usr/bin/env python3
"""
Dakota Parser Generator - JSON Schema Edition

Drop-in replacement for generate_parser_from_xml.py
Reads JSON Schema and generates C++ PEGTL parsers for Dakota input files.
"""

import json
import sys
import argparse
from pathlib import Path
from typing import Dict, Set, List, Any, Optional, Tuple
from collections import defaultdict

import schema_utils as su

# Import common code
from generator_common import (
    VERBOSE_MODE,
    KeywordInfo,
    BlockInfo,
    CodeGenerator,
    ALL_BLOCKS,
    generate_stub_parser,
    generate_error_messages,
    generate_error_messages_stub,
    generate_error_message_tests_stub
)

import generator_common  # For setting VERBOSE_MODE


class JSONSchemaParser:
    """Parse Dakota JSON Schema and extract grammar information."""
    
    def __init__(self, schema_path: Path):
        """Load and initialize schema."""
        if VERBOSE_MODE:
            print(f"Loading JSON Schema: {schema_path}")
        
        with open(schema_path) as f:
            self.schema = json.load(f)
        
        self.defs = self.schema.get('$defs', {})
        self.top_level = self.schema
    
    def resolve_ref(self, ref: str) -> Dict[str, Any]:
        """Resolve a $ref to its definition."""
        result = su.resolve(self.defs, ref)
        if result is None:
            raise ValueError(f"Cannot resolve reference: {ref}")
        return result
    
    def parse_block(self, block_name: str) -> BlockInfo:
        """Parse a single block and return BlockInfo compatible with CodeGenerator."""
        if VERBOSE_MODE:
            print(f"  Parsing {block_name} block from schema...")
        
        # Get block definition from top-level properties
        block_schema = self.top_level['properties'][block_name]
        
        # Determine if array or single
        is_required = block_name in self.top_level.get('required', [])
        
        # Extract the config reference(s) - can be multiple for union types
        config_refs, is_array = su.collect_block_refs(block_schema)
        
        # Create BlockInfo with structure compatible with CodeGenerator
        block_info = BlockInfo(name=block_name)
        
        # Detect if this is a block-level union (multiple config refs)
        is_block_level_union = len(config_refs) > 1
        
        # Parse all configs and merge keywords
        for config_ref in config_refs:
            config = self.resolve_ref(config_ref)
            self.populate_block_keywords(config, block_info, block_name, 
                                        is_block_level_union=is_block_level_union)
        
        if VERBOSE_MODE:
            print(f"    Found {len(block_info.keywords)} keywords")
            print(f"    Built {len(block_info.parent_child_map)} parent-child relationships")
        
        # Determine if all keywords are optional (block can be empty).
        # Union blocks always need at least one keyword (the discriminator).
        # For non-union blocks, check if every property has a default or is nullable.
        if not is_block_level_union and len(config_refs) == 1:
            config = self.resolve_ref(config_refs[0])
            required = set(config.get('required', []))
            all_optional = True
            for prop_name, prop_spec in config.get('properties', {}).items():
                has_default = 'default' in prop_spec
                is_nullable = any(
                    s.get('type') == 'null' for s in prop_spec.get('anyOf', [])
                ) if 'anyOf' in prop_spec else False
                if prop_name in required and not has_default and not is_nullable:
                    all_optional = False
                    break
            block_info.all_keywords_optional = all_optional
        
        return block_info
    
    def populate_block_keywords(self, config: Dict[str, Any], block_info: BlockInfo, parent_context: str, parent_path: Optional[str] = None, is_block_level_union: bool = False):
        """
        Populate block_info with keywords from config, recursively following $refs.
        
        Args:
            config: The config object with properties to parse
            block_info: BlockInfo to populate
            parent_context: The immediate parent keyword name  
            parent_path: Full dotted path to parent (e.g., "environment.results_output.hdf5")
            is_block_level_union: True if this block has multiple config types (e.g., method)
        """
        # If no parent_path, we're at the top level - use block name
        if parent_path is None:
            parent_path = block_info.name
        
        # For block-level unions, direct children are the union selectors
        # They should NOT have min_occurs=1 (only one needs to be present)
        is_direct_child_of_union_block = is_block_level_union and parent_path == block_info.name
            
        properties = config.get('properties', {})
        required = set(config.get('required', []))
        
        for field_name, field_schema in properties.items():
            # Check if this is a union (anchor field OR anyOf with multiple $refs)
            is_union = field_schema.get('anchor') == True or self.is_union_pattern(field_schema)
            
            if is_union:
                # For non-anchor unions, create the selector field as a FLAG keyword
                # (In XML, these are container keywords; in DSL they appear in the syntax)
                if not field_schema.get('anchor'):
                    # For block-level unions (like method), selectors should have min_occurs=0
                    # because only ONE of them needs to be present
                    if is_direct_child_of_union_block:
                        selector_min_occurs = 0
                    else:
                        selector_min_occurs = 1 if field_name in required else 0
                    
                    selector_kw = KeywordInfo(
                        id=field_name,
                        name=field_name,
                        has_param=False,
                        param_type=None,
                        parent=parent_context,
                        min_occurs=selector_min_occurs,
                        max_occurs=1
                    )
                    block_info.keywords[selector_kw.id] = selector_kw
                    block_info.keywords_by_name[selector_kw.name].append(selector_kw.id)
                    sig = selector_kw.signature()
                    if sig not in block_info.keywords_by_signature:
                        block_info.keywords_by_signature[sig] = selector_kw.id
                    
                    # Add to parent-child map (selector is child of current parent)
                    if parent_path not in block_info.parent_child_map:
                        block_info.parent_child_map[parent_path] = set()
                    block_info.parent_child_map[parent_path].add(field_name)
                
                # Extract and parse union discriminators
                discriminators = self.extract_union_discriminators(field_schema)
                
                # Build path for discriminators
                # For anchor fields, discriminators are children of current parent_path
                # For non-anchor fields, discriminators are children of the selector field
                if field_schema.get('anchor'):
                    disc_parent_path = parent_path
                else:
                    disc_parent_path = f"{parent_path}.{field_name}"
                    
                for disc_name in discriminators:
                    # Add to parent-child map with full path
                    if disc_parent_path not in block_info.parent_child_map:
                        block_info.parent_child_map[disc_parent_path] = set()
                    block_info.parent_child_map[disc_parent_path].add(disc_name)
                    
                    # Recursively parse discriminator configs
                    self.parse_union_discriminator(disc_name, field_schema, block_info, parent_context, disc_parent_path)
                continue
            
            # Parse this field into KeywordInfo
            kw_info = self.create_keyword_info(field_name, field_schema, required, parent_context)
            if kw_info:
                # For block-level unions, direct children are selectors that should have min_occurs=0
                # (only ONE of them needs to be present)
                if is_direct_child_of_union_block:
                    kw_info.min_occurs = 0
                
                # Add to keywords dict keyed by ID
                block_info.keywords[kw_info.id] = kw_info
                
                # Add to keywords_by_name
                block_info.keywords_by_name[kw_info.name].append(kw_info.id)
                
                # Add to keywords_by_signature  
                sig = kw_info.signature()
                if sig not in block_info.keywords_by_signature:
                    block_info.keywords_by_signature[sig] = kw_info.id
                
                # Track aliases in alias_map
                if kw_info.aliases:
                    for alias_name in kw_info.aliases:
                        # Key: (parent_path, alias_name) -> canonical_name
                        alias_key = (parent_path, alias_name)
                        block_info.alias_map[alias_key] = kw_info.name
                
                # Add to parent-child map with full path
                if parent_path not in block_info.parent_child_map:
                    block_info.parent_child_map[parent_path] = set()
                block_info.parent_child_map[parent_path].add(field_name)
                
                # Check if it has children and recursively parse them
                nested_config = self.get_nested_config(field_name, field_schema)
                if nested_config:
                    # Build full path for this keyword
                    my_path = f"{parent_path}.{field_name}"
                    
                    # Recursively parse nested keywords
                    self.populate_block_keywords(nested_config, block_info, field_name, my_path)
    
    def is_union_pattern(self, field_schema: Dict[str, Any]) -> bool:
        """
        Detect if a field represents a union of discriminators.
        
        A field is a union if it has anyOf with multiple $refs that point
        to objects (not primitive types or null).
        """
        if 'anyOf' not in field_schema:
            return False
        
        # Count how many $refs point to objects (discriminator variants)
        object_refs = 0
        for option in field_schema['anyOf']:
            if '$ref' in option:
                # This is a reference to a type - likely a discriminator variant
                object_refs += 1
        
        # If we have 2+ object refs, it's a union pattern
        # (1 ref could just be an optional field)
        return object_refs >= 2
    
    def get_nested_config(self, field_name: str, field_schema: Dict[str, Any]) -> Optional[Dict[str, Any]]:
        """Get nested config for a field if it has one."""
        # Skip argument field itself - we only want its children
        arg_field_name = field_schema.get('argument')
        
        # Resolve nested config
        nested_config = None
        
        # Check allOf (used for discriminators with single schema ref)
        if 'allOf' in field_schema:
            for option in field_schema['allOf']:
                if '$ref' in option:
                    nested_config = self.resolve_ref(option['$ref'])
                    break
        # Check anyOf (used for optional fields OR union discriminators)
        elif 'anyOf' in field_schema:
            # For union patterns, we need to merge properties from ALL variants
            # to collect all possible children (e.g., bfgs from one variant, sr1 from another)
            merged_properties = {}
            merged_required = set()
            
            for option in field_schema['anyOf']:
                if '$ref' in option:
                    variant_config = self.resolve_ref(option['$ref'])
                    if variant_config and 'properties' in variant_config:
                        # Merge properties from this variant
                        merged_properties.update(variant_config['properties'])
                        # Merge required fields
                        if 'required' in variant_config:
                            merged_required.update(variant_config['required'])
            
            if merged_properties:
                nested_config = {
                    'properties': merged_properties,
                    'required': list(merged_required)
                }
            else:
                # Fallback: try to get first non-null option (for optional fields)
                for option in field_schema['anyOf']:
                    if '$ref' in option:
                        nested_config = self.resolve_ref(option['$ref'])
                        break
        # Check direct $ref
        elif '$ref' in field_schema:
            nested_config = self.resolve_ref(field_schema['$ref'])
        
        if not nested_config or 'properties' not in nested_config:
            return None
        
        # If this field has an argument attribute, we need to:
        # 1. Skip the argument field itself (it's the inline value)
        # 2. Process other fields as children
        # 3. Also check for anchor fields and process their discriminators
        if arg_field_name:
            # Filter out the argument field
            filtered_props = {
                k: v for k, v in nested_config['properties'].items()
                if k != arg_field_name
            }
            if filtered_props:
                return {
                    'properties': filtered_props,
                    'required': nested_config.get('required', [])
                }
            return None
        
        return nested_config
    
    def parse_union_discriminator(self, disc_name: str, union_schema: Dict[str, Any], 
                                  block_info: BlockInfo, parent_context: str, parent_path: str):
        """Parse a union discriminator and its nested keywords."""
        if 'anyOf' not in union_schema:
            return
        
        # Find the discriminator's variant in the anyOf list
        for variant_ref in union_schema['anyOf']:
            if '$ref' not in variant_ref:
                continue
            
            variant_config = self.resolve_ref(variant_ref['$ref'])
            if 'properties' not in variant_config:
                continue
            
            # Check if this variant has our discriminator
            if disc_name not in variant_config['properties']:
                continue
            
            # Found it! Now parse the discriminator's config
            disc_schema = variant_config['properties'][disc_name]
            disc_required = set(variant_config.get('required', []))
            
            # Create keyword info for the discriminator
            disc_kw_info = self.create_keyword_info(disc_name, disc_schema, disc_required, parent_context)
            if disc_kw_info:
                # Add to keywords
                block_info.keywords[disc_kw_info.id] = disc_kw_info
                block_info.keywords_by_name[disc_kw_info.name].append(disc_kw_info.id)
                
                sig = disc_kw_info.signature()
                if sig not in block_info.keywords_by_signature:
                    block_info.keywords_by_signature[sig] = disc_kw_info.id
                
                # Track aliases in alias_map
                if disc_kw_info.aliases:
                    for alias_name in disc_kw_info.aliases:
                        # Key: (parent_path, alias_name) -> canonical_name
                        alias_key = (parent_path, alias_name)
                        block_info.alias_map[alias_key] = disc_kw_info.name
                
                # Recursively parse discriminator's children
                nested_config = self.get_nested_config(disc_name, disc_schema)
                if nested_config:
                    # Build full path for this discriminator
                    disc_path = f"{parent_path}.{disc_name}"
                    
                    self.populate_block_keywords(nested_config, block_info, disc_name, disc_path)
            
            break  # Found and processed the discriminator
    
    def create_keyword_info(self, field_name: str, field_schema: Dict[str, Any], 
                          required: Set[str], parent: Optional[str]) -> Optional[KeywordInfo]:
        """Create KeywordInfo from field schema."""
        # Generate unique ID by combining parent and field name
        # This allows same field name in different contexts (e.g., primary_scales in calibration_terms vs objective_functions)
        if parent:
            kw_id = f"{parent}_{field_name}"
        else:
            kw_id = field_name
        
        # Check if this is a const field (flag keyword)
        if 'const' in field_schema:
            # Const fields are flags (boolean keywords)
            kw_info = KeywordInfo(
                id=kw_id,
                name=field_name,
                has_param=False,
                param_type=None,
                parent=parent,
                min_occurs=1 if field_name in required else 0,
                max_occurs=1
            )
        else:
            # Determine parameter type
            param_type = self.get_parameter_type(field_name, field_schema)
            has_param = param_type not in [None, 'FLAG', 'CONTAINER']
            
            # Create keyword info
            kw_info = KeywordInfo(
                id=kw_id,  # Use unique ID
                name=field_name,
                has_param=has_param,
                param_type=param_type if has_param else None,
                parent=parent,
                min_occurs=1 if field_name in required else 0,
                max_occurs=1  # Schema typically has max 1 for fields
            )
            
            # Extract constraints
            constraints = self.extract_constraints(field_schema)
            for constraint in constraints:
                if constraint['type'] == 'minimum':
                    kw_info.param_constraint = f">= {constraint['value']}"
                elif constraint['type'] == 'maximum':
                    kw_info.param_constraint = f"<= {constraint['value']}"
                elif constraint['type'] == 'greater_than':
                    kw_info.param_constraint = f"> {constraint['value']}"
                elif constraint['type'] == 'less_than':
                    kw_info.param_constraint = f"< {constraint['value']}"
        
        # Extract aliases (x-aliases field)
        if 'x-aliases' in field_schema:
            aliases = field_schema['x-aliases']
            if isinstance(aliases, list):
                kw_info.aliases = aliases.copy()
            elif isinstance(aliases, str):
                # Handle single alias as string
                kw_info.aliases = [aliases]
        
        return kw_info
    
    def get_parameter_type(self, field_name: str, field_schema: Dict[str, Any]) -> Optional[str]:
        """Determine the Dakota parameter type for a field."""
        
        # Check for 'argument' attribute - get type from nested field
        if 'argument' in field_schema:
            arg_field_name = field_schema['argument']
            
            # Resolve the nested config
            nested_config = None
            # Check allOf first (used for single required reference)
            if 'allOf' in field_schema:
                for option in field_schema['allOf']:
                    if '$ref' in option:
                        nested_config = self.resolve_ref(option['$ref'])
                        break
            # Then check anyOf (used for optional references)
            elif 'anyOf' in field_schema:
                for option in field_schema['anyOf']:
                    if '$ref' in option:
                        nested_config = self.resolve_ref(option['$ref'])
                        break
            # Finally check direct $ref
            elif '$ref' in field_schema:
                nested_config = self.resolve_ref(field_schema['$ref'])
            
            if nested_config and 'properties' in nested_config:
                arg_field_schema = nested_config['properties'].get(arg_field_name)
                if arg_field_schema:
                    # Argument field might also have anyOf (optional)
                    if 'anyOf' in arg_field_schema:
                        for option in arg_field_schema['anyOf']:
                            if 'type' in option and option['type'] != 'null':
                                return self._map_json_type_to_dakota(option)
                    return self._map_json_type_to_dakota(arg_field_schema)
        
        # Check for direct type
        if 'type' in field_schema:
            return self._map_json_type_to_dakota(field_schema)
        
        # Check for anyOf (might be optional)
        if 'anyOf' in field_schema:
            for option in field_schema['anyOf']:
                if 'type' in option and option['type'] != 'null':
                    return self._map_json_type_to_dakota(option)
                elif '$ref' in option:
                    # This might be a container keyword
                    return 'CONTAINER'
        
        # Check for $ref
        if '$ref' in field_schema:
            return 'CONTAINER'
        
        return None
    
    def _map_json_type_to_dakota(self, type_schema: Dict[str, Any]) -> str:
        """Map JSON Schema type to Dakota parameter type."""
        json_type = type_schema.get('type')
        
        if json_type == 'boolean':
            return 'FLAG'
        elif json_type == 'integer':
            return 'INTEGER'
        elif json_type == 'number':
            return 'REAL'
        elif json_type == 'string':
            return 'STRING'
        elif json_type == 'array':
            items_type = type_schema.get('items', {}).get('type')
            if items_type == 'string':
                return 'STRINGLIST'
            elif items_type == 'integer':
                return 'INTEGERLIST'
            elif items_type == 'number':
                return 'REALLIST'
        
        return 'UNKNOWN'
    
    def extract_constraints(self, field_schema: Dict[str, Any]) -> List[Dict[str, Any]]:
        """Extract validation constraints from field schema."""
        constraints = []
        
        # Numeric constraints
        if 'minimum' in field_schema:
            constraints.append({
                'type': 'minimum',
                'value': field_schema['minimum']
            })
        if 'maximum' in field_schema:
            constraints.append({
                'type': 'maximum', 
                'value': field_schema['maximum']
            })
        if 'gt' in field_schema:
            constraints.append({
                'type': 'greater_than',
                'value': field_schema['gt']
            })
        if 'lt' in field_schema:
            constraints.append({
                'type': 'less_than',
                'value': field_schema['lt']
            })
        
        # Block pointer
        if 'x-block-pointer' in field_schema:
            constraints.append({
                'type': 'block_pointer',
                'target': field_schema['x-block-pointer']
            })
        
        return constraints
    
    def get_field_children(self, field_name: str, field_schema: Dict[str, Any]) -> Set[str]:
        """Get children of a field based on {} nesting rule."""
        children = set()
        
        # If has 'argument' attribute, resolve nested config
        if 'argument' in field_schema:
            arg_field_name = field_schema['argument']
            
            # Resolve nested config
            nested_config = None
            if 'anyOf' in field_schema:
                for option in field_schema['anyOf']:
                    if '$ref' in option:
                        nested_config = self.resolve_ref(option['$ref'])
                        break
            elif '$ref' in field_schema:
                nested_config = self.resolve_ref(field_schema['$ref'])
            
            if nested_config and 'properties' in nested_config:
                for prop_name, prop_schema in nested_config['properties'].items():
                    # Skip the argument field itself
                    if prop_name == arg_field_name:
                        continue
                    
                    # Skip anchor fields, but extract discriminators
                    if prop_schema.get('anchor') == True:
                        discriminators = self.extract_union_discriminators(prop_schema)
                        children.update(discriminators)
                        continue
                    
                    # Regular child
                    children.add(prop_name)
        
        # If $ref to object (without argument), properties are children
        elif '$ref' in field_schema:
            nested_config = self.resolve_ref(field_schema['$ref'])
            if 'properties' in nested_config:
                for prop_name, prop_schema in nested_config['properties'].items():
                    if not prop_schema.get('anchor'):
                        children.add(prop_name)
        
        # Check in anyOf for $ref
        elif 'anyOf' in field_schema:
            for option in field_schema['anyOf']:
                if '$ref' in option:
                    nested_config = self.resolve_ref(option['$ref'])
                    if 'properties' in nested_config:
                        for prop_name, prop_schema in nested_config['properties'].items():
                            if not prop_schema.get('anchor'):
                                children.add(prop_name)
                    break
        
        return children
    
    def extract_union_discriminators(self, union_schema: Dict[str, Any]) -> List[str]:
        """
        Extract discriminator names from a union field.
        
        Works for both anchor fields and regular union patterns.
        """
        discriminators = []
        
        if 'anyOf' not in union_schema:
            return discriminators
        
        for variant_ref in union_schema['anyOf']:
            if '$ref' not in variant_ref:
                continue
            
            variant_config = self.resolve_ref(variant_ref['$ref'])
            if 'properties' not in variant_config:
                continue
            
            # The discriminator is the (first) property key in the variant
            # For single-property variants, this is the discriminator name
            props = variant_config['properties']
            if len(props) == 1:
                # Single property = discriminator
                discriminator_name = next(iter(props.keys()))
                discriminators.append(discriminator_name)
            elif len(props) > 1:
                # Multiple properties - need to figure out which is the discriminator
                # Heuristic: if there's a 'const' field, that's likely the discriminator
                for prop_name, prop_schema in props.items():
                    if 'const' in prop_schema:
                        discriminators.append(prop_name)
                        break
                else:
                    # No const field, take the first property as discriminator
                    discriminator_name = next(iter(props.keys()))
                    discriminators.append(discriminator_name)
        
        return discriminators


def main():
    """Main entry point - matches XML generator interface exactly."""
    parser = argparse.ArgumentParser(
        description='Generate Dakota parser from JSON Schema (drop-in replacement for XML generator)',
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    
    parser.add_argument('schema_file', type=Path, help='Dakota JSON Schema specification')
    parser.add_argument('--output-dir', type=Path, required=True,
                       help='Root directory for all generated files (creates generated/ and generated_tests/ subdirs)')
    parser.add_argument('--block', type=str, 
                       choices=['environment', 'method', 'model', 'variables', 'interface', 'responses'],
                       help='Block to generate parser for (if not specified, generates all blocks)')
    parser.add_argument('--generate-tests', action='store_true',
                       help='Generate error message test files')
    parser.add_argument('--force', action='store_true',
                       help='Force regeneration even if generated directory exists')
    parser.add_argument('--verbose', '-v', action='store_true',
                       help='Enable verbose output (show detailed generation info)')
    
    args = parser.parse_args()
    
    # Set global verbose flag
    generator_common.VERBOSE_MODE = args.verbose
    
    if not args.schema_file.exists():
        print(f"Error: Schema file not found: {args.schema_file}", file=sys.stderr)
        return 1
    
    try:
        # Setup directory paths
        root_output_dir = args.output_dir
        source_output_dir = root_output_dir / "generated"
        test_output_dir = root_output_dir / "generated_tests"
        
        # Check if we should skip generation
        if source_output_dir.exists() and not args.force:
            print(f"Generated code already exists at {source_output_dir}")
            print("Use --force to regenerate")
            return 0
        
        # Create output directories
        source_output_dir.mkdir(parents=True, exist_ok=True)
        
        print("Generating Dakota parser code from JSON Schema...")
        print()
        
        # =====================================================================
        # PHASE 1: Generate all source files
        # =====================================================================
        
        if args.block:
            # Single block mode: generate specified block + stubs for others
            if VERBOSE_MODE:
                print(f"=== Generating {args.block} block with stubs for others ===")
                print()
            
            # Parse and generate the requested block
            json_parser = JSONSchemaParser(args.schema_file)
            block = json_parser.parse_block(args.block)
            
            generator = CodeGenerator(block, source_output_dir)
            generator.generate_all()
            
            # Generate stubs for the other blocks
            if VERBOSE_MODE:
                print(f"=== Generating stubs for remaining blocks ===")
                print()
            for block_name in ALL_BLOCKS:
                if block_name != args.block:
                    generate_stub_parser(block_name, source_output_dir)
            
            # Generate minimal error messages file for single-block mode
            print("Generating error messages stub...")
            generate_error_messages_stub(args.block, source_output_dir)
            print(f"  Generated dakota_error_messages.hpp (stub for {args.block})")
            
            print()
            print(f"Generation complete! Generated parser for {args.block} block with stubs.")
        else:
            # All blocks mode: generate all blocks
            if VERBOSE_MODE:
                print("=== Generating all blocks ===")
                print()
            
            json_parser = JSONSchemaParser(args.schema_file)
            
            for block_name in ALL_BLOCKS:
                if VERBOSE_MODE:
                    print(f"--- Processing {block_name} block ---")
                
                # Parse and generate each block
                block = json_parser.parse_block(block_name)
                
                generator = CodeGenerator(block, source_output_dir)
                generator.generate_all()
            
            print()
            print("Generation complete!")
            print()
            
            # Generate unified error messages file
            print("Generating error messages...")
            
            # Collect all block information
            all_blocks = {}
            for block_name in ALL_BLOCKS:
                all_blocks[block_name] = json_parser.parse_block(block_name)
            
            generate_error_messages(all_blocks, source_output_dir)
            print(f"  Generated dakota_error_messages.hpp")

        # =====================================================================
        # PHASE 1.5: Generate default expansion metadata
        # =====================================================================
        
        print()
        print("Generating default expansion metadata...")
        
        try:
            from default_expander_generator import (
                DefaultExpansionSchemaExtractor,
                generate_default_metadata_header,
                generate_default_types_header,
                generate_default_expander_header,
                generate_default_expander_cpp
            )
            
            with open(args.schema_file) as f:
                schema = json.load(f)
            
            extractor = DefaultExpansionSchemaExtractor(schema)
            
            if args.block:
                # Single block mode
                blocks_to_process = [args.block]
            else:
                # All blocks mode
                blocks_to_process = ALL_BLOCKS
            
            for block_name in blocks_to_process:
                fields = extractor.extract_block_metadata(block_name)
                generate_default_metadata_header(block_name, fields, source_output_dir)
            
            # Generate common files
            generate_default_types_header(source_output_dir)
            generate_default_expander_header(source_output_dir)
            generate_default_expander_cpp(source_output_dir)
            
            print(f"  Generated default expansion files for {len(blocks_to_process)} blocks")
            
        except ImportError as e:
            print(f"  Warning: Could not import default_expander_generator ({e}), skipping")
        
        # =====================================================================
        # PHASE 1.6: Generate AST-to-JSON metadata
        # =====================================================================
        
        print()
        print("Generating AST-to-JSON metadata...")
        
        try:
            from generate_ast_metadata import ASTMetadataGenerator
            
            ast_gen = ASTMetadataGenerator(args.schema_file)
            ast_gen.process_all()
            
            header_content = ast_gen.generate_header()
            output_path = source_output_dir / "dakota_ast_metadata.hpp"
            with open(output_path, "w") as f:
                f.write(header_content)
            
            # Count context-specific argument fields
            context_specific_count = sum(
                len({f for f, a in fields.items() if f not in ast_gen.argument_fields})
                for fields in ast_gen.context_argument_fields.values()
            )
            
            print(f"  Generated dakota_ast_metadata.hpp")
            print(f"    - {len(ast_gen.boolean_fields)} boolean fields")
            print(f"    - {len(ast_gen.argument_fields)} global argument fields")
            print(f"    - {context_specific_count} context-specific argument fields")
            print(f"    - {len(ast_gen.anchor_fields)} parent contexts with anchors")
            print(f"    - {len(ast_gen.array_fields)} array fields")
            print(f"    - {len(ast_gen.discriminator_configs)} discriminator configs")
            
        except ImportError as e:
            print(f"  Warning: Could not import generate_ast_metadata ({e}), skipping")
        
        # =====================================================================
        # PHASE 1.7: Generate validation metadata
        # =====================================================================
        
        print()
        print("Generating validation metadata...")
        
        try:
            from generate_validation_metadata import ValidationMetadataGenerator
            
            val_gen = ValidationMetadataGenerator(str(args.schema_file))
            val_gen.process_all()
            
            header_content = val_gen.generate_header()
            output_path = source_output_dir / "dakota_validation_metadata.hpp"
            with open(output_path, "w") as f:
                f.write(header_content)
            
            num_nc = len(val_gen.numeric_constraints)
            num_mv = sum(len(v) for v in val_gen.model_validations.values())
            num_cf = sum(len(v) for v in val_gen.computed_fields.values())
            num_wm = sum(len(v) for v in val_gen.schema_walk_map.values())
            
            print(f"  Generated dakota_validation_metadata.hpp")
            print(f"    - {num_nc} definitions with numeric constraints")
            print(f"    - {len(val_gen.model_validations)} definitions with x-model-validations ({num_mv} rules)")
            print(f"    - {len(val_gen.computed_fields)} definitions with x-computed-fields ({num_cf} computed fields)")
            print(f"    - {len(val_gen.schema_walk_map)} definitions in walk map ({num_wm} property refs)")
            
        except ImportError as e:
            print(f"  Warning: Could not import generate_validation_metadata ({e}), skipping")
        
        # =====================================================================
        # PHASE 2: Generate test files (if requested)
        # =====================================================================
        
        if args.generate_tests:
            print()
            print("Generating tests...")
            
            # Create test directory
            test_output_dir.mkdir(parents=True, exist_ok=True)
            
            json_parser = JSONSchemaParser(args.schema_file)
            
            # Collect all block information (if not already done)
            if args.block:
                # For single block mode, only generate tests for that block
                all_blocks = {args.block: json_parser.parse_block(args.block)}
            else:
                # For all blocks mode, collect all
                all_blocks = {}
                for block_name in ALL_BLOCKS:
                    all_blocks[block_name] = json_parser.parse_block(block_name)
            
            # Generate constraint tests
            print("  Generating constraint tests...")
            constraint_test_count = 0
            
            # Import constraint test generator
            try:
                from generate_constraint_tests import generate_constraint_tests
                
                for block_name, block_info in all_blocks.items():
                    block_test_dir = test_output_dir / block_name / "constraints"
                    count = generate_constraint_tests(block_info, block_name, block_test_dir)
                    constraint_test_count += count
                    if VERBOSE_MODE:
                        print(f"    Generated {count} constraint tests for {block_name}")
                
                print(f"  Generated {constraint_test_count} constraint tests")
            except ImportError:
                print("  Warning: Could not import generate_constraint_tests, skipping")
            
            # Generate error message tests (stub for now)
            print("  Generating error message tests...")
            error_test_count = 0
            for block_name in all_blocks.keys():
                count = generate_error_message_tests_stub(all_blocks, block_name, test_output_dir)
                error_test_count += count
            
            if error_test_count > 0:
                print(f"  Generated {error_test_count} error message tests")
            print(f"  Total tests: {constraint_test_count + error_test_count}")
        
        return 0
        
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc()
        return 1


if __name__ == '__main__':
    sys.exit(main())
