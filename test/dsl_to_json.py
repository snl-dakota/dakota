#!/usr/bin/env python3
"""
Dakota DSL to JSON Converter

Converts Dakota input files (in DSL format, canonicalized via dakota_order_input)
to JSON format suitable for validation against Pydantic models.

This script uses the XML grammar to understand the hierarchical structure of
keywords and correctly interpret literal arguments.
"""

import re
import json
import os
import sys
import importlib
import importlib.util
import xml.etree.ElementTree as ET
from pathlib import Path
from dataclasses import dataclass, field
from enum import Enum
from typing import Dict, List, Optional, Any, Tuple, Set


def get_project_dir() -> Path:
    """Return the Dakota project root for test-local path discovery."""
    env_project_dir = os.environ.get("PROJECT_DIR")
    if env_project_dir:
        return Path(env_project_dir).resolve()
    return Path(__file__).resolve().parents[3]


# Major Dakota sections
MAJOR_SECTIONS = {'environment', 'method', 'model', 'variables', 'interface', 'responses'}


class Severity(Enum):
    WARNING = "warning"
    ERROR = "error"


@dataclass
class ParseDiagnostic:
    """A diagnostic message (warning or error) from DSL parsing."""
    severity: Severity
    message: str
    line_number: Optional[int] = None
    keyword: Optional[str] = None
    context: Optional[str] = None

    def __str__(self):
        parts = [f"[{self.severity.value.upper()}]"]
        if self.line_number is not None:
            parts.append(f"line {self.line_number}:")
        if self.context:
            parts.append(f"(in {self.context})")
        parts.append(self.message)
        return " ".join(parts)

# Type mapping for parsing literals
TYPE_MAP = {
    'REAL': float,
    'INTEGER': int,
    'STRING': str,
    'REALLIST': lambda x: [float(v) for v in x],
    'INTEGERLIST': lambda x: [int(v) for v in x],
    'STRINGLIST': lambda x: list(x),
    'INPUT_FILE': str,
    'OUTPUT_FILE': str,
}


@dataclass
class KeywordSpec:
    """Specification for a keyword from the XML grammar"""
    name: str
    argument: Optional[str] = None  # synthetic field name for literal argument
    param_type: Optional[str] = None  # INTEGER, REAL, STRING, etc.
    children: Dict[str, 'KeywordSpec'] = field(default_factory=dict)
    aliases: Dict[str, str] = field(default_factory=dict)  # alias -> canonical name
    anchored_oneofs: Dict[str, Set[str]] = field(default_factory=dict)  # anchor -> member keywords
    is_leaf: bool = False  # True if keyword has no children and no param (boolean flag)


def expand_entities(xml_content: str) -> str:
    """Expand all entity references in XML content"""
    # Extract entity definitions
    entities = {}
    for match in re.finditer(r'<!ENTITY\s+(\w+)\s+"([^"]*)"', xml_content, re.DOTALL):
        entities[match.group(1)] = match.group(2)
    
    # Remove entity definitions from content
    content = re.sub(r'<!ENTITY\s+\w+\s+"[^"]*">', '', xml_content, flags=re.DOTALL)
    
    # Repeatedly expand entity references until no more remain
    max_iterations = 100
    for _ in range(max_iterations):
        new_content = content
        for name, value in entities.items():
            new_content = new_content.replace(f'&{name};', value)
        if new_content == content:
            break
        content = new_content
    
    return content


