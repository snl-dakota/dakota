#ifndef DAKOTA_AST_TO_JSON_HPP
#define DAKOTA_AST_TO_JSON_HPP

#include <nlohmann/json.hpp>
#include "dakota_ast.hpp"
#include "dakota_ast_metadata.hpp"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <fstream>
#include <iostream>
#include <cmath>
#include <cstdio>

namespace dakota {

using json = nlohmann::json;

// =============================================================================
// Schema Metadata for Conversion (runtime schema loading - optional)
// =============================================================================

struct SchemaMetadata {
    json schema;
    std::map<std::string, json> defs;
    
    SchemaMetadata() = default;
    
    explicit SchemaMetadata(const std::string& schema_path) {
        std::ifstream f(schema_path);
        if (f.is_open()) {
            f >> schema;
            if (schema.contains("$defs")) {
                for (auto& [name, def] : schema["$defs"].items()) {
                    defs[name] = def;
                }
            }
        }
    }
    
    json get_properties(const std::string& def_name) const {
        auto it = defs.find(def_name);
        if (it != defs.end()) {
            const json& def = it->second;
            if (def.contains("properties")) {
                return def["properties"];
            }
        }
        return json::object();
    }
    
    // Get the referenced definition type from a field
    std::string get_ref_type(const std::string& def_name, const std::string& field_name) const {
        json props = get_properties(def_name);
        if (!props.contains(field_name)) return "";
        
        const json& field = props[field_name];
        
        if (field.contains("$ref")) {
            std::string ref = field["$ref"];
            if (ref.rfind("#/$defs/", 0) == 0) {
                return ref.substr(8);
            }
        }
        if (field.contains("anyOf")) {
            for (const auto& opt : field["anyOf"]) {
                if (opt.contains("$ref")) {
                    std::string ref = opt["$ref"];
                    if (ref.rfind("#/$defs/", 0) == 0) {
                        return ref.substr(8);
                    }
                }
            }
        }
        return "";
    }
    
    // Find anchor field that contains a given keyword
    std::string find_anchor_for_keyword(const std::string& parent_def, const std::string& kw_name) const {
        json props = get_properties(parent_def);
        
        for (auto& [field_name, field_schema] : props.items()) {
            // Check if this is an anchor field
            if (!field_schema.contains("anchor") || field_schema["anchor"] != true) continue;
            
            // Check anyOf options for this anchor
            if (field_schema.contains("anyOf")) {
                for (const auto& opt : field_schema["anyOf"]) {
                    if (opt.contains("$ref")) {
                        std::string ref = opt["$ref"];
                        if (ref.rfind("#/$defs/", 0) == 0) {
                            std::string ref_type = ref.substr(8);
                            json ref_props = get_properties(ref_type);
                            if (ref_props.contains(kw_name)) {
                                return field_name;
                            }
                        }
                    }
                }
            }
        }
        return "";
    }
};

// Global schema metadata (optional runtime loading)
inline SchemaMetadata g_schema_metadata;
inline bool g_schema_loaded = false;

inline void load_schema_metadata(const std::string& schema_path) {
    g_schema_metadata = SchemaMetadata(schema_path);
    g_schema_loaded = !g_schema_metadata.defs.empty();
    if (g_schema_loaded) {
        std::cerr << "[DEBUG] Loaded schema with " << g_schema_metadata.defs.size() << " definitions\n";
    }
}

// =============================================================================
// Block Type Classification (only top-level blocks are hardcoded)
// =============================================================================

inline std::string get_config_type(const std::string& block_name) {
    if (block_name == "environment") return "EnvironmentConfig";
    if (block_name == "method") return "";  // Union block - handled per-keyword
    if (block_name == "model") return "";   // Union block - handled per-keyword
    if (block_name == "variables") return "VariablesConfig";
    if (block_name == "interface") return "InterfaceConfig";
    if (block_name == "responses") return "ResponsesConfig";
    return "";
}

inline bool allows_multiple_instances(const std::string& block_name) {
    return (block_name == "method" || 
            block_name == "model" || 
            block_name == "variables" || 
            block_name == "interface" || 
            block_name == "responses");
}

// =============================================================================
// Value Conversion
// =============================================================================

inline json value_to_json(const Value& val, bool force_float = false) {
    return std::visit([force_float](auto&& arg) -> json {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string>) {
            return arg;
        } else if constexpr (std::is_same_v<T, int64_t>) {
            if (force_float) {
                return static_cast<double>(arg);
            }
            return arg;
        } else if constexpr (std::is_same_v<T, double>) {
            return arg;
        } else if constexpr (std::is_same_v<T, bool>) {
            return arg;
        }
        return nullptr;
    }, val);
}

