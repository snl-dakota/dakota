#pragma once

#include "dakota_data_types.hpp"
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

/// Interface synchronization modes.
enum { SYNCHRONOUS_INTERFACE, ASYNCHRONOUS_INTERFACE };

/// Algebraic function categories.
enum { OBJECTIVE, INEQUALITY_CONSTRAINT, EQUALITY_CONSTRAINT };

inline String interface_enum_to_string(unsigned short interface_type)
{
  switch (interface_type) {
  case DEFAULT_INTERFACE: return String("default");
  case APPROX_INTERFACE:  return String("approximation");
  case FORK_INTERFACE:    return String("fork");
  case SYSTEM_INTERFACE:  return String("system");
  case GRID_INTERFACE:    return String("grid");
  case TEST_INTERFACE:    return String("direct");
  case MATLAB_INTERFACE:  return String("matlab");
  case PYTHON_INTERFACE:  return String("pybind11");
  case SCILAB_INTERFACE:  return String("scilab");
  default:
    Cerr << "\nError: Unknown interface enum " << interface_type << std::endl;
    abort_handler(-1);
    return String();
  }
}

} // namespace Dakota
