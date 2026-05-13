#!/usr/bin/env python3

from __future__ import annotations

import argparse
import json
import sys
import textwrap
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any


FIELD_HEADERS = (
    "Blurb::",
    "Description::",
    "Topics::",
    "Examples::",
    "Theory::",
    "Faq::",
    "See_Also::",
)


@dataclass
class Metadata:
    blurb: str = ""
    description: str = ""
    topics: str = ""
    examples: str = ""
    theory: str = ""
    faq: str = ""
    see_also: str = ""


@dataclass
class ChildRow:
    name: str
    hierarchy: str
    required: bool
    group_name: str = ""
    group_description: str = ""
    json_group_key: str = ""


@dataclass
class KeywordPage:
    hierarchy: str
    name: str
    description: str = ""
    aliases: list[str] = field(default_factory=list)
    default: Any = None
    has_default: bool = False
    arguments: str = ""
    json_argument_key: str | None = None
    children: list[ChildRow] = field(default_factory=list)


class SchemaResolver:
    def __init__(self, schema: dict[str, Any]) -> None:
        self.schema = schema

    def resolve_ref(self, ref: str) -> dict[str, Any]:
        node: Any = self.schema
        for part in ref.lstrip("#/").split("/"):
            node = node[part]
        if not isinstance(node, dict):
            raise TypeError(f"Reference {ref} did not resolve to an object")
        return node

    def dereference(self, node: dict[str, Any]) -> dict[str, Any]:
        while isinstance(node, dict) and "$ref" in node:
            node = self.resolve_ref(node["$ref"])
        return node

    def strip_null_anyof(self, node: dict[str, Any]) -> list[dict[str, Any]]:
        return [
            entry
            for entry in node.get("anyOf", [])
            if not (isinstance(entry, dict) and entry.get("type") == "null")
        ]


class KeywordMetadataStore:
    def __init__(self, metadata_dir: Path) -> None:
        self.metadata_dir = metadata_dir
        self.entries: dict[str, Metadata] = {}
        self.raw_files: set[str] = set()
        self._parse()

    def _parse(self) -> None:
        for path in sorted(self.metadata_dir.iterdir()):
            if path.name.startswith(".svn") or path.name.startswith("."):
                continue
            self.raw_files.add(path.name)
            if path.name.startswith("DUPLICATE-"):
                continue
            self.entries[path.name] = self._read_with_duplicates(path)

    def _read_with_duplicates(self, path: Path) -> Metadata:
        lines = path.read_text(encoding="utf-8").splitlines()
        if lines and lines[0].strip().startswith("DUPLICATE-"):
            duplicate_path = self.metadata_dir / lines[0].strip()
            if not duplicate_path.is_file():
                print(
                    f"Warning: Missing DUPLICATE metadata file {duplicate_path}",
                    file=sys.stderr,
                )
                return Metadata()
            return self._read_fields(duplicate_path.read_text(encoding="utf-8").splitlines())
        return self._read_fields(lines)

    def _read_fields(self, lines: list[str]) -> Metadata:
        values: dict[str, str] = {}
        current_field = ""
        current_data: list[str] = []
        for line in lines:
            if line.startswith(FIELD_HEADERS):
                if current_field:
                    values[current_field] = "\n".join(current_data).strip("\n")
                field, data = (line.split("::", 1) + [""])[:2]
                current_field = field.strip()
                current_data = [data.lstrip()] if data else []
            else:
                current_data.append(line)
        if current_field:
            values[current_field] = "\n".join(current_data).strip("\n")
        return Metadata(
            blurb=values.get("Blurb", "").strip(),
            description=values.get("Description", "").strip(),
            topics=values.get("Topics", "").strip(),
            examples=values.get("Examples", "").strip(),
            theory=values.get("Theory", "").strip(),
            faq=values.get("Faq", "").strip(),
            see_also=values.get("See_Also", "").strip(),
        )