inline json values_to_json(const std::vector<Value>& values, bool force_float = false) {
    if (values.empty()) return nullptr;
    if (values.size() == 1) return value_to_json(values[0], force_float);
    
    json array = json::array();
    for (const auto& val : values) {
        array.push_back(value_to_json(val, force_float));
    }
    return array;
}

// =============================================================================
// Keyword to JSON Conversion
// =============================================================================

// Forward declaration
json convert_keyword(const KeywordNode& kw, const std::string& parent_def, const std::string& parent_context = "");

// =============================================================================
// Variable Default Generation
// =============================================================================

// Note: Variable defaults (initial_point, descriptors) are now handled by
// the validation pipeline's x-model-validations (default_variable_descriptors,
// default_initial_point_real, binomial_uncertain_initial, etc.) which correctly
// compute defaults from distribution parameters rather than using generic zeros.

// =============================================================================
// Keyword Conversion Helpers
// =============================================================================

// Try a string-returning lookup on primary def, then fallback def.
// Returns the first non-empty result and the def it came from.
using StringLookup = std::string(*)(const std::string&, const std::string&);

struct LookupResult {
    std::string value;
    std::string source_def;  // which def the result came from
};

inline LookupResult lookup_with_fallback(
    StringLookup fn,
    const std::string& primary, const std::string& fallback,
    const std::string& key)
{
    std::string result = fn(primary, key);
    if (!result.empty()) return {result, primary};
    if (primary != fallback) {
        result = fn(fallback, key);
        if (!result.empty()) return {result, fallback};
    }
    return {"", primary};
}

// Extract a boolean value from a Value variant.  Returns true/false/default.
inline json bool_from_value(const std::vector<Value>& values) {
    if (values.empty()) return true;
    const auto& val = values[0];
    if (std::holds_alternative<bool>(val)) return std::get<bool>(val);
    if (std::holds_alternative<int64_t>(val)) return std::get<int64_t>(val) != 0;
    return true;
}

// Check if a keyword is boolean in the current context.
// Returns false if get_child_config_type resolves it to an object.
inline bool is_boolean_in_context(const std::string& field_name, const std::string& parent_def) {
    if (!ast_metadata::is_boolean_field(field_name)) return false;
    return ast_metadata::get_child_config_type(parent_def, field_name).empty();
}

// =============================================================================
// Keyword Conversion Implementation
// =============================================================================

