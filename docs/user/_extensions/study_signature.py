"""Signature presentation helpers for the public ``dakota.study`` API."""

from __future__ import annotations

import re


_PRIVATE_STUDY_MODULE = "dakota.study._study."


def normalize_study_signature(app, what, name, obj, options, signature,
                              return_annotation):
    """Present pybind11 Study signatures as ordinary public Python APIs."""
    if not name.startswith("dakota.study."):
        return None

    if signature:
        signature = signature.replace(_PRIVATE_STUDY_MODULE, "dakota.study.")
        if what == "method":
            signature = re.sub(
                r"^\(self(?:\s*:\s*[^,)=]+)?(?:,\s*)?", "(", signature)
    if return_annotation:
        return_annotation = return_annotation.replace(
            _PRIVATE_STUDY_MODULE, "dakota.study.")
    return signature, return_annotation
