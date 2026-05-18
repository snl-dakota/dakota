"""Generated Pydantic models for method.external_python"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.misc import MethodThreeOptionalKeywordsMixin


class ExternalPythonConfig(MethodThreeOptionalKeywordsMixin):
    "Use the external python methods interface"

    class_path_and_name: str = DakotaField(
        description="Specify the module and class name of the external python method",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.class_path_and_name",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    options_file: str | None = DakotaField(
        default=None,
        description="Filename to pass to exteral python methods",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.advanced_options_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ExternalPythonSelection(MethodSelection):
    "Generated model for ExternalPythonSelection"

    external_python: ExternalPythonConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "EXTERNAL_PYTHON",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