inline json convert_keyword(const KeywordNode& kw, const std::string& parent_def, const std::string& parent_context) {
    std::string kw_name = kw.effective_name();
    
    // Get metadata from generated tables (with optional runtime schema fallback)
    // Context-aware lookup first: uses parent_def to resolve the correct $ref
    std::string child_def = ast_metadata::get_child_config_type(parent_def, kw_name);
    if (child_def.empty()) {
        child_def = ast_metadata::get_discriminator_config_type(kw_name);
    }
    
    // Argument field: prefer definition-keyed lookup (unambiguous) over keyword-keyed
    // The same keyword (e.g., collocation_ratio) can have an argument in one definition
    // but be a plain scalar in another — definition-keyed lookup resolves this correctly
    std::string arg_field = ast_metadata::get_argument_field_by_def(parent_def, kw_name);
    if (arg_field.empty()) {
        // Fall back to keyword-level context lookup, but guard against false matches:
        // if parent_def explicitly has this keyword as a plain scalar, the keyword-level
        // argument field comes from a different context and should not be applied
        if (!ast_metadata::is_scalar_in_definition(parent_def, kw_name)) {
            arg_field = ast_metadata::get_argument_field(kw_name, parent_context);
        }
    }
    
    [[maybe_unused]] bool is_boolean = ast_metadata::is_boolean_field(kw_name);
    bool is_array = ast_metadata::is_array_field(kw_name);
    bool is_number = ast_metadata::should_output_as_float(kw_name, parent_context);
    
    // Override global is_array when this field is NOT an array in the current definition
    if (is_array && ast_metadata::is_non_array_in_definition(parent_def, kw_name)) {
        is_array = false;
    }
    
    // Optional: Try runtime schema if available and generated metadata didn't find it
    if (g_schema_loaded) {
        if (child_def.empty()) {
            child_def = g_schema_metadata.get_ref_type(parent_def, kw_name);
        }
    }
    
    json result;
    
    // Case 1: Simple flag (no values, no children)
    if (kw.param_values.empty() && !kw.has_children()) {
        if (is_boolean_in_context(kw_name, parent_def)) {
            return true;
        }
        return json::object();
    }
    
    // Case 1b: Boolean flag with explicit value (e.g., default false).
    // Use context-specific child def (not the global discriminator fallback) to
    // decide: if annotated is a $ref config object HERE return {}, if it's a
    // plain boolean HERE return true.  The global child_def may be polluted by
    // get_discriminator_config_type() returning a $ref from a different context.
    if (kw.is_flag && !kw.param_values.empty() && !kw.has_children()) {
        std::string context_child_def = ast_metadata::get_child_config_type(parent_def, kw_name);
        if (!context_child_def.empty()) {
            // $ref config object in this specific context → empty object.
            return json::object();
        }
        if (is_boolean_in_context(kw_name, parent_def)) {
            return bool_from_value(kw.param_values);
        }
    }
    
    // Case 2: Keyword with values only (no children)
    if (!kw.param_values.empty() && !kw.has_children()) {
        json vals = values_to_json(kw.param_values, is_number);
        
        // If this field should be an array but values_to_json unwrapped a single value, re-wrap it
        if (is_array && !vals.is_array()) {
            json arr = json::array();
            arr.push_back(vals);
            vals = arr;
        }
        
        // If there's an argument field, wrap the value
        if (!arg_field.empty()) {
            result = json::object();
            // Check whether THIS keyword's argument target is an array type.
            // This is unambiguous per-keyword (no field-name-level ambiguity).
            if (!ast_metadata::has_array_argument(kw_name) && !vals.is_array()) {
                result[arg_field] = vals;
            } else if (vals.is_array()) {
                result[arg_field] = vals;
            } else {
                json arr = json::array();
                arr.push_back(vals);
                result[arg_field] = arr;
            }
            return result;
        }
        return vals;
    }
    
    // Case 3: Keyword with children (container)
    result = json::object();
    
    // If there are param values AND an argument field, add them
    if (!kw.param_values.empty() && !arg_field.empty()) {
        if (!ast_metadata::has_array_argument(kw_name) && kw.param_values.size() == 1) {
            result[arg_field] = value_to_json(kw.param_values[0], is_number);
        } else {
            json arr = json::array();
            for (const auto& val : kw.param_values) {
                arr.push_back(value_to_json(val, is_number));
            }
            result[arg_field] = arr;
        }
    } else if (!kw.param_values.empty()) {
        // Don't output _value for boolean discriminator flags (e.g., "annotated")
        // that also have children — the true is just a variant selector, not data
        bool is_discriminator_flag = kw.is_flag && kw.param_values.size() == 1 &&
            std::holds_alternative<bool>(kw.param_values[0]) &&
            std::get<bool>(kw.param_values[0]) == true;
        if (!is_discriminator_flag) {
            result["_value"] = values_to_json(kw.param_values, is_number);
        }
    }
    
    // Track anchor field contents
    std::map<std::string, json> anchor_contents;
    
    // Determine context for children
    // For union-type keywords, child_def may be the first variant, which might not
    // contain all discriminator children. parent_def (the grandparent's definition)
    // has all variants' children through propagation, so we keep it as fallback.
    std::string context_def = child_def.empty() ? parent_def : child_def;
    
    // Process children
    for (const auto& [child_name, child_list] : kw.children) {
        for (const auto& child : child_list) {
            std::string child_effective = child->effective_name();
            
            // Skip children whose name matches the argument field when the
            // value was already placed from param_values in Case 3 above. The
            // default expansion may create a child node for the argument property,
            // but it would overwrite the actual parsed value.
            if (!arg_field.empty() && child_effective == arg_field && !kw.param_values.empty()) {
                continue;
            }
            
            // Resolve child's config type: try context_def, then parent_def fallback
            auto child_config = lookup_with_fallback(
                ast_metadata::get_child_config_type, context_def, parent_def, child_effective);
            // For boolean decisions, only trust the direct context_def lookup
            std::string child_context_def = ast_metadata::get_child_config_type(context_def, child_effective);
            
            // Check if this child belongs in an anchor field
            auto anchor_result = lookup_with_fallback(
                ast_metadata::get_anchor_for_child, context_def, parent_def, child_effective);
            std::string anchor = anchor_result.value;
            
            // Optional: Try runtime schema if generated metadata didn't find anchor
            if (anchor.empty() && g_schema_loaded) {
                anchor = g_schema_metadata.find_anchor_for_keyword(context_def, child_effective);
            }
            
            if (!anchor.empty()) {
                if (!anchor_contents.count(anchor)) {
                    anchor_contents[anchor] = json::object();
                }
                json child_json = convert_keyword(*child, child_config.source_def, kw_name);
                anchor_contents[anchor][child_effective] = child_json;
            } else {
                // Boolean check: only suppress via direct context_def, not fallback
                bool child_is_boolean = ast_metadata::is_boolean_field(child_effective);
                if (child_is_boolean && !child_context_def.empty()) {
                    child_is_boolean = false;
                }
                
                if (!child->has_children() && child_is_boolean &&
                    (child->is_flag || child->param_values.empty())) {
                    result[child_effective] = bool_from_value(child->param_values);
                    continue;
                }
                
                json child_json = convert_keyword(*child, child_config.source_def, kw_name);
                result[child_effective] = child_json;
            }
        }
    }
    
    // Add anchor contents to result
    for (const auto& [anchor_name, anchor_content] : anchor_contents) {
        result[anchor_name] = anchor_content;
    }
    
    return result;
}

