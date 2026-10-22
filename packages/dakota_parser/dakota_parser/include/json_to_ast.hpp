// json_to_ast.hpp — convert Dakota JSON input into the parser AST
//
// The inverse of ast_to_json(): it uses the same generated metadata
// (dakota_ast_metadata.hpp) and the same lookups, so that
// ast_to_json(json_to_ast(j)) == j for the JSON that ast_to_json produces
// (checked over the Dakota regression inputs by test_json_to_ast).
// JSON input then goes through the same pipeline as freeform input:
// validate_document → analyze_semantics → expand_defaults → ast_to_json →
// validate_json_document.
//
// The input is expected to have passed validation_metadata::
// check_json_structure(), which checks it against the schema; the
// conversion loses some of what is checked there (e.g. a scalar where the
// schema wants an array).
//
// Mapping, per JSON object key:
//   anchor wrapper (e.g. "gradient_type")  → its children become keywords
//   true / false                           → flag keyword (false as a value),
//                                            where the schema wants a boolean
//   {}                                     → keyword without values, where
//                                            the schema wants an object
//   scalar / scalar array                  → keyword with parameter values
//   object                                 → keyword whose argument field
//                                            (e.g. "count") or "_value" gives
//                                            the parameter values and whose
//                                            other keys are child keywords
//   null                                   → omitted (same as absent; no
//                                            nullable field has a non-null
//                                            default)

#ifndef DAKOTA_JSON_TO_AST_HPP
#define DAKOTA_JSON_TO_AST_HPP

#include "ast_to_json.hpp"

#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace dakota {
namespace json_to_ast_detail {

// Collects conversion errors with the JSON path of the offending value.
struct Context {
    std::vector<std::string>& errors;
};

inline bool to_value(const json& v, Value& out) {
    if (v.is_boolean())        { out = v.get<bool>();        return true; }
    if (v.is_number_unsigned()) {
        // Above the int64 range, keep the value as the freeform parser
        // reads it: as a double.
        const auto u = v.get<uint64_t>();
        if (u <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max()))
            out = static_cast<int64_t>(u);
        else
            out = static_cast<double>(u);
        return true;
    }
    if (v.is_number_integer()) { out = v.get<int64_t>();     return true; }
    if (v.is_number_float())   { out = v.get<double>();      return true; }
    if (v.is_string())         { out = v.get<std::string>(); return true; }
    return false;
}

inline bool append_values(const json& v, KeywordNode& node,
                          const std::string& path, Context& ctx) {
    const json single = json::array({v});
    for (const auto& e : v.is_array() ? v : single) {
        Value value;
        if (!to_value(e, value)) {
            ctx.errors.push_back(path + ": expected a number, string or boolean, got " +
                                 e.dump());
            return false;
        }
        node.param_values.push_back(std::move(value));
    }
    return true;
}

// True if `key` is an anchor wrapper: ast_to_json files every child of
// `val` under `key` for this (context_def, parent_def).
inline bool is_anchor_key(const std::string& key, const json& val,
                          const std::string& context_def,
                          const std::string& parent_def) {
    if (!val.is_object() || val.empty()) return false;
    for (const auto& [child, _] : val.items()) {
        const auto anchor = lookup_with_fallback(
            ast_metadata::get_anchor_for_child, context_def, parent_def, child);
        if (anchor.value != key) return false;
    }
    return true;
}

inline std::shared_ptr<KeywordNode> keyword_from_json(const std::string& key, const json& val,
                                                      const std::string& parent_def,
                                                      const std::string& parent_context,
                                                      bool boolean_form,
                                                      const std::string& path, Context& ctx);

// Add one child keyword per key of `obj` (looking through anchor wrappers),
// mirroring the child loop of convert_keyword().
inline void add_children(KeywordNode& node, const json& obj,
                         const std::string& context_def, const std::string& parent_def,
                         const std::string& kw_name, const std::string& skip_key,
                         const std::string& path, Context& ctx) {
    auto add = [&](const std::string& child, const json& child_val, bool anchored,
                   const std::string& child_path) {
        if (child_val.is_null()) return;
        const auto cfg = lookup_with_fallback(
            ast_metadata::get_child_config_type, context_def, parent_def, child);
        // Whether convert_keyword() writes this child, given without values,
        // as a boolean: its child loop tests the direct context_def for
        // children outside an anchor, then the child's own source_def.
        const bool boolean_form =
            is_boolean_in_context(child, cfg.source_def) ||
            (!anchored && is_boolean_in_context(child, context_def));
        if (auto kw = keyword_from_json(child, child_val, cfg.source_def, kw_name,
                                        boolean_form, child_path, ctx))
            node.add_child(child, kw);
    };
    for (const auto& [k, v] : obj.items()) {
        if (k == skip_key) continue;
        if (is_anchor_key(k, v, context_def, parent_def)) {
            for (const auto& [child, child_val] : v.items())
                add(child, child_val, true, path + "." + k + "." + child);
        } else {
            add(k, v, false, path + "." + k);
        }
    }
}

