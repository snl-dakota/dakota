// dakota_json_input.hpp — JSON input processor for Dakota
//
// Loads a JSON file and expands schema defaults so the document is equivalent
// to what the DSL path produces after DefaultExpander runs.  The expanded
// document is then passed to the existing validate_json_document pipeline.
//
// Pipeline (JSON mode):
//   file  → load_json_file()              [parse + basic error check]
//         → JsonDefaultExpander::expand() [this file: fill schema defaults]
//         → validate_json_document()      [dakota_validation_metadata.hpp]
//         → json_to_string()              [ast_to_json.hpp]
//
// What "expand defaults" means here:
//   1. For each absent primitive/boolean property that has a non-null "default"
//      in the schema: insert the default value.
//   2. For each absent union property with x-union-pattern==1 and an
//      x-model-default class: instantiate that variant (mirrors DSL expander
//      behaviour for omitted pattern-1 union fields).
//   3. For each empty-dict union property with x-union-pattern==3 and an
//      x-model-default: replace {} with the instantiated default variant.
//   4. Recurse into $ref objects, union variant values, and anchor wrappers.
//
// What this does NOT do:
//   - Computed fields (x-computed-fields) — handled by validate_json_document
//   - x-model-validations              — handled by validate_json_document
//   - Numeric constraint checks        — handled by validate_json_document
//   - Block-pointer validation         — handled by validate_json_document
//
// NOTE: JSON input is expected to already have the correct anchor structure
// (e.g. "response_type": {"objective_functions": {...}}).  The expander just
// fills in any missing defaults within whatever structure is present.

#ifndef DAKOTA_JSON_INPUT_HPP
#define DAKOTA_JSON_INPUT_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace dakota {

using json = nlohmann::json;

// ============================================================================
// JsonDefaultExpander
// ============================================================================

class JsonDefaultExpander {
public:
    // Construct from a loaded schema JSON object.
    explicit JsonDefaultExpander(const json& schema) : schema_(schema) {
        // Copy all $defs into a flat name→json map for O(1) lookup.
        if (schema_.contains("$defs")) {
            for (auto& [name, def] : schema_["$defs"].items()) {
                defs_[name] = def;
            }
        }
        init_block_info();
    }

    // Expand schema defaults in-place inside `doc`.
    // Returns false only on a structural error (wrong top-level type).
    // Expansion errors are non-fatal and logged to stderr when debug is on.
    bool expand(json& doc, bool debug = false) const {
        debug_ = debug;
        if (!doc.is_object()) return false;

        for (auto& [block_name, binfo] : block_info_) {
            if (!doc.contains(block_name)) continue;
            auto& bval = doc[block_name];

            if (binfo.is_array) {
                if (!bval.is_array()) continue;
                for (auto& item : bval) {
                    if (item.is_object()) expand_block_item(item, binfo);
                }
            } else {
                if (bval.is_object()) expand_block_item(bval, binfo);
            }
        }
        return true;
    }

private:
    // -----------------------------------------------------------------------
    // Internal types
    // -----------------------------------------------------------------------

    struct BlockInfo {
        std::string config_type;        // non-empty for fixed-schema blocks (variables, interface, ...)
        bool        is_union = false;   // true for method / model (items are discriminated unions)
        bool        is_array = false;
        std::vector<std::string> variant_defs;  // anyOf ref names for union blocks
    };

    const json                        schema_;
    std::map<std::string, json>       defs_;
    std::map<std::string, BlockInfo>  block_info_;
    mutable bool                      debug_ = false;

    // -----------------------------------------------------------------------
    // Initialisation: classify top-level blocks
    // -----------------------------------------------------------------------

    void init_block_info() {
        if (!schema_.contains("properties")) return;
        for (auto& [bname, bschema] : schema_["properties"].items()) {
            BlockInfo info;
            classify_block_schema(bschema, info);
            block_info_[bname] = info;
        }
    }

    void classify_block_schema(const json& s, BlockInfo& info) {
        if (s.contains("$ref")) {
            info.config_type = ref_name(s["$ref"]);
            return;
        }
        if (s.contains("items")) {
            info.is_array = true;
            classify_items(s["items"], info);
            return;
        }
        if (s.contains("anyOf")) {
            for (auto& opt : s["anyOf"]) {
                if (is_null_type(opt)) continue;
                if (opt.contains("$ref")) {
                    info.config_type = ref_name(opt["$ref"]);
                    return;
                }
                if (opt.contains("items")) {
                    info.is_array = true;
                    classify_items(opt["items"], info);
                    return;
                }
            }
        }
    }

