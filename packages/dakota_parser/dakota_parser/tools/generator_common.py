#!/usr/bin/env python3
"""
Dakota Parser Generator - Common Code

Shared code between XML and JSON parser generators.
Contains dataclasses, CodeGenerator, and helper functions.
"""

import argparse
import sys
from pathlib import Path
from typing import Dict, Set, Optional, List
from collections import defaultdict

# Global verbose flag (set by command-line argument)
VERBOSE_MODE = False

class KeywordInfo:
    """Information about a single keyword"""
    def __init__(self, id, name, has_param, param_type=None, aliases=None, parent=None, 
                 children=None, line_number=None, min_occurs=0, max_occurs=None, 
                 param_constraint=None, oneof_group=None):
        self.id = id
        self.name = name
        self.has_param = has_param
        self.param_type = param_type
        self.aliases = aliases if aliases is not None else []
        self.parent = parent
        self.children = children if children is not None else set()
        self.line_number = line_number
        self.min_occurs = min_occurs
        self.max_occurs = max_occurs
        self.param_constraint = param_constraint
        self.oneof_group = oneof_group
    
    def signature(self):
        """Return signature for deduplication - name + has_param + param_type"""
        return "{}:{}:{}".format(self.name, self.has_param, self.param_type or 'none')

class BlockInfo:
    """Information about a top-level block"""
    def __init__(self, name, keywords=None, keywords_by_name=None, keywords_by_signature=None,
                 parent_child_map=None, observed_relationships=None, oneof_groups=None,
                 alias_map=None, all_keywords_optional=False):
        from collections import defaultdict
        self.name = name
        self.keywords = keywords if keywords is not None else {}
        self.keywords_by_name = keywords_by_name if keywords_by_name is not None else defaultdict(list)
        self.keywords_by_signature = keywords_by_signature if keywords_by_signature is not None else {}
        self.parent_child_map = parent_child_map if parent_child_map is not None else defaultdict(set)
        self.observed_relationships = observed_relationships if observed_relationships is not None else defaultdict(set)
        self.oneof_groups = oneof_groups if oneof_groups is not None else {}
        self.alias_map = alias_map if alias_map is not None else {}
        self.all_keywords_optional = all_keywords_optional

