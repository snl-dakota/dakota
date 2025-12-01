# ROL Interface Refactor Summary

## Overview
This document summarizes the refactored `DakotaROLOptimizer` implementation that uses the new modular `rol_interface` components.

## New Files Created/Updated

### `rol_interface/include/DakotaROLOptimizer.hpp`
- Modernized header with cleaner interface
- Uses **pIMPL idiom** to hide ROL implementation details from the header
- Maintains same public API as old `ROLOptimizer.hpp` for backward compatibility
- Includes `ROLTraits` class unchanged

### `rol_interface/src/DakotaROLOptimizer.cpp`
- Complete rewrite using the new `rol_interface` components:
  - `rol_interface::Objective` - handles objective function evaluations
  - `rol_interface::Constraint` - handles constraint evaluations (linear/nonlinear, equality/inequality)
  - `rol_interface::Vector` - wraps Dakota RealVector for ROL
  - `rol_interface::OutputStreamFilter` - prefixes ROL output

## Key Improvements

### 1. **Modular Design**
- Separates concerns into distinct classes (Objective, Constraint, Vector)
- Each component is independently testable
- Easier to maintain and extend

### 2. **Modern ROL API**
- Uses ROL's newer `ROL::Problem` and `ROL::Solver` interface
- Cleaner problem setup with explicit constraint types
- Better handling of constraint bounds

### 3. **pIMPL Pattern**
- Hides ROL types from the header
- Reduces compilation dependencies
- Allows ROL implementation to change without affecting header users

### 4. **Better Constraint Handling**
- Explicit handling of four constraint types:
  - Linear equality
  - Linear inequality
  - Nonlinear equality
  - Nonlinear inequality
- Uses factory pattern (`Constraint::createSetFromModel`) for automatic setup

## Implementation Details

### Constructor Flow
1. Call base `Optimizer` constructor with `ROLTraits`
2. Create pIMPL object
3. Call `set_problem()` to build ROL problem
4. Call `set_rol_parameters()` to configure solver

### Problem Setup (`set_problem()`)
1. Determine problem type (U, B, E, EB)
2. Create initial guess vector
3. Create ROL::Problem
4. Add objective using `Objective::createFromModel()`
5. Add variable bounds if applicable
6. Add constraints using `Constraint::createSetFromModel()`
7. Finalize problem and create solver

### Solver Execution (`core_run()`)
1. Create output stream filter
2. Rebuild problem (in case of changes)
3. Run ROL solver
4. Extract solution and store in Dakota structures
5. Lookup or re-evaluate final response

## Backward Compatibility

The new implementation maintains the same public interface:
- Same constructor signatures
- Same public methods (`initialize_run()`, `core_run()`, `reset_solver_options()`)
- Same behavior from user perspective

Old code using `ROLOptimizer` should work without modification.

## Testing Recommendations

1. **Unit Tests**: Test each `rol_interface` component independently
2. **Integration Tests**: Run existing ROL test cases to verify compatibility
3. **Regression Tests**: Compare results with old implementation
4. **Constraint Tests**: Verify all four constraint types work correctly

## Migration from Old Implementation

The new implementation is automatically selected when `HAVE_DAKOTA_ROL_INTERFACE` is defined:

### Automatic Selection in `iterator_utils.cpp`
When `HAVE_DAKOTA_ROL_INTERFACE` is defined:
- Includes `DakotaROLOptimizer.hpp` from `rol_interface/include/`
- Uses the new `Dakota::ROLOptimizer` class implementation

When `HAVE_DAKOTA_ROL_INTERFACE` is NOT defined (but `HAVE_ROL` is):
- Falls back to old `ROLOptimizer.hpp` from parent directory
- Uses the monolithic old implementation

### CMake Configuration
1. Ensure `HAVE_DAKOTA_ROL_INTERFACE` is defined in your CMake configuration
2. The rol_interface sources are integrated into `dakota_src` library
3. No separate linking required - everything is in one library
4. No code changes needed in user code - API is compatible

## Future Enhancements

Possible improvements:
- Add support for multi-objective optimization
- Implement warm-start capabilities
- Add more ROL algorithm options
- Performance optimizations for large-scale problems
