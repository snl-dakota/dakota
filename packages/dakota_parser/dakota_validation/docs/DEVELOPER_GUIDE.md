# Dakota Validation System - Developer Guide

This guide explains how to add new validation functions to the Dakota validation system and use them in Pydantic models.

## Overview

The Dakota validation system provides cross-field validation for Pydantic models with a single implementation shared between Python and C++. This ensures consistent behavior whether validation runs during Python model construction or C++ DSL file parsing.

### Architecture

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         JSON Schema                                      │
│   x-model-validations: [                                                │
│     { validationRuleName, validationFields, validationLiterals, ... }   │
│   ]                                                                      │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                    ┌───────────────┴───────────────┐
                    ▼                               ▼
          ┌─────────────────┐             ┌─────────────────┐
          │  Python/Pydantic │             │   C++ DSL       │
          │                 │             │   Reader        │
          │  model.model_   │             │                 │
          │  dump() → dict  │             │  parse() → json │
          └────────┬────────┘             └────────┬────────┘
                   │                               │
                   │   ┌───────────────────────┐   │
                   └──►│  C++ Validators       │◄──┘
                       │  (dakota_validation.cpp)
                       │                       │
                       │  Standard Signature:  │
                       │  (instance, fields,   │
                       │   literals, context)  │
                       │        → mutations    │
                       └───────────┬───────────┘
                                   │
                                   ▼
                          ┌─────────────────┐
                          │   Mutations     │
                          │  (JSON object)  │
                          │  Applied back   │
                          │  to instance    │
                          └─────────────────┘
```

### Key Concepts

- **Validator Function**: A C++ function that validates a model instance and optionally returns mutations
- **ValidationRule**: A Python class that wraps a C++ validator for use with Pydantic
- **Mutations**: A JSON object containing fields to update on the instance (empty if no changes)
- **Fields**: Names of model fields used by the validator (exported to schema)
- **Literals**: Constant values used by the validator (exported to schema)
- **Context**: A string identifying where the validator is used (for error messages)

## Adding a New Validation Function

### Step 1: Implement the C++ Validator

All validators follow a standard signature:

```cpp
json validator_name(
    const json& instance,                    // Full model instance (read-only)
    const std::vector<std::string>& fields,  // Field names from schema
    const json& literals,                    // Literal values from schema (JSON array)
    const std::string& context               // Context for error messages
);
```

Add your function to `src/dakota_validation.cpp`:

```cpp
json my_validator(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
) {
    // fields[0], fields[1], ... are field names
    // literals[0], literals[1], ... are literal values (use .get<T>() to extract)
    
    const std::string& target_field = fields[0];
    double threshold = literals[0].get<double>();
    
    // Read from instance (can read any field, including nested)
    if (!instance.contains(target_field) || instance[target_field].is_null()) {
        return json::object();  // Skip validation if field is missing/null
    }
    
    double value = instance.at(target_field).get<double>();
    
    // Validate
    if (value < threshold) {
        throw ValidationError("value is below threshold");
    }
    
    // Return mutations (empty object if no changes needed)
    // Only mutate direct primitive fields - see Limitations section
    json mutations;
    // mutations["some_field"] = new_value;  // Only if modification needed
    return mutations;
}
```

Add the declaration to `include/dakota_validation.hpp`:

```cpp
/// Brief description of what the validator does.
/// 
/// Fields: [field1, field2, ...]
/// Literals: [literal1, literal2, ...]
/// 
/// @throws ValidationError if validation fails
json my_validator(
    const json& instance,
    const std::vector<std::string>& fields,
    const json& literals,
    const std::string& context
);
```

### Step 2: Register in ValidatorRegistry

In `src/dakota_validation.cpp`, add registration in the `ValidatorRegistry` constructor:

```cpp
ValidatorRegistry::ValidatorRegistry() {
    // Existing registrations...
    register_validator("compare_len", compare_length);
    register_validator("compare_len1", compare_length_one);
    register_validator("default_bounds", default_bounds);
    register_validator("default_initial_point", default_initial_point);
    
    // Add your validator
    register_validator("my_validator", my_validator);
}
```

The string name (e.g., `"my_validator"`) is what appears in the JSON schema as `validationRuleName`.

### Step 3: Add Python Bindings (Optional)

If you want to call the validator directly from Python (outside of the ValidationRule wrapper), add it to `src/bindings.cpp`:

```cpp
m.def("my_validator", &my_validator,
    py::arg("instance"),
    py::arg("fields"),
    py::arg("literals"),
    py::arg("context"),
    R"pbdoc(
        Brief description.
        
        Fields: [field1, field2]
        Literals: [threshold]
    )pbdoc"
);
```

### Step 4: Create Python ValidationRule Wrapper

Add a class to `python/dakota/spec/validation/rules.py`:

```python
class MyValidator(ValidationRule):
    """Description of what this validator does.
    
    Fields: [target_field]
    Literals: [threshold]
    
    Args:
        context: Validation context (e.g., "my_model")
        target_field: Name of the field to validate
        threshold: Minimum allowed value
    """
    
    def __init__(self, context: str, target_field: str, threshold: float):
        super().__init__(
            context=context,
            rule_name="my_validator",  # Must match registry name
            fields=[target_field],
            literals=[threshold],
            error_message="For {context}, {F0} must be at least {L0}."
        )

    def __call__(self, instance: Any) -> None:
        if _USE_CPP:
            instance_dict = instance.model_dump(mode='python')
            mutations = _cpp.my_validator(
                instance_dict,
                self.fields,
                self.literals,
                self.context
            )
            _apply_mutations(instance, mutations)
        else:
            self._python_impl(instance)
    
    def _python_impl(self, instance: Any) -> None:
        """Pure Python fallback implementation."""
        target_field = self.fields[0]
        threshold = self.literals[0]
        
        value = getattr(instance, target_field)
        if value is None:
            return
        
        if value < threshold:
            raise ValueError(f"value is below threshold")
