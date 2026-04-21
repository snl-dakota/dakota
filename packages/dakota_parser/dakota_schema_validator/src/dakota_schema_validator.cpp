// dakota_schema_validator.cpp
// Implementation of C++ JSON Schema Validator for Dakota Pydantic models

#include <dakota/schema_validator.hpp>
#include <dakota/validation.hpp>
#include <dakota/computed_fields.hpp>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <limits>

namespace dakota::schema {

// ============================================================================
// SchemaError
// ============================================================================

std::string SchemaError::format() const {
    std::ostringstream oss;
    if (!path.empty()) {
        oss << path << ": ";
    }
    oss << message;
    if (!rule.empty()) {
        oss << " [" << rule << "]";
    }
    return oss.str();
}

// ============================================================================
// SchemaValidationResult
// ============================================================================

void SchemaValidationResult::add_error(SchemaError error) {
    errors_.push_back(std::move(error));
}

void SchemaValidationResult::add_errors(const SchemaValidationResult& other) {
    errors_.insert(errors_.end(), other.errors_.begin(), other.errors_.end());
}

std::string SchemaValidationResult::format() const {
    if (errors_.empty()) {
        return "Validation successful";
    }
    
    std::ostringstream oss;
    oss << errors_.size() << " validation error" << (errors_.size() > 1 ? "s" : "") << ":\n";
    for (const auto& error : errors_) {
        oss << "  " << error.format() << "\n";
    }
    return oss.str();
}

void SchemaValidationResult::raise_if_invalid() const {
    if (!is_valid()) {
        throw SchemaValidationException(*this);
    }
}

// ============================================================================
// SchemaValidationException
// ============================================================================

SchemaValidationException::SchemaValidationException(SchemaValidationResult result)
    : std::runtime_error(result.format())
    , result_(std::move(result))
{}

// ============================================================================
// ValidationContext
// ============================================================================

const std::unordered_set<std::string> ValidationContext::empty_set_;

ValidationContext::ValidationContext(const SchemaValidator& validator)
    : validator_(validator)
{}

void ValidationContext::push_path(const std::string& segment) {
    path_.push_back(segment);
}

void ValidationContext::push_path(size_t index) {
    path_.push_back("[" + std::to_string(index) + "]");
}

void ValidationContext::pop_path() {
    if (!path_.empty()) {
        path_.pop_back();
    }
}

std::string ValidationContext::current_path() const {
    return format_path(path_);
}

void ValidationContext::add_error(const std::string& message, const std::string& rule) {
    result_.add_error({current_path(), message, rule});
}

void ValidationContext::register_block_id(const std::string& block_type, const std::string& id) {
    block_ids_[block_type].insert(id);
}

bool ValidationContext::has_block_id(const std::string& block_type, const std::string& id) const {
    auto it = block_ids_.find(block_type);
    if (it == block_ids_.end()) {
        return false;
    }
    return it->second.count(id) > 0;
}

const std::unordered_set<std::string>& ValidationContext::get_block_ids(
    const std::string& block_type) const 
{
    auto it = block_ids_.find(block_type);
    if (it == block_ids_.end()) {
        return empty_set_;
    }
    return it->second;
}

void ValidationContext::defer_block_pointer_check(const std::string& path,
                                                   const std::string& block_type,
                                                   const std::string& ref_id) 
{
    deferred_checks_.push_back({path, block_type, ref_id});
}

// ============================================================================
// Utility Functions
// ============================================================================

std::string parse_ref_name(const std::string& ref) {
    // Parse "#/$defs/ModelName" -> "ModelName"
    const std::string prefix = "#/$defs/";
    if (ref.substr(0, prefix.size()) == prefix) {
        return ref.substr(prefix.size());
    }
    return ref;
}

std::string format_path(const std::vector<std::string>& parts) {
    if (parts.empty()) {
        return "";
    }
    
    std::ostringstream oss;
    bool first = true;
    for (const auto& part : parts) {
        if (part.empty()) continue;
        
        // Array index
        if (part[0] == '[') {
            oss << part;
        } else {
            if (!first) {
                oss << ".";
            }
            oss << part;
        }
        first = false;
    }
    return oss.str();
}

bool matches_type(const json& value, SchemaNode::Type type) {
    switch (type) {
        case SchemaNode::Type::Object:
            return value.is_object();
        case SchemaNode::Type::Array:
            return value.is_array();
        case SchemaNode::Type::String:
            return value.is_string();
        case SchemaNode::Type::Integer:
            return value.is_number_integer();
        case SchemaNode::Type::Number:
            return value.is_number();
        case SchemaNode::Type::Boolean:
            return value.is_boolean();
        case SchemaNode::Type::Null:
            return value.is_null();
        case SchemaNode::Type::Any:
            return true;
        default:
            return false;
    }
}

std::string type_name(SchemaNode::Type type) {
    switch (type) {
        case SchemaNode::Type::Object: return "object";
        case SchemaNode::Type::Array: return "array";
        case SchemaNode::Type::String: return "string";
        case SchemaNode::Type::Integer: return "integer";
        case SchemaNode::Type::Number: return "number";
        case SchemaNode::Type::Boolean: return "boolean";
        case SchemaNode::Type::Null: return "null";
        case SchemaNode::Type::Const: return "const";
        case SchemaNode::Type::AnyOf: return "anyOf";
        case SchemaNode::Type::AllOf: return "allOf";
        case SchemaNode::Type::Ref: return "ref";
        case SchemaNode::Type::Any: return "any";
    }
    return "unknown";
}

std::string json_type_name(const json& value) {
    if (value.is_null()) return "null";
    if (value.is_boolean()) return "boolean";
    if (value.is_number_integer()) return "integer";
    if (value.is_number()) return "number";
    if (value.is_string()) return "string";
    if (value.is_array()) return "array";
    if (value.is_object()) return "object";
    return "unknown";
}

// ============================================================================
// SchemaValidator - Construction
// ============================================================================

SchemaValidator::SchemaValidator(const json& schema)
    : schema_(schema)
{
    parse_schema();
}

SchemaValidator SchemaValidator::from_file(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Failed to open schema file: " + path);
    }
    json schema = json::parse(file);
    return SchemaValidator(schema);
}

