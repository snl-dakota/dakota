# iterator_utils.cpp Update Summary

## Changes Made

Updated `iterator_utils.cpp` to correctly use the new refactored `DakotaROLOptimizer` when `HAVE_DAKOTA_ROL_INTERFACE` is defined.

### 1. Header Include (Lines 96-102)

**Before:**
```cpp
#if defined(HAVE_ROL) && !defined(HAVE_DAKOTA_ROL_INTERFACE)
#include "ROLOptimizer.hpp"
#endif
#if defined(HAVE_DAKOTA_ROL_INTERFACE)
#include "DakotaROLOptimizerNew.hpp"  // ❌ File doesn't exist
#endif
```

**After:**
```cpp
#ifdef HAVE_ROL
#if defined(HAVE_DAKOTA_ROL_INTERFACE)
#include "DakotaROLOptimizer.hpp"  // ✅ Correct new header
#else
#include "ROLOptimizer.hpp"        // ✅ Old fallback
#endif
#endif
```

### 2. Standard Constructor Usage (Line 447-449)

**Before:**
```cpp
#ifdef HAVE_ROL
case ROL:
#if defined(HAVE_DAKOTA_ROL_INTERFACE)
    return std::make_shared<DakotaROLOptimizerNew>(problem_db, parallel_lib, model); // ❌ Wrong class
#else
    return std::make_shared<ROLOptimizer>(problem_db, parallel_lib, model);
#endif
#endif
```

**After:**
```cpp
#ifdef HAVE_ROL
case ROL:
    return std::make_shared<ROLOptimizer>(problem_db, parallel_lib, model); // ✅ Works for both
#endif
```

### 3. Alternate Constructor Usage (Line 566-568)

**Before:**
```cpp
#ifdef HAVE_ROL
else if (method_string == "rol")
#if defined(HAVE_DAKOTA_ROL_INTERFACE)
    return std::make_shared<ROLOptimizerImpl>(method_string, model);  // ❌ Wrong class
#else
    return std::make_shared<ROLOptimizer>(method_string, model);
#endif
#endif
```

**After:**
```cpp
#ifdef HAVE_ROL
else if (method_string == "rol")
    return std::make_shared<ROLOptimizer>(method_string, model);  // ✅ Works for both
#endif
```

## Rationale

The new `Dakota::ROLOptimizer` class maintains **the exact same API** as the old `ROLOptimizer`:
- Same constructor signatures
- Same public method names
- Same behavior

Therefore, the instantiation code doesn't need conditional compilation - it works identically for both versions. The only difference is which header file is included, which is controlled by `HAVE_DAKOTA_ROL_INTERFACE`.

## Benefits

1. **Cleaner Code**: No more nested `#if defined()` checks in instantiation
2. **Less Duplication**: Single instantiation code path
3. **Easier Maintenance**: Changes to constructor calls only need to be made once
4. **Correct Behavior**: Now uses the actual new implementation when enabled

## Backward Compatibility

When `HAVE_DAKOTA_ROL_INTERFACE` is:
- **Defined**: Uses new modular implementation from `rol_interface/`
- **Not defined**: Falls back to old monolithic implementation

Both versions provide identical functionality from the user's perspective.