```

### Step 5: Use in a Pydantic Model

```python
from typing import ClassVar, List
from pydantic import BaseModel
from dakota.spec.validation import ValidationRule
from dakota.spec.validation.rules import MyValidator, CompareLength

class MyModel(DakotaBaseModel):
    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        MyValidator(
            context="my_model",
            target_field="temperature",
            threshold=0.0
        ),
        CompareLength(
            context="my_model",
            list_field="values",
            target_field="count"
        ),
    ]
    
    count: int
    temperature: float
    values: list[float] | None = None
```

Validators run in order during model construction, after Pydantic's built-in field validation.

## Error Message Templates

The `error_message` parameter supports token substitution:

- `{F0}`, `{F1}`, ... → Field names from `fields` list
- `{L0}`, `{L1}`, ... → Literal values from `literals` list
- `{context}` → The context string
- `{rule_name}` → The rule name

Example:
```python
error_message="For {context}, length of {F0} must equal {F1}. Threshold is {L0}."
```

With `fields=["values", "count"]`, `literals=[10]`, `context="my_model"`:
```
"For my_model, length of values must equal count. Threshold is 10."
```

## JSON Schema Export

Validators automatically export their specification to JSON schema under `x-model-validations`:

```json
{
  "x-model-validations": [
    {
      "validationRuleName": "my_validator",
      "validationFields": ["temperature"],
      "validationLiterals": [0.0],
      "validationContext": "my_model",
      "validationErrorMessage": "For my_model, temperature must be at least 0.0."
    }
  ]
}
```

The C++ DSL reader uses this to reconstruct and execute the same validation.

## Using ValidatorRegistry (C++ Side)

The DSL reader can execute validators by name:

```cpp
#include "dakota_validation.hpp"

using namespace dakota::validation;

// Execute a single validator
json mutations = ValidatorRegistry::instance().validate(
    instance,
    "my_validator",           // validationRuleName
    {"temperature"},          // validationFields
    json::array({0.0}),       // validationLiterals
    "my_model"                // validationContext
);
instance.merge_patch(mutations);