// Inverse of convert_keyword(): same child-def and argument-field lookups.
inline std::shared_ptr<KeywordNode> keyword_from_json(const std::string& key, const json& val,
                                                      const std::string& parent_def,
                                                      const std::string& parent_context,
                                                      bool boolean_form,
                                                      const std::string& path, Context& ctx) {
    auto node = std::make_shared<KeywordNode>();
    node->name = key;

    std::string child_def = ast_metadata::get_child_config_type(parent_def, key);
    if (child_def.empty()) child_def = ast_metadata::get_discriminator_config_type(key);
    std::string arg_field = ast_metadata::get_argument_field_by_def(parent_def, key);
    if (arg_field.empty() && !ast_metadata::is_scalar_in_definition(parent_def, key))
        arg_field = ast_metadata::get_argument_field(key, parent_context);

    // A boolean keyword and an object keyword given without members are the
    // same AST node; ast_to_json() writes it as true/false or {} depending on
    // the schema (`boolean_form`), so accept only the form it writes.
    if (val.is_boolean() || (val.is_object() && val.empty())) {
        if (val.is_boolean() != boolean_form) {
            ctx.errors.push_back(path + (val.is_boolean() ? ": expected an object, got "
                                                          : ": expected a boolean, got ") +
                                 val.dump());
            return nullptr;
        }
        node->is_flag = true;
        if (val.is_boolean() && !val.get<bool>()) node->param_values.push_back(false);
        return node;
    }
    if (!val.is_object()) {
        if (!append_values(val, *node, path, ctx)) return nullptr;
        return node;
    }

    std::string skip;
    if (!arg_field.empty() && val.contains(arg_field) && !val[arg_field].is_object() &&
        !val[arg_field].is_null()) {
        if (!append_values(val[arg_field], *node, path + "." + arg_field, ctx)) return nullptr;
        skip = arg_field;
    } else if (val.contains("_value")) {
        if (!append_values(val["_value"], *node, path + "._value", ctx)) return nullptr;
        skip = "_value";
    }
    const std::string context_def = child_def.empty() ? parent_def : child_def;
    add_children(*node, val, context_def, parent_def, key, skip, path, ctx);
    return node;
}

// Inverse of block_to_json().
inline void block_from_json(const std::string& block_name, const json& obj,
                            const std::string& path, Document& doc, Context& ctx) {
    if (!obj.is_object()) {
        ctx.errors.push_back(path + ": expected an object");
        return;
    }
    Block block;
    block.name = block_name;
    const std::string config_type = get_config_type(block_name);
    auto add = [&](const std::string& kw, const json& kw_val, const std::string& kw_path) {
        if (kw_val.is_null()) return;
        if (auto node = keyword_from_json(kw, kw_val, config_type, block_name,
                                          is_boolean_in_context(kw, config_type), kw_path, ctx))
            block.add_keyword(kw, node);
    };
    for (const auto& [k, v] : obj.items()) {
        bool anchor = !config_type.empty() && v.is_object() && !v.empty();
        if (anchor) {
            for (const auto& [child, _] : v.items()) {
                if (ast_metadata::get_anchor_for_child(config_type, child) != k) {
                    anchor = false;
                    break;
                }
            }
        }
        if (anchor) {
            for (const auto& [child, child_val] : v.items())
                add(child, child_val, path + "." + k + "." + child);
        } else {
            add(k, v, path + "." + k);
        }
    }
    doc.blocks.push_back(std::move(block));
}

inline bool is_block_name(const std::string& name) {
    return name == "environment" || allows_multiple_instances(name);
}

} // namespace json_to_ast_detail

// Convert a JSON input document into `doc`.  Returns false and appends to
// `errors` if the document does not have the expected block structure.
inline bool json_to_ast(const json& input, Document& doc, std::vector<std::string>& errors) {
    using namespace json_to_ast_detail;
    const size_t initial_errors = errors.size();
    Context ctx{errors};
    if (!input.is_object()) {
        errors.push_back("JSON input must be an object of blocks");
        return false;
    }
    for (const auto& [name, val] : input.items()) {
        if (!is_block_name(name)) {
            errors.push_back("Unexpected top-level block: " + name);
            continue;
        }
        if (val.is_null()) continue;
        if (allows_multiple_instances(name)) {
            if (!val.is_array()) {
                errors.push_back(name + ": expected an array of blocks");
                continue;
            }
            for (size_t i = 0; i < val.size(); ++i)
                block_from_json(name, val[i], name + "[" + std::to_string(i) + "]", doc, ctx);
        } else {
            block_from_json(name, val, name, doc, ctx);
        }
    }
    return errors.size() == initial_errors;
}

} // namespace dakota

#endif // DAKOTA_JSON_TO_AST_HPP