    void classify_items(const json& items, BlockInfo& info) {
        if (items.contains("$ref")) {
            info.config_type = ref_name(items["$ref"]);
        } else if (items.contains("anyOf")) {
            info.is_union = true;
            for (auto& opt : items["anyOf"]) {
                if (opt.contains("$ref")) {
                    info.variant_defs.push_back(ref_name(opt["$ref"]));
                }
            }
        }
    }

    // -----------------------------------------------------------------------
    // Block-item dispatch
    // -----------------------------------------------------------------------

    void expand_block_item(json& item, const BlockInfo& binfo) const {
        if (binfo.is_union) {
            // e.g. method[0] = {"dot_mmfd": {...}}
            // Each key is a method name; find its config def and recurse.
            for (auto& [key, val] : item.items()) {
                if (!val.is_object()) continue;
                std::string child_def = child_def_for_key(binfo.variant_defs, key);
                if (!child_def.empty()) expand_def(val, child_def);
            }
        } else if (!binfo.config_type.empty()) {
            expand_def(item, binfo.config_type);
        }
    }

    // -----------------------------------------------------------------------
    // Core recursive expander
    // -----------------------------------------------------------------------

    // Expand defaults for `node` against schema definition `def_name`.
    void expand_def(json& node, const std::string& def_name) const {
        if (!node.is_object()) return;
        auto it = defs_.find(def_name);
        if (it == defs_.end()) return;
        const json& def = it->second;

        if (!def.contains("properties")) return;

        for (auto& [prop, pschema] : def["properties"].items()) {
            // Skip x-internal-only fields.  These are set by the validator
            // library after validation, not by users or the default expander.
            // Inserting them before validation triggers "must not be provided
            // by the user" errors from the library's guard check.
            if (pschema.value("x-internal-only", false)) continue;

            if (is_anchor(pschema)) {
                // Anchor: transparent JSON wrapper, not a DSL keyword.
                if (node.contains(prop) && node[prop].is_object()) {
                    // Anchor already present: recurse into it.
                    expand_anchor(node[prop], pschema);
                } else if (!node.contains(prop)) {
                    // Anchor absent: if it is a pattern-1 union with x-model-default,
                    // insert the default variant (e.g. format: {annotated: true}).
                    int pat = union_pattern(pschema);
                    std::string mdef = x_model_default(pschema);
                    if (pat == 1 && !mdef.empty()) {
                        node[prop] = instantiate_model_default(mdef, pschema);
                        expand_union_node(node[prop], pschema);
                    }
                    // Non-union anchors or those without x-model-default stay absent.
                }
                continue;
            }

            auto refs = all_refs(pschema);
            bool is_union  = refs.size() > 1;
            bool is_objref = refs.size() == 1;

            if (is_union) {
                expand_union_property(node, prop, pschema);
            } else if (is_objref) {
                // Simple $ref object: recurse if present.
                if (node.contains(prop) && node[prop].is_object()) {
                    expand_def(node[prop], refs[0]);
                }
                // ($ref fields rarely have non-null defaults; skip if absent.)
            } else {
                // Primitive / array-of-primitives / nullable primitive.
                if (!node.contains(prop)) {
                    insert_default(node, prop, pschema);
                }
            }
        }
    }

