#!/usr/bin/env python3
"""
Generate constraint validation tests from Dakota XML specification

Generates minimal test cases for:
- oneOf constraints (mutually exclusive)
- minOccurs/maxOccurs (occurrence validation)
- Parameter constraints (numeric bounds)

Key feature: Generates VALID Dakota input files by building proper
keyword hierarchies from the parent-child map.
"""

import re
from pathlib import Path
from typing import Dict, Set, List, Tuple, Optional

class ConstraintTest:
    """Represents a single constraint test case"""
    def __init__(self, name, block, content, should_pass, description):
        self.name = name
        self.block = block
        self.content = content
        self.should_pass = should_pass
        self.description = description

def sanitize_test_name(name: str) -> str:
    """
    Sanitize a string for use in test names.
    
    CMake/CTest test names should not contain:
    - Spaces
    - Parentheses
    - Special characters that could be interpreted by CMake
    
    This function converts problematic characters to safe alternatives.
    """
    # Replace spaces with underscores
    name = name.replace(' ', '_')
    
    # Remove or replace parentheses
    name = name.replace('(', '')
    name = name.replace(')', '')
    
    # Replace forward slashes with underscores
    name = name.replace('/', '_')
    
    # Replace any other problematic characters
    # Keep only alphanumeric, underscores, and hyphens
    name = re.sub(r'[^a-zA-Z0-9_\-]', '_', name)
    
    # Collapse multiple underscores into one
    name = re.sub(r'_+', '_', name)
    
    # Remove leading/trailing underscores
    name = name.strip('_')
    
    return name