# List of all Dakota blocks
ALL_BLOCKS = ["environment", "method", "model", "variables", "interface", "responses"]
class CodeGenerator:
    """Generate C++ code for staged parsing"""
    
    def __init__(self, block: BlockInfo, output_dir: Path, schema_defs: dict = None):
        self.block = block
        self.output_dir = output_dir
        self.schema_defs = schema_defs or {}
    
    def generate_all(self):
        """Generate all files for the block"""
        print(f"\nGenerating code for {self.block.name} block...")
        
        # Create block directory
        block_dir = self.output_dir / self.block.name
        block_dir.mkdir(parents=True, exist_ok=True)
        
        # Generate files
        self.generate_grammar(block_dir)
        self.generate_actions(block_dir)
        self.generate_parent_child_map()
        self.generate_block_parser(block_dir)
        self.generate_constraints(block_dir)
        self.generate_semantic_metadata(block_dir)
        
        print(f"\nGeneration complete!")
    
    def _make_struct_name(self, keyword_id: str) -> str:
        """Create C++ struct name for keyword based on unique ID"""
        safe_id = keyword_id.replace('-', '_').replace('.', '_')
        return f"{safe_id}_kw"
    
    def _build_keyword_path(self, kw_info, kw_id: str) -> List[str]:
        """
        Build all possible paths for a keyword by finding it in parent_child_map
        and validating against its parent attribute.
        
        Returns a list of full dotted paths where this keyword can appear.
        """
        keyword_paths = []
        
        # Search parent_child_map for this keyword name
        for parent_path, children in self.block.parent_child_map.items():
            if kw_info.name in children:
                # Found a path where this keyword appears
                # Validate that the parent matches
                path_parts = parent_path.split('.')
                
                if hasattr(kw_info, 'parent') and kw_info.parent:
                    # Check if the last part of the path matches the keyword's parent
                    if path_parts[-1] == kw_info.parent:
                        full_path = f"{parent_path}.{kw_info.name}"
                        keyword_paths.append(full_path)
                else:
                    # No parent attribute - this is a block-level keyword
                    # Only include if parent_path is just the block name
                    if parent_path == self.block.name:
                        full_path = f"{parent_path}.{kw_info.name}"
                        keyword_paths.append(full_path)
        
        # If no paths found, use just the keyword name as fallback
        if not keyword_paths:
            keyword_paths = [kw_info.name]
        
        return keyword_paths
    
    def _get_keyword_wrapper(self, struct_name: str, param_type: Optional[str]) -> str:
        """
        Get the appropriate keyword wrapper template based on parameter type.
        
        This ensures type-safe parsing - STRING types must be quoted, etc.
        """
        if param_type is None:
            return f"keyword_with_param<{struct_name}>"
        
        param_type_upper = param_type.upper()
        
        # Map param types to wrapper templates
        if param_type_upper in ('STRING', 'FILENAME', 'INPUT_FILE', 'OUTPUT_FILE'):
            return f"keyword_with_string<{struct_name}>"
        elif param_type_upper == 'STRINGLIST':
            return f"keyword_with_stringlist<{struct_name}>"
        elif param_type_upper == 'INTEGER':
            return f"keyword_with_integer<{struct_name}>"
        elif param_type_upper == 'INTEGERLIST':
            return f"keyword_with_integerlist<{struct_name}>"
        elif param_type_upper == 'REAL':
            return f"keyword_with_real<{struct_name}>"
        elif param_type_upper == 'REALLIST':
            return f"keyword_with_reallist<{struct_name}>"
        else:
            # Unknown type - use generic (may cause ambiguity)
            return f"keyword_with_param<{struct_name}>"
    
    def generate_grammar(self, block_dir: Path):
        """Generate flat grammar file"""
        output_file = block_dir / f'dakota_grammar_{self.block.name}.hpp'
        
        guard = f"DAKOTA_GRAMMAR_{self.block.name.upper()}_HPP"
        lines = []
        
        lines.append(f"#ifndef {guard}")
        lines.append(f"#define {guard}")
        lines.append("")
        lines.append('#include "dakota_grammar_common.hpp"')
        lines.append("")
        lines.append("namespace dakota {")
        lines.append("")
        lines.append("// " + "=" * 76)
        lines.append(f"// {self.block.name.upper()} KEYWORDS - FLAT (Order-Independent)")
        lines.append("// " + "=" * 76)
        lines.append("")
        
        # Collect ALL unique keyword texts (canonical names + aliases)
        # Map: keyword_text -> (has_param, representative_kw_info)
        # Also track keywords that appear both as flag and param (ambiguous)
        unique_texts = {}
        ambiguous_texts = set()  # Keywords that appear both as flag and param
        type_conflicts = {}  # Track keywords with conflicting param_types
        
        def get_more_permissive_type(type1, type2):
            """Return the more permissive of two param types.
            
            REALLIST > INTEGERLIST (reals can parse integers)
            When types are incompatible (e.g., REALLIST vs STRINGLIST), 
            return None to signal use of generic safe_value_list.
            None (generic) is most permissive - keep it if already set.
            """
            if type1 == type2:
                return type1
            
            # If either is already generic (None), keep generic
            if type1 is None or type2 is None:
                return None
            
            # Normalize to upper case for comparison
            t1 = type1.upper()
            t2 = type2.upper()
            
            # REALLIST can parse integers, so it's more permissive
            if set([t1, t2]) == {'REALLIST', 'INTEGERLIST'}:
                return 'REALLIST'
            
            # If one is a list and other is single, prefer list
            if 'LIST' in t1 and 'LIST' not in t2:
                return type1
            if 'LIST' in t2 and 'LIST' not in t1:
                return type2
            
            # REAL > INTEGER
            if set([t1, t2]) == {'REAL', 'INTEGER'}:
                return 'REAL'
            
            # Incompatible types (e.g., REALLIST vs STRINGLIST)
            # Return None to signal use of generic safe_value_list
            if ('STRING' in t1 and 'STRING' not in t2) or ('STRING' in t2 and 'STRING' not in t1):
                return None  # Use generic type
            
            # Default: keep first (or could use generic)
            return type1
        
        # First pass: collect canonical keywords and detect ambiguous ones
        for sig, canonical_id in sorted(self.block.keywords_by_signature.items()):
            kw_info = self.block.keywords[canonical_id]
            text = kw_info.name
            
            if text not in unique_texts:
                unique_texts[text] = (kw_info.has_param, kw_info)
            else:
                # Check if this is an ambiguous keyword (appears both as flag and param)
                existing_has_param, existing_kw = unique_texts[text]
                if existing_has_param != kw_info.has_param:
                    ambiguous_texts.add(text)
                    # Keep the parameterized version as the primary
                    if kw_info.has_param:
                        unique_texts[text] = (kw_info.has_param, kw_info)
                elif kw_info.has_param and existing_kw.param_type != kw_info.param_type:
                    # Same keyword with different param_types - pick most permissive
                    type_conflicts[text] = (existing_kw.param_type, kw_info.param_type)
                    better_type = get_more_permissive_type(existing_kw.param_type, kw_info.param_type)
                    if better_type is None:
                        # Incompatible types - use generic parser
                        # Create a modified kw_info with generic type
                        from copy import copy
                        generic_kw = copy(kw_info)
                        generic_kw.param_type = None  # Will use keyword_with_param
                        unique_texts[text] = (kw_info.has_param, generic_kw)
                    elif better_type == kw_info.param_type:
                        # Use the kw_info with the better type
                        unique_texts[text] = (kw_info.has_param, kw_info)
                    # else keep existing
        
        # Second pass: collect all aliases from ALL keywords (not just canonical)
        # This ensures context-specific aliases from duplicate keywords are included
        for kw_id, kw_info in self.block.keywords.items():
            for alias in kw_info.aliases:
                if alias not in unique_texts:
                    # Use same param status as source keyword
                    unique_texts[alias] = (kw_info.has_param, kw_info)
                else:
                    # Check for ambiguous aliases too
                    existing_has_param, existing_kw = unique_texts[alias]
                    if existing_has_param != kw_info.has_param:
                        ambiguous_texts.add(alias)
                        if kw_info.has_param:
                            unique_texts[alias] = (kw_info.has_param, kw_info)
                    elif kw_info.has_param and existing_kw.param_type != kw_info.param_type:
                        # Alias with different param_type - pick most permissive
                        type_conflicts[alias] = (existing_kw.param_type, kw_info.param_type)
                        better_type = get_more_permissive_type(existing_kw.param_type, kw_info.param_type)
                        if better_type is None:
                            # Incompatible types - use generic parser
                            from copy import copy
                            generic_kw = copy(kw_info)
                            generic_kw.param_type = None
                            unique_texts[alias] = (kw_info.has_param, generic_kw)
                        elif better_type == kw_info.param_type:
                            unique_texts[alias] = (kw_info.has_param, kw_info)
        
        if ambiguous_texts:
            print(f"  Ambiguous keywords (both flag and param): {sorted(ambiguous_texts)}")
        
        if type_conflicts:
            print(f"  Type conflicts resolved (using most permissive): {type_conflicts}")
        
        # Generate keyword structs - ONE per unique text
        # Use block-specific names to avoid collisions across blocks
        for text in sorted(unique_texts.keys()):
            has_param, representative_kw = unique_texts[text]
            struct_name = f"{self.block.name}_{text}_kw"  # Block-specific!
            
            # Build informative comment
            comment_parts = []
            
            # Find all canonical keywords that have this text
            canonical_with_text = [ki for ki in self.block.keywords.values() if ki.name == text]
            if canonical_with_text:
                comment_parts.append(f"canonical: {canonical_with_text[0].id}")
            
            # Find all canonical keywords that have this as an alias
            alias_sources = [ki.name for ki in self.block.keywords.values() if text in ki.aliases]
            if alias_sources:
                comment_parts.append(f"alias_for: {', '.join(sorted(set(alias_sources)))}")
            
            if has_param:
                comment_parts.append(f"param: {representative_kw.param_type}")
            
            lines.append(f"// {text} - {' | '.join(comment_parts)}")
            lines.append(f"struct {struct_name} : pegtl::seq<")
            lines.append(f'    TAO_PEGTL_STRING("{text}"),')
            lines.append("    pegtl::not_at<pegtl::sor<pegtl::alnum, pegtl::one<'_'>>>")
            lines.append("> {};")
            lines.append("")
        
        # Generate catch-all for truncated/unknown keywords
        lines.append("// " + "=" * 76)
        lines.append("// CATCH-ALL FOR TRUNCATED/UNKNOWN KEYWORDS")
        lines.append("// " + "=" * 76)
        lines.append("")
        lines.append("// Unknown keyword: identifier that doesn't match known keywords")
        lines.append(f"struct {self.block.name}_unknown_keyword : pegtl::seq<")
        lines.append("    pegtl::identifier,")
        lines.append("    pegtl::not_at<pegtl::sor<pegtl::alnum, pegtl::one<'_'>>>")
        lines.append("> {};")
        lines.append("")
        
        # Generate entry point (flat list) - one per unique text
        lines.append("// " + "=" * 76)
        lines.append(f"// {self.block.name.upper()} ENTRY POINT")
        lines.append("// " + "=" * 76)
        lines.append("")
        lines.append(f"struct {self.block.name}_entry : pegtl::sor<")
        
        entry_rules = []
        # Use the unified keyword texts
        for text in sorted(unique_texts.keys()):
            has_param, kw_info = unique_texts[text]
            struct_name = f"{self.block.name}_{text}_kw"  # Block-specific!
            
            # For ambiguous keywords (appearing both as flag and param in different contexts),
            # use keyword_with_optional_param which accepts both forms.
            # Semantic analysis will validate the correct usage based on context.
            if text in ambiguous_texts:
                wrapper = f"keyword_with_optional_param<{struct_name}>"
            elif has_param:
                # Select type-specific template based on param_type
                wrapper = self._get_keyword_wrapper(struct_name, kw_info.param_type)
            else:
                wrapper = f"keyword_flag<{struct_name}>"
            
            entry_rules.append(f"    {wrapper}")
        
        # Add catch-all rules at the end (lowest priority).
        # Use keyword_with_optional_param (safe_value_list: numbers + quoted strings only),
        # NOT keyword_with_param (value_list: also matches bare identifiers).
        # keyword_with_param would greedily consume the next keyword name as a string
        # value when an unrecognised truncated keyword appears (e.g. "asynch" consuming
        # "evaluation_concurrency" before it can be parsed as its own keyword).
        entry_rules.append(f"    keyword_with_optional_param<{self.block.name}_unknown_keyword>")
        entry_rules.append(f"    keyword_flag<{self.block.name}_unknown_keyword>")
        
        lines.append(",\n".join(entry_rules))
        lines.append("> {};")
        lines.append("")
        
        # Generate content rule
        # If all keywords are optional, the block can be empty (star); otherwise require at least one (plus)
        quantifier = "star" if self.block.all_keywords_optional else "plus"
        comment = "zero or more keywords (all optional)" if quantifier == "star" else "at least one keyword required"
        lines.append(f"// {self.block.name.upper()} content - {comment}")
        lines.append(f"struct {self.block.name}_content : pegtl::seq<")
        lines.append("    opt_value_separator,")
        lines.append(f"    pegtl::{quantifier}<pegtl::seq<{self.block.name}_entry, opt_value_separator>>")
        lines.append("> {};")
        lines.append("")
        
        lines.append("} // namespace dakota")
        lines.append("")
        lines.append(f"#endif // {guard}")
        lines.append("")
        
        output_file.write_text('\n'.join(lines), encoding='utf-8')
        if VERBOSE_MODE:
            print(f"  Generated {output_file.name}")
    
    def _compute_reallist_keywords(self) -> set:
        """Return keyword names that have REAL or REALLIST type in any context.
        These need int64_t→double coercion at Phase 2 placement time when the
        grammar wrapper is generic (e.g. due to STRINGLIST type conflicts)."""
        reallist_names = set()
        for kw_id, kw in self.block.keywords.items():
            if kw.has_param and kw.param_type:
                pt = kw.param_type.upper()
                if 'REAL' in pt:
                    reallist_names.add(kw.name)
        return reallist_names

    def _compute_union_default_variants(self) -> dict:
        """Return map of {union_dotted_path: default_variant_name} for Pass 3 tiebreaker.
        When Pass 3 has tied candidates, prefer the path whose next component
        matches the union's x-model-default (e.g. solution_mode → online_pilot)."""
        import re as _re
        defs = self.schema_defs
        pcm = self.block.parent_child_map
        result = {}
        for defname, defn in defs.items():
            for prop, pschema in defn.get('properties', {}).items():
                xmd = pschema.get('x-model-default')
                if not xmd or not pschema.get('x-union-pattern'): continue
                snake = _re.sub(r'(?<!^)(?=[A-Z])', '_', xmd).lower()
                for pcm_path, children in pcm.items():
                    if prop not in children: continue
                    union_path = pcm_path + '.' + prop
                    if snake in pcm.get(union_path, set()):
                        result[union_path] = snake
        return result

    def _compute_integerlist_keywords(self) -> set:
        """Return keyword names that are ONLY INTEGER/INTEGERLIST in ALL contexts.
        These need double→int64_t coercion when scientific notation like 1.E5
        is parsed as real_value fallback in integer_or_repeat.
        Keywords that also appear as REALLIST (e.g. initial_point) must NOT
        be coerced — they legitimately hold floating-point values."""
        integer_names = set()
        non_integer_names = set()  # any name that appears as REAL in any context
        for kw_id, kw in self.block.keywords.items():
            if kw.has_param and kw.param_type:
                pt = kw.param_type.upper()
                if 'REAL' in pt or 'STRING' in pt:
                    non_integer_names.add(kw.name)
                elif 'INTEGER' in pt:
                    integer_names.add(kw.name)
        return integer_names - non_integer_names

    def generate_actions(self, block_dir: Path):
        """Generate actions file with reorganization"""
        output_file = block_dir / f'dakota_actions_{self.block.name}.hpp'
        
        guard = f"DAKOTA_ACTIONS_{self.block.name.upper()}_HPP"
        lines = []
        
        lines.append(f"#ifndef {guard}")
        lines.append(f"#define {guard}")
        lines.append("")
        lines.append('#include "dakota_actions_common.hpp"')
        lines.append(f'#include "{self.block.name}_parent_child_map.hpp"')
        lines.append(f'#include "dakota_semantic_{self.block.name}.hpp"')
        lines.append(f'#include "dakota_grammar_{self.block.name}.hpp"')
        lines.append('#include <unordered_set>')
        lines.append("")
        lines.append("namespace dakota {")
        lines.append("namespace actions {")
        lines.append("")
        lines.append("using namespace dakota::actions;")
        lines.append("")
        lines.append("// Global vector to track keywords in parse order")
        lines.append("// This preserves the order keywords appeared in the input file")
        lines.append("// which is critical for correct parent assignment when multiple parents")
        lines.append("// can have children with the same name")
        lines.append(f"static std::vector<std::shared_ptr<KeywordNode>> {self.block.name}_parse_order;")
        lines.append("")
        
        # Phase 2 reorganization function with 6-step algorithm
        lines.append("// " + "=" * 76)
        lines.append(f"// PHASE 2: {self.block.name.upper()} HIERARCHY REORGANIZATION")
        lines.append("// " + "=" * 76)
        lines.append("")
        
        lines.append(f"inline void reorganize_{self.block.name}_hierarchy(Block* block) {{")
        lines.append("    if (!block) return;")
        lines.append("")
        lines.append('    DEBUG_OUT("[DEBUG] Phase 2: Reorganizing ' + self.block.name + ' block hierarchy (Order-Preserving Algorithm)\\n");')
        lines.append("")
        lines.append(f"    const auto& parent_child_map = {self.block.name}_parent_child_map;")
        lines.append("")
        lines.append("    // Step 1: Get keywords in parse order")
        lines.append("    // Use the globally tracked parse order vector instead of iterating over block->keywords")
        lines.append("    // (which is a map and would give alphabetical order, not parse order)")
        lines.append(f"    auto& collected_keywords = {self.block.name}_parse_order;")
        lines.append("")
        lines.append('    DEBUG_OUT("[DEBUG] Phase 2: Total collected keywords: " << collected_keywords.size() << "\\n");')
        lines.append('    for (const auto& kw : collected_keywords) {')
        lines.append('        DEBUG_OUT("[DEBUG]   Collected: " << kw->name);')
        lines.append('        if (kw->unresolved) DEBUG_OUT(" (UNRESOLVED)");')
        lines.append('        DEBUG_OUT("\\n");')
        lines.append('    }')
        lines.append("")
        lines.append("    // Clear the block - we'll rebuild it")
        lines.append("    block->keywords.clear();")
        lines.append("")

        # ---- Emit static set of REALLIST-typed keyword names ----
        # Keywords that have REALLIST type in any context get their
        # int64_t param values coerced to double at placement time.
        # This fixes the case where the grammar collapses to generic
        # keyword_with_param (due to STRINGLIST conflicts) and bare
        # integers like "1" are stored as int64_t instead of double.
        reallist_kws = self._compute_reallist_keywords()
        if reallist_kws:
            kw_literals = ", ".join(f'"{k}"' for k in sorted(reallist_kws))
            lines.append("    // Keywords whose param values must be stored as double,")
            lines.append("    // even when the generic value_item parser stored them as int64_t.")
            lines.append(f"    static const std::unordered_set<std::string> reallist_keywords = {{{kw_literals}}};")
        else:
            lines.append("    static const std::unordered_set<std::string> reallist_keywords = {};")
        lines.append("")
        integerlist_kws = self._compute_integerlist_keywords()
        if integerlist_kws:
            int_literals = ", ".join(f'"{k}"' for k in sorted(integerlist_kws))
            lines.append("    // Keywords whose param values must be stored as int64_t,")
            lines.append("    // even when scientific notation caused them to parse as double.")
            lines.append(f"    static const std::unordered_set<std::string> integerlist_keywords = {{{int_literals}}};")
        else:
            lines.append("    static const std::unordered_set<std::string> integerlist_keywords = {};")
        lines.append("")
        udvs = self._compute_union_default_variants()
        if udvs:
            udv_entries = ", ".join(f'{{"{k}", "{v}"}}' for k, v in sorted(udvs.items()))
            lines.append("    // Union x-model-default variants: used in Pass 3 tiebreaker.")
            lines.append(f"    static const std::map<std::string, std::string> union_default_variants = {{{udv_entries}}};")
        else:
            lines.append("    static const std::map<std::string, std::string> union_default_variants = {};")
        lines.append("")
        lines.append("")
        lines.append(f"    // Step 2: Create processing stack with block keyword (\"{self.block.name}\")")
        lines.append("    // The stack represents our current nesting context")
        lines.append("    struct StackEntry {")
        lines.append("        std::string name;")
        lines.append("        std::string dotted_path;")
        lines.append("        std::shared_ptr<KeywordNode> node;  // nullptr for block itself")
        lines.append("    };")
        lines.append("")
        lines.append("    std::vector<StackEntry> processing_stack;")
        lines.append(f'    processing_stack.push_back({{"{self.block.name}", "{self.block.name}", nullptr}});')
        lines.append("")
        lines.append(f'    DEBUG_OUT("[DEBUG] Phase 2: Starting with stack: [{self.block.name}]\\n");')
        lines.append("")
        lines.append("    // Step 3: Process keywords with deferred retry mechanism")
        lines.append("    // Keywords that don't match initially are retried after each successful match")
        lines.append("    // Implicit container creation (Pass 3) only runs as a LAST RESORT")
        lines.append("    std::vector<std::shared_ptr<KeywordNode>> deferred_keywords;")
        lines.append("    std::vector<std::shared_ptr<KeywordNode>> remaining_keywords = collected_keywords;")
        lines.append("    bool try_implicit_containers = false;  // Only true on final retry")
        lines.append("")
        lines.append("    // Process keywords with restart-on-progress strategy")
        lines.append("    // When a keyword is successfully matched, we restart from the beginning")
        lines.append("    // of remaining keywords to give deferred keywords a chance with the new stack")
        lines.append("    while (!remaining_keywords.empty()) {")
        lines.append("        bool made_progress_this_round = false;")
        lines.append("        size_t i = 0;")
        lines.append("")
        lines.append("        while (i < remaining_keywords.size()) {")
        lines.append("            auto& kw = remaining_keywords[i];")
        lines.append('            DEBUG_OUT("[DEBUG] --- Processing keyword: " << kw->name << "\\n");')
        lines.append("")
        lines.append("            std::string kw_name = kw->name;")
        lines.append("")
        lines.append("            // Step 3a: TWO-PASS resolution to find valid parent")
        lines.append("            // PASS 1: Check all stack levels for EXACT matches (direct or alias)")
        lines.append("            // PASS 2: If no exact match, try truncation resolution")
        lines.append("            int parent_index = -1;")
        lines.append("            std::string resolved_name = kw_name;")
        lines.append("")
        lines.append("            // PASS 1: Check all stack levels for exact matches")
        lines.append('            DEBUG_OUT("[DEBUG]   Pass 1: Checking for exact matches\\n");')
        lines.append("            for (int i = processing_stack.size() - 1; i >= 0; --i) {")
        lines.append("            const auto& potential_parent = processing_stack[i];")
        lines.append("")
        lines.append("            auto map_it = parent_child_map.find(potential_parent.dotted_path);")
        lines.append("            if (map_it == parent_child_map.end()) {")
        lines.append("                continue;")
        lines.append("            }")
        lines.append("")
        lines.append("            const auto& valid_children = map_it->second;")
        lines.append("")
        lines.append("            // Check for direct match")
        lines.append("            if (valid_children.find(kw_name) != valid_children.end()) {")
        lines.append('                DEBUG_OUT("[DEBUG]     Exact match found at level " << i << " (" << potential_parent.name << ")\\n");')
        lines.append("                parent_index = i;")
        lines.append("                break;  // Exact match - done!")
        lines.append("            }")
        lines.append("")
        lines.append("            // Check if keyword is an alias that resolves to a valid child")
        lines.append(f"            auto alias_key = std::make_pair(potential_parent.dotted_path, kw_name);")
        lines.append(f"            auto alias_it = {self.block.name}_semantic::get_alias_map().find(alias_key);")
        lines.append(f"            if (alias_it != {self.block.name}_semantic::get_alias_map().end()) {{")
        lines.append(f"                const std::string& canonical_name = alias_it->second;")
        lines.append(f"                if (valid_children.find(canonical_name) != valid_children.end()) {{")
        lines.append('                    DEBUG_OUT("[DEBUG]     Alias match found at level " << i << ": " << kw_name << " -> " << canonical_name << "\\n");')
        lines.append(f"                    resolved_name = canonical_name;")
        lines.append(f"                    kw->resolved_name = resolved_name;")
        lines.append(f"                    kw->unresolved = false;")
        lines.append(f"                    parent_index = i;")
        lines.append(f"                    break;  // Alias match - done!")
        lines.append(f"                }}")
        lines.append(f"            }}")
        lines.append("            }")
        lines.append("")
        lines.append("            // PASS 2: If no exact match, try truncation resolution")
        lines.append("            if (parent_index == -1) {")
        lines.append('                DEBUG_OUT("[DEBUG]   Pass 2: No exact match found, trying truncation resolution\\n");')
        lines.append("")
        lines.append("                for (int i = processing_stack.size() - 1; i >= 0; --i) {")
        lines.append("                    const auto& potential_parent = processing_stack[i];")
        lines.append('                    DEBUG_OUT("[DEBUG]     Checking truncations against \'" << potential_parent.name << "\' (path: " << potential_parent.dotted_path << ")\\n");')
        lines.append("")
        lines.append("                    auto map_it = parent_child_map.find(potential_parent.dotted_path);")
        lines.append("                    if (map_it == parent_child_map.end()) {")
        lines.append('                        DEBUG_OUT("[DEBUG]       Parent has no children in map\\n");')
        lines.append("                        continue;")
        lines.append("                    }")
        lines.append("")
        lines.append("                    const auto& valid_children = map_it->second;")
        lines.append("")
        lines.append("                    std::vector<std::string> matches;")
        lines.append("                    for (const auto& valid_child : valid_children) {")
        lines.append("                        if (valid_child.rfind(kw_name, 0) == 0) {  // starts_with")
        lines.append("                            matches.push_back(valid_child);")
        lines.append("                        }")
        lines.append("                    }")
        lines.append("                    // Also check aliases: if kw_name is a prefix of an alias")
        lines.append("                    // that maps to a valid canonical child, use that.")
        lines.append("                    // e.g. 'sobol' is a prefix of alias 'sobol_sequence'")
        lines.append("                    // which maps to canonical 'digital_net'.")
        lines.append("                    if (matches.empty()) {")
        lines.append(f"                        const auto& alias_map = {self.block.name}_semantic::get_alias_map();")
        lines.append("                        for (const auto& [alias_key, canonical] : alias_map) {")
        lines.append("                            if (alias_key.first != potential_parent.dotted_path) continue;")
        lines.append("                            const std::string& alias_name = alias_key.second;")
        lines.append("                            if (alias_name.rfind(kw_name, 0) != 0) continue;  // kw_name not a prefix of alias")
        lines.append("                            if (valid_children.find(canonical) == valid_children.end()) continue;")
        lines.append("                            // Unique alias prefix match: resolve to canonical")
        lines.append("                            matches.push_back(canonical);")
        lines.append('                            DEBUG_OUT("[DEBUG]       Alias prefix match: " << kw_name << " -> alias " << alias_name << " -> canonical " << canonical << "\\n");')
        lines.append("                        }")
        lines.append("                    }")
        lines.append("")
        lines.append("                    // Check if truncation is valid (unique and not ambiguous)")
        lines.append("                    if (matches.size() == 1) {")
        lines.append('                        DEBUG_OUT("[DEBUG]       Resolved truncation: " << kw_name << " -> " << matches[0] << "\\n");')
        lines.append("                        resolved_name = matches[0];")
        lines.append("                        kw->resolved_name = resolved_name;")
        lines.append("                        kw->unresolved = false;")
        lines.append("                        parent_index = i;")
        lines.append("                        break;")
        lines.append("                    } else if (matches.size() > 1) {")
        lines.append("                        // Check for ambiguity")
        lines.append("                        bool ambiguous = false;")
        lines.append("                        size_t trunc_len = kw_name.length();")
        lines.append("                        for (size_t mi = 0; mi < matches.size() && !ambiguous; ++mi) {")
        lines.append("                            for (size_t mj = mi + 1; mj < matches.size(); ++mj) {")
        lines.append("                                if (matches[mi].substr(0, trunc_len) == matches[mj].substr(0, trunc_len)) {")
        lines.append("                                    ambiguous = true;")
        lines.append("                                    break;")
        lines.append("                                }")
        lines.append("                            }")
        lines.append("                        }")
        lines.append("")
        lines.append("                        if (!ambiguous && matches.size() > 0) {")
        lines.append("                            // Pick first match (all have different prefixes)")
        lines.append('                            DEBUG_OUT("[DEBUG]       Resolved truncation (multiple matches, not ambiguous): " << kw_name << " -> " << matches[0] << "\\n");')
        lines.append("                            resolved_name = matches[0];")
        lines.append("                            kw->resolved_name = resolved_name;")
        lines.append("                            kw->unresolved = false;")
        lines.append("                            parent_index = i;")
        lines.append("                            break;")
        lines.append("                        }")
        lines.append("                    }")
        lines.append("                }")
        lines.append("            }")
        lines.append("")
        lines.append("            // PASS 3: If still no match and on final retry, try creating implicit containers")
        lines.append("            // Some keywords require a parent container that doesn't appear in DSL")
        lines.append("            // e.g., 'direct' requires 'analysis_drivers' container")
        lines.append("            // Only run this on the final pass to avoid creating containers prematurely")
        lines.append("            if (parent_index == -1 && try_implicit_containers) {")
        lines.append('                DEBUG_OUT("[DEBUG]   Pass 3: Trying implicit container creation\\n");')
        lines.append("")
        lines.append("                // Search all paths in parent_child_map for this keyword.")
        lines.append("                // Prefer the path that shares the longest common prefix with")
        lines.append("                // the current stack — this prevents choosing a bayes_calibration")
        lines.append("                // path alphabetically over the correct mfpc path when the stack")
        lines.append("                // already has mfpc as an ancestor.")
        lines.append("                std::string found_path;")
        lines.append("                size_t best_prefix_len = 0;")
        lines.append("                std::string best_alias_resolved;")
        lines.append("                for (const auto& [path, children] : parent_child_map) {")
        lines.append("                    // Exact match on kw_name or resolved_name")
        lines.append("                    bool matches_kw = children.find(kw_name) != children.end() ||")
        lines.append("                                      children.find(resolved_name) != children.end();")
        lines.append("                    std::string candidate_alias;")
        lines.append("                    // Truncation match: kw_name is a prefix of a canonical child")
        lines.append("                    // (e.g. 'scale_type' is prefix of 'scale_types').")
        lines.append("                    // Resolve to that child just as Pass 2 would.")
        lines.append("                    if (!matches_kw) {")
        lines.append("                        std::vector<std::string> trunc_matches;")
        lines.append("                        for (const auto& child : children) {")
        lines.append("                            if (child.rfind(kw_name, 0) == 0 && child != kw_name)")
        lines.append("                                trunc_matches.push_back(child);")
        lines.append("                        }")
        lines.append("                        if (trunc_matches.size() == 1) {")
        lines.append("                            matches_kw = true;")
        lines.append("                            candidate_alias = trunc_matches[0];  // resolve to full canonical name")
        lines.append("                        }")
        lines.append("                    }")
        lines.append("                    if (!matches_kw) {")
        lines.append(f"                        auto alias_key = std::make_pair(path, kw_name);")
        lines.append(f"                        auto alias_it = {self.block.name}_semantic::get_alias_map().find(alias_key);")
        lines.append(f"                        if (alias_it != {self.block.name}_semantic::get_alias_map().end() &&")
        lines.append("                            children.find(alias_it->second) != children.end()) {")
        lines.append("                            matches_kw = true;")
        lines.append("                            candidate_alias = alias_it->second;")
        lines.append("                        }")
        lines.append("                    }")
        lines.append("                    if (!matches_kw) continue;")
        lines.append("                    // Compute how many stack levels share a prefix with this path")
        lines.append("                    size_t prefix_len = 0;")
        lines.append("                    for (int si = 0; si < (int)processing_stack.size(); ++si) {")
        lines.append("                        const std::string& sp = processing_stack[si].dotted_path;")
        lines.append("                        if (path.rfind(sp, 0) == 0 &&")
        lines.append("                            (path.length() == sp.length() || path[sp.length()] == '.')) {")
        lines.append("                            prefix_len = sp.length();")
        lines.append("                        }")
        lines.append("                    }")
        lines.append("                    // Tiebreaker: when prefix_len ties, prefer the path whose")
        lines.append("                    // route through the AST follows committed nodes most deeply.")
        lines.append("                    // e.g. with collocation_ratio committed under expansion_order,")
        lines.append("                    // prefer pc.eo.collocation_ratio over pc.eo.collocation_points.")
        lines.append("                    bool is_better = found_path.empty() || prefix_len > best_prefix_len;")
        lines.append("                    if (!is_better && prefix_len == best_prefix_len) {")
        lines.append("                        // Count committed steps along candidate path after the prefix")
        lines.append("                        auto count_committed_depth = [&](const std::string& cand_path) -> int {")
        lines.append("                            // Find the ancestor node on the stack at prefix depth")
        lines.append("                            const KeywordNode* node = nullptr;")
        lines.append("                            for (int si = (int)processing_stack.size() - 1; si >= 0; --si) {")
        lines.append("                                if (processing_stack[si].dotted_path.length() == prefix_len) {")
        lines.append("                                    node = processing_stack[si].node.get(); break;")
        lines.append("                                }")
        lines.append("                            }")
        lines.append("                            // Parse remaining path components")
        lines.append("                            std::string rem = cand_path.substr(prefix_len);")
        lines.append("                            if (!rem.empty() && rem[0] == '.') rem = rem.substr(1);")
        lines.append("                            int depth = 0;")
        lines.append("                            while (!rem.empty()) {")
        lines.append("                                size_t dot = rem.find('.');")
        lines.append("                                std::string cpt = (dot != std::string::npos) ? rem.substr(0, dot) : rem;")
        lines.append('                                rem = (dot != std::string::npos) ? rem.substr(dot + 1) : "";')
        lines.append("                                // Check committed children (node) or block keywords (root)")
        lines.append("                                if (node) {")
        lines.append("                                    auto ci = node->children.find(cpt);")
        lines.append("                                    if (ci == node->children.end() || ci->second.empty()) break;")
        lines.append("                                    node = ci->second.back().get();")
        lines.append("                                } else {")
        lines.append("                                    auto ki = block->keywords.find(cpt);")
        lines.append("                                    if (ki == block->keywords.end() || ki->second.empty()) break;")
        lines.append("                                    node = ki->second.back().get();")
        lines.append("                                }")
        lines.append("                                ++depth;")
        lines.append("                            }")
        lines.append("                            return depth;")
        lines.append("                        };")
        lines.append("                        int new_depth = count_committed_depth(path);")
        lines.append("                        int cur_depth = count_committed_depth(found_path);")
        lines.append("                        if (new_depth > cur_depth) is_better = true;")
        lines.append("                        // Secondary tiebreaker: if depths still tie,")
        lines.append("                        // prefer path whose variant component matches")
        lines.append("                        // the union x-model-default (e.g. online_pilot).")
        lines.append("                        if (!is_better && new_depth == cur_depth && prefix_len > 0) {")
        lines.append("                            auto next_after = [](const std::string& p, size_t pfx) {")
        lines.append("                                std::string a = p.substr(pfx);")
        lines.append("                                if (!a.empty() && a[0] == '.') a = a.substr(1);")
        lines.append("                                size_t d = a.find('.');")
        lines.append("                                return d != std::string::npos ? a.substr(0, d) : a;")
        lines.append("                            };")
        lines.append("                            std::string new_sel = next_after(path, prefix_len);")
        lines.append("                            std::string cur_sel = next_after(found_path, prefix_len);")
        lines.append("                            // Look up one level deeper: prefix + '.' + sel = union path")
        lines.append("                            std::string anc = path.substr(0, prefix_len);")
        lines.append("                            std::string new_first = next_after(path, prefix_len);")
        lines.append("                            if (new_first == cur_sel) {")
        lines.append("                                // Same first component — check second")
        lines.append("                                std::string u_path = anc + \".\" + new_first;")
        lines.append("                                size_t u_len = u_path.size();")
        lines.append("                                std::string new_var = next_after(path, u_len);")
        lines.append("                                std::string cur_var = next_after(found_path, u_len);")
        lines.append("                                auto udv_it = union_default_variants.find(u_path);")
        lines.append("                                if (udv_it != union_default_variants.end()")
        lines.append("                                    && new_var == udv_it->second")
        lines.append("                                    && cur_var != udv_it->second)")
        lines.append("                                    is_better = true;")
        lines.append("                            } else {")
        lines.append("                                // Different first component — new_sel is the union selector")
        lines.append("                                std::string u_path = anc;")
        lines.append("                                auto udv_it = union_default_variants.find(u_path + \".\" + new_sel);")
        lines.append("                                // not this level; check if anc itself is a union")
        lines.append("                                udv_it = union_default_variants.find(anc);")
        lines.append("                                if (udv_it != union_default_variants.end()")
        lines.append("                                    && new_sel == udv_it->second")
        lines.append("                                    && cur_sel != udv_it->second)")
        lines.append("                                    is_better = true;")
        lines.append("                            }")
        lines.append("                        }")
        lines.append("                    }")
        lines.append("                    if (is_better) {")
        lines.append("                        found_path = path;")
        lines.append("                        best_prefix_len = prefix_len;")
        lines.append("                        if (!candidate_alias.empty()) {")
        lines.append("                            best_alias_resolved = candidate_alias;")
        lines.append("                        } else {")
        lines.append("                            best_alias_resolved.clear();")
        lines.append("                        }")
        lines.append("                    }")
        lines.append("                }")
        lines.append("                if (!best_alias_resolved.empty()) {")
        lines.append("                    resolved_name = best_alias_resolved;")
        lines.append("                    kw->resolved_name = resolved_name;")
        lines.append("                    kw->unresolved = false;")
        lines.append("                }")
        lines.append("")
        lines.append("                if (!found_path.empty()) {")
        lines.append('                    DEBUG_OUT("[DEBUG]     Found keyword at path: " << found_path << "\\n");')
        lines.append("")
        lines.append("                    // Search backwards through stack to find best ancestor")
        lines.append("                    // e.g., if found_path is 'interface.analysis_drivers' and stack has")
        lines.append("                    // [interface, id_interface], we need to pop to 'interface' level")
        lines.append("                    int best_ancestor_index = -1;")
        lines.append("                    for (int si = processing_stack.size() - 1; si >= 0; --si) {")
        lines.append("                        const std::string& ancestor_path = processing_stack[si].dotted_path;")
        lines.append("                        // Check if found_path starts with this ancestor's path")
        lines.append("                        if (found_path.rfind(ancestor_path, 0) == 0 &&")
        lines.append("                            (found_path.length() == ancestor_path.length() ||")
        lines.append("                             found_path[ancestor_path.length()] == '.')) {")
        lines.append("                            best_ancestor_index = si;")
        lines.append('                            DEBUG_OUT("[DEBUG]     Found ancestor at level " << si << ": " << ancestor_path << "\\n");')
        lines.append("                            break;")
        lines.append("                        }")
        lines.append("                    }")
        lines.append("")
        lines.append("                    if (best_ancestor_index >= 0) {")
        lines.append("                        // Pop stack down to ancestor level")
        lines.append("                        while (processing_stack.size() > static_cast<size_t>(best_ancestor_index + 1)) {")
        lines.append("                            auto popped = processing_stack.back();")
        lines.append('                            DEBUG_OUT("[DEBUG]       Popping to reach ancestor: " << popped.name << "\\n");')
        lines.append("                            processing_stack.pop_back();")
        lines.append("                            if (popped.node && !processing_stack.empty()) {")
        lines.append("                                auto& parent = processing_stack.back();")
        lines.append("                                if (parent.node) {")
        lines.append("                                    parent.node->children[popped.name].push_back(popped.node);")
        lines.append("                                } else {")
        lines.append("                                    block->keywords[popped.name].push_back(popped.node);")
        lines.append("                                }")
        lines.append("                            }")
        lines.append("                        }")
        lines.append("")
        lines.append("                        // Extract the missing container name(s)")
        lines.append("                        std::string ancestor_path = processing_stack.back().dotted_path;")
        lines.append("                        std::string remainder = found_path.substr(ancestor_path.length());")
        lines.append("                        if (!remainder.empty() && remainder[0] == '.') remainder = remainder.substr(1);")
        lines.append("")
        lines.append("                        // Split remainder by '.' to get container hierarchy")
        lines.append("                        std::vector<std::string> containers;")
        lines.append("                        size_t pos = 0;")
        lines.append("                        while ((pos = remainder.find('.')) != std::string::npos) {")
        lines.append("                            containers.push_back(remainder.substr(0, pos));")
        lines.append("                            remainder = remainder.substr(pos + 1);")
        lines.append("                        }")
        lines.append("                        if (!remainder.empty()) {")
        lines.append("                            containers.push_back(remainder);")
        lines.append("                        }")
        lines.append("")
        lines.append("                        // Create implicit containers, reusing any already-committed")
        lines.append("                        // nodes of the same name to avoid duplicates.")
        lines.append("                        for (const auto& container_name : containers) {")
        lines.append("                            auto& cur = processing_stack.back();")
        lines.append("                            std::string container_path = cur.dotted_path + \".\" + container_name;")
        lines.append("                            std::shared_ptr<KeywordNode> container_kw;")
        lines.append("                            if (cur.node) {")
        lines.append("                                auto ci = cur.node->children.find(container_name);")
        lines.append("                                if (ci != cur.node->children.end() && !ci->second.empty()) {")
        lines.append("                                    container_kw = ci->second.back();")
        lines.append("                                    ci->second.pop_back();")
        lines.append("                                    if (ci->second.empty()) cur.node->children.erase(ci);")
        lines.append('                                    DEBUG_OUT("[DEBUG]       Re-using committed container: " << container_name << "\\\\n");')
        lines.append("                                }")
        lines.append("                            } else {")
        lines.append("                                auto ki = block->keywords.find(container_name);")
        lines.append("                                if (ki != block->keywords.end() && !ki->second.empty()) {")
        lines.append("                                    container_kw = ki->second.back();")
        lines.append("                                    ki->second.pop_back();")
        lines.append("                                    if (ki->second.empty()) block->keywords.erase(ki);")
        lines.append('                                    DEBUG_OUT("[DEBUG]       Re-using committed block container: " << container_name << "\\\\n");')
        lines.append("                                }")
        lines.append("                            }")
        lines.append("                            if (!container_kw) {")
        lines.append('                                DEBUG_OUT("[DEBUG]       Creating implicit container: " << container_name << "\\\\n");')
        lines.append("                                container_kw = create_keyword(container_name, true);")
        lines.append("                            }")
        lines.append("                            processing_stack.push_back({container_name, container_path, container_kw});")
        lines.append("                        }")
        lines.append("                        // Now we can place the keyword")
        lines.append("                        parent_index = processing_stack.size() - 1;")
        lines.append("                    }")
        lines.append("                }")
        lines.append("            }")
        lines.append("")
        lines.append("            // Step 3b: If parent found, handle keyword placement")
        lines.append("            if (parent_index >= 0) {")
        lines.append("                // Coerce int64_t param values to double for REALLIST-typed keywords.")
        lines.append("                // Needed when the grammar used generic keyword_with_param (due to")
        lines.append("                // STRINGLIST type conflicts) and bare integers were stored as int64_t.")
        lines.append("                if (reallist_keywords.count(resolved_name)) {")
        lines.append("                    for (auto& v : kw->param_values) {")
        lines.append("                        if (std::holds_alternative<int64_t>(v)) {")
        lines.append("                            v = static_cast<double>(std::get<int64_t>(v));")
        lines.append("                        }")
        lines.append("                    }")
        lines.append("                }")
        lines.append("                // Coerce double→int64_t for INTEGER-typed keywords")
        lines.append("                // (e.g. 1.E5 parsed as real_value fallback in integer_or_repeat).")
        lines.append("                if (integerlist_keywords.count(resolved_name)) {")
        lines.append("                    for (auto& v : kw->param_values) {")
        lines.append("                        if (std::holds_alternative<double>(v)) {")
        lines.append("                            v = static_cast<int64_t>(std::get<double>(v));")
        lines.append("                        }")
        lines.append("                    }")
        lines.append("                }")
        lines.append("                // Check if resolved keyword already exists on the stack")
        lines.append("                // (e.g., analysis_driver truncates to analysis_drivers which may already exist)")
        lines.append("                bool found_existing = false;")
        lines.append("                for (size_t si = parent_index + 1; si < processing_stack.size(); ++si) {")
        lines.append("                    if (processing_stack[si].name == resolved_name) {")
        lines.append('                        DEBUG_OUT("[DEBUG]   Found existing \'" << resolved_name << "\' on stack at level " << si << "\\n");')
        lines.append("                        // Merge values into existing keyword")
        lines.append("                        if (processing_stack[si].node && !kw->param_values.empty()) {")
        lines.append("                            for (const auto& val : kw->param_values) {")
        lines.append("                                processing_stack[si].node->param_values.push_back(val);")
        lines.append("                            }")
        lines.append('                            DEBUG_OUT("[DEBUG]   Merged " << kw->param_values.size() << " values into existing keyword\\n");')
        lines.append("                        }")
        lines.append("                        // Pop stack to that level (to continue processing from there)")
        lines.append("                        while (processing_stack.size() > si + 1) {")
        lines.append("                            auto popped = processing_stack.back();")
        lines.append('                            DEBUG_OUT("[DEBUG]   Popping: " << popped.name << "\\n");')
        lines.append("                            processing_stack.pop_back();")
        lines.append("                            if (popped.node && !processing_stack.empty()) {")
        lines.append("                                auto& parent = processing_stack.back();")
        lines.append("                                if (parent.node) {")
        lines.append("                                    parent.node->children[popped.name].push_back(popped.node);")
        lines.append("                                } else {")
        lines.append("                                    block->keywords[popped.name].push_back(popped.node);")
        lines.append("                                }")
        lines.append("                            }")
        lines.append("                        }")
        lines.append("                        found_existing = true;")
        lines.append("                        break;")
        lines.append("                    }")
        lines.append("                }")
        lines.append("")
        lines.append("                if (!found_existing) {")
        lines.append("                    // Pop all elements above the parent")
        lines.append("                    while (processing_stack.size() > static_cast<size_t>(parent_index + 1)) {")
        lines.append("                        auto popped = processing_stack.back();")
        lines.append('                        DEBUG_OUT("[DEBUG]   Popping: " << popped.name << "\\n");')
        lines.append("                        processing_stack.pop_back();")
        lines.append("")
        lines.append("                        // Add popped node to its parent")
        lines.append("                        if (popped.node && !processing_stack.empty()) {")
        lines.append("                            auto& parent = processing_stack.back();")
        lines.append("                            if (parent.node) {")
        lines.append("                                parent.node->children[popped.name].push_back(popped.node);")
        lines.append("                            } else {")
        lines.append("                                block->keywords[popped.name].push_back(popped.node);")
        lines.append("                            }")
        lines.append("                        }")
        lines.append("                    }")
        lines.append("")
        lines.append("")
        lines.append("                    // Before pushing a new node, check whether a committed node")
        lines.append("                    // with the same name already exists at the parent level.")
        lines.append("                    // This handles e.g. analysis_driver arriving after")
        lines.append("                    // analysis_drivers was already committed as implicit container.")
        lines.append("                    {")
        lines.append("                        auto& tgt = processing_stack.back();")
        lines.append("                        std::shared_ptr<KeywordNode> reused;")
        lines.append("                        if (tgt.node) {")
        lines.append("                            auto ci = tgt.node->children.find(resolved_name);")
        lines.append("                            if (ci != tgt.node->children.end() && !ci->second.empty()) {")
        lines.append("                                reused = ci->second.back(); ci->second.pop_back();")
        lines.append("                                if (ci->second.empty()) tgt.node->children.erase(ci);")
        lines.append('                                DEBUG_OUT("[DEBUG]   Re-using committed child node " << resolved_name << "\\\\n");' )
        lines.append("                            }")
        lines.append("                        } else {")
        lines.append("                            auto ki = block->keywords.find(resolved_name);")
        lines.append("                            if (ki != block->keywords.end() && !ki->second.empty()) {")
        lines.append("                                reused = ki->second.back(); ki->second.pop_back();")
        lines.append("                                if (ki->second.empty()) block->keywords.erase(ki);")
        lines.append('                                DEBUG_OUT("[DEBUG]   Re-using committed block node " << resolved_name << "\\\\n");' )
        lines.append("                            }")
        lines.append("                        }")
        lines.append("                        if (reused) {")
        lines.append("                            for (const auto& v : kw->param_values) reused->param_values.push_back(v);")
        lines.append("                            for (const auto& s : kw->original_value_strings) reused->original_value_strings.push_back(s);")
        lines.append("                            processing_stack.push_back({resolved_name, tgt.dotted_path + \".\" + resolved_name, reused});")
        lines.append('                            DEBUG_OUT("[DEBUG]   Pushed re-used \'" << resolved_name << "\'\\n");')
        lines.append("                        } else {")
        lines.append("                            processing_stack.push_back({resolved_name, tgt.dotted_path + \".\" + resolved_name, kw});")
        lines.append('                            DEBUG_OUT("[DEBUG]   Pushed \'" << resolved_name << "\' onto stack (depth " << processing_stack.size() << ")\\n");')
        lines.append("                        }")
        lines.append("                    }")
        lines.append("                }")
        lines.append("")
        lines.append("                // Successfully matched - remove from list and restart from beginning")
        lines.append("                remaining_keywords.erase(remaining_keywords.begin() + i);")
        lines.append("                made_progress_this_round = true;")
        lines.append("                i = 0;  // Restart from beginning")
        lines.append('                DEBUG_OUT("[DEBUG]   Restarting keyword processing from beginning (" << remaining_keywords.size() << " remaining)\\n");')
        lines.append("                try_implicit_containers = false;  // Reset since we made progress")
        lines.append("            } else {")
        lines.append("                // No valid parent found - skip for now")
        lines.append('                DEBUG_OUT("[DEBUG]   No valid parent found for \'" << kw_name << "\' - skipping for now\\n");')
        lines.append("                ++i;")
        lines.append("            }")
        lines.append("        }")
        lines.append("")
        lines.append("        // If we completed a full pass with no progress")
        lines.append("        if (!made_progress_this_round) {")
        lines.append("            if (!try_implicit_containers) {")
        lines.append("                // Enable implicit container creation and try again")
        lines.append('                DEBUG_OUT("[DEBUG] No progress, enabling implicit container creation for " << remaining_keywords.size() << " remaining keywords\\n");')
        lines.append("                try_implicit_containers = true;")
        lines.append("            } else {")
        lines.append("                // Already tried implicit containers - remaining keywords are orphaned")
        lines.append('                DEBUG_OUT("[DEBUG] No progress made, " << remaining_keywords.size() << " keywords remain orphaned\\n");')
        lines.append("                for (auto& orphan : remaining_keywords) {")
        lines.append('                    DEBUG_OUT("[DEBUG]   Orphaned keyword: " << orphan->name << "\\n");')
        lines.append("                }")
        lines.append("                remaining_keywords.clear();")
        lines.append("            }")
        lines.append("        }")
        lines.append("    }")
        lines.append("")
        lines.append("    // Step 4: Pop remaining stack and finalize tree")
        lines.append('    DEBUG_OUT("[DEBUG] Phase 2: Finalizing remaining stack\\n");')
        lines.append("    while (processing_stack.size() > 1) {  // Don't pop the block itself")
        lines.append("        auto current = processing_stack.back();")
        lines.append('        DEBUG_OUT("[DEBUG]   Finalizing: " << current.name << "\\n");')
        lines.append("        processing_stack.pop_back();")
        lines.append("")
        lines.append("        if (current.node && !processing_stack.empty()) {")
        lines.append("            auto& parent = processing_stack.back();")
        lines.append("            if (parent.node) {")
        lines.append("                parent.node->children[current.name].push_back(current.node);")
        lines.append("            } else {")
        lines.append("                block->keywords[current.name].push_back(current.node);")
        lines.append("            }")
        lines.append("        }")
        lines.append("    }")
        lines.append("")
        lines.append('    DEBUG_OUT("[DEBUG] Phase 2: Complete\\n\\n");')
        lines.append("")
        lines.append("    // Clear parse order vector for next use")
        lines.append(f"    {self.block.name}_parse_order.clear();")
        lines.append("}")
        lines.append("")
        lines.append("")
        
        # Content complete action
        lines.append("// Trigger Phase 2 when content completes")
        lines.append("template<>")
        lines.append(f"struct action<{self.block.name}_content> {{")
        lines.append("    template<typename ActionInput>")
        lines.append("    static void apply([[maybe_unused]] const ActionInput& in, ParserState& state) {")
        lines.append(f'        DEBUG_OUT("[DEBUG] {self.block.name.capitalize()} content complete - starting Phase 2\\n");')
        lines.append(f"        reorganize_{self.block.name}_hierarchy(state.current_block);")
        lines.append("        state.current_block = nullptr;")
        lines.append("    }")
        lines.append("};")
        lines.append("")
        
        # Keyword actions
        lines.append("// " + "=" * 76)
        lines.append(f"// {self.block.name.upper()} KEYWORD ACTIONS")
        lines.append("// " + "=" * 76)
        lines.append("")
        
        # Generate actions - ONE per unique keyword TEXT (canonical + aliases)
        # Collect all unique texts from grammar generation
        all_keyword_texts = {}  # text -> (has_param, canonical_kw_info)
        
        # First, add all canonical keywords
        for sig, canonical_id in sorted(self.block.keywords_by_signature.items()):
            kw_info = self.block.keywords[canonical_id]
            text = kw_info.name
            
            if text not in all_keyword_texts:
                all_keyword_texts[text] = (kw_info.has_param, kw_info)
            else:
                # If this signature has params and we didn't record that yet, update
                existing_has_param, _ = all_keyword_texts[text]
                if kw_info.has_param and not existing_has_param:
                    all_keyword_texts[text] = (kw_info.has_param, kw_info)
        
        # Second, add all aliases from ALL keywords (not just canonical)
        # This ensures context-specific aliases from duplicate keywords are included
        for kw_id, kw_info in self.block.keywords.items():
            for alias in kw_info.aliases:
                if alias not in all_keyword_texts:
                    # Alias inherits param status from source keyword
                    all_keyword_texts[alias] = (kw_info.has_param, kw_info)
        
        # Generate actions in TWO PASSES to avoid forward reference issues:
        # Pass 1: Generate full actions for canonical keywords
        # Pass 2: Generate forwarding declarations for aliases
        
        # Pass 1: Canonical keywords only
        for text in sorted(all_keyword_texts.keys()):
            has_param, canonical_kw_info = all_keyword_texts[text]
            
            # Skip aliases in this pass
            if text != canonical_kw_info.name:
                continue
            
            struct_name = f"{self.block.name}_{text}_kw"  # Block-specific!
            
            # CANONICAL - Generate full action
            lines.append(f"// {text} (canonical)")
            lines.append("template<>")
            lines.append(f"struct action<{struct_name}> {{")
            lines.append("    template<typename ActionInput>")
            lines.append("    static void apply([[maybe_unused]] const ActionInput& in, ParserState& state) {")
            
            # Use canonical name directly
            if has_param:
                lines.append(f'        DEBUG_OUT("[DEBUG] Keyword (param): {text}\\n");')
                lines.append(f'        auto kw = create_keyword("{text}");')
                lines.append("        add_keyword_to_context(kw, state);")
                lines.append("        // Always track parse order in Phase 1")
                lines.append(f"        {self.block.name}_parse_order.push_back(kw);")
                lines.append(f'        DEBUG_OUT("[DEBUG]   Added to parse_order\\n");')
                lines.append("        state.current_keyword = kw;")
                lines.append("        state.in_param_value = true;")
            else:
                lines.append(f'        DEBUG_OUT("[DEBUG] Keyword (flag): {text}\\n");')
                lines.append(f'        auto kw = create_keyword("{text}", true);')
                lines.append("        add_keyword_to_context(kw, state);")
                lines.append("        // Always track parse order in Phase 1")
                lines.append(f"        {self.block.name}_parse_order.push_back(kw);")
                lines.append(f'        DEBUG_OUT("[DEBUG]   Added to parse_order\\n");')
                lines.append("        state.current_keyword = nullptr;")
            
            lines.append("    }")
            lines.append("};")
            lines.append("")
        
        # Pass 2: Aliases only (now that canonicals are defined)
        for text in sorted(all_keyword_texts.keys()):
            has_param, canonical_kw_info = all_keyword_texts[text]
            
            # Skip canonicals in this pass
            if text == canonical_kw_info.name:
                continue
            
            struct_name = f"{self.block.name}_{text}_kw"  # Block-specific!
            canonical_struct_name = f"{self.block.name}_{canonical_kw_info.name}_kw"  # Block-specific!
            
            # ALIAS - Forward to canonical action
            lines.append(f"// {text} (alias for {canonical_kw_info.name}) - forwarding to canonical")
            lines.append("template<>")
            lines.append(f"struct action<{struct_name}>")
            lines.append(f"    : action<{canonical_struct_name}> {{}};")
            lines.append("")
        
        lines.append("// " + "=" * 76)
        lines.append("// UNKNOWN/TRUNCATED KEYWORD ACTIONS")
        lines.append("// " + "=" * 76)
        lines.append("")
        lines.append("// Capture unknown keywords (possibly truncated)")
        lines.append("template<>")
        lines.append(f"struct action<{self.block.name}_unknown_keyword> {{")
        lines.append("    template<typename ActionInput>")
        lines.append("    static void apply(const ActionInput& in, ParserState& state) {")
        lines.append('        std::string kw_text = in.string();')
        lines.append('        DEBUG_OUT("[DEBUG] Unknown keyword (truncation?): " << kw_text << "\\n");')
        lines.append("        ")
        lines.append("        // Create keyword node marked as unresolved")
        lines.append('        auto kw = create_keyword(kw_text);')
        lines.append("        kw->unresolved = true;  // Mark for resolution in Phase 2")
        lines.append("        add_keyword_to_context(kw, state);")
        lines.append("        // Always track parse order in Phase 1")
        lines.append(f"        {self.block.name}_parse_order.push_back(kw);")
        lines.append('        DEBUG_OUT("[DEBUG]   Added to parse_order\\n");')
        lines.append("        ")
        lines.append("        // Set up to capture any values that follow this keyword")
        lines.append("        // We don't know yet if the resolved keyword takes values,")
        lines.append("        // but we need to capture them now and validate in Phase 2")
        lines.append("        state.current_keyword = kw;")
        lines.append("        state.in_param_value = true;  // Capture values if present")
        lines.append("    }")
        lines.append("};")
        lines.append("")
        
        lines.append("// " + "=" * 76)
        lines.append("// FORWARDING ACTIONS FOR TEMPLATE WRAPPERS")
        lines.append("// " + "=" * 76)
        lines.append("//")
        lines.append("// keyword_flag<X> needs forwarding because it's treated as distinct rule")
        lines.append("// keyword_with_param<X> does NOT need forwarding (action fires on base)")
        lines.append("")
        
        # Generate forwarding for keyword_flag only - ONE per unique text
        for text in sorted(all_keyword_texts.keys()):
            has_param, _ = all_keyword_texts[text]
            if not has_param:  # Only for flags
                struct_name = f"{self.block.name}_{text}_kw"  # Block-specific!
                lines.append("template<>")
                lines.append(f"struct action<keyword_flag<{struct_name}>>")
                lines.append(f"    : action<{struct_name}> {{}};")
                lines.append("")
        
        lines.append("} // namespace actions")
        lines.append("} // namespace dakota")
        lines.append("")
        lines.append(f"#endif // {guard}")
        lines.append("")
        
        output_file.write_text('\n'.join(lines), encoding='utf-8')
        if VERBOSE_MODE:
            print(f"  Generated {output_file.name}")
    
    def generate_parent_child_map(self):
        """Generate {block}_parent_child_map.hpp in block subdirectory"""
        block_dir = self.output_dir / self.block.name
        output_file = block_dir / f'{self.block.name}_parent_child_map.hpp'
        
        lines = []
        guard = f"DAKOTA_{self.block.name.upper()}_PARENT_CHILD_MAP_HPP"
        lines.append(f"#ifndef {guard}")
        lines.append(f"#define {guard}")
        lines.append("")
        lines.append("#include <map>")
        lines.append("#include <set>")
        lines.append("#include <string>")
        lines.append("")
        lines.append("namespace dakota {")
        lines.append("")
        lines.append(f"// {self.block.name.upper()} block parent-child map")
        lines.append(f"inline const std::map<std::string, std::set<std::string>> {self.block.name}_parent_child_map = {{")
        
        for parent_name in sorted(self.block.parent_child_map.keys()):
            children = self.block.parent_child_map[parent_name]
            if children:
                children_str = ', '.join(f'"{c}"' for c in sorted(children))
                lines.append(f'    {{"{parent_name}", {{{children_str}}}}},')
        
        lines.append("};")
        lines.append("")
        lines.append("} // namespace dakota")
        lines.append("")
        lines.append(f"#endif // {guard}")
        lines.append("")
        
        output_file.write_text('\n'.join(lines), encoding='utf-8')
        if VERBOSE_MODE:
            print(f"  Generated {output_file.name}")
    
    def generate_block_parser(self, block_dir: Path):
        """Generate block parser cpp file"""
        cpp_file = self.output_dir / 'src' / f'{self.block.name}_parser.cpp'
        
        lines = []
        lines.append(f'// {self.block.name}_parser.cpp - Isolated parser for {self.block.name.upper()} block')
        lines.append('')
        lines.append('#include "dakota_ast.hpp"')
        lines.append('#include "dakota_grammar_common.hpp"')
        lines.append(f'#include "{self.block.name}/dakota_grammar_{self.block.name}.hpp"')
        lines.append(f'#include "{self.block.name}/dakota_actions_{self.block.name}.hpp"')
        lines.append('#include <tao/pegtl.hpp>')
        lines.append('#include <tao/pegtl/contrib/analyze.hpp>')
        lines.append('#include <tao/pegtl/contrib/trace.hpp>')
        lines.append('#include <iostream>')
        lines.append('')
        lines.append('namespace dakota {')
        lines.append('namespace outer {')
        lines.append('')
        lines.append(f'// Analyze {self.block.name} grammar for cycles')
        lines.append(f'size_t analyze_{self.block.name}_grammar() {{')
        lines.append(f'    return tao::pegtl::analyze<{self.block.name}_content>();')
        lines.append('}')
        lines.append('')
        lines.append(f'bool parse_{self.block.name}_block(const std::string& content, Document& doc, bool enable_trace) {{')
        lines.append(f'    tao::pegtl::memory_input input(content, "{self.block.name}_block");')
        lines.append('')
        lines.append('    ParserState state;')
        lines.append('    doc.blocks.emplace_back();')
        lines.append('    state.current_block = &doc.blocks.back();')
        lines.append(f'    state.current_block->name = "{self.block.name}";')
        lines.append('    state.doc = doc;')
        lines.append('')
        lines.append('    try {')
        lines.append('        bool result;')
        lines.append('        if (enable_trace) {')
        lines.append(f'            std::cout << "\\n=== Tracing {self.block.name.upper()} block parser ===\\n";')
        lines.append(f'            result = tao::pegtl::standard_trace<{self.block.name}_content, actions::action>(input, state);')
        lines.append('        } else {')
        lines.append(f'            result = tao::pegtl::parse<{self.block.name}_content, actions::action>(input, state);')
        lines.append('        }')
        lines.append('        if (!result) {')
        lines.append(f'            std::cerr << "Error: Failed to parse {self.block.name} block content\\n";')
        lines.append('        }')
        lines.append('        return result;')
        lines.append('    } catch (const tao::pegtl::parse_error& e) {')
        lines.append(f'        std::cerr << "Parse error in {self.block.name} block: " << e.what() << "\\n";')
        lines.append('        return false;')
        lines.append('    } catch (const std::exception& e) {')
        lines.append(f'        std::cerr << "Error parsing {self.block.name} block: " << e.what() << "\\n";')
        lines.append('        return false;')
        lines.append('    }')
        lines.append('}')
        lines.append('')
        lines.append('} // namespace outer')
        lines.append('} // namespace dakota')
        lines.append('')
        
        cpp_file.parent.mkdir(parents=True, exist_ok=True)
        cpp_file.write_text('\n'.join(lines), encoding='utf-8')
        if VERBOSE_MODE:
            print(f"  Generated {cpp_file.name}")
    
    def generate_constraints(self, block_dir: Path):
        """Generate constraint metadata for validation"""
        header_file = block_dir / f"dakota_constraints_{self.block.name}.hpp"
        
        lines = []
        lines.append(f"// Generated constraint metadata for {self.block.name} block")
        lines.append(f"// DO NOT EDIT - Generated by generate_parser.py")
        lines.append("")
        lines.append(f"#ifndef DAKOTA_CONSTRAINTS_{self.block.name.upper()}_HPP")
        lines.append(f"#define DAKOTA_CONSTRAINTS_{self.block.name.upper()}_HPP")
        lines.append("")
        lines.append("#include <string>")
        lines.append("#include <vector>")
        lines.append("#include <map>")
        lines.append("#include <set>")
        lines.append("")
        lines.append("namespace dakota {")
        lines.append("")
        
        # Create struct for constraints (legal as template parameter)
        struct_name = f"{self.block.name}_constraints"
        lines.append(f"// Constraint data for {self.block.name} block")
        lines.append(f"struct {struct_name} {{")
        lines.append("")
        
        # Keyword occurrence constraints (minOccurs, maxOccurs)
        lines.append("    // Keyword occurrence constraints")
        lines.append("    struct OccurrenceConstraint {")
        lines.append("        int min_occurs;      // -1 means no constraint")
        lines.append("        int max_occurs;      // -1 means no constraint")
        lines.append("    };")
        lines.append("")
        
        # oneOf groups
        lines.append("    // oneOf groups: only one keyword from each set may appear")
        lines.append("    struct OneOfGroup {")
        lines.append("        std::string parent_path;")
        lines.append("        std::string label;")
        lines.append("        std::set<std::string> keywords;")
        lines.append("    };")
        lines.append("")
        
        # Static method to get occurrence constraints
        lines.append("    // Map: keyword_name -> occurrence constraints")
        lines.append("    static const std::map<std::string, OccurrenceConstraint>& get_occurrence_constraints() {")
        lines.append("        static const std::map<std::string, OccurrenceConstraint> data = {")
        
        for kw_id, kw_info in self.block.keywords.items():
            if kw_info.min_occurs > 0 or kw_info.max_occurs is not None:
                max_val = kw_info.max_occurs if kw_info.max_occurs is not None else -1
                lines.append(f'            {{"{kw_info.name}", {{{kw_info.min_occurs}, {max_val}}}}},')
        
        lines.append("        };")
        lines.append("        return data;")
        lines.append("    }")
        lines.append("")
        
        # Static method to get parameter constraints
        # Use path-based keys to handle ambiguous keywords correctly
        lines.append("    // Map: keyword_path -> constraint string")
        lines.append("    // Path format: 'block.parent1.parent2.keyword' for context-aware lookups")
        lines.append("    static const std::map<std::string, std::string>& get_param_constraints() {")
        lines.append("        static const std::map<std::string, std::string> data = {")
        
        # Build path for each keyword using parent chain
        constraint_entries = []  # Collect entries for debugging
        for kw_id, kw_info in self.block.keywords.items():
            if kw_info.param_constraint:
                # Build the path(s) for this specific keyword instance
                keyword_paths = self._build_keyword_path(kw_info, kw_id)
                
                # Add entries for all paths where this keyword appears
                constraint_str = kw_info.param_constraint.replace('"', '\\"')
                for path in keyword_paths:
                    constraint_entries.append((path, constraint_str, kw_id))
        
        # Sort and print for debugging
        if VERBOSE_MODE:
            print(f"\n  Param constraint entries for {self.block.name}:")
            for path, constraint, kw_id in sorted(constraint_entries):
                if "population_size" in path or "coliny_ea" in path:
                    print(f"    {path} -> '{constraint}' (from {kw_id})")
        
        # Generate the C++ map
        for path, constraint, kw_id in constraint_entries:
            lines.append(f'            {{"{path}", "{constraint}"}},')
        
        lines.append("        };")
        lines.append("        return data;")
        lines.append("    }")
        lines.append("")
        
        # Static method to get oneOf groups
        lines.append("    // Vector of oneOf groups")
        lines.append("    static const std::vector<OneOfGroup>& get_oneof_groups() {")
        lines.append("        static const std::vector<OneOfGroup> data = {")
        
        for (parent_path, label), keywords in sorted(self.block.oneof_groups.items()):
            lines.append("            {")
            lines.append(f'                "{parent_path}",')
            lines.append(f'                "{label}",')
            # Build keyword list manually to avoid f-string nesting issues
            kw_list = ', '.join(f'"{kw}"' for kw in sorted(keywords))
            lines.append(f'                {{{kw_list}}}')
            lines.append("            },")
        
        lines.append("        };")
        lines.append("        return data;")
        lines.append("    }")
        
        lines.append("};")
        lines.append("")
        lines.append("} // namespace dakota")
        lines.append("")
        lines.append(f"#endif // DAKOTA_CONSTRAINTS_{self.block.name.upper()}_HPP")
        lines.append("")
        
        header_file.write_text('\n'.join(lines), encoding='utf-8')
        if VERBOSE_MODE:
            print(f"  Generated {header_file.name}")
    
    def generate_semantic_metadata(self, block_dir: Path):
        """Generate semantic resolution metadata (alias mappings by context)"""
        header_file = block_dir / f"dakota_semantic_{self.block.name}.hpp"
        
        lines = []
        lines.append(f"// Generated semantic resolution metadata for {self.block.name} block")
        lines.append(f"// DO NOT EDIT - Generated by generate_parser.py")
        lines.append("")
        lines.append(f"#ifndef DAKOTA_SEMANTIC_{self.block.name.upper()}_HPP")
        lines.append(f"#define DAKOTA_SEMANTIC_{self.block.name.upper()}_HPP")
        lines.append("")
        lines.append("#include <string>")
        lines.append("#include <map>")
        lines.append("#include <utility>")
        lines.append("")
        lines.append("namespace dakota {")
        lines.append("")
        
        # Create struct for semantic metadata
        struct_name = f"{self.block.name}_semantic"
        lines.append(f"// Semantic resolution data for {self.block.name} block")
        lines.append(f"struct {struct_name} {{")
        lines.append("")
        
        # Alias resolution map: (parent_path, alias_name) -> canonical_name
        lines.append("    // Alias resolution map")
        lines.append("    // Key: (parent_path, alias_name)")
        lines.append("    // Value: canonical_name")
        lines.append("    static const std::map<std::pair<std::string, std::string>, std::string>&")
        lines.append("    get_alias_map() {")
        lines.append("        static const std::map<std::pair<std::string, std::string>, std::string> data = {")
        
        # Sort by parent path for readability
        for (parent_path, alias_name), canonical_name in sorted(self.block.alias_map.items()):
            lines.append(f'            {{{{"{parent_path}", "{alias_name}"}}, "{canonical_name}"}},')
        
        lines.append("        };")
        lines.append("        return data;")
        lines.append("    }")
        lines.append("")
        
        lines.append("};")
        lines.append("")
        lines.append("} // namespace dakota")
        lines.append("")
        lines.append(f"#endif // DAKOTA_SEMANTIC_{self.block.name.upper()}_HPP")
        lines.append("")
        
        header_file.write_text('\n'.join(lines), encoding='utf-8')
        if VERBOSE_MODE:
            print(f"  Generated {header_file.name}")