// =============================================================================
// Block to JSON Conversion
// =============================================================================

inline json block_to_json(const Block& block) {
    json result = json::object();
    
    std::string config_type = get_config_type(block.name);
    
    // Track anchor field contents for top-level keywords
    std::map<std::string, json> anchor_contents;
    
    for (const auto& [kw_name, kw_list] : block.keywords) {
        for (const auto& kw : kw_list) {
            std::string effective_name = kw->effective_name();
            
            // Check if this top-level keyword belongs in an anchor field
            // Use config_type (definition name) to match anchor_map which is keyed by def name
            std::string anchor = ast_metadata::get_anchor_for_child(config_type, effective_name);
            
            json kw_json = convert_keyword(*kw, config_type, block.name);
            
            if (!anchor.empty()) {
                // This keyword goes into an anchor wrapper
                if (!anchor_contents.count(anchor)) {
                    anchor_contents[anchor] = json::object();
                }
                anchor_contents[anchor][effective_name] = kw_json;
            } else {
                result[effective_name] = kw_json;
            }
        }
    }
    
    // Add anchor contents to result
    for (const auto& [anchor_name, anchor_content] : anchor_contents) {
        result[anchor_name] = anchor_content;
    }
    
    return result;
}

// =============================================================================
// Document to JSON Conversion
// =============================================================================

