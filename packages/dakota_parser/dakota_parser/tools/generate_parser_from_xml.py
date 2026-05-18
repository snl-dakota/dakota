#!/usr/bin/env python3
"""
Dakota Parser Generator - XML Edition

Parses dakota.xml specification file and generates C++ PEGTL parsers.
"""

try:
    from lxml import etree as ET
    USING_LXML = True
except ImportError:
    import xml.etree.ElementTree as ET
    USING_LXML = False
    print("Warning: lxml not available, falling back to xml.etree")

import argparse
import sys
from pathlib import Path
from typing import Dict, Set, Optional, List
from collections import defaultdict

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
    generate_error_message_tests_stub,
    write_file_if_changed
)

import generator_common  # For setting VERBOSE_MODE

class DakotaXMLParser:
    """Parse dakota.xml and extract block structure"""
    
    def __init__(self, xml_file: Path, block_name: str):
        self.xml_file = xml_file
        self.block_name = block_name
        self.tree = None
        self.root = None
        self.namespaces = {}
    
    def parse(self) -> BlockInfo:
        """Parse XML and return specified block info"""
        if VERBOSE_MODE:
            print(f"Parsing {self.xml_file} for {self.block_name} block...")
        
        if USING_LXML:
            self.tree = ET.parse(str(self.xml_file))
            self.root = self.tree.getroot()
            # Extract namespace
            if self.root.tag.startswith('{'):
                ns = self.root.tag[1:self.root.tag.index('}')]
                self.namespaces = {'d': ns}
        else:
            self.tree = ET.parse(str(self.xml_file))
            self.root = self.tree.getroot()
        
        # Find specified block
        block_elem = self._find_block()
        if block_elem is None:
            raise ValueError(f"Could not find {self.block_name} block in XML")
        
        # Extract keywords from block
        block = BlockInfo(name=self.block_name)
        self._extract_keywords(block_elem, block, parent_name=None, parent_path=None, oneof_label=None)
        self._build_parent_child_map(block)
        self._validate_consistency(block)
        
        return block
    
    def _find_block(self):
        """Find the specified block keyword element"""
        if USING_LXML and self.namespaces:
            # Try with namespace
            elem = self.root.find(f".//d:keyword[@name='{self.block_name}']", self.namespaces)
            if elem is not None:
                return elem
        
        # Try without namespace
        for keyword in self.root.iter():
            tag = keyword.tag
            if '}' in tag:
                tag = tag.split('}')[1]
            if tag == 'keyword' and keyword.get('name') == self.block_name:
                return keyword
        
        return None
    
    def _extract_keywords(self, elem, block: BlockInfo, parent_name: Optional[str], parent_path: Optional[str], 
                         depth: int = 0, oneof_label: Optional[str] = None, 
                         selector_name: Optional[str] = None, selector_path: Optional[str] = None):
        """Recursively extract keywords from XML element
        
        Args:
            elem: XML element to process
            block: BlockInfo to populate
            parent_name: Name of parent keyword (for KeywordInfo.parent field)
            parent_path: Full dotted path of parent (for observed_relationships)
            depth: Current recursion depth
            selector_name: Name of last selector keyword (code="{0}")
            selector_path: Path of last selector keyword
        """
        
        # Track selector keywords (code="{0}") to associate with following oneOf groups
        last_selector_name = selector_name
        last_selector_path = selector_path
        
        # Process all child elements
        for child in elem:
            # Skip comments
            if callable(child.tag):
                continue
            
            tag = child.tag
            if '}' in tag:
                tag = tag.split('}')[1]
            
            # Handle different element types
            if tag == 'keyword':
                kw_id = child.get('id')
                kw_name = child.get('name')
                code = child.get('code', '')
                
                if not kw_id or not kw_name:
                    continue
                
                # Check if this is a selector keyword (code="{0}")
                is_selector = (code == '{0}')
                
                # Always record the parent-child relationship for this occurrence
                # Use the full parent path as the key
                if parent_path:
                    block.observed_relationships[parent_path].add(kw_name)
                else:
                    # Top-level keyword
                    block.observed_relationships[block.name].add(kw_name)
                
                # Check for parameter - ONLY direct children, not descendants
                # Do this BEFORE checking if ID exists, so we can detect conflicts
                has_param = False
                param_elem = None
                for child_elem in child:
                    if callable(child_elem.tag):
                        continue
                    child_tag = child_elem.tag
                    if '}' in child_tag:
                        child_tag = child_tag.split('}')[1]
                    if child_tag == 'param':
                        has_param = True
                        param_elem = child_elem
                        break
                
                # Check if this ID already seen
                already_exists = kw_id in block.keywords
                if already_exists:
                    # Check if the existing keyword has the same parameter configuration
                    existing_kw = block.keywords[kw_id]
                    if existing_kw.has_param != has_param:
                        # Different configuration! Need to create a new ID
                        # Find a unique ID by appending a number
                        counter = 2
                        new_kw_id = f"{kw_id}{counter}"
                        while new_kw_id in block.keywords:
                            counter += 1
                            new_kw_id = f"{kw_id}{counter}"
                        kw_id = new_kw_id
                        already_exists = False  # Treat as new keyword
                        print(f"WARNING: Keyword '{kw_name}' has different configurations:")
                        print(f"  Existing ID '{existing_kw.id}': has_param={existing_kw.has_param}")
                        print(f"  Creating new ID '{kw_id}': has_param={has_param}")
                    else:
                        # Same configuration - just process children and continue
                        # Build the path for this keyword instance
                        if parent_path:
                            my_path = f"{parent_path}.{kw_name}"
                        else:
                            my_path = f"{block.name}.{kw_name}"
                        # Don't pass selector info into keyword's children
                        self._extract_keywords(child, block, parent_name=kw_name, parent_path=my_path, 
                                             depth=depth+1, oneof_label=None, selector_name=None, selector_path=None)
                        # Track selector for next sibling
                        if is_selector:
                            last_selector_name = kw_name
                            last_selector_path = my_path
                        else:
                            last_selector_name = None
                            last_selector_path = None
                        continue
                
                # Check if this name with different params
                existing_ids = block.keywords_by_name.get(kw_name, [])
                
                # Validate consistency across all instances of this name
                for existing_id in existing_ids:
                    existing = block.keywords[existing_id]
                    if existing.has_param != has_param:
                        print(f"WARNING: Keyword '{kw_name}' has different configurations:")
                        print(f"  ID '{existing.id}': has_param={existing.has_param}")
                        print(f"  ID '{kw_id}': has_param={has_param}")
                
                # Extract keyword info
                kw_info = KeywordInfo(id=kw_id, name=kw_name, has_param=has_param, parent=parent_name)
                kw_info.line_number = child.sourceline if hasattr(child, 'sourceline') else None
                
                # Extract occurrence constraints
                min_occurs = child.get('minOccurs')
                if min_occurs is not None:
                    try:
                        kw_info.min_occurs = int(min_occurs)
                    except ValueError:
                        pass
                
                max_occurs = child.get('maxOccurs')
                if max_occurs is not None:
                    try:
                        kw_info.max_occurs = int(max_occurs)
                    except ValueError:
                        pass
                
                if param_elem is not None:
                    kw_info.param_type = param_elem.get('type', 'STRING')
                    # Extract constraint attribute from param
                    constraint = param_elem.get('constraint')
                    if constraint:
                        kw_info.param_constraint = constraint
                
                # Extract aliases
                for alias_elem in child:
                    if callable(alias_elem.tag):
                        continue
                    alias_tag = alias_elem.tag
                    if '}' in alias_tag:
                        alias_tag = alias_tag.split('}')[1]
                    if alias_tag == 'alias':
                        alias_name = alias_elem.get('name')
                        if alias_name:
                            kw_info.aliases.append(alias_name)
                            # Track this alias in context: (parent_path, alias_name) -> canonical_name
                            if parent_path:
                                alias_key = (parent_path, alias_name)
                            else:
                                alias_key = (block.name, alias_name)
                            block.alias_map[alias_key] = kw_name
                
                # Associate with oneOf group if we're inside one
                if oneof_label:
                    kw_info.oneof_group = oneof_label
                    # Track this group
                    group_key = (parent_path or block.name, oneof_label)
                    if group_key not in block.oneof_groups:
                        block.oneof_groups[group_key] = set()
                    block.oneof_groups[group_key].add(kw_name)
                
                # Add to block
                block.keywords[kw_id] = kw_info
                block.keywords_by_name[kw_name].append(kw_id)
                
                # Track by signature for deduplication
                sig = kw_info.signature()
                if sig not in block.keywords_by_signature:
                    # First time seeing this signature - this is the canonical ID
                    block.keywords_by_signature[sig] = kw_id
                
                # Recursively process children (this keyword becomes parent)
                # Build the path for this keyword
                if parent_path:
                    my_path = f"{parent_path}.{kw_name}"
                else:
                    my_path = f"{block.name}.{kw_name}"
                # Don't pass selector info into keyword's children
                self._extract_keywords(child, block, parent_name=kw_name, parent_path=my_path, 
                                     depth=depth+1, oneof_label=None, selector_name=None, selector_path=None)
                
                # Track selector for next sibling, or clear it for regular keywords
                if is_selector:
                    last_selector_name = kw_name
                    last_selector_path = my_path
                else:
                    last_selector_name = None
                    last_selector_path = None
            
            elif tag in ['oneOf', 'optional', 'group']:
                # These don't create keywords themselves, just groupings
                # For oneOf following a selector keyword, use selector as parent
                if tag == 'oneOf':
                    label = child.get('label', 'unnamed_oneof')
                    union_pattern = child.get('union_pattern')
                    
                    # Pattern 4 means flatten the union - all variants are available as direct children
                    # Don't treat this as a oneOf group - just process children as siblings
                    if union_pattern == '4':
                        # Process children as direct children of parent, not as a oneOf group
                        self._extract_keywords(child, block, parent_name=parent_name, parent_path=parent_path, 
                                             depth=depth, oneof_label=None, selector_name=None, selector_path=None)
                        # Clear selector since we've processed the oneOf
                        last_selector_name = None
                        last_selector_path = None
                    elif last_selector_path:
                        # This oneOf follows a selector keyword - children are children of the selector
                        self._extract_keywords(child, block, parent_name=last_selector_name, parent_path=last_selector_path, 
                                             depth=depth, oneof_label=label, selector_name=None, selector_path=None)
                        # Clear selector after using it
                        last_selector_name = None
                        last_selector_path = None
                    else:
                        # Regular oneOf without selector
                        self._extract_keywords(child, block, parent_name=parent_name, parent_path=parent_path, 
                                             depth=depth, oneof_label=label, selector_name=None, selector_path=None)
                elif tag == 'optional':
                    # Optional blocks process their children as siblings of the current context
                    # DO NOT pass selector through - selector only applies to direct children, not siblings
                    self._extract_keywords(child, block, parent_name=parent_name, parent_path=parent_path, 
                                         depth=depth, oneof_label=oneof_label,
                                         selector_name=None, selector_path=None)
                    # Clear selector since we've moved past it
                    last_selector_name = None
                    last_selector_path = None
                else:
                    # group doesn't affect validation
                    self._extract_keywords(child, block, parent_name=parent_name, parent_path=parent_path, 
                                         depth=depth, oneof_label=oneof_label, selector_name=None, selector_path=None)
                    last_selector_name = None
                    last_selector_path = None
    
    def _build_parent_child_map(self, block: BlockInfo):
        """Build parent-child relationships with dotted notation for unique keys"""
        
        if VERBOSE_MODE:
            print(f"\n=== Building parent-child map for {block.name} block ===")
            print(f"Observed relationships (parent path -> children):")
            for parent_path, children in sorted(block.observed_relationships.items()):
                print(f"  {parent_path}: {', '.join(sorted(list(children)[:10]))}")
                if len(children) > 10:
                    print(f"    ... and {len(children) - 10} more")
        
        # The observed_relationships already uses dotted paths as keys!
        # We can use it directly as the parent-child map
        block.parent_child_map = dict(block.observed_relationships)
        
        # AUGMENT STEP 1: Add aliases to parent's children sets
        # If a parent has "single" as a child, and "single" has alias "simulation",
        # then parent should also list "simulation" as a valid child
        # BUT: only add aliases from the keyword instance that belongs to this context
        additional_children = defaultdict(set)
        for parent_path, children in list(block.parent_child_map.items()):
            # Extract parent keyword name from path
            if '.' in parent_path:
                parent_keyword_name = parent_path.split('.')[-1]
            else:
                parent_keyword_name = parent_path  # Block name
            
            for child_name in list(children):
                # Find keywords with this child name and check for aliases
                # Only use aliases from keyword instances whose parent matches this context
                for kw_id, kw_info in block.keywords.items():
                    if kw_info.name == child_name and kw_info.aliases:
                        # Check if this keyword instance's parent matches the context
                        if kw_info.parent == parent_keyword_name:
                            # Add all aliases as additional children of this parent
                            for alias in kw_info.aliases:
                                additional_children[parent_path].add(alias)
        
        # Merge additional children into parent-child map
        for parent_path, aliases in additional_children.items():
            block.parent_child_map[parent_path].update(aliases)
        
        if VERBOSE_MODE and additional_children:
            print(f"\nAdded aliases to parent children sets:")
            for parent_path, aliases in sorted(list(additional_children.items())[:10]):
                print(f"  {parent_path}: added {', '.join(sorted(aliases))}")
            if len(additional_children) > 10:
                print(f"  ... and {len(additional_children) - 10} more parents updated")
        
        # AUGMENT STEP 2: Add alias path entries
        # For each keyword with aliases, add parent-child entries using alias names
        alias_entries = {}
        for parent_path, children in list(block.parent_child_map.items()):
            # Extract the last component of the path (the keyword name)
            if '.' in parent_path:
                path_parts = parent_path.split('.')
                parent_keyword_name = path_parts[-1]
                path_prefix = '.'.join(path_parts[:-1])
            else:
                parent_keyword_name = parent_path
                path_prefix = None
            
            # Find all keywords with this name and check for aliases
            for kw_id, kw_info in block.keywords.items():
                if kw_info.name == parent_keyword_name and kw_info.aliases:
                    # This keyword has aliases - add entries for each alias
                    for alias in kw_info.aliases:
                        if path_prefix:
                            alias_path = f"{path_prefix}.{alias}"
                        else:
                            alias_path = alias
                        
                        # Alias points to same children as canonical
                        alias_entries[alias_path] = children.copy()
        
        # Add alias entries to the map
        if VERBOSE_MODE and alias_entries:
            print(f"\nAdding {len(alias_entries)} alias path entries to parent-child map:")
            for alias_path, children in sorted(list(alias_entries.items())[:10]):
                print(f"  {alias_path}: {', '.join(sorted(list(children)[:5]))}")
                if len(children) > 5:
                    print(f"    ... and {len(children) - 5} more")
            if len(alias_entries) > 10:
                print(f"  ... and {len(alias_entries) - 10} more alias entries")
            
        block.parent_child_map.update(alias_entries) if alias_entries else None
        
        # Debug: Show the final map
        if VERBOSE_MODE:
            print(f"\nParent-child map entries (total: {len(block.parent_child_map)}):")
            for parent, children in sorted(list(block.parent_child_map.items())[:10]):
                print(f"  {parent}:")
                print(f"    Children ({len(children)}): {', '.join(sorted(list(children)[:10]))}")
                if len(children) > 10:
                    print(f"      ... and {len(children) - 10} more")
            if len(block.parent_child_map) > 10:
                print(f"  ... and {len(block.parent_child_map) - 10} more entries")
    
    def _validate_consistency(self, block: BlockInfo):
        """Validate extracted keywords for consistency"""
        print(f"\nValidation for {block.name} block:")
        print(f"  Total unique keyword IDs: {len(block.keywords)}")
        print(f"  Total unique keyword names: {len(block.keywords_by_name)}")
        print(f"  Total unique signatures: {len(block.keywords_by_signature)}")
        print(f"  Keywords with params: {sum(1 for k in block.keywords.values() if k.has_param)}")
        print(f"  Keywords without params: {sum(1 for k in block.keywords.values() if not k.has_param)}")
        print(f"  Parent-child relationships: {len(block.parent_child_map)}")
        
        # Show keywords with multiple IDs but same signature (duplicates)
        sig_to_ids = defaultdict(list)
        for kw_id, kw_info in block.keywords.items():
            sig = kw_info.signature()
            sig_to_ids[sig].append(kw_id)
        
        duplicates = {sig: ids for sig, ids in sig_to_ids.items() if len(ids) > 1}
        if duplicates:
            print(f"  Keywords with duplicate signatures (will use first): {len(duplicates)}")
            for sig, ids in list(duplicates.items())[:3]:
                kw_name = block.keywords[ids[0]].name
                print(f"    {kw_name}: {len(ids)} IDs with same signature")
        
        # Show keywords with different signatures
        multi_sig = {}
        for kw_name, id_list in block.keywords_by_name.items():
            sigs = set(block.keywords[kid].signature() for kid in id_list)
            if len(sigs) > 1:
                multi_sig[kw_name] = len(sigs)
        
        if multi_sig:
            print(f"  Keywords with multiple signatures: {len(multi_sig)}")
            for kw_name, count in list(multi_sig.items())[:5]:
                print(f"    {kw_name}: {count} different signatures")
        
        # Show top-level keywords
        top_level = block.parent_child_map.get(block.name, set())
        print(f"  Top-level keywords ({len(top_level)}): {', '.join(sorted(list(top_level)[:10]))}")
        if len(top_level) > 10:
            print(f"    ... and {len(top_level) - 10} more")


