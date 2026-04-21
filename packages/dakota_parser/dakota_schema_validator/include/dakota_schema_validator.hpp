// dakota_schema_validator.hpp
// C++ JSON Schema Validator for Dakota Pydantic models
// Mimics Pydantic validation behavior including custom validators and union patterns

#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <optional>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <stdexcept>
#include <variant>

namespace dakota::schema {

using json = nlohmann::json;

// ============================================================================
// Error Handling
// ============================================================================

/// Represents a single validation error with path context
struct SchemaError {
    std::string path;      // JSON path to the error (e.g., "method[0].sampling.count")
    std::string message;   // Human-readable error message
    std::string rule;      // Rule that failed (e.g., "type", "required", "union")
    
    std::string format() const;
};

/// Collection of validation errors with formatting
class SchemaValidationResult {
public:
    bool is_valid() const { return errors_.empty(); }
    void add_error(SchemaError error);
    void add_errors(const SchemaValidationResult& other);
    const std::vector<SchemaError>& errors() const { return errors_; }
    std::string format() const;
    
    // Throw if invalid
    void raise_if_invalid() const;

private:
    std::vector<SchemaError> errors_;
};

/// Exception thrown when validation fails
class SchemaValidationException : public std::runtime_error {
public:
    explicit SchemaValidationException(SchemaValidationResult result);
    const SchemaValidationResult& result() const { return result_; }
    
private:
    SchemaValidationResult result_;
};

// ============================================================================
// Forward Declarations
// ============================================================================

class SchemaValidator;
class SchemaNode;
class ValidationContext;

// ============================================================================
// Union Pattern Definitions
// ============================================================================

/// The 6 union patterns from Dakota's Pydantic models
enum class UnionPattern {
    Pattern1 = 1,  // Optional with model default (default_factory=Model)
    Pattern2 = 2,  // Optional defaults to None (Union[...] | None = None)
    Pattern3 = 3,  // Three states with default (Union[..., dict] | None, default_factory)
    Pattern4 = 4,  // Required union (Union[...])
    Pattern5 = 5,  // Three states no default (Union[..., dict] | None = None)
    Pattern6 = 6   // Required but selection optional (Union[..., dict])
};

/// Information about a union field
struct UnionInfo {
    UnionPattern pattern;
    std::vector<std::string> variant_refs;  // $ref paths to variant schemas
    std::optional<std::string> default_variant;  // x-model-default
    bool allows_null;
    bool allows_empty_dict;
};

// ============================================================================
// Schema Node Types
// ============================================================================

/// Represents a resolved schema node for efficient validation
class SchemaNode {
public:
    enum class Type {
        Object,
        Array,
        String,
        Integer,
        Number,
        Boolean,
        Null,
        Const,
        AnyOf,    // Union type (including nullable)
        AllOf,    // Intersection (often used for $ref + extra)
        Ref,      // Unresolved reference
        Any       // No type restriction
    };
    
    Type type = Type::Any;
    
    // For objects
    std::unordered_map<std::string, std::shared_ptr<SchemaNode>> properties;
    std::vector<std::string> required;
    bool additional_properties = false;
    
    // For arrays
    std::shared_ptr<SchemaNode> items;
    
    // Numeric constraints
    std::optional<double> gt, ge, lt, le;
    std::optional<double> minimum, maximum;
    std::optional<double> exclusive_minimum, exclusive_maximum;
    
    // Const value
    json const_value;
    
    // Union/AnyOf handling
    std::vector<std::shared_ptr<SchemaNode>> any_of;
    std::optional<UnionInfo> union_info;
    
    // AllOf
    std::vector<std::shared_ptr<SchemaNode>> all_of;
    
    // Reference
    std::string ref_path;
    
    // Default value
    std::optional<json> default_value;
    
    // Dakota extensions
    std::optional<std::string> block_pointer;  // x-block-pointer
    std::vector<std::string> aliases;           // x-aliases
    bool is_anchor = false;                     // anchor attribute
    bool is_argument = false;                   // argument attribute
    std::string argument_field;                 // argument field name
    
    // x-model-validations (stored but executed by validator)
    std::vector<json> model_validations;
    
    // x-computed-fields
    std::unordered_map<std::string, json> computed_fields;
    
    // Metadata
    std::string title;
    std::string description;
};

// ============================================================================
// Validation Context
// ============================================================================

/// Context passed during validation
class ValidationContext {
public:
    ValidationContext(const SchemaValidator& validator);
    
    // Path tracking
    void push_path(const std::string& segment);
    void push_path(size_t index);
    void pop_path();
    std::string current_path() const;
    
    // Error collection
    void add_error(const std::string& message, const std::string& rule);
    SchemaValidationResult& result() { return result_; }
    