def generate_error_messages(blocks: Dict[str, BlockInfo], output_dir: Path):
    """
    Generate dakota_error_messages.hpp with custom error messages for all blocks.
    """
    
    error_file = output_dir / "dakota_error_messages.hpp"
    
    # Build content as string
    content = """#ifndef DAKOTA_ERROR_MESSAGES_HPP
#define DAKOTA_ERROR_MESSAGES_HPP

#include "dakota_grammar_common.hpp"
#include "dakota_outer_grammar.hpp"
#include <tao/pegtl.hpp>

// Auto-generated custom error messages for Dakota parser

namespace dakota {
namespace errors {

template<typename>
inline constexpr const char* error_message = nullptr;

// Top-level errors
template<>
inline constexpr auto error_message<outer::grammar> = 
    "invalid Dakota input file structure";

template<>
inline constexpr auto error_message<tao::pegtl::eof> = 
    "unexpected content - check for typos in block keywords or remove invalid text";

template<>
inline constexpr auto error_message<outer::block_name> = 
    "expected block keyword (environment, method, model, variables, interface, or responses)";

// Common errors
template<>
inline constexpr auto error_message<integer> = 
    "expected integer value";

template<>
inline constexpr auto error_message<integer_value> = 
    "expected integer value";

template<>
inline constexpr auto error_message<real_value> = 
    "expected real number";

template<>
inline constexpr auto error_message<string_value> = 
    "expected quoted string value";

template<>
inline constexpr auto error_message<integer_list> = 
    "expected list of integers";

template<>
inline constexpr auto error_message<real_list> = 
    "expected list of real numbers";

template<>
inline constexpr auto error_message<string_list> = 
    "expected list of quoted strings";

// Block keywords
template<>
inline constexpr auto error_message<outer::environment_kw> = 
    "expected 'environment' keyword";

template<>
inline constexpr auto error_message<outer::method_kw> = 
    "expected 'method' keyword";

template<>
inline constexpr auto error_message<outer::model_kw> = 
    "expected 'model' keyword";

template<>
inline constexpr auto error_message<outer::variables_kw> = 
    "expected 'variables' keyword";

template<>
inline constexpr auto error_message<outer::interface_kw> = 
    "expected 'interface' keyword";

template<>
inline constexpr auto error_message<outer::responses_kw> = 
    "expected 'responses' keyword";

"""
    
    content += "} // namespace errors\n"
    content += "} // namespace dakota\n\n"
    content += "#endif // DAKOTA_ERROR_MESSAGES_HPP\n"
    
    # Only write if content changed
    if write_file_if_changed(error_file, content):
        if VERBOSE_MODE:
            print(f"  Updated {error_file}")
    else:
        if VERBOSE_MODE:
            print(f"  Unchanged {error_file}")