def main():
    parser = argparse.ArgumentParser(
        description='Generate Dakota parser with support for all blocks or single block with stubs',
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    
    parser.add_argument('xml_file', type=Path, help='Dakota XML specification')
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
    
    if not args.xml_file.exists():
        print(f"Error: XML file not found: {args.xml_file}", file=sys.stderr)
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
        
        print("Generating Dakota parser code from XML specification...")
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
            xml_parser = DakotaXMLParser(args.xml_file, args.block)
            block = xml_parser.parse()
            
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
            
            for block_name in ALL_BLOCKS:
                if VERBOSE_MODE:
                    print(f"--- Processing {block_name} block ---")
                
                # Parse and generate each block
                xml_parser = DakotaXMLParser(args.xml_file, block_name)
                block = xml_parser.parse()
                
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
                xml_parser = DakotaXMLParser(args.xml_file, block_name)
                all_blocks[block_name] = xml_parser.parse()
            
            generate_error_messages(all_blocks, source_output_dir)
            print(f"  Generated dakota_error_messages.hpp")
        
        # =====================================================================
        # PHASE 2: Generate test files (if requested)
        # =====================================================================
        
        if args.generate_tests:
            print()
            print("Generating tests...")
            
            # Create test directory
            test_output_dir.mkdir(parents=True, exist_ok=True)
            
            # Collect all block information (if not already done)
            if args.block:
                # For single block mode, only generate tests for that block
                xml_parser = DakotaXMLParser(args.xml_file, args.block)
                all_blocks = {args.block: xml_parser.parse()}
            else:
                # For all blocks mode, we already have all_blocks
                pass  # all_blocks already populated above
            
            # Generate constraint tests
            print("  Generating constraint tests...")
            constraint_test_count = 0
            
            try:
                # Import constraint test generator
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
            
            # Generate error message tests
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
