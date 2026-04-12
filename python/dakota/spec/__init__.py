"""Dakota specification models

For the top-level study model, import directly:
    from dakota.spec.study import DakotaStudy

This avoids circular import issues that arise from eager imports.
"""

from .base import DakotaBaseModel, DakotaField


# Lazy import for DakotaStudy to avoid circular imports
def __getattr__(name):
    if name == "DakotaStudy":
        from .study import DakotaStudy

        return DakotaStudy
    raise AttributeError(f"module {__name__!r} has no attribute {name!r}")


__all__ = ["DakotaStudy", "DakotaBaseModel"]
