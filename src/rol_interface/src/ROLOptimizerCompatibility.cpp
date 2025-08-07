#include "ROLOptimizerCompatibility.hpp"

// This file provides the implementation for the compatibility layer.
// All the actual functionality is implemented in rol_interface::Optimizer,
// so we just need to ensure the symbols exist for linking.

// Note: The constructors are defined inline in the header file,
// so this file mainly ensures the vtable and typeinfo are generated.