// Or execute all validators from schema
json validated = ValidatorRegistry::instance().validate_all(
    instance,
    schema["x-model-validations"]
);
```

## Testing

### C++ Tests

Add tests to `test/test_validation.cpp`:

```cpp
TEST(MyValidator, PassesWhenAboveThreshold) {
    json instance = {
        {"temperature", 25.0}
    };
    
    json mutations = my_validator(
        instance,
        {"temperature"},
        json::array({0.0}),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(MyValidator, ThrowsWhenBelowThreshold) {
    json instance = {
        {"temperature", -10.0}
    };
    
    EXPECT_THROW(
        my_validator(instance, {"temperature"}, json::array({0.0}), "test"),
        ValidationError
    );
}
```

### Python Tests

Add tests to `test/test_validation.py`:

```python
class TestMyValidator:
    def test_passes_when_above_threshold(self):
        rule = MyValidator(context="test", target_field="temp", threshold=0.0)
        instance = SomeModel(temp=25.0)
        rule(instance)  # Should not raise
    
    def test_raises_when_below_threshold(self):
        rule = MyValidator(context="test", target_field="temp", threshold=0.0)
        instance = SomeModel(temp=-10.0)
        with pytest.raises(ValueError):
            rule(instance)
```

## Known Limitations

### 1. Mutations Cannot Replace Entire Nested Models

**Validators can mutate fields at any depth using dotted paths, but cannot replace an entire nested model instance.**

This will raise a `RuntimeError`:
```python
# BAD: Trying to replace an entire nested model
mutations["nested_model"] = {"x": 1.0, "y": 2.0}
```

This is fine:
```python
# GOOD: Mutating a primitive field within a nested model
mutations["nested_model.x"] = 1.0
mutations["nested_model.y"] = 2.0
```

**Reason**: Setting a nested model field to a plain dict would corrupt the Pydantic model's state, bypassing type checking and validation.

### 2. Nested Mutations via Dotted Paths

Validators can mutate fields at any nesting depth using dotted path notation:

```cpp
json my_validator(const json& instance, ...) {
    json mutations;
    
    // Single-level nested mutation
    mutations["child.value"] = 42.0;
    
    // Deep nested mutation
    mutations["outer.middle.inner.x"] = 99.5;
    
    // List field in nested model
    mutations["config.settings.values"] = {1.0, 2.0, 3.0};
    
    return mutations;
}
```

The Python side will navigate through the model hierarchy and apply the mutation at the correct level.

**Constraints:**
- All intermediate path components must be existing BaseModel fields
- The leaf field must NOT be a BaseModel type (primitives/lists only)
- Intermediate fields cannot be None

### 3. Reading Nested Fields

Validators can read any field at any depth for validation purposes:

```cpp
// This is fine - reading nested data for validation
double nested_value = instance["outer"]["inner"]["value"].get<double>();
if (nested_value < 0) {
    throw ValidationError("nested value must be non-negative");
}
return json::object();  // No mutations, just validation
```

### 4. Unknown Mutation Keys are Rejected

If a validator returns a mutation for a field that doesn't exist on the model, a `RuntimeError` is raised:

```
RuntimeError: Cannot apply mutation to 'typo_field.value': 
field 'typo_field' not found on MyModel. Valid fields are: count, child
```

### 5. Validation Order Matters

Validators run in the order they appear in `_VALIDATION_RULES`. If one validator depends on mutations from another, order them correctly:

```python
_VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
    DefaultBounds(...),      # Sets lower_bounds, upper_bounds if None
    CompareLength(...),      # Now safe to check length of bounds
    DefaultInitialPoint(...),# Depends on bounds being set
]
```

### 6. Pure Python Fallback

If the C++ module isn't installed, validators fall back to pure Python implementations. Ensure `_python_impl()` produces identical results to the C++ version.

### 7. model_dump() Serialization

The full model is serialized via `model_dump(mode='python')` before passing to C++. This means:
- Custom types must be JSON-serializable
- Very large models may have serialization overhead
- Circular references will cause errors

## Debugging Tips

1. **Check if C++ backend is active**:
   ```python
   from dakota.spec.validation import CPP_BACKEND_AVAILABLE
   from dakota.spec.validation.rules import _USE_CPP
   print(f"C++ available: {CPP_BACKEND_AVAILABLE}, using: {_USE_CPP}")
   ```

2. **Test C++ directly**:
   ```python
   from dakota.spec.validation import dakota_validation_cpp as dv
   
   result = dv.my_validator(
       {"temperature": 25.0},
       ["temperature"],
       [0.0],
       "test"
   )
   print(result)
   ```

3. **Inspect schema export**:
   ```python
   print(MyModel.model_json_schema())
   # Look for x-model-validations
   ```

4. **Verbose validation errors**: The error location tuple shows the path:
   ```
   ValidationError: 1 validation error
     my_model.my_validator.temperature
       For my_model, temperature must be at least 0.0: value is below threshold
   ```

## Computed Fields

Computed fields are read-only properties derived from other model fields. Unlike validators
which mutate mutable fields, computed fields represent values that are always derived from
the model's state and never set directly by users.

### Use Cases

- **Bounds for discrete distributions**: Poisson, binomial, geometric, etc. have bounds
  that are derived from distribution parameters, not user-specified.
- **Derived statistics**: Means, variances, or other values computed from parameters.

### Architecture

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         JSON Schema                                      │
│   property: {                                                           │
│     "readOnly": true,                                                   │
│     "x-computed-field": { "function": "..." }                           │
│   }                                                                      │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                    ┌───────────────┴───────────────┐
                    ▼                               ▼
          ┌─────────────────┐             ┌─────────────────┐
          │  Python/Pydantic │             │   C++ DSL       │
          │  @computed_field │             │   Reader        │
          │                 │             │                 │
          │  calls helper   │             │  calls C++ fn   │
          │  class method   │             │                 │
          └────────┬────────┘             └────────┬────────┘
                   │                               │
                   │   ┌───────────────────────┐   │
                   └──►│  C++ Function         │◄──┘
                       │  (dakota_computed_    │
                       │   fields.cpp)         │
                       │                       │
                       │  Signature:           │
                       │  (instance) → value   │
                       └───────────────────────┘
```

### Adding a Computed Field Function

#### Step 1: Implement the C++ Function

```cpp
// In dakota_computed_fields.cpp

json my_computed_field(const json& instance) {
    // Read input fields
    int count = instance.at("count").get<int>();
    const auto& params = instance.at("params");
    
    // Compute the result
    json result = json::array();
    for (const auto& p : params) {
        result.push_back(some_computation(p.get<double>()));
    }
    
    return result;
}
```

#### Step 2: Declare in Header

```cpp
// In dakota_computed_fields.hpp

json my_computed_field(const json& instance);
```

#### Step 3: Register in ComputedFieldRegistry

```cpp
// In ComputedFieldRegistry constructor

register_function("my_computed_field", my_computed_field);
```

#### Step 4: Add Python Binding

```cpp
// In bindings.cpp

m.def("my_computed_field", &my_computed_field,
    py::arg("instance"),
    "Compute my_field from params");
```

#### Step 5: Add Python Helper Class

Add a helper class to `python/dakota/spec/validation/computed_fields.py`:

```python
class MyBounds:
    """Computed bounds for my distribution."""
    
    @staticmethod
    def lower_bounds(instance: Any) -> List[float]:
        if _USE_CPP:
            return _cpp.my_lower_bounds(instance.model_dump(mode='python'))
        # Pure Python fallback
        return [0.0] * instance.count
    
    @staticmethod
    def upper_bounds(instance: Any) -> List[float]:
        if _USE_CPP:
            return _cpp.my_upper_bounds(instance.model_dump(mode='python'))
        # Pure Python fallback
        return [some_computation(p) for p in instance.params]
```

#### Step 6: Use in Pydantic Model

Use the helper class in your model's computed field properties:

```python
from pydantic import computed_field
from dakota.spec.validation.computed_fields import (
    computed_field_schema,
    MyBounds,
)

class MyModel(BaseModel):
    count: int
    params: List[float]
    
    @computed_field(**computed_field_schema("my_lower_bounds"))
    @property
    def lower_bounds(self) -> List[float]:
        return MyBounds.lower_bounds(self)
    
    @computed_field(**computed_field_schema("my_upper_bounds"))
    @property
    def upper_bounds(self) -> List[float]:
        return MyBounds.upper_bounds(self)
```

The helper classes automatically use C++ when available and fall back to pure Python otherwise.

### Schema Output

Computed fields are emitted in a separate `x-computed-fields` section of the model schema, not in `properties` (which represents parsed input). The model generator handles emitting JSON Schema types for each field:

```json
{
  "type": "object",
  "properties": {
    "count": {"type": "integer"},
    "params": {"type": "array", "items": {"type": "number"}}
  },
  "x-computed-fields": {
    "lower_bounds": {
      "type": "array",
      "items": {"type": "number"},
      "function": "my_lower_bounds",
      "description": "Computed lower bounds"
    },
    "upper_bounds": {
      "type": "array",
      "items": {"type": "number"},
      "function": "my_upper_bounds",
      "description": "Computed upper bounds"
    }
  }
}
```

The C++ DSL reader uses this to populate computed fields after parsing:
1. Parse input into `properties` fields
2. Iterate `x-computed-fields` and call each function from `ComputedFieldRegistry`
3. Store results in instance

### Validators vs Computed Fields

| Aspect | Validators | Computed Fields |
|--------|------------|-----------------|
| Purpose | Set defaults, repair values | Derive read-only values |
| Mutability | Can modify instance | Never modify instance |
| Schema marker | `x-model-validations` | `x-computed-field` + `readOnly` |
| When called | After field validation | On property access / serialization |
| Signature | `(instance, fields, literals, context) → mutations` | `(instance) → value` |

### Built-in Computed Fields

The following computed fields are registered for discrete aleatory uncertain variables:

| Function | Description |
|----------|-------------|
| `poisson_lower_bounds` | Always zeros |
| `poisson_upper_bounds` | `ceil(λ + 3√λ)` |
| `binomial_lower_bounds` | Always zeros |
| `binomial_upper_bounds` | `num_trials` |
| `negative_binomial_lower_bounds` | `num_trials` |
| `negative_binomial_upper_bounds` | `ceil(mean + 3*stdev)` |
| `geometric_lower_bounds` | Always zeros |
| `geometric_upper_bounds` | `ceil(mean + 3*stdev)` |
| `hypergeometric_lower_bounds` | Always zeros |
| `hypergeometric_upper_bounds` | `min(num_drawn, selected_population)` |
