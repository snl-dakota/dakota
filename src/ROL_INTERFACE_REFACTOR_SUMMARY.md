# Dakota ROL Interface Refactoring Summary

## Objective
Eliminate circular dependencies between `rol_interface/include` and `dakota/src` by integrating ROL interface source files directly into the `dakota_src` library while maintaining the physical file organization in the `rol_interface` subdirectory.

## Changes Made

### 1. Main CMakeLists.txt (src/CMakeLists.txt)

#### Lines 795-811: Added ROL interface sources to dakota_src
- Modified the `HAVE_ROL` block to include ROL interface source files directly in `iterator_src`
- Added all `.cpp` files from `rol_interface/src/`:
  - `rol_interface/src/DakotaROLObjective.cpp`
  - `rol_interface/src/DakotaROLConstraint.cpp`
  - `rol_interface/src/DakotaROLVector.cpp`
  - `rol_interface/src/DakotaROLOptimizer.cpp`
  - `rol_interface/src/ROLOptimizerCompatibility.cpp`
- Added `rol_interface/include` to `DAKOTA_INCDIRS` to make headers available
- **Removed obsolete reference** to non-existent `DakotaROLOptimizerNew.cpp`

#### Lines 938-961: Updated dakota_src library configuration
- Kept Teuchos include paths (needed for ROL compatibility)
- Added comment explaining the integration approach
- Added `add_subdirectory(rol_interface)` back to enable test building
- ROL libraries are linked directly to `dakota_src` via `target_link_libraries(dakota_src ${ROL_LIBRARIES})`

### 2. ROL Interface CMakeLists.txt (src/rol_interface/CMakeLists.txt)

**Completely rewrote** this file to:
- Remove the separate `rol_interface` library target
- Add documentation explaining the integrated build approach
- Only handle the test subdirectory when testing is enabled
- Source files are no longer compiled here (compiled in main dakota_src)

**Old approach:**
```cmake
add_library(rol_interface STATIC ...)
target_include_directories(rol_interface ...)
target_link_libraries(rol_interface ...)
```

**New approach:**
```cmake
# Documentation only - sources built in dakota_src
if(DAKOTA_ENABLE_TESTS OR BUILD_TESTING)
  add_subdirectory(test)
endif()
```

### 3. Test CMakeLists.txt (src/unit/dakota_rol/CMakeLists.txt)

**Updated test include paths** to explicitly add `../../rol_interface/include`:
- All test targets now include the ROL interface headers from their new location
- Tests use `LINK_DAKOTA_LIBS` which includes the ROL interface code via `dakota_src`

**Note**: The duplicate test directory `src/rol_interface/test/` is no longer used (tests moved to standard location `src/unit/dakota_rol/`)

## Benefits

1. **Eliminates Circular Dependencies**:
   - No more circular dependency between `rol_interface` library and `dakota_src`
   - ROL interface headers can freely include Dakota headers and vice versa

2. **Maintains Organization**:
   - Files remain in `rol_interface/` subdirectory
   - Logical separation is preserved
   - No cluttering of the already-crowded `src/` directory

3. **Simplifies Build**:
   - One less library target to manage
   - Clearer dependency chain
   - Faster incremental builds

4. **Consistent with Dakota Patterns**:
   - Similar to how other Dakota components are organized
   - Follows the pattern used for `iterator_src`, `interface_src`, etc.

## File Locations

All files remain in their original locations:

```
dakota/src/
├── CMakeLists.txt                           (modified)
├── rol_interface/
│   ├── CMakeLists.txt                       (replaced/simplified)
│   ├── include/
│   │   ├── DakotaROLObjective.hpp          (unchanged)
│   │   ├── DakotaROLConstraint.hpp         (unchanged)
│   │   ├── DakotaROLVector.hpp             (unchanged)
│   │   ├── DakotaROLOptimizer.hpp          (unchanged)
│   │   └── ...                              (unchanged)
│   ├── src/
│   │   ├── DakotaROLObjective.cpp          (unchanged)
│   │   ├── DakotaROLConstraint.cpp         (unchanged)
│   │   ├── DakotaROLVector.cpp             (unchanged)
│   │   ├── DakotaROLOptimizer.cpp          (unchanged)
│   │   └── ...                              (unchanged)
│   └── test/
│       ├── CMakeLists.txt                   (unchanged)
│       └── *.cpp                            (unchanged)
```

## Build Verification

To verify the refactoring:

1. **Clean build recommended**:
   ```bash
   cd build
   rm -rf *
   cmake ..
   ```

2. **Check for ROL interface integration**:
   Look for the message: "ROL Interface: Sources integrated into dakota_src (no separate library)"

3. **Verify no circular dependencies**:
   - Build should not fail with undefined references
   - No complaints about missing `rol_interface` target

4. **Run tests**:
   ```bash
   ctest -L ROL
   ```

## Troubleshooting

### Common Issues

1. **Duplicate test targets error**:
   - Cause: Tests being built from both `rol_interface/test/` and `unit/dakota_rol/`
   - Fix: Tests should only be built from `unit/dakota_rol/` (already fixed in this refactoring)

2. **Cannot find source file error**:
   - Cause: Reference to obsolete `DakotaROLOptimizerNew.cpp`
   - Fix: Removed from CMakeLists.txt (already fixed in this refactoring)

3. **ROL interface headers not found**:
   - Ensure `rol_interface/include` is in `DAKOTA_INCDIRS`
   - Test targets should include `../../rol_interface/include`

4. **Circular dependency warnings**:
   - Should be eliminated by this refactoring
   - If still present, verify no separate `rol_interface` library is being built

## Rollback Plan

If needed, the original `rol_interface/CMakeLists.txt` can be restored from version control.

To rollback:
1. Restore `rol_interface/CMakeLists.txt` from git
2. Revert changes to main `CMakeLists.txt` and `unit/dakota_rol/CMakeLists.txt`
3. Re-run CMake configuration
