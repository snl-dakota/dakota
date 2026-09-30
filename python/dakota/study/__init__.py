#  _______________________________________________________________________
#
#    Dakota: Explore and predict with confidence.
#    Copyright 2014-2025
#    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#    This software is distributed under the GNU Lesser General Public License.
#    For more information, see the README file in the top Dakota directory.
#    _______________________________________________________________________

"""
Dependency-injection/library-mode Dakota study construction.
"""

from ._study import (  # noqa: F401
    ConcurrentMetaIterator,
    DataFitSurrModel,
    EnsembleSurrModel,
    Interface,
    Iterator,
    MethodFactory,
    Model,
    ModelFactory,
    NestedModel,
    NonDGlobalSingleInterval,
    NonDLHSSampling,
    NonDLHSSingleInterval,
    NonDLocalSingleInterval,
    ParamStudy,
    RichExtrapVerification,
    Response,
    SimulationModel,
    Study,
    StudyConfig,
    StudyOutputConfig,
    StudyRunConfig,
    Variables,
)

try:
    from ._study import DOTOptimizer  # noqa: F401
except ImportError:
    pass

try:
    from ._study import EffGlobalMinimizer  # noqa: F401
except ImportError:
    pass

try:
    from ._study import NPSOLOptimizer  # noqa: F401
except ImportError:
    pass

try:
    from ._study import NL2SOLLeastSq  # noqa: F401
except ImportError:
    pass

# Optional iterator types are exported only when their native libraries are built.
from . import _study as _native
from ._iterator_bindings import ITERATOR_TYPES as _iterator_types

for _type_name in _iterator_types:
    if hasattr(_native, _type_name):
        globals()[_type_name] = getattr(_native, _type_name)
del _native, _iterator_types, _type_name
