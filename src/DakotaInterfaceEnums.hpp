#pragma once

#include "dakota_global_defs.hpp"

namespace Dakota {

// Offset for external process interface types.
enum : unsigned short { PROCESS_INTERFACE_BIT = 8 };
// Offset for direct coupled interface types.
enum : unsigned short { DIRECT_INTERFACE_BIT = 16 };

// Special values for interface type.
#define DAKOTA_INTERFACE_TYPE_ENUMS \
  X(DEFAULT_INTERFACE, 0) X(APPROX_INTERFACE, 1) \
  X(FORK_INTERFACE, 8) X(SYSTEM_INTERFACE, 9) X(GRID_INTERFACE, 10) \
  X(TEST_INTERFACE, 16) X(PLUGIN_INTERFACE, 17) \
  X(MATLAB_INTERFACE, 18) X(PYTHON_INTERFACE, 19) X(SCILAB_INTERFACE, 20)

#define X(name, value) name = value,
enum : unsigned short {
  DAKOTA_INTERFACE_TYPE_ENUMS
};
#undef X

#define X(name, value) REGISTER_DAKOTA_ENUM(name, value)
DAKOTA_INTERFACE_TYPE_ENUMS
#undef X

#undef DAKOTA_INTERFACE_TYPE_ENUMS

} // namespace Dakota