def parse_grammar(xml_path: str) -> Dict[str, KeywordSpec]:
    """Parse the XML grammar and return keyword specifications for each major section"""
    
    with open(xml_path, 'r') as f:
        xml_content = f.read()
    
    # Expand entities
    expanded = expand_entities(xml_content)
    
    # Remove DOCTYPE declaration (ET can't handle it after entity expansion)
    expanded = re.sub(r'<!DOCTYPE[^>]*\[[\s\S]*?\]>', '', expanded)
    
    # Parse XML
    root = ET.fromstring(expanded)
    
    # Handle namespace - the document uses xmlns="http://www.sandia.gov/dakota/1.0"
    ns = {'d': 'http://www.sandia.gov/dakota/1.0'}
    
    # Find the input element (might be root or child)
    input_elem = root
    if root.tag.endswith('document') or root.tag == '{http://www.sandia.gov/dakota/1.0}document':
        # Look for input child
        for child in root:
            if child.tag.endswith('input') or child.tag == '{http://www.sandia.gov/dakota/1.0}input':
                input_elem = child
                break
    
    # Build keyword specs for each major section
    sections = {}
    
    for section_elem in input_elem:
        tag = section_elem.tag
        # Handle namespaced tags
        if '}' in tag:
            tag = tag.split('}')[1]
        
        if tag == 'keyword':
            section_name = section_elem.get('name')
            if section_name in MAJOR_SECTIONS:
                sections[section_name] = parse_keyword_element(section_elem)
    
    return sections


def parse_keyword_element(elem: ET.Element) -> KeywordSpec:
    """Recursively parse a keyword element into a KeywordSpec"""
    name = elem.get('name')
    argument = elem.get('argument')
    
    # Find param element (direct child only)
    param_elem = None
    for child in elem:
        tag = child.tag.split('}')[-1] if '}' in child.tag else child.tag
        if tag == 'param':
            param_elem = child
            break
    
    param_type = param_elem.get('type') if param_elem is not None else None
    
    # Collect children, aliases, and anchored oneOf groups
    children = {}
    aliases = {}
    anchored_oneofs = {}
    
    def process_children(parent_elem, in_anchor: Optional[str] = None):
        """Process child elements, tracking oneOf anchors"""
        for child in parent_elem:
            tag = child.tag.split('}')[-1] if '}' in child.tag else child.tag
            
            if tag == 'keyword':
                child_name = child.get('name')
                child_spec = parse_keyword_element(child)
                children[child_name] = child_spec
                
                # Track if this keyword is in an anchored oneOf
                if in_anchor:
                    if in_anchor not in anchored_oneofs:
                        anchored_oneofs[in_anchor] = set()
                    anchored_oneofs[in_anchor].add(child_name)
                    
            elif tag == 'alias':
                alias_name = child.get('name')
                aliases[alias_name] = name
                
            elif tag == 'oneOf':
                anchor = child.get('anchor')
                process_children(child, in_anchor=anchor)
                
            elif tag == 'optional':
                process_children(child, in_anchor=in_anchor)
                
            elif tag == 'group':
                # Group is a UI organizational element, process its children normally
                process_children(child, in_anchor=in_anchor)
    
    process_children(elem)
    
    # Determine if this is a leaf (boolean flag)
    is_leaf = (not children) and (param_type is None) and (argument is None)
    
    return KeywordSpec(
        name=name,
        argument=argument,
        param_type=param_type,
        children=children,
        aliases=aliases,
        anchored_oneofs=anchored_oneofs,
        is_leaf=is_leaf
    )


@dataclass
class ParsedLine:
    """A parsed line from the DSL input"""
    indent: int
    keyword: str
    has_equals: bool
    values: List[str]
    line_number: Optional[int] = None


def tokenize_line(line: str) -> Optional[ParsedLine]:
    """Parse a single line into keyword and values"""
    if not line.strip() or line.strip().startswith('#'):
        return None
    
    # Count leading whitespace (indent level)
    stripped = line.lstrip()
    indent = len(line) - len(stripped)
    
    # Check for equals sign
    has_equals = '=' in stripped
    
    if has_equals:
        # Split on first equals
        parts = stripped.split('=', 1)
        keyword = parts[0].strip()
        value_str = parts[1].strip() if len(parts) > 1 else ''
    else:
        # First token is keyword, rest are values
        tokens = tokenize_values(stripped)
        keyword = tokens[0] if tokens else ''
        value_str = ' '.join(tokens[1:]) if len(tokens) > 1 else ''
    
    values = tokenize_values(value_str) if value_str else []
    
    return ParsedLine(indent=indent, keyword=keyword, has_equals=has_equals, values=values)