    // Block ID tracking for x-block-pointer validation
    void register_block_id(const std::string& block_type, const std::string& id);
    bool has_block_id(const std::string& block_type, const std::string& id) const;
    const std::unordered_set<std::string>& get_block_ids(const std::string& block_type) const;
    
    // Deferred block pointer checks
    void defer_block_pointer_check(const std::string& path, 
                                    const std::string& block_type,
                                    const std::string& ref_id);
    struct DeferredBlockCheck {
        std::string path;
        std::string block_type;
        std::string ref_id;
    };
    const std::vector<DeferredBlockCheck>& deferred_checks() const { return deferred_checks_; }
    
    const SchemaValidator& validator() const { return validator_; }

private:
    const SchemaValidator& validator_;
    std::vector<std::string> path_;
    SchemaValidationResult result_;
    std::unordered_map<std::string, std::unordered_set<std::string>> block_ids_;
    std::vector<DeferredBlockCheck> deferred_checks_;
    static const std::unordered_set<std::string> empty_set_;
};

// ============================================================================
// Schema Validator
// ============================================================================

/// Main validator class that loads schema and validates instances
class SchemaValidator {
public:
    /// Load schema from JSON
    explicit SchemaValidator(const json& schema);
    
    /// Load schema from file
    static SchemaValidator from_file(const std::string& path);
    
    /// Validate an instance against the schema
    /// Returns the (possibly mutated) instance with defaults applied
    /// Throws SchemaValidationException if validation fails
    json validate(const json& instance) const;
    
    /// Validate without throwing - returns result with errors
    std::pair<json, SchemaValidationResult> validate_safe(const json& instance) const;
    
    /// Get the root schema node
    const SchemaNode& root() const { return *root_; }
    
    /// Resolve a $ref path to a schema node
    std::shared_ptr<SchemaNode> resolve_ref(const std::string& ref) const;
    
    /// Get a definition by name
    std::shared_ptr<SchemaNode> get_definition(const std::string& name) const;
    
    /// Set whether to exclude null values from output
    void set_exclude_null(bool exclude) { exclude_null_ = exclude; }
    bool exclude_null() const { return exclude_null_; }

private:
    json schema_;
    std::shared_ptr<SchemaNode> root_;
    std::unordered_map<std::string, std::shared_ptr<SchemaNode>> definitions_;
    bool exclude_null_ = false;
    
    // Schema parsing
    void parse_schema();
    std::shared_ptr<SchemaNode> parse_node(const json& node_schema);
    void resolve_references(SchemaNode& node);
    UnionInfo analyze_union(const json& any_of_schema, const json& parent_schema);
    
    // Validation implementation
    json validate_node(const json& instance, 
                       const SchemaNode& node,
                       ValidationContext& ctx) const;
    
    json validate_object(const json& instance,
                         const SchemaNode& node,
                         ValidationContext& ctx) const;
    
    json validate_array(const json& instance,
                        const SchemaNode& node,
                        ValidationContext& ctx) const;
    
    json validate_union(const json& instance,
                        const SchemaNode& node,
                        ValidationContext& ctx) const;
    
    json validate_discriminated_union(const json& instance,
                                      const SchemaNode& node,
                                      const UnionInfo& info,
                                      ValidationContext& ctx) const;
    
    void validate_numeric_constraints(const json& instance,
                                      const SchemaNode& node,
                                      ValidationContext& ctx) const;
    
    // Model validation rules (x-model-validations)
    json apply_model_validations(json instance,
                                  const std::vector<json>& validations,
                                  ValidationContext& ctx) const;
    
    // Computed fields (x-computed-fields)
    json apply_computed_fields(json instance,
                               const std::unordered_map<std::string, json>& fields,
                               ValidationContext& ctx) const;
    
    // Block pointer validation
    void validate_block_pointers(ValidationContext& ctx) const;
    
    // Extract discriminator key from a variant schema
    std::optional<std::string> get_discriminator_key(const SchemaNode& variant) const;
};

// ============================================================================
// Utility Functions
// ============================================================================

/// Parse a JSON pointer path (e.g., "#/$defs/ModelName" -> "ModelName")
std::string parse_ref_name(const std::string& ref);

/// Format a path as a JSON pointer or dot notation
std::string format_path(const std::vector<std::string>& parts);

/// Check if a JSON value matches a type
bool matches_type(const json& value, SchemaNode::Type type);

/// Get type name for error messages
std::string type_name(SchemaNode::Type type);
std::string json_type_name(const json& value);

/// Recursively remove null values from a JSON object/array
void strip_nulls(json& j);

}  // namespace dakota::schema