// ============================================================================
// SchemaValidator - Schema Parsing
// ============================================================================

void SchemaValidator::parse_schema() {
    // First pass: parse all definitions
    if (schema_.contains("$defs")) {
        for (auto& [name, def_schema] : schema_["$defs"].items()) {
            definitions_[name] = parse_node(def_schema);
        }
    }
    
    // Second pass: resolve references in definitions
    for (auto& [name, node] : definitions_) {
        resolve_references(*node);
    }
    
    // Parse root schema
    root_ = parse_node(schema_);
    resolve_references(*root_);
}

std::shared_ptr<SchemaNode> SchemaValidator::parse_node(const json& node_schema) {
    auto node = std::make_shared<SchemaNode>();
    
    // Handle $ref
    if (node_schema.contains("$ref")) {
        node->type = SchemaNode::Type::Ref;
        node->ref_path = node_schema["$ref"].get<std::string>();
        return node;
    }
    
    // Handle allOf
    if (node_schema.contains("allOf")) {
        node->type = SchemaNode::Type::AllOf;
        for (const auto& sub : node_schema["allOf"]) {
            node->all_of.push_back(parse_node(sub));
        }
        // Copy other properties that might be alongside allOf
        if (node_schema.contains("description")) {
            node->description = node_schema["description"].get<std::string>();
        }
        if (node_schema.contains("x-aliases")) {
            for (const auto& alias : node_schema["x-aliases"]) {
                node->aliases.push_back(alias.get<std::string>());
            }
        }
        return node;
    }
    
    // Handle anyOf (union types)
    if (node_schema.contains("anyOf")) {
        node->type = SchemaNode::Type::AnyOf;
        for (const auto& sub : node_schema["anyOf"]) {
            node->any_of.push_back(parse_node(sub));
        }
        
        // Analyze union pattern if present
        if (node_schema.contains("x-union-pattern")) {
            node->union_info = analyze_union(node_schema["anyOf"], node_schema);
        }
        
        // Copy default
        if (node_schema.contains("default")) {
            node->default_value = node_schema["default"];
        }
        
        return node;
    }
    
    // Handle const
    if (node_schema.contains("const")) {
        node->type = SchemaNode::Type::Const;
        node->const_value = node_schema["const"];
        if (node_schema.contains("default")) {
            node->default_value = node_schema["default"];
        }
        return node;
    }
    
    // Determine type
    if (node_schema.contains("type")) {
        const std::string& type_str = node_schema["type"].get<std::string>();
        if (type_str == "object") node->type = SchemaNode::Type::Object;
        else if (type_str == "array") node->type = SchemaNode::Type::Array;
        else if (type_str == "string") node->type = SchemaNode::Type::String;
        else if (type_str == "integer") node->type = SchemaNode::Type::Integer;
        else if (type_str == "number") node->type = SchemaNode::Type::Number;
        else if (type_str == "boolean") node->type = SchemaNode::Type::Boolean;
        else if (type_str == "null") node->type = SchemaNode::Type::Null;
    }
    
    // Parse object properties
    if (node_schema.contains("properties")) {
        for (auto& [name, prop_schema] : node_schema["properties"].items()) {
            auto prop_node = parse_node(prop_schema);
            
            // Copy Dakota extensions from property schema
            if (prop_schema.contains("x-block-pointer")) {
                prop_node->block_pointer = prop_schema["x-block-pointer"].get<std::string>();
            }
            if (prop_schema.contains("x-aliases")) {
                for (const auto& alias : prop_schema["x-aliases"]) {
                    prop_node->aliases.push_back(alias.get<std::string>());
                }
            }
            if (prop_schema.contains("anchor")) {
                prop_node->is_anchor = prop_schema["anchor"].get<bool>();
            }
            if (prop_schema.contains("argument")) {
                prop_node->is_argument = true;
                prop_node->argument_field = prop_schema["argument"].get<std::string>();
            }
            
            node->properties[name] = prop_node;
        }
    }
    
    // Required fields
    if (node_schema.contains("required")) {
        for (const auto& req : node_schema["required"]) {
            node->required.push_back(req.get<std::string>());
        }
    }
    
    // Additional properties
    if (node_schema.contains("additionalProperties")) {
        node->additional_properties = node_schema["additionalProperties"].get<bool>();
    }
    
    // Array items
    if (node_schema.contains("items")) {
        node->items = parse_node(node_schema["items"]);
    }
    
    // Numeric constraints
    if (node_schema.contains("gt")) node->gt = node_schema["gt"].get<double>();
    if (node_schema.contains("ge")) node->ge = node_schema["ge"].get<double>();
    if (node_schema.contains("lt")) node->lt = node_schema["lt"].get<double>();
    if (node_schema.contains("le")) node->le = node_schema["le"].get<double>();
    if (node_schema.contains("minimum")) node->minimum = node_schema["minimum"].get<double>();
    if (node_schema.contains("maximum")) node->maximum = node_schema["maximum"].get<double>();
    if (node_schema.contains("exclusiveMinimum")) {
        node->exclusive_minimum = node_schema["exclusiveMinimum"].get<double>();
    }
    if (node_schema.contains("exclusiveMaximum")) {
        node->exclusive_maximum = node_schema["exclusiveMaximum"].get<double>();
    }
    
    // Default value
    if (node_schema.contains("default")) {
        node->default_value = node_schema["default"];
    }
    
    // Metadata
    if (node_schema.contains("title")) {
        node->title = node_schema["title"].get<std::string>();
    }
    if (node_schema.contains("description")) {
        node->description = node_schema["description"].get<std::string>();
    }
    
    // x-model-validations
    if (node_schema.contains("x-model-validations")) {
        for (const auto& v : node_schema["x-model-validations"]) {
            node->model_validations.push_back(v);
        }
    }
    
    // x-computed-fields
    if (node_schema.contains("x-computed-fields")) {
        for (auto& [name, spec] : node_schema["x-computed-fields"].items()) {
            node->computed_fields[name] = spec;
        }
    }
    
    return node;
}