def tokenize_values(s: str) -> List[str]:
    """Tokenize a string, respecting quoted strings"""
    tokens = []
    current = ''
    in_quote = False
    quote_char = None
    
    i = 0
    while i < len(s):
        c = s[i]
        
        if in_quote:
            if c == quote_char:
                # End of quoted string - don't include the quote
                tokens.append(current)
                current = ''
                in_quote = False
            else:
                current += c
        else:
            if c in ('"', "'"):
                # Start of quoted string
                if current:
                    tokens.append(current)
                    current = ''
                in_quote = True
                quote_char = c
            elif c.isspace():
                if current:
                    tokens.append(current)
                    current = ''
            else:
                current += c
        i += 1
    
    if current:
        tokens.append(current)
    
    return tokens


def parse_value(value_str: str, param_type: Optional[str]) -> Any:
    """Parse a single value string according to its type"""
    if param_type == 'INTEGER':
        return int(value_str)
    elif param_type == 'REAL':
        return float(value_str)
    else:
        return value_str


def parse_values(values: List[str], param_type: Optional[str]) -> Any:
    """Parse a list of value strings according to type"""
    if not values:
        return None
    
    if param_type in ('REALLIST', 'INTEGERLIST', 'STRINGLIST'):
        if param_type == 'REALLIST':
            return [float(v) for v in values]
        elif param_type == 'INTEGERLIST':
            return [int(v) for v in values]
        else:
            return values
    elif param_type in ('INTEGER', 'REAL', 'STRING', 'INPUT_FILE', 'OUTPUT_FILE'):
        # Single value expected
        if len(values) == 1:
            return parse_value(values[0], param_type)
        else:
            # Multiple values but scalar type - return first but flag it
            return parse_value(values[0], param_type)
    else:
        # Unknown type, return as-is
        if len(values) == 1:
            # Try to infer type
            try:
                return int(values[0])
            except ValueError:
                try:
                    return float(values[0])
                except ValueError:
                    return values[0]
        else:
            # Multiple values, try to infer list type
            result = []
            for v in values:
                try:
                    result.append(int(v))
                except ValueError:
                    try:
                        result.append(float(v))
                    except ValueError:
                        result.append(v)
            return result