inline json ast_to_json(const Document& doc) {
    json result = json::object();
    
    std::map<std::string, std::vector<const Block*>> blocks_by_name;
    
    for (const auto& block : doc.blocks) {
        blocks_by_name[block.name].push_back(&block);
    }
    
    for (const auto& [block_name, block_list] : blocks_by_name) {
        if (allows_multiple_instances(block_name)) {
            json array = json::array();
            for (const auto* block : block_list) {
                json block_obj = block_to_json(*block);
                if (!block_obj.empty()) {
                    array.push_back(block_obj);
                }
            }
            if (!array.empty()) {
                result[block_name] = array;
            }
        } else {
            if (!block_list.empty()) {
                json block_obj = block_to_json(*block_list[0]);
                if (!block_obj.empty()) {
                    result[block_name] = block_obj;
                }
            }
        }
    }
    
    return result;
}

// =============================================================================
// Utility Functions
// =============================================================================

// Replace IEEE non-finite values and ±1e308 sentinel with JSON string
// representations ("inf", "-inf", "nan").
// nlohmann::json stores IEEE infinity correctly in its internal double field
// (round-trips through get<double>()) but dump() outputs "null" since JSON
// has no infinity literal.  Two sources of infinity values:
//   1. Library validators push std::numeric_limits<double>::infinity() directly
//   2. Python metadata generator encodes float('inf') as ±1e308 (JSON has no
//      infinity literal), which flows through default_bounds_real as a finite
//      double — caught by exact equality (1e308 != DBL_MAX ≈ 1.798e308)
inline void replace_inf_values(json& j) {
    if (j.is_number_float()) {
        double val = j.get<double>();
        if (std::isnan(val)) {
            j = "nan";
            return;
        }
        // Emit "inf"/"-inf" for:
        //   1. IEEE infinity (user-supplied inf in DSL/JSON)
        //   2. ±1e308 (Python metadata sentinel for infinity in schema defaults)
        // Do NOT convert ±1.7976931348623157e+308 (DBL_MAX) — that is a legitimate
        // finite schema default (e.g. solution_target).
        bool pos = (std::isinf(val) && val > 0) || val == 1e308;
        bool neg = (std::isinf(val) && val < 0) || val == -1e308;
        if (pos) j = "inf";
        else if (neg) j = "-inf";
    } else if (j.is_array()) {
        for (auto& elem : j) {
            replace_inf_values(elem);
        }
    } else if (j.is_object()) {
        for (auto& [key, val] : j.items()) {
            replace_inf_values(val);
        }
    }
}

// Produce shortest decimal representation of a double that round-trips exactly.
// Matches Python's float repr: fixed notation when exponent is in [-4, 16),
// scientific notation otherwise.
inline std::string shortest_float_repr(double val) {
    if (std::isinf(val)) return val > 0 ? "inf" : "-inf";
    if (std::isnan(val)) return "nan";
    if (val == 0.0) return "0.0";
    
    // Determine the base-10 exponent to decide format
    int exponent = static_cast<int>(std::floor(std::log10(std::abs(val))));
    bool prefer_fixed = (exponent >= -4 && exponent < 16);
    
    char buf[64];
    
    // Find the shortest %g representation that round-trips exactly (up to 17 sig figs).
    // Matches Python repr() / pydantic output used in validated JSON targets.
    for (int prec = 1; prec <= 17; ++prec) {
        int n = std::snprintf(buf, sizeof(buf), "%.*g", prec, val);
        char* end;
        double reparsed = std::strtod(buf, &end);
        if (reparsed == val) {
            std::string s(buf, n);
            bool is_sci = s.find('e') != std::string::npos || s.find('E') != std::string::npos;
            
            if (prefer_fixed && is_sci) {
                // We want fixed notation but %g gave scientific — keep trying
                // at higher precision where %g switches to fixed
                continue;
            }
            
            // Ensure floats have a decimal point
            if (!is_sci && s.find('.') == std::string::npos) {
                s += ".0";
            }
            return s;
        }
    }
    
    std::snprintf(buf, sizeof(buf), "%.17g", val);
    return buf;
}