class ConstraintTestGenerator:
    """Generate constraint tests from parsed block information"""
    
    def __init__(self, output_dir: Path):
        self.output_dir = output_dir
        self.positive_dir = output_dir / "positive"
        self.negative_dir = output_dir / "negative"
        self.positive_dir.mkdir(parents=True, exist_ok=True)
        self.negative_dir.mkdir(parents=True, exist_ok=True)
        self.tests_generated = 0
        self.block_info = None
        self.block_name = None
        
        # Cache for keyword paths
        self._keyword_path_cache: Dict[str, str] = {}
        
        # Cache for keyword info by name
        self._keyword_info_cache: Dict[str, 'KeywordInfo'] = {}
    
    def _get_placeholder_value(self, param_type: Optional[str], kw_info=None) -> str:
        """
        Generate a placeholder value based on parameter type and constraints.
        
        Args:
            param_type: The parameter type from XML (STRING, INTEGER, REAL, etc.)
            kw_info: Optional KeywordInfo with param_constraint to respect
        
        Returns:
            Appropriate placeholder string that satisfies constraints
        """
        if param_type is None:
            return "'placeholder'"
        
        param_type_upper = param_type.upper()
        
        # Check if we have constraints to respect
        constraint_value = None
        if kw_info and hasattr(kw_info, 'param_constraint') and kw_info.param_constraint:
            constraint = kw_info.param_constraint
            # Parse constraints like ">= 3" or "> 0" or "<= 10"
            try:
                if ">=" in constraint:
                    bound = float(constraint.split(">=")[1].strip())
                    # Use the minimum valid value (at the bound)
                    constraint_value = int(bound) if 'INTEGER' in param_type_upper else bound
                elif ">" in constraint and "<" not in constraint:
                    bound = float(constraint.split(">")[1].strip())
                    # Use value just above bound
                    if 'INTEGER' in param_type_upper:
                        constraint_value = int(bound) + 1
                    else:
                        constraint_value = bound + 0.1
                elif "<=" in constraint:
                    bound = float(constraint.split("<=")[1].strip())
                    # Use the maximum valid value (at the bound)
                    constraint_value = int(bound) if 'INTEGER' in param_type_upper else bound
                elif "<" in constraint and ">" not in constraint:
                    bound = float(constraint.split("<")[1].strip())
                    # Use value just below bound
                    if 'INTEGER' in param_type_upper:
                        constraint_value = int(bound) - 1
                    else:
                        constraint_value = bound - 0.1
            except (ValueError, IndexError):
                # If parsing fails, fall back to default values
                pass
        
        if 'STRING' in param_type_upper:
            if 'LIST' in param_type_upper:
                return "'value1' 'value2'"
            return "'placeholder'"
        elif 'INTEGER' in param_type_upper:
            if 'LIST' in param_type_upper:
                # For integer lists with constraints, replicate the valid value
                if constraint_value is not None:
                    val = int(constraint_value)
                    return f"{val} {val+1} {val+2}"
                return "1 2 3"
            # Use constraint value if available, otherwise default to 1
            return str(int(constraint_value)) if constraint_value is not None else "1"
        elif 'REAL' in param_type_upper:
            if 'LIST' in param_type_upper:
                # For real lists with constraints, replicate the valid value
                if constraint_value is not None:
                    return f"{constraint_value} {constraint_value + 1.0} {constraint_value + 2.0}"
                return "1.0 2.0 3.0"
            # Use constraint value if available, otherwise default to 1.0
            return str(float(constraint_value)) if constraint_value is not None else "1.0"
        elif 'FILENAME' in param_type_upper:
            return "'input.dat'"
        else:
            # Default to quoted string for unknown types
            return "'placeholder'"
    
    def _get_keyword_info_by_name(self, keyword_name: str, parent_path: Optional[str] = None) -> Optional[object]:
        """
        Get KeywordInfo for a keyword by its name, optionally in a specific context.
        
        Args:
            keyword_name: The keyword to look up
            parent_path: Optional parent path for context-aware lookup
        
        Returns the matching keyword info, or None if not found.
        Prefers keywords that match the context (are valid children of parent).
        """
        # Check cache first (only use cache if no parent_path specified)
        if parent_path is None and keyword_name in self._keyword_info_cache:
            return self._keyword_info_cache[keyword_name]
        
        if not hasattr(self.block_info, 'keywords'):
            return None
        
        # If we have a parent path, try to find the keyword in context
        if parent_path and hasattr(self.block_info, 'parent_child_map'):
            parent_child_map = self.block_info.parent_child_map
            
            # Check if this keyword is a valid child of the parent
            if parent_path in parent_child_map:
                children = parent_child_map[parent_path]
                if keyword_name in children:
                    # Find the keyword info that matches this context
                    # We need to find one where the parent matches
                    for kw_id, kw_info in self.block_info.keywords.items():
                        if kw_info.name == keyword_name:
                            # Check if this keyword's parent matches our context
                            if hasattr(kw_info, 'parent'):
                                # Extract the immediate parent from the path
                                parent_name = parent_path.split('.')[-1] if '.' in parent_path else parent_path
                                if kw_info.parent == parent_name:
                                    return kw_info
        
        # Fall back to finding first match by name
        for kw_id, kw_info in self.block_info.keywords.items():
            if kw_info.name == keyword_name:
                if parent_path is None:
                    self._keyword_info_cache[keyword_name] = kw_info
                return kw_info
            # Also check aliases
            if hasattr(kw_info, 'aliases') and keyword_name in kw_info.aliases:
                if parent_path is None:
                    self._keyword_info_cache[keyword_name] = kw_info
                return kw_info
        
        return None
    
    def generate_tests_for_block(self, block_info, block_name: str):
        """Generate constraint tests for a specific block"""
        self.block_info = block_info
        self.block_name = block_name
        self._keyword_path_cache = {}
        
        tests = []
        
        # Generate oneOf constraint tests
        tests.extend(self._generate_oneof_tests(block_info, block_name))
        
        # Generate minOccurs/maxOccurs tests
        tests.extend(self._generate_occurs_tests(block_info, block_name))
        
        # Generate parameter constraint tests
        tests.extend(self._generate_param_tests(block_info, block_name))
        
        # Write tests to files
        for test in tests:
            self._write_test(test)
        
        return len(tests)
    
    def _find_keyword_path_by_id(self, kw_id: str, kw_info) -> Optional[str]:
        """
        Find the full dotted path to a keyword by its ID and parent chain.
        
        This is more accurate than searching by name when there are multiple
        keywords with the same name but different IDs (like population_size and population_size1).
        
        Returns the parent path or None if keyword is a direct child of the block.
        """
        if not hasattr(self.block_info, 'parent_child_map'):
            return None
        
        parent_child_map = self.block_info.parent_child_map
        
        # Get the keyword's immediate parent
        if not hasattr(kw_info, 'parent') or not kw_info.parent:
            return None
        
        immediate_parent = kw_info.parent
        
        # Find ALL paths in parent_child_map where this keyword appears as a child
        candidate_paths = []
        for path, children in parent_child_map.items():
            if kw_info.name in children:
                candidate_paths.append(path)
        
        if not candidate_paths:
            return None
        
        # If only one candidate, use it
        if len(candidate_paths) == 1:
            return candidate_paths[0]
        
        # Multiple candidates - find the one whose path ends with the immediate parent
        # and whose parent chain matches the keyword's parent chain
        for path in candidate_paths:
            path_parts = path.split('.')
            
            # Check if the last part of the path matches the immediate parent
            if path_parts[-1] == immediate_parent:
                # Now validate the rest of the parent chain
                # Walk up from kw_info.parent and check each level matches the path
                is_valid = True
                current_parent_name = immediate_parent
                path_index = len(path_parts) - 1
                
                # Build a quick lookup of parent relationships
                visited = set()
                while current_parent_name and path_index >= 0:
                    if current_parent_name in visited:
                        break
                    visited.add(current_parent_name)
                    
                    # Check if current position in path matches
                    if path_parts[path_index] != current_parent_name:
                        is_valid = False
                        break
                    
                    # Find the parent keyword's parent
                    parent_kw_info = None
                    for pid, pinfo in self.block_info.keywords.items():
                        if pinfo.name == current_parent_name:
                            # Try to find one whose own parent matches the next level up in path
                            if path_index > 0 and hasattr(pinfo, 'parent') and pinfo.parent == path_parts[path_index - 1]:
                                parent_kw_info = pinfo
                                break
                            elif parent_kw_info is None:
                                parent_kw_info = pinfo
                    
                    if parent_kw_info and hasattr(parent_kw_info, 'parent'):
                        current_parent_name = parent_kw_info.parent
                    else:
                        current_parent_name = None
                    
                    path_index -= 1
                
                if is_valid:
                    return path
        
        # Fallback: return the first candidate (may not be correct)
        return candidate_paths[0]
    
    def _find_keyword_path(self, keyword_name: str) -> Optional[str]:
        """
        Find the full dotted path to a keyword by searching the parent-child map.
        
        Returns the parent path (e.g., "environment.results_output.hdf5" for "interface_selection")
        or None if keyword is a direct child of the block.
        """
        if keyword_name in self._keyword_path_cache:
            return self._keyword_path_cache[keyword_name]
        
        if not hasattr(self.block_info, 'parent_child_map'):
            return None
        
        parent_child_map = self.block_info.parent_child_map
        
        # Search for the keyword in the parent-child map
        for parent_path, children in parent_child_map.items():
            if keyword_name in children:
                self._keyword_path_cache[keyword_name] = parent_path
                return parent_path
        
        # Not found - might be the block itself or not in map
        return None
    
    def _build_hierarchy_content(self, block_name: str, keyword_name: str, 
                                  keyword_value: Optional[str] = None,
                                  additional_keywords: List[Tuple[str, Optional[str]]] = None) -> str:
        """
        Build a valid Dakota input with proper hierarchy for the given keyword.
        
        Args:
            block_name: Top-level block name (e.g., "environment")
            keyword_name: Target keyword to include
            keyword_value: Optional value for the keyword parameter
            additional_keywords: List of (keyword_name, value) tuples to add at same level
        
        Returns:
            Valid Dakota input string with proper nesting
        """
        # Find the path to this keyword
        parent_path = self._find_keyword_path(keyword_name)
        
        if parent_path is None or parent_path == block_name:
            # Direct child of block
            hierarchy = []
        else:
            # Parse the path to get hierarchy
            # parent_path is like "environment.results_output.hdf5"
            path_parts = parent_path.split('.')
            # Remove block name from start if present
            if path_parts[0] == block_name:
                hierarchy = path_parts[1:]
            else:
                hierarchy = path_parts
        
        # Build the output
        lines = [block_name]
        indent = "  "
        
        # Add each level of hierarchy
        for level, parent_kw in enumerate(hierarchy):
            lines.append(f"{indent * (level + 1)}{parent_kw}")
        
        # Add the target keyword at the correct indentation
        target_indent = indent * (len(hierarchy) + 1)
        if keyword_value is not None:
            lines.append(f"{target_indent}{keyword_name} = {keyword_value}")
        else:
            lines.append(f"{target_indent}{keyword_name}")
        
        # Add any additional keywords at the same level
        if additional_keywords:
            for add_kw, add_val in additional_keywords:
                if add_val is not None:
                    lines.append(f"{target_indent}{add_kw} = {add_val}")
                else:
                    lines.append(f"{target_indent}{add_kw}")
        
        return "\n".join(lines)
    
    def _generate_oneof_tests(self, block_info, block_name: str) -> List[ConstraintTest]:
        """Generate tests for oneOf (mutually exclusive) constraints"""
        tests = []
        
        # Get oneOf groups from block_info
        if not hasattr(block_info, 'oneof_groups'):
            return tests
        
        for group_key, group_members in block_info.oneof_groups.items():
            parent_path, group_label = group_key
            
            if len(group_members) < 2:
                continue
            
            members_list = sorted(list(group_members))
            
            # Sanitize group label for test name
            safe_label = sanitize_test_name(group_label)
            
            # Include parent path to make test names unique
            # For example: "sampling" or "sampling.refinement_samples"
            if parent_path:
                safe_parent = sanitize_test_name(parent_path.replace('.', '_'))
                test_prefix = f"{block_name}_oneof_{safe_parent}_{safe_label}"
            else:
                test_prefix = f"{block_name}_oneof_{safe_label}"
            
            # Positive test: Only one member from group
            test_name = f"{test_prefix}_valid"
            content = self._build_hierarchy_for_oneof(block_name, parent_path, [members_list[0]])
            tests.append(ConstraintTest(
                name=test_name,
                block=block_name,
                content=content,
                should_pass=True,
                description=f"oneOf group '{group_label}' in {parent_path or 'root'}: valid (only {members_list[0]})"
            ))
            
            # Negative test: Two members from group
            test_name = f"{test_prefix}_invalid"
            content = self._build_hierarchy_for_oneof(block_name, parent_path, 
                                                      [members_list[0], members_list[1]])
            tests.append(ConstraintTest(
                name=test_name,
                block=block_name,
                content=content,
                should_pass=False,
                description=f"oneOf group '{group_label}' in {parent_path or 'root'}: invalid (both {members_list[0]} and {members_list[1]})"
            ))
        
        return tests
    
    def _build_hierarchy_for_oneof(self, block_name: str, parent_path: str, 
                                    keywords: List[str]) -> str:
        """
        Build hierarchy for oneOf test given the parent path directly.
        
        For oneOf groups, we have the parent_path directly from the group key.
        """
        # Parse the parent path
        if parent_path == block_name:
            hierarchy = []
        else:
            path_parts = parent_path.split('.')
            if path_parts[0] == block_name:
                hierarchy = path_parts[1:]
            else:
                hierarchy = path_parts
        
        # Build the output
        lines = [block_name]
        indent = "  "
        
        # Build up the current path as we go for context-aware lookup
        current_path = block_name
        
        # Add each level of hierarchy with parameter values if needed
        for level, parent_kw in enumerate(hierarchy):
            kw_info = self._get_keyword_info_by_name(parent_kw, current_path)
            if kw_info and kw_info.has_param:
                placeholder = self._get_placeholder_value(kw_info.param_type, kw_info)
                lines.append(f"{indent * (level + 1)}{parent_kw} {placeholder}")
            else:
                lines.append(f"{indent * (level + 1)}{parent_kw}")
            # Update current path for next iteration
            current_path = current_path + "." + parent_kw
        
        # Add the target keywords at the correct indentation
        # The parent_path IS the context for these keywords
        target_indent = indent * (len(hierarchy) + 1)
        for kw in keywords:
            kw_info = self._get_keyword_info_by_name(kw, parent_path)
            if kw_info and kw_info.has_param:
                placeholder = self._get_placeholder_value(kw_info.param_type, kw_info)
                lines.append(f"{target_indent}{kw} {placeholder}")
            else:
                lines.append(f"{target_indent}{kw}")
        
        return "\n".join(lines)
    
    def _generate_occurs_tests(self, block_info, block_name: str) -> List[ConstraintTest]:
        """Generate tests for minOccurs/maxOccurs constraints"""
        tests = []
        
        for kw_id, kw_info in block_info.keywords.items():
            # Sanitize keyword ID for test name (IDs are unique, names may not be)
            safe_kw_id = sanitize_test_name(kw_info.id)
            
            # Test maxOccurs violations
            if kw_info.max_occurs is not None and kw_info.max_occurs > 0:
                # Positive: At maxOccurs
                if kw_info.max_occurs == 1:
                    test_name = f"{block_name}_maxoccurs_{safe_kw_id}_valid"
                    content = self._build_keyword_occurrence(block_name, kw_info, 1)
                    tests.append(ConstraintTest(
                        name=test_name,
                        block=block_name,
                        content=content,
                        should_pass=True,
                        description=f"{kw_info.name} (id={kw_info.id}) occurs 1 time (maxOccurs={kw_info.max_occurs})"
                    ))
                
                # Negative: Over maxOccurs
                test_name = f"{block_name}_maxoccurs_{safe_kw_id}_invalid"
                content = self._build_keyword_occurrence(block_name, kw_info, 
                                                         kw_info.max_occurs + 1)
                tests.append(ConstraintTest(
                    name=test_name,
                    block=block_name,
                    content=content,
                    should_pass=False,
                    description=f"{kw_info.name} (id={kw_info.id}) occurs {kw_info.max_occurs + 1} times (exceeds maxOccurs={kw_info.max_occurs})"
                ))
        
        return tests
    
    def _build_keyword_occurrence(self, block_name: str, kw_info, count: int) -> str:
        """Build test with keyword appearing 'count' times with proper hierarchy"""
        # Find the path to this keyword
        parent_path = self._find_keyword_path(kw_info.name)
        
        if parent_path is None or parent_path == block_name:
            hierarchy = []
        else:
            path_parts = parent_path.split('.')
            if path_parts[0] == block_name:
                hierarchy = path_parts[1:]
            else:
                hierarchy = path_parts
        
        # Build the output
        lines = [block_name]
        indent = "  "
        
        # Build up the current path as we go for context-aware lookup
        current_path = block_name
        
        # Add each level of hierarchy with parameter values if needed
        for level, parent_kw in enumerate(hierarchy):
            parent_kw_info = self._get_keyword_info_by_name(parent_kw, current_path)
            if parent_kw_info and parent_kw_info.has_param:
                placeholder = self._get_placeholder_value(parent_kw_info.param_type, parent_kw_info)
                lines.append(f"{indent * (level + 1)}{parent_kw} {placeholder}")
            else:
                lines.append(f"{indent * (level + 1)}{parent_kw}")
            # Update current path for next iteration
            current_path = current_path + "." + parent_kw
        
        # Add the keyword 'count' times at the correct indentation
        target_indent = indent * (len(hierarchy) + 1)
        for i in range(count):
            if kw_info.has_param:
                # Use type-appropriate placeholder with index for uniqueness
                base_placeholder = self._get_placeholder_value(kw_info.param_type, kw_info)
                # For multiple occurrences, we need unique values
                # Modify the placeholder to include index if count > 1
                if count > 1:
                    param_type = (kw_info.param_type or '').upper()
                    # Get base value from constraint or default
                    base_val = None
                    if hasattr(kw_info, 'param_constraint') and kw_info.param_constraint:
                        constraint = kw_info.param_constraint
                        try:
                            if ">=" in constraint:
                                base_val = float(constraint.split(">=")[1].strip())
                            elif ">" in constraint and "<" not in constraint:
                                bound = float(constraint.split(">")[1].strip())
                                base_val = bound + 1
                        except (ValueError, IndexError):
                            pass
                    
                    if 'STRING' in param_type or 'FILENAME' in param_type:
                        placeholder = f"'value{i + 1}'"
                    elif 'INTEGER' in param_type:
                        if base_val is not None:
                            placeholder = str(int(base_val) + i)
                        else:
                            placeholder = str(i + 1)
                    elif 'REAL' in param_type:
                        if base_val is not None:
                            placeholder = str(base_val + i)
                        else:
                            placeholder = f"{i + 1}.0"
                    else:
                        placeholder = f"'value{i + 1}'"
                else:
                    placeholder = base_placeholder
                lines.append(f"{target_indent}{kw_info.name} = {placeholder}")
            else:
                lines.append(f"{target_indent}{kw_info.name}")
        
        return "\n".join(lines)
    
    def _generate_param_tests(self, block_info, block_name: str) -> List[ConstraintTest]:
        """Generate tests for parameter constraints (numeric bounds)"""
        tests = []
        
        for kw_id, kw_info in block_info.keywords.items():
            if not kw_info.has_param or not kw_info.param_constraint:
                continue
            
            # Sanitize keyword ID for test name (IDs are unique, names may not be)
            safe_kw_id = sanitize_test_name(kw_info.id)
            
            constraint = kw_info.param_constraint
            
            # Parse constraint like ">= 0" or "> 1.0"
            if ">=" in constraint:
                try:
                    bound = float(constraint.split(">=")[1].strip())
                except ValueError:
                    continue  # Skip non-numeric constraints like "LEN continuous_design"
                
                # Positive: At bound
                test_name = f"{block_name}_param_{safe_kw_id}_valid_at_bound"
                content = self._build_param_test(block_name, kw_info, bound, kw_id)
                tests.append(ConstraintTest(
                    name=test_name,
                    block=block_name,
                    content=content,
                    should_pass=True,
                    description=f"{kw_info.name} (id={kw_info.id}) = {bound} (valid, >= {bound})"
                ))
                
                # Negative: Below bound
                test_name = f"{block_name}_param_{safe_kw_id}_invalid_below"
                content = self._build_param_test(block_name, kw_info, bound - 1, kw_id)
                tests.append(ConstraintTest(
                    name=test_name,
                    block=block_name,
                    content=content,
                    should_pass=False,
                    description=f"{kw_info.name} (id={kw_info.id}) = {bound - 1} (invalid, < {bound})"
                ))
            
            elif ">" in constraint and "<" not in constraint:
                try:
                    bound = float(constraint.split(">")[1].strip())
                except ValueError:
                    continue  # Skip non-numeric constraints
                
                # Positive: Above bound
                test_name = f"{block_name}_param_{safe_kw_id}_valid_above"
                content = self._build_param_test(block_name, kw_info, bound + 1, kw_id)
                tests.append(ConstraintTest(
                    name=test_name,
                    block=block_name,
                    content=content,
                    should_pass=True,
                    description=f"{kw_info.name} (id={kw_info.id}) = {bound + 1} (valid, > {bound})"
                ))
                
                # Negative: At bound (not strictly greater)
                test_name = f"{block_name}_param_{safe_kw_id}_invalid_at"
                content = self._build_param_test(block_name, kw_info, bound, kw_id)
                tests.append(ConstraintTest(
                    name=test_name,
                    block=block_name,
                    content=content,
                    should_pass=False,
                    description=f"{kw_info.name} (id={kw_info.id}) = {bound} (invalid, not > {bound})"
                ))
        
        return tests
    
    def _build_param_test(self, block_name: str, kw_info, value: float, kw_id: str = None) -> str:
        """Build test with specific parameter value and proper hierarchy"""
        # Find the path to this keyword using its specific parent chain
        # This is more accurate for keywords with the same name but different IDs
        parent_path = self._find_keyword_path_by_id(kw_id or '', kw_info)
        
        # Fall back to name-based lookup if ID-based lookup fails
        if parent_path is None:
            parent_path = self._find_keyword_path(kw_info.name)
        
        if parent_path is None or parent_path == block_name:
            hierarchy = []
        else:
            path_parts = parent_path.split('.')
            if path_parts[0] == block_name:
                hierarchy = path_parts[1:]
            else:
                hierarchy = path_parts
        
        # Build the output
        lines = [block_name]
        indent = "  "
        
        # Build up the current path as we go for context-aware lookup
        current_path = block_name
        
        # Add each level of hierarchy with parameter values if needed
        for level, parent_kw in enumerate(hierarchy):
            parent_kw_info = self._get_keyword_info_by_name(parent_kw, current_path)
            if parent_kw_info and parent_kw_info.has_param:
                placeholder = self._get_placeholder_value(parent_kw_info.param_type, parent_kw_info)
                lines.append(f"{indent * (level + 1)}{parent_kw} {placeholder}")
            else:
                lines.append(f"{indent * (level + 1)}{parent_kw}")
            # Update current path for next iteration
            current_path = current_path + "." + parent_kw
        
        # Add the keyword with value
        target_indent = indent * (len(hierarchy) + 1)
        lines.append(f"{target_indent}{kw_info.name} = {value}")
        
        return "\n".join(lines)
    
    def _write_test(self, test: ConstraintTest):
        """Write test to appropriate directory"""
        target_dir = self.positive_dir if test.should_pass else self.negative_dir
        
        # Sanitize the test name for the filename as well
        safe_filename = sanitize_test_name(test.name)
        test_file = target_dir / f"{safe_filename}.in"
        
        # Write test file
        with open(test_file, 'w') as f:
            f.write(f"# {test.description}\n")
            f.write(test.content)
            f.write("\n")
        
        self.tests_generated += 1

def generate_constraint_tests(block_info, block_name: str, output_dir: Path) -> int:
    """Generate constraint tests for a block"""
    generator = ConstraintTestGenerator(output_dir)
    count = generator.generate_tests_for_block(block_info, block_name)
    return count