class DSLParser:
    """Parses Dakota DSL input files to JSON"""
    
    def __init__(self, grammar: Dict[str, KeywordSpec]):
        self.grammar = grammar
        self.diagnostics: List[ParseDiagnostic] = []
    
    def _diag(self, severity: Severity, message: str, *,
              line_number: Optional[int] = None,
              keyword: Optional[str] = None,
              context: Optional[str] = None):
        """Record a diagnostic."""
        self.diagnostics.append(ParseDiagnostic(
            severity=severity, message=message,
            line_number=line_number, keyword=keyword, context=context,
        ))
    
    def parse_file(self, input_path: str) -> Dict[str, Any]:
        """Parse a Dakota input file and return JSON structure"""
        with open(input_path, 'r') as f:
            lines = f.readlines()
        
        return self.parse_lines(lines)
    
    def parse_lines(self, lines: List[str]) -> Dict[str, Any]:
        """Parse lines into JSON structure"""
        self.diagnostics.clear()
        
        result = {
            'environment': None,
            'method': [],
            'model': [],
            'variables': [],
            'interface': [],
            'responses': []
        }
        
        # Parse all lines, preserving line numbers
        parsed_lines = []
        for line_num, line in enumerate(lines, start=1):
            parsed = tokenize_line(line)
            if parsed:
                parsed.line_number = line_num
                parsed_lines.append(parsed)
        
        if not parsed_lines:
            self._diag(Severity.WARNING, "Input file is empty or contains only comments/blank lines")
            return result
        
        # Detect the environment concatenation bug (e.g. "environmentmethod")
        for pline in parsed_lines:
            for section in MAJOR_SECTIONS:
                if (pline.keyword.startswith(section)
                        and pline.keyword != section
                        and len(pline.keyword) > len(section)
                        and pline.keyword[len(section):] not in ('.', '_', '-')
                        and pline.keyword[len(section)].isalpha()):
                    remainder = pline.keyword[len(section):]
                    self._diag(
                        Severity.ERROR,
                        f"Possible keyword concatenation bug: '{pline.keyword}' "
                        f"looks like '{section}' + '{remainder}' with no separating whitespace. "
                        f"This is a known dakota_order_input bug for empty blocks.",
                        line_number=pline.line_number,
                        keyword=pline.keyword,
                    )
        
        # Process lines into sections
        i = 0
        while i < len(parsed_lines):
            line = parsed_lines[i]
            keyword = line.keyword
            
            # Check if this line starts a major section
            section_name = None
            
            if keyword in MAJOR_SECTIONS:
                section_name = keyword
            elif ' ' in keyword:
                first_word = keyword.split()[0]
                if first_word in MAJOR_SECTIONS:
                    section_name = first_word
            
            if section_name:
                spec = self.grammar.get(section_name)
                
                if not spec:
                    self._diag(
                        Severity.ERROR,
                        f"No grammar specification for section '{section_name}'",
                        line_number=line.line_number,
                        keyword=section_name,
                    )
                    i += 1
                    continue
                
                # Collect lines for this section
                section_lines = []
                
                # Handle the initial line - may contain section + keyword + value
                if keyword == section_name:
                    # Simple case: "environment" or "environment graphics"
                    if line.values:
                        first_keyword = line.values[0]
                        remaining_values = line.values[1:]
                        section_lines.append(ParsedLine(
                            indent=1,
                            keyword=first_keyword,
                            has_equals=line.has_equals and bool(remaining_values),
                            values=remaining_values,
                            line_number=line.line_number,
                        ))
                else:
                    # Complex case: "variables normal_uncertain = 2"
                    rest = keyword[len(section_name):].strip()
                    combined_values = ([rest] if rest else []) + line.values
                    
                    if combined_values:
                        first_keyword = combined_values[0]
                        remaining_values = combined_values[1:]
                        section_lines.append(ParsedLine(
                            indent=1,
                            keyword=first_keyword,
                            has_equals=line.has_equals,
                            values=remaining_values,
                            line_number=line.line_number,
                        ))
                
                # Collect subsequent lines belonging to this section
                i += 1
                while i < len(parsed_lines):
                    next_line = parsed_lines[i]
                    
                    next_keyword = next_line.keyword
                    is_new_section = False
                    
                    if next_line.indent == 0:
                        if next_keyword in MAJOR_SECTIONS:
                            is_new_section = True
                        elif ' ' in next_keyword:
                            first_word = next_keyword.split()[0]
                            if first_word in MAJOR_SECTIONS:
                                is_new_section = True
                    
                    if is_new_section:
                        break
                    
                    section_lines.append(next_line)
                    i += 1
                
                # Parse the section
                section_data = self.parse_section(section_lines, spec)
                
                if section_name == 'environment':
                    result['environment'] = section_data
                else:
                    result[section_name].append(section_data)
            else:
                # Line at top level that doesn't match any major section
                self._diag(
                    Severity.ERROR,
                    f"Unrecognized top-level token '{keyword}'; "
                    f"expected one of: {', '.join(sorted(MAJOR_SECTIONS))}",
                    line_number=line.line_number,
                    keyword=keyword,
                )
                i += 1
       
        # Check for empty result
        has_content = False
        for key, value in result.items():
            if key == 'environment' and isinstance(value, dict):
                has_content = True
            elif isinstance(value, list) and value:
                has_content = True
        
        if not has_content:
            self._diag(Severity.ERROR, "Parsing produced no sections; "
                       "the input may be malformed or empty")
        
        final_result = {}
        for key, value in result.items():
            if key == "environment":
                if isinstance(value, dict):
                    final_result[key] = value
            elif value:
                final_result[key] = value
                
        return final_result
    
    def parse_section(self, lines: List[ParsedLine], spec: KeywordSpec) -> Dict[str, Any]:
        """Parse a section's lines into a dictionary"""
        result = {}
        
        i = 0
        while i < len(lines):
            line = lines[i]
            i = self.process_keyword_line(lines, i, result, spec, base_indent=0)
        
        return result
    
    def resolve_keyword(self, keyword: str, spec: KeywordSpec) -> Tuple[str, Optional[KeywordSpec]]:
        """Resolve a keyword (possibly an alias) to its canonical name and spec"""
        # Check direct children
        if keyword in spec.children:
            return keyword, spec.children[keyword]
        
        # Check aliases in children
        for child_name, child_spec in spec.children.items():
            if keyword in child_spec.aliases:
                return child_name, child_spec
            # Also check if the keyword itself is aliased at this level
            if keyword in spec.aliases:
                canonical = spec.aliases[keyword]
                if canonical in spec.children:
                    return canonical, spec.children[canonical]
        
        return keyword, None
    
    def find_anchor_for_keyword(self, keyword: str, spec: KeywordSpec) -> Optional[str]:
        """Find if a keyword belongs to an anchored oneOf group"""
        for anchor, members in spec.anchored_oneofs.items():
            if keyword in members:
                return anchor
        return None
    
    def process_keyword_line(self, lines: List[ParsedLine], idx: int, 
                             target: Dict[str, Any], spec: KeywordSpec,
                             base_indent: int) -> int:
        """Process a keyword line and its children, return next index to process"""
        if idx >= len(lines):
            return idx
        
        line = lines[idx]
        keyword = line.keyword
        
        # Resolve aliases
        canonical_name, child_spec = self.resolve_keyword(keyword, spec)
        
        if child_spec is None:
            # Unknown keyword - record error and store raw
            self._diag(
                Severity.ERROR,
                f"Unknown keyword '{keyword}' in context '{spec.name}'",
                line_number=line.line_number,
                keyword=keyword,
                context=spec.name,
            )
            if line.values:
                target[keyword] = parse_values(line.values, None)
            else:
                target[keyword] = True
            return idx + 1
        
        # Check if this keyword belongs to an anchored oneOf
        anchor = self.find_anchor_for_keyword(canonical_name, spec)
        
        # Determine the target dictionary for this keyword
        if anchor:
            # Create/get the anchor wrapper
            if anchor not in target:
                target[anchor] = {}
            actual_target = target[anchor]
        else:
            actual_target = target
        
        # Process based on keyword type
        if child_spec.is_leaf:
            # Boolean flag
            actual_target[canonical_name] = True
            return idx + 1
        
        elif child_spec.argument:
            # Keyword with argument attribute - create nested structure
            arg_name = child_spec.argument
            
            # Parse the literal value
            if line.values:
                arg_value = parse_values(line.values, child_spec.param_type)
            else:
                arg_value = None
            
            # Create the nested structure
            nested = {}
            if arg_value is not None:
                nested[arg_name] = arg_value
            
            # Process child keywords
            idx += 1
            child_indent = line.indent
            while idx < len(lines):
                next_line = lines[idx]
                # Children must be more indented than this line
                if next_line.indent <= child_indent:
                    break
                idx = self.process_keyword_line(lines, idx, nested, child_spec, child_indent)
            
            actual_target[canonical_name] = nested
            return idx
        
        elif child_spec.param_type:
            # Keyword with param (literal value)
            if line.values:
                if (child_spec.param_type in ('INTEGER', 'REAL', 'STRING', 'INPUT_FILE', 'OUTPUT_FILE')
                        and len(line.values) > 1):
                    self._diag(
                        Severity.WARNING,
                        f"Keyword '{canonical_name}' expects a single {child_spec.param_type} value "
                        f"but got {len(line.values)} values: {line.values!r}; using first value only",
                        line_number=line.line_number,
                        keyword=canonical_name,
                        context=spec.name,
                    )
                value = parse_values(line.values, child_spec.param_type)
                actual_target[canonical_name] = value
            else:
                actual_target[canonical_name] = None
            return idx + 1
        
        elif child_spec.children:
            # Keyword with children but no param - create nested structure
            nested = {}
            
            # Process child keywords
            idx += 1
            child_indent = line.indent
            while idx < len(lines):
                next_line = lines[idx]
                if next_line.indent <= child_indent:
                    break
                idx = self.process_keyword_line(lines, idx, nested, child_spec, child_indent)
            
            actual_target[canonical_name] = nested
            return idx
        
        else:
            # Fallback - treat as boolean or value
            if line.values:
                actual_target[canonical_name] = parse_values(line.values, None)
            else:
                actual_target[canonical_name] = True
            return idx + 1


