#!/usr/bin/env python3
"""Portable test-harness entrypoint for Dakota DSL -> JSON conversion.

This wrapper lives in the test directory so the regression harness can invoke a
stable, repo-relative script. It forwards to the main converter implementation
under ``dsl_to_json/dsl_to_json.py`` and preconfigures a generated Dakota
package root when one is available.
"""

from __future__ import annotations

import importlib.util
import os
import sys
from pathlib import Path


def _load_converter_main():
    repo_root = Path(__file__).resolve().parents[3]

    # The DAKOTA_PYTHON_PACKAGE_DIR environment variable must be set externally
    # to point to the location of the Pydantic models package
    # This allows flexibility for different development environments
    package_root = os.environ.get("DAKOTA_PYTHON_PACKAGE_DIR")
    if not package_root:
        raise RuntimeError(
            "DAKOTA_PYTHON_PACKAGE_DIR environment variable not set. "
            "Please set it to the directory containing the Pydantic models package "
            "(e.g., /path/to/model_generation/build/dakota_package)"
        )

    converter_path = repo_root / "dsl_to_json" / "dsl_to_json.py"
    spec = importlib.util.spec_from_file_location("dakota_dsl_to_json_impl", converter_path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Could not load converter from {converter_path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module.main


def main():
    converter_main = _load_converter_main()
    converter_main()


if __name__ == "__main__":
    main()