def _generate_keyword_error_message(kw_info: KeywordInfo) -> str:
    """Generate helpful error message based on parameter type."""
    param_type = kw_info.param_type
    kw_name = kw_info.name
    constraint = kw_info.param_constraint
    
    if param_type == 'INTEGER':
        if constraint and '>=' in constraint:
            return f"expected positive integer for {kw_name}"
        return f"expected integer for {kw_name}"
    elif param_type == 'REAL':
        if constraint and '>=' in constraint:
            return f"expected positive real number for {kw_name}"
        return f"expected real number for {kw_name}"
    elif param_type in ['INTEGERLIST', 'INTEGER_LIST']:
        if constraint and 'LEN' in constraint:
            parts = constraint.split()
            if len(parts) > 1:
                return f"expected list of integers for {kw_name} (must match {parts[1]} count)"
        return f"expected list of integers for {kw_name}"
    elif param_type in ['REALLIST', 'REAL_LIST']:
        if constraint and 'LEN' in constraint:
            parts = constraint.split()
            if len(parts) > 1:
                return f"expected list of real numbers for {kw_name} (must match {parts[1]} count)"
        return f"expected list of real numbers for {kw_name}"
    elif param_type in ['STRING', 'FILENAME', 'INPUT_FILE', 'OUTPUT_FILE']:
        return f"expected quoted string for {kw_name}"
    elif param_type in ['STRINGLIST', 'STRING_LIST']:
        return f"expected list of quoted strings for {kw_name}"
    else:
        return f"expected value for {kw_name}"