    // Handle a union-typed property (anyOf with multiple $ref variants).
    void expand_union_property(json& node, const std::string& prop, const json& pschema) const {
        int         pattern    = union_pattern(pschema);
        std::string model_def  = x_model_default(pschema);

        if (node.contains(prop)) {
            json& val = node[prop];
            if (val.is_object() && !val.empty()) {
                // Non-empty union object: expand the chosen variant.
                expand_union_node(val, pschema);
            } else if (val.is_object() && val.empty()
                       && pattern == 3 && !model_def.empty()) {
                // Pattern 3 + empty dict: replace with instantiated default.
                if (debug_) {
                    std::cerr << "[EXPAND] pattern-3 empty dict for " << prop
                              << " → instantiating " << model_def << "\n";
                }
                val = instantiate_model_default(model_def, pschema);
                expand_union_node(val, pschema);
            }
            // Other patterns with empty/null value: leave as-is.
        } else {
            // Property absent.
            if (pattern == 1 && !model_def.empty()) {
                // Pattern 1: auto-instantiate the default variant.
                if (debug_) {
                    std::cerr << "[EXPAND] pattern-1 absent " << prop
                              << " → instantiating " << model_def << "\n";
                }
                node[prop] = instantiate_model_default(model_def, pschema);
                expand_union_node(node[prop], pschema);
            } else {
                // Non-union-default absent field: check for a plain default.
                insert_default(node, prop, pschema);
            }
        }
    }

    // Given a discriminated union node (e.g. {"dakota": {...}}), find the
    // chosen variant and recurse into its config value.
    void expand_union_node(json& node, const json& union_schema) const {
        if (!node.is_object() || node.empty()) return;

        // Discriminated union has exactly one key.
        auto disc_it = node.begin();
        const std::string& disc_key = disc_it.key();

        for (const auto& ref_str : all_refs(union_schema)) {
            auto def_it = defs_.find(ref_str);
            if (def_it == defs_.end()) continue;
            const json& vdef = def_it->second;
            if (!vdef.contains("properties")) continue;
            if (!vdef["properties"].contains(disc_key)) continue;

            // This variant owns disc_key.
            const json& disc_prop_schema = vdef["properties"][disc_key];
            json& disc_val = disc_it.value();

            if (disc_val.is_object()) {
                auto child_refs = all_refs(disc_prop_schema);
                if (child_refs.size() == 1) {
                    // Simple config object.
                    expand_def(disc_val, child_refs[0]);
                } else if (child_refs.size() > 1) {
                    // Nested discriminated union (rare but possible).
                    expand_union_node(disc_val, disc_prop_schema);
                }
            }
            // bool / const value: nothing to expand.
            return;
        }

        if (debug_) {
            std::cerr << "[EXPAND] warning: couldn't resolve union variant key '"
                      << disc_key << "'\n";
        }
    }

    // Expand an anchor node.  An anchor is a JSON wrapper object whose keys
    // come from one (or more) variant definition(s).  We determine which
    // variant is "active" (has at least one key present) and expand its
    // properties, including any absent union-default fields.
    void expand_anchor(json& node, const json& anchor_schema) const {
        auto refs = all_refs(anchor_schema);

        for (const auto& ref_str : refs) {
            auto def_it = defs_.find(ref_str);
            if (def_it == defs_.end()) continue;
            const json& vdef = def_it->second;
            if (!vdef.contains("properties")) continue;
            const json& vprops = vdef["properties"];

            // For multi-variant anchors only expand the active variant.
            if (refs.size() > 1) {
                bool active = false;
                for (auto& [k, _] : vprops.items()) {
                    if (node.contains(k)) { active = true; break; }
                }
                if (!active) continue;
            }

            // Expand this variant's properties as if `node` were the def object.
            for (auto& [prop, pschema] : vprops.items()) {
                if (is_anchor(pschema)) {
                    if (node.contains(prop) && node[prop].is_object())
                        expand_anchor(node[prop], pschema);
                    continue;
                }

                auto sub_refs  = all_refs(pschema);
                bool sub_union = sub_refs.size() > 1;
                bool sub_ref   = sub_refs.size() == 1;

                if (sub_union) {
                    expand_union_property(node, prop, pschema);
                } else if (sub_ref) {
                    if (node.contains(prop) && node[prop].is_object())
                        expand_def(node[prop], sub_refs[0]);
                } else {
                    if (!node.contains(prop))
                        insert_default(node, prop, pschema);
                }
            }
        }
    }

    // -----------------------------------------------------------------------
    // Instantiate x-model-default variant
    // -----------------------------------------------------------------------

