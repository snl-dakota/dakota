# Dakota Validation C++

C++ implementation of Dakota model validation functions with Python bindings via pybind11.

## Overview

This package provides validation functions for Dakota Pydantic models, implemented in C++ for:

1. **Consistency**: The same validation logic runs in both Python (via pybind11) and the C++ DSL reader
2. **Performance**: Native C++ implementation for validation-heavy workloads
3. **Single source of truth**: No risk of Python and C++ validators diverging

**For detailed instructions on adding new validators, see [docs/DEVELOPER_GUIDE.md](docs/DEVELOPER_GUIDE.md).**

## Standard Validator Signature

All validators follow a standardized signature that matches the JSON Schema export format:

```cpp
json validator_name(
    const json& instance,              // Full model instance
    const std::vector<std::string>& fields,  // Field names (validationFields)
    const json& literals,              // Literal values (validationLiterals) - JSON array
    const std::string& context         // Validation context (validationContext)
) -> json;  // Returns mutations to apply (empty object if none)
```

This makes it trivial for the C++ DSL reader to dispatch validation based on schema data:

```cpp
// In DSL reader - dispatch from x-model-validations schema entry
for (const auto& rule_spec : schema["x-model-validations"]) {
    json mutations = ValidatorRegistry::instance().validate(
        instance,
        rule_spec["validationRuleName"],
        rule_spec["validationFields"].get<std::vector<std::string>>(),
        rule_spec["validationLiterals"],  // Already a JSON array
        rule_spec["validationContext"]
    );
    instance.merge_patch(mutations);
}

// Or use the convenience method:
instance = ValidatorRegistry::instance().validate_all(instance, schema["x-model-validations"]);
```

## Validation Functions

### Core Validators

| Rule Name | Fields | Literals | Description |
|-----------|--------|----------|-------------|
| `compare_len` | [list_field, target_field] | [] | Validate list length equals target |
| `compare_len1` | [list_field, target_field] | [] | Validate list length is 1 (expand) or target |
| `default_bounds_real` | [lower, upper, count] | [lower_default, upper_default] | Set default bounds (floats) |
| `default_bounds_int` | [lower, upper, count] | [lower_default, upper_default] | Set default bounds (ints) |
| `default_initial_point_real` | [ip, lower, upper, count] | [] | Set/clamp initial point (floats) |
| `default_initial_point_int` | [ip, lower, upper, count] | [] | Set/clamp initial point (ints) |

### Variable-Specific Validators

Validators for setting default `initial_point` based on distribution parameters:

**Continuous Aleatory Uncertain:**
- `normal_uncertain_initial`, `lognormal_uncertain_initial`
- `uniform_uncertain_initial`, `loguniform_uncertain_initial`, `triangular_uncertain_initial`
- `exponential_uncertain_initial`, `beta_uncertain_initial`, `gamma_uncertain_initial`
- `gumbel_uncertain_initial`, `frechet_uncertain_initial`, `weibull_uncertain_initial`

**Discrete Aleatory Uncertain:**
- `poisson_uncertain_initial`, `binomial_uncertain_initial`
- `negative_binomial_uncertain_initial`, `geometric_uncertain_initial`
- `hypergeometric_uncertain_initial`

## Computed Fields

Computed fields are read-only values derived from model parameters. They use a simpler signature:

```cpp
json function_name(const json& instance) -> json;  // Returns the computed value
```

### Built-in Computed Fields

| Function | Description |
|----------|-------------|
| `poisson_lower_bounds` | Always zeros |
| `poisson_upper_bounds` | `ceil(λ + 3√λ)` |
| `binomial_lower_bounds` / `binomial_upper_bounds` | 0 / num_trials |
| `negative_binomial_lower_bounds` / `negative_binomial_upper_bounds` | num_trials / ceil(mean + 3*stdev) |
| `geometric_lower_bounds` / `geometric_upper_bounds` | 0 / ceil(mean + 3*stdev) |
| `hypergeometric_lower_bounds` / `hypergeometric_upper_bounds` | 0 / min(num_drawn, selected_pop) |

### Usage with Pydantic

Use the helper classes (PoissonBounds, BinomialBounds, etc.) which automatically use C++ when available and fall back to pure Python:

```python
from pydantic import computed_field
from dakota.spec.validation.computed_fields import (
    computed_field_schema,
    PoissonBounds,
)

class PoissonUncertain(BaseModel):
    count: int
    lambdas: List[float]
    
    @computed_field(**computed_field_schema("poisson_lower_bounds"))
    @property
    def lower_bounds(self) -> List[int]:
        return PoissonBounds.lower_bounds(self)
    
    @computed_field(**computed_field_schema("poisson_upper_bounds"))
    @property
    def upper_bounds(self) -> List[int]:
        return PoissonBounds.upper_bounds(self)
```

Available helper classes:
- `PoissonBounds` - lower_bounds, upper_bounds
- `BinomialBounds` - lower_bounds, upper_bounds  
- `NegativeBinomialBounds` - lower_bounds, upper_bounds
- `GeometricBounds` - lower_bounds, upper_bounds
- `HypergeometricBounds` - lower_bounds, upper_bounds

### JSON Schema

Computed fields are emitted in a separate `x-computed-fields` section of the model schema (not in `properties`, which represents parsed input):