def generate_error_message_tests(blocks: Dict[str, BlockInfo], block_name: str, test_dir: Path) -> int:
    """Generate error message tests for a block."""
    block_info = blocks[block_name]
    test_count = 0
    
    error_test_dir = test_dir / block_name / "error_messages"
    error_test_dir.mkdir(parents=True, exist_ok=True)
    
    for kw_id, kw_info in block_info.keywords.items():
        if not kw_info.has_param or not kw_info.param_type:
            continue
        
        # Wrong type test - use keyword ID for unique filename
        test_file = error_test_dir / f"error_{kw_info.id}_wrong_type.in"
        invalid_value = _generate_invalid_value(kw_info.param_type)
        
        with open(test_file, 'w', encoding='utf-8') as f:
            f.write(f"# Error test: {kw_info.name} (id={kw_info.id}) with wrong type\n")
            f.write(f"{block_name}\n  {kw_info.name} = {invalid_value}\n")
        test_count += 1
    
    return test_count


def _generate_invalid_value(param_type: str) -> str:
    """Generate invalid value to trigger error message."""
    if param_type == 'INTEGER':
        return '"not_an_integer"'  # String where integer expected
    elif param_type == 'REAL':
        return '"not_a_number"'  # String where real expected
    elif param_type in ['STRING', 'FILENAME', 'INPUT_FILE', 'OUTPUT_FILE']:
        return '12345'  # Number where string expected (unquoted)
    elif param_type in ['STRINGLIST', 'STRING_LIST']:
        return '12345 67890'  # Numbers where string list expected
    else:
        return 'invalid_value'