void SchemaValidator::resolve_references(SchemaNode& node) {
    // Resolve $ref to actual definition
    if (node.type == SchemaNode::Type::Ref) {
        std::string name = parse_ref_name(node.ref_path);
        auto it = definitions_.find(name);
        if (it != definitions_.end()) {
            // Copy the resolved node's data
            const auto& resolved = *it->second;
            node.type = resolved.type;
            node.properties = resolved.properties;
            node.required = resolved.required;
            node.additional_properties = resolved.additional_properties;
            node.items = resolved.items;
            node.any_of = resolved.any_of;
            node.all_of = resolved.all_of;
            node.union_info = resolved.union_info;
            node.const_value = resolved.const_value;
            node.gt = resolved.gt;
            node.ge = resolved.ge;
            node.lt = resolved.lt;
            node.le = resolved.le;
            node.minimum = resolved.minimum;
            node.maximum = resolved.maximum;
            node.exclusive_minimum = resolved.exclusive_minimum;
            node.exclusive_maximum = resolved.exclusive_maximum;
            node.model_validations = resolved.model_validations;
            node.computed_fields = resolved.computed_fields;
            node.title = resolved.title;
            node.description = resolved.description;
            if (!node.default_value.has_value()) {
                node.default_value = resolved.default_value;
            }
        }
    }
    
    // Recursively resolve
    for (auto& [name, prop] : node.properties) {
        resolve_references(*prop);
    }
    if (node.items) {
        resolve_references(*node.items);
    }
    for (auto& sub : node.any_of) {
        resolve_references(*sub);
    }
    for (auto& sub : node.all_of) {
        resolve_references(*sub);
    }
}

