"""Base class for all Dakota method configurations"""

from ..base import DakotaBaseModel
from typing import ClassVar


class MethodSelection(DakotaBaseModel):
    """Base class for method discriminator wrappers

    Wrappers auto-register via get_registry()/get_union().
    Must end with 'Selection' and have exactly one field.
    """

    _registry: ClassVar[dict[str, type["MethodSelection"]]] = {}

    def __init_subclass__(cls, **kwargs):
        super().__init_subclass__(**kwargs)
        if cls.__name__ == "MethodSelection":
            return
        if not cls.__name__.endswith("Selection"):
            raise TypeError(f"{cls.__name__} must end with 'Selection'")

    @classmethod
    def get_registry(cls) -> dict[str, type["MethodSelection"]]:
        """Get registry, performing deferred registration on first call"""
        if not cls._registry:
            for subclass in cls.__subclasses__():
                if subclass.__name__ == "MethodSelection":
                    continue
                fields = list(subclass.model_fields.keys())
                if len(fields) == 1:
                    cls._registry[fields[0]] = subclass
        return cls._registry.copy()

    @classmethod
    def get_union(cls):
        """Generate Union from all registered selections"""
        cls.get_registry()
        from typing import Union

        return Union[tuple(cls._registry.values())]