// DSL float representation: capped at 15 significant digits to match Dakota's
// ordered DSL output (output_precision = 16 in Dakota means 15 sig figs displayed).
inline std::string dsl_float_repr(double val) {
    if (std::isinf(val)) return val > 0 ? "inf" : "-inf";
    if (std::isnan(val)) return "nan";
    if (val == 0.0) return "0.0";

    int exponent = static_cast<int>(std::floor(std::log10(std::abs(val))));
    bool prefer_fixed = (exponent >= -4 && exponent < 16);

    char buf[64];
    for (int prec = 1; prec <= 15; ++prec) {
        int n = std::snprintf(buf, sizeof(buf), "%.*g", prec, val);
        char* end;
        double reparsed = std::strtod(buf, &end);
        if (reparsed == val) {
            std::string s(buf, n);
            bool is_sci = s.find('e') != std::string::npos || s.find('E') != std::string::npos;
            if (prefer_fixed && is_sci) continue;
            if (!is_sci && s.find('.') == std::string::npos) s += ".0";
            return s;
        }
    }
    // Fallback: 15 sig figs (may not round-trip, but matches Dakota's display)
    std::snprintf(buf, sizeof(buf), "%.15g", val);
    std::string s(buf);
    if (s.find('.') == std::string::npos && s.find('e') == std::string::npos &&
        s.find('E') == std::string::npos) s += ".0";
    return s;
}

// Custom JSON serializer with shortest float representation
inline void serialize_json(std::string& out, const json& j, int indent, int depth) {
    std::string pad(depth * indent, ' ');
    std::string pad_inner((depth + 1) * indent, ' ');
    
    if (j.is_null()) {
        out += "null";
    } else if (j.is_boolean()) {
        out += j.get<bool>() ? "true" : "false";
    } else if (j.is_number_integer()) {
        out += std::to_string(j.get<int64_t>());
    } else if (j.is_number_unsigned()) {
        out += std::to_string(j.get<uint64_t>());
    } else if (j.is_number_float()) {
        double val = j.get<double>();
        if (std::isnan(val)) { out += "\"nan\""; }
        // Handle inf/1e308 sentinels
        // Emit "inf"/"-inf" for:
        //   1. IEEE infinity (user-supplied inf in DSL/JSON)
        //   2. ±1e308 (Python metadata sentinel for infinity in schema defaults)
        // Do NOT convert ±1.7976931348623157e+308 (DBL_MAX) — that is a legitimate
        // finite schema default (e.g. solution_target).
        else {
            bool pos = (std::isinf(val) && val > 0) || val == 1e308;
            bool neg = (std::isinf(val) && val < 0) || val == -1e308;
            if (pos) { out += "\"inf\""; }
            else if (neg) { out += "\"-inf\""; }
            else { out += shortest_float_repr(val); }
        }
    } else if (j.is_string()) {
        out += j.dump();  // nlohmann handles string escaping correctly
    } else if (j.is_array()) {
        if (j.empty()) {
            out += "[]";
        } else {
            out += "[\n";
            for (size_t i = 0; i < j.size(); ++i) {
                out += pad_inner;
                serialize_json(out, j[i], indent, depth + 1);
                if (i + 1 < j.size()) out += ",";
                out += "\n";
            }
            out += pad + "]";
        }
    } else if (j.is_object()) {
        if (j.empty()) {
            out += "{}";
        } else {
            out += "{\n";
            size_t i = 0;
            for (auto it = j.begin(); it != j.end(); ++it, ++i) {
                out += pad_inner + "\"" + it.key() + "\": ";
                serialize_json(out, it.value(), indent, depth + 1);
                if (i + 1 < j.size()) out += ",";
                out += "\n";
            }
            out += pad + "}";
        }
    }
}

inline std::string json_to_string(const json& j, int indent = 2) {
    std::string result;
    result.reserve(4096);
    serialize_json(result, j, indent, 0);
    return result;
}

inline std::string json_to_compact_string(const json& j) {
    json copy = j;
    replace_inf_values(copy);
    return copy.dump();
}

} // namespace dakota

#endif // DAKOTA_AST_TO_JSON_HPP