UnionInfo SchemaValidator::analyze_union(const json& any_of_schema, const json& parent_schema) {
    UnionInfo info;
    
    // Get pattern number
    int pattern = parent_schema["x-union-pattern"].get<int>();
    info.pattern = static_cast<UnionPattern>(pattern);
    
    // Get default variant if present
    if (parent_schema.contains("x-model-default")) {
        info.default_variant = parent_schema["x-model-default"].get<std::string>();
    }
    
    // Analyze variants
    info.allows_null = false;
    info.allows_empty_dict = false;
    
    for (const auto& variant : any_of_schema) {
        if (variant.contains("type") && variant["type"] == "null") {
            info.allows_null = true;
        } else if (variant.contains("$ref")) {
            info.variant_refs.push_back(variant["$ref"].get<std::string>());
        } else if (variant.contains("type") && variant["type"] == "object") {
            // Patterns 3, 5, 6 include dict
            info.allows_empty_dict = true;
        }
    }
    
    return info;
}

std::shared_ptr<SchemaNode> SchemaValidator::resolve_ref(const std::string& ref) const {
    std::string name = parse_ref_name(ref);
    auto it = definitions_.find(name);
    if (it != definitions_.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<SchemaNode> SchemaValidator::get_definition(const std::string& name) const {
    auto it = definitions_.find(name);
    if (it != definitions_.end()) {
        return it->second;
    }
    return nullptr;
}

// ============================================================================
// SchemaValidator - Validation Entry Points
// ============================================================================

json SchemaValidator::validate(const json& instance) const {
    auto [result_instance, result] = validate_safe(instance);
    result.raise_if_invalid();
    return result_instance;
}

std::pair<json, SchemaValidationResult> SchemaValidator::validate_safe(const json& instance) const {
    ValidationContext ctx(*this);
    
    // Phase 1: Validate and collect block IDs
    json result = validate_node(instance, *root_, ctx);
    
    // Phase 2: Validate block pointers
    validate_block_pointers(ctx);
    
    // Phase 3: Strip nulls if requested
    if (exclude_null_) {
        strip_nulls(result);
    }
    
    return {result, ctx.result()};
}

// ============================================================================
// SchemaValidator - Node Validation
// ============================================================================

json SchemaValidator::validate_node(const json& instance,
                                     const SchemaNode& node,
                                     ValidationContext& ctx) const 
{
    // Handle anyOf (union)
    if (node.type == SchemaNode::Type::AnyOf) {
        return validate_union(instance, node, ctx);
    }
    
    // Handle allOf
    if (node.type == SchemaNode::Type::AllOf) {
        json result = instance;
        for (const auto& sub : node.all_of) {
            result = validate_node(result, *sub, ctx);
        }
        return result;
    }
    
    // Handle const
    if (node.type == SchemaNode::Type::Const) {
        if (instance != node.const_value) {
            ctx.add_error("Value must be " + node.const_value.dump(), "const");
        }
        return instance;
    }
    
    // Type checking
    if (node.type != SchemaNode::Type::Any && 
        node.type != SchemaNode::Type::AnyOf &&
        node.type != SchemaNode::Type::AllOf &&
        node.type != SchemaNode::Type::Ref) 
    {
        if (!matches_type(instance, node.type)) {
            ctx.add_error("Expected " + type_name(node.type) + ", got " + json_type_name(instance), 
                          "type");
            return instance;
        }
    }
    
    // Type-specific validation
    switch (node.type) {
        case SchemaNode::Type::Object:
            return validate_object(instance, node, ctx);
        
        case SchemaNode::Type::Array:
            return validate_array(instance, node, ctx);
        
        case SchemaNode::Type::Integer:
        case SchemaNode::Type::Number:
            validate_numeric_constraints(instance, node, ctx);
            return instance;
        
        default:
            return instance;
    }
}

json SchemaValidator::validate_object(const json& instance,
                                       const SchemaNode& node,
                                       ValidationContext& ctx) const 
{
    json result = instance;
    
    // Check required fields
    for (const auto& req : node.required) {
        if (!instance.contains(req)) {
            ctx.add_error("Missing required field: " + req, "required");
        }
    }
    
    // Check for additional properties
    if (!node.additional_properties) {
        for (auto& [key, value] : instance.items()) {
            if (node.properties.find(key) == node.properties.end()) {
                ctx.add_error("Unknown field: " + key, "additionalProperties");
            }
        }
    }
    
    // Validate and apply defaults for each property
    for (const auto& [prop_name, prop_node] : node.properties) {
        ctx.push_path(prop_name);
        
        if (instance.contains(prop_name)) {
            // Validate existing value
            result[prop_name] = validate_node(instance[prop_name], *prop_node, ctx);
            
            // Track block IDs
            if (prop_name.substr(0, 3) == "id_") {
                std::string block_type = prop_name.substr(3);  // "id_model" -> "model"
                if (instance[prop_name].is_string()) {
                    ctx.register_block_id(block_type, instance[prop_name].get<std::string>());
                }
            }
            
            // Defer block pointer checks
            if (prop_node->block_pointer.has_value()) {
                const auto& bp = *prop_node->block_pointer;
                if (instance[prop_name].is_string()) {
                    ctx.defer_block_pointer_check(ctx.current_path(), bp,
                                                  instance[prop_name].get<std::string>());
                } else if (instance[prop_name].is_array()) {
                    for (size_t i = 0; i < instance[prop_name].size(); ++i) {
                        if (instance[prop_name][i].is_string()) {
                            ctx.push_path(i);
                            ctx.defer_block_pointer_check(ctx.current_path(), bp,
                                                          instance[prop_name][i].get<std::string>());
                            ctx.pop_path();
                        }
                    }
                }
            }
        } else if (prop_node->default_value.has_value()) {
            // Apply default
            result[prop_name] = *prop_node->default_value;
        }
        
        ctx.pop_path();
    }
    
    // Apply x-model-validations
    if (!node.model_validations.empty()) {
        result = apply_model_validations(result, node.model_validations, ctx);
    }
    
    // Apply x-computed-fields
    if (!node.computed_fields.empty()) {
        result = apply_computed_fields(result, node.computed_fields, ctx);
    }
    
    return result;
}

json SchemaValidator::validate_array(const json& instance,
                                      const SchemaNode& node,
                                      ValidationContext& ctx) const 
{
    json result = json::array();
    
    if (node.items) {
        for (size_t i = 0; i < instance.size(); ++i) {
            ctx.push_path(i);
            result.push_back(validate_node(instance[i], *node.items, ctx));
            ctx.pop_path();
        }
    } else {
        result = instance;
    }
    
    return result;
}

void SchemaValidator::validate_numeric_constraints(const json& instance,
                                                    const SchemaNode& node,
                                                    ValidationContext& ctx) const 
{
    double value = instance.get<double>();
    
    // Pydantic-style constraints
    if (node.gt.has_value() && !(value > *node.gt)) {
        ctx.add_error("Value must be greater than " + std::to_string(*node.gt), "gt");
    }
    if (node.ge.has_value() && !(value >= *node.ge)) {
        ctx.add_error("Value must be greater than or equal to " + std::to_string(*node.ge), "ge");
    }
    if (node.lt.has_value() && !(value < *node.lt)) {
        ctx.add_error("Value must be less than " + std::to_string(*node.lt), "lt");
    }
    if (node.le.has_value() && !(value <= *node.le)) {
        ctx.add_error("Value must be less than or equal to " + std::to_string(*node.le), "le");
    }
    
    // JSON Schema-style constraints
    if (node.minimum.has_value() && value < *node.minimum) {
        ctx.add_error("Value must be >= " + std::to_string(*node.minimum), "minimum");
    }
    if (node.maximum.has_value() && value > *node.maximum) {
        ctx.add_error("Value must be <= " + std::to_string(*node.maximum), "maximum");
    }
    if (node.exclusive_minimum.has_value() && !(value > *node.exclusive_minimum)) {
        ctx.add_error("Value must be > " + std::to_string(*node.exclusive_minimum), "exclusiveMinimum");
    }
    if (node.exclusive_maximum.has_value() && !(value < *node.exclusive_maximum)) {
        ctx.add_error("Value must be < " + std::to_string(*node.exclusive_maximum), "exclusiveMaximum");
    }
}

// ============================================================================
// SchemaValidator - Union Validation
// ============================================================================

json SchemaValidator::validate_union(const json& instance,
                                      const SchemaNode& node,
                                      ValidationContext& ctx) const 
{
    // If we have Dakota union info, use discriminated validation
    if (node.union_info.has_value()) {
        return validate_discriminated_union(instance, node, *node.union_info, ctx);
    }
    
    // Standard anyOf: try each variant until one succeeds
    for (const auto& variant : node.any_of) {
        ValidationContext test_ctx(ctx.validator());
        try {
            json result = validate_node(instance, *variant, test_ctx);
            if (test_ctx.result().is_valid()) {
                return result;
            }
        } catch (...) {
            // Continue to next variant
        }
    }
    
    // None matched - report error
    ctx.add_error("Value does not match any variant in union", "anyOf");
    return instance;
}

json SchemaValidator::validate_discriminated_union(const json& instance,
                                                    const SchemaNode& node,
                                                    const UnionInfo& info,
                                                    ValidationContext& ctx) const 
{
    // Handle null
    if (instance.is_null()) {
        if (info.allows_null) {
            return instance;
        }
        ctx.add_error("Null not allowed for this union", "union");
        return instance;
    }
    
    // Handle non-object (only objects expected for discriminated unions)
    if (!instance.is_object()) {
        // Check if any simple type variant matches
        for (const auto& variant : node.any_of) {
            if (variant->type == SchemaNode::Type::Null && instance.is_null()) {
                return instance;
            }
            if (matches_type(instance, variant->type)) {
                return validate_node(instance, *variant, ctx);
            }
        }
        ctx.add_error("Expected object for union type, got " + json_type_name(instance), "union");
        return instance;
    }
    
    // Handle empty dict
    if (instance.empty()) {
        switch (info.pattern) {
            case UnionPattern::Pattern3:
                // Convert {} to default variant
                if (info.default_variant.has_value()) {
                    auto def = get_definition(*info.default_variant);
                    if (def) {
                        // Get the discriminator key from the default variant
                        auto disc_key = get_discriminator_key(*def);
                        if (disc_key.has_value()) {
                            json default_instance = json::object();
                            default_instance[*disc_key] = true;
                            return validate_node(default_instance, *def, ctx);
                        }
                    }
                }
                // Fall through to error if no default
                break;
                
            case UnionPattern::Pattern5:
            case UnionPattern::Pattern6:
                // Empty dict is valid, signaling "use implementation defaults"
                return instance;
                
            default:
                // Patterns 1, 2, 4 don't allow empty dict
                break;
        }
        
        if (!info.allows_empty_dict) {
            std::vector<std::string> valid_keys;
            for (const auto& ref : info.variant_refs) {
                auto variant = resolve_ref(ref);
                if (variant) {
                    auto key = get_discriminator_key(*variant);
                    if (key.has_value()) {
                        valid_keys.push_back(*key);
                    }
                }
            }
            std::string keys_str;
            for (size_t i = 0; i < valid_keys.size(); ++i) {
                if (i > 0) keys_str += ", ";
                keys_str += valid_keys[i];
            }
            ctx.add_error("Must specify one of: " + 
                          (valid_keys.empty() ? "(unknown variants)" : keys_str), "union");
        }
        return instance;
    }
    
    // Check for exactly one key (discriminated union semantics)
    if (instance.size() > 1) {
        std::vector<std::string> keys;
        for (auto& [k, v] : instance.items()) {
            keys.push_back(k);
        }
        std::string keys_str;
        for (size_t i = 0; i < keys.size(); ++i) {
            if (i > 0) keys_str += ", ";
            keys_str += keys[i];
        }
        ctx.add_error("Union type must have exactly one key, found: " + keys_str, "union");
        return instance;
    }
    
    // Get the single key (discriminator)
    std::string disc_key = instance.begin().key();
    
    // Find matching variant
    for (const auto& ref : info.variant_refs) {
        auto variant = resolve_ref(ref);
        if (!variant) continue;
        
        auto var_key = get_discriminator_key(*variant);
        if (var_key.has_value() && *var_key == disc_key) {
            // Found matching variant - validate against it
            return validate_node(instance, *variant, ctx);
        }
    }
    
    // No matching variant found
    std::vector<std::string> valid_keys;
    for (const auto& ref : info.variant_refs) {
        auto variant = resolve_ref(ref);
        if (variant) {
            auto key = get_discriminator_key(*variant);
            if (key.has_value()) {
                valid_keys.push_back(*key);
            }
        }
    }
    
    std::string keys_str;
    for (size_t i = 0; i < valid_keys.size(); ++i) {
        if (i > 0) keys_str += ", ";
        keys_str += valid_keys[i];
    }
    
    ctx.add_error("Unknown union discriminator '" + disc_key + "', valid options: " + keys_str, 
                  "union");
    
    return instance;
}

std::optional<std::string> SchemaValidator::get_discriminator_key(const SchemaNode& variant) const {
    // For discriminated unions, the variant should have exactly one required field
    // with const: true, and that field name is the discriminator
    
    if (variant.properties.size() == 1) {
        return variant.properties.begin()->first;
    }
    
    // Check required fields
    if (!variant.required.empty()) {
        return variant.required[0];
    }
    
    // Look for a const field
    for (const auto& [name, prop] : variant.properties) {
        if (prop->type == SchemaNode::Type::Const) {
            return name;
        }
    }
    
    return std::nullopt;
}

// ============================================================================
// SchemaValidator - Model Validations
// ============================================================================

json SchemaValidator::apply_model_validations(json instance,
                                               const std::vector<json>& validations,
                                               ValidationContext& ctx) const 
{
    auto& registry = validation::ValidatorRegistry::instance();
    
    for (const auto& rule : validations) {
        std::string rule_name = rule["validationRuleName"].get<std::string>();
        std::string context = rule["validationContext"].get<std::string>();
        std::string error_msg = rule["validationErrorMessage"].get<std::string>();
        
        std::vector<std::string> fields;
        for (const auto& f : rule["validationFields"]) {
            fields.push_back(f.get<std::string>());
        }
        
        json literals = rule["validationLiterals"];
        
        if (!registry.has_validator(rule_name)) {
            // Skip unknown validators with a warning (could log this)
            continue;
        }
        
        try {
            json mutations = registry.validate(instance, rule_name, fields, literals, context);
            
            // Apply mutations
            for (auto& [key, value] : mutations.items()) {
                // Handle dotted paths
                if (key.find('.') != std::string::npos) {
                    // Split path and navigate
                    std::istringstream iss(key);
                    std::string segment;
                    json* target = &instance;
                    std::vector<std::string> path_parts;
                    
                    while (std::getline(iss, segment, '.')) {
                        path_parts.push_back(segment);
                    }
                    
                    for (size_t i = 0; i < path_parts.size() - 1; ++i) {
                        target = &(*target)[path_parts[i]];
                    }
                    (*target)[path_parts.back()] = value;
                } else {
                    instance[key] = value;
                }
            }
        } catch (const validation::ValidationError& e) {
            ctx.add_error(error_msg + ": " + e.what(), rule_name);
        } catch (const std::exception& e) {
            // Log but don't fail for unexpected errors
            ctx.add_error("Validator '" + rule_name + "' threw exception: " + e.what(), rule_name);
        }
    }
    
    return instance;
}

// ============================================================================
// SchemaValidator - Computed Fields
// ============================================================================

json SchemaValidator::apply_computed_fields(json instance,
                                             const std::unordered_map<std::string, json>& fields,
                                             ValidationContext& ctx) const 
{
    auto& registry = computed_fields::ComputedFieldRegistry::instance();
    
    for (const auto& [field_name, spec] : fields) {
        std::string function_name = spec["function"].get<std::string>();
        
        if (!registry.has_function(function_name)) {
            // Skip unknown functions
            continue;
        }
        
        try {
            json computed_value = registry.compute(instance, function_name);
            instance[field_name] = computed_value;
        } catch (const std::exception& e) {
            // Log error but continue - computed fields shouldn't cause validation failure
            ctx.add_error("Failed to compute field '" + field_name + "': " + e.what(),
                          "computed_field");
        }
    }
    
    return instance;
}

// ============================================================================
// SchemaValidator - Block Pointer Validation
// ============================================================================

void SchemaValidator::validate_block_pointers(ValidationContext& ctx) const {
    for (const auto& check : ctx.deferred_checks()) {
        if (!ctx.has_block_id(check.block_type, check.ref_id)) {
            ctx.result().add_error({
                check.path,
                "References '" + check.ref_id + "' but no " + check.block_type + 
                " block with id_" + check.block_type + "='" + check.ref_id + "' exists",
                "x-block-pointer"
            });
        }
    }
}

// ============================================================================
// Utility Functions
// ============================================================================

void strip_nulls(json& j) {
    if (j.is_object()) {
        // Collect keys to remove (can't modify while iterating)
        std::vector<std::string> keys_to_remove;
        for (auto& [key, val] : j.items()) {
            if (val.is_null()) {
                keys_to_remove.push_back(key);
            } else {
                strip_nulls(val);
            }
        }
        for (const auto& key : keys_to_remove) {
            j.erase(key);
        }
    } else if (j.is_array()) {
        for (auto& elem : j) {
            strip_nulls(elem);
        }
    }
}

}  // namespace dakota::schema
