#!/usr/bin/env python3
"""Generate an expanded Dakota XML grammar from dakota.json.

This script treats the emitted Dakota JSON schema as the source of truth and
reconstructs an XML grammar that preserves the user-visible keyword hierarchy:

- keywords and aliases
- inline arguments and scalar params
- oneOf/union structure
- defaults and basic numeric constraints
- top-level repeatable blocks

Some legacy XML attributes are no longer recoverable from the JSON schema.
When needed for compatibility, this generator emits stable synthetic values:

- ``code="{generated}"`` for concrete keywords
- ``code="{0}"`` for structural wrapper keywords
- ``scenario`` on ``oneOf`` elements mirrors ``x-union-pattern`` when present,
  otherwise ``0``

The output is intentionally fully expanded and does not recreate XML entities.
"""

from __future__ import annotations

import argparse
import json
from copy import deepcopy
from dataclasses import dataclass
from pathlib import Path
from typing import Any
from xml.dom import minidom
from xml.etree import ElementTree as ET


DAKOTA_NS = "http://www.sandia.gov/dakota/1.0"
XSI_NS = "http://www.w3.org/2001/XMLSchema-instance"
SCHEMA_LOCATION = "http://www.sandia.gov/dakota/1.0 dakota.xsd"
MAX_OCCURS_UNBOUNDED = "999999"
TOP_LEVEL_BLOCKS = ("environment", "method", "model", "variables", "responses", "interface")


ET.register_namespace("", DAKOTA_NS)
ET.register_namespace("xsi", XSI_NS)


def ns(tag: str) -> str:
    return f"{{{DAKOTA_NS}}}{tag}"


@dataclass
class ChoiceAlternative:
    name: str
    schema: dict[str, Any]
    target_ref_name: str | None = None
    wrapper_ref_name: str | None = None


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

    def get_object_schema(self, node: dict[str, Any]) -> dict[str, Any] | None:
        resolved = self._resolve_singleton_node(node)
        if resolved.get("type") == "object" or "properties" in resolved:
            return resolved
        return None

    def get_array_schema(self, node: dict[str, Any]) -> dict[str, Any] | None:
        resolved = self._resolve_singleton_node(node)
        if resolved.get("type") == "array":
            return resolved
        return None

    def _resolve_singleton_node(self, node: dict[str, Any]) -> dict[str, Any]:
        current = node
        if "$ref" in current:
            current = self.resolve_ref(current["$ref"])
        elif "anyOf" in current:
            non_null = self.strip_null_anyof(current)
            if len(non_null) == 1:
                current = non_null[0]
                if "$ref" in current:
                    current = self.resolve_ref(current["$ref"])
        return current

    def flatten_choice_alternative(
        self, alternative: dict[str, Any]
    ) -> ChoiceAlternative | None:
        wrapper_ref_name = alternative["$ref"].split("/")[-1] if "$ref" in alternative else None
        wrapper = self.dereference(alternative)
        properties = wrapper.get("properties", {})
        if len(properties) != 1:
            return None

        child_name, child_schema = next(iter(properties.items()))
        merged = deepcopy(child_schema)
        for key in ("x-aliases", "x-materialization", "description", "title", "default", "argument"):
            if key in wrapper and key not in merged:
                merged[key] = wrapper[key]

        target_ref_name = None
        if isinstance(child_schema, dict) and "$ref" in child_schema:
            target_ref_name = child_schema["$ref"].split("/")[-1]

        return ChoiceAlternative(
            name=child_name,
            schema=merged,
            target_ref_name=target_ref_name,
            wrapper_ref_name=wrapper_ref_name,
        )

    def is_choice_node(self, node: dict[str, Any]) -> bool:
        non_null = self.strip_null_anyof(node)
        count = 0
        for entry in non_null:
            if self.flatten_choice_alternative(entry) is not None:
                count += 1
        return count >= 2

    def is_nullable(self, node: dict[str, Any]) -> bool:
        return any(
            isinstance(entry, dict) and entry.get("type") == "null"
            for entry in node.get("anyOf", [])
        )

    def find_argument_schema(self, node: dict[str, Any]) -> dict[str, Any] | None:
        argument_name = node.get("argument")
        if not argument_name:
            return None
        object_schema = self.get_object_schema(node)
        if object_schema is None:
            return None
        return object_schema.get("properties", {}).get(argument_name)

    def computed_field_names(self, node: dict[str, Any]) -> set[str]:
        object_schema = self.get_object_schema(node)
        if object_schema is None:
            return set()
        computed = object_schema.get("x-computed-fields", {})
        if isinstance(computed, dict):
            return set(computed)
        return set()

    def is_internal_only(self, node: dict[str, Any]) -> bool:
        if node.get("x-internal-only"):
            return True
        resolved = self._resolve_singleton_node(node)
        return bool(resolved.get("x-internal-only"))