def write_file_if_changed(filepath: Path, content: str):
    """
    Write file only if content has changed.
    This prevents unnecessary recompilation when content is identical.
    """
    # Check if file exists and has same content
    if filepath.exists():
        existing_content = filepath.read_text(encoding='utf-8')
        if existing_content == content:
            # Content is identical, don't write
            return False
    
    # Write new/changed content
    filepath.write_text(content, encoding='utf-8')
    return True


def generate_stub_parser(block_name: str, output_dir: Path):
    """Generate stub parser"""  
    if VERBOSE_MODE:
        print(f"  Generating stub for {block_name}...")
    block_dir = output_dir / block_name
    block_dir.mkdir(parents=True, exist_ok=True)
    
    # Create minimal stub grammar file
    (block_dir / f"dakota_grammar_{block_name}.hpp").write_text(
        f"#ifndef DAKOTA_GRAMMAR_{block_name.upper()}_HPP\n"
        f"#define DAKOTA_GRAMMAR_{block_name.upper()}_HPP\n"
        f"#include <tao/pegtl.hpp>\n"
        f"namespace dakota {{ struct {block_name}_content : tao::pegtl::star<tao::pegtl::any> {{ }}; }}\n"
        f"#endif\n",
        encoding='utf-8')
    
    # Create minimal stub actions file
    (block_dir / f"dakota_actions_{block_name}.hpp").write_text(
        f"#ifndef DAKOTA_ACTIONS_{block_name.upper()}_HPP\n"
        f"#define DAKOTA_ACTIONS_{block_name.upper()}_HPP\n"
        f"namespace dakota {{ namespace actions {{ template<typename Rule> struct action {{ }}; }} }}\n"
        f"#endif\n",
        encoding='utf-8')
    
    # Create minimal stub parent_child_map file
    (block_dir / f"{block_name}_parent_child_map.hpp").write_text(
        f"#ifndef DAKOTA_{block_name.upper()}_PARENT_CHILD_MAP_HPP\n"
        f"#define DAKOTA_{block_name.upper()}_PARENT_CHILD_MAP_HPP\n"
        "#include <map>\n"
        "#include <set>\n"
        "#include <string>\n"
        f"namespace dakota {{ inline const std::map<std::string, std::set<std::string>> {block_name}_parent_child_map = {{}}; }}\n"
        f"#endif\n",
        encoding='utf-8')
    
    # Create minimal stub constraints file
    (block_dir / f"dakota_constraints_{block_name}.hpp").write_text(
        f"// Stub constraint metadata for {block_name} block\n"
        f"#ifndef DAKOTA_CONSTRAINTS_{block_name.upper()}_HPP\n"
        f"#define DAKOTA_CONSTRAINTS_{block_name.upper()}_HPP\n"
        "#include <string>\n"
        "#include <vector>\n"
        "#include <map>\n"
        "#include <set>\n"
        "namespace dakota {\n"
        f"struct {block_name}_constraints {{\n"
        "\n"
        "    // Keyword occurrence constraints\n"
        "    struct OccurrenceConstraint {\n"
        "        int min_occurs;  // -1 means no constraint\n"
        "        int max_occurs;  // -1 means no constraint\n"
        "    };\n"
        "\n"
        "    // OneOf group definition\n"
        "    struct OneOfGroup {\n"
        "        std::string parent_path;\n"
        "        std::string label;\n"
        "        std::set<std::string> keywords;\n"
        "    };\n"
        "\n"
        "    // Map: keyword_name -> occurrence constraints\n"
        "    static const std::map<std::string, OccurrenceConstraint>& get_occurrence_constraints() {\n"
        "        static const std::map<std::string, OccurrenceConstraint> data = {};\n"
        "        return data;\n"
        "    }\n"
        "\n"
        "    // Map: keyword_path -> constraint string\n"
        "    static const std::map<std::string, std::string>& get_param_constraints() {\n"
        "        static const std::map<std::string, std::string> data = {};\n"
        "        return data;\n"
        "    }\n"
        "\n"
        "    // Vector of oneOf groups\n"
        "    static const std::vector<OneOfGroup>& get_oneof_groups() {\n"
        "        static const std::vector<OneOfGroup> data = {};\n"
        "        return data;\n"
        "    }\n"
        "};\n"
        "}\n"
        f"#endif // DAKOTA_CONSTRAINTS_{block_name.upper()}_HPP\n",
        encoding='utf-8')
    
    # Create minimal stub semantic file
    (block_dir / f"dakota_semantic_{block_name}.hpp").write_text(
        f"// Stub semantic metadata for {block_name} block\n"
        f"#ifndef DAKOTA_SEMANTIC_{block_name.upper()}_HPP\n"
        f"#define DAKOTA_SEMANTIC_{block_name.upper()}_HPP\n"
        "#include <string>\n"
        "#include <vector>\n"
        "#include <map>\n"
        "#include <utility>\n"
        "namespace dakota {\n"
        f"struct {block_name}_semantic {{\n"
        "\n"
        "    // Map: keyword_name -> vector of full paths where it appears\n"
        "    static const std::map<std::string, std::vector<std::string>>& get_keyword_paths() {\n"
        "        static const std::map<std::string, std::vector<std::string>> data = {};\n"
        "        return data;\n"
        "    }\n"
        "\n"
        "    // Map: any_name -> canonical_name (for aliases and validation)\n"
        "    static const std::map<std::string, std::string>& get_canonical_names() {\n"
        "        static const std::map<std::string, std::string> data = {};\n"
        "        return data;\n"
        "    }\n"
        "\n"
        "    // Alias resolution map: (parent_path, alias_name) -> canonical_name\n"
        "    // Key: (parent_path, alias_name)\n"
        "    // Value: canonical_name\n"
        "    static const std::map<std::pair<std::string, std::string>, std::string>&\n"
        "    get_alias_map() {\n"
        "        static const std::map<std::pair<std::string, std::string>, std::string> data = {};\n"
        "        return data;\n"
        "    }\n"
        "\n"
        "};\n"
        "}\n"
        f"#endif // DAKOTA_SEMANTIC_{block_name.upper()}_HPP\n",
        encoding='utf-8')
    
    # Create stub parser implementation
    cpp_file = output_dir / "src" / f"{block_name}_parser.cpp"
    cpp_file.parent.mkdir(parents=True, exist_ok=True)
    cpp_file.write_text(
        f"// Stub parser for {block_name} block\n"
        f"#include \"dakota_ast.hpp\"\n"
        f"#include <cstddef>\n"
        f"\n"
        f"namespace dakota {{\n"
        f"namespace outer {{\n"
        f"\n"
        f"// Stub analyze function (returns 0 issues)\n"
        f"size_t analyze_{block_name}_grammar() {{\n"
        f"    return 0;\n"
        f"}}\n"
        f"\n"
        f"// Stub parse function (always succeeds)\n"
        f"bool parse_{block_name}_block(const std::string&, Document&, bool) {{\n"
        f"    return true;\n"
        f"}}\n"
        f"\n"
        f"}} // namespace outer\n"
        f"}} // namespace dakota\n",
        encoding='utf-8')