```json
{
  "type": "object",
  "properties": {
    "count": {"type": "integer"},
    "lambdas": {"type": "array", "items": {"type": "number"}}
  },
  "required": ["count", "lambdas"],
  "x-computed-fields": {
    "lower_bounds": {
      "type": "array",
      "items": {"type": "integer"},
      "function": "poisson_lower_bounds",
      "description": "Lower bounds (always 0 for Poisson)"
    },
    "upper_bounds": {
      "type": "array",
      "items": {"type": "integer"},
      "function": "poisson_upper_bounds",
      "description": "Upper bounds: ceil(lambda + 3*sqrt(lambda))"
    }
  }
}
```

The C++ DSL reader uses `x-computed-fields` to populate derived values after parsing:

```cpp
for (const auto& [field_name, spec] : schema["x-computed-fields"].items()) {
    instance[field_name] = ComputedFieldRegistry::instance().compute(
        instance, spec["function"]
    );
}
```

## Architecture

```
                    ┌─────────────────────┐
                    │   JSON Instance     │
                    │  (Python dict or    │
                    │   C++ nlohmann::json)│
                    └──────────┬──────────┘
                               │
              ┌────────────────┴────────────────┐
              │                                 │
              ▼                                 ▼
    ┌─────────────────┐               ┌─────────────────┐
    │  Python/Pydantic │               │   C++ DSL       │
    │                 │               │   Reader        │
    │  model.model_   │               │                 │
    │  dump() → dict  │               │  parse() → json │
    └────────┬────────┘               └────────┬────────┘
             │                                 │
             │   ┌───────────────────────┐     │
             └──►│  ValidatorRegistry    │◄────┘
                 │                       │
                 │  validate(instance,   │
                 │    rule_name, fields, │
                 │    literals, context) │
                 └───────────┬───────────┘
                             │
                             ▼
                    ┌─────────────────┐
                    │   Mutations     │
                    │  (JSON object)  │
                    └─────────────────┘
```

## Dependencies

- **nlohmann_json** (>= 3.2.0): JSON for Modern C++
- **pybind11** (>= 2.6): Python bindings for C++
- **pybind11_json**: Automatic conversion between nlohmann::json and Python dicts

## Building

### Prerequisites

Install dependencies via conda (recommended):

```bash
conda install -c conda-forge cmake nlohmann_json pybind11 pybind11_json
```

### Build

```bash
mkdir build && cd build
cmake ..
make
```

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_PYTHON_BINDINGS` | ON | Build Python module |
| `BUILD_TESTS` | ON | Build tests |

## Usage

### C++ (DSL Reader)

```cpp
#include "dakota_validation.hpp"

using namespace dakota::validation;

// Option 1: Use registry with schema data
auto& registry = ValidatorRegistry::instance();
json result = registry.validate_all(instance, schema["x-model-validations"]);

// Option 2: Call validators directly
json mutations = compare_length(
    instance,
    {"lower_bounds", "count"},  // fields
    json::array(),               // literals (empty)
    "continuous_design"          // context
);
instance.merge_patch(mutations);
```

### Python

```python
import dakota_validation_cpp as dv

# Option 1: Use registry
result = dv.validate_all(instance_dict, schema["x-model-validations"])

# Option 2: Call validators directly  
mutations = dv.compare_length(
    instance_dict,
    ["lower_bounds", "count"],  # fields
    [],                          # literals
    "continuous_design"          # context
)

# Option 3: Use high-level ValidationRule classes (for Pydantic integration)
from dakota.spec.validation.rules import CompareLength

rule = CompareLength(
    context="continuous_design",
    list_field="lower_bounds",
    target_field="count"
)
rule(my_pydantic_instance)  # Validates and applies mutations
```

## JSON Schema Export

Validation rules export their specification to JSON Schema under `x-model-validations`:

```json
{
  "x-model-validations": [
    {
      "validationRuleName": "compare_len",
      "validationFields": ["lower_bounds", "count"],
      "validationLiterals": [],
      "validationContext": "continuous_design",
      "validationErrorMessage": "For continuous_design, length of lower_bounds must equal count."
    },
    {
      "validationRuleName": "default_bounds",
      "validationFields": ["lower_bounds", "upper_bounds", "count"],
      "validationLiterals": [-1.7976931348623157e+308, 1.7976931348623157e+308],
      "validationContext": "continuous_design",
      "validationErrorMessage": "Bounds setup failed."
    }
  ]
}
```

The C++ DSL reader uses this to reconstruct and execute validation.

## Error Handling

Validation errors throw `ValidationError` (C++) which translates to `ValueError` (Python):

```cpp
// C++
try {
    compare_length(instance, {"values", "count"}, json::array(), "my_model");
} catch (const ValidationError& e) {
    std::cerr << e.what() << std::endl;  // "(2 != 3)"
}
```

```python
# Python
try:
    dv.compare_length(instance, ["values", "count"], [], "my_model")
except ValueError as e:
    print(e)  # "(2 != 3)"
```

## Adding New Validators

See [docs/DEVELOPER_GUIDE.md](docs/DEVELOPER_GUIDE.md) for complete instructions. Summary:

1. Implement the C++ validator in `src/dakota_validation.cpp` using the standard signature
2. Register it in `ValidatorRegistry` constructor
3. Create a Python `ValidationRule` wrapper class in `rules.py`
4. Add tests for both C++ and Python

## Known Limitations

- **Cannot replace entire nested models**: Mutations can target nested fields via dotted paths (e.g., `"child.value"`), but cannot replace an entire model instance
- **Reading nested fields is allowed**: Cross-model validation (read-only) works fine
- **Validation order matters**: Validators run in list order; dependencies must be ordered correctly

See the Developer Guide for details.

## License

BSD-3-Clause (matches Dakota licensing)
