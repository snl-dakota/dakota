# ROLOptimizerCompatibility Note

## Status: NOT IN USE

The files `ROLOptimizerCompatibility.hpp` and `ROLOptimizerCompatibility.cpp` are **not currently being used** in the build.

## Why Not Used?

The new `DakotaROLOptimizer` implementation provides a direct `Dakota::ROLOptimizer` class that:
- Inherits from `Dakota::Optimizer`
- Has the **same public API** as the old `ROLOptimizer`
- Works as a drop-in replacement

Therefore, no compatibility layer is needed. The new class can be used directly wherever the old one was used.

## History

These compatibility files were initially created to provide a factory-based interface using:
- `rol_interface::OptimizerInterface` (abstract interface)
- `rol_interface::OptimizerFactory` (factory methods)

However, this design was abandoned in favor of a simpler direct implementation approach.

## Current Situation

- **Removed from build**: `ROLOptimizerCompatibility.cpp` removed from CMakeLists.txt
- **Not included**: `ROLOptimizerCompatibility.hpp` not included by any active code
- **No compilation errors**: Files don't compile because they reference non-existent types

## Future Action

These files can be:
1. **Left as-is**: They're not causing problems since they're not in the build
2. **Deleted**: They're not needed and won't be used
3. **Archived**: Kept for historical reference

**Recommendation**: Delete these files in a future cleanup pass.
