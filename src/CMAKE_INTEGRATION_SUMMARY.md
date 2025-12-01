# CMake Integration Summary for ROL Interface

## Current Configuration (CORRECT ✅)

The ROL interface is **integrated directly into `dakota_src`** with no separate library, avoiding circular dependencies.

### Key Points

1. **No Separate Library**
   - `rol_interface` sources compiled directly into `dakota_src.so`
   - No `add_library(rol_interface ...)` call
   - No circular dependency issues

2. **Source Files Added to iterator_src**
   ```cmake
   set(iterator_src ${iterator_src}
       rol_interface/src/DakotaROLObjective.cpp
       rol_interface/src/DakotaROLConstraint.cpp
       rol_interface/src/DakotaROLVector.cpp
       rol_interface/src/DakotaROLOptimizer.cpp)
   ```

3. **Include Directory Added to DAKOTA_INCDIRS**
   ```cmake
   list(APPEND DAKOTA_INCDIRS ${Dakota_SOURCE_DIR}/src/rol_interface/include)
   ```

4. **Preprocessor Definitions Set**
   ```cmake
   add_definitions("-DHAVE_ROL")
   add_definitions("-DHAVE_DAKOTA_ROL_INTERFACE")
   ```

### File Organization

```
dakota/src/
├── CMakeLists.txt              ← Adds rol_interface sources to iterator_src
├── iterator_utils.cpp          ← Uses ROLOptimizer (new or old based on flags)
├── ROLOptimizer.hpp            ← Old monolithic implementation
├── ROLOptimizer.cpp            ← Old monolithic implementation
└── rol_interface/              ← Subdirectory organization (not a library!)
    ├── CMakeLists.txt          ← Documentation only, no library build
    ├── include/
    │   ├── DakotaROLOptimizer.hpp
    │   ├── DakotaROLObjective.hpp
    │   ├── DakotaROLConstraint.hpp
    │   └── ...
    └── src/
        ├── DakotaROLOptimizer.cpp
        ├── DakotaROLObjective.cpp
        ├── DakotaROLConstraint.cpp
        └── ...
```

### How It Works

1. **When `HAVE_ROL` is true:**
   - Adds `-DHAVE_ROL` flag
   - Adds `-DHAVE_DAKOTA_ROL_INTERFACE` flag
   - Adds rol_interface source files to `iterator_src`
   - Adds rol_interface include directory to `DAKOTA_INCDIRS`

2. **In `iterator_utils.cpp`:**
   ```cpp
   #ifdef HAVE_ROL
   #if defined(HAVE_DAKOTA_ROL_INTERFACE)
   #include "DakotaROLOptimizer.hpp"  // New modular implementation
   #else
   #include "ROLOptimizer.hpp"        // Old monolithic implementation
   #endif
   #endif
   ```

3. **At link time:**
   - All sources compiled into `dakota_src.so`
   - No separate `librol_interface` needed
   - No circular dependency possible

### Benefits

✅ **No Circular Dependencies**
- ROL interface can include Dakota headers freely
- Dakota code can include ROL interface headers freely
- All compiled into one library

✅ **Clean Organization**
- Sources stay in `rol_interface/` subdirectory
- Logical separation maintained
- Not cluttering main `src/` directory

✅ **Simpler Build**
- One less library target
- Faster builds
- Clearer dependency chain

✅ **Backward Compatible**
- Old implementation still available
- Controlled by preprocessor flag
- Easy to switch between versions

### What NOT to Do ❌

**DO NOT create a separate library:**
```cmake
# ❌ WRONG - Creates circular dependency
add_library(rol_interface STATIC ${rol_interface_sources})
target_link_libraries(dakota_src rol_interface)
target_link_libraries(rol_interface dakota_src)  # Circular!
```

**DO NOT use add_subdirectory for building:**
```cmake
# ❌ WRONG - If rol_interface/CMakeLists.txt builds a library
add_subdirectory(rol_interface)
```

### Testing

Tests are handled through Dakota's standard unit test framework:
- Location: `src/unit/dakota_rol/`
- Tests link against `dakota_src` (which includes ROL interface code)
- No separate test linking needed

### Build Verification

To verify the integration:
```bash
cd build
cmake ..
# Look for message: "Using new modular ROL interface (rol_interface/)"
make dakota_src
# Verify no librol_interface.a or librol_interface.so created
```

### Migration Path

For projects wanting to use the old implementation:
1. Comment out `-DHAVE_DAKOTA_ROL_INTERFACE` definition
2. Code will automatically use old `ROLOptimizer.hpp`
3. No other changes needed