class DSLParseError(Exception):
    """Raised when DSL parsing encounters errors."""
    def __init__(self, message: str, diagnostics: List[ParseDiagnostic]):
        self.diagnostics = diagnostics
        super().__init__(message)


class DakotaDSLConverter:
    """Reusable converter for Dakota DSL files to JSON.
    
    Parses the grammar once and can convert multiple files efficiently.
    
    Usage:
        converter = DakotaDSLConverter('dakota_grammar.xml')
        
        # Convert multiple files
        for input_file in input_files:
            result = converter.convert(input_file)
            
        # Or with validation
        converter = DakotaDSLConverter('dakota_grammar.xml', validate=True)
        result = converter.convert('input.in', output_path='output.json')
    """
    
    def __init__(self, grammar_path: str, validate: bool = False, verbose: bool = True):
        """Initialize converter with grammar.
        
        Args:
            grammar_path: Path to Dakota XML grammar file
            validate: If True, validate output against Pydantic models
            verbose: If True, print progress messages
        """
        self.grammar_path = grammar_path
        self.validate = validate
        self.verbose = verbose
        self._dakota_study_class = None
        self.last_diagnostics: List[ParseDiagnostic] = []
        
        if self.verbose:
            print(f"Parsing grammar from {grammar_path}...")
        self.grammar = parse_grammar(grammar_path)
        self.parser = DSLParser(self.grammar)
        
        # Pre-load Pydantic model if validation requested
        if self.validate:
            self._load_pydantic_model()
    
    def _load_pydantic_model(self):
        """Load the Pydantic DakotaStudy model for validation"""
        candidate_roots = []

        env_root = os.environ.get("DAKOTA_PYTHON_PACKAGE_DIR")
        if env_root:
            candidate_roots.append(Path(env_root))

        repo_root = get_project_dir()
        candidate_roots.extend([
            repo_root / "model_generation" / "build" / "dakota_package",
            repo_root / "model_generation" / "build" / "generated_models",
        ])

        def clear_dakota_modules():
            for mod_name in list(sys.modules.keys()):
                if mod_name == "dakota" or mod_name.startswith("dakota."):
                    del sys.modules[mod_name]
            importlib.invalidate_caches()

        def try_import():
            mod = importlib.import_module("dakota.spec.study")
            return mod.DakotaStudy

        def try_import_from_root(root: Path):
            dakota_init = root / "dakota" / "__init__.py"
            spec_init = root / "dakota" / "spec" / "__init__.py"
            study_py = root / "dakota" / "spec" / "study.py"
            if not (dakota_init.exists() and spec_init.exists() and study_py.exists()):
                raise ImportError(f"incomplete Dakota package under {root}")

            clear_dakota_modules()

            dakota_spec = importlib.util.spec_from_file_location(
                "dakota",
                dakota_init,
                submodule_search_locations=[str(root / "dakota")],
            )
            dakota_mod = importlib.util.module_from_spec(dakota_spec)
            sys.modules["dakota"] = dakota_mod
            dakota_spec.loader.exec_module(dakota_mod)

            spec_spec = importlib.util.spec_from_file_location(
                "dakota.spec",
                spec_init,
                submodule_search_locations=[str(root / "dakota" / "spec")],
            )
            spec_mod = importlib.util.module_from_spec(spec_spec)
            sys.modules["dakota.spec"] = spec_mod
            spec_spec.loader.exec_module(spec_mod)

            study_spec = importlib.util.spec_from_file_location(
                "dakota.spec.study",
                study_py,
            )
            study_mod = importlib.util.module_from_spec(study_spec)
            sys.modules["dakota.spec.study"] = study_mod
            study_spec.loader.exec_module(study_mod)
            return study_mod.DakotaStudy

        try:
            DakotaStudy = try_import()
            self._dakota_study_class = DakotaStudy
            if self.verbose:
                print("Pydantic model loaded for validation")
            return
        except ImportError:
            pass

        for root in candidate_roots:
            if not root.exists():
                continue
            root_str = str(root)
            if root_str not in sys.path:
                sys.path.insert(0, root_str)
            try:
                DakotaStudy = try_import()
                self._dakota_study_class = DakotaStudy
                if self.verbose:
                    print(f"Pydantic model loaded for validation from {root}")
                return
            except ImportError:
                try:
                    DakotaStudy = try_import_from_root(root)
                    self._dakota_study_class = DakotaStudy
                    if self.verbose:
                        print(f"Pydantic model loaded for validation from {root}")
                    return
                except ImportError:
                    continue

        details = ", ".join(str(p) for p in candidate_roots if p.exists())
        print(
            "Warning: Could not import Pydantic models for validation: "
            f"No module named 'dakota.spec' (searched PYTHONPATH and fallback roots: {details})"
        )
        raise ModuleNotFoundError("No module named 'dakota.spec'")
    
    def convert(self, input_path: str, output_path: Optional[str] = None,
                raw_output_path: Optional[str] = None) -> Dict[str, Any]:
        """Convert a Dakota DSL file to JSON.
        
        Args:
            input_path: Path to Dakota input file (.in)
            output_path: Optional path to write (validated) JSON output
            raw_output_path: Optional path to write raw JSON before validation
            
        Returns:
            Dictionary containing the parsed/validated JSON structure
            
        Raises:
            DSLParseError: If parsing encounters errors (unrecognized keywords,
                concatenation bugs, empty results, etc.)
        """
        if self.verbose:
            print(f"Parsing input file {input_path}...")
        
        result = self.parser.parse_file(input_path)
        self.last_diagnostics = list(self.parser.diagnostics)
        
        # Log warnings
        for diag in self.last_diagnostics:
            if self.verbose or diag.severity == Severity.ERROR:
                print(f"  {input_path}: {diag}")
        
        # Raise on errors
        errors = [d for d in self.last_diagnostics if d.severity == Severity.ERROR]
        if errors:
            summary = "; ".join(d.message for d in errors)
            raise DSLParseError(
                f"DSL parsing failed for {input_path} with {len(errors)} error(s): {summary}",
                diagnostics=self.last_diagnostics,
            )
        
        result = self.parser.parse_file(input_path)
        
        # Write raw output if requested (before validation)
        if raw_output_path:
            Path(raw_output_path).parent.mkdir(parents=True, exist_ok=True)
            with open(raw_output_path, 'w') as f:
                json.dump(result, f, indent=2, sort_keys=True)
            if self.verbose:
                print(f"Raw JSON written to {raw_output_path}")
        
        # Validate if requested and model is available
        if self.validate and self._dakota_study_class is not None:
            if self.verbose:
                print("Validating against Pydantic model...")
            try:
                study = self._dakota_study_class(**result)
                result = study.model_dump(exclude_none=True, mode="json")
                if self.verbose:
                    print("Validation successful!")
            except Exception as e:
                print(f"Validation error: {e}")
                raise
        
        # Write output
        if output_path:
            # Ensure parent directory exists
            Path(output_path).parent.mkdir(parents=True, exist_ok=True)
            with open(output_path, 'w') as f:
                json.dump(result, f, indent=2, sort_keys=True)
            if self.verbose:
                print(f"Output written to {output_path}")
        
        return result
    
    def convert_string(self, dsl_content: str) -> Dict[str, Any]:
        """Convert Dakota DSL content from a string.
        
        Args:
            dsl_content: Dakota DSL input as a string
            
        Returns:
            Dictionary containing the parsed/validated JSON structure
            
        Raises:
            DSLParseError: If parsing encounters errors
        """
        lines = dsl_content.splitlines(keepends=True)
        result = self.parser.parse_lines(lines)
        self.last_diagnostics = list(self.parser.diagnostics)
        
        errors = [d for d in self.last_diagnostics if d.severity == Severity.ERROR]
        if errors:
            summary = "; ".join(d.message for d in errors)
            raise DSLParseError(
                f"DSL parsing failed with {len(errors)} error(s): {summary}",
                diagnostics=self.last_diagnostics,
            )
        
        if self.validate and self._dakota_study_class is not None:
            study = self._dakota_study_class(**result)
            result = study.model_dump(exclude_none=True)
        
        return result