    // Build the initial JSON object for a default union variant.
    // E.g. x-model-default "RelativeStepScaling" → {"relative": true}
    //      x-model-default "Abort"               → {"abort": true}
    json instantiate_model_default(const std::string& model_default_class,
                                   const json& union_schema) const {
        for (const auto& ref_str : all_refs(union_schema)) {
            if (ref_str != model_default_class) continue;

            auto def_it = defs_.find(ref_str);
            if (def_it == defs_.end()) return json::object();
            const json& vdef = def_it->second;

            if (!vdef.contains("properties")) return json::object();
            const json& vprops = vdef["properties"];
            if (vprops.empty()) return json::object();

            // Variant defs for discriminated unions have exactly one property.
            auto pit = vprops.begin();
            const std::string& disc_key    = pit.key();
            const json&        disc_schema = pit.value();

            json result = json::object();

            if (disc_schema.contains("const")) {
                result[disc_key] = disc_schema["const"];
            } else {
                auto child_refs = all_refs(disc_schema);
                if (!child_refs.empty()) {
                    // Config-object discriminator: start empty, expander fills it.
                    result[disc_key] = json::object();
                } else if (disc_schema.contains("default")
                           && !disc_schema["default"].is_null()) {
                    result[disc_key] = disc_schema["default"];
                } else {
                    // Boolean flag discriminator by convention.
                    result[disc_key] = true;
                }
            }
            return result;
        }

        if (debug_) {
            std::cerr << "[EXPAND] warning: x-model-default class '"
                      << model_default_class
                      << "' not found in anyOf refs\n";
        }
        return json::object();
    }

    // -----------------------------------------------------------------------
    // Helpers
    // -----------------------------------------------------------------------

    // Extract definition name from a $ref string "#/$defs/Foo" → "Foo".
    static std::string ref_name(const json& ref_val) {
        const std::string& s = ref_val.get_ref<const std::string&>();
        if (s.rfind("#/$defs/", 0) == 0) return s.substr(8);
        return s;
    }

    // Return all $ref definition names from a schema (direct or via anyOf).
    // Null-type anyOf entries are skipped.
    std::vector<std::string> all_refs(const json& schema) const {
        std::vector<std::string> result;
        if (schema.contains("$ref")) {
            result.push_back(ref_name(schema["$ref"]));
        } else if (schema.contains("anyOf")) {
            for (auto& opt : schema["anyOf"]) {
                if (!is_null_type(opt) && opt.contains("$ref"))
                    result.push_back(ref_name(opt["$ref"]));
            }
        }
        return result;
    }

    static bool is_anchor(const json& schema) {
        return schema.value("anchor", false) == true;
    }

    static bool is_null_type(const json& schema) {
        return schema.contains("type") && schema["type"] == "null";
    }

    static int union_pattern(const json& schema) {
        if (schema.contains("x-union-pattern"))
            return schema["x-union-pattern"].get<int>();
        return 0;
    }

    static std::string x_model_default(const json& schema) {
        if (schema.contains("x-model-default"))
            return schema["x-model-default"].get<std::string>();
        return "";
    }

    // Insert a property's schema default into `node` if:
    //   - property is absent
    //   - schema has a non-null "default" (direct or in a non-null anyOf branch)
    static void insert_default(json& node, const std::string& prop, const json& pschema) {
        if (node.contains(prop)) return;

        // Direct default on the property schema.
        if (pschema.contains("default") && !pschema["default"].is_null()) {
            node[prop] = pschema["default"];
            return;
        }
        // Default inside a non-null anyOf branch (nullable fields pattern).
        if (pschema.contains("anyOf")) {
            for (auto& opt : pschema["anyOf"]) {
                if (is_null_type(opt)) continue;
                if (opt.contains("default") && !opt["default"].is_null()) {
                    node[prop] = opt["default"];
                    return;
                }
            }
        }
    }

    // Given a list of variant def names and a discriminator key, return the
    // definition name of the key's VALUE type (the $ref in the variant's property).
    // e.g. variant_defs=["DotMmfd","Coliny"], key="dot_mmfd" → "DotMmfdConfig"
    std::string child_def_for_key(const std::vector<std::string>& variant_defs,
                                  const std::string& key) const {
        for (const auto& vname : variant_defs) {
            auto it = defs_.find(vname);
            if (it == defs_.end()) continue;
            const json& vdef = it->second;
            if (!vdef.contains("properties")) continue;
            if (!vdef["properties"].contains(key)) continue;
            auto child_refs = all_refs(vdef["properties"][key]);
            if (!child_refs.empty()) return child_refs[0];
        }
        return "";
    }
};