def generate_error_messages_stub(block_name: str, output_dir: Path):
    """
    Generate minimal dakota_error_messages.hpp for single-block mode.
    
    This creates a stub file with basic error messages so compilation works
    even when only one block is fully generated.
    """
    error_file = output_dir / "dakota_error_messages.hpp"
    
    content = f"""#ifndef DAKOTA_ERROR_MESSAGES_HPP
#define DAKOTA_ERROR_MESSAGES_HPP

#include "dakota_grammar_common.hpp"
#include "dakota_outer_grammar.hpp"
#include <tao/pegtl.hpp>

// Auto-generated stub error messages for single-block mode ({block_name})

namespace dakota {{
namespace errors {{

template<typename>
inline constexpr const char* error_message = nullptr;

// Top-level errors
template<>
inline constexpr auto error_message<outer::grammar> = 
    "invalid Dakota input file structure";

template<>
inline constexpr auto error_message<tao::pegtl::eof> = 
    "unexpected content - check for typos in block keywords or remove invalid text";

template<>
inline constexpr auto error_message<outer::block_name> = 
    "expected block keyword (environment, method, model, variables, interface, or responses)";

// Common errors
template<>
inline constexpr auto error_message<integer> = 
    "expected integer value";

template<>
inline constexpr auto error_message<integer_value> = 
    "expected integer value";

template<>
inline constexpr auto error_message<real_value> = 
    "expected real number";

template<>
inline constexpr auto error_message<string_value> = 
    "expected quoted string value";

template<>
inline constexpr auto error_message<integer_list> = 
    "expected list of integers";

template<>
inline constexpr auto error_message<real_list> = 
    "expected list of real numbers";

template<>
inline constexpr auto error_message<string_list> = 
    "expected list of quoted strings";

// Block keywords (all blocks for outer grammar)
template<>
inline constexpr auto error_message<outer::environment_kw> = 
    "expected 'environment' keyword";

template<>
inline constexpr auto error_message<outer::method_kw> = 
    "expected 'method' keyword";

template<>
inline constexpr auto error_message<outer::model_kw> = 
    "expected 'model' keyword";

template<>
inline constexpr auto error_message<outer::variables_kw> = 
    "expected 'variables' keyword";

template<>
inline constexpr auto error_message<outer::interface_kw> = 
    "expected 'interface' keyword";

template<>
inline constexpr auto error_message<outer::responses_kw> = 
    "expected 'responses' keyword";

}} // namespace errors
}} // namespace dakota

#endif // DAKOTA_ERROR_MESSAGES_HPP
"""
    
    error_file.write_text(content, encoding='utf-8')


def generate_error_message_tests_stub(all_blocks, block_name, test_output_dir):
    """Stub for error message test generation."""
    # TODO: Implement when needed
    return 0