def convert_file(input_path: str, grammar_path_or_converter, output_path: Optional[str] = None,
                 validate: bool = True) -> Dict[str, Any]:
    """Convert a Dakota DSL file to JSON.
    
    Args:
        input_path: Path to Dakota input file
        grammar_path_or_converter: Either a path to XML grammar file (str) or 
                                   a pre-initialized DakotaDSLConverter instance
        output_path: Optional path to write JSON output
        validate: If True and grammar_path is a string, validate against Pydantic models
        
    Returns:
        Dictionary containing the parsed/validated JSON structure
    """
    # Allow passing either a grammar path or a pre-built converter
    if isinstance(grammar_path_or_converter, DakotaDSLConverter):
        converter = grammar_path_or_converter
        return converter.convert(input_path, output_path)
    else:
        # Legacy behavior: create converter for single use
        grammar_path = grammar_path_or_converter
        converter = DakotaDSLConverter(grammar_path, validate=validate)
        return converter.convert(input_path, output_path)


def main():
    import argparse
    
    arg_parser = argparse.ArgumentParser(
        description='Convert Dakota DSL input files to JSON'
    )
    arg_parser.add_argument('input', nargs='+', help='Dakota input file(s) (.in)')
    arg_parser.add_argument('--grammar', '-g', 
                          default='dakota_with_defaults_blurbs_oneofs.xml',
                          help='Path to Dakota XML grammar')
    arg_parser.add_argument('--output', '-o', help='Output JSON file (only valid for single input)')
    arg_parser.add_argument('--output-dir', '-d', help='Output directory for JSON files')
    arg_parser.add_argument('--raw-output-dir', '-r', help='Output directory for raw (pre-validation) JSON files')
    arg_parser.add_argument('--no-validate', action='store_true',
                          help='Skip Pydantic validation')
    arg_parser.add_argument('--print', '-p', action='store_true',
                          help='Print JSON to stdout')
    arg_parser.add_argument('--quiet', '-q', action='store_true',
                          help='Suppress progress messages')
    
    args = arg_parser.parse_args()
    
    # Validate arguments
    if args.output and len(args.input) > 1:
        print("Error: --output can only be used with a single input file")
        sys.exit(1)
    
    # Create converter once
    converter = DakotaDSLConverter(
        args.grammar, 
        validate=not args.no_validate,
        verbose=not args.quiet
    )
    
    # Process each input file
    for input_path in args.input:
        # Determine output path
        if args.print:
            output_path = None
        elif args.output:
            output_path = args.output
        elif args.output_dir:
            output_path = Path(args.output_dir) / (Path(input_path).stem + '.json')
        else:
            output_path = Path(input_path).stem + '.json'
        
        # Determine raw output path
        raw_output_path = None
        if args.raw_output_dir:
            raw_output_path = Path(args.raw_output_dir) / (Path(input_path).stem + '_raw.json')
        
        result = converter.convert(
            input_path, 
            str(output_path) if output_path else None,
            str(raw_output_path) if raw_output_path else None
        )
        
        if args.print:
            print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