// ============================================================================
// File loading
// ============================================================================

// Replace non-standard JSON infinity/NaN literals with sentinel strings
// before parsing.  Python's json.dumps() can produce "Infinity", "-Infinity",
// and "NaN" which are not valid JSON.  We map them to sentinel doubles that
// the expander/validator later recognises.
// Replace every bare (unquoted) occurrence of `from` with `to` in a JSON text.
// Only replaces when `from` is not inside a string literal.
inline void json_replace_bare(std::string& s,
                               const std::string& from,
                               const std::string& to) {
    bool in_string = false;
    bool escaped   = false;
    for (size_t i = 0; i < s.size(); ) {
        if (escaped) { escaped = false; ++i; continue; }
        if (s[i] == '\\' && in_string) { escaped = true; ++i; continue; }
        if (s[i] == '"') { in_string = !in_string; ++i; continue; }
        if (!in_string && s.compare(i, from.size(), from) == 0) {
            s.replace(i, from.size(), to);
            i += to.size();
        } else {
            ++i;
        }
    }
}

// After json::parse(), walk the tree and replace our unique string sentinels
// ("__POS_INF__" / "__NEG_INF__") with IEEE double infinity.  This preserves
// the distinction between user-supplied infinity and schema-default DBL_MAX.
inline void restore_inf_sentinels(json& j) {
    if (j.is_string()) {
        const std::string& s = j.get_ref<const std::string&>();
        if (s == "__POS_INF__") j = std::numeric_limits<double>::infinity();
        else if (s == "__NEG_INF__") j = -std::numeric_limits<double>::infinity();
    } else if (j.is_array()) {
        for (auto& el : j) restore_inf_sentinels(el);
    } else if (j.is_object()) {
        for (auto& [k, v] : j.items()) restore_inf_sentinels(v);
    }
}

inline std::string sanitize_json_for_parse(std::string s) {
    // Replace bare (unquoted) infinity/NaN literals with unique string sentinels.
    // After parsing, restore_inf_sentinels() converts them to IEEE double ±inf.
    // This keeps user-specified infinity distinct from schema-default DBL_MAX values.
    json_replace_bare(s, "-Infinity",  "\"__NEG_INF__\"");
    json_replace_bare(s, "Infinity",   "\"__POS_INF__\"");
    json_replace_bare(s, "-infinity",  "\"__NEG_INF__\"");
    json_replace_bare(s, "infinity",   "\"__POS_INF__\"");
    json_replace_bare(s, "NaN",  "null");
    json_replace_bare(s, "nan",  "null");
    return s;
}

inline bool load_json_file(const std::string& filename,
                           json& doc,
                           std::vector<std::string>& errors) {
    std::ifstream f(filename);
    if (!f.is_open()) {
        errors.push_back("Cannot open JSON file: " + filename);
        return false;
    }
    try {
        std::string content((std::istreambuf_iterator<char>(f)),
                             std::istreambuf_iterator<char>());
        doc = json::parse(sanitize_json_for_parse(content));
        restore_inf_sentinels(doc);
    } catch (const json::parse_error& e) {
        errors.push_back(std::string("JSON parse error: ") + e.what());
        return false;
    }
    return true;
}

// ============================================================================
// Convenience entry point
// ============================================================================

// Load schema from `schema_path`, then expand defaults in `doc`.
// Returns false on schema load failure or doc structural error.
inline bool expand_json_defaults(json& doc,
                                 const std::string& schema_path,
                                 std::vector<std::string>& errors,
                                 bool debug = false) {
    json schema;
    {
        std::ifstream sf(schema_path);
        if (!sf.is_open()) {
            errors.push_back("Cannot open schema file: " + schema_path);
            return false;
        }
        try {
            std::string sc((std::istreambuf_iterator<char>(sf)),
                            std::istreambuf_iterator<char>());
            schema = json::parse(sanitize_json_for_parse(sc));
            restore_inf_sentinels(schema);
        } catch (const json::parse_error& e) {
            errors.push_back(std::string("Schema parse error: ") + e.what());
            return false;
        }
    }

    JsonDefaultExpander expander(schema);
    return expander.expand(doc, debug);
}

} // namespace dakota

#endif // DAKOTA_JSON_INPUT_HPP