class KeywordReferenceGenerator:
    def __init__(self, schema: dict[str, Any], metadata_store: KeywordMetadataStore) -> None:
        self.resolver = SchemaResolver(schema)
        self.metadata_store = metadata_store
        self.pages: dict[str, KeywordPage] = {}
        self.synthetic_paths: set[str] = set()
        self.reported_missing_metadata: set[str] = set()

    def build_pages(self) -> dict[str, KeywordPage]:
        for root_name in ("environment", "interface", "method", "model", "responses", "variables"):
            root_schema = self.resolver.schema["properties"][root_name]
            page = self._ensure_page(
                hierarchy=root_name,
                name=root_name,
                schema_node=root_schema,
                description=root_schema.get("description", ""),
            )
            page.children.extend(self._build_page_children(root_name, root_schema))
        return self.pages

    def validate_metadata(self) -> None:
        page_names = set(self.pages)
        extras = sorted(
            name
            for name in self.metadata_store.entries
            if name not in page_names and name not in self.synthetic_paths
        )
        misses = sorted(name for name in page_names if name not in self.metadata_store.entries)
        for name in extras:
            print(
                f"Warning: keyword metadata file {name} exists, but no such keyword in dakota.json. Skipping.",
                file=sys.stderr,
            )
        for name in misses:
            print(
                f"Warning: Missing metadata entry for keyword {name}",
                file=sys.stderr,
            )

    def write_pages(self, output_dir: Path) -> None:
        output_dir.mkdir(parents=True, exist_ok=True)
        for hierarchy, page in self.pages.items():
            output_path = output_dir / f"{hierarchy}.rst"
            output_path.write_text(self._render_page(page), encoding="utf-8")

    def _ensure_page(
        self,
        hierarchy: str,
        name: str,
        schema_node: dict[str, Any],
        description: str = "",
    ) -> KeywordPage:
        if hierarchy in self.pages:
            return self.pages[hierarchy]
        page = KeywordPage(
            hierarchy=hierarchy,
            name=name,
            description=description or schema_node.get("description", ""),
            aliases=list(schema_node.get("x-aliases", [])),
            default=schema_node.get("default"),
            has_default="default" in schema_node and schema_node.get("default") is not None,
            arguments=self._derive_arguments(schema_node),
            json_argument_key=schema_node.get("argument"),
        )
        self.pages[hierarchy] = page
        return page

    def _build_page_children(self, parent_hierarchy: str, schema_node: dict[str, Any]) -> list[ChildRow]:
        if self._node_is_array_root(schema_node):
            return self._children_from_array_root(parent_hierarchy, schema_node)

        non_null_anyof = self.resolver.strip_null_anyof(schema_node)
        if self._is_choice_node(non_null_anyof):
            return self._children_from_choice(parent_hierarchy, non_null_anyof, False, "")

        object_schema = self._get_object_schema(schema_node)
        if object_schema is None:
            return []

        rows: list[ChildRow] = []
        required_names = set(object_schema.get("required", []))
        json_argument_key = schema_node.get("argument")
        for child_name, child_schema in object_schema.get("properties", {}).items():
            if child_name == json_argument_key:
                self.synthetic_paths.add(f"{parent_hierarchy}-{child_name}")
                continue
            child_required = child_name in required_names
            if child_schema.get("anchor") and self._is_choice_node(
                self.resolver.strip_null_anyof(child_schema)
            ):
                self.synthetic_paths.add(f"{parent_hierarchy}-{child_name}")
                rows.extend(
                    self._children_from_choice(
                        parent_hierarchy,
                        self.resolver.strip_null_anyof(child_schema),
                        child_required,
                        child_name,
                        child_schema.get("title", ""),
                        child_schema.get("description", ""),
                    )
                )
                continue

            child_hierarchy = f"{parent_hierarchy}-{child_name}"
            rows.append(
                ChildRow(
                    name=child_name,
                    hierarchy=child_hierarchy,
                    required=child_required,
                )
            )
            page = self._ensure_page(
                hierarchy=child_hierarchy,
                name=child_name,
                schema_node=child_schema,
            )
            page.children.extend(self._build_page_children(child_hierarchy, child_schema))
        return rows

    def _node_is_array_root(self, schema_node: dict[str, Any]) -> bool:
        if schema_node.get("type") == "array":
            return True
        non_null_anyof = self.resolver.strip_null_anyof(schema_node)
        return bool(
            len(non_null_anyof) == 1
            and isinstance(non_null_anyof[0], dict)
            and non_null_anyof[0].get("type") == "array"
        )

    def _children_from_array_root(self, parent_hierarchy: str, schema_node: dict[str, Any]) -> list[ChildRow]:
        array_schema = schema_node
        if schema_node.get("type") != "array":
            non_null_anyof = self.resolver.strip_null_anyof(schema_node)
            array_schema = non_null_anyof[0]
        items = array_schema.get("items", {})
        if "anyOf" in items:
            return self._children_from_choice(parent_hierarchy, items["anyOf"], True, "")
        if "$ref" in items:
            return self._children_from_referenced_object(parent_hierarchy, items)
        return []

    def _children_from_referenced_object(
        self, parent_hierarchy: str, schema_node: dict[str, Any]
    ) -> list[ChildRow]:
        object_schema = self._get_object_schema(schema_node)
        if object_schema is None:
            return []
        rows: list[ChildRow] = []
        required_names = set(object_schema.get("required", []))
        json_argument_key = schema_node.get("argument")
        for child_name, child_schema in object_schema.get("properties", {}).items():
            if child_name == json_argument_key:
                self.synthetic_paths.add(f"{parent_hierarchy}-{child_name}")
                continue
            child_required = child_name in required_names
            if child_schema.get("anchor") and self._is_choice_node(
                self.resolver.strip_null_anyof(child_schema)
            ):
                self.synthetic_paths.add(f"{parent_hierarchy}-{child_name}")
                rows.extend(
                    self._children_from_choice(
                        parent_hierarchy,
                        self.resolver.strip_null_anyof(child_schema),
                        child_required,
                        child_name,
                        child_schema.get("title", ""),
                        child_schema.get("description", ""),
                    )
                )
                continue
            child_hierarchy = f"{parent_hierarchy}-{child_name}"
            rows.append(
                ChildRow(
                    name=child_name,
                    hierarchy=child_hierarchy,
                    required=child_required,
                )
            )
            page = self._ensure_page(
                hierarchy=child_hierarchy,
                name=child_name,
                schema_node=child_schema,
            )
            page.children.extend(self._build_page_children(child_hierarchy, child_schema))
        return rows

    def _children_from_choice(
        self,
        parent_hierarchy: str,
        alternatives: list[dict[str, Any]],
        required: bool,
        json_group_key: str,
        group_name: str = "",
        group_description: str = "",
    ) -> list[ChildRow]:
        rows: list[ChildRow] = []
        for alternative in alternatives:
            if not isinstance(alternative, dict) or alternative.get("type") == "null":
                continue
            flattened = self._flatten_choice_alternative(alternative)
            if flattened is None:
                continue
            child_name, child_schema = flattened
            child_hierarchy = f"{parent_hierarchy}-{child_name}"
            rows.append(
                ChildRow(
                    name=child_name,
                    hierarchy=child_hierarchy,
                    required=required,
                    group_name=group_name,
                    group_description=group_description,
                    json_group_key=json_group_key,
                )
            )
            page = self._ensure_page(
                hierarchy=child_hierarchy,
                name=child_name,
                schema_node=child_schema,
            )
            page.children.extend(self._build_page_children(child_hierarchy, child_schema))
        return rows

    def _flatten_choice_alternative(
        self, alternative: dict[str, Any]
    ) -> tuple[str, dict[str, Any]] | None:
        node = self.resolver.dereference(alternative)
        properties = node.get("properties", {})
        if len(properties) == 1:
            child_name, child_schema = next(iter(properties.items()))
            return child_name, child_schema
        return None

    def _is_choice_node(self, non_null_anyof: list[dict[str, Any]]) -> bool:
        flattened_count = sum(
            1 for entry in non_null_anyof if self._flatten_choice_alternative(entry) is not None
        )
        return flattened_count >= 2

    def _get_object_schema(self, schema_node: dict[str, Any]) -> dict[str, Any] | None:
        node = schema_node
        if "$ref" in node:
            node = self.resolver.resolve_ref(node["$ref"])
        elif "anyOf" in node:
            non_null_anyof = self.resolver.strip_null_anyof(node)
            if len(non_null_anyof) == 1 and "$ref" in non_null_anyof[0]:
                node = self.resolver.resolve_ref(non_null_anyof[0]["$ref"])
            elif len(non_null_anyof) == 1:
                node = non_null_anyof[0]
        if node.get("type") == "object":
            return node
        if "properties" in node:
            return node
        return None

    def _render_page(self, page: KeywordPage) -> str:
        metadata = self.metadata_store.entries.get(page.hierarchy, Metadata())
        title = page.name
        parts = [
            f".. _{page.hierarchy}:",
            "",
            '"' * len(title),
            title,
            '"' * len(title),
            "",
        ]

        if metadata.blurb:
            parts.extend([metadata.blurb, ""])

        if metadata.topics:
            parts.extend(["**Topics**", "", metadata.topics, ""])

        parts.extend(self._render_toc(page))
        parts.extend(self._render_specification(page))

        parts.extend(["**Description**", ""])
        description = metadata.description or page.description
        if description:
            parts.extend([description, ""])
        else:
            parts.append("")

        if metadata.examples:
            parts.extend(["**Examples**", "", metadata.examples, ""])
        if metadata.theory:
            parts.extend(["**Theory**", "", metadata.theory, ""])
        if metadata.see_also:
            parts.extend(["**See Also**", ""])
            for link in self._split_see_also(metadata.see_also):
                parts.append(f"- :ref:`{link.split('-')[-1]} <{link}>`")
            parts.append("")
        if metadata.faq:
            parts.extend(["**FAQ**", "", metadata.faq, ""])

        return "\n".join(parts).rstrip() + "\n"

    def _render_toc(self, page: KeywordPage) -> list[str]:
        if not page.children:
            return []
        lines = [".. toctree::", "   :hidden:", "   :maxdepth: 1", ""]
        seen = set()
        for child in page.children:
            if child.hierarchy in seen:
                continue
            seen.add(child.hierarchy)
            lines.append(f"   {child.hierarchy}")
        lines.append("")
        return lines

    def _render_specification(self, page: KeywordPage) -> list[str]:
        lines = ["**Specification**", ""]
        alias_text = " ".join(page.aliases) if page.aliases else "None"
        lines.append(f"- *Alias:* {alias_text}")
        lines.append(f"- *Arguments:* {page.arguments if page.arguments else 'None'}")
        lines.append(
            f"- *JSON argument key:* {page.json_argument_key if page.json_argument_key else 'None'}"
        )
        if page.has_default:
            lines.append(f"- *Default:* {self._format_default(page.default)}")
        lines.append("")
        if page.children:
            table_text, links = self._render_table(page.children)
            lines.extend(["**Child Keywords:**", "", table_text, ""])
            lines.extend(links)
        return lines

    def _render_table(self, rows: list[ChildRow]) -> tuple[str, list[str]]:
        header = [
            "Required/Optional",
            "Description of Group",
            "JSON Group Key",
            "Dakota Keyword",
            "Dakota Keyword Description",
        ]
        table_rows: list[list[str]] = [header]
        links: list[str] = []
        for row in rows:
            metadata = self.metadata_store.entries.get(row.hierarchy)
            description = ""
            if metadata and metadata.blurb:
                description = metadata.blurb.strip().replace("\n", " ")
            else:
                description = self.pages[row.hierarchy].description.strip().replace("\n", " ")
            links.append(f"{row.hierarchy}.html")
            table_rows.append(
                [
                    self._required_label(row),
                    row.group_name or "",
                    row.json_group_key or "",
                    f"`{row.name}`__",
                    description,
                ]
            )
        required_width = max(
            25,
            max(self._longest_token_length(row[0]) + 2 for row in table_rows),
        )
        group_width = max(
            24,
            max(self._longest_token_length(row[1]) + 2 for row in table_rows),
        )
        keyword_width = max(
            32,
            max(self._longest_token_length(row[3]) + 2 for row in table_rows),
        )
        description_width = max(
            60,
            max(self._longest_token_length(row[4]) + 2 for row in table_rows),
        )
        json_group_width = max(
            20,
            max(self._longest_token_length(row[2]) + 2 for row in table_rows),
        )
        widths = [
            required_width,
            group_width,
            json_group_width,
            keyword_width,
            description_width,
        ]
        link_lines = []
        if links:
            link_lines.append(".. __: " + links[0])
            for link in links[1:]:
                link_lines.append("__ " + link)
            link_lines.append("")
        return self._grid_table_with_rowspans(rows=table_rows, widths=widths), link_lines

    def _grid_table_with_rowspans(self, rows: list[list[str]], widths: list[int]) -> str:
        def divider(fill: str = "-") -> str:
            return "+" + "+".join(fill * width for width in widths) + "+"

        def wrap_cell(text: str, width: int) -> list[str]:
            if not text:
                return [""]
            wrapped = textwrap.wrap(
                text,
                width=width - 2,
                break_long_words=False,
                break_on_hyphens=False,
            )
            return wrapped or [""]

        def compute_spans(data_rows: list[list[str]]) -> list[list[int]]:
            spans = [[1] * len(widths) for _ in data_rows]
            row_count = len(data_rows)
            for col in (0, 1, 2):
                row = 0
                while row < row_count:
                    value = data_rows[row][col]
                    if not value:
                        row += 1
                        continue
                    if col == 0 and "(Choose One)" not in value:
                        row += 1
                        continue
                    if col in (1, 2) and not value:
                        row += 1
                        continue
                    span = 1
                    next_row = row + 1
                    while next_row < row_count and data_rows[next_row][col] == value:
                        span += 1
                        next_row += 1
                    if span > 1:
                        spans[row][col] = span
                        for hidden_row in range(row + 1, row + span):
                            spans[hidden_row][col] = 0
                    row = next_row
            return spans

        def row_divider(
            fill: str,
            active_spans_after_row: list[int],
            continued_columns: set[int],
        ) -> str:
            segments = []
            for col, width in enumerate(widths):
                if col in continued_columns:
                    segments.append(" " * width)
                elif active_spans_after_row[col] > 0:
                    segments.append(" " * width)
                else:
                    segments.append(fill * width)
            return "+" + "+".join(segments) + "+"

        header = rows[0]
        body_rows = rows[1:]
        spans = compute_spans(body_rows)
        active_after_row = [0] * len(widths)
        lines = [divider("-")]
        for row_index, row in enumerate([header] + body_rows):
            if row_index == 0:
                display_row = row
            else:
                body_index = row_index - 1
                display_row = [
                    "" if spans[body_index][col] == 0 else value
                    for col, value in enumerate(row)
                ]
            wrapped_cells = [
                wrap_cell(cell, width) for cell, width in zip(display_row, widths)
            ]
            height = max(len(cell_lines) for cell_lines in wrapped_cells)
            for line_index in range(height):
                rendered = []
                for cell_lines, width in zip(wrapped_cells, widths):
                    text = cell_lines[line_index] if line_index < len(cell_lines) else ""
                    rendered.append(f" {text.ljust(width - 2)} ")
                lines.append("|" + "|".join(rendered) + "|")
            if row_index == 0:
                lines.append(divider("="))
                continue

            body_index = row_index - 1
            for col in range(len(widths)):
                if spans[body_index][col] > 1:
                    active_after_row[col] = spans[body_index][col] - 1
                elif spans[body_index][col] == 0 and active_after_row[col] > 0:
                    active_after_row[col] -= 1
                else:
                    active_after_row[col] = 0

            continued_columns = {
                col for col, remaining in enumerate(active_after_row) if remaining > 0
            }
            lines.append(row_divider("-", active_after_row, continued_columns))
        return "\n".join(lines)

    def _required_label(self, row: ChildRow) -> str:
        label = "Required" if row.required else "Optional"
        if row.group_name or row.json_group_key:
            return f"{label} (Choose One)"
        return label

    def _format_default(self, value: Any) -> str:
        if isinstance(value, bool):
            return "true" if value else "false"
        if isinstance(value, list):
            return ", ".join(self._format_default(item) for item in value)
        return str(value)

    def _split_see_also(self, value: str) -> list[str]:
        links = []
        for part in value.replace("\n", ",").split(","):
            cleaned = part.strip()
            if cleaned:
                links.append(cleaned)
        return links

    def _longest_token_length(self, value: str) -> int:
        if not value:
            return 0
        return max(len(token) for token in value.split())

    def _derive_arguments(self, schema_node: dict[str, Any]) -> str:
        argument_key = schema_node.get("argument")
        if not argument_key:
            return ""
        object_schema = self._get_object_schema(schema_node)
        if object_schema is None:
            return ""
        argument_schema = object_schema.get("properties", {}).get(argument_key)
        if not isinstance(argument_schema, dict):
            return ""
        return self._format_argument_type(argument_schema)

    def _format_argument_type(self, schema_node: dict[str, Any]) -> str:
        node = schema_node
        if "anyOf" in node:
            non_null_anyof = self.resolver.strip_null_anyof(node)
            if len(non_null_anyof) == 1:
                node = non_null_anyof[0]
        node = self.resolver.dereference(node)
        node_type = node.get("type")
        if node_type == "string":
            return "STRING"
        if node_type == "integer":
            return "INTEGER"
        if node_type == "number":
            return "REAL"
        if node_type == "boolean":
            return "BOOLEAN"
        if node_type == "array":
            item_type = node.get("items", {}).get("type")
            if item_type == "string":
                return "STRING ARRAY"
            if item_type == "integer":
                return "INTEGER ARRAY"
            if item_type == "number":
                return "REAL ARRAY"
            return "ARRAY"
        return ""


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate Dakota keyword reference RST pages from dakota.json."
    )
    parser.add_argument("schema_path", type=Path)
    parser.add_argument("metadata_dir", type=Path)
    parser.add_argument("output_dir", type=Path)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    schema = json.loads(args.schema_path.read_text(encoding="utf-8"))
    metadata_store = KeywordMetadataStore(args.metadata_dir)
    generator = KeywordReferenceGenerator(schema, metadata_store)
    generator.build_pages()
    generator.validate_metadata()
    generator.write_pages(args.output_dir)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