class DakotaXmlGenerator:
    def __init__(self, schema: dict[str, Any]) -> None:
        self.schema = schema
        self.resolver = SchemaResolver(schema)

    def build_tree(self) -> ET.ElementTree:
        root = ET.Element(
            ns("document"),
            {
                f"{{{XSI_NS}}}schemaLocation": SCHEMA_LOCATION,
            },
        )
        root.append(
            ET.Comment(
                "Generated from dakota.json. code and scenario attributes are synthetic."
            )
        )
        input_elem = ET.SubElement(root, ns("input"))
        required_top = set(self.schema.get("required", []))

        for name in TOP_LEVEL_BLOCKS:
            field_schema = self.schema["properties"][name]
            if name == "environment":
                input_elem.append(
                    self.build_keyword(
                        name=name,
                        schema_node=field_schema,
                        required=name in required_top,
                        path=(name,),
                        max_occurs="1",
                    )
                )
                continue

            if name in {"method", "model", "variables", "responses", "interface"}:
                input_elem.append(
                    self.build_keyword(
                        name=name,
                        schema_node=field_schema,
                        required=name in required_top,
                        path=(name,),
                        max_occurs=MAX_OCCURS_UNBOUNDED,
                    )
                )
                continue

        return ET.ElementTree(root)

    def build_keyword(
        self,
        *,
        name: str,
        schema_node: dict[str, Any],
        required: bool,
        path: tuple[str, ...],
        max_occurs: str = "1",
    ) -> ET.Element:
        keyword = ET.Element(ns("keyword"))
        keyword.set("name", name)
        keyword.set("id", "_".join(path))
        keyword.set("code", self._keyword_code(schema_node))
        keyword.set("label", self._keyword_label(schema_node, name))
        keyword.set("maxOccurs", max_occurs)
        if not required:
            keyword.set("minOccurs", "0")

        aliases = schema_node.get("x-aliases", [])
        if isinstance(aliases, str):
            aliases = [aliases]
        for alias in aliases:
            alias_elem = ET.SubElement(keyword, ns("alias"))
            alias_elem.set("name", alias)

        argument_name = schema_node.get("argument")
        if argument_name:
            keyword.set("argument", argument_name)
            argument_schema = self.resolver.find_argument_schema(schema_node)
            if argument_schema is not None:
                keyword.append(self.build_param(argument_schema))
                self._propagate_default(keyword, argument_schema)
        elif self._node_has_direct_param(schema_node):
            keyword.append(self.build_param(schema_node))
            self._propagate_default(keyword, schema_node)

        self._append_children(keyword, schema_node, path)
        return keyword

    def build_param(self, schema_node: dict[str, Any]) -> ET.Element:
        param = ET.Element(ns("param"))
        param_type = self._param_type(schema_node)
        if param_type is None:
            raise ValueError(f"Unable to derive Dakota param type from schema node: {schema_node}")
        param.set("type", param_type)
        constraint = self._constraint_string(schema_node)
        if constraint:
            param.set("constraint", constraint)
        default = self._default_string(schema_node)
        if default is not None:
            param.set("default", default)
        return param

    def build_choice(
        self,
        *,
        schema_node: dict[str, Any],
        path: tuple[str, ...],
    ) -> ET.Element:
        choice = ET.Element(ns("oneOf"))
        union_pattern = schema_node.get("x-union-pattern")
        choice.set("scenario", str(union_pattern if union_pattern is not None else 0))
        if union_pattern is not None:
            choice.set("union_pattern", str(union_pattern))

        label = schema_node.get("title") or schema_node.get("description")
        if label:
            choice.set("label", label)

        if schema_node.get("anchor"):
            choice.set("anchor", path[-1])

        default_branch = self._default_branch_name(schema_node)
        choice.set("default_branch", default_branch if default_branch else "None")

        for entry in self.resolver.strip_null_anyof(schema_node):
            flattened = self.resolver.flatten_choice_alternative(entry)
            if flattened is None or self.resolver.is_internal_only(flattened.schema):
                continue
            if schema_node.get("anchor"):
                child_path = path[:-1] + (flattened.name,)
            else:
                child_path = path + (flattened.name,)
            choice.append(
                self.build_keyword(
                    name=flattened.name,
                    schema_node=flattened.schema,
                    required=True,
                    path=child_path,
                )
            )
        return choice

    def _append_children(
        self, keyword: ET.Element, schema_node: dict[str, Any], path: tuple[str, ...]
    ) -> None:
        array_schema = self.resolver.get_array_schema(schema_node)
        if array_schema is not None:
            self._append_children_from_array(keyword, array_schema, path)
            return

        if self.resolver.is_choice_node(schema_node):
            keyword.append(self.build_choice(schema_node=schema_node, path=path))
            return

        object_schema = self.resolver.get_object_schema(schema_node)
        if object_schema is None:
            return

        required_names = set(object_schema.get("required", []))
        computed_names = self.resolver.computed_field_names(schema_node)
        argument_name = schema_node.get("argument")
        for child_name, child_schema in object_schema.get("properties", {}).items():
            if child_name in computed_names or child_name == argument_name:
                continue
            if self.resolver.is_internal_only(child_schema):
                continue

            child_path = path + (child_name,)
            child_required = child_name in required_names

            if child_schema.get("anchor") and self.resolver.is_choice_node(child_schema):
                group_tag = "required" if child_required else "optional"
                group = ET.SubElement(keyword, ns(group_tag))
                group.append(self.build_choice(schema_node=child_schema, path=child_path))
                continue

            child_keyword = self.build_keyword(
                name=child_name,
                schema_node=child_schema,
                required=child_required,
                path=child_path,
            )
            keyword.append(child_keyword)

    def _append_children_from_array(
        self, keyword: ET.Element, array_schema: dict[str, Any], path: tuple[str, ...]
    ) -> None:
        items = array_schema.get("items", {})
        if "anyOf" in items:
            keyword.append(self.build_choice(schema_node=items, path=path + ("choice",)))
            return

        object_schema = self.resolver.get_object_schema(items)
        if object_schema is None:
            return

        required_names = set(object_schema.get("required", []))
        computed_names = self.resolver.computed_field_names(items)
        argument_name = items.get("argument")
        for child_name, child_schema in object_schema.get("properties", {}).items():
            if child_name in computed_names or child_name == argument_name:
                continue
            if self.resolver.is_internal_only(child_schema):
                continue
            child_path = path + (child_name,)
            child_required = child_name in required_names
            if child_schema.get("anchor") and self.resolver.is_choice_node(child_schema):
                group_tag = "required" if child_required else "optional"
                group = ET.SubElement(keyword, ns(group_tag))
                group.append(self.build_choice(schema_node=child_schema, path=child_path))
                continue
            keyword.append(
                self.build_keyword(
                    name=child_name,
                    schema_node=child_schema,
                    required=child_required,
                    path=child_path,
                )
            )

    def _keyword_label(self, schema_node: dict[str, Any], name: str) -> str:
        resolved = self.resolver._resolve_singleton_node(schema_node)
        direct_description = schema_node.get("description")
        if direct_description and not direct_description.startswith("Generated model for "):
            return direct_description
        if schema_node.get("title") and not direct_description:
            return schema_node["title"]
        return resolved.get("description") or resolved.get("title") or name

    def _keyword_code(self, schema_node: dict[str, Any]) -> str:
        if self._node_has_direct_materialization(schema_node) or self._node_has_direct_param(schema_node):
            return "{generated}"
        return "{0}"

    def _node_has_direct_materialization(self, schema_node: dict[str, Any]) -> bool:
        return isinstance(schema_node.get("x-materialization"), list)

    def _node_has_direct_param(self, schema_node: dict[str, Any]) -> bool:
        if schema_node.get("argument"):
            return False
        if self.resolver.is_choice_node(schema_node):
            return False
        object_schema = self.resolver.get_object_schema(schema_node)
        if object_schema is not None:
            return False
        return self._param_type(schema_node) is not None

    def _default_string(self, schema_node: dict[str, Any]) -> str | None:
        if "default" not in schema_node:
            return None
        default = schema_node["default"]
        if default is None:
            return None
        if isinstance(default, bool):
            return "true" if default else "false"
        if isinstance(default, (int, float, str)):
            return str(default)
        return json.dumps(default, separators=(", ", ": "))

    def _propagate_default(self, keyword: ET.Element, schema_node: dict[str, Any]) -> None:
        default = self._default_string(schema_node)
        if default is not None:
            keyword.set("default", default)

    def _default_branch_name(self, schema_node: dict[str, Any]) -> str | None:
        model_default = schema_node.get("x-model-default")
        if not model_default:
            return None

        for entry in self.resolver.strip_null_anyof(schema_node):
            flattened = self.resolver.flatten_choice_alternative(entry)
            if flattened is None:
                continue
            if flattened.wrapper_ref_name == model_default:
                return flattened.name
            if flattened.target_ref_name == model_default:
                return flattened.name
            resolved = self.resolver.dereference(flattened.schema)
            if resolved.get("title") == model_default:
                return flattened.name
        return None

    def _constraint_string(self, schema_node: dict[str, Any]) -> str | None:
        node = self.resolver._resolve_singleton_node(schema_node)
        if "exclusiveMinimum" in node:
            return f"> {node['exclusiveMinimum']}"
        if "minimum" in node:
            return f">= {node['minimum']}"
        if "exclusiveMaximum" in node:
            return f"< {node['exclusiveMaximum']}"
        if "maximum" in node:
            return f"<= {node['maximum']}"
        return None

    def _param_type(self, schema_node: dict[str, Any]) -> str | None:
        node = self.resolver._resolve_singleton_node(schema_node)

        if "const" in node and node.get("type") == "boolean":
            return None

        node_type = node.get("type")
        if node_type == "integer":
            return "INTEGER"
        if node_type == "number":
            return "REAL"
        if node_type == "string":
            return self._file_like_type(schema_node)
        if node_type == "array":
            items = node.get("items", {})
            item_type = self.resolver._resolve_singleton_node(items).get("type")
            if item_type == "integer":
                return "INTEGERLIST"
            if item_type == "number":
                return "REALLIST"
            if item_type == "string":
                return "STRINGLIST"
        return None

    def _file_like_type(self, schema_node: dict[str, Any]) -> str:
        name_hints = {
            "input_file": "INPUT_FILE",
            "output_file": "OUTPUT_FILE",
            "file_name": "OUTPUT_FILE",
            "filename": "OUTPUT_FILE",
        }
        argument_name = schema_node.get("argument")
        if argument_name and argument_name in name_hints:
            return name_hints[argument_name]
        return "STRING"


def prettify_xml(tree: ET.ElementTree) -> str:
    raw = ET.tostring(tree.getroot(), encoding="utf-8")
    parsed = minidom.parseString(raw)
    return parsed.toprettyxml(indent="  ", encoding="UTF-8").decode("utf-8")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--schema",
        type=Path,
        default=Path(__file__).resolve().parents[1] / "dakota.json",
        help="Path to dakota.json",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=Path(__file__).resolve().parents[1] / "dakota.xml",
        help="Path to write dakota.xml",
    )
    parser.add_argument(
        "--stdout",
        action="store_true",
        help="Write XML to stdout instead of updating the output file",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    schema = json.loads(args.schema.read_text(encoding="utf-8"))
    generator = DakotaXmlGenerator(schema)
    tree = generator.build_tree()
    xml_text = prettify_xml(tree)
    ET.fromstring(xml_text)

    if args.stdout:
        print(xml_text, end="")
    else:
        args.output.write_text(xml_text, encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
