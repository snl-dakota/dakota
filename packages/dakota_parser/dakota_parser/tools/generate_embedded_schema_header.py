#!/usr/bin/env python3

"""
Generate a C++ header containing a CBOR-encoded Dakota schema blob.

This keeps the parser's runtime schema asset in-memory and compiled into the
binary rather than requiring a schema file at runtime.
"""

import argparse
import json
import math
from pathlib import Path
from typing import Any


def _encode_unsigned(major_type: int, value: int) -> bytes:
    if value < 0:
        raise ValueError("CBOR unsigned encoder received negative value")

    if value < 24:
        return bytes([(major_type << 5) | value])
    if value <= 0xFF:
        return bytes([(major_type << 5) | 24, value])
    if value <= 0xFFFF:
        return bytes([(major_type << 5) | 25]) + value.to_bytes(2, "big")
    if value <= 0xFFFFFFFF:
        return bytes([(major_type << 5) | 26]) + value.to_bytes(4, "big")
    if value <= 0xFFFFFFFFFFFFFFFF:
        return bytes([(major_type << 5) | 27]) + value.to_bytes(8, "big")
    raise ValueError(f"Integer out of CBOR range: {value}")


def _encode_bytes_payload(major_type: int, payload: bytes) -> bytes:
    return _encode_unsigned(major_type, len(payload)) + payload


def _encode_float(value: float) -> bytes:
    if not math.isfinite(value):
        raise ValueError("Non-finite floats are not supported in embedded schema")

    import struct

    return bytes([0xFB]) + struct.pack(">d", value)


def encode_cbor(value: Any) -> bytes:
    if value is None:
        return b"\xF6"
    if value is False:
        return b"\xF4"
    if value is True:
        return b"\xF5"
    if isinstance(value, int):
        if value >= 0:
            return _encode_unsigned(0, value)
        return _encode_unsigned(1, -1 - value)
    if isinstance(value, float):
        return _encode_float(value)
    if isinstance(value, str):
        payload = value.encode("utf-8")
        return _encode_bytes_payload(3, payload)
    if isinstance(value, list):
        out = bytearray(_encode_unsigned(4, len(value)))
        for item in value:
            out.extend(encode_cbor(item))
        return bytes(out)
    if isinstance(value, dict):
        out = bytearray(_encode_unsigned(5, len(value)))
        for key in sorted(value.keys()):
            if not isinstance(key, str):
                raise ValueError("Only string-keyed JSON objects are supported")
            out.extend(encode_cbor(key))
            out.extend(encode_cbor(value[key]))
        return bytes(out)
    raise TypeError(f"Unsupported value for CBOR encoding: {type(value)!r}")


def format_cpp_byte_array(data: bytes) -> str:
    lines = []
    row = []
    for index, byte in enumerate(data, start=1):
        row.append(f"0x{byte:02X}")
        if index % 12 == 0:
            lines.append(", ".join(row))
            row = []
    if row:
        lines.append(", ".join(row))
    return ",\n    ".join(lines)


def generate_header(schema_path: Path, output_path: Path) -> None:
    with schema_path.open("r", encoding="utf-8") as infile:
        schema = json.load(infile)

    cbor_blob = encode_cbor(schema)
    byte_literals = format_cpp_byte_array(cbor_blob)

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(
        f"""// Auto-generated from {schema_path.name}. Do not edit.
#ifndef DAKOTA_EMBEDDED_SCHEMA_CBOR_HPP
#define DAKOTA_EMBEDDED_SCHEMA_CBOR_HPP

#include <cstddef>
#include <cstdint>

namespace dakota::embedded_schema {{

inline constexpr std::uint8_t kDakotaSchemaCbor[] = {{
    {byte_literals}
}};

inline constexpr std::size_t kDakotaSchemaCborSize = sizeof(kDakotaSchemaCbor);

}} // namespace dakota::embedded_schema

#endif // DAKOTA_EMBEDDED_SCHEMA_CBOR_HPP
""",
        encoding="utf-8",
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("schema", type=Path, help="Path to the Dakota JSON schema")
    parser.add_argument("output", type=Path, help="Path to the generated header")
    args = parser.parse_args()

    generate_header(args.schema, args.output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